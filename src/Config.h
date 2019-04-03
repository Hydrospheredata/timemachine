//
// Created by Dmitry Isaev on 2019-02-14.
//
#include <iostream>

#ifndef TIMEMACHINE_CONFIG_H
#define TIMEMACHINE_CONFIG_H

namespace timemachine {
    class Config {

    public:

        std::string ToString();
        Config ();

        void Init();

        char *keyid;
        char *secret;
        char *kRegion;
        char *walProvider;
        char *sourceLocalDir;
        char *destinationLocalDir;
        char *sourceBucket;
        char *destBucket;
        char *dbName;
        bool useKinesis;
        bool debug;

        char *gprc_port = nullptr;
        char *http_port = nullptr;
        int http_timeout = 100000;
        int http_max_queued = 100;
        int http_max_threads = 4;

    private:

        char *getEnvironmentVariableOrDefault(const std::string &variable_name, char *default_value);

        int getEnvironmentVariableOrDefaultInt(const std::string &variable_name, int default_value);

    };
}


#endif //TIMEMACHINE_CONFIG_H
