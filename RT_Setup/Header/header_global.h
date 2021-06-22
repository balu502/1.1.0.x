#ifndef HEADER_GLOBAL_H
#define HEADER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(HEADER_LIBRARY)
#  define HEADERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define HEADERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // HEADER_GLOBAL_H
