include( ../config.pri )

TARGET = mathparser

TEMPLATE = lib

contains(DEFINES, BUILD_STATIC) {
    CONFIG += staticlib
} else {
    CONFIG += shared
}

win32 {
    DEFINES += BUILD_MATHPARSER_LIB
}

SOURCES += \
    mathparser.cpp

HEADERS  += \
    mathparser.h \
    mathparser_export.h
