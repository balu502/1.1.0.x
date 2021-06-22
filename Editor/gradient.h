#ifndef GRADIENT_H
#define GRADIENT_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QBoxLayout>
#include <QGroupBox>
#include <QEvent>
#include <QCloseEvent>
#include <QStyleFactory>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QStringList>
#include <QTabWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QColor>
#include <QVector>
#include <QTableWidget>
#include <QHeaderView>
#include <QFont>
#include <QApplication>
#include <QDebug>

#include "windows.h"
#include <vector>
#include <iterator>

#include "algorithm.h"
#include "protocol.h"
#include "utility.h"

#define LIST_DEVICE "96,384,48,192"
#define List_DEVICENAME "Prime_96,Prime_384,Lite_48,Lite_192"

typedef HWND (__stdcall *create_3D)(int);
typedef void (__stdcall *destroy_3D)(HWND);
typedef void (__stdcall *show_3D)();
typedef void (__stdcall *plate_3D)(int);
typedef void (__stdcall *channel_3D)(int);
typedef void (__stdcall *data_3D)(quint16*,int);
typedef void (__stdcall *leftaxis)(int,int);
typedef void (__stdcall *data_Grad)(double*,int*,int);

enum gradient_Type {None, v_Gradient, h_Gradient, h8_Gradient, step_Gradient};

//-----------------------------------------------------------------------------

typedef struct
{
    gradient_Type type;
    double gradient;
    QString gradient_block;

}Gradient_INFO;

//-----------------------------------------------------------------------------
class Block_Temperature: public QDoubleSpinBox
{
Q_OBJECT

public:
    Block_Temperature(int id, QWidget *parent = 0);
    ~Block_Temperature();

    int         Index;      // 0-5 blocks
    QWidget     *Parent;

signals:
    void sChange_TemperatureBlock(void*);

public slots:
    void Change_Temperature();

};

//-----------------------------------------------------------------------------
class Gradient : public QDialog
{
    Q_OBJECT
public:
    Gradient(QWidget *parent = 0);
    ~Gradient();

    QString styleApp;
    double base;
    Gradient_INFO *gradient_Info;

    QGroupBox   *Image_Box;
    QGroupBox   *Control_Box;
    QLabel      *Base_Temperature;    
    QComboBox   *Device;
    QComboBox   *Type_gradient;
    QPushButton *apply;
    QPushButton *cancel;
    QTabWidget  *Tab_Temperature;
    QGroupBox   *Box_Single;
    QDoubleSpinBox  *set_SingleTemp;
    QTableWidget    *Diff_table;
    QVector<double> BLOCKs;

private:
    //...
    HINSTANCE   dll_handle;
    HWND        handle_3D;
    create_3D   create_Bars;
    destroy_3D  destroy_Bars;
    show_3D     show_Bars;
    plate_3D    plate_Bars;
    channel_3D  channel_Bars;
    data_3D     data_Bars;
    data_Grad   data_Gradient;
    leftaxis    setMinMax;
    //...

    QVector<double> Digit;
    QVector<int> Color;

    int RGB_Value(double, double, double);

signals:

public slots:
    void Load_lib(HINSTANCE);
    void Apply();
    void Draw_Gradient();
    void GetDevVal(int, double, double, double*);
    void GetTubeVal(int, double, double, double*);
    void Change_BlockTemperature(void*);
    void Set_DiffTemperature(QString);
    void Set_TabGradient(int);

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
};

#endif // GRADIENT_H
