#ifndef ZOOMPLATE_H
#define ZOOMPLATE_H

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QTableWidget>
#include <QLayout>
#include <QBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QDebug>
#include <QFileDialog>
#include <QPixmap>

#include <protocol.h>

//-----------------------------------------------------------------------------
class ZoomPlateItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    rt_Protocol *p_prot;
    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

//-----------------------------------------------------------------------------
class ZoomPlate: public QDialog
{
    Q_OBJECT
public:

    ZoomPlate(rt_Protocol *p, QString fn, QMap<QString,QString>*, QWidget *parent = 0);
    ~ZoomPlate();

    QString file_name;
    QTableWidget    *zoom_Table;
    QPushButton     *print_Button;
    QPushButton     *png_Button;

    ZoomPlateItemDelegate *delegate;

    private slots:
    void zoom_SaveAsPNG();


};

#endif // ZOOMPLATE_H
