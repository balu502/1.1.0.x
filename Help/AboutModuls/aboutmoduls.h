#ifndef ABOUTMODULS_H
#define ABOUTMODULS_H

#include "aboutmoduls_global.h"

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QGroupBox>
#include <QTranslator>
#include <QSettings>
#include <QSplitter>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QBoxLayout>
#include <QTableWidget>
#include <QVector>
#include <QShowEvent>
#include <QDir>
#include <QDebug>
#include <QMap>
#include <QVector>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QLineEdit>
#include <QInputDialog>
#include <QPalette>
#include <QFont>
#include <QFontMetrics>
#include <QDomDocument>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QTranslator>

#include <QDebug>

#include <winver.h>
#include <Strsafe.h>
#include <stdio.h>
#include <shlobj.h>

typedef void (__stdcall *GetDTRDisplayInfo)(char* FormID, char** DisplayName, char** Path, int* Release, int* Build);

//-----------------------------------------------------------------------------

class ABOUTMODULSSHARED_EXPORT AboutModuls: public QDialog
{
    Q_OBJECT

public:
    AboutModuls(QWidget *parent = 0);
    virtual ~AboutModuls();

    void readCommonSettings();
    QTranslator translator;

private:
    QPushButton *close_button;
    QTreeWidget *info;
    GetDTRDisplayInfo get_Info;
    HINSTANCE ext_dll_handle;           // DTReport

    void Fill_Info();
    void Files_Version(QString);
    void Files_DTR(QString);
};

#endif // ABOUTMODULS_H
