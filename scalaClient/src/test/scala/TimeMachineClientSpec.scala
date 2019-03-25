package io.hydrosphere.timemachine

import java.util.UUID
import java.util.concurrent.TimeUnit

import io.grpc.stub.StreamObserver
import org.scalatest.{FlatSpec, Matchers}
import timemachine.timeMachine.Data

import scala.concurrent.duration._
import scala.concurrent.{Await, ExecutionContext, Future, Promise}


class TimeMachineClientSpec extends FlatSpec with Matchers {

  implicit val ec = ExecutionContext.global

  def fixture(size: Int) = Array.fill[Byte](1024 * 1024 * size)(1)

  val service = TimeMachineClient.client("127.0.0.1", 8989)

//  val service2 = TimeMachineClient.client("127.0.0.1", 8999)

  "TimeMachine" should "save data to storage" in {


//
//    val data1 = for (i <- Range(0, 10)) yield {
//      val data = fixture(1)
//      val uuid = service.save("156", data)
//
//      TimeUnit.MILLISECONDS.sleep(1)
//
////      val data2 = Await.result(service.get("first", uuid), 30 second)
////      data2
//
//      uuid
//    }

    def fallback(future: Future[Data]) = future.map(Some(_)).fallbackTo(Future.successful(None))
//
//    val data2 = for(d <- data1.collect{case Data(Some(id), _) => TimeMachineClient.fromByteString(id.uuid)})
//      yield Await.result(fallback(service2.get("first", d)), 30 second)

    val promise1 = Promise[List[Data]]

  //  val uuid = new UUID(-1273898908248420814l, -4843414857704647621l)

    service.getRange("156", None, None, new StreamObserver[Data] {

      var list = List[Data]()

      override def onError(t: Throwable): Unit = {promise1.failure(t)}

      override def onCompleted(): Unit = {promise1.success(list)}

      override def onNext(value: Data): Unit = list = list :+ value

    })

//    val promise2 = Promise[List[Data]]

//    service2.getRange("first", None, None, new StreamObserver[Data] {
//
//      var list = List[Data]()
//
//      override def onError(t: Throwable): Unit = {promise2.failure(t)}
//
//      override def onCompleted(): Unit = {promise2.success(list)}
//
//      override def onNext(value: Data): Unit = list = list :+ value
//
//    })

    val f1 = promise1.future;
//    val f2 = promise2.future;

    val results = Future.sequence(Seq(f1))

    val r = Await.result(results, 300 seconds)

//    (-1273898908248420814l, -4843414857704647621l)
    val f = 123

  }

}
