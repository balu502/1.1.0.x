#ifndef EXPO_H
#define EXPO_H

#include "expo_global.h"

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
#include <QMessageBox>
#include <QMouseEvent>
#include <QPalette>
#include <QCheckBox>
#include <QMap>
#include <QStyle>
#include <QDesktopWidget>
#include <QEvent>
#include <QCloseEvent>
#include <QSettings>
#include <QTranslator>


#include <QDebug>

#include "request_dev.h"
#include "device_param.h"
#include "../../Analysis/RT_Analysis/define_PCR.h"


typedef HWND (__stdcall *create_3D)(int);
typedef void (__stdcall *destroy_3D)(HWND);
typedef void (__stdcall *show_3D)();
typedef void (__stdcall *plate_3D)(int);
typedef void (__stdcall *channel_3D)(int);
typedef void (__stdcall *data_3D)(quint16*,int);

//-----------------------------------------------------------------------------
class FluorExpo_ItemDelegate: public QStyledItemDelegate
{
Q_OBJECT

public:

    void paint (QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const Q_DECL_OVERRIDE;

    QString style;
};

//-----------------------------------------------------------------------------

class EXPOSHARED_EXPORT Expo: public QMainWindow
{
    Q_OBJECT

public:
    Expo(QWidget *parent = 0);
    ~Expo();

    int type_dev;           // count tubes: 96,384,48,192
    int active_channels;    // 1 bit on single channel (for first 4 channel: 0x0f)
    bool *flag_DeviceSettings;
    QVector<quint16> *expozition_Def;

    bool close_Cancel;

signals:
    void SavePar(int, QString);
    void MakeMeasure_Request(int,int,int);      // channel(0-4), expo(50-10000ms),ctrl(0-video,1-digits,2-all)
    void SaveParDefault(QString, QString);

private:
    QGroupBox   *Main_Box;
    QGroupBox   *main_Box;
    QGroupBox   *buttons_Box;    

    QGroupBox   *Image_Box;
    QGroupBox   *ExpoControl_Box;
    QPushButton *Make_Measure;
    QSpinBox    *Set_Expo;
    QComboBox   *Set_Fluor;
    FluorExpo_ItemDelegate *fluor_delegate;
    QLabel      *expo_TestCoefficient;

    QPushButton *SaveExpo_Button;
    QPushButton *Cancel_Button;
    QCheckBox   *SaveAsDefault;

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

    QMap<QString,QString> map_DeviceInfo;
    QMessageBox message;
    QVector<quint16> Digit;
    QVector<quint16> Expozition;    // 2 expt for channel: 2*CountMax = 2*8
    QVector<quint16> Expozition_initial;


    //... methods ...
    //void Treat_DigitBuf(QByteArray&, QVector<quint16>&);

public slots:
    void Load_Fluor();
    void Get_InfoDevice(QMap<QString,QString>*);
    void Load_lib(HINSTANCE);
    void Save_Parameters();
    void Change_Channel(int);
    void Get_Digit();
    void Get_Digitization(QMap<QString,QByteArray>*);
    void Change_Expo(int);
    void Change_AsDefault(bool);

    void Check_ExpoChange();

    void Set_ExpoTestCoeff();

    void Close_ByCancel();

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

};

#endif // EXPO_H
