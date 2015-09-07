include( ../config.pri )

TARGET = odtcreator
TEMPLATE = lib

contains(DEFINES, BUILD_STATIC) {
  CONFIG += staticlib
}

win32 {
    LIBS += -L"c:/zlib" -lz64
    INCLUDEPATH += "c:/zlib"
}

linux {
    LIBS += -lz
}

SOURCES += \
    zip.cpp \
    odtwriter.cpp

HEADERS  += \
    zipwriter.h \
    zipreader.h \
    odtwriter.h \
    qtextformatuserdefined.h
