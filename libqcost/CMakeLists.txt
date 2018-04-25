cmake_minimum_required(VERSION 2.8.11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the required modules
find_package(Qt5 COMPONENTS Core REQUIRED)

# source files
set( SRC
    projectbillparentitem.cpp
    projectdataparentitem.cpp
    projectrootitem.cpp
    projectitem.cpp
    projectpricelistparentitem.cpp
    project.cpp
    unitmeasure.cpp
    unitmeasuremodel.cpp
    var.cpp
    varsmodel.cpp
    attribute.cpp
    attributesmodel.cpp
    pricefieldmodel.cpp
    priceitem.cpp
    pricelist.cpp
    pricelistprinter.cpp
    measure.cpp
    measuresmodel.cpp
    billitem.cpp
    bill.cpp
    billprinter.cpp
    priceitemdatasetmodel.cpp
    projectaccountingparentitem.cpp
    accountingprinter.cpp
    accountingpricefieldmodel.cpp
    accountingbillitem.cpp
    accountingbillitemprivate.cpp
    accountingbill.cpp
    accountingtammeasure.cpp
    accountingtammeasuresmodel.cpp
    accountingtambillitemprivate.cpp
    accountingtambillitem.cpp
    accountingtambill.cpp
    accountinglsmeasure.cpp
    accountinglsmeasuresmodel.cpp
    accountinglsbillitem.cpp
    accountinglsbill.cpp
    accountinglsbills.cpp
    paymentdata.cpp
    paymentdatamodel.cpp
)

# header files
set( HDR
    projectbillparentitem.h
    projectdataparentitem.h
    projectrootitem.h
    projectitem.h
    projectpricelistparentitem.h
    project.h
    unitmeasure.h
    unitmeasuremodel.h
    var.h
    varsmodel.h
    attribute.h
    attributesmodel.h
    pricefieldmodel.h
    priceitem.h
    pricelist.h
    pricelistprinter.h
    measure.h
    measuresmodel.h
    billitem.h
    bill.h
    billprinter.h
    priceitemdatasetmodel.h
    projectaccountingparentitem.h
    accountingprinter.h
    accountingpricefieldmodel.h
    accountingbillitem.h
    accountingbillitemprivate.h
    accountingbill.h
    accountingtammeasure.h
    accountingtammeasuresmodel.h
    accountingtambillitemprivate.h
    accountingtambillitem.h
    accountingtambill.h
    accountinglsmeasure.h
    accountinglsmeasuresmodel.h
    accountinglsbillitem.h
    accountinglsbill.h
    accountinglsbills.h
    paymentdata.h
    paymentdatamodel.h
)

# Tell CMake to create the library
add_library( libqcost
    ${SRC} ${HDR}
)

include_directories(
    ../libmathparser
    ../libodtcreator
)

# Use required modules from Qt 5
target_link_libraries(libqcost Qt5::Core libodtcreator libmathparser)
