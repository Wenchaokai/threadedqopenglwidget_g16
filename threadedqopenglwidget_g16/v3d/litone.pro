# #####################################################################
# Created: 2020-12-29 
# produced by Jian Lu, 2020
# ######################################################################


message(CONFIG=$$unique(CONFIG))

CONFIG += warn_on  # should turn off later to view all warning during compilation
#CONFIG += CONSOLE   # make a console application instead of a windows GUI only application

include(litone_essential.pro)

win32 { 
    ## Windows common build here 
	
	HEADERS += ../basic_c_fun/v3d_basicdatatype.h \ 
				../basic_c_fun/vcdiff.h
				
	SOURCES += ../basic_c_fun/vcdiff.cpp
 
    !contains(QMAKE_HOST.arch, x86_64) { 
        message("x86 build") 
 
        ## Windows x86 (32bit) specific build here 
        
        MSVC_DIR = "C:\\Program Files\\Microsoft Visual Studio 9.0\\VC"
		MSSDK_DIR = "C:\\Program Files\\Microsoft SDKs\\Windows\\v6.0A\\"
		LOCAL_DIR = ..\ 

		LIBS += -L$$MSVC_DIR\\lib
		LIBS += -L$$MSSDK_DIR\\Lib
		LIBS += -L$$LOCAL_DIR\\common_lib\\winlib
		
		LIBS += \ #-lm
				-llibtiff \
				-ltiff \
                -lteem \
				-llibnewmat \ #libnewmat.lib also works, 2010-05-21. PHC
				-llibjba \    #libjba.lib also works
				-llibFL_cellseg \
				-llibFL_brainseg 
        DEFINES *= TEEM_STATIC
        QMAKE_CXXFLAGS += -DTEEMSTATIC
        QMAKE_CXXFLAGS += /MP
	
		INCLUDEPATH += $$LOCAL_DIR\\basic_c_fun\\include \
		               $$LOCAL_DIR\\common_lib\\include
 
    } else { 
        message("x86_64 build") 
 
        ## Windows x64 (64bit) specific build here 
 	
 		MSVC_DIR = "C:\\Program Files\\Microsoft Visual Studio 9.0\\VC"
		MSSDK_DIR = "C:\\Program Files\\Microsoft SDKs\\Windows\\v6.0A\\"
		LOCAL_DIR = ..\ 

		LIBS = -L$$SHARED_FOLDER
		Release:LIBS += -L$$SHARED_FOLDER\\release
		Debug:LIBS += -L$$SHARED_FOLDER\\debug
		
		LIBS += -L$$MSVC_DIR\\lib	
		LIBS += -L$$MSSDK_DIR\\Lib\\x64	

		MSVCVERSION = $$(QMAKESPEC)
		BOOSTPATH = $$(BOOST_PATH)
		OPENCVHOME = $$(OPENCV_HOME)
		#CAMERA_INCLUDE = $$(CAMERA_INCLUDE)
		#MMCORE_INCLUDE = $$(MMCORE_INCLUDE)
		LIBS += -L$$BOOSTPATH\\stage\\x64\\lib
				
		Release:LIBS += -L$$LOCAL_DIR\\common_lib\\winlib64\\release
		Debug:LIBS += -L$$LOCAL_DIR\\common_lib\\winlib64\\debug

		LIBS += -L$$OPENCVHOME\\build\\x64\\vc14\\lib

		Debug:LIBS += opencv_world340d.lib libfftw3f-3.lib OpenGL32.lib glu32.lib

		Release:LIBS += opencv_world340.lib libfftw3f-3.lib OpenGL32.lib glu32.lib 
		
		LIBS += \
				-llibFL_cellseg \
                                -llibFL_brainseg \
                                -lopenvr_api \
                            #    -lSDL2 \
                            #    -lSDL2main \
                                -llibszip \
                                -llibzlib 

		Debug:LIBS += \
		              -llibtiffd \
					  -lteemd \
					  -llibnewmatd \
					  -llibjbad \ 
					  -lglew32d \
					  -llibhdf5d \
					  #-lMMCore \
					  #-l$$SHARED_FOLDER\\debug\demo_shared 
					  
					  
		Release:LIBS += \
		              -llibtiff \
					  -lteem \
					  -llibnewmat \
					  -llibjba \ 
					  -lglew32 \
					  -llibhdf5 \
					  #-lMMCore \
                      #-l$$SHARED_FOLDER\\release\demo_shared
                      			  
		
        DEFINES *= TEEM_STATIC
        QMAKE_CXXFLAGS += -DTEEMSTATIC

        QMAKE_CXXFLAGS += /MP
	
		INCLUDEPATH += $$LOCAL_DIR\\basic_c_fun\\include \
		               $$LOCAL_DIR\\common_lib\\include \
					   $$LOCAL_DIR\\common_lib\\include\\glew \
		               $$CAMERA_INCLUDE \
					   $$MMCORE_INCLUDE \
					   $$BOOSTPATH \
					   $$OPENCVHOME\\build\\include \
					   $$OPENCVHOME\\build\\include\\opencv \
					   $$OPENCVHOME\\build\\include\\opencv2 \
					   $$LOCAL_DIR\\common_lib\\include\\libtiff
    } 
    
    INCLUDEPATH = $$unique(INCLUDEPATH)
    LIBS = $$unique(LIBS)

}