#ifndef DOCKWIDGETTITLEBAR_H
#define DOCKWIDGETTITLEBAR_H

#include <QToolBar>
#include <QDockWidget>
#include <QList>
#include <QSize>
#include <QDebug>
#include <QSettings>
#include <QTranslator>
#include <QEvent>
#include <QMouseEvent>
#include <QToolButton>
#include <QButtonGroup>
#include <QTimer>
#include <QGroupBox>
#include <QPalette>

#include "DockWidgetTitleBar_global.h"

class QToolButton;
typedef QToolButton pToolButton;
class QStyleOptionToolButton;

enum  MainWinDock{none_win, chart_win, grid_win};

//-----------------------------------------------------------------------------

/*class Color_ToolButton: public QToolButton
{
    Q_OBJECT

public:

    Color_ToolButton(QColor col, QWidget* parent = 0)
            : QToolButton(parent)   {color = col;}

    QColor color;

protected:

    virtual void paintEvent(QPaintEvent* e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
};*/

//-----------------------------------------------------------------------------


class DOCKWIDGETTITLEBARSHARED_EXPORT DockWidgetTitleBar: public QToolBar
{
    Q_OBJECT

public:

    //--- Create ---
    DockWidgetTitleBar(QDockWidget* parent = 0);

    //--- Reimplemented ---
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;


    //	Insert a separator at index.
    //	If index is -1 then it's append at end.
    //
    void addSeparator(int index = -1);
    void updateStyleChange();

    QList<QMainWindow*> list_MainWindow;
    //enum MainWinDock{none_win, chart_win, grid_win};// type_MainWin;
    MainWinDock type_MainWin;

    QDockWidget* mDock;             // parent widget - dockWidget
    QSize oldMin, oldMax;

    /*QButtonGroup        *Color_Group;
    QList<QAction*>     List_ColorAction;

    Color_ToolButton    *first_ColorButton;
    Color_ToolButton    *second_ColorButton;
    Color_ToolButton    *third_ColorButton;
    Color_ToolButton    *fourth_ColorButton;
    Color_ToolButton    *fifth_ColorButton;
    Color_ToolButton    *sixth_ColorButton;
    Color_ToolButton    *seventh_ColorButton;*/

    QAction* aSelect;
    QAction* aNumerate;

    pToolButton* tbSelect;
    pToolButton* tbNumerate;

    QAction* aSet_LeftWindow;
    QAction* aSet_RightWindow;
    QAction* aOrientation;
    QAction* aFloat;
    QAction* aClose;


public slots:
        //void change_ColorRejime(bool);

signals:

        void close_Signal(void);        
protected:

        //QAction* aEnable_Dock;



        //pToolButton* tbEnableDock;
        pToolButton* tbLeftWindow;
        pToolButton* tbRightWindow;
        pToolButton* tbOrientation;
        pToolButton* tbFloat;
        pToolButton* tbClose;

        //--- Reimplemented ---
        virtual void paintEvent(QPaintEvent* event);
        virtual void enterEvent (QEvent* event);
        virtual void leaveEvent(QEvent* event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseDoubleClickEvent(QMouseEvent *event);

        //virtual bool eventFilter( QObject *, QEvent * );

        QIcon icon() const;
        QSize windowIconSize() const;

private slots:
        void aOrientation_triggered();
        void aFloat_triggered();
        void dockWidget_featuresChanged(QDockWidget::DockWidgetFeatures features);

        void SetLeftMainWindow();
        void SetRightMainWindow();
        void dockWidget_Close();

        //void Color_Button(int);
        //void On_FirstColorButton();

private:
        QTranslator translator;
        void readCommonSettings();
        QString StyleApp;

};

#endif // DOCKWIDGETTITLEBAR_H
