1、三方依赖：
（1）opencv450
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/opencv/arm3588_opencv450_gcc940/include)
  link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/opencv/arm3588_opencv450_gcc940/lib)
2、算法库目录
.
├── 3rd
│   └── arm3588_opencv450_gcc940
│       ├── include
│       └── lib
├── include
│   └── JZSoTracker.h
├── lib
│   └── libsdk_tracker.so
└── readme.txt
3、调用
  参看democ.cpp