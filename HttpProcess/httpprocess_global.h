#ifndef HTTPPROCESS_GLOBAL_H
#define HTTPPROCESS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(HTTPPROCESS_LIBRARY)
#  define HTTPPROCESSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define HTTPPROCESSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // HTTPPROCESS_GLOBAL_H
