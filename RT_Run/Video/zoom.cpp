#include "zoom.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Zoom_Dialog::Zoom_Dialog(QWidget *parent)
        :QDialog(parent)
{
    image = new QImage(300, 300, QImage::Format_RGB32);
    image->fill(Qt::red);
    zoom_widget = new ZoomImage_Widget(image, this);
    zoom_widget->setFixedSize(300, 300);

    QVBoxLayout *Main_layout = new QVBoxLayout;
    Main_layout->setMargin(0);
    setLayout(Main_layout);
    Main_layout->addWidget(zoom_widget);

    Qt::WindowFlags flags = 0;
    flags = Qt::Drawer; //Qt::Dialog;
    flags |= Qt::MSWindowsFixedSizeDialogHint;
    flags |= Qt::WindowCloseButtonHint;
    setWindowFlags(flags);
    //setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle("Zoom");        
    hide();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Zoom_Dialog::~Zoom_Dialog()
{
    delete image;
    delete zoom_widget;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ZoomImage_Widget::ZoomImage_Widget(QImage *picture, QWidget *parent): QWidget(parent)
{
    p_zoomDialog = (Zoom_Dialog*)parent;
    image = picture;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ZoomImage_Widget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    //... image ...
    painter.drawImage(0,0,*image);
}
