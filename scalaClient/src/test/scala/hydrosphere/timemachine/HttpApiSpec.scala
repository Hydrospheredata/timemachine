package hydrosphere.timemachine

import java.nio.ByteBuffer

import org.scalatest.{FlatSpec, Matchers}
import com.softwaremill.sttp._

import scala.concurrent.duration._
import spray.json._
import DefaultJsonProtocol._
import io.grpc.stub.StreamObserver
import timemachine.timeMachine.Data

import scala.concurrent.{Future, Promise}

class HttpApiSpec extends FlatSpec with Matchers with ReqtoreDockerKit  {

  case class Id(ts:Long, uniq:Long)

  implicit val idFormat = jsonFormat2(Id)

  override val StartContainersTimeout = 120 seconds

  override def bucket: String = "otherone"

  val folder = "123"

  val postUri = uri"http://localhost:$exposedHttpPort/$folder/put"
  val getUri = uri"http://localhost:$exposedHttpPort/$folder/get"
  implicit val backend = HttpURLConnectionBackend()


  "reqstore" should "save messages to storage via http" in {



    val requestPost = sttp.response(asString)
      .body(data2save())
      .post(postUri)

    val ids = Range(0, 10).map{_ =>

      val response = requestPost.send()
      val body = response.unsafeBody

      val jsonAst = body.parseJson
      val id = jsonAst.convertTo[Id]

      id.ts isValidLong

      id
    } toList

    val list = rangeRequest(folder)


    ids.foreach{id =>
      list.contains(id) shouldBe(true)
    }

    val last5 = rangeRequest(folder, reverse = true, maxMessages = 5)

    last5.size shouldBe(5)
    (last5(4).ts < last5(0).ts) shouldBe(true)

  }

  def rangeRequest(folderName:String,
                   from: Option[Long] = None,
                   till:Option[Long] = None,
                   maxMessages:Long = 0,
                   maxBytes:Long = 0,
                   reverse:Boolean = false):List[Id] = {

    var qs:List[String] = List()
    if(from.isDefined) qs = s"from=${from.get}" :: qs
    if(till.isDefined) qs = s"to=${from.get}" :: qs
    if(maxMessages > 0) qs = s"maxMessages=5" :: qs
    if(maxBytes > 0) qs = s"maxBytes=${maxBytes}" :: qs
    if(reverse == true) qs = s"reverse=true" :: qs

    val url =  uri"http://localhost:$exposedHttpPort/$folder/get?from=${from.getOrElse(0)}&to=${till
      .getOrElse(0)}&maxBytes=${maxBytes}&maxMessages=${maxMessages}&reverse=${reverse}"

    val requestGet = sttp.response(asByteArray).get(url)

    val response2 = requestGet.send()

    val b = response2.unsafeBody
    val bb = ByteBuffer.wrap(b)

    var list = List[Id]()

    while (bb.position() < bb.limit()){
      val ts = bb.getLong()
      val unique = bb.getLong()
      val nextSize = bb.getInt()
      val data = Array.fill[Byte](nextSize)(0)
      bb.get(data)

      val newId = Id(ts, unique)

      log.info("id is: {}, data size: {}", newId, nextSize)

      list = newId :: list
    }

    return list.reverse;

  }

}
