#include "dockwidgettitlebar.h"

#include <QObject>
#include <QToolButton>
#include <QAction>
#include <QStyleOptionToolButton>
#include <QMenu>
#include <QEvent>
#include <QPainter>
#include <QLayout>
#include <QToolBar>
#include <QMainWindow>
#include <QColorDialog>

// added for debugging the reported bug
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>


DockWidgetTitleBar::DockWidgetTitleBar(QDockWidget *parent)
    : QToolBar(parent)
{

    Q_ASSERT(parent);
    mDock = parent;

    readCommonSettings();
    setFont(qApp->font());

    // a fake spacer widget
    QWidget* spacer = new QWidget(this);    
    spacer->setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum));
    addWidget(spacer);

    /*first_ColorButton = new Color_ToolButton(Qt::red, this);
    List_ColorAction.append(addWidget(first_ColorButton));
    first_ColorButton->setCheckable(true);
    second_ColorButton = new Color_ToolButton(QColor(0xFF, 0xA0, 0x00), this);
    List_ColorAction.append(addWidget(second_ColorButton));
    second_ColorButton->setCheckable(true);
    third_ColorButton = new Color_ToolButton(Qt::yellow, this);
    List_ColorAction.append(addWidget(third_ColorButton));
    third_ColorButton->setCheckable(true);
    fourth_ColorButton = new Color_ToolButton(Qt::green, this);
    List_ColorAction.append(addWidget(fourth_ColorButton));
    fourth_ColorButton->setCheckable(true);
    fifth_ColorButton = new Color_ToolButton(Qt::darkBlue, this);
    List_ColorAction.append(addWidget(fifth_ColorButton));
    fifth_ColorButton->setCheckable(true);
    sixth_ColorButton = new Color_ToolButton(Qt::blue, this);
    List_ColorAction.append(addWidget(sixth_ColorButton));
    sixth_ColorButton->setCheckable(true);
    seventh_ColorButton = new Color_ToolButton(Qt::magenta, this);
    List_ColorAction.append(addWidget(seventh_ColorButton));
    seventh_ColorButton->setCheckable(true);

    foreach(QAction* a, List_ColorAction) {a->setVisible(false);}*/


    /*Color_Group = new QButtonGroup(this);
    Color_Group->addButton(first_ColorButton);
    Color_Group->addButton(second_ColorButton);
    Color_Group->addButton(third_ColorButton);
    Color_Group->addButton(fourth_ColorButton);
    Color_Group->addButton(fifth_ColorButton);
    Color_Group->addButton(sixth_ColorButton);
    Color_Group->addButton(seventh_ColorButton);*/


    QWidget* spacer_fix = new QWidget(this);
    spacer_fix->setFixedWidth(30);
    addWidget(spacer_fix);

    //tbEnableDock = new pToolButton(this);
    tbLeftWindow = new pToolButton(this);
    tbRightWindow = new pToolButton(this);
    tbOrientation = new pToolButton(this);
    tbFloat = new pToolButton(this);
    tbClose = new pToolButton(this);
    //tbClose->installEventFilter(this);

    tbSelect = new pToolButton(this);
    tbNumerate = new pToolButton(this);

    //aEnable_Dock = addWidget(tbEnableDock);
    aSelect = addWidget(tbSelect);
    aNumerate = addWidget(tbNumerate);
    addSeparator();
    aSet_LeftWindow = addWidget(tbLeftWindow);
    aSet_RightWindow = addWidget(tbRightWindow);
    addSeparator();
    aOrientation = addWidget(tbOrientation);
    addSeparator();
    aFloat = addWidget(tbFloat);
    aClose = addWidget(tbClose);

    aSelect->setCheckable(true);
    aNumerate->setCheckable(true);
    //aClose->setEnabled(false);


    aSelect->setText(tr("color select"));
    aNumerate->setText(tr("tubes numeration"));
    aSet_LeftWindow->setText(tr("dock to Chart"));
    aSet_RightWindow->setText(tr("dock to Table"));
    aOrientation->setText(tr("titul orientation"));
    aFloat->setText(tr("dock/undock"));
    aClose->setText(tr("close"));

    //tbEnableDock->setDefaultAction(aEnable_Dock);
    tbLeftWindow->setDefaultAction(aSet_LeftWindow);
    tbRightWindow->setDefaultAction(aSet_RightWindow);
    tbOrientation->setDefaultAction(aOrientation);
    tbFloat->setDefaultAction(aFloat);
    tbClose->setDefaultAction(aClose);
    tbSelect->setDefaultAction(aSelect);
    tbNumerate->setDefaultAction(aNumerate);

    setMovable(false);
    setFloatable(false);

    updateStyleChange();
    dockWidget_featuresChanged(mDock->features());

    //connect(Color_Group, SIGNAL(buttonClicked(int)), this, SLOT(Color_Button(int)));

    //connect(aSelect, SIGNAL(changed()), this, SLOT

    connect(mDock, SIGNAL(featuresChanged(QDockWidget::DockWidgetFeatures)), this, SLOT(dockWidget_featuresChanged(QDockWidget::DockWidgetFeatures)));
    connect(aOrientation, SIGNAL(triggered()), this, SLOT(aOrientation_triggered()));
    connect(aFloat, SIGNAL(triggered()), this, SLOT(aFloat_triggered()));
    connect(aClose, SIGNAL(triggered()), this, SLOT(dockWidget_Close()));

    connect(aSet_LeftWindow, SIGNAL(triggered()), this, SLOT(SetLeftMainWindow()));
    connect(aSet_RightWindow, SIGNAL(triggered()), this, SLOT(SetRightMainWindow()));

    type_MainWin = chart_win;

    setCursor(Qt::ArrowCursor);
    //setCursor(Qt::PointingHandCursor);



}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/dock_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QIcon DockWidgetTitleBar::icon() const
{
       /*
        QIcon icon = mDock->toggleViewAction()->icon();

        if ( icon.isNull() ) {
                icon = mDock->windowIcon();
        }

        if ( icon.isNull() ) {
                icon =  toggleViewAction()->icon();
        }

        if ( icon.isNull() ) {
                icon =  windowIcon();
        }

        if ( icon.isNull() ) {
                icon = window()->windowIcon();
        }
        */
        return QIcon(); //return null icon
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QSize DockWidgetTitleBar::windowIconSize() const
{
        const int size = orientation() == Qt::Horizontal ? height() -2 : width() -2;
        return icon().isNull() ? QSize() : QSize( size, size );
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    event->ignore();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::mouseMoveEvent(QMouseEvent *event)
{

    QToolBar::mouseMoveEvent(event);
    setCursor(Qt::ArrowCursor);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::enterEvent(QEvent *event)
{
    QPoint P = mapFromGlobal(QCursor::pos());
    //qDebug() << "enter event: ok" << P;



    /*QMouseEvent *pressedLeftMouseButton = new QMouseEvent(QEvent::MouseButtonPress,
                                                          P,
                                                          Qt::LeftButton,
                                                          Qt::LeftButton,
                                                          Qt::NoModifier);
    QApplication::sendEvent(this, pressedLeftMouseButton);*/

    QToolBar::enterEvent(event);

    setCursor(Qt::ArrowCursor);
    mDock->setCursor(Qt::ArrowCursor);
    mDock->repaint();
    repaint();
    //QCursor::setPos(QCursor::pos());


    //QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::leaveEvent(QEvent* event)
{
    QToolBar::leaveEvent(event);
    setCursor(Qt::ArrowCursor);
    //QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::paintEvent(QPaintEvent* event)
{
        Q_UNUSED(event);

        QRect rect = this->rect();
        QTransform transform;
        QPainter painter(this);


        QColor topColor = QColor(192,192,192).lighter(120);
        QColor bottomColor = QColor(192,192,192).darker(120);

        if(StyleApp == "fusion")
        {
            topColor = QColor(200,200,200);
            bottomColor = QColor(200,200,200);
        }

        QLinearGradient gradient( rect.topLeft(), rect.bottomLeft() );

        topColor.setAlphaF( .7 );
        bottomColor.setAlphaF( .7 );

        gradient.setColorAt( 0, topColor );
        gradient.setColorAt( 1, bottomColor );

        if(mDock->features() & QDockWidget::DockWidgetVerticalTitleBar)
        {
            gradient.setFinalStop( rect.topRight() );
        }

        painter.setPen( Qt::NoPen );
        painter.setBrush( gradient );
        painter.drawRect( rect );

        painter.setPen( bottomColor.darker( 130 ) );
        painter.setBrush( Qt::NoBrush );
        painter.drawRect( rect.adjusted( 0, 0, -1, -1 ) );


        if ( mDock->features() & QDockWidget::DockWidgetVerticalTitleBar ) {
                rect.setSize( QSize( rect.height(), rect.width() ) );
                transform.rotate( -90 );
                transform.translate( -rect.width(), 0 );
        }

        painter.setTransform( transform );

        // icon / title
        QStyleOptionButton optionB;
        optionB.initFrom( mDock );
        optionB.rect = rect.adjusted( 2, 0, -( orientation() == Qt::Horizontal ? minimumSizeHint().width() : minimumSizeHint().height() ), 0 );
        optionB.text = mDock->windowTitle();
        optionB.iconSize = windowIconSize();
        optionB.icon = icon();

        style()->drawControl( QStyle::CE_PushButtonLabel, &optionB, &painter, mDock );

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::updateStyleChange()
{
        setIconSize(QSize(15, 15));
        layout()->setSpacing(0);
        layout()->setMargin(2);

        QIcon icon;

        //icon = style()->standardIcon( QStyle::SP_ArrowLeft, 0, widgetForAction( aEnable_Dock ) );
        //aEnable_Dock->setIcon( icon );

        //qDebug() << list_MainWindow.count();
        icon = style()->standardIcon( QStyle::SP_ArrowLeft, 0, widgetForAction( aSet_LeftWindow ) );
        aSet_LeftWindow->setIcon( icon );
        if(list_MainWindow.count() < 2) aSet_LeftWindow->setVisible(false);
        else aSet_LeftWindow->setVisible(true);

        icon = style()->standardIcon( QStyle::SP_ArrowRight, 0, widgetForAction( aSet_RightWindow ) );
        aSet_RightWindow->setIcon( icon );
        if(list_MainWindow.count() < 2) aSet_RightWindow->setVisible(false);
        else aSet_RightWindow->setVisible(true);

        //if(list_MainWindow.count() < 2) aFloat->setVisible(false);
        //else aFloat->setVisible(true);
        //aFloat->setEnabled(true);

        icon = style()->standardIcon( QStyle::SP_TitleBarShadeButton, 0, widgetForAction( aOrientation ) );
        aOrientation->setIcon( icon );

        if(mDock->allowedAreas())
        {
          icon = style()->standardIcon( QStyle::SP_TitleBarNormalButton, 0, widgetForAction( aFloat ) );
        }
        //else icon = style()->standardIcon( QStyle::SP_BrowserStop, 0, widgetForAction( aFloat ) );
        else icon = QIcon(":/images/anchor.ico");
        aFloat->setIcon( icon );

        icon = style()->standardIcon( QStyle::SP_TitleBarCloseButton, 0, widgetForAction( aClose ) );
        aClose->setIcon( icon );


        if(StyleApp == "fusion")
        {
            aSet_LeftWindow->setIcon(QIcon(":/images/flat/arrow_left_flat.png"));
            aSet_RightWindow->setIcon(QIcon(":/images/flat/arrow_right_flat.png"));
            aOrientation->setIcon(QIcon(":/images/flat/arrow_flat.png"));
            if(!mDock->allowedAreas()) aFloat->setIcon(QIcon(":/images/flat/unlock_flat.png"));
            else aFloat->setIcon(QIcon(":/images/flat/lock_flat.png"));
            aClose->setIcon(QIcon(":/images/flat/close_flat.png"));
            aSelect->setIcon(QIcon(":/images/flat/color_fill_24.png"));
            aNumerate->setIcon(QIcon(":/images/flat/numerate.png"));
        }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QSize DockWidgetTitleBar::minimumSizeHint() const
{
        return QToolBar::sizeHint();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QSize DockWidgetTitleBar::sizeHint() const
{
        const QSize wis = windowIconSize();
        QSize size = QToolBar::sizeHint();
        QFontMetrics fm( font() );

        if(mDock->features() & QDockWidget::DockWidgetVerticalTitleBar )
        {
                size.rheight() += fm.width( mDock->windowTitle() ) +wis.width();
        }
        else
        {
                size.rwidth() += fm.width( mDock->windowTitle() ) +wis.width();
        }

        return size;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::addSeparator( int index )
{
        if(index != -1)
        {
                index++;
        }

        if(index >= 0 && index < actions().count())
        {
                QToolBar::insertSeparator(actions().value(index));
        }
        else
        {
                QToolBar::addSeparator();
        }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::aOrientation_triggered()
{
        const QDockWidget::DockWidgetFeatures features = mDock->features();

        if(features & QDockWidget::DockWidgetVerticalTitleBar )
        {
            mDock->setFeatures(features ^ QDockWidget::DockWidgetVerticalTitleBar);
        }
        else
        {
            mDock->setFeatures(features | QDockWidget::DockWidgetVerticalTitleBar);
        }
        //QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::aFloat_triggered()
{
    if(mDock->isFloating())
    {
        mDock->setFloating(false);
        mDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
        mDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    }
    else
    {
        mDock->setFloating(true);
        mDock->setAllowedAreas(Qt::NoDockWidgetArea);
        mDock->setFeatures(QDockWidget::DockWidgetFloatable);
        mDock->move(10,10);
    }



    /*
        //if(mDock->isFloating()) QApplication::restoreOverrideCursor();

        if(mDock->allowedAreas()) mDock->setAllowedAreas(Qt::NoDockWidgetArea);
        else mDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

        mDock->setFloating(!mDock->isFloating());
    */
        updateStyleChange();
        repaint();
        mDock->repaint();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------

void DockWidgetTitleBar::dockWidget_featuresChanged( QDockWidget::DockWidgetFeatures features )
{
        //aFloat->setVisible( features & QDockWidget::DockWidgetFloatable );
        //aClose->setVisible( features & QDockWidget::DockWidgetClosable );

        // update toolbar orientation
        if(features & QDockWidget::DockWidgetVerticalTitleBar )
        {
                if( orientation() == Qt::Vertical )
                {
                        return;
                }

                setOrientation(Qt::Vertical);
        }
        else
        {
                if(orientation() == Qt::Horizontal )
                {
                        return;
                }

                setOrientation( Qt::Horizontal );
        }

        // re-order the actions
        const QList<QAction*> actions = this->actions();
        QList<QAction*> items;

        for(int i = actions.count() -1; i > -1; i-- )
        {
                items << actions.at( i );
        }

        clear();
        addActions(items);
        updateStyleChange();
        //QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::SetLeftMainWindow()
{
    if(list_MainWindow.count() < 2) return;
    QMainWindow *p = list_MainWindow.at(0);
    if(p)
    {
        mDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
        p->addDockWidget(Qt::TopDockWidgetArea, mDock);
        mDock->setFloating(false);
        updateStyleChange();

        type_MainWin = chart_win;
    }
    //QApplication::restoreOverrideCursor();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::SetRightMainWindow()
{
    if(list_MainWindow.count() < 2) return;
    QMainWindow *p = list_MainWindow.at(1);
    if(p)
    {
        mDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
        p->addDockWidget(Qt::TopDockWidgetArea, mDock);
        mDock->setFloating(false);
        updateStyleChange();

        type_MainWin = grid_win;
    }
    //QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
//--- dockWidget_Close()
//-----------------------------------------------------------------------------
void DockWidgetTitleBar::dockWidget_Close()
{
    emit close_Signal();

    mDock->close();

}

//-------------------------------------------------------------------------------------------------
//--- eventFilter
//-------------------------------------------------------------------------------------------------
/*bool DockWidgetTitleBar::eventFilter(QObject *obj, QEvent *evt)
{
    if(evt->type() == QEvent::HoverEnter && obj == tbClose)
    {
        setCursor(Qt::ArrowCursor);
        tbClose->repaint();

        //QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));

    }
    return(QWidget::eventFilter(obj, evt));
}*/
//-----------------------------------------------------------------------------
//--- change_ColorRejime(bool state)
//-----------------------------------------------------------------------------
/*void DockWidgetTitleBar::change_ColorRejime(bool state)
{
    foreach(QAction* a, List_ColorAction)
    {
        a->setVisible(state);
    }
    if(state && Color_Group->checkedId() == -1) QTimer::singleShot(500, this, SLOT(On_FirstColorButton()));
}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void DockWidgetTitleBar::On_FirstColorButton()
{
    Color_Group->buttonClicked(0);
}*/
//-----------------------------------------------------------------------------
//--- Color_Button(int id)
//-----------------------------------------------------------------------------
/*void DockWidgetTitleBar::Color_Button(int id)
{
    int i=0;
    QAbstractButton *button;

    foreach(button, Color_Group->buttons())
    {
        if(i == id) button->setChecked(true);
        else button->setChecked(false);
        i++;
    }
}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void Color_ToolButton::paintEvent(QPaintEvent *e)
{
    //QColor color;

    if(isChecked()) color.setAlpha(255);
    else color.setAlpha(100);

    QToolButton::paintEvent(e);
    //return;

    QPainter painter(this);

    //QPen pen = painter.pen();
    //pen.setWidth(2);
    //painter.setPen(pen);

    QRect rect = painter.viewport();
    rect.setX(1);
    rect.setY(1);
    rect.setWidth(rect.width() - 1);
    rect.setHeight(rect.height() - 1);

    //painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    //painter.drawRect(rect);

    painter.fillRect(rect, color);
    if(isChecked()) painter.drawRect(rect);

}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void Color_ToolButton::mouseDoubleClickEvent(QMouseEvent *e)
{
    QToolButton::mouseDoubleClickEvent(e);

    //qDebug() << "mouseDoubleClickEvent: ";

    QColor color_temp = QColorDialog::getColor(color, this, tr("Select Color"));
    if(color_temp.isValid()) color = color_temp;
}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void Color_ToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    QToolButton::mouseReleaseEvent(e);

    if(e->button() == Qt::RightButton)
    {
        QColor color_temp = QColorDialog::getColor(color, this, tr("Select Color"));
        if(color_temp.isValid()) color = color_temp;
    }
}*/

