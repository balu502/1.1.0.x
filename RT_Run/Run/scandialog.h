#ifndef SCANDIALOG_H
#define SCANDIALOG_H

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QBoxLayout>
#include <QTextEdit>
#include <QTextBrowser>
#include <QTabWidget>
#include <QLineEdit>
#include <QTextDocument>
#include <QEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
#include <QDomDocument>
#include <QLabel>
#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>

#include <protocol.h>
#include <utility.h>


class ScanDialog: public QDialog
{
    Q_OBJECT

public:
    ScanDialog(QWidget *parent = 0, bool def_Param = false);
    virtual ~ScanDialog();

    QTabWidget  *Tab;
    QTextEdit   *TextEdit;
    QGroupBox   *box_parameters;
    QLineEdit   *TPeaks_values;
    QLineEdit   *Amplitude_values;
    QLineEdit   *ID_plate;
    QLabel      *TPeaks_label;
    QLabel      *Amplitude_label;
    QLabel      *ID_label;
    QPushButton *Cancel_Button;
    QPushButton *Apply_Button;    
    QPushButton *Default_Button;
    QButtonGroup *GroupButtons;
    QPushButton *ClearTextEdit;

    QMessageBox message;
    QTimer  *timer;
    int Result;
    QString name_Dev;
    int count_ActiveCh;

    QString sn_LOT;
    QString date_plate;
    int Limit_startup;

    QMap<QString, QMap<QString,QString>*> map_Plate;


    void Write_RegisteredItem();


private slots:
    void slotTimerCheck();
    void close_Exit(QAbstractButton*);
    void Default_Values();
    bool Check_ValidCoefficients();
    void Check_ApplyChanges();
    void clear_TextEdit();
    void Set_DefaultValue_Exit();

signals:
    void sReadSettings(QMap<QString,QString>*);


};

#endif // SCANDIALOG_H
