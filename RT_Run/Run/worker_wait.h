#ifndef WORKER_WAIT_H
#define WORKER_WAIT_H

#include <QObject>
#include <QApplication>
#include <QString>
#include <QTimer>
#include <QDebug>
#include <QThread>

enum Wait_state {wRun,wStop,wMeasure,wOpen,wClose,wPreRun,wCmd,wPic};

class Worker_wait : public QObject
{
    Q_OBJECT
public:
    explicit Worker_wait(Wait_state, QString, QObject *parent = 0);
    virtual ~Worker_wait();

signals:
    void finished();
    void display_process(int, QString);
    void cursor_wait(bool);

public slots:
    void start_process();
    void stop_process();

    void slot_timeout();

private:

    QString Label;
    int  count;
    Wait_state state;

public:

};

#endif // WORKER_WAIT_H
