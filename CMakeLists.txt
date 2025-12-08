#1、指定cmake最小版本
cmake_minimum_required(VERSION 3.10)

#2、设置项目名称
#3、设置变量 包含交叉编译器类型
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR aarch64) 
#设置平台切换变量如x86_64 HI3559AV100 RK3588J RK3399PRO
SET(CMAKE_C_COMPILER  aarch64-none-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER  aarch64-none-linux-gnu-g++)

SET(CMAKE_FIND_ROOT_PATH  /mnt/d/work/softwork/gcc-arm-10.2-2020.11-x86_64-aarch64-none-linux-gnu/bin)
project(29003_XTKZ_A)

#添加宏定义
add_definitions(-DRKNN)
add_definitions(-DBUILD_WITH_OPENCV)
add_definitions(-DENABLE_DEBUG -DABC) # 参数之间用空格分隔

#4、设置编译工程类型和C/C++编译优化参数
set(CMAKE_BUILD_TYPE RELEASE)
#set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXE_LINKER_FLAGS "-pthread -lrt -ldl")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lrt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lrt")
#set(CMAKE_CXX_FLAGS "-g -Wall -O2 -std=c++11 -fpermissive -Wno-deprecated-declarations -Wall -O3 -fPIC")

# suppress specific types of warnings
add_definitions(-Wno-attributes -Wno-deprecated-declarations)
# skip 3rd-party lib dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")
# install target and libraries
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/out/install/rk3588_29003_XTKZ_A_install)
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_CXX_STANDARD 17)


#5、三方库包含目录和库目录
# rockit api
set(ROCKIT_PATH ${CMAKE_SOURCE_DIR}/3rd/rockit)
set(ROCKIT_LIBS
    ${ROCKIT_PATH}/lib/librockit.so
    ${ROCKIT_PATH}/lib/libgraphic_lsf.so
    ${ROCKIT_PATH}/lib/librkgfx_avs.so
    ${ROCKIT_PATH}/lib/libpanoStitchApp.so
    ${ROCKIT_PATH}/lib/libRkSwCac.so                
)
include_directories(${ROCKIT_PATH}/include)

# opencv api
set(OpenCV_PATH ${CMAKE_SOURCE_DIR}/3rd/opencv/arm3588_opencv450_gcc940)
set(OpenCV_LIBS 
    ${OpenCV_PATH}/lib/libopencv_highgui.so.4.5
    ${OpenCV_PATH}/lib/libopencv_imgproc.so.4.5
    ${OpenCV_PATH}/lib/libopencv_core.so.4.5
    ${OpenCV_PATH}/lib/libopencv_imgcodecs.so.4.5
    ${OpenCV_PATH}/lib/libopencv_flann.so.4.5
    ${OpenCV_PATH}/lib/libopencv_features2d.so.4.5
    ${OpenCV_PATH}/lib/libopencv_calib3d.so.4.5
    ${OpenCV_PATH}/lib/libopencv_video.so.4.5
    ${OpenCV_PATH}/lib/libopencv_videoio.so.4.5
 )
include_directories(${OpenCV_PATH}/include)

# freetype api
set(FreeType_PATH ${CMAKE_SOURCE_DIR}/3rd/freetype)
set(FreeType_LIBS 
    ${FreeType_PATH}/lib/libfreetype.so
    ${FreeType_PATH}/lib/libfreetype.so.6
    ${FreeType_PATH}/lib/libfreetype.so.6.16.0
)
include_directories(${FreeType_PATH}/include)

# ffmpeg api
set(FFMPEG_PATH ${CMAKE_SOURCE_DIR}/3rd/ffmpeg)
set(FFMPEG_LIBS
    ${FFMPEG_PATH}/lib/libavcodec.so.58
    ${FFMPEG_PATH}/lib/libavformat.so.58
    ${FFMPEG_PATH}/lib/libavutil.so.56
    ${FFMPEG_PATH}/lib/libswresample.so.3
)
include_directories(${FFMPEG_PATH}/include)

# rtsp api
set(RTSP_PATH ${CMAKE_SOURCE_DIR}/3rd/rtsp)
set(RTSP_LIB ${RTSP_PATH}/lib/librtsp.a)
include_directories(${RTSP_PATH}/include)

#eigen3.4.0
set(EIGEN_PATH ${CMAKE_SOURCE_DIR}/3rd/eigen3.4.0/install-aarch64-linux-gnu9.4.0)
include_directories(${EIGEN_PATH}/include/eigen3)

#myalgo
set(MYALGO_PATH ${CMAKE_SOURCE_DIR}/algo)
set(MYALGO_LIBS
    ${MYALGO_PATH}/JZCoordTrans/lib/libJzcoordTrans.so
    ${MYALGO_PATH}/JZMOTCTracker/lib/libmot_tracker.so
    ${MYALGO_PATH}/JZRKInference/lib/librk3588_sdk_share.so
    ${MYALGO_PATH}/JZSingleObjTracker/lib/libsot_tracker.so
)
include_directories(
    ${MYALGO_PATH}/JZCoordTrans/include
    ${MYALGO_PATH}/JZMOTCTracker/include
    ${MYALGO_PATH}/JZRKInference/include
    ${MYALGO_PATH}/JZSingleObjTracker/include
)

