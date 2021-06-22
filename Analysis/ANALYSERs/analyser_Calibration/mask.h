#ifndef MASK_H
#define MASK_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QGroupBox>
#include <QBoxLayout>
#include <QDir>
#include <QProgressBar>
#include <QLabel>
#include <QPainter>
#include <QMap>
#include <QImage>
#include <QVector>
#include <QTableWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QImage>
#include <QVector>
#include <QtMath>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QHeaderView>
#include <QTemporaryDir>
#include <QToolButton>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPoint>

#include <QDebug>

#include "protocol.h"
#include "utility.h"
#include "gsl/gsl_statistics_double.h"
//#include "define_PCR.h"

#define LEFT_OFFSET 66          // offset for 96,384
#define LEFT_OFFSET_DT48 7      // offset for 48,192
#define W_IMAGE 752             // width image
#define H_IMAGE 292             // heigth image
#define COEF_IMAGE 1.93         //
#define W_IMAGE_COEF 390        // width image (width/coef: 752/1.93)
#define W_REALIMAGE 825         //
#define H_REALIMAGE 312         //
#define TOP_OFFSET 5

#define fluor_NAME  {"fam","hex","rox","cy5","cy5.5"}
#define count_CH 5

class Image_Widget;
class Image_Obj;

//-----------------------------------------------------------------------------
class FluorVideo_ItemDelegate: public QStyledItemDelegate
{
Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

};
//-----------------------------------------------------------------------------

class ResultsItemDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QVector<short> *background;
    int count_ActiveCh;

};
//-----------------------------------------------------------------------------
class Mask: public QGroupBox
{
    Q_OBJECT
public:
    Mask(QWidget *parent = 0);
    ~Mask();


    //QDir            *Catalogue;
    QString         Catalogue;
    QGroupBox       *Control_Group;
    QGroupBox       *Image_Group;
    QGroupBox       *Result_Group;
    QImage          *Image;
    Image_Widget    *image_Widget;
    QVector<int>    *Video_BUF;
    QCheckBox       *view_mask;
    QLabel          *label_mask;
    QToolButton     *save_button;


    QTableWidget        *Table_Result;
    ResultsItemDelegate *results_Delegate;

    QComboBox               *Fluor_Box;
    FluorVideo_ItemDelegate *fluor_delegate;

    QPushButton     *prev_image;
    QPushButton     *next_image;
    //QPushButton     *Open_Dir;
    QProgressBar    *main_progress;
    QLabel          *PrBar_status;

    QMap<QString,QStringList*> map_FN;
    int type_dev;                       // 384,96,48,192
    QMap<int, QImage*> map_IMAGE;
    QMap<QString, QString> map_RESULTs;
    QMap<QString, QString> map_ATTENTION;
    QMap<int, QVector<QPoint>*> map_MASK;
    QMap<int, QVector<int>*> map_VALUE;

    bool OptMask_status;
    bool Exist_Mask;    
    QTemporaryDir *dir_temp;

    // borders:
    double border_MaskQuality;
    double border_DistanceChannels;
    double border_DifferenceDevice;

    Image_Obj           *Img;
    QVector<Image_Obj*> imgs;
    typedef struct
    {
        int min,max,sat,nmax,nmin;
    } ChanPar;
    ChanPar chanPar[count_CH];

    // methods:
    int Analyser(rt_Protocol *prot);
    void create_MapFN(int);
    void Draw_Image(QString fn, int type_dev);
    void Draw_Mask(QPainter*);
    void Display_ProgressBar(int percent, QString text);
    void LoadFluor(rt_Protocol *prot);
    void Fill_ResultsTable();
    void Clear_AllMaps();

    void Create_FilesImage();

    int Round(double);
    bool Validate_Mask(int ch, Image_Obj*);
    double Calculate_DeltaMask();
    bool Distance_RealCalcMask(rt_Protocol*, double&, double&);

    void SaveImageToProperty(int,int,int);
    void SaveRawDataToProperty();
    void Load_map_VALUE();

private:
    rt_Protocol     *Prot;

signals:

public slots:
    void Change_Image(int);
    void Save_Mask();
};

//-----------------------------------------------------------------------------
class Image_Widget: public QWidget
{
    Q_OBJECT

public:
    Image_Widget(QImage *i, QWidget *parent = 0);

protected:
        void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;

private:
        QImage          *image;
        Mask            *p_mask;
};

//-----------------------------------------------------------------------------
class Image_Obj: public QWidget
{
    Q_OBJECT

public:
    Image_Obj(int type = 384, QWidget *parent = 0);
    ~Image_Obj();

    int import_Data(int,int,int);
    void calcHist();
    void lookPrep();
    uint calcSpot(QPoint);
    int copy(Image_Obj*);
    int minus(Image_Obj*);
    int look4Blot(int n);
    uint look4LeftTop();
    int look4Right(uint n);
    uint look4Bottom(uint n);
    double dist(QPoint, QPoint);
    uint getNT()                    {return Dmx*Dmy;}
    void setSpot(uint n, QPoint p)  {Spot[n] = p;}
    int doSeq();
    void data2picI(QImage*);
    double recognition_quality();
    double getBlot0(int);


    int Dark, Bright, Contrast, Sat;

//private:

    uint Sx,Sy;
    uint Dmx,Dmy;
    uint Tot,Sum;
    uint Sqr;
    uint hData[256];

    QPoint Spot[385],sSpot[385];
    uint Blot[385];
    int sSeq[384];

    uint Data[H_IMAGE][W_IMAGE];
    uint pData[H_IMAGE][W_IMAGE];
    uchar Mask[H_IMAGE][W_IMAGE];

};

#endif // MASK_H
