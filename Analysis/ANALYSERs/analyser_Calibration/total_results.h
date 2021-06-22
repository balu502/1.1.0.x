#ifndef TOTAL_RESULTS_H
#define TOTAL_RESULTS_H

#include <QObject>
#include <QApplication>
#include <QWidget>
#include <QGroupBox>
#include <QBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QTextEdit>

#include "protocol.h"
#include "utility.h"

//-----------------------------------------------------------------------------
class Total_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

};
//-----------------------------------------------------------------------------
class Total_Results : public QGroupBox
{
    Q_OBJECT
public:
    Total_Results(QWidget *parent = 0);
    virtual ~Total_Results();

    QTableWidget        *Total_Table;
    Total_ItemDelegate  *Delegate;

    QLabel      *Date_label;
    QLabel      *Device_label;
    QLabel      *Operator_label;
    QLabel      *Plate_label;
    QTextEdit   *Conclusion;
    int         total_result;

    QToolButton *Email_button;
    QToolButton *Web_button;
    QPushButton *Open_Dir;

    QMap<QString,QString> *mask_ATTENTION;
    QMap<QString,QString> *temperature_ATTENTION;
    QMap<QString,QString> *optic_ATTENTION;
    bool mask_status;
    bool temperature_status;
    bool optic_status;

    void fill_TotalResults(rt_Protocol *p, QVector<int> *error);


signals:

public slots:
};

#endif // TOTAL_RESULTS_H
