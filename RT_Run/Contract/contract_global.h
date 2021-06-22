#ifndef CONTRACT_GLOBAL_H
#define CONTRACT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CONTRACT_LIBRARY)
#  define CONTRACTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CONTRACTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CONTRACT_GLOBAL_H
