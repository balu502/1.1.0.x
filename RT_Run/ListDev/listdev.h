#ifndef LISTDEV_H
#define LISTDEV_H

#include "listdev_global.h"

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

#include <QNetworkInterface>

#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDataStream>
#include <QByteArray>
//#include "../../Main_RT/global_param.h"

//#include "point_action.h"

//----------------------------------------------------------------------------

class Connect_Status: public QEvent
{
public:
    Connect_Status(): QEvent((Type)2001)
    {
    }
    int connect;    // 0 - disconnect  1 - connect  2 - reconnect
    QString ip;
    int port;
};

//-----------------------------------------------------------------------------

class ListDev_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

};

//----------------------------------------------------------------------------

class LISTDEVSHARED_EXPORT ListDev: public QDialog
{
    Q_OBJECT

public:
    ListDev(QWidget *parent = 0, QString ip = "localhost", int port = 9001);
    virtual ~ListDev();

    QWidget *p_obj;         // parent
    QWidget *p_main;        // main widget - integrator
    QString ip_addres;
    QString application_name;

    QTableWidget            *Dev_Table;
    ListDev_ItemDelegate    *Delegate;
    //int                     current_dev;
    QVector<int>            current_dev;
    int                     dev_state;
    int                     port_serverUSB;

    QPushButton     *connect_a5d411;
    QPushButton     *connect_dev;
    QPushButton     *disconnect_dev;
    QPushButton     *cancel_dev;
    //QFont           app_font;

    Connect_Status  connect_event;    
    bool            PA_active;
    QString         PA_DeviceName;

    QTimer dev_reconnected;
    QString name_reconnect;

    QString Device_Debug;
    QString Port_Debug;
    QString IP_Debug;

private:
    //... NetWork ...
    QTcpSocket  *m_pTcpSocket;          // socket for connection with Server_USB
    quint32      m_nNextBlockSize;      // ...

    QTcpSocket  *s_pTcpSocket;          // socket for check connection status for every Server_Dev's
    quint32      s_nNextBlockSize;      // ...

    QTranslator translator;
    void readCommonSettings();

    QVector<QString> Client_Net;        // [0] - command [1...] - parameters

private slots:
    void slotConnected();
    void slotReadyRead();
    void slotSendToServer();
    void slotError(QAbstractSocket::SocketError);

    void slotConnected_status();
    void slotReadyRead_status();
    void slotError_status(QAbstractSocket::SocketError);
    void slotConnection_status(int);
    void slotCheckStatusDevice();

    void slotApply_ConnectAction(QTableWidgetItem*);

//public slots:
//    virtual void showEvent(QShowEvent *event);
//    virtual void closeEvent(QCloseEvent *event);

public slots:
    void slotConnectDev_a5d411();
    void slotConnectDev();
    void slotDisconnectDev();
    void slotRefresh();

    void slotCheckSelection();

    void sReConnected();

signals:
    void signal_connected(QString);
    void signal_disconnected();
    void signal_connect(int);

    // Point Action signals:
    void sPA_listDevice();

private slots:
    // Point Action slots:
    void PA_listDevice();

protected:
    void closeEvent(QCloseEvent *e);
    void showEvent(QShowEvent *e);

};

#endif // LISTDEV_H
