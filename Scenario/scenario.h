#ifndef SCENARIO_H
#define SCENARIO_H

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QSplitter>
#include <QFont>
#include <QFontMetrics>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSize>
#include <QList>
#include <QString>
#include <QFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QWindow>
#include <QTabWidget>
#include <QEvent>
#include <QResizeEvent>
#include <QVariant>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QPainterPath>
#include <QHeaderView>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPalette>
#include <QShowEvent>
#include <QTranslator>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMovie>
#include <QTimer>
#include <QHash>

#include <QDomDocument>


#include "scenario_global.h"
#include "point_action.h"

#include <protocol.h>
#include <utility.h>

//-----------------------------------------------------------------------------

class Scenario_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    bool    *active;
    int     *current;

};
//-----------------------------------------------------------------------------
class SCENARIOSHARED_EXPORT Scenario: public QDialog
{
    Q_OBJECT

public:
    Scenario(QWidget *parent = 0);
    virtual ~Scenario();

    void readCommonSettings();
    QTranslator translator;

    bool open_xmlScenario(QString);
    void load_TableScenario();

    QMap<QString, QString>  *List_Protocols;
    QMap<QString, QWidget*> Map_Receivers;
    QList<Point_Action*>    List_PointAction;
    bool                    active_scenario;
    int                     current_point;
    Action_Event            scenario_Event;
    QString                 name_Scenario;
    bool                    web_break;

    QTableWidget    *Table_Scenario;
    Scenario_ItemDelegate    *Delegate;

    QLabel *active_label;
    QMovie *active_gif;
    QLabel *label_status;
    QTimer *timer_wait;

    QPushButton     *Scenario_Open;
    QPushButton     *Scenario_Start;
    QPushButton     *Scenario_Stop;
    QPushButton     *Scenario_Clear;

    Point_Action    *Stop_Point;

public slots:
    void slot_StopScenario();
    void Stop_Scenario();
    void Start_Scenario();
    void Open_Scenario(QString fn = "");
    void Clear_Scenario();
    void Run_Scenario(QString, QMap<QString,QString>);

    void Open_CalibrationScenario();

    void RunNextPoint(bool next = true);
    void Change_Status();

    void GetRunButton();
    void GetProtocolFromWeb(QString);
    void WaitProtocolfromWeb();

    void CreateInfoExecutePoint(Action_Event*);

signals:
    void sSignalToReceiver(QWidget*);
    void sGetProtocolFromList(QString,QString); // name Protocol, barcode Protocol
    void sStatus_ExecutePoint(QString);

protected:
    bool event(QEvent *e);
    void resizeEvent(QResizeEvent *e);
    //void closeEvent(QShowEvent * event);
};

#endif // SCENARIO_H
