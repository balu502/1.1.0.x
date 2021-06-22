#ifndef SELECT_TUBES_H
#define SELECT_TUBES_H

#include <QDialog>
#include <QBoxLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QStyledItemDelegate>
#include <QPalette>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QString>
#include <QChar>
#include <QHeaderView>
#include <QBitArray>
#include <QDebug>
#include <QMessageBox>
#include <QEvent>
#include <QApplication>
#include <QAbstractButton>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QTabWidget>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QPushButton>
#include <QToolButton>
#include <QDockWidget>
#include <QTranslator>
#include <QFont>
#include <QHeaderView>
#include <QIcon>
#include <QEventLoop>
#include <QTimer>
#include <QTranslator>
#include <QSettings>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QColorDialog>
#include <QColor>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QLine>

#include "dialogs_global.h"
#include "protocol.h"
#include "define_PCR.h"

#include "dockwidgettitlebar.h"

//-----------------------------------------------------------------------------
/*class MyHorizontalHeaderView: public QHeaderView
{
    Q_OBJECT

public:
    MyHorizontalHeaderView(Qt::Orientation orientation, QWidget *parent = 0)
        : QHeaderView(orientation, parent){}

    bool selected;
    bool moving;
    QVector<int> Column_Selected;

protected:
    void mouseReleaseEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintSection(QPainter * painter, const QRect & rect, int logicalIndex);
    {
        if(selected && moving)
        {
            //QPainter painter_section(viewport());

            //painter_section.fillRect(viewport()->rect(), QColor(255,255,225));
        }

    }

};*/
//-----------------------------------------------------------------------------

class MyHeader: public QHeaderView
{
    Q_OBJECT

public:

        MyHeader(Qt::Orientation orientation, QWidget *parent = 0)
            : QHeaderView(orientation, parent) {selected = false;}

        bool selected;
        bool moving;
        QVector<int> Section_Selected;

protected:

        void mouseReleaseEvent(QMouseEvent* event);
        void mousePressEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void paintSection(QPainter * painter, const QRect & rect, int logicalIndex) const;

signals:
        void sSectionsSelect(QVector<int>);
};
//-----------------------------------------------------------------------------

class Color_ToolButton: public QToolButton
{
    Q_OBJECT

public:

    Color_ToolButton(QColor col, QWidget* parent = 0)
            : QToolButton(parent)   {color = col;}

    QColor color;

protected:

    //virtual void paintEvent(QPaintEvent* e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
};


//-----------------------------------------------------------------------------

class TableWidget : public QTableWidget
{
    Q_OBJECT

public:
        TableWidget(int rows, int cols, QWidget* parent = 0);//: QTableWidget(rows, cols, parent);
        ~TableWidget();

        bool selected;
        bool moving;
        QPoint point_0, point_1;
        QPoint coord_0, coord_1;

        void *address;
        QAbstractButton* corner_btn;
        QRect A1;
        QCursor         cursor_ColorFill;

        QActionGroup    *select_rejime;
        QAction         *position_select;
        QAction         *color_select;
        QAction         *clear_plate;
        QAction         *back_InitialState;
        QAction         *numeration_rejime;

        DockWidgetTitleBar* mTitleBar;
        QButtonGroup* color_buttons;

        QMap<int,int> map_ColorHistory;
        QToolButton *color_history;

        QVector<int>   *ColorTube_Current;


private slots:
        void change_SelectRejime(QAction*);

protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void paintEvent(QPaintEvent *e);
        void resizeEvent(QResizeEvent *event);
signals:
    void color_Rejime(bool);

};
//-----------------------------------------------------------------------------

class ItemDelegate: public QStyledItemDelegate
{
public:
    ItemDelegate()
    {                
    }

    rt_Protocol *p_prot;

    void get_prot(rt_Protocol *p) {p_prot = p;}
    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QString style;
    QRect *A1;
    bool numeration;
    QVector<short> *enable_initial;
};

//-----------------------------------------------------------------------------

class EnableTubes: public QEvent
{
public:
    EnableTubes(): QEvent((Type)1100)
    {
    }

    QVector<short> enable_tubes;
};

//-----------------------------------------------------------------------------

class ActiveSamples: public QEvent
{
public:
    ActiveSamples(): QEvent((Type)1101)
    {
    }

    QString samples;
};


//-----------------------------------------------------------------------------

