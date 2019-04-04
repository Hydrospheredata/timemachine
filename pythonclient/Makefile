INSTALL_PY_REQ = false
VERSION := $(shell cat version)
BASE_DIR := $(shell pwd)
PYTHON = python3
PROTOC = protoc
PROTOS_PATH = ../src/proto
PY_WORK_PATH = python-package
PROTO_FILES := $(shell find ../src/proto -name '*.proto')
GRPC_FILES = $(shell find ../src/proto -name '*.proto')


all: scala python

scala:
	cd scala-package && sbt +package

scala_publish_local:
	cd scala-package && sbt +publishLocal

python: python_wheel

python_wheel: python_grpc
	cd $(PY_WORK_PATH) && $(PYTHON) setup.py bdist_wheel

python_grpc: python_proto | py_requirements
	$(PYTHON) -m grpc_tools.protoc -I $(PROTOS_PATH) --python_out=$(PY_WORK_PATH) --grpc_python_out=$(PY_WORK_PATH) $(GRPC_FILES)

python_proto:
	$(PYTHON) -m grpc_tools.protoc -I $(PROTOS_PATH) --python_out=$(PY_WORK_PATH) $(PROTO_FILES)

py_requirements:
ifeq ($(INSTALL_PY_REQ), true)
	pip install -r python-package/requirements.txt
endif
