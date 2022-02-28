#ifndef ANALYSER_HRM_H
#define ANALYSER_HRM_H

#include <QtCore/QObject>
#include <QtCore/qmath.h>
#include <QObject>
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
#include <QMouseEvent>
#include <QFont>
#include <QFontMetrics>
#include <QSplitter>
#include <QBoxLayout>
#include <QTranslator>
#include <QSettings>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QMainWindow>
#include <QToolBar>
#include <QFileDialog>
#include <QTime>
#include <QDoubleValidator>
#include <QSpinBox>
#include <QClipboard>
#include <QFileDialog>
#include <QFile>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QTextCodec>
#include <QPointF>
#include <QSlider>
#include <QRadioButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QButtonGroup>
#include <QToolButton>
#include <QFileDialog>
#include <QScrollArea>
#include <QProcess>
#include <QColorDialog>

#include <QDomDocument>

#include <QDebug>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_zoomer.h"
#include "qwt_picker.h"
#include "qwt_plot_canvas.h"
#include "qwt_curve_fitter.h"
#include "qwt_plot_directpainter.h"
#include "qwt_legend.h"

#include <qwt_plot_shapeitem.h>
#include <qwt_plot_magnifier.h>
#include <qwt_legend.h>
#include <qwt_plot_renderer.h>

#include "analyser_hrm_global.h"
#include "alg_interface_qevent.h"
#include "analysis_interface.h"
#include "algorithm.h"
#include "utility.h"
#include "define_PCR.h"
#include "shapefactory.h"

#include <algorithm>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_math.h>


//-----------------------------------------------------------------------------
class Legend: public QGroupBox
{
Q_OBJECT

public:
    Legend(int id, QWidget *parent = 0);
    ~Legend();

    QLabel      *name_legend;
    QPushButton *color_legend;
    int         index;

signals:
    void sChange_color(int);

public slots:
    void Change_color();
};


//-----------------------------------------------------------------------------
class HrmPlot: public QwtPlot
{
    Q_OBJECT

public:
    HrmPlot(bool visible_axis, bool visible_AxisTitle, QColor bg, QString titul, QWidget *parent = NULL);
    virtual ~HrmPlot();

    QList<QwtPlotCurve*> list_Curves;
    QwtPlotGrid     *grid;
    QLabel          *label_caption;
    rt_Protocol     *Prot;
    int             active_ch;
    QVector<int>    group_Samples;
    QVector<short>  enable_Samples;

    QAction *save_as_PNG;
    QAction *copy_to_clipboard_as_image;

    //void Create_Curves();
    void Delete_Curves();
    //void Clear_Curves();

    //... qwtplot copy image ...
    QGroupBox *box_image;
    QwtPlot *plot_image;
    QwtPlotGrid *grid_image;
    QwtPlotCurve *curve_image;
    QwtPlotShapeItem *shape_item;
    QVector<QwtPlotCurve*> listCurve_image;
    QVector<QwtPlotShapeItem*> listShape_image;
    //...
    QVector<QwtPlotShapeItem*> *Shape_Cluster;

protected:
    //virtual bool eventFilter(QObject *, QEvent *);

private slots:
    void contextMenu_Plot();
    void to_Image();
    void to_ImageClipBoard();
    //void to_ImageClipBoard_Clusters();

};
//-----------------------------------------------------------------------------
class HISTORY:  public QObject
{
    Q_OBJECT

public:
    HISTORY(QWidget *parent = NULL);

    int Param_Clustering;
    int depth_clustering;
    int count_clustering;
    int Temperature_Bordering;
    double left_border;
    double right_border;
    bool Temperature_Correction;
};

//-----------------------------------------------------------------------------
class CLUSTER:  public QObject
{
    Q_OBJECT

public:
    CLUSTER(QWidget *parent = NULL);
    virtual ~CLUSTER();

    QVector<int> curves;
    //QVector<QVector<double>*> XY;   // arrays for each curve
    //QVector<double> XY_mass;        // array for mass_centre_cluster

    QVector<QPointF*> XY_points;    // array for each curve as f(x,y) (look on Cluster_Plot)
    QPointF XY_cluster;             // mass_centre for cluster

    QColor color;

};

//-----------------------------------------------------------------------------
class CURVE_RESULTS:  public QWidget
{
    Q_OBJECT

public:
    CURVE_RESULTS(QWidget *parent = NULL);
    virtual ~CURVE_RESULTS();

    QString Identificator;
    int     pos;
    double  Tpeaks;
    int     Group;
    QColor  color;
    bool    enable;
    double  Percent_Clustering;
    //QString G_type;

    rt_Sample *sample;
};
//-----------------------------------------------------------------------------

