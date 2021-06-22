#ifndef ZOOM_H
#define ZOOM_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QDialog>
#include <QBoxLayout>
#include <QImage>
#include <QPainter>
#include <QBoxLayout>

class ZoomImage_Widget;

class Zoom_Dialog : public QDialog
{
    Q_OBJECT
public:
    Zoom_Dialog(QWidget *parent = 0);
    ~Zoom_Dialog();


    ZoomImage_Widget *zoom_widget;
    QImage           *image;

signals:

public slots:
};

//-----------------------------------------------------------------------------
class ZoomImage_Widget: public QWidget
{
    Q_OBJECT

public:
        ZoomImage_Widget(QImage *i, QWidget *parent = 0);

        QImage  *image;
        Zoom_Dialog *p_zoomDialog;

protected:
        void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;
};

#endif // ZOOM_H
