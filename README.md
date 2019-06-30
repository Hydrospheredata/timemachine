# reqstore-cpp
rocksdb-cloud based kv service

## Development process

Since rocksdb-cloud couldn't be compiled in mac os, i do it inside docker container. 
It will allow you do incremental compilation and run app from container.
For more comfortable development you should use Visual Studio code.
All commands described in .vscode/tasks.json. To call them you should use Terminal/Run Task.

Commands:

**build develop image** builds developmen docker from Devdockerfile. 
**start develop env** starts dev container (builded by 'build develop image' cmd)

all below commands works if 'start develop env' performed (dev container started)

**stop develop env** stops dev container
**cmake generate** generates makefile
**build** compilation
**clean** clean all compilation info
**start-gdbserver** starts application in debug mode (you'll be able to debug it from vs code using gdb)
**run app** subj
**stop app** subj
**run app detached** subj

To start development you should use:
**build develop image** -> **start develop env** -> **cmake generate**

To compile:
**build**

## More

All intermidiate dockers for build in dockers folder

All grpc endpoint implementation in src/GRPCServer.cpp 
Http endpoints in src/handlers/... , entrypoint (router-like class) in src/handlers/HandlerFactory.cpp (more info in [POCO documentation](https://pocoproject.org/))

There are 2 implementations of DbClient (client to rocksdb-cloud):
- src/CloudDBClient.cpp uses rocksdb-client tools with aws s3
- src/LocalDBClient.cpp uses plain old rocksdb functionality without s3 (local mode)

configuration in src/Config.cpp

## Tests

in scalaClient folder you'll find sbt projects with e2e tests. To make it works localy (without s3) you should use env like that:
DOCKER_IMAGE=reqstore-cpp:latest;TEST_REGION=none;BACKUP_PROVIDER=none;TEST_AWS_KEY=none;DEBUG=1;TEST_AWS_SECRET=none
Where DOCKER_IMAGE image with app you want to test.

- HttpApiSpec.scala for http endpoints
- TimeMachineClientSpec.scala for grpc

## TODO list
- build info endpoint

