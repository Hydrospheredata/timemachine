package hydrosphere.timemachine

import java.nio.ByteBuffer

import org.scalatest.{FlatSpec, Matchers}
import com.softwaremill.sttp._

import scala.concurrent.duration._
import spray.json._
import DefaultJsonProtocol._
import io.grpc.stub.StreamObserver
import org.scalatest.Matchers._
import org.scalatest._
import Assertions._


import scala.concurrent.{Future, Promise}

class HttpApiSpec extends FlatSpec with Matchers with ReqtoreDockerKit  {

  case class Id(ts:Long, uniq:Long)

  implicit val idFormat = jsonFormat2(Id)

  override val StartContainersTimeout = 120 seconds

  val folder = "123"

  def postUri(ts:Long) = uri"http://localhost:$exposedHttpPort/$folder/put?timestamp=$ts"
  val getUri = uri"http://localhost:$exposedHttpPort/$folder/get"
  val subsamplingUri = uri"http://localhost:$exposedHttpPort/$folder/subsampling"
  implicit val backend = HttpURLConnectionBackend()


  "reqstore" should "save messages to storage via http" in {



    def requestPost(ts:Long) = sttp.response(asString)
      .body(data2save())
      .post(postUri(ts))

    val ids = Range(1, 300).map{i =>

      val response = requestPost(i * 10000).send()
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

    val subsampling = subsamplingRequest(3)
    subsampling.size shouldBe(3)

    val subsampling2 = subsamplingRequest(40)
    subsampling2.size shouldBe(40)

    val subsampling3 = subsamplingRequest(20, 40)
    subsampling3.size shouldBe(20)

    val subsampling4 = subsamplingRequestByTs(20,2500000, 3000000, 100)
    subsampling4.size shouldBe(20)
    val minIdUnique:Long = subsampling4.map(_.ts).min

  }

  def subsamplingRequestByTs(amount:Int, tsFrom:Long, tsTill:Long, step:Int = 5):List[Id] ={
    val url = uri"http://localhost:$exposedHttpPort/$folder/subsampling?amount=$amount&from=$tsFrom&to=$tsTill&step=$step"
    val requestGet = sttp.response(asByteArray).get(url)
    val response = requestGet.send()
    bodyToIdList(response.unsafeBody)
  }

  def subsamplingRequest(amount:Int, step:Int = 5):List[Id] = {
    val url = uri"http://localhost:$exposedHttpPort/$folder/subsampling?amount=$amount&step=$step"
    val requestGet = sttp.response(asByteArray).get(url)
    val response = requestGet.send()
    bodyToIdList(response.unsafeBody)
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
    bodyToIdList(b)

  }

  def bodyToIdList(byteArray:Array[Byte]):List[Id] = {
    val bb = ByteBuffer.wrap(byteArray)

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
