#ifndef SERVER_USB_H
#define SERVER_USB_H

#include <QMainWindow>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenu>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QAction>
#include <QMessageBox>
#include <QEvent>
#include <QCloseEvent>
#include <QTimer>
#include <QTableWidget>
#include <QHeaderView>
#include <QVector>
#include <QStringList>
#include <QTableWidgetItem>
#include <QDebug>
#include <QTime>
#include <QLabel>
#include <QStatusBar>
#include <QVector>
#include <QHash>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QThread>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QSettings>
#include <QTranslator>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QToolButton>
#include <QSettings>
#include <QTimer>
#include <QCoreApplication>

#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QDataStream>
#include <QByteArray>
#include <QNetworkInterface>

#include "CypressUsb.h"

#define APPLICATION_NAMES  "DTmaster" << "DTcheck" << "RealTime__PCR"

//#include "../Main_RT/global_param.h"
//-----------------------------------------------------------------------------
typedef struct
{
    HANDLE devHandle;
    QString SerNum;
    USHORT PID;
    USHORT VID;
    HWND handle_win;
    USHORT status;
    QString IP_port;
    bool busy;

}USB_Info;
//-----------------------------------------------------------------------------
class Server_Process : public QProcess
{
    Q_OBJECT

public:
    Server_Process(QWidget *parent = 0);
    ~Server_Process();

    QString SerName;
    QString IP_port;
    int port;

signals:

    void start_proc(Server_Process*);
    void finish_proc(Server_Process*);

public slots:
    void Started()
    {
        emit start_proc(this);
    }
    void Finished()
    {
        emit finish_proc(this);
    }
};

//-----------------------------------------------------------------------------
class ServerItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};
//-----------------------------------------------------------------------------
class Server_USB : public QMainWindow
{
    Q_OBJECT

public:
    Server_USB(int nPort, QWidget *parent = 0);
    ~Server_USB();

    void readCommonSettings();
    QTranslator translator;

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    //bool event(QEvent *e);

private slots:        
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void setTrayIconActions();
    void showTrayIcon();
    void create_FXCypress();
    void create_InfoUSBDevice();
    void fill_TableDevice();

    void Run_externalSingleServer(QString, int);
    void Run_externalServer();
    void Stop_externalServer();
    void Started_Process(Server_Process*);
    void Finished_Process(Server_Process*);

    void Check_CompatibilityVersion();
    void Check_DeviceBusy();
    void ReLoad_InfoUSBDevice();
    void Kill_Server(QString);

    void Check_SleepRejime();

private:
    QSystemTrayIcon *trayIcon;
    QMenu           *trayIconMenu;
    QAction         *minimizeAction;
    QAction         *restoreAction;
    QAction         *quitAction;

    CyDev           *FX2;           // usb
    QVector<USB_Info*> list_fx2;
    QHash<QString, Server_Process*> list_server;
    Server_Process *server_SYT;

    QTableWidget    *Dev_Table;
    ServerItemDelegate *delegate;
    //QToolButton *event_button;

    QStringList application_names;
    int PORT;
    QString ip_addres;

    //... NetWork ...
    QTcpServer* m_ptcpServer;
    //ushort      m_nNextBlockSize;
    quint32      m_nNextBlockSize;
    QLabel      *net_message;

    QVector<QString> Server_Net;
    QList<QTcpSocket*> m_pClientSocketList;

    void sendToClient(QTcpSocket* pSocket);

    QMessageBox message;
    QTimer      *timer;
    bool        view_CloseMessage;
    bool        exists_RT7;

    QTimer *timer_SLEEP;

public slots:
    virtual void slotNewConnection();
            void slotReadClient();
private:
    //... Client NetWork ...
    //QTcpSocket  *m_pTcpSocket;
    //ushort      m_nNextBlockSize_client;

private slots:
    void slotConnected();
    void slotReadyRead();
    void slotSendToServer(QString request);
    void slotError(QAbstractSocket::SocketError);
    void slotClientDisconnected();
    void slotCLOSE();

};

#endif // SERVER_USB_H
