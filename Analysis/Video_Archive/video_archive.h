#ifndef VIDEO_ARCHIVE_H
#define VIDEO_ARCHIVE_H

#include <QWindow>
#include <QApplication>
#include <QObject>
#include <QDialog>
#include <QtWidgets>
#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QBoxLayout>
#include <QSplitter>
#include <QDebug>
#include <QList>
#include <QSettings>
#include <QTranslator>
#include <QAction>
#include <QMainWindow>
#include <QDockWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDate>
#include <QToolBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QImage>
#include <QDir>
#include <QFileInfo>
#include <QDataStream>
#include <QStringList>
#include <QtAlgorithms>
#include <QPushButton>


#include "video_archive_global.h"

#define LEFT_OFFSET 66          // offset for 96,384
#define LEFT_OFFSET_DT48 7      // offset for 48,192
#define W_IMAGE 752             // width image
#define H_IMAGE 292             // heigth image
#define COEF_IMAGE 1.93         //
#define W_IMAGE_COEF 390        // width image (width/coef: 752/1.93)
#define W_REALIMAGE 825         //
#define H_REALIMAGE 312         //
#define TOP_OFFSET 5


class Image_Widget;

//-----------------------------------------------------------------------------
class VIDEO_ARCHIVESHARED_EXPORT Video_Archive: public QDialog
{
    Q_OBJECT

public:
    Video_Archive();
    ~Video_Archive();

    QTranslator translator;
    void readCommonSettings();

    void Clear_Image();
    void Draw_Image(QString fn);

    QGroupBox       *open_Box;
    QLabel          *file_open;
    QPushButton     *open;
    QPushButton     *next_video;
    QPushButton     *previous_video;

    QImage          *Image;
    Image_Widget    *image_Widget;
    QVector<int>    *Video_BUF;
    QStringList     *List_FN;
    int             current_index;

private slots:
    void Open_File();
    void Next_Video();
    void Previous_Video();

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
        Video_Archive   *p_archive;

};

#endif // VIDEO_ARCHIVE_H
