#ifndef SAVER_GLOBAL_H
#define SAVER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SAVER_LIBRARY)
#  define SAVERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SAVERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SAVER_GLOBAL_H
