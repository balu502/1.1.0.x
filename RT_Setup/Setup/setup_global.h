#ifndef SETUP_GLOBAL_H
#define SETUP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SETUP_LIBRARY)
#  define SETUPSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SETUPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // SETUP_GLOBAL_H
