//
// Created by Dmitry Isaev on 2019-02-14.
//
#include <iostream>
#ifndef TIMEMACHINE_CONFIG_H
#define TIMEMACHINE_CONFIG_H

namespace timemachine {
    class Config {

    public:

        Config();
        std::string ToString();

        char *keyid = nullptr;
        char *secret = nullptr;
        char *kRegion = nullptr;
        char *walProvider = nullptr;
        char *sourceLocalDir = nullptr;
        char *destinationLocalDir = nullptr;
        char *sourceBucket = nullptr;
        char *destBucket = nullptr;
        char *dbName = nullptr;
        bool useKinesis = false;
        bool debug = false;

    private:

        char* getEnvironmentVariableOrDefault(const std::string& variable_name, char* default_value);

    };
}


#endif //TIMEMACHINE_CONFIG_H
