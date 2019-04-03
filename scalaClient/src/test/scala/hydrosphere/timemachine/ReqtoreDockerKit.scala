package hydrosphere.timemachine

import java.net.ServerSocket

import com.whisk.docker.impl.spotify.DockerKitSpotify
import com.whisk.docker.{DockerContainer, DockerReadyChecker, LogLineReceiver}
import org.scalatest.{BeforeAndAfterAll, Suite}
import org.scalatest.concurrent.ScalaFutures
import org.scalatest.time.{Millis, Seconds, Span}
import org.slf4j.LoggerFactory
import scala.concurrent.duration._

trait ReqtoreDockerKit extends DockerKitSpotify with BeforeAndAfterAll with ScalaFutures { self: Suite =>

  def dockerInitPatienceInterval =
    PatienceConfig(scaled(Span(60, Seconds)), scaled(Span(10, Millis)))

  def dockerPullImagesPatienceInterval =
    PatienceConfig(scaled(Span(1200, Seconds)), scaled(Span(250, Millis)))

  override def beforeAll(): Unit = {
    super.beforeAll()
    startAllOrFail()
  }

  override def afterAll(): Unit = {
    stopAllQuietly()
    super.afterAll()

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


  val bucket: String = sys.env("TEST_BUCKET")
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
      "DB_NAME=first",
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
