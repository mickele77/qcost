#ifndef MATHPARSER_EXPORT_H
#define MATHPARSER_EXPORT_H

#include <QtGlobal>

#ifdef BUILD_SHARED_WIN
    #ifdef BUILD_MATHPARSER_LIB
        #define EXPORT_MATHPARSER_LIB_OPT Q_DECL_EXPORT
    #else
        #define EXPORT_MATHPARSER_LIB_OPT Q_DECL_IMPORT
    #endif
#else
    #define EXPORT_MATHPARSER_LIB_OPT
#endif

#endif // MATHPARSER_EXPORT_H

