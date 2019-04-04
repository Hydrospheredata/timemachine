package hydrosphere.timemachine

import java.net.ServerSocket
import java.nio.ByteBuffer

import com.whisk.docker.impl.spotify.DockerKitSpotify
import com.whisk.docker.{DockerContainer, DockerReadyChecker, LogLineReceiver}
import io.hydrosphere.serving.tensorflow.api.model.ModelSpec
import io.hydrosphere.serving.tensorflow.api.predict.{PredictRequest, PredictResponse}
import io.hydrosphere.serving.tensorflow.tensor.TensorProto
import org.scalatest.{BeforeAndAfterAll, Suite}
import org.scalatest.concurrent.ScalaFutures
import org.scalatest.time.{Millis, Seconds, Span}
import org.slf4j.LoggerFactory

import scala.collection.immutable.{Range => ScalaRange}
import scala.concurrent.duration._

trait ReqtoreDockerKit extends DockerKitSpotify with BeforeAndAfterAll with ScalaFutures { self: Suite =>

  def dockerInitPatienceInterval =
    PatienceConfig(scaled(Span(60, Seconds)), scaled(Span(10, Millis)))

  def dockerPullImagesPatienceInterval =
    PatienceConfig(scaled(Span(1200, Seconds)), scaled(Span(250, Millis)))

  def useS3 = true

  override def beforeAll(): Unit = {
    super.beforeAll()
    startAllOrFail()
  }

  override def afterAll(): Unit = {
    stopAllQuietly()
    super.afterAll()

  }

  // as in gateway
  def data2save():Array[Byte] = {

    val pRequestBody = PredictRequest
      .defaultInstance
      .withModelSpec(ModelSpec("wtf", Some(1), "fdfdfdf"))
      .toByteArray

    val pRequest = ByteBuffer.allocate(pRequestBody.size + 1).put(1.toByte).put(pRequestBody).array()

    val pResponseBody = PredictResponse
      .defaultInstance
        .withInternalInfo(Map("key" -> TensorProto.defaultInstance))
      .toByteArray

    val pResponse = ByteBuffer.allocate(pResponseBody.size + 1).put(1.toByte).put(pResponseBody).array()

    val respOrError = ByteBuffer.allocate(4 + pResponse.size)
      .putInt(3)
      .put(pResponse)
      .array()

    val header =  ByteBuffer.allocate(8)
      .putInt(pRequest.size)
      .putInt(respOrError.size)
      .array()

    val result = ByteBuffer.allocate(header.size + pRequest.size + respOrError.size)

    val item = result.put(header)
      .put(pRequest)
      .put(respOrError)
      .array()


    //check data

    val testbb = ByteBuffer.wrap(item)

    val (reqSize, resSize) = (testbb.getInt, testbb.getInt)
    val reqB = Array.fill[Byte](reqSize)(0)
    val resB = Array.fill[Byte](resSize - 4)(0)

    testbb.get(reqB)
    val index = testbb.getInt
    testbb.get(resB)

    val res = PredictResponse.parseFrom(resB.slice(1, resB.size))
    val req = PredictRequest.parseFrom(reqB.slice(1, reqB.size))

    return item

  }

  val httpPort = 8080
  val grpcPort = 8081

  val exposedHttpPort = freePort()
  val exposedGrpcPort = freePort()

  protected lazy val log = LoggerFactory.getLogger(this.getClass)

  private def freePort() = {
    val socket = new ServerSocket(0)
    socket.setReuseAddress(true)
    socket.getLocalPort()
  }


  def bucket: String = sys.env("TEST_BUCKET")
  val region: String = sys.env("TEST_REGION")
  val awsKey: String = sys.env("TEST_AWS_KEY")
  val awsSecret: String = sys.env("TEST_AWS_SECRET")

  def writeDockerLog(in:String):Unit = {
    log.debug("[DOCKER[reqstore]] "+in)
  }

  // running reqstor in local file WAL mode (if useWAL in message == true)
  val reqstoreContainer = DockerContainer("timemachine2:latest")
    .withPorts(
      httpPort -> Some(exposedHttpPort),
      grpcPort -> Some(exposedGrpcPort)
    )
    .withEnv(
      s"DST_LOCAL_DIR=$bucket",
      s"DST_BUCKET=$bucket",
      s"SRC_LOCAL_DIR=$bucket",
      s"SRC_BUCKET=$bucket",
      s"AWS_DEFAULT_REGION=$region",
      s"AWS_ACCESS_KEY_ID=$awsKey",
      s"AWS_SECRET_ACCESS_KEY=$awsSecret",
      s"DB_NAME=$bucket",
      s"BACKUP_PROVIDER=${if (useS3) "s3" else "none" }",
      "DEBUG=1"
    ).withReadyChecker(
    DockerReadyChecker.HttpResponseCode(httpPort, "/health", Some("localhost"))
      .looped(50, 3 seconds)
  ).withLogLineReceiver(
    LogLineReceiver(
      true,
      line => log.debug("[DOCKER[reqstore]] " + line )
    )
  )

  abstract override def dockerContainers: List[DockerContainer] =
    reqstoreContainer :: super.dockerContainers


}
