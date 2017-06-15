cmake_minimum_required(VERSION 2.8.11)

# Find the required modules
find_package(Qt5 COMPONENTS Core Widgets Sql REQUIRED)

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

set ( FRMS
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
    ${SRC} ${FRMS} ${QRC} )

# Use required modules from Qt 5
target_link_libraries(QCost libqcost Qt5::Widgets Qt5::Sql)
