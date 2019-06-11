package hydrosphere.timemachine

import io.grpc.stub.StreamObserver
import io.hydrosphere.reqstore.reqstore_service.Data
import io.hydrosphere.timemachine.TimeMachineClient
import org.scalatest.{FlatSpec, Matchers}

import scala.concurrent.duration._
import scala.concurrent.{Await, Future, Promise}

class TimeMachineClientSpec extends FlatSpec with Matchers with ReqtoreDockerKit {

  val service = TimeMachineClient.client("127.0.0.1", exposedGrpcPort)

  override val StartContainersTimeout = 120 seconds

  "reqstore" should "save messages to storage via grpc" in {

    val folderName = "4356"



    val saved = for(i <- Range(0, 300)) yield {


      val data = data2save()
      val id = Await.result(service.save(folderName, data, Some(i * 10000), true), 10 seconds)

      id.timestamp isValidLong

      val saved = Await.result(service.get(folderName, id.timestamp, id.unique), 10 seconds)
      saved.id shouldBe(Some(id))

      saved
    }

    val allMessages = rangeRequest(folderName)
    val list = Await.result(allMessages, 10 seconds)
    val idsList = list.collect{case Data(Some(id), _) => id}

    for(s <- saved){
      idsList.contains(s.id.get) shouldBe(true)
    }

    val last5 = rangeRequest(folderName, maxMessages = 5, reverse = true)

    val lm = Await.result(last5, 10 seconds)

    lm.size shouldBe(5)
    (lm(4).id.get.timestamp < lm(0).id.get.timestamp) shouldBe(true)

    val futureSample1 = Await.result(sampling(folderName, None, 3), 10 seconds)
    futureSample1.size shouldBe(3)
    val futureSample2 = Await.result(sampling(folderName, None, 40), 10 seconds)
    futureSample2.size shouldBe(40)
    val futureSample3 = Await.result(sampling(folderName, None, 20, 40), 10 seconds)
    futureSample3.size shouldBe(20)
    val futureSample4 = Await.result(sampling(folderName, Some(2500000, 3000000), 20), 10 seconds)
    futureSample4.size shouldBe(20)

  }

  def sampling(folderName:String, period:Option[(Long, Long)], amount:Int, step:Int = 5):Future[List[Data]] = {
    val listPromise = Promise[List[Data]]

    val so = new StreamObserver[Data] {

      var list = List[Data]()

      override def onError(t: Throwable): Unit = {listPromise.failure(t)}

      override def onCompleted(): Unit = {listPromise.success(list)}

      override def onNext(value: Data): Unit = list = list :+ value

    }

    period.map(r => service.rangeSubsample(folderName, r._1, r._2, amount, step, so)).getOrElse{
      service.totalSubsample(folderName, amount, step, so)
    }

    return listPromise.future
  }

  def rangeRequest(folderName:String,
                   from: Option[Long] = None,
                   till:Option[Long] = None,
                   maxMessages:Long = 0,
                   maxBytes:Long = 0,
                   reverse:Boolean = false):Future[List[Data]] = {

    val listPromise = Promise[List[Data]]

    service.getRange(folderName, from, till, maxMessages, maxBytes, reverse,  new StreamObserver[Data] {

      var list = List[Data]()

      override def onError(t: Throwable): Unit = {listPromise.failure(t)}

      override def onCompleted(): Unit = {listPromise.success(list)}

      override def onNext(value: Data): Unit = list = list :+ value

    })

    return listPromise.future;

  }



}
