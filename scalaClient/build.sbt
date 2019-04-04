name := "timeMachineClient"

version := "0.1"

scalaVersion := "2.12.8"

libraryDependencies ++= Seq(
  "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,
  "com.thesamet.scalapb" %% "scalapb-runtime-grpc" % scalapb.compiler.Version.scalapbVersion,
  "com.softwaremill.sttp" %% "core" % "1.5.11" % "test",
  "org.scalatest" %% "scalatest" % "3.0.5" % "test",
  "com.whisk" %% "docker-testkit-scalatest" % "0.9.8" % "test",
  "com.whisk" %% "docker-testkit-impl-spotify" % "0.9.8" % "test",
  "ch.qos.logback" % "logback-classic" % "1.1.3" % "test",
  "io.spray" %%  "spray-json" % "1.3.5" % "test",
  "io.hydrosphere" % "serving-grpc-scala_2.12" % "2.0.0" % "test"
)


PB.targets in Compile := Seq(
  scalapb.gen() -> (sourceManaged in Compile).value
)

lazy val log = taskKey[Unit]("Prints 'Hello World'")

log := println(baseDirectory.value / ".." / "src" / "proto")

PB.protoSources in Compile += baseDirectory.value / ".." / "src" / "proto"