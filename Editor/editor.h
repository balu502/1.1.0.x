#ifndef EDITOR_H
#define EDITOR_H

#include "editor_global.h"

#include <QObject>
#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QSplitter>
#include <QVector>
#include <QColor>
#include <QStringList>
#include <QPen>
#include <QFontMetrics>
#include <QDateTime>
#include <QImage>
#include <QPalette>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QItemDelegate>
#include <QTableWidget>
#include <QString>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QFont>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QString>
#include <QAbstractButton>
#include <QPalette>
#include <QKeyEvent>
#include <QRegExp>

#include <QDebug>

#include <string>
#include <math.h>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"
#include "qwt_picker.h"
#include "qwt_plot_canvas.h"
#include "qwt_curve_fitter.h"

#include "gradient.h"
#include "incr_temperature.h"

#define TR_PRG_BLK 8        // max count of blocks
#define TR_PRG_LEV 32       // max count of level in the program
#define TR_BLK_LEV 16       // max count of level in the block
#define MAX_LEV_TIME 28800  // max leng of level: sec (8 hours)
#define MAX_BLK_REP 10000   // max count of cycles in block

//-----------------------------------------------------------------------------

enum block_Type {bUnknown, bCycle, bMelt, bPause, bHold};


//-----------------------------------------------------------------------------

class Block_Pr
{

public:
    QVector<short>  Temperature;
    QVector<short>  Time;
    QVector<short>  Measure;
    int             Num_Cycle;
    block_Type      Type_Block;
    QVector<short>  Incr_Temp;
    QVector<short>  Incr_Time;
    QVector<short>  Gradient;
    QVector<QString> Temperature_Gradient;

    QVector<double> Coordinates_Level;

    Block_Pr()
    {
        Temperature.reserve(TR_BLK_LEV);
        Time.reserve(TR_BLK_LEV);
        Measure.reserve(TR_BLK_LEV);
        Num_Cycle = 0;
        Type_Block = bCycle;
        Incr_Temp.reserve(TR_BLK_LEV);
        Incr_Time.reserve(TR_BLK_LEV);
        Gradient.reserve(TR_BLK_LEV);
        Temperature_Gradient.reserve(TR_BLK_LEV);
        Coordinates_Level.reserve(TR_BLK_LEV*2);
    }
    ~Block_Pr()
    {
        Temperature.clear();
        Time.clear();
        Measure.clear();
        Incr_Temp.clear();
        Incr_Time.clear();
        Coordinates_Level.clear();
        Gradient.clear();
        Temperature_Gradient.clear();
    }
};


//-----------------------------------------------------------------------------

class ColorCurve: public QwtPlotCurve
{

public:
    ColorCurve(QWidget *parent = NULL){Blocks = NULL;}
    virtual ~ColorCurve() {}
    QVector<QColor> color_lev;
    QVector<Block_Pr*> *Blocks;
    QVector<QwtPlotCurve*> *curves_border;

    gradient_Type   *gradient;

    virtual void drawLines( QPainter *painter,
                            const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                            const QRectF &canvasRect, int from, int to ) const;

};

//-----------------------------------------------------------------------------

class Plot_ProgramAmpl: public QwtPlot
{
    Q_OBJECT

public:
    Plot_ProgramAmpl(QWidget *parent = NULL);
    virtual ~Plot_ProgramAmpl();

    ColorCurve      *curve;
    QwtPlotGrid     *grid;
    QLabel          *name_program;
    QLabel          *param_program;

    //gradient_Type   *gradient;

    QVector<QwtPlotCurve*> Curves_Border;
    void Delete_Curves_Border();

public slots:
    void clear_Program(void);
    void draw_Program(QString);
};

//-----------------------------------------------------------------------------

class EditorItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void paint(QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QVector<short> Type;

};

//-----------------------------------------------------------------------------

class TimeEditorDelegate: public QItemDelegate
{
    Q_OBJECT

 public:
    TimeEditorDelegate(QObject *parent = 0)
    {

    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;

    QVector<short> *Type;
};

//-----------------------------------------------------------------------------

class ComboBoxDelegate : public QItemDelegate
 {
    Q_OBJECT

 public:

