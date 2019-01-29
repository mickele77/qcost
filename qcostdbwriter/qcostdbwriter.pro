QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QCostDB
TEMPLATE = app

RESOURCES += \
    ../qcostgui/icons.qrc

SOURCES += main.cpp\
    loadfromtxtdialog.cpp \
    pricelistdbmodel.cpp \
    pricelistdbwidget.cpp \
    unitmeasuremodel.cpp \
    pricelistdbdelegate.cpp \
    qcostdbwritergui.cpp

HEADERS  += \
    loadfromtxtdialog.h \
    pricelistdbmodel.h \
    pricelistdbwidget.h \
    unitmeasuremodel.h \
    pricelistdbdelegate.h \
    qcostdbwritergui.h

FORMS    += \
    loadfromtxtdialog.ui \
    pricelistdbwidget.ui
