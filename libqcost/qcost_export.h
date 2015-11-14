#ifndef QCOST_EXPORT_H
#define QCOST_EXPORT_H

#include <QtGlobal>

#ifdef BUILD_SHARED_WIN
    #ifdef BUILD_QCOST_LIB
        #define EXPORT_QCOST_LIB_OPT Q_DECL_EXPORT
    #else
        #define EXPORT_QCOST_LIB_OPT Q_DECL_IMPORT
    #endif
#else
    #define EXPORT_QCOST_LIB_OPT
#endif

#endif // QCOST_EXPORT_H