    ComboBoxDelegate(QObject *parent = 0)
    {
        connect(this, SIGNAL(showComboPopup(QComboBox *)), this, SLOT(openComboPopup(QComboBox *)), Qt::QueuedConnection);
    }


    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
    {        
        QComboBox *editor = new QComboBox(parent);
        editor->addItem(tr("Off"));
        editor->addItem(tr("On"));
        editor->setStyleSheet("selection-background-color: #d7d7ff; selection-color: black; background: white;");

        connect(editor,SIGNAL(currentIndexChanged(int)), this, SLOT(Close_Editor(int)));

        emit showComboPopup(editor);

        return(editor);
    }


    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const
    {
        bool ok;
        QString text = index.model()->data(index, Qt::EditRole).toString();
        int value = text.toInt(&ok);

        QComboBox *cBox = static_cast<QComboBox*>(editor);
        cBox->blockSignals(true);
        switch(value)
        {
        case 0:     cBox->setCurrentIndex(0);   break;
        default:    cBox->setCurrentIndex(1);   break;
        }
        cBox->setCurrentIndex(-1);
        cBox->blockSignals(false);
    }

    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const
    {
        QComboBox *cBox = static_cast<QComboBox*>(editor);
        QString text = "0";

        if(cBox->currentIndex()) text = "1";
        model->setData(index, text, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
    {
        editor->setGeometry(option.rect);
    }

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QVector<short> *Type;

signals:
   void showComboPopup(QComboBox *) const;

public slots:
    void Close_Editor(int);
    void openComboPopup(QComboBox *);

};

//-----------------------------------------------------------------------------

class CEditLineButton : public QFrame
{
    Q_OBJECT
public:
    explicit CEditLineButton(QWidget *parent = 0);
    ~CEditLineButton();
    void setTextToEditLine(QString text);

signals:
    void clickButton();

public slots:

private slots:
    void clicked();

public:
    QLineEdit *lineEdit;
    QToolButton *button;
    QString Param;

private:
    QHBoxLayout *layout;
};

//-----------------------------------------------------------------------------

class DelegatPerson : public QItemDelegate
{
    Q_OBJECT

public:
    DelegatPerson(QObject *parent = 0)
    {

    }


    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem& option,
                       const QModelIndex& index) const;

    QVector<short>  *Type;
    int             current_Rejime;        // col == 5 (Gradient)  col == 6 (Incr_Time) col == 7 (Incr_Temperature)


public slots:
    void show_Dialog();

signals:
    void signal_3D();
    void signal_IncrTemp();

};

//-----------------------------------------------------------------------------

class EDITORSHARED_EXPORT Editor : public QDialog
{
    Q_OBJECT

public:
    Editor(QWidget *parent = 0);
    ~Editor();

    QMessageBox msgBox;    
    Plot_ProgramAmpl *plot_Program;
    QTableWidget *Editor_Table;

    QToolBar *ToolBar;

    EditorItemDelegate *Editor_Delegate;
    ComboBoxDelegate   *ComboBox_Delegate;
    DelegatPerson      *Button_Delegate;
    TimeEditorDelegate *Time_Delegate;

    // In_Source,Out_Source
    QString In_Source, Out_Source;


    // methods:
    void Text_To_Block(QString);                // converting text(string of Programm) to QVector<Block_Pr*>
    QString Block_To_Text();                    // converting blocks to text
    void DrawBlock_inGraph();                   // draw Graph (from Block_Pr)
    void DrawBlock_inTable();                   // draw Table (from Block_Pr)

    void createActions();                       // create actions

    bool Check_Validation();                    // validation
    void Check_MeltingBlock();                  // validation and change melting blocks

private:
    QDialogButtonBox *box;
    QPushButton      *Save_button;
    QPushButton      *Cancel_button;
    QGroupBox *main_Box;
    QGroupBox *graph_Box;
    QGroupBox *create_Box;
    QGroupBox *control_Box;

    QLabel      *label_Name;
    QLineEdit   *Name_Program;
    QLabel      *label_Volume;
    QSpinBox    *Volume_Tube;
    QHBoxLayout *name_layout;
    QHBoxLayout *volume_layout;


    QPushButton *delete_Block;
    QPushButton *delete_Level;
    QPushButton *add_Block;
    QPushButton *add_Level;
    QComboBox   *type_Block;

    QSplitter *main_spl;
    QSplitter *control_spl;

    QVector<Block_Pr*> Blocks;
    QString Name_Pr;
    short Volume_Pr;
    gradient_Type Gradient_Type;
    Gradient_INFO Gradient_Info;
    IncrTemperature_INFO *IncrTemperature_Info;

    Gradient *Gradient_Widget;      // 3d Gradient
    HINSTANCE dll_3D;
    Incr_Temperature *Incr_Temp;    // Increment temperature

    // Actions
    QAction *new_program;
    QAction *open_program;
    QAction *save_program;

    void readCommonSettings();
    QTranslator translator;
    QTranslator translator_sys;
    QString StyleApp;

private slots:
    void Value_Changed(int,int);                // value of cell is changed
    void Selection_Changed(int,int,int,int);    // selection changed
    void get_select(int,int);
    void Change_TypeBlock(int);

    void add_block();           // add block
    void add_level();           // add level
    void delete_block();        // delete block
    void delete_level();        // delete level

    void Name_Changed(QString);
    void Volume_Changed(int);

    void New_Program();         // new program
    void Save_Program();        // save program in file(*.rta)
    void Open_Program();        // open program in protocol(*.rt) or file(*.rta)
    void Accept_withValidation();

    void show_Gradient();
    void show_IncrTemperature();

signals:
    void put_select(int,int);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

};

#endif // EDITOR_H
