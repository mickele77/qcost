cmake_minimum_required(VERSION 2.8.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the required modules
find_package(Qt5 COMPONENTS Core REQUIRED)

# Tell CMake to create the library
add_library( libqcost SHARED
    projectbillparentitem.cpp
    projectdataparentitem.cpp
    projectrootitem.cpp
    projectitem.cpp
    projectpricelistparentitem.cpp
    pricelist.cpp
    bill.cpp
    project.cpp
    unitmeasure.cpp
    unitmeasuremodel.cpp
    var.cpp
    varsmodel.cpp
    billitem.cpp
    priceitem.cpp
    pricefieldmodel.cpp
    pricelistprinter.cpp
    billprinter.cpp
    priceitemdatasetmodel.cpp
    projectaccountingparentitem.cpp
    accountingprinter.cpp
    attribute.cpp
    accountingpricefieldmodel.cpp
    accountingbill.cpp
    accountingbillitem.cpp
    accountingbillitemprivate.cpp
    accountinglsbills.cpp
    accountingtambill.cpp
    accountingtambillitem.cpp
    measuresmodel.cpp
    accountinglsbill.cpp
    accountinglsbillitem.cpp
    accountinglsitemmeasure.cpp
    measureslsmodel.cpp
    paymentdata.cpp
    paymentdatamodel.cpp
    measure.cpp
    attributesmodel.cpp
 )

# Use required modules from Qt 5
target_link_libraries(libqcost Qt5::Core libodtcreator libmathparser)
