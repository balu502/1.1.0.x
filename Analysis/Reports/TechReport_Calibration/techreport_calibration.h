#ifndef TECHREPORT_CALIBRATION_H
#define TECHREPORT_CALIBRATION_H

#include <QtCore/QObject>
#include <QtCore/qmath.h>
#include <QObject>
#include <QApplication>
#include <QWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QGroupBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPoint>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QEvent>
#include <QMouseEvent>
#include <QFont>
#include <QFontMetrics>
#include <QSplitter>
#include <QBoxLayout>
#include <QTranslator>
#include <QSettings>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QMainWindow>
#include <QToolBar>
#include <QFileDialog>
#include <QTime>
#include <QDoubleValidator>
#include <QSpinBox>
#include <QClipboard>
#include <QFileDialog>
#include <QFile>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QPrinter>
#include <QDomDocument>
#include <QDebug>

#include <ncreport.h>
#include <ncreportoutput.h>

#include "techreport_calibration_global.h"
#include "protocol.h"
#include "report_interface.h"

class TECHREPORT_CALIBRATIONSHARED_EXPORT TechReport_Calibration: public QObject, public Report_Interface
{
    Q_OBJECT

public:
    TechReport_Calibration();

    void *Create_Report(rt_Protocol*);
    void Destroy_Report();
    void Type_Report(int*, QString*);
    void Set_SamplesEnable(QVector<short>*);

    rt_Protocol *prot;
    NCReport *report;
    int type_test;
    QString test_description;

    //QTranslator translator;
    QTranslator translator_report;
    void readCommonSettings();
    QString lang;
};

#endif // REPORT_CALIBRATION_H