class TreeWidget: public QTreeWidget
{
        Q_OBJECT
public:
    TreeWidget()
    {
    }
    //void mousePressEvent(QMouseEvent *event);
    //void Recheck_TreeItems();

//private slots:
    void click_item(QTreeWidgetItem* item, int col);


};

//-----------------------------------------------------------------------------


class DIALOGSSHARED_EXPORT Select_tubes: public QDockWidget
{
    Q_OBJECT
    friend class DockWidgetTitleBar;    

public:
    Select_tubes(const QString& title, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~Select_tubes();

    //void create_GridBox();
    void create_TabBox();
    void create_ColorButtons();
    void load_SelectGrid(rt_Protocol *p);
    void clear_SelectGrid();
    bool check_Mask();

    void Fill_Tests();
    void Fill_Samples();

    void PositionInProtocol(rt_Protocol *p);
    void Previous_EnableTubes(QVector<short>*);

    QIcon logo;
    QIcon logonull;

    QMap<QString,QString> *map_TestTranslate;

private slots:

    void cellSelected(int nRow, int nCol);
    void columnSelected(int col);
    void rowSelected(int row);
    void columnMoved(int col);
    void rowMoved(int row);
    void all_Selected();
    void apply_change();
    void selectALL_Test();
    void selectALL_Sample();
    void selectNONE_Test();
    void selectNONE_Sample();

    void start_ColumnSelect(int);
    void add_ColumnSelect(int);

    void contextMenu_SelectGrid();
    void ClearPlate();
    void InitialState();

    void tree_apply_change(QTreeWidgetItem* item, int col);

    void ColorHistory();

    void ColumnsSelect(QVector<int>);
    void RowsSelect(QVector<int>);

private slots:

    void change_dock(Qt::DockWidgetArea area)
    {
        dock_area = area;
    }

protected:    

    //--- Reimplemented ---
    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent *e);

private:

    QVBoxLayout *main_layout;
    QVBoxLayout *grid_layout;
    QVBoxLayout *layout_box;
    QHBoxLayout *layout_select;

    QSplitter *box_spl;            // splitter between Test and Sample boxes

    //QGroupBox *OptionsBox;

    ItemDelegate *Delegate;


    QVector<short> mask_Tests;
    QVector<short> mask_Samples;

    /*QPushButton *checkedALL_Test;
    QPushButton *checkedNONE_Test;
    QPushButton *checkedALL_Sample;
    QPushButton *checkedNONE_Sample;*/
    Color_ToolButton    *first_ColorButton;
    Color_ToolButton    *second_ColorButton;
    Color_ToolButton    *third_ColorButton;
    Color_ToolButton    *fourth_ColorButton;
    Color_ToolButton    *fifth_ColorButton;
    Color_ToolButton    *sixth_ColorButton;
    Color_ToolButton    *seventh_ColorButton;

    QToolButton         *color_History;

public:
    QButtonGroup *Fluor_Buttons;

    QTranslator translator;
    void readCommonSettings();
    QString StyleApp;

    //Color_Button    *first_ColorButton;

public:

    QWidget *main_widget;
    DockWidgetTitleBar* mTitleBar;
    Qt::DockWidgetArea dock_area;

    QVector<short> Protocol_ActiveTubes;
    QVector<short> EnableTube_Initial;
    QVector<int>   ColorTube_Initial;
    QVector<int>   ColorTube_Current;

    QVector<QColor> Color_Buttons;

    //QVector<int> Column_Selected;
    QCursor             cursor_FREE;


    TreeWidget *Test_tree;
    TreeWidget *Sample_tree;

    QTabWidget *Tab_Select;
    QGroupBox *SampleTest_Box;
    QGroupBox *sample_Box;
    QGroupBox *test_Box;
    TableWidget *Select_Grid;
    QGroupBox *Color_Box;
    QGroupBox *Select_Box;
    //MyHorizontalHeaderView *Horizontal_Header;
    rt_Protocol *p_prot;
    EnableTubes enable_event;
    ActiveSamples samples_event;
    QVector<int> ID_Methods;
    MyHeader *header_hor;
    MyHeader *header_ver;

private slots:
    void slot_FluorButton(int);
    void slot_ColorRejime(bool);
    void slot_SelectChange();
    void slot_NumerateChange();

    void slot_HideTabWidget();

public slots:
    void restore_Cursor();


};

#endif // SELECT_TUBES_H
