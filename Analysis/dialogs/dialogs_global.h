#ifndef DIALOGS_GLOBAL_H
#define DIALOGS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DIALOGS_LIBRARY)
#  define DIALOGSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DIALOGSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DIALOGS_GLOBAL_H
