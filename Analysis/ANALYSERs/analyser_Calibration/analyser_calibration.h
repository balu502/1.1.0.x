#ifndef ANALYSER_CALIBRATION_H
#define ANALYSER_CALIBRATION_H

//#include <QtCore/QObject>
//#include <QtCore/qmath.h>
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
#include <QDir>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QMessageBox>
#include <QRegExp>



#include <QDomDocument>

#include <QDebug>

#include "analyser_calibration_global.h"
#include "alg_interface_qevent.h"
#include "analysis_interface.h"
#include "utility.h"
#include "define_PCR.h"
#include "device_param.h"

#include <algorithm>
#include <gsl/gsl_fit.h>

#include "total_results.h"
#include "mask.h"
#include "melt_curve.h"

#define NAME_METHOD "Diagnostic_CARD"

//-----------------------------------------------------------------------------
class BaseExpo_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

};
//-----------------------------------------------------------------------------
class Tech_Delegate_1: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;
};
//-----------------------------------------------------------------------------
class Tech_Delegate_2: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QVector<short>  *TechReport_Item;
};
//-----------------------------------------------------------------------------
class ANALYSER_CALIBRATIONSHARED_EXPORT Analyser_Calibration: public QObject, public Analysis_Interface
{
    Q_OBJECT

public:
    Analyser_Calibration();

    void *Create_Win(void *, void *);
    void Destroy_Win();
    void Show();
    void Destroy();

    void GetInformation(QVector<QString> *info);
    void Analyser(rt_Protocol *prot);
    void Save_Results(char*);
    void Select_Tube(int pos);
    void Enable_Tube(QVector<short>*);

    void readCalibrationSettings();
    void readCalibrationSettings(QString);

    void readCommonSettings();
    QTranslator translator;
    QString StyleApp;
    QString Lang;
    QMessageBox message;
    bool flag_ControlDepartment;
    bool flag_BaseExposition;

    void Create_OpticResults(QDomDocument&, QDomElement&);
    void Create_TemperatureResults(QDomDocument&, QDomElement&);
    void Create_AmplitudeResults(QDomDocument&, QDomElement&);
    void Create_RawDataResults(QDomDocument&, QDomElement&);

    int Check_MinMaxAmplitude();

    void Fill_TechDoc();
    void Fill_BaseExposition();

    Open_TechReport event_TechReport;
    QWidget *main_widget;

private:
    rt_Protocol     *Prot;
    QTabWidget      *MainTab;
    QDir            VideoData;

    Total_Results   *Total;
    Mask            *mask_Optical;
    Melt_Curve      *melt_curve;

    QGroupBox       *Additional_Doc;
    QTableWidget    *Tech_Table;
    Tech_Delegate_1 *delegate_col1;
    Tech_Delegate_2 *delegate_col2;
    QPushButton     *Tech_Report;

    QGroupBox       *BaseExpo_Box;
    QTableWidget    *BaseExpo_Table;
    BaseExpo_ItemDelegate *delegate_BaseExpo;
    QLabel          *Info_BaseExpo;

    QVector<short>  TechReport_Item;
    QVector<double> TechReport_Borders;

private slots:
    void to_ClipBoard();
    void OpenTechReport();

};

#endif // ANALYSER_CALIBRATION_H
