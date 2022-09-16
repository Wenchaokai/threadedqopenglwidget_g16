
TEMPLATE = app
TARGET = lit3d_sample_g16
DEPENDPATH += . v3d
INCLUDEPATH += .


#DEFINES += USE_Qt5  #this might be invoked from commandline like "~/Qt5.4.1/5.4/clang_64/bin/qmake DEFINES+=USE_Qt5 vaa3d64.pro", however it seems there is some bug


# commented the -app_bundle as on Mac the not-automatically closed terminal is quite annoying!
# macx: CONFIG-=app_bundle #by PHC, 101119

# cross-OS-platform, cross-Qt-version
#QT_DIR = $$[QT_INSTALL_PREFIX]
QT_DIR = $$(QTDIR)
LOCAL_DIR = ../common_lib/ 				# unix-liked platform: macx, unix, win32-msys-mingw

MINGW_DIR = /mingw # platform: win32-msys-mingw
win32 { # platform: win32-command-mingw
	MINGW_DIR = c:/mingw
	LOCAL_DIR = ../common_lib/      # c:/msys/local
	CONFIG = $$unique(CONFIG)
LIBS += -L$$MINGW_DIR/lib \
	-L$$LOCAL_DIR/lib_win32

}


INCLUDEPATH += $$LOCAL_DIR/include #./basic_c_fun
LIBS += -L$$LOCAL_DIR/lib

USE_Qt5 {
  INCLUDEPATH += $$QT_DIR/lib/QtConcurrent.framework/Versions/5/Headers  # for QtConcurrent, by PHC 2015May
  #SHARED_FOLDER = $$QT_DIR/demos/shared # for arthurwidgets
  SHARED_FOLDER = ./painting/shared/ # for arthurwidgets
  include($$SHARED_FOLDER/shared.pri)
  INCLUDEPATH += $$SHARED_FOLDER
  LIBS += -L$$SHARED_FOLDER
} else {
  SHARED_FOLDER = $$QT_DIR\\demos\\shared # for arthurwidgets
  include($$SHARED_FOLDER\\shared.pri)
  INCLUDEPATH += $$SHARED_FOLDER
  LIBS += -L$$SHARED_FOLDER
}



# the following trick was figured out by Ruan Zongcai
CONFIG += warn_off  # only work for complier
# need Qt 4.5.0 above and reCreate Makefile, this will be automatic.
CONFIG += thread

#CONFIG += console

# Input
HEADERS += ../basic_c_fun/mg_utilities.h \
    ../basic_c_fun/mg_image_lib.h \
    ../basic_c_fun/v3d_basicdatatype.h \
    ../basic_c_fun/basic_memory.h \
    ../basic_c_fun/stackutil.h \
    ../basic_c_fun/img_definition.h \
    ../basic_c_fun/volimg_proc_declare.h \
    ../basic_c_fun/volimg_proc.h \
    ../basic_c_fun/v3d_message.h \
    ../basic_c_fun/color_xyz.h \
    ../basic_c_fun/basic_4dimage.h \
    ../basic_c_fun/v3d_global_preference.h \
    ../basic_c_fun/customary_structs/v3d_imaging_para.h \
	../basic_c_fun/ome_tiff_info.h \
    ../litone/utils.h \
    ../3drenderer/mainwindow.h \
    ../3drenderer/glwidget.h \
    ../3drenderer/renderer.h \
	../3drenderer/helper.h \
	../3drenderer/lit3dr_common.h \
	../3drenderer/glsl_r.h \
  ../3drenderer/base_model.h \
  ../3drenderer/core_model.h \
  ../3drenderer/backfill_model.h \
    my4dimage.h \
    colormap.h


SOURCES += ../basic_c_fun/mg_utilities.cpp \
    ../basic_c_fun/mg_image_lib.cpp \
    ../basic_c_fun/stackutil.cpp \
    ../basic_c_fun/basic_memory.cpp \
    ../basic_c_fun/v3d_message.cpp \
    ../basic_c_fun/basic_4dimage.cpp \
    ../basic_c_fun/basic_4dimage_create.cpp \
	../basic_c_fun/ome_tiff_info.cpp \
    ../litone/utils.cpp \
    ../3drenderer/mainwindow.cpp \
    ../3drenderer/glwidget.cpp \ 
    ../3drenderer/renderer.cpp \ 
	../3drenderer/helper.cpp \
	../3drenderer/glsl_r.cpp \
  ../3drenderer/base_model.cpp \
  ../3drenderer/core_model.cpp \
  ../3drenderer/backfill_model.cpp \
    my4dimage.cpp \
    colormap.cpp \ 
    main.cpp 

  
    


#FORMS += landmark_property.ui \
   

#RESOURCES += v3d.qrc
RESOURCES += ../3drenderer/3drenderer.qrc  ../3drenderer/textures.qrc
QT += opengl
QT += network
QT += xml svg
QT += widgets


# NOT REALLY USED, LIBS are replaced by vaa3d_msvc.pro
win32:LIBS += -lm -lv3dtiff \
    -lv3dnewmat
win32:LIBS += -L../common_lib/winlib64 -lteem  -lbz2 -lz  #for nrrd support


INCLUDEPATH += ../common_lib/include \
                ../basic_c_fun \
                ../litone  \


DEFINES *= TEEM_STATIC
QMAKE_CXXFLAGS += -DTEEM_STATIC

INCLUDEPATH = $$unique(INCLUDEPATH)
message(CONFIG=$$CONFIG)
