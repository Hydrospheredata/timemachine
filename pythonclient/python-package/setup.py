from setuptools import setup, find_packages

with open("../../version") as v:
    version = v.read()
pkgs = find_packages()
print("found packages", pkgs)
setup(
    name='reqstore_grpc',
    version=version,
    author='Hydrospheredata',
    author_email='info@hydrosphere.io',
    long_description='reqstore_grpc',
    description='reqstore_grpc',
    url='https://github.com/Hydrospheredata/timemachine',
    packages=pkgs,
    install_requires=['protobuf>=3.6.1', 'grpcio>=1.7.0'],
    zip_safe=True,
    license='Apache 2.0',
    classifiers=(
        'Natural Language :: English',
        'License :: OSI Approved :: Apache Software License',
        'Programming Language :: Python'
    )
)