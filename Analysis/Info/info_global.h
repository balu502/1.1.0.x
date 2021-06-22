#ifndef INFO_GLOBAL_H
#define INFO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(INFO_LIBRARY)
#  define INFOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define INFOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // INFO_GLOBAL_H
