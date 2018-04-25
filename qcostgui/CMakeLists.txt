cmake_minimum_required(VERSION 2.8.11)

# Find the required modules
find_package(Qt5 COMPONENTS Widgets Sql REQUIRED)

set( QRC
    icons.qrc
)

set( SRC
    main.cpp
    projectitemsview.cpp
    pricelisttreegui.cpp
    pricelistdelegate.cpp
    generaldatagui.cpp
    pricelistgui.cpp
    priceitemgui.cpp
    billgui.cpp
    pricelistdatagui.cpp
    billdatagui.cpp
    billtreegui.cpp
    billitemgui.cpp
    billsetpricelistmodegui.cpp
    editpriceitemapdialog.cpp
    editpriceitemdialog.cpp
    pricedatatabledelegate.cpp
    billitemtitlegui.cpp
    pricelistdbmodel.cpp
    pricelistdbwidget.cpp
    importpriceitemdbdialog.cpp
    qcostgui.cpp
    pricelistdbviewer.cpp
    billattributeselectmodel.cpp
    qcostclipboarddata.cpp
    pricefieldtabledelegate.cpp
    billprintergui.cpp
    pricelistprintergui.cpp
    billattributeprintergui.cpp
    priceitemdatasetviewmodel.cpp
    settingsdialog.cpp
    importbillitemmeasurestxt.cpp
    accountingtreegui.cpp
    accountingsetpricelistmodegui.cpp
    accountingitemattributemodel.cpp
    accountingattributeprintergui.cpp
    qcalendardialog.cpp
    accountinggui.cpp
    attributechangedialog.cpp
    billitemattributemodel.cpp
    attributeselectmodel.cpp
    accountingtambillgui.cpp
    accountingtambilldatagui.cpp
    accountingitemcommentgui.cpp
    accountingitemppugui.cpp
    accountingbillsetpricelistmodegui.cpp
    accountingbillgui.cpp
    accountingbilldatagui.cpp
    accountingitemlsgui.cpp
    accountingitemtamgui.cpp
    accountinglsbilldatagui.cpp
    accountinglsbillgui.cpp
    accountinglsbillitemgui.cpp
    accountinglstreegui.cpp
    accountinglsbillitemtitlegui.cpp
    importlsitemmeasurestxt.cpp
    accountingitempaymentgui.cpp
    accountingbillprintergui.cpp
    accountingtambillprintergui.cpp
    accountinglsbillprintergui.cpp
    attributesgui.cpp
    varsgui.cpp
    accountingbillpricedatagui.cpp
    accountingtambillpricedatagui.cpp
    accountinglsbillsgui.cpp
    accountinglsbillsdatagui.cpp
    accountinglsbillpricedatagui.cpp
)

set( HDR
    projectitemsview.h
    pricelisttreegui.h
    pricelistdelegate.h
    generaldatagui.h
    pricelistgui.h
    priceitemgui.h
    billgui.h
    pricelistdatagui.h
    billdatagui.h
    billtreegui.h
    billitemgui.h
    billsetpricelistmodegui.h
    editpriceitemapdialog.h
    editpriceitemdialog.h
    pricedatatabledelegate.h
    billitemtitlegui.h
    pricelistdbmodel.h
    pricelistdbwidget.h
    importpriceitemdbdialog.h
    qcostgui.h
    pricelistdbviewer.h
    billattributeselectmodel.h
    qcostclipboarddata.h
    pricefieldtabledelegate.h
    billprintergui.h
    pricelistprintergui.h
    billattributeprintergui.h
    priceitemdatasetviewmodel.h
    settingsdialog.h
    importbillitemmeasurestxt.h
    accountingtreegui.h
    accountingsetpricelistmodegui.h
    accountingitemattributemodel.h
    accountingattributeprintergui.h
    qcalendardialog.h
    accountinggui.h
    attributechangedialog.h
    billitemattributemodel.h
    attributeselectmodel.h
    accountingtambillgui.h
    accountingtambilldatagui.h
    accountingitemcommentgui.h
    accountingitemppugui.h
    accountingbillsetpricelistmodegui.h
    accountingbillgui.h
    accountingbilldatagui.h
    accountingitemlsgui.h
    accountingitemtamgui.h
    accountinglsbilldatagui.h
    accountinglsbillgui.h
    accountinglsbillitemgui.h
    accountinglstreegui.h
    accountinglsbillitemtitlegui.h
    importlsitemmeasurestxt.h
    accountingitempaymentgui.h
    accountingbillprintergui.h
    accountingtambillprintergui.h
    accountinglsbillprintergui.h
    attributesgui.h
    varsgui.h
    accountingbillpricedatagui.h
    accountingtambillpricedatagui.h
    accountinglsbillsgui.h
    accountinglsbillsdatagui.h
    accountinglsbillpricedatagui.h
)

set ( FRM
    generaldatagui.ui
    projectitemsview.ui
    pricelisttreegui.ui
    priceitemgui.ui
    billdatagui.ui
    pricelistdatagui.ui
    billtreegui.ui
    billitemgui.ui
    billitemtitlegui.ui
    pricelistdbwidget.ui
    billprintergui.ui
    pricelistprintergui.ui
    billattributeprintergui.ui
    settingsdialog.ui
    importbillitemmeasurestxt.ui
    accountingtreegui.ui
    accountingattributeprintergui.ui
    setpricelistmodegui.ui
    attributechangedialog.ui
    qcalendardialog.ui
    accountinggui.ui
    accountingtambilldatagui.ui
    accountingitemcommentgui.ui
    accountingitemppugui.ui
    accountingbilldatagui.ui
    accountingitemlsgui.ui
    accountingitemtamgui.ui
    accountinglsbilldatagui.ui
    accountinglsbillitemgui.ui
    accountinglstreegui.ui
    accountinglsbillitemtitlegui.ui
    importlsitemmeasurestxt.ui
    accountingitempaymentgui.ui
    accountingbillprintergui.ui
    accountingtambillprintergui.ui
    accountinglsbillprintergui.ui
    attributesgui.ui
    varsgui.ui
    accountingbillpricedatagui.ui
    accountingtambillpricedatagui.ui
    accountinglsbillsdatagui.ui
    accountinglsbillpricedatagui.ui
)

include_directories(
    ../libmathparser
    ../libqcost
)

# Tell CMake to create the library
add_executable( QCost
    ${SRC} ${HDR} ${FRM} ${QRC} )

# Use required libraries
target_link_libraries(QCost qcost Qt5::Widgets Qt5::Sql)
