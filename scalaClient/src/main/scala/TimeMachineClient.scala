package io.hydrosphere.timemachine

import java.util.UUID

import com.google.protobuf.ByteString
import io.grpc.ManagedChannelBuilder
import java.nio.ByteBuffer
import java.util.concurrent.atomic.AtomicLong


import io.grpc.stub.StreamObserver
import io.hydrosphere.reqstore.reqstore_service._

import scala.concurrent.{Await, Future}




object TimeMachineClient {


  def client(host:String, port:Int):TimeMachineClient = {
    val channel = ManagedChannelBuilder.forAddress(host, port)
      .maxInboundMessageSize(Int.MaxValue)
      .usePlaintext()

    new TimeMachineClient(TimemachineGrpc.stub(channel.build));
  }

  implicit class UUIDExtension(uuid:UUID){

    def toByteString():ByteString = {
      val bb = ByteBuffer.wrap(new Array[Byte](16))
      bb.putLong(uuid.getMostSignificantBits)
      bb.putLong(uuid.getLeastSignificantBits)
      bb.position(0)

      ByteString.copyFrom(bb)
    }

  }

  def fromByteString(bs: ByteString):UUID = {
    val bb = bs.asReadOnlyByteBuffer()
    val high = bb.getLong
    val low = bb.getLong

    new UUID(high, low)
  }


}

class TimeMachineClient(stub: TimemachineGrpc.TimemachineStub){

  def save(folder:String, data: Array[Byte], useWAL:Boolean = false): Future[ID] = {

    val request = SaveRequest(folder=folder, useWAL = useWAL, data = ByteString.copyFrom(data))
    stub.save(request)

  }

  def get(folder:String, ts: Long, inc:Long): Future[Data] = {
    val request = GetRequest(ts, inc, folder)
    stub.get(request)
  }

  def getRange(folder:String,
               from: Option[Long],
               till:Option[Long],
               maxMessages:Long = 0,
               maxBytes:Long = 0,
               reverse:Boolean = false,
               so:StreamObserver[Data]):Unit = {
    val range = RangeRequest(from.getOrElse(0), till.getOrElse(0), folder, reverse, maxMessages, maxBytes)
    stub.getRange(range, so)
  }



}
