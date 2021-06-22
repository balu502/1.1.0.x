#ifndef COMMON_H
#define COMMON_H

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

#include <QDebug>

#include "protocol.h"
#include "define_PCR.h"
#include "utility.h"

#include "windows.h"
#include "common_global.h"

//-----------------------------------------------------------------------------
class CommonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CommonDelegate(QObject *parent=0) : QStyledItemDelegate(parent)
    {
        //connect(this, SIGNAL(showComboPopup(QComboBox *)), this, SLOT(openComboPopup(QComboBox *)), Qt::QueuedConnection);
        Item1_isDisable = false;
    }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //void setEditorData(QWidget *editor, const QModelIndex &index) const;
    //void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    bool Item1_isDisable;

signals:
   //void showComboPopup(QComboBox *) const;

public slots:
    //void Close_Editor(int);
    //void openComboPopup(QComboBox *);

};
//-----------------------------------------------------------------------------
class Fluor_Channel : QComboBox
{
    Q_OBJECT

public:
    Fluor_Channel(int index, QWidget* parent = 0);
    virtual ~Fluor_Channel();

};

//-----------------------------------------------------------------------------
class Edit_Button : QPushButton
{
    Q_OBJECT

public:
    Edit_Button(QString str, QWidget* parent = 0):  QPushButton(str, parent)
    {
        setFixedSize(25,18);
        setToolTip(tr("edit"));
    }
};

//-----------------------------------------------------------------------------

class COMMONSHARED_EXPORT Common: public QTreeWidget
{
    Q_OBJECT

public:
    Common(QWidget *parent = NULL);
    virtual ~Common();

    void Load_XML(QDomNode &node);
    void Fill_Properties(rt_Test *ptest);

    void Write_XML(QString &text);
    void Read_XML(QString);

    void readCommonSettings();
    QTranslator translator;
    QMessageBox message;

public:
    CommonDelegate *common_Delegate;
    int active_channel;

private:

    Edit_Button *edit_CriterionPosResult;
    Edit_Button *edit_CriterionValidity;
    Edit_Button *edit_CriterionAFF;
    Edit_Button *edit_MinBorderCriterionAFF;
    Edit_Button *edit_MaxBorderCriterionAFF;
    Edit_Button *edit_MinBorderSigmoidValidity;
    Edit_Button *edit_MaxBorderSigmoidValidity;
    Edit_Button *edit_ThresholdValue;
    Edit_Button *edit_0_expo;
    Edit_Button *edit_1_expo;
    Edit_Button *edit_2_expo;
    Edit_Button *edit_3_expo;
    Edit_Button *edit_4_expo;

    QComboBox   *Fluor_0;

    QPushButton *back_Default;

private slots:

    void CheckValid_Value(QTreeWidgetItem*,int);
    void Edit_Show();
    void Set_DefaultValues();
};

#endif // COMMON_H
