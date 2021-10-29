#include "server_usb.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //---
    QSystemSemaphore semaphore("balu_9001", 1); // create semaphore
    semaphore.acquire();                        // up semaphore

    QSharedMemory sharedMemory("balu_9002");    // create shared memory
    bool is_running;                            //
    if(sharedMemory.attach()) is_running = true;
    else
    {
        sharedMemory.create(1);                 // create 1 byte memory
        is_running = false;
    }
    semaphore.release();                        // down semaphore
    if(is_running) return 1;
    //---

    QStringList args = a.arguments();
    int port = 9011;
    bool ok;

    //qDebug() << "args.size(): "  << args.size();
    if(args.size() > 1)
    {
        port = args.at(1).toInt(&ok);
    }
    else
    {
        QString dir_path = qApp->applicationDirPath();
        QSettings* RunSettings = new QSettings(dir_path + "/tools/ini/preference_run.ini", QSettings::IniFormat);

        RunSettings->beginGroup("Server_USB");
        port = RunSettings->value("port","9011").toInt();
        RunSettings->endGroup();

        delete RunSettings;
    }    

    Server_USB w(port);
    w.show();
    QTimer::singleShot(5000, &w, SLOT(hide()));
    QTimer::singleShot(100, &w, SLOT(create_FXCypress()));

    return a.exec();
}
