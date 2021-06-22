#ifndef WAIT_PROCESS_H
#define WAIT_PROCESS_H

#include "wait_process_global.h"

#include <QWidget>
#include <QString>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

typedef bool (callback_WaitProcess)(void*, int, QString);


class WAIT_PROCESSSHARED_EXPORT Wait_Process : public QWidget
{
    Q_OBJECT

public:
    Wait_Process(QWidget *parent = 0);
    virtual ~Wait_Process();

    void wait(void*, callback_WaitProcess*, QString);
};

#endif // WAIT_PROCESS_H
