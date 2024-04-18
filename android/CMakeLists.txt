
cmake_minimum_required(VERSION 3.18.1)
project(MassiveVinScannerPlugin LANGUAGES C CXX)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/ncnn-20230816-android/${ANDROID_ABI}/include/ncnn
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

add_library(lib_opencv STATIC IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/jni/main/jniLibs/${ANDROID_ABI}/libopencv_java4.so)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")

if (DEFINED ANDROID_NDK_MAJOR AND ${ANDROID_NDK_MAJOR} GREATER 20)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-openmp")
endif ()


set(ncnn_DIR ${CMAKE_SOURCE_DIR}/ncnn-20230816-android/${ANDROID_ABI}/lib/cmake/ncnn)
find_package(ncnn REQUIRED)

set_target_properties(
        ncnn PROPERTIES
        INTERFACE_COMPILE_OPTIONS "-frtti;-fexceptions"
        # ncnn.cmake 里面是关的，把它重新打开防止跟opencv2冲突，如果是重新编译ncnn的请自己尝试要开还是关
)

set(SOURCES
    ../ios/Classes/opencv-cpp/main.cpp
)
find_library( # Defines the name of the path variable that stores the
              # location of the NDK library.
              log-lib

              # Specifies the name of the NDK library that
              # CMake needs to locate.
              log )

add_library(OpenCV_ffi SHARED ${SOURCES})
target_compile_definitions(OpenCV_ffi PUBLIC USE_NCNN_SIMPLEOCV)
target_link_libraries(OpenCV_ffi lib_opencv ncnn ${log-lib} )




