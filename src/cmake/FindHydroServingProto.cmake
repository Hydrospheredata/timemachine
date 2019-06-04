include(ExternalProject)
ExternalProject_Add(protos
        GIT_REPOSITORY https://github.com/Hydrospheredata/hydro-serving-protos.git
        GIT_TAG "origin/subsampling"
        TIMEOUT 300
        PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/proto"
        CONFIGURE_COMMAND "" # Disable configure step
        BUILD_COMMAND "" # Disable build step
        INSTALL_COMMAND "" # Disable install step
        UPDATE_COMMAND "" # Disable update step: clones the project only once
        )

# Specify include dir
ExternalProject_Get_Property(spdlog source_dir)
set(spdlog_INCLUDE_DIR ${source_dir}/include)
#endif()

if(EXISTS "${spdlog_INCLUDE_DIR}")
set(spdlog_FOUND 1)
else()
set(spdlog_FOUND 0)
endif()