include( ../config.pri )

TARGET = qcost

TEMPLATE = lib

contains(DEFINES, BUILD_STATIC) {
  CONFIG += staticlib
}

win32 {
    DEFINES += BUILD_LIB
}

LIBS += \
    -L../bin \
    -lodtcreator \
    -lmathparser

INCLUDEPATH += \
    ../libmathparser \
    ../libodtcreator

DEPENDPATH += \
    ../libmatparser \
    ../libqcost

TRANSLATIONS = \
    libqcost_en.ts

SOURCES += \
    projectbillparentitem.cpp \
    projectdataparentitem.cpp \
    projectrootitem.cpp \
    projectitem.cpp \
    projectpricelistparentitem.cpp \
    pricelist.cpp \
    bill.cpp \
    project.cpp \
    unitmeasure.cpp \
    billitem.cpp \
    priceitem.cpp \
    billitemmeasuresmodel.cpp \
    billitemmeasure.cpp \
    billattributemodel.cpp \
    billattribute.cpp \
    unitmeasuremodel.cpp \
    pricefieldmodel.cpp \
    pricelistprinter.cpp \
    billprinter.cpp \
    priceitemdatasetmodel.cpp

HEADERS  += \
    library_common.h \
    projectpricelistparentitem.h \
    projectitem.h \
    projectbillparentitem.h \
    projectdataparentitem.h \
    projectrootitem.h \
    pricelist.h \
    bill.h \
    project.h \
    unitmeasure.h \
    treeitem.h \
    billitem.h \
    priceitem.h \
    billitemmeasuresmodel.h \
    billitemmeasure.h \
    billattributemodel.h \
    billattribute.h \
    unitmeasuremodel.h \
    pricefieldmodel.h \
    pricelistprinter.h \
    billprinter.h \
    priceitemdatasetmodel.h
