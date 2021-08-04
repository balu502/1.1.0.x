#ifndef VIDEO_H
#define VIDEO_H

#include "video_global.h"

#include <QObject>
#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QSpinBox>
#include <QImage>
#include <QLabel>
#include <QFile>
#include <QByteArray>
#include <QVector>
#include <QRgb>
#include <QThread>
#include <QTabWidget>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPalette>
#include <QCheckBox>
#include <QMap>
#include <QStyle>
#include <QDesktopWidget>
#include <QEvent>
#include <QCloseEvent>
#include <QRadioButton>
#include <QSettings>
#include <QTranslator>
#include <QStandardPaths>
#include <QFileDialog>

#include <QDebug>

#include <protocol.h>
#include <utility.h>
#include "zoom.h"

//#include <plate_3d.h>

#include "request_dev.h"
#include "device_param.h"
#include "../../Analysis/RT_Analysis/define_PCR.h"


/*#define LEFT_OFFSET 66          // offset for 96,384
#define LEFT_OFFSET_DT48 7      // offset for 48,192
#define W_IMAGE 752             // width image
#define H_IMAGE 292             // heigth image
#define COEF_IMAGE 1.93         //
#define W_IMAGE_COEF 390        // width image (width/coef: 752/1.93)
#define W_REALIMAGE 825         //
#define H_REALIMAGE 312         //
#define TOP_OFFSET 5*/

/*#define LEFT_OFFSET 0          // offset for 96,384
#define LEFT_OFFSET_DT48 0      // offset for 48,192
#define W_IMAGE 640             // width image
#define H_IMAGE 480             // heigth image
#define COEF_IMAGE 1.00         //
#define W_IMAGE_COEF 640        // width image (width/coef: 752/1.93)
#define W_REALIMAGE 640         //
#define H_REALIMAGE 480         //
#define TOP_OFFSET 0*/

#define LABEL_96    "A1,A12,H1,H12"
#define LABEL_384   "A1,A24,P1,P24"
#define LABEL_48    "A1,A8,F1,F8"
#define LABEL_192   "A1,A16,L1,L16"

typedef HWND (__stdcall *create_3D)(int);
typedef void (__stdcall *destroy_3D)(HWND);
typedef void (__stdcall *show_3D)();
typedef void (__stdcall *plate_3D)(int);
typedef void (__stdcall *channel_3D)(int);
typedef void (__stdcall *data_3D)(quint16*,int);

class Image_Widget;

//-----------------------------------------------------------------------------
class FluorVideo_ItemDelegate: public QStyledItemDelegate
{
Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QString style;
};

//-----------------------------------------------------------------------------
class Corner: public QGroupBox
{
Q_OBJECT

public:
    Corner(int id, QWidget *parent = 0);
    ~Corner();

    QLabel      *name_corner;
    QSpinBox    *x_corner;
    QSpinBox    *y_corner;
    int         Index;      // 0-3 corner
    QWidget     *Parent;

    int         W_IMAGE;
    int         H_IMAGE;

signals:
    void sChange_XY(int);

public slots:
    void Change_center();


};

//-----------------------------------------------------------------------------
class Median: public QGroupBox
{
Q_OBJECT

public:
    Median(int id, QWidget *parent = 0);
    ~Median();

    QLabel      *id_image;
    QSpinBox    *x_median;
    QSpinBox    *y_median;
    int         Index;      // 0-3 median
    QWidget     *Parent;

signals:
    void sChange_Median(int);

public slots:
    void Change_Median();


};

//-----------------------------------------------------------------------------

class VIDEOSHARED_EXPORT Video : public QMainWindow
{
Q_OBJECT

public:
    Video(QWidget *parent = 0);
    ~Video();

    int type_dev;           // count tubes: 96,384,48,192
    int active_channels;    // 1 bit on single channel (for first 4 channel: 0x0f)

    bool *flag_DeviceSettings;

                // int ctrl(0-mask,1-exp,...), QString data

    // Video items...
    int LEFT_OFFSET;
    int LEFT_OFFSET_DT48;
    int W_IMAGE;
    int H_IMAGE;
    double COEF_IMAGE;
    int W_IMAGE_COEF;
    int W_REALIMAGE;
    int H_REALIMAGE;
    int TOP_OFFSET;

    double FHW;


protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public slots:
    void Get_Picture(QMap<QString,QByteArray>*);
    void Load_Fluor();
    void Clear_Image();
    void Get_InfoDevice(QMap<QString,QString>*, bool save = true);
    void Draw_Mask(QPainter*, int rep = -1);
    void Change_XY(int);
    void Change_Radius();
    void Return_To_Initial();
    void Save_Parameters();
    void Show_DigitPlate();
    void Load_lib(HINSTANCE);
    void Toggle_digit(bool);
    void Show_ZoomDialog();

signals:
    void MakeMeasure_Request(int,int,int);      // channel(0-4), expo(50-10000ms),ctrl(0-video,1-digits,2-all)
    void SavePar(int, QString);
    void WriteSector(int, QString);
    void ReadSector(int, QString);
    void SaveParMedian(QString, QString);

