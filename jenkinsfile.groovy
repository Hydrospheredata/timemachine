def isRelease() {
  return env.IS_RELEASE_JOB == "true"
}

def readVersion(){
    def version = sh returnStdout: true, script: 'cat version.txt'
    return version
}

def getUpdatedVersion(String versionType, String currentVersion){

    def split = currentVersion.split('\\.')
    split = [split[0], split[1], split[2].split('-')[0]]
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

    def version = split.join('.')

    if(!isRelease()){
        version = "${version}-SNAPSHOT")
    }

    return version
}

node("JenkinsOnDemand") {
    def organization = 'Provectus'
    def repository = 'reqstore_cpp'
    def accessTokenId = 'HydroRobot_AccessToken' 

    
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
       env.LAST_VERSION = readVersion()
       env.NEW_VERSION = getUpdatedVersion("minor.minor", env.LAST_VERSION)
       env.DOCKER_IMAGE = "hydrosphere/${repository}:${env.NEW_VERSION}"
       sh "docker build -t ${env.DOCKER_IMAGE} ."
   }

   stage('test') {
       sh 'docker images'
       sh "cd scalaClient && sbt test"
   }

  
  if (isRelease()) {
    stage("Publish docker") {
      sh "docker push ${env.DOCKER_IMAGE}"
    } 
  }


}
