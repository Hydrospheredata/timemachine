def isRelease() {
  return env.IS_RELEASE_JOB == "true"
}

def readVersion(){
    env.WORKSPACE = pwd()
    def version = readFile "version.txt"
}

def getUpdatedVersion(String versionType, String currentVersion){

    def split = currentVersion.split('\\.')
    switch (versionType){
        case "minor.minor":
            split[2] = Integer.parseInt(split[2]) + 1
            break
        case "minor":
            split[1] = Integer.parseInt(split[1]) + 1
            break;
        case "major":
        split[0] = Integer.parseInt(split[0]) + 1
        break;
    }
    return split.join('.')
}

node("JenkinsOnDemand") {
    def organization = 'Provectus'
    def repository = 'reqstore_cpp'
    def accessTokenId = 'HydroRobot_AccessToken' 

    
    stage("Checkout") {
        echo 'checkout'
        env.LAST_VERSION = readVersion()
        env.NEW_VERSION = getUpdatedVersion("minor.minor", env.LAST_VERSION)
        env.DOCKER_IMAGE = "hydrosphere/${repository}:${env.NEW_VERSION}"

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
       sh 'docker images'
       sh "cd scalaClient && sbt test && docker images"
   }

  
//   if (isRelease()) {
//     stage("Publish docker") {
//       sh "sbt dockerBuildAndPush"
//     } 
//   }


}
