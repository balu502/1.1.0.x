#ifndef HTTP_H
#define HTTP_H

#include <QObject>
#include <QWidget>
#include <QCoreApplication>
#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTextCodec>
#include <QFile>
#include <QDir>

#include <QtDebug>

#define TOUT_IN_S 3 // 3 sec timeout on network operation
#define ERR_NONE       0
#define ERR_TOUT       1
#define ERR_EMPTY      2
#define ERR_OPERATION  3
#define ERR_UANSWER    4
#define ERR_FEXIST     5
#define ERR_FOPEN      6
#define ERR_FWRITE     7
#define ERR_NFPHP      8

class http :public QObject
{
    Q_OBJECT

public:
    explicit  http(QWidget *parent = 0);
    virtual ~http();

    QStringList ListFiles;

    int getError(void){return error;} // return code of error 0 if OK
    QString getStrError(void); // return description of error

    void Sleep(int ms);

    void setDeviceAddress(QString str){SERVICE_URL="http://"+str+"/rt/data/glfulf.php";}
    int get_InputOutputListFiles(QString);   // Contents of Input directory on the device


signals:
    void sSetListFiles(QStringList&);
    void sSetError(int);
    void sUpdateListFiles();

public slots:
    void getListFiles(QString, bool);
    int putFile(QString);
    int getFile(QString, QString);
    int remove_File(QString, bool);

private slots:
    void replyFinishedNetworkReq(QNetworkReply*); // slot on reply from device

private:
    //QEventLoop *loop;
    QString SERVICE_URL;
    QNetworkAccessManager  *manager;

    bool reqCompleate;  // true after request
    int error;          // error code    
    QByteArray data;    // data with answer from device


};

#endif // HTTP_H
