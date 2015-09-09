include( ../config.pri )

TARGET = odtcreator
TEMPLATE = lib

contains(DEFINES, BUILD_STATIC) {
    CONFIG += staticlib
} else {
    CONFIG += shared
}

contains(DEFINES, BUILD_MSVC){
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
