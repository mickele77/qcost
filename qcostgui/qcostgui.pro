include( ../config.pri )

QT       += widgets sql

TARGET = QCost

contains(DEFINES, BUILD_RELEASE) {
    target.path = /usr/bin
    INSTALLS += target
}

TEMPLATE = app

LIBS += \
    -L../bin \
    -lqcost \
    -lodtcreator \
    -lmathparser \
    -lz

DEPENDPATH += \
    ../libmatparser \
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
    billattributechangedialog.cpp \
    billitemattributemodel.cpp \
    billattributeselectmodel.cpp \
    qcostclipboarddata.cpp \
    pricefieldtabledelegate.cpp \
    billprintergui.cpp \
    pricelistprintergui.cpp \
    billattributeprintergui.cpp \
    priceitemdatasetviewmodel.cpp \
    settingsdialog.cpp \
    importbillitemmeasurestxt.cpp

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
    billattributechangedialog.h \
    billitemattributemodel.h \
    billattributeselectmodel.h \
    qcostclipboarddata.h \
    pricefieldtabledelegate.h \
    billprintergui.h \
    pricelistprintergui.h \
    billattributeprintergui.h \
    priceitemdatasetviewmodel.h \
    settingsdialog.h \
    importbillitemmeasurestxt.h

FORMS += \
    generaldatagui.ui \
    projectitemsview.ui \
    pricelisttreegui.ui \
    priceitemgui.ui \
    billdatagui.ui \
    pricelistdatagui.ui \
    billtreegui.ui \
    billitemgui.ui \
    billsetpricelistmodegui.ui \
    billitemtitlegui.ui \
    pricelistdbwidget.ui \
    billattributechangedialog.ui \
    billprintergui.ui \
    pricelistprintergui.ui \
    billattributeprintergui.ui \
    settingsdialog.ui \
    importbillitemmeasurestxt.ui

OTHER_FILES += \
    ../LICENSE \
    ../LICENSE.GPLv3
