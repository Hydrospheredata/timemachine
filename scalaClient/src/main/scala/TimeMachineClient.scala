package io.hydrosphere.timemachine

import java.util.UUID

import com.google.protobuf.ByteString
import io.grpc.ManagedChannelBuilder
import java.nio.ByteBuffer

import io.grpc.stub.StreamObserver

import scala.concurrent.{Await, Future}
import timemachine.timeMachine._

object TimeMachineClient {


  def client(host:String, port:Int):TimeMachineClient = {
    val channel = ManagedChannelBuilder.forAddress(host, port).usePlaintext().build
    new TimeMachineClient(TimemachineGrpc.stub(channel));
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

  import TimeMachineClient._

  def save(folder:String, data: Array[Byte]): UUID = {
    val uuid = UUID.randomUUID();
    val id = ID(uuid.toByteString(), folder)
    val r = stub.save(Data(Some(id), ByteString.copyFrom(data)))

    import scala.concurrent.duration._

    val response = Await.result(r, 100 seconds)

    uuid
  }

  def get(folder:String, uUID: UUID): Future[Data] = {
    val id = ID(uUID.toByteString(), folder)
    stub.get(id)
  }

  def getRange(folder:String, from: Option[UUID], till:Option[UUID], so:StreamObserver[Data]):Unit = {
    val range = Range(folder)

    val r = for{
      r1 <- from.map(f => range.withFrom(f.toByteString())).orElse(Some(range))
      r2 <- till.map(t => r1.withTill(t.toByteString())).orElse(Some(r1))
    } yield r2

    stub.getRange(r.getOrElse(range), so)
  }

}
