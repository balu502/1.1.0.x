#ifndef EXPO_GLOBAL_H
#define EXPO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EXPO_LIBRARY)
#  define EXPOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EXPOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // EXPO_GLOBAL_H
