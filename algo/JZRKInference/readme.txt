1、三方依赖：
（1）opencv450
（2）rknn_api

  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/opencv/arm3588_opencv450_gcc940/include)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/opencv/arm3588_opencv450_gcc940/lib)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/RK3588/Linux/librknn_api/aarch64)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/RK3588/Linux/librknn_api/include)
2、算法库目录
.
├── 3rd
│   └── arm3588_opencv450_gcc940
│   └── RK3588
├── include
│   └── rk3588classifier.h
│	└── rk3588_detector.h
│	└── common.h
│	└── base_detector.h
│
├── lib
│   └── librk3588_sdk_share.so
└── readme.txt
3、调用
   参看rk3588_test.cpp

4、功能性能
  （1）功能
	检测类别： 舰船、浮标、人员、车辆、建筑、坦克
  （2） 性能
     单帧耗时：29ms

