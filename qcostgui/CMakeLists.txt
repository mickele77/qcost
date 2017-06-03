cmake_minimum_required(VERSION 2.8.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the required modules
find_package(Qt5 COMPONENTS Core REQUIRED)

# Tell CMake to create the library
add_executable( QCost
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

# Use required modules from Qt 5
target_link_libraries(QCost libqcost)
