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
    billitem.cpp \
    priceitem.cpp \
    billitemmeasure.cpp \
    unitmeasuremodel.cpp \
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
    attributemodel.cpp \
    measuresmodel.cpp \
    accountinglsbill.cpp \
    accountinglsbillitem.cpp \
    accountinglsitemmeasure.cpp \
    measureslsmodel.cpp \
    paymentdata.cpp \
    paymentdatamodel.cpp

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
    treeitem.h \
    billitem.h \
    priceitem.h \
    billitemmeasure.h \
    unitmeasuremodel.h \
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
    attributemodel.h \
    measuresmodel.h \
    accountinglsbill.h \
    accountinglsbillitem.h \
    accountinglsitemmeasure.h \
    measureslsmodel.h \
    paymentdata.h \
    qcost_export.h \
    paymentdatamodel.h
