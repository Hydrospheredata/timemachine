def isRelease() {
  return env.IS_RELEASE_JOB == "true"
}

node("JenkinsOnDemand") {
    def organization = 'Provectus'
    def repository = 'reqstore_cpp'
    def accessTokenId = 'HydroRobot_AccessToken' 
    def curVersion = getVersion()
    def imageToCompile = "hydrosphere/${repository}:${curlVersion}"
    
    stage("Checkout") {
        def branches = (isRelease()) ? [[name: env.BRANCH_NAME]] : scm.branches
        checkout([
            $class: 'GitSCM',
            branches: branches,
            doGenerateSubmoduleConfigurations: scm.doGenerateSubmoduleConfigurations,
            extensions: [[$class: 'CloneOption', noTags: false, shallow: false, depth: 0, reference: '']],
            userRemoteConfigs: scm.userRemoteConfigs,
       ])
   }
   stage('Build image') {
       echo imageToCompile
       sh "docker build -t ${imageToCompile} ."
   }

   stage('test') {
       echo 'end2end tests'
       sh "export DOCKER_IMAGE=${imageToCompile} && cd scalaClient && sbt test"
   }

  
//   if (isRelease()) {
//     stage("Publish docker") {
//       sh "sbt dockerBuildAndPush"
//     } 
//   }
}
