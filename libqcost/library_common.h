#ifndef LIBRARY_COMMON_H
#define LIBRARY_COMMON_H

#include <QtGlobal>

#ifdef BUILD_SHARED_WIN
    #ifdef BUILD_LIB
        #define EXPORT_LIB_OPT Q_DECL_EXPORT
    #else
        #define EXPORT_LIB_OPT Q_DECL_IMPORT
    #endif
#else
    #define EXPORT_LIB_OPT
#endif

#endif // LIBRARY_COMMON_H
