contains(QT_CONFIG, opengl): QT += opengl
QT += 
INCLUDEPATH += /c/PFM_ABEv7.0.0_Win64/include
LIBS += -L /c/PFM_ABEv7.0.0_Win64/lib -lgsf -lpfm -lCZMIL -lCHARTS -lllz -lnvutility -llas -lgdal -lxml2 -lpoppler -liconv -lwsock32 -lm
DEFINES += WIN32 NVWIN3X UINT32_C INT32_C
CONFIG += console
CONFIG += exceptions
QMAKE_CXXFLAGS += -fno-strict-aliasing
QMAKE_LFLAGS += 
######################################################################
# Automatically generated by qmake (2.01a) Wed Jan 22 14:54:34 2020
######################################################################

TEMPLATE = app
TARGET = pfm_unload
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += pfm_unload.hpp slas.hpp version.hpp
SOURCES += gsf_flags.cpp \
           pfm_unload.cpp \
           slas.cpp \
           unload_file.cpp \
           write_history.cpp
TRANSLATIONS += pfm_unload_xx.ts