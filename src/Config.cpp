//
// Created by Dmitry Isaev on 2019-02-14.
//

#include "Config.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <stdlib.h>

namespace timemachine {

    char* Config::getEnvironmentVariableOrDefault(const std::string& variable_name, char* default_value)
    {
        const char* value = getenv(variable_name.c_str());
        return value ? (char *)value : default_value;
    }

    std::string Config::ToString() {
        std::stringstream ss;

        ss << "Config(" << std::endl <<
        " keyid: " << keyid << std::endl <<
        " secret: <secret>" << std::endl <<
        " kRegion: " << kRegion << std::endl <<
        " walProvider: " << walProvider << std::endl <<
        " sourceLocalDir: " << sourceLocalDir << std::endl <<
        " destinationLocalDir: " << destinationLocalDir << std::endl <<
        " sourceBucket: " << sourceBucket << std::endl <<
        " destBucket: " << destBucket << std::endl <<
        " dbName: " << dbName << std::endl <<
        " useKinesis: " << useKinesis << std::endl <<
        " )";

        return ss.str();
    }

    Config::Config() {
        keyid = getenv("AWS_ACCESS_KEY_ID");
        secret = getenv("AWS_SECRET_ACCESS_KEY");
        kRegion = getenv("AWS_DEFAULT_REGION");
        dbName = getenv("DB_NAME");

        char* debugMode = getenv("DEBUG");

        if(debugMode != nullptr){
            debug = true;
        }

        sourceLocalDir = getEnvironmentVariableOrDefault("SRC_LOCAL_DIR", (char* )"default-timemachine");
        destinationLocalDir = getEnvironmentVariableOrDefault("DST_LOCAL_DIR", (char* )"default-timemachine");
        sourceBucket = getEnvironmentVariableOrDefault("SRC_BUCKET", (char* )"default-timemachine");
        destBucket = getEnvironmentVariableOrDefault("DST_BUCKET", (char* )"default-timemachine");
        walProvider = getEnvironmentVariableOrDefault("WAL_PROVIDER", (char* )"localFile");

        useKinesis = walProvider != nullptr && (strcmp(walProvider, "kinesis") == 0);

        if (keyid == nullptr || secret == nullptr  || kRegion == nullptr || dbName == nullptr) {
            fprintf(
                    stderr,
                    "Please set env variables "
                    "AWS_BUCKET_NAME, AWS_DEFAULT_REGION, AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY with cloud credentials");
            throw std::runtime_error("Please set env variables AWS_BUCKET_NAME, AWS_DEFAULT_REGION, AWS_ACCESS_KEY_ID and AWS_SECRET_ACCESS_KEY with cloud credentials");
        }
    }

}
