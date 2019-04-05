def isRelease() {
  return env.IS_RELEASE_JOB == "true"
}

def readVersion(){
    def v = sh 'cat version.txt'
}

def getUpdatedVersion(String versionType, String currentVersion){

    def split = currentVersion.split('\\.')
    switch (versionType){
        case "minor.minor":
            split[2]=++Integer.parseInt(split[2])
            break
        case "minor":
            split[1]=++Integer.parseInt(split[1])
            break;
        case "major":
        split[0]=++Integer.parseInt(split[0])
        break;
    }
    return split.join('.')
}

node("JenkinsOnDemand") {
    def organization = 'Provectus'
    def repository = 'reqstore_cpp'
    def accessTokenId = 'HydroRobot_AccessToken' 
    def prevVersion = readVersion()
    def curVersion = getUpdatedVersion("minor.minor", prevVersion)
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
