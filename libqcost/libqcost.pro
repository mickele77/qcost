include( ../config.pri )

TARGET = qcost

TEMPLATE = lib

contains(DEFINES, BUILD_STATIC) {
    CONFIG += staticlib
} else {
    CONFIG += shared
}

win32 {
    DEFINES += BUILD_QCOST_LIB
}

contains(DEFINES, BUILD_MSVC){
    INCLUDEPATH += $$ZLIB_INCLUDE
} else {
    LIBS += \
        -L../bin \
        -lodtcreator \
        -lmathparser \
        -lz
}

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
    unitmeasuremodel.cpp \
    var.cpp \
    varsmodel.cpp \
    billitem.cpp \
    priceitem.cpp \
    pricefieldmodel.cpp \
    pricelistprinter.cpp \
    billprinter.cpp \
    priceitemdatasetmodel.cpp \
    projectaccountingparentitem.cpp \
    accountingprinter.cpp \
    attribute.cpp \
    accountingpricefieldmodel.cpp \
    accountingbill.cpp \
    accountingbillitem.cpp \
    accountingbillitemprivate.cpp \
    accountinglsbills.cpp \
    accountingtambill.cpp \
    accountingtambillitem.cpp \
    accountingtammeasure.cpp \
    accountingtammeasuresmodel.cpp \
    measuresmodel.cpp \
    accountinglsbill.cpp \
    accountinglsbillitem.cpp \
    accountinglsmeasure.cpp \
    accountinglsmeasuresmodel.cpp \
    paymentdata.cpp \
    paymentdatamodel.cpp \
    measure.cpp \
    attributesmodel.cpp

HEADERS  += \
    projectpricelistparentitem.h \
    projectitem.h \
    projectbillparentitem.h \
    projectdataparentitem.h \
    projectrootitem.h \
    pricelist.h \
    bill.h \
    project.h \
    unitmeasure.h \
    unitmeasuremodel.h \
    var.h \
    varsmodel.h \
    treeitem.h \
    billitem.h \
    priceitem.h \
    pricefieldmodel.h \
    pricelistprinter.h \
    billprinter.h \
    priceitemdatasetmodel.h \
    projectaccountingparentitem.h \
    accountingprinter.h \
    attribute.h \
    accountingpricefieldmodel.h \
    accountingbill.h \
    accountingbillitem.h \
    accountingbillitemprivate.h \
    accountinglsbills.h \
    accountingtambill.h \
    accountingtambillitem.h \
    accountingtammeasure.h \
    accountingtammeasuresmodel.h \
    measuresmodel.h \
    accountinglsbill.h \
    accountinglsbillitem.h \
    accountinglsmeasure.h \
    accountinglsmeasuresmodel.h \
    paymentdata.h \
    qcost_export.h \
    paymentdatamodel.h \
    measure.h \
    attributesmodel.h
