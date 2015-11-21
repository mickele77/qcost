include( ../config.pri )

TARGET = odtcreator
TEMPLATE = lib

contains(DEFINES, BUILD_STATIC) {
    CONFIG += staticlib
} else {
    CONFIG += shared
}

contains(DEFINES, BUILD_MSVC){
    INCLUDEPATH += $$ZLIB_INCLUDE
} else {
    LIBS += -lz
}

win32 {
    DEFINES += BUILD_ODTCREATOR_LIB
}

SOURCES += \
    zip.cpp \
    odtwriter.cpp

HEADERS  += \
    zipwriter.h \
    zipreader.h \
    odtwriter.h \
    qtextformatuserdefined.h \
    odtcreator_export.h
