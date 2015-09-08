include( ../config.pri )

TARGET = odtcreator
TEMPLATE = lib

contains(DEFINES, BUILD_STATIC) {
  CONFIG += staticlib
}

contains(DEFINES, BUILD_MSVC){
    LIBS += -L"c:/zlib" -lz64
    INCLUDEPATH += "c:/zlib"
} else {
    LIBS += -lz
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
