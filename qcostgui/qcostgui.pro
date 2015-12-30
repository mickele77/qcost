include( ../config.pri )

QT       += widgets sql

TARGET = QCostGUI

TEMPLATE = app

contains(DEFINES, BUILD_RELEASE) {
    target.path = /usr/bin
    INSTALLS += target
}

contains(DEFINES, BUILD_MSVC){
    LIBS += \
        ../bin/qcost.lib \
        ../bin/odtcreator.lib \
        $$ZLIB_STATIC_LIB \
        ../bin/mathparser.lib
    INCLUDEPATH += $$ZLIB_INCLUDE
} else {
    LIBS += \
        -L../bin \
        -lqcost \
        -lodtcreator \
        -lmathparser \
        -lz
}

DEPENDPATH += \
    ../libmathparser \
    ../libqcost

INCLUDEPATH += \
    ../libmathparser \
    ../libqcost

RESOURCES += \
    ../icons.qrc

TRANSLATIONS = \
    qcostgui_en.ts

SOURCES += \
    main.cpp\
    projectitemsview.cpp \
    pricelisttreegui.cpp \
    pricelistdelegate.cpp \
    generaldatagui.cpp \
    pricelistgui.cpp \
    priceitemgui.cpp \
    billgui.cpp \
    pricelistdatagui.cpp \
    billdatagui.cpp \
    billtreegui.cpp \
    billitemgui.cpp \
    billsetpricelistmodegui.cpp \
    editpriceitemapdialog.cpp \
    editpriceitemdialog.cpp \
    pricedatatabledelegate.cpp \
    billitemtitlegui.cpp \
    pricelistdbmodel.cpp \
    pricelistdbwidget.cpp \
    importpriceitemdbdialog.cpp \
    qcostgui.cpp \
    pricelistdbviewer.cpp \
    billattributeselectmodel.cpp \
    qcostclipboarddata.cpp \
    pricefieldtabledelegate.cpp \
    billprintergui.cpp \
    pricelistprintergui.cpp \
    billattributeprintergui.cpp \
    priceitemdatasetviewmodel.cpp \
    settingsdialog.cpp \
    importbillitemmeasurestxt.cpp \
    accountingtreegui.cpp \
    accountingsetpricelistmodegui.cpp \
    accountingitemattributemodel.cpp \
    accountingattributeprintergui.cpp \
    qcalendardialog.cpp \
    accountinggui.cpp \
    attributechangedialog.cpp \
    billitemattributemodel.cpp \
    attributeselectmodel.cpp \
    accountingtambillgui.cpp \
    accountingtambilldatagui.cpp \
    accountingitemcommentgui.cpp \
    accountingitemppugui.cpp \
    accountingbillsetpricelistmodegui.cpp \
    accountingbillgui.cpp \
    accountingbilldatagui.cpp \
    accountingitemlsgui.cpp \
    accountingitemtamgui.cpp \
    accountinglsbilldatagui.cpp \
    accountinglsbillgui.cpp \
    accountinglsbillitemgui.cpp \
    accountinglstreegui.cpp \
    accountinglsbillitemtitlegui.cpp \
    importlsitemmeasurestxt.cpp \
    accountingitempaymentgui.cpp \
    accountingbillprintergui.cpp \
    accountingtambillprintergui.cpp \
    accountinglsbillprintergui.cpp \
    attributesgui.cpp \
    varsgui.cpp \
    accountingbillpricegui.cpp

HEADERS  += \
    projectitemsview.h \
    pricelisttreegui.h \
    pricelistdelegate.h \
    generaldatagui.h \
    pricelistgui.h \
    priceitemgui.h \
    billgui.h \
    pricelistdatagui.h \
    billdatagui.h \
    billtreegui.h \
    billitemgui.h \
    billsetpricelistmodegui.h \
    editpriceitemapdialog.h \
    editpriceitemdialog.h \
    pricedatatabledelegate.h \
    billitemtitlegui.h \
    pricelistdbmodel.h \
    pricelistdbwidget.h \
    importpriceitemdbdialog.h \
    qcostgui.h \
    pricelistdbviewer.h \
    billitemattributemodel.h \
    billattributeselectmodel.h \
    qcostclipboarddata.h \
    pricefieldtabledelegate.h \
    billprintergui.h \
    pricelistprintergui.h \
    billattributeprintergui.h \
    priceitemdatasetviewmodel.h \
    settingsdialog.h \
    importbillitemmeasurestxt.h \
    accountingtreegui.h \
    accountingsetpricelistmodegui.h \
    accountingitemattributemodel.h \
    accountingattributeprintergui.h \
    qcalendardialog.h \
    accountinggui.h \
    attributechangedialog.h \
    attributeselectmodel.h \
    accountingtambillgui.h \
    accountingtambilldatagui.h \
    accountingitemcommentgui.h \
    accountingitemppugui.h \
    accountingbillsetpricelistmodegui.h \
    accountingbillgui.h \
    accountingbilldatagui.h \
    accountingitemlsgui.h \
    accountingitemtamgui.h \
    accountinglsbilldatagui.h \
    accountinglsbillgui.h \
    accountinglsbillitemgui.h \
    accountinglstreegui.h \
    accountinglsbillitemtitlegui.h \
    importlsitemmeasurestxt.h \
    accountingitempaymentgui.h \
    accountingbillprintergui.h \
    accountingtambillprintergui.h \
    accountinglsbillprintergui.h \
    attributesgui.h \
    varsgui.h \
    accountingbillpricegui.h

FORMS += \
    generaldatagui.ui \
    projectitemsview.ui \
    pricelisttreegui.ui \
    priceitemgui.ui \
    billdatagui.ui \
    pricelistdatagui.ui \
    billtreegui.ui \
    billitemgui.ui \
    billitemtitlegui.ui \
    pricelistdbwidget.ui \
    billprintergui.ui \
    pricelistprintergui.ui \
    billattributeprintergui.ui \
    settingsdialog.ui \
    importbillitemmeasurestxt.ui \
    accountingtreegui.ui \
    accountingattributeprintergui.ui \
    setpricelistmodegui.ui \
    attributechangedialog.ui \
    qcalendardialog.ui \
    accountinggui.ui \
    accountingtambilldatagui.ui \
    accountingitemcommentgui.ui \
    accountingitemppugui.ui \
    accountingbilldatagui.ui \
    accountingitemlsgui.ui \
    accountingitemtamgui.ui \
    accountinglsbilldatagui.ui \
    accountinglsbillitemgui.ui \
    accountinglstreegui.ui \
    accountinglsbillitemtitlegui.ui \
    importlsitemmeasurestxt.ui \
    accountingitempaymentgui.ui \
    accountingbillprintergui.ui \
    accountingtambillprintergui.ui \
    accountinglsbillprintergui.ui \
    attributesgui.ui \
    varsgui.ui \
    accountingbillpricegui.ui

OTHER_FILES += \
    ../LICENSE \
    ../LICENSE.GPLv3 \
    ../README
