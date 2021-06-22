#include "http.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
http::http(QWidget *parent): QObject(parent)
{    
    manager = new QNetworkAccessManager(this);
    setDeviceAddress("127.0.0.1"); // set local    
    ListFiles.clear();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedNetworkReq(QNetworkReply*)));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
http::~http()
{
    if(manager) {delete manager; manager = 0;}
    data.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void http::Sleep(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}
//-----------------------------------------------------------------------------
//---  answer on request from device
//-----------------------------------------------------------------------------
void http::replyFinishedNetworkReq(QNetworkReply *reply)
{
  data.clear();
  data = reply->readAll(); 

  if(data.isEmpty()) error=ERR_EMPTY;
  if(data[0]=='-') error=ERR_OPERATION; // operation not execute
  if(data.startsWith("File not found")) error=ERR_NFPHP;
  reqCompleate = true;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void http::getListFiles(QString ip, bool in)
{
    QString in_out = "list_input";
    if(!in) in_out = "list_output";

    setDeviceAddress(ip);
    get_InputOutputListFiles(in_out);

    if(!error) emit sSetListFiles(ListFiles);

    emit sSetError(error);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int http::get_InputOutputListFiles(QString in_out)
{
    QUrl url(SERVICE_URL);
    QNetworkRequest request(url);
    QByteArray postData = in_out.toLatin1();

    reqCompleate = false;
    error = ERR_NONE;
    ListFiles.clear();

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request,postData);

    int tout=0;
    while(!reqCompleate)
    {
        qApp->processEvents();
        Sleep(100);

        if(tout++>TOUT_IN_S*10)
        {
            error = ERR_TOUT;
            break;
        }
    }
    //disconnect(manager,0,0,0);
    if(!error)
    {
        QString st(data);
        ListFiles.append(st.simplified().split(' '));
    }
    return error;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int http::putFile(QString fn)
{
    QFile file(fn);

    QUrl url(SERVICE_URL);
    QNetworkRequest request(url);
    QString bound = "margin";

    reqCompleate=false;
    error=ERR_NONE;

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        QByteArray postData(QString("--" + bound + "\r\n").toLatin1());
        postData.append("Content-Disposition: form-data; name=\"uploaded\"; filename=\"" + fn + "\"\r\n");
        postData.append("Content-Type: data/rt\r\n\r\n");
        postData.append(file.readAll());
        file.close();

        postData.append("\r\n");
        postData.append("--" + bound + "--\r\n");
        request.setRawHeader(QString("Content-Type").toLatin1(),QString("multipart/form-data; boundary=" + bound).toLatin1());
        request.setRawHeader(QString("Content-Length").toLatin1(), QString::number(postData.length()).toLatin1());
        manager->post(request,postData);

        int tout=0;
        while(!reqCompleate)
        {
            qApp->processEvents();
            Sleep(100);

            if(tout++>TOUT_IN_S*10)
            {
                error = ERR_TOUT;
                break;
            }
        }

        if(!error)
        {
            qDebug() << "Put ok: ";
        }
        else qDebug() << "Put Error: " << error;

        emit sUpdateListFiles();

        return(error);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int http::getFile(QString fn, QString filename)
{
    QUrl url(SERVICE_URL);
    QNetworkRequest request(url);
    QByteArray postData("get_output=");
    postData.append(fn);

    reqCompleate=false;
    error=ERR_NONE;

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request, postData);

    int tout=0;
    while(!reqCompleate)
    {
        qApp->processEvents();
        Sleep(100);

        if(tout++>TOUT_IN_S*10)
        {
            error = ERR_TOUT;
            break;
        }
    }

    if(!error)
    {
        qDebug() << "Get ok: ";

        QFile file(filename);
        if(file.open(QIODevice::WriteOnly))
        {
            file.write(data);
            file.close();
        }
        else qDebug() << "Error write file!";

    }
    else qDebug() << "Get Error: " << error;

    emit sUpdateListFiles();

    return(error);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int http::remove_File(QString fn, bool in)
{
    QUrl url(SERVICE_URL);
    QNetworkRequest request(url);
    QString remove = "remove_input=";

    if(!in) remove = "remove_output=";

    QByteArray postData(remove.toLatin1());
    postData.append(fn);

    qDebug() << "remove: " << fn << in << postData;

    reqCompleate=false;
    error=ERR_NONE;

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    manager->post(request,postData);

    int tout=0;
    while(!reqCompleate)
    {
        qApp->processEvents();
        Sleep(100);

        if(tout++>TOUT_IN_S*10)
        {
            error = ERR_TOUT;
            break;
        }
    }

    if(!error)
    {
        qDebug() << "Remove ok: ";
    }
    else qDebug() << "Remove Error: " << error;

    emit sUpdateListFiles();

    return(error);
}
