#ifndef ADD_TESTS_GLOBAL_H
#define ADD_TESTS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ADD_TESTS_LIBRARY)
#  define ADD_TESTSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ADD_TESTSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ADD_TESTS_GLOBAL_H
