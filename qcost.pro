TEMPLATE = subdirs

SUBDIRS	= \
    libmathparser \
    libodtcreator \
    libqcost \
    qcostgui

qcost.depends = libqcost
qcost.depends += libmathparser

libqcost.depends = libodtcreator
