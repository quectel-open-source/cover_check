QT += core gui sql multimediawidgets multimedia network serialport # serialbus   # xlsx
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat

QMAKE_CC += -g

QMAKE_CXX += -g

QMAKE_LINK += -g

INCLUDEPATH += $$PWD/date
INCLUDEPATH += $$PWD/curl
INCLUDEPATH += $$PWD/libs/include
INCLUDEPATH += $$PWD/MVS/include
INCLUDEPATH += $$PWD/MVinclude/include
INCLUDEPATH += $$PWD/jpeg/include

RC_ICONS = myico.ico

DESTDIR     = $$PWD/bin
#自动拷贝文件
src_file = $$PWD/TA.db  $$PWD/icon
dst_file = $$DESTDIR
win32 {
#转换斜杠 / 到 \\
src_file ~= s,/,\\,g
dst_file ~= s,/,\\,g
dst_file2 ~= s,/,\\,g
QMAKE_POST_LINK += copy $$src_file $$dst_file
}
unix {
QMAKE_POST_LINK += cp -r -f $$src_file $$dst_file
}

#-lSNPE -lyolov5_ring # #-lMVSDK
  # \

LIBS += -L$$PWD/libs -L/usr/lib/ -L/usr/local/cuda-11.4/targets/x86_64-linux/lib/ -L/home/q/source/TensorRT-8.5.3.1/lib/ \
         -ljpeg -lcurl -lgxiapi -lMvCameraControl -lMVSDK -ljsoncpp \
         -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_dnn -ltrt -lcudart -lnvinfer -lcublas -lcublasLt

TARGET      = cover_check
TEMPLATE    = app

CONFIG *= force_debug_info
CONFIG += exception

QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -O2

HEADERS     += head.h \
    GXCamera.h \
    IMVCamera.h \
    MVinclude/include/IMVApi.h \
    MVinclude/include/IMVDefines.h \
    MVinclude/include/VideoRender.h \
    Quectel_Infer.h \
    frmvideopanel.h \
    jpeg/include/jconfig.h \
    jpeg/include/jerror.h \
    jpeg/include/jmorecfg.h \
    jpeg/include/jpeglib.h \
    jpeg/include/turbojpeg.h \
    videobox.h \
    videopanel.h \
    dbpage.h \
    frmdbpage.h \
    settingsview.h \
    ftpview.h \
    moduleview.h \
    outview.h \
    transferview.h \
    GetFrame.h \
    gxview.h \
    switchbutton.h \
    date/date_time.h \
    date/date_time_edit.h \
    date/date_time_range_widget.h \
    Util.hpp \
    json/allocator.h \
    json/assertions.h \
    json/config.h \
    json/forwards.h \
    json/json.h \
    json/json_features.h \
    json/reader.h \
    json/value.h \
    json/value.h \
    json/version.h \
    UartControl.hpp \
    ifaddrs.h \
    packetParser.hpp \
    SettingConfig.h \
    ServerConnect.h \
    socketMain.h \
    DaHengCamera/DxImageProc.h \
    DaHengCamera/GxIAPI.h \
    FtpUtil.h \
    curl/curl.h \
    curl/curlver.h \
    curl/easy.h \
    curl/mprintf.h \
    curl/multi.h \
    curl/stdcheaders.h \
    curl/system.h \
    curl/typecheck-gcc.h \
    curl/urlapi.h \
    Serverplatform.h \
    ModbusTcpClient.h \
    modbus.h \
    MVS/include/MvCameraControl.h \
    MvCamera.h\
    form/common.h \
    AlgoThread.h \
    MemThread.h \
    LightThread.h \
    runview.h \
    algoview.h \
    quickview.h \
    QuickSetThread.h
SOURCES     += \
    GXCamera.cpp \
    IMVCamera.cpp \
    frmvideopanel.cpp \
    main.cpp \
    modbus.cpp \
    videobox.cpp \
    videopanel.cpp \
    dbpage.cpp \
    frmdbpage.cpp \
    settingsview.cpp \
    ftpview.cpp \
    moduleview.cpp \
    outview.cpp \
    transferview.cpp \
    GetFrame.cpp \
    borderlayout.cpp \
    gxview.cpp \
    SwitchButton.cpp \
    date/date_time.cpp \
    date/date_time_edit.cpp \
    date/date_time_range_widget.cpp \
    Util.cpp \
    UartControl.cpp \
    packetParser.cpp \
    SettingConfig.cpp \
    ServerConnect.cpp \
    FtpUtil.cpp \
    Serverplatform.cpp \
    ModbusTcpClient.cpp \
    MvCamera.cpp\
    form/common.cpp \
    AlgoThread.cpp \
    MemThread.cpp \
    LightThread.cpp \
    runview.cpp \
    algoview.cpp \
    quickview.cpp \
    QuickSetThread.cpp
RESOURCES   += other/main.qrc \
    uimain.qrc
RESOURCES   += $$PWD/core_qss/qss.qrc

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/form
include ($$PWD/form/form.pri)

INCLUDEPATH += $$PWD/core_base
include ($$PWD/core_base/core_base.pri)

DISTFILES += \
    TA.db \
    icon/m1.png \
    icon/u8.png \
    icon/u17.png \
    icon/u20.png \
    icon/u21.png \
    icon/u22.png \
    icon/u29.png \
    icon/u30.png \
    icon/u31.png \
    icon/u32.png \
    icon/u38.png \
    icon/u41.png \
    icon/u42.png \
    icon/u361.png \
    icon/u380.png \
    icon/u415.png \
    icon/u502.png \
    icon/u503.png \
    curl/Makefile.am
