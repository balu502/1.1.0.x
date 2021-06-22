#ifndef ALL_CP_H
#define ALL_CP_H

#include <QtCore/QObject>
#include <QApplication>
#include <QWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QGroupBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QHeaderView>
#include <QDebug>
#include <QMessageBox>
#include <QPoint>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QSplitter>
#include <QBoxLayout>
#include <QTranslator>
#include <QSettings>
#include <QClipboard>
#include <QFileDialog>
#include <QFile>
#include <QVariant>
#include <QMessageBox>
#include <QAbstractButton>
#include <QTextEdit>
#include <QCheckBox>
#include <QTextCodec>
#include <QAxObject>

#include "all_cp_global.h"
#include "algorithm.h"
#include "protocol.h"
#include "alg_interface_qevent.h"
#include "alg_interface.h"
#include "utility.h"

#include "pcranalysis.h"
#include "crossbox.h"

#include "windows.h"

#define ID_LANG_RU	0x0419
#define ID_LANG_EN	0x0409

typedef void (__stdcall *TranslateService) (int Lang, char *source, char **target);


class Cp_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QString style;
};

class MC_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QString style;
};

class ALL_CPSHARED_EXPORT All_Cp: public QObject, public Alg_Interface
{        
    Q_OBJECT

public:

    All_Cp();


    void *Create_Win(void *, void *, void *);
    void Destroy_Win();
    void Show();
    void Destroy();
    void Analyser(rt_Protocol *p, int param = 0);
    void Select_Tube(int pos);
    void Enable_Tube(QVector<short>*);
    void *ReferenceForMap();

    QAxObject *ax_user;

    QMap<QString,QString> Map_TestTranslate;
    void Translate_Tests(QStringList&);         // translate names and research
    void Create_TranslateList(rt_Test*, QStringList&);
    HINSTANCE ext_dll_handle;                   // DTReport
    TranslateService translate_service;
    int ID_LANG;                                // lanquage for DTReport2.dll

private:

    rt_Protocol *p_prot;
    void Fill_Cp_Table();
    void Fill_MC_Table();


    void Validate_PCR();
    QMap<QString,QString> Map_Validity;
    void Validity_MC();

    void AddResult(vector<string>&, QString, QString);
    void Load_RelativeValidityStructure(QMap<QString,double>&, rt_Protocol*);

    QTabWidget *TabGrid;

    QTableWidget *Cp_Table;         // Cp
    Cp_ItemDelegate *Delegate;

    QTableWidget *MC_Table;         // Melting Curve
    MC_ItemDelegate *MC_Delegate;

    CrossBox    *Cross_Box;         // Cross Table
    PCRAnalysis *PCR_Analysis;      // PCR Analysis
    /*QGroupBox *PCR_Analysis;      // PCR Analysis
    QSplitter *main_spl;            // splitter between ChartBox and GridBox
    QSplitter *chart_spl;           // splitter between Charts
    QGroupBox *ChartBox;
    QGroupBox *GridBox;*/

    QTranslator translator;
    QTranslator qt_translator;
    void readCommonSettings();
    QString StyleApp;
    bool ru_Lang;

    Select_Plot event_plot;
    QWidget *main_widget;

    //--- flags ---
    bool flag_AddAnalysis;
    bool flag_CrossTable;

    //--- Actions -----------
    QAction *copy_to_clipboard;
    QAction *copy_to_excelfile;
    QAction *copy_to_clipboard_MC;
    QAction *copy_to_excelfile_MC;
    QAction *copy_Cp;

public slots:
    void Select_Curve(int);

private slots:
    void contextMenu_CpTable();
    void contextMenu_MCTable();
    void SelelectCell_CpTable(int row, int col);
    void SelelectCell_MCTable(int row, int col);
    void to_ClipBoard();
    void to_Cp();
    void to_Excel();
    void to_ClipBoard_MC();
    void to_Excel_MC();
    QString CpTable_ToDigits();
    QString MCTable_ToDigits();
    QString CpTable_Cp();
    bool Validate_Peaks(int pos, QPointF Param_peak, Criterion_Test* cri);

};

/*
extern "C" ALL_CPSHARED_EXPORT Alg_Interface* __stdcall create_plugin()
{
    return(new All_Cp());
}
*/


#endif // ALL_CP_H
