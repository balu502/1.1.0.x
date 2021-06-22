#include "wrapperax.h"

uint WrapperAx::_count = 0;

WrapperAx::WrapperAx(QObject *parent)
    : QObject(parent)
    , _com( 0 )
{

    if (!_count
    &&  FAILED( CoInitialize( NULL ))){

        qDebug() << "Unable to initialize COM";
    }

    _count++;
}

WrapperAx::WrapperAx(QAxObject *c, QObject *parent)
    : QObject(parent)
    , _com( c )
{
    if (!_count
    &&  FAILED( CoInitialize( NULL ))){

        qDebug() << "Unable to initialize COM";
    }

    _count++;
}


bool WrapperAx::setControl(const QString &ax){

    if ( _com )
        delete  _com;

                _com = new QAxObject();
    bool scr =  _com->setControl( ax );

    connect(_com, SIGNAL(exception(int, const QString &, const QString &, const QString &)),
            this, SLOT(onAxWidgetException(int, const QString &, const QString &, const QString &)));

    emit invalidChanged();
    return scr;
}

bool WrapperAx::setControl(QAxObject *cax){

    if ( _com )
        delete  _com;

    _com = cax;

    connect(_com, SIGNAL(exception(int, const QString &, const QString &, const QString &)),
            this, SLOT(onAxWidgetException(int, const QString &, const QString &, const QString &)));

    emit invalidChanged();
    return !cax->isNull();
}


QVariant WrapperAx::getProperty(const QString &n){

    return  isInvalid()? QVariant()
                : _com->property( n.toStdString().c_str() );
}

bool WrapperAx::setProperty(const QString &n, const QVariant &v){

    return  (!isInvalid())
            && _com->setProperty(n.toStdString().c_str(), v);
}


QVariant WrapperAx::asVariant() const{

    return  isInvalid() ? QVariant() : _com->asVariant();
}

void WrapperAx::onAxWidgetException(int c, const QString &s, const QString &d, const QString &h)
{
    qDebug() << "Exception:" << c << " in "
             << s << endl << d << endl << h << endl;
}


WrapperAx::~WrapperAx()
{
    if (!_count--)
        CoUninitialize();
}


bool WrapperAx::isInvalid() const
{
    return _com? _com->isNull(): true;
}


QVariant WrapperAx::dynamicCall(const QString& name,
    const QVariant &v1, const QVariant &v2, const QVariant &v3, const QVariant &v4,
    const QVariant &v5, const QVariant &v6, const QVariant &v7, const QVariant &v8){

    return  isInvalid()? QVariant()
                       : _com->dynamicCall( name.toStdString().c_str(),
                                            v1, v2, v3, v4, v5, v6, v7, v8  );
}


bool WrapperAx::querySubObject(WrapperAx* rw, const QString& name,
    const QVariant &v1, const QVariant &v2, const QVariant &v3, const QVariant &v4,
    const QVariant &v5, const QVariant &v6, const QVariant &v7, const QVariant &v8){

//    WrapperAx *rw = 0;

    if( !isInvalid() ){
        QAxObject *cr = _com->querySubObject( name.toStdString().c_str(),
                                              v1, v2, v3, v4, v5, v6, v7, v8  );
        (!cr->isNull())
        && rw->setControl( cr );
    }

    return  rw;
}


