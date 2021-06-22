#ifndef ALL_CP_GLOBAL_H
#define ALL_CP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ALL_CP_LIBRARY)
#  define ALL_CPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ALL_CPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ALL_CP_GLOBAL_H
