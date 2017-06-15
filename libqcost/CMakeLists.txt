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
    accountingtammeasure.cpp
    accountingtammeasuresmodel.cpp
    accountinglsbill.cpp
    accountinglsbillitem.cpp
    accountinglsmeasure.cpp
    accountinglsmeasuresmodel.cpp
    paymentdata.cpp
    paymentdatamodel.cpp
    measure.cpp
    measuresmodel.cpp
    attributesmodel.cpp
)

# header files
set( HDR
    projectbillparentitem.h
    projectdataparentitem.h
    projectrootitem.h
    projectitem.h
    projectpricelistparentitem.h
    pricelist.h
    bill.h
    project.h
    unitmeasure.h
    unitmeasuremodel.h
    var.h
    varsmodel.h
    billitem.h
    priceitem.h
    pricefieldmodel.h
    pricelistprinter.h
    billprinter.h
    priceitemdatasetmodel.h
    projectaccountingparentitem.h
    accountingprinter.h
    attribute.h
    accountingpricefieldmodel.h
    accountingbill.h
    accountingbillitem.h
    accountingbillitemprivate.h
    accountinglsbills.h
    accountingtambill.h
    accountingtambillitem.h
    accountingtammeasure.h
    accountingtammeasuresmodel.h
    accountinglsbill.h
    accountinglsbillitem.h
    accountinglsmeasure.h
    accountinglsmeasuresmodel.h
    paymentdata.h
    paymentdatamodel.h
    measure.h
    measuresmodel.h
    attributesmodel.h
)

# Tell CMake to create the library
add_library( libqcost SHARED
    ${SRC} ${HDR}
)

include_directories(
    ../libmathparser
    ../libodtcreator
)

# Use required modules from Qt 5
target_link_libraries(libqcost Qt5::Core libodtcreator libmathparser)
