name := "timeMachineClient"

version := "0.1"

scalaVersion := "2.12.8"

libraryDependencies ++= Seq(
  "io.grpc" % "grpc-netty" % scalapb.compiler.Version.grpcJavaVersion,
  "com.thesamet.scalapb" %% "scalapb-runtime-grpc" % scalapb.compiler.Version.scalapbVersion,
  "io.hydrosphere" %% "serving-grpc-scala" % "2.0.4-preview3",
  "com.softwaremill.sttp" %% "core" % "1.5.11" % "test",
  "org.scalatest" %% "scalatest" % "3.0.5" % "test",
  "com.whisk" %% "docker-testkit-scalatest" % "0.9.8" % "test",
  "com.whisk" %% "docker-testkit-impl-spotify" % "0.9.8" % "test",
  "ch.qos.logback" % "logback-classic" % "1.1.3" % "test",
  "io.spray" %%  "spray-json" % "1.3.5" % "test"
)


PB.targets in Compile := Seq(
  scalapb.gen() -> (sourceManaged in Compile).value
)
