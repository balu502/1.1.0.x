#ifndef CONTRACT_H
#define CONTRACT_H

#include "contract_global.h"

#include <QObject>
#include <QWindow>
#include <QApplication>
#include <QDialog>
#include <QtWidgets>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QWindow>
#include <QFont>
#include <QString>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QIcon>
#include <QAction>
#include <QSettings>
#include <QShowEvent>
#include <QDebug>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStringList>
#include <QMessageBox>
#include <QEvent>
#include <QThread>
#include <QTimer>
#include <QSettings>
#include <QTranslator>
#include <QMessageBox>
#include <QToolButton>
#include <QShowEvent>


class CONTRACTSHARED_EXPORT Contract: public QDialog
{
    Q_OBJECT

public:
    Contract(QWidget *parent = 0);
    virtual ~Contract();


    QTranslator translator;
    void readCommonSettings();

    QDir *user_Dir;
    int count_days;
    QMap<QString, QByteArray> *map_CRYPTO;

signals:
    void sSend_CRYPTO();


private:

    QGroupBox   *Control_Box;
    QLabel      *label_CRYR;
    QLabel      *label_CRYS;
    QLabel      *label_CRYG;
    QPushButton *button_CRYR;
    QPushButton *button_CRYS;
    QPushButton *button_CRYG;
    QToolButton *tool_CRYR;
    QToolButton *tool_CRYS;
    QToolButton *tool_CRYG;

    QLabel      *remained_Days;

    QLabel      *state_CRYR;
    QLabel      *state_CRYS;
    QLabel      *state_CRYG;

    QGroupBox   *Container_Box;
    QTextEdit   *Text_Container;

public slots:
    void slot_CRYR();
    void slot_SaveCRYR();
    void slot_CRYS();
    void slot_OpenCRYS();
    void slot_CRYG();
    void slot_SaveCRYG();

    void Read_CRYPTO(QMap<QString,QByteArray>*);

protected:
    void showEvent(QShowEvent *e);
};

#endif // CONTRACT_H
