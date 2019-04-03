package hydrosphere.timemachine

import java.nio.ByteBuffer

import org.scalatest.{FlatSpec, Matchers}
import com.softwaremill.sttp._

import scala.concurrent.duration._
import spray.json._
import DefaultJsonProtocol._

class HttpApiSpec extends FlatSpec with Matchers with ReqtoreDockerKit  {

  case class Id(ts:Long, uniq:Long)

  implicit val idFormat = jsonFormat2(Id)

  override val StartContainersTimeout = 120 seconds

  def fixture(size: Int) = Array.fill[Byte](1024 * size)(1)


  "reqstore" should "save messages to storage via http" in {

    val folder = "3"

    val postUri = uri"http://localhost:$exposedHttpPort/$folder/put"
    val getUri = uri"http://localhost:$exposedHttpPort/$folder/get"

    implicit val backend = HttpURLConnectionBackend()

    val requestPost = sttp.response(asString)
      .body(fixture(20))
      .post(postUri)

    val response = requestPost.send()
    val body = response.unsafeBody

    val jsonAst = body.parseJson
    val id = jsonAst.convertTo[Id]

    id.ts isValidLong

    val requestGet = sttp.response(asByteArray).get(getUri)

    val response2 = requestGet.send()

    val b = response2.unsafeBody
    val bb = ByteBuffer.wrap(b)

    var list = List[Id]()

    while (bb.position() < bb.limit()){
      val ts= bb.getLong()
      val unique = bb.getLong()
      val nextSize = bb.getInt()
      val data = Array.fill[Byte](nextSize)(0)
      bb.get(data)

      val newId = Id(ts, unique)

      log.info("id is: {}, data size: {}", newId, nextSize)

      list = newId :: list
    }

    list.contains(id) shouldBe(true)


  }

}