    void Load_3DAxis(int,int);                  // emit row and column
    void Load_3DValue(QVector<quint16>*, int);  // emit QVector<quint16>(data) and channel
    //void Clear_3DPlate();                       // emit clear command

public:
    QGroupBox   *Median_Box;

private:
    QGroupBox   *Main_Box;

    QGroupBox   *main_Box;
    QGroupBox   *buttons_Box;

    QGroupBox   *Video_Box;
    QGroupBox   *Control_Box;
    QGroupBox   *Additional_Box;

    QGroupBox   *Image_Box;
    QGroupBox   *VideoControl_Box;    
    QPushButton *Make_Measure;
    QToolButton *Load_AutoMask;
    QSpinBox    *Set_Expo;
    QImage      *Image;
    //QLabel      *Label_Image;
    Image_Widget *image_Widget;

    QTabWidget  *Preference_Box;
    QGroupBox   *Corner_Box;
    QGroupBox   *Radius_Box;

    QGroupBox   *Temp_Box;
    QPushButton *Read_Sector;
    QPushButton *Write_Sector;
    QGroupBox   *Control_subBox;
    QVector<Corner*> corners_Mask;
    QVector<Median*> median_Mask;
    QSpinBox    *Rx_spot;
    QSpinBox    *Ry_spot;
    QLabel      *Rxy_label;
    QCheckBox   *Paint_Mask;
    QPushButton *Return_ToInitial;
    QPushButton *Zoom;
    QPushButton *Digit_Plate;
    QRadioButton *DT_digit;
    QRadioButton *Soft_digit;

    QPushButton *SaveGeometry_Button;
    QPushButton *Cancel_Button;

    QComboBox   *Set_Fluor;
    FluorVideo_ItemDelegate *fluor_delegate;

    QMessageBox message;

    QTranslator translator;
    void readCommonSettings();
    QString StyleApp;

    //...
    HINSTANCE   dll_handle;
    HWND        handle_3D;
    create_3D   create_Bars;
    destroy_3D  destroy_Bars;
    show_3D     show_Bars;
    plate_3D    plate_Bars;
    channel_3D  channel_Bars;
    data_3D     data_Bars;
    //...

public:
    //...
    int zoom_h;
    int zoom_w;
    int zoom_tube;
    Zoom_Dialog *zoom;

    QVector<short> XY_initial;
    qint16 Rx_initial;
    qint16 Ry_initial;
    QVector<short> XY;
    qint16 Rx;
    qint16 Ry;
    QVector<qint8> offset_XY;

    QStringList list_Label;     // 4 label (A1,A12,H1,H12) for 96 dev
    QVector<short> rep_Point;
    int Rep;                    // {-1,0,...,3}
    bool drag_Mode;
    QVector<QPoint> XY_corner;  // 4 points(QPoint): corners
    QVector<QPoint> XY_median;  // 4 points(QPoint): medians
    QVector<QPoint> median_Point;   // calculated median parameters (4 points)
    QMap<QString,QString> map_DeviceInfo;

    QVector<quint16> Digit;         //
    QVector<quint16> Digit_soft;    //
    QVector<int> BUF_Video;         // video buffer

    //--- methods ---
    void Treat_VideoImage(QVector<ushort>&, QVector<int>&);
    void Draw_VideoImage(QVector<int>&);
    void Treat_DigitBuf(QByteArray&, QVector<quint16>&);

    void Recalculate_Geometry();
    void Change_Corner(QVector<short>&);
    void Calculate_SingleLength(QPoint, QPoint, int, short*);


    void Calculate_Median();
    void Recalculate_MedianPoint();
    void ReCalculate_MedianPoint(int num_point,
                                 int index_point,
                                 int id_0,
                                 int id_1,
                                 int col,
                                 int row,
                                 QPoint &point);
    void Recalculate_GeometryMedian();


    int Round(double);

    void Digitise_VideoImage(QVector<short>&,
                             qint16,
                             qint16,
                             QVector<qint8>&,
                             int,
                             int,
                             int,
                             QVector<int>&,
                             QVector<quint16>&);

    void Draw_ZoomVideoImage(int);

    void Check_GeometryChange();

private slots:
    void Get_Image();
    void Load_MASK();
    void Camera_Sound();
    void Change_MedianRejime();
    void Change_Median(int index);

    void slot_ReadSector();
    void slot_WriteSector();

};

//-----------------------------------------------------------------------------
class Image_Widget: public QWidget
{
    Q_OBJECT

public:
    Image_Widget(QImage *i, QWidget *parent = 0);

    double COEF_IMAGE;

protected:
        void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;
        void mousePressEvent(QMouseEvent*) Q_DECL_OVERRIDE;
        void mouseReleaseEvent(QMouseEvent*) Q_DECL_OVERRIDE;
        void mouseMoveEvent(QMouseEvent*) Q_DECL_OVERRIDE;

private:
        QImage  *image;
        Video   *p_video;

};

#endif // VIDEO_H
