#ifndef CROSSBOX_H
#define CROSSBOX_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QGroupBox>
#include <QTableWidget>
#include <QBoxLayout>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QHeaderView>
#include <QMap>
#include <QSplitter>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QTextCodec>

#include "protocol.h"
#include "define_PCR.h"
#include "point_takeoff.h"
#include "algorithm.h"
#include "utility.h"

//-----------------------------------------------------------------------------
class Cross_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    bool mc_rejime;
};
//-----------------------------------------------------------------------------
class CrossTest_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;
};
//-----------------------------------------------------------------------------
class CrossBox : public QGroupBox
{
    Q_OBJECT

public:
    CrossBox(QWidget *parent = NULL);
    virtual ~CrossBox();

    rt_Protocol     *prot;
    QString output_Mode;
    bool ru_Lang;
    QMap<QString,QString> *map_TestTranslate;

    QSplitter       *main_spl;
    QGroupBox       *box_Table;
    QGroupBox       *box_Test;
    QTableWidget    *CrossTable;
    Cross_ItemDelegate *Delegate;
    QTableWidget    *CrossTests;
    CrossTest_ItemDelegate *Delegate_Test;

    void Fill_CrossInfo(rt_Protocol*);

    QMap<int, QString>          Test_Num;
    QMap<QString, QStringList*> Test_Names;
    QMap<QString, int>          Test_Offset;

    QMap<int, QString>          Sample_Num;
    QMap<QString, QString>      Sample_Test;
    QMap<QString, QString>      Sample_Names;
    QMap<QString, QStringList*> Sample_Cp;

    //--- Actions -----------
    QAction *copy_to_clipboard;
    QAction *copy_to_excelfile;
    QAction *choice_Cp;
    QAction *choice_Ct;
    QAction *choice_Peaks;
    QAction *create_FileTempCorrection;
    QActionGroup *choice_rejime;

signals:
    void sSelect_Curve(int);

private slots:
    void contextMenu_CrossTable();
    void to_ClipBoard();
    void to_Excel();
    QString CrossTable_ToDigits();
    void reFill_CrossInfo(QAction*);
    void Create_FileCorrection();
    void SelelectCell_CrossTable(int,int);
};

#endif // CROSSBOX_H
