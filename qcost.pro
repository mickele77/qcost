TEMPLATE = subdirs

SUBDIRS	= \
    libmathparser \
    libodtcreator \
    libqcost \
    qcostgui

qcostgui.depends = libqcost

libqcost.depends = libodtcreator
libqcost.depends += libmathparser
