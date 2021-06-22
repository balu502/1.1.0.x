#ifndef RUNTIME_GLOBAL_H
#define RUNTIME_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(RUNTIME_LIBRARY)
#  define RUNTIMESHARED_EXPORT Q_DECL_EXPORT
#else
#  define RUNTIMESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // RUNTIME_GLOBAL_H