class Samples_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QMap<int, short> *map_reference;
    int *border_quality;
};
//-----------------------------------------------------------------------------

class Groups_ItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor,
                      QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;

public slots:
    void Close_LineEditor();

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
    QMap<int, CURVE_RESULTS*> *Curve_Results;
    QMap<int, short> *map_reference;
};

//-----------------------------------------------------------------------------
class TableWidget : public QTableWidget
{
    Q_OBJECT

public:
        TableWidget(int rows, int cols, QWidget* parent = 0);
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

        QAction         *reference_rejime;
        QAction         *clear_AllReference;

        QButtonGroup* color_buttons;
        QMap<int, CURVE_RESULTS*> *Curve_Results;

private slots:
        //void change_SelectRejime(QAction*);

protected:
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void paintEvent(QPaintEvent *e);
        void resizeEvent(QResizeEvent *event);

signals:
        void sReanalysis();

};

//-----------------------------------------------------------------------------

class ANALYSER_HRMSHARED_EXPORT Analyser_HRM: public QObject, public Analysis_Interface
{
    Q_OBJECT

public:
    Analyser_HRM();

    void *Create_Win(void *, void *);
    void Destroy_Win();
    void Show();
    void Destroy();

    void GetInformation(QVector<QString> *info);
    void Analyser(rt_Protocol *prot);
    void Save_Results(char*);
    void Select_Tube(int pos);
    void Enable_Tube(QVector<short>*);

    void readCommonSettings();
    QTranslator translator;
    QString StyleApp;
    bool ru_Lang;

    bool first_cycle;
    int act_Channel;
    QString file_Correction;
    int Border_Quality;
    double Cp_Validity;

private:
    rt_Protocol     *Prot;
    QGroupBox       *MainBox;
    QWidget         *main_widget;

    QSplitter       *main_spl;
    QSplitter       *graph_spl;
    QSplitter       *control_spl;
    QTabWidget      *Tab_Graph;
    QGroupBox       *Graph_Claster;
    QTabWidget      *Tab_Control;
    QTabWidget      *Tab_Result;
    QGroupBox       *Control_Box;
    QScrollArea     *Scroll_Box;

    //... Main_Control ...
    QGroupBox       *Select_Group;
    QComboBox       *Groups;
    QLabel          *Label_Groups;
    QComboBox       *Fluors;
    QLabel          *Label_Fluors;
    QToolBar        *Control_Tool;

    QAction         *Numerate_tool;

    QGroupBox       *Control_Group;

    //... Control ...    
    QGroupBox       *ClusterParameters_Box;
    QLabel          *Parameters_Titul;
    QRadioButton    *Auto_clustering;
    QRadioButton    *Depth_clustering;
    QRadioButton    *Count_clustering;
    QRadioButton    *Reference_clustering;
    QButtonGroup    *Group_clustering;
    QSpinBox        *Count_Clusters;
    QSlider         *Depth_Clusters;
    QLabel          *ValueDepth_Percent;
    TableWidget     *Select_Grid;
    ItemDelegate    *Delegate;

    QGroupBox       *Temperature_Box;
    QLabel          *Temperature_Titul;
    QRadioButton    *Auto_Temperature;
    QRadioButton    *Manual_Temperature;
    QButtonGroup    *Group_Temperature;
    QDoubleSpinBox  *Left_Border;
    QDoubleSpinBox  *Right_Border;

    QGroupBox       *TemperatureCorrect_Box;
    QLabel          *TemperatureCorrect_Titul;
    QCheckBox       *Use_TemperatureCorrect;
    QToolButton     *Open_FileCorrect;
    QLabel          *Label_FileCorrect;
    QToolButton     *Surface_3D;

    QGroupBox       *TemperatureNorm_Box;
    QLabel          *TemperatureNorm_Titul;
    QCheckBox       *Use_TemperatureNorm;

    //--- Color Legend ------
    QScrollArea     *Scroll_Legend;
    QGroupBox       *Legend_Box;
    QGroupBox       *ButtonsColor_Box;
    QGroupBox       *ControlColor_Box;
    QList<Legend*>  List_Legend;
    QPushButton     *Default_legend;
    QPushButton     *Apply_legend;

    //--- Actions -----------
    QAction *copy_to_clipboard;
    QAction *copy_to_excelfile;

    //... Cluster Plot attributes ...
    QLabel          *Label_CountClusters;
    QLabel          *Label_ResultClustering;
    QLabel          *Label_AxisY;
    QLabel          *Label_AxisX;

    //... Results ...
    QTableWidget            *Sample_Results;
    Samples_ItemDelegate    *Samples_Delegate;
    QTableWidget            *Groups_Results;
    Groups_ItemDelegate     *Groups_Delegate;
    QTableWidget            *active_Table;

