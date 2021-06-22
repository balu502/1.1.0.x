#ifndef EDITOR_GLOBAL_H
#define EDITOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(EDITOR_LIBRARY)
#  define EDITORSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EDITORSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // EDITOR_GLOBAL_H
