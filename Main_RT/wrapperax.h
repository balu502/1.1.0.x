#ifndef WRAPPERAX_H
#define WRAPPERAX_H

#include <QObject>
#include <QAxObject>
#include <QDebug>

#include "objbase.h"


class QAxObject;


class WrapperAx: public QObject{

    Q_OBJECT

    Q_PROPERTY(bool isInvalid  READ isInvalid NOTIFY invalidChanged)

    QAxObject    *_com;
    static uint _count;

public:
    WrapperAx(QObject *p=0);
    WrapperAx(QAxObject*, QObject *p=0);
   ~WrapperAx();

    bool isInvalid() const;

    Q_INVOKABLE
    QVariant dynamicCall(const QString&,
                        const QVariant &v1 = QVariant(),   const QVariant &v2 = QVariant(),
                        const QVariant &v3 = QVariant(),   const QVariant &v4 = QVariant(),
                        const QVariant &v5 = QVariant(),   const QVariant &v6 = QVariant(),
                        const QVariant &v7 = QVariant(),   const QVariant &v8 = QVariant());
    Q_INVOKABLE
    bool querySubObject( WrapperAx*, const QString&,
                        const QVariant &v1 = QVariant(),    const QVariant &v2 = QVariant(),
                        const QVariant &v3 = QVariant(),    const QVariant &v4 = QVariant(),
                        const QVariant &v5 = QVariant(),    const QVariant &v6 = QVariant(),
                        const QVariant &v7 = QVariant(),    const QVariant &v8 = QVariant());

    Q_INVOKABLE
    bool setControl(const QString &ax);

    bool setControl(QAxObject*);

    Q_INVOKABLE
    QVariant getProperty(const QString&);

    Q_INVOKABLE
    bool setProperty(const QString&,const QVariant&);

    Q_INVOKABLE
    QVariant asVariant() const;

signals:
    void    invalidChanged();



    private slots:
    void onAxWidgetException(int, const QString &, const QString &, const QString &);
};


#endif // WRAPPERAX_H