# jsoncpp api 
set(JSONCPP_PATH ${CMAKE_SOURCE_DIR}/3rd/jsoncpp)
set(JSONCPP_LIBS 
    ${JSONCPP_PATH}/lib/libjsoncpp.so
    ${JSONCPP_PATH}/lib/libjsoncpp.so.1.9.5
    ${JSONCPP_PATH}/lib/libjsoncpp.so.25
)
include_directories(${JSONCPP_PATH}/include)

# sdl api
#set(SDL_PATH ${CMAKE_SOURCE_DIR}/3rd/sdl)
#set(SDL_LIBS
#    ${SDL_PATH}/lib/libSDL2.so
 #   ${SDL_PATH}/lib/libSDL2-2.0.so.0
# #   ${SDL_PATH}/lib/libSDL2-2.0.so.0.3000.5
#    ${SDL_PATH}/lib/libSDL2_ttf.so
#    ${SDL_PATH}/lib/libSDL2_ttf-2.0.so.0
#    ${SDL_PATH}/lib/libSDL2_ttf-2.0.so.0.2200.0
#)
#include_directories(${SDL_PATH}/include)

# rtsp api
set(RTSP_PATH ${CMAKE_SOURCE_DIR}/3rd/rtsp)
set(RTSP_LIB ${RTSP_PATH}/lib/librtsp.a)
include_directories(${RTSP_PATH}/include)



# glog
set(GLOGE_PATH ${CMAKE_SOURCE_DIR}/3rd/glog)
set(GLOGE_LIBS 
    ${GLOGE_PATH}/lib/libglog.so
    ${GLOGE_PATH}/lib/libglog.so.0.8.0
    ${GLOGE_PATH}/lib/libglog.so.3
)
include_directories(${GLOGE_PATH}/include)
add_definitions(-DGLOG_USE_GLOG_EXPORT)


#6、添加编译头文件和源文件
#添加编译头文件
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/comtest/include
    ${CMAKE_CURRENT_SOURCE_DIR}/media_process/include
    ${CMAKE_CURRENT_SOURCE_DIR}/dma_handle/include
    ${CMAKE_CURRENT_SOURCE_DIR}/usr_app/include
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rd/zkbfan/include
)

#指定编译源文件
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/3rd/zkbfan/src SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/comtest/src SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/media_process/src SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/dma_handle/src SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/usr_app/src SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/protocol/src SRC_LIST)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/utils/src SRC_LIST)
aux_source_directory(${CMAKE_SOURCE_DIR}/usr_app SRC_LIST)

#7、生成可执行文件或动态库
#动态库
#add_library(83002_XTKZ_A_lib SHARED ${SRC_LIST})
#target_link_libraries(83002_XTKZ_A_lib
#                opencv_highgui
#                opencv_imgproc
#                opencv_core
#                opencv_imgcodecs
#                opencv_flann
#                opencv_features2d
#                opencv_calib3d
#                opencv_video
# )
# install(TARGETS 83002_XTKZ_A_lib DESTINATION lib)
  
 #可执行文件
 add_executable(29003_XTKZ_A  ${SRC_LIST})
 target_link_libraries(29003_XTKZ_A
        ${OpenCV_LIBS}
        ${ROCKIT_LIBS}
        ${RTSP_LIB}
        ${FreeType_LIBS}
        ${FFMPEG_LIBS}
        ${MYALGO_LIBS}
        ${JSONCPP_LIBS}
        ${GLOGE_LIBS}
         #${SDL_LIBS}
        dl m pthread rt
 )

 # 8、程序安装
install(TARGETS 29003_XTKZ_A DESTINATION ./)
install(PROGRAMS ${ROCKIT_LIBS} DESTINATION lib)
install(PROGRAMS ${FFMPEG_LIBS} DESTINATION lib)
install(PROGRAMS ${RTSP_LIB} DESTINATION lib)
install(PROGRAMS ${FreeType_LIBS} DESTINATION lib)
install(PROGRAMS ${OpenCV_LIBS} DESTINATION lib)
install(PROGRAMS ${MYALGO_LIBS} DESTINATION lib)
install(PROGRAMS ${JSONCPP_LIBS} DESTINATION lib)
install(PROGRAMS ${SDL_LIBS} DESTINATION lib)
install(DIRECTORY  ${CMAKE_SOURCE_DIR}/resource DESTINATION ./)
install(DIRECTORY  ${CMAKE_SOURCE_DIR}/config DESTINATION ./)
install(PROGRAMS ${GLOGE_LIBS} DESTINATION lib)
