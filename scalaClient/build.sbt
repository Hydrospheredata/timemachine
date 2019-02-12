name := "timeMachineClient"

version := "0.1"

scalaVersion := "2.12.8"

libraryDependencies ++= Seq(
  "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,
  "com.thesamet.scalapb" %% "scalapb-runtime-grpc" % scalapb.compiler.Version.scalapbVersion,
  "org.scalatest" %% "scalatest" % "3.0.5" % "test"
)




PB.targets in Compile := Seq(
  scalapb.gen() -> (sourceManaged in Compile).value
)

lazy val log = taskKey[Unit]("Prints 'Hello World'")

log := println(baseDirectory.value / ".." / "src" / "proto")

PB.protoSources in Compile += baseDirectory.value / ".." / "src" / "proto"