    QMap<int,short> HRM_Reference;
    HISTORY *current_History;
    QMap<QString, HISTORY*> HRM_HISTORY;
    QMap<QString, QVector<int>*> HRM_GROUPs;
    QVector<int> HRM_Group;
    QVector<int> *hrm;
    QMap<int, CURVE_RESULTS*> Curve_Results;
    QVector<double> X_Temp, X_Norm, X_Centered;
    QVector<QVector<double>*> Y_Norm, DY_Norm, DY_Centered, Diff_Norm;
    //QVector<QVector<QPointF>*> DY_Cluster, DYmass_Cluster;
    QVector<QPointF*> XY_Points;
    QVector<QwtPlotShapeItem*> Shape_Cluster;
    QVector<QwtPlotCurve*> Center_Cluster;
    QMap<int, double> Silhouette_Quality;
    QMap<int, double> InnerDistance_Quality;
    QVector<double> temperature_Correction;
    QVector<double> Tpeaks;

    QVector<QColor> Color_Cluster;
    QVector<double> Curve_NULL;

    QVector<CLUSTER*> List_CLUSTERs;
    void Clear_Clusters();
    void Create_Cluster();

    HrmPlot *Hrm_RawData;
    HrmPlot *Hrm_dFdT;
    HrmPlot *Hrm_RawData_Border;
    HrmPlot *Hrm_Norm;
    HrmPlot *Hrm_DY_Norm;
    HrmPlot *Hrm_DY_Centered;
    HrmPlot *Hrm_Diff;
    HrmPlot *Cluster_Plot;

    QwtPlotCurve    *curve_BorderLeft;
    QwtPlotCurve    *curve_BorderRight;

    void Analysis_Group(QVector<int>*);
    void FinishAuto_Analysis();
    void Cluster_Analysis(QVector<CLUSTER*>&, int num_cluster = -1);
    void Cluster_ReferenceAnalysis(QVector<CLUSTER*>&);
    void Distance_Matrix(QVector<CLUSTER*>&, QVector<QVector<double>*>*, QPoint*, QPoint*);
    void Draw_ClusterMass(QVector<CLUSTER*>&);
    void Clear_ClusterMass();
    void ChangeColor_Curve(HrmPlot*, QVector<int>, QColor);
    double Distance(QVector<double>*,QVector<double>*);
    double Distance_DM(QVector<double>*,QVector<double>*);
    double Distance_Points(QPointF,QPointF);
    double Accumulate(QVector<double>*);
    double Calculate_ChangeDirection(QVector<double>*);
    void Normalization_Vector(QVector<double>*);
    void Calculate_Angle(QVector<QVector<double>*>*, QVector<double>*);
    void Calculate_Diff(QVector<QVector<double>*>*, QVector<double>*);
    void Calc_Atan2(QVector<double>*, QVector<double>*);
    int Find_MinArea(QVector<QVector<double>*>*);

    void Fill_SampleResults();
    void Fill_GroupResults();
    int Quality_Clustering(QVector<CLUSTER*>&, QMap<int, CURVE_RESULTS*>&);
    //void InnerDistance_Clustering(QVector<CLUSTER*>&, QMap<int, double>&);
    int ScoreFunction_Quality(QVector<CLUSTER*>&);

    void Load_SelectGrid(QVector<int>*);

    void Temperature_Correction(QVector<int>*, QVector<double>*, QVector<QVector<double>*>*, QVector<double>*, double);
    void Interpolation_steffen(QVector<double>*, QVector<double>*, double);
    void Temperature_Centered(QVector<double>*, QVector<QVector<double>*>*, QVector<double>*);

    void AddResult(vector<string>&, QString, QString);

public slots:
    void Gtype(int, int);

private slots:
    void slot_ChangeParamCluster();
    void slot_ChangeRejimeClustering(int);
    void slot_ChangeTemperatureBorders(int);
    void slot_SampleResults_clicked(int,int);
    void contextMenu_SampleResults();
    void contextMenu_GroupsResults();
    void slot_reAnalysis();
    void slot_ChangeGroup();
    void slot_ChangeSelectGrid();
    void slot_OpenFileCorrection();
    void contextMenu_SelectGrid();
    void change_Reference();
    void clear_ALLReferences();
    void Use_TempCorrection(bool);
    void Use_TempNormalization(bool);

    void View_3D();
    void Numerate_Plate();

    void to_ClipBoard();
    void to_Excel();
    QString Table_ToDigits(QTableWidget*);

    void Init_Colors();
    void Default_Colors();
    void Apply_Colors();
    void Check_ChangeColor(int);
};

#endif // ANALYSER_HRM_H
