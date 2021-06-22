#ifndef LISTDEV_GLOBAL_H
#define LISTDEV_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LISTDEV_LIBRARY)
#  define LISTDEVSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LISTDEVSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LISTDEV_GLOBAL_H
