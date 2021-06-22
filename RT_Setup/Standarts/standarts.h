#ifndef STANDARTS_H
#define STANDARTS_H

#include <QtCore/QObject>
#include <QApplication>
#include <QWidget>
#include <QObject>
#include <QTabWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QVector>
#include <QTableWidget>
#include <QHeaderView>
#include <QFont>
#include <QStyledItemDelegate>
#include <QPalette>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QLabel>
#include <QBoxLayout>
#include <QFont>
#include <QTreeWidget>
#include <QScrollArea>
#include <QDateTime>
#include <QPolygon>
#include <QSettings>
#include <QTranslator>
#include <QDomDocument>
#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QSpinBox>

#include <QDebug>

#include "protocol.h"
#include "define_PCR.h"
#include "utility.h"

#include "windows.h"
#include "standarts_global.h"

#define STANDARTS_INFORMATION "Standarts Information"

//-----------------------------------------------------------------------------

class Standarts1ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    Standarts1ItemDelegate(QObject *parent=0) : QStyledItemDelegate(parent)
    {
        Standart_isDisable = false;
    }

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    //QString style;
    int *active_ch;
    bool Standart_isDisable;
};

//-----------------------------------------------------------------------------
class STANDARTSSHARED_EXPORT Standarts: public QGroupBox
{
    Q_OBJECT

public:
    Standarts(QWidget *parent = NULL);
    virtual ~Standarts();


    void readCommonSettings();
    QTranslator translator;

    int active_ch;

    QSpinBox *count_Standarts;
    QSpinBox *count_Doubles;
    QComboBox *Units_Box;
    QLabel *label_count;
    QLabel *label_Doubles;
    QLabel *label_unit;

    QTableWidget *Standarts_Table;
    Standarts1ItemDelegate *standarts_Delegate;

    void Load_XML(QDomNode &node);
    void Set_ActiveCh(int);

    void Write_XML(QString &text);
    void Read_XML(QString);

private slots:
    void Change_CountStandarts(int);

};

#endif // STANDARTS_H
