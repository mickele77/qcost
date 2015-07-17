include( ../config.pri )

TARGET = mathparser

TEMPLATE = lib

contains(DEFINES, BUILD_STATIC) {
  CONFIG += staticlib
}

SOURCES += \
    mathparser.cpp

HEADERS  += \
    mathparser.h
