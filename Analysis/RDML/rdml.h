#ifndef RDML_H
#define RDML_H

#include <QtWidgets>
#include <QWidget>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>


#include <QDebug>

#include "protocol.h"
#include "utility.h"
#include "define_PCR.h"

#include "RDML_global.h"

class RDML_EXPORT RDML
{
public:
    RDML();
};

void RDML_LoadTest(QString, rt_Test*);
void RDML_LoadSourceSet(QString, rt_Protocol*);

#endif // RDML_H
