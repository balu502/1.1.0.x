#ifndef REPORT_QUALITY_H
#define REPORT_QUALITY_H

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

#include "report_quality_global.h"
#include "protocol.h"
#include "utility.h"
#include "report_interface.h"
#include "../../RT_Analysis/define_PCR.h"

class REPORT_QUALITYSHARED_EXPORT Report_Quality: public QObject, public Report_Interface
{
    Q_OBJECT

public:
    Report_Quality();

    void *Create_Report(rt_Protocol*);
    void Destroy_Report();
    void Type_Report(int*, QString*);
    void Set_SamplesEnable(QVector<short>*);

    rt_Protocol *prot;
    NCReport *report;
    int type_test;
    QString test_description;
    QVector<short> sample_enable;

    //QTranslator translator;
    QTranslator translator_report;
    void readCommonSettings();
    QString lang;
};

#endif // REPORT_QUALITY_H
