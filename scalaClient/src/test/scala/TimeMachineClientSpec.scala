package io.hydrosphere.timemachine

import java.util.concurrent.TimeUnit

import io.grpc.stub.StreamObserver
import org.scalatest.{FlatSpec, Matchers}
import timemachine.timeMachine.Data

import scala.concurrent.duration._
import scala.concurrent.{Await, Promise}


class TimeMachineClientSpec extends FlatSpec with Matchers {

  def fixture(sizeInMib: Int) = Array.fill[Byte](1024 * 1024 * sizeInMib)(1)

  val service = TimeMachineClient.client("127.0.0.1", 8999)

  "TimeMachine" should "save data to storage" in {



//    val values = for (i <- Range(0, 300)) yield {
//      val data = fixture(1)
//      val uuid = service.save("first", data)
//
//      TimeUnit.MILLISECONDS.sleep(100)
//
//      val data2 = service.get("first", uuid)
//      data2
//    }

    val promise = Promise[List[Data]]

    service.getRange("first", None, None, new StreamObserver[Data] {

      var list: List[Data] = List()

      override def onError(t: Throwable): Unit = promise.failure(t)

      override def onCompleted(): Unit = promise.success(list)

      override def onNext(value: Data): Unit = list = list :+ value

    })

    val r = Await.result(promise.future, 40 second)
    val f = 123

  }

}
