#include "video.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Video::Video(QWidget *parent): QMainWindow(parent)
{
    int i;
    int x, y;

    readCommonSettings();
    setFont(qApp->font());

    type_dev = 96;
    active_channels = 0x0f;

    dll_handle = NULL;
    handle_3D = NULL;

    W_IMAGE_COEF = 390;
    H_IMAGE = 292;
    W_IMAGE = 752;

    BUF_Video.resize(H_IMAGE*W_IMAGE);
    BUF_Video.fill(0);

    //QFont app_font = QFont("Comic Sans MS", 10, QFont::Normal);
    //setFont(app_font);

    Main_Box = new QGroupBox(this);
    Main_Box->setObjectName("Transparent");
    setCentralWidget(Main_Box);

    main_Box = new QGroupBox(this);
    main_Box->setObjectName("Transparent");
    buttons_Box = new QGroupBox(this);
    buttons_Box->setObjectName("Transparent");
    QVBoxLayout *Main_layout = new QVBoxLayout;
    Main_layout->setMargin(0);
    Main_Box->setLayout(Main_layout);
    Main_layout->addWidget(main_Box);
    Main_layout->addWidget(buttons_Box);

    QHBoxLayout *main_layout = new QHBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(4);
    main_Box->setLayout(main_layout);

    Video_Box = new QGroupBox(this);
    Control_Box = new QGroupBox(this);
    //Additional_Box = new QGroupBox(this);

    main_layout->addWidget(Video_Box);
    main_layout->addWidget(Control_Box);
    //main_layout->addWidget(Additional_Box);

    //--- Video_Box ---
    Image_Box = new QGroupBox(this);
    VideoControl_Box = new QGroupBox(this);

    Set_Fluor = new QComboBox(this);
    fluor_delegate = new FluorVideo_ItemDelegate;
    fluor_delegate->style = StyleApp;
    Set_Fluor->setItemDelegate(fluor_delegate);


    if(StyleApp == "fusion")
    {
        Make_Measure = new QPushButton(QIcon(":/images/flat/video_flat.png"),tr("   Measure"), this);
        Make_Measure->setIconSize(QSize(24,24));
    }
    else
    {
        Make_Measure = new QPushButton(QIcon(":/images/foto.png"),tr("   Measure"), this);
        Make_Measure->setIconSize(QSize(33,24));
    }

    Load_AutoMask = new QToolButton(this);
    Load_AutoMask->setIcon(QIcon(":/images/flat/open_flat.png"));
    Load_AutoMask->setToolTip(tr("Auto load mask"));
    Load_AutoMask->setIconSize(QSize(24,24));

    Set_Expo = new QSpinBox(this);
    Set_Expo->setSuffix(tr("  ms"));
    Set_Expo->setRange(0,10000);
    Set_Expo->setSingleStep(50);
    Set_Expo->setValue(1000);    
    Set_Expo->setFont(QFont("Times New Roman", 14, QFont::Bold));

    Image = new QImage(W_IMAGE_COEF, H_IMAGE, QImage::Format_RGB32);
    Image->fill(Qt::white);
    //Label_Image = new QLabel(this);
    //Label_Image->setPixmap(QPixmap::fromImage(*Image));
    image_Widget = new Image_Widget(Image, this);
    image_Widget->setFixedSize(W_IMAGE_COEF, H_IMAGE);    

    QHBoxLayout *layout_video = new QHBoxLayout;
    Video_Box->setLayout(layout_video);
    layout_video->addWidget(Image_Box);
    layout_video->addWidget(VideoControl_Box);
    QHBoxLayout *layout_image = new QHBoxLayout;
    Image_Box->setLayout(layout_image);
    layout_image->addWidget(image_Widget);
    QVBoxLayout *layout_VideoControl = new QVBoxLayout;
    VideoControl_Box->setLayout(layout_VideoControl);
    layout_VideoControl->addWidget(Set_Fluor);
    layout_VideoControl->addWidget(Set_Expo);
    layout_VideoControl->addStretch();
    layout_VideoControl->addWidget(Load_AutoMask, 0, Qt::AlignRight);
    layout_VideoControl->addWidget(Make_Measure);

    if(StyleApp == "fusion")
    {
        Video_Box->setObjectName("Transparent");
        layout_video->setMargin(0);
        Image_Box->setStyleSheet("margin-top: 1ex; background: #ffffff; border: 1px solid #ddd;");
    }

    //--- Control_Box ---
    //Control_Box->setFixedWidth(200);

    Preference_Box = new QTabWidget(this);
    Corner_Box = new QGroupBox(this);
    if(StyleApp == "fusion") Corner_Box->setObjectName("Transparent");
    Radius_Box = new QGroupBox(this);
    if(StyleApp == "fusion") Radius_Box->setObjectName("Transparent");
    Median_Box = new QGroupBox(tr("Nonlinear characteristics"),this);
    Median_Box->setCheckable(true);
    Median_Box->setChecked(false);
    if(StyleApp == "fusion") Median_Box->setObjectName("Transparent");

    /*Temp_Box = new QGroupBox(this);
    QVBoxLayout *layout_temp = new QVBoxLayout;
    Temp_Box->setLayout(layout_temp);
    Read_Sector = new QPushButton("Read Sector", this);
    Write_Sector = new QPushButton("Write Sector", this);
    layout_temp->addWidget(Read_Sector);
    layout_temp->addWidget(Write_Sector);
    connect(Read_Sector, SIGNAL(clicked(bool)), this, SLOT(slot_ReadSector()));
    connect(Write_Sector, SIGNAL(clicked(bool)), this, SLOT(slot_WriteSector()));*/


    Preference_Box->addTab(Corner_Box, tr("XY corner"));
    Preference_Box->addTab(Radius_Box, tr("Radius"));
    Preference_Box->addTab(Median_Box, QIcon(":/images/flat/checkbox_null.ico"), tr("Nonlinearity"));
    //Preference_Box->addTab(Temp_Box, "RW_sector");
    Preference_Box->setMinimumWidth(300);

    Control_subBox = new QGroupBox(this);
    QVBoxLayout *layout_control = new QVBoxLayout;
    Control_Box->setLayout(layout_control);
    layout_control->addWidget(Preference_Box);
    //layout_control->addWidget(Corner_Box);
    //layout_control->addWidget(Radius_Box);
    layout_control->addStretch(1);
    layout_control->addWidget(Control_subBox);
    if(StyleApp == "fusion")
    {
        Control_Box->setObjectName("Transparent");
        layout_control->setMargin(0);
    }

    QVBoxLayout *layout_corner = new QVBoxLayout;
    Corner_Box->setLayout(layout_corner);
    if(StyleApp == "fusion")
    {

    }
    else Corner_Box->setTitle(tr("Corner points coordinates: (x,y)"));
    layout_corner->setMargin(2);
    layout_corner->setSpacing(2);
    Corner *corner;
    for(i=0; i<4; i++)
    {
        corner = new Corner(i, this);
        corner->setObjectName("Transparent");
        corners_Mask.append(corner);
        layout_corner->addWidget(corner);
    }

    QHBoxLayout *layout_radius = new QHBoxLayout;

    if(StyleApp == "fusion")
    {

    }
    else Radius_Box->setTitle(tr("Light spot size: (Rx,Ry)"));
    Radius_Box->setLayout(layout_radius);
    Rx_spot = new QSpinBox(this);
    Ry_spot = new QSpinBox(this);
    Rx_spot->setFixedWidth(70);
    Ry_spot->setFixedWidth(70);
    Rx_spot->setFont(QFont("Times New Roman", 10, QFont::Bold));
    Ry_spot->setFont(QFont("Times New Roman", 10, QFont::Bold));    
    Rxy_label = new QLabel(tr("Rx,Ry:"), this);
    //Rxy_label->setFont(QFont("Times New Roman", 10, QFont::Bold));

    layout_radius->setMargin(2);
    layout_radius->setSpacing(2);
    //layout_radius->addSpacing(1);
    layout_radius->addWidget(Rxy_label);
    layout_radius->addStretch(1);
    layout_radius->addWidget(Rx_spot);
    layout_radius->addWidget(Ry_spot);
    Rx_spot->setRange(1,100);
    Ry_spot->setRange(1,100);

    QVBoxLayout *layout_median = new QVBoxLayout;
    Median_Box->setLayout(layout_median);
    layout_median->setMargin(10);
    //layout_median->setSpacing(2);
    Median *median;
    for(i=0; i<4; i++)
    {
        median = new Median(i, this);
        median->setObjectName("Transparent");
        median_Mask.append(median);
        layout_median->addWidget(median);
    }


    Paint_Mask = new QCheckBox(tr("Paint Mask"), this);
    Digit_Plate = new QPushButton(tr("Digit Plate"), this);

    //... zoom ...
    Zoom = new QPushButton(tr("Zoom"), this);
    zoom = new Zoom_Dialog(this);
    zoom->move(qApp->desktop()->availableGeometry().width()-310,0);
    zoom_h = 27;
    zoom_w = 52;
    zoom_tube = -1;
    //...

    Return_ToInitial = new QPushButton(tr("Return To Initial Values"), this);
    DT_digit = new QRadioButton(tr("DT"), this);
    Soft_digit = new QRadioButton(tr("RT"), this);
    DT_digit->setChecked(true);

    if(StyleApp == "fusion")
    {
        Digit_Plate->setIcon(QIcon(":/images/flat/digitize_flat.png"));
        Digit_Plate->setToolTip(Digit_Plate->text());
        Digit_Plate->setText("");
        Digit_Plate->setIconSize(QSize(48,24));
        Zoom->setIcon(QIcon(":/images/flat/scale_flat.png"));
        Zoom->setToolTip(Zoom->text());
        Zoom->setIconSize(QSize(24,24));
        Zoom->setText("");
        Return_ToInitial->setIcon(QIcon(":/images/flat/back_flat.png"));
        Return_ToInitial->setToolTip(Return_ToInitial->text());
        Return_ToInitial->setIconSize(QSize(24,24));
        Return_ToInitial->setText("");
    }

    QVBoxLayout *layout_subcontrol = new QVBoxLayout;
    Control_subBox->setLayout(layout_subcontrol);
    QHBoxLayout *layout_digit = new QHBoxLayout();
    layout_digit->addWidget(Digit_Plate);
    layout_digit->addStretch(1);
    layout_digit->addWidget(DT_digit);
    layout_digit->addWidget(Soft_digit);
    layout_subcontrol->addWidget(Paint_Mask);
    layout_subcontrol->addLayout(layout_digit);
    //layout_subcontrol->addWidget(Digit_Plate);
    layout_subcontrol->addWidget(Zoom);
    layout_subcontrol->addWidget(Return_ToInitial);
    Paint_Mask->setChecked(true);

    //... buttons_Box ...
    SaveGeometry_Button = new QPushButton(tr("Save Geometry"), this);
    Cancel_Button = new QPushButton(tr("Cancel"), this);
    QHBoxLayout *layout_buttons = new QHBoxLayout;
    layout_buttons->setMargin(4);
    buttons_Box->setLayout(layout_buttons);
    layout_buttons->addStretch(1);
    layout_buttons->addWidget(SaveGeometry_Button);
    layout_buttons->addWidget(Cancel_Button);

    connect(Make_Measure, SIGNAL(clicked(bool)), this, SLOT(Get_Image()));
    connect(Load_AutoMask, SIGNAL(clicked(bool)), this, SLOT(Load_MASK()));
    connect(Set_Fluor, SIGNAL(currentIndexChanged(int)), this, SLOT(Clear_Image()));
    connect(Rx_spot, SIGNAL(valueChanged(int)), this, SLOT(Change_Radius()));
    connect(Ry_spot, SIGNAL(valueChanged(int)), this, SLOT(Change_Radius()));
    connect(Paint_Mask, SIGNAL(clicked(bool)), image_Widget, SLOT(update()));
    connect(Return_ToInitial, SIGNAL(clicked(bool)), this, SLOT(Return_To_Initial()));
    connect(SaveGeometry_Button, SIGNAL(clicked(bool)), this, SLOT(Save_Parameters()));
    connect(Cancel_Button, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(Digit_Plate, SIGNAL(clicked(bool)), this, SLOT(Show_DigitPlate()));
    connect(DT_digit, SIGNAL(toggled(bool)), this, SLOT(Toggle_digit(bool)));
    connect(Zoom, SIGNAL(clicked(bool)), this, SLOT(Show_ZoomDialog()));
    connect(Median_Box, SIGNAL(clicked(bool)), this, SLOT(Change_MedianRejime()));



    setWindowFlags(this->windowFlags() & ~Qt::WindowMinimizeButtonHint
                                       & ~Qt::WindowMaximizeButtonHint);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Video::~Video()
{

    if(dll_handle)
    {
        if(handle_3D) destroy_Bars(handle_3D);
    }

    delete zoom;
    delete Image;

    delete Image_Box;
    delete VideoControl_Box;

    qDeleteAll(corners_Mask.begin(),corners_Mask.end());
    corners_Mask.clear();

    qDeleteAll(median_Mask.begin(),median_Mask.end());
    median_Mask.clear();

    /*delete Read_Sector;
    delete Write_Sector;
    delete Temp_Box;*/

    delete Corner_Box;
    delete Radius_Box;
    delete Median_Box;
    delete Preference_Box;
    delete Control_subBox;

    delete Video_Box;
    delete Control_Box;
    //delete Additional_Box;
    delete main_Box;
    delete buttons_Box;
    delete Main_Box;

}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Video::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/video_" + text + ".qm"))
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
//--- Load_lib
//-----------------------------------------------------------------------------
void Video::Load_lib(HINSTANCE handle)
{
    dll_handle = handle;

    if(dll_handle)
    {
        create_Bars = reinterpret_cast<create_3D>(::GetProcAddress(dll_handle,"create_bars3D"));
        destroy_Bars = reinterpret_cast<destroy_3D>(::GetProcAddress(dll_handle,"destroy_bars3D"));
        show_Bars = reinterpret_cast<show_3D>(::GetProcAddress(dll_handle,"show_bars3D"));
        plate_Bars = reinterpret_cast<plate_3D>(::GetProcAddress(dll_handle,"plate_bars3D"));
        channel_Bars = reinterpret_cast<channel_3D>(::GetProcAddress(dll_handle,"channel_bars3D"));
        data_Bars = reinterpret_cast<data_3D>(::GetProcAddress(dll_handle,"data_bars3D"));

        if(create_Bars)
        {
            handle_3D = create_Bars(5);
        }        
    }

    DT_digit->blockSignals(true);
    DT_digit->setChecked(true);
    DT_digit->blockSignals(false);
}

//-----------------------------------------------------------------------------
//--- closeEvent
//-----------------------------------------------------------------------------
void Video::closeEvent(QCloseEvent *event)
{
    Digit.clear();
    Digit_soft.clear();
    BUF_Video.clear();

    Preference_Box->setCurrentIndex(0);

    zoom->close();

    if(dll_handle)
    {
        if(handle_3D)
        {
            DestroyWindow(handle_3D);
            //ShowWindow(handle_3D, SW_HIDE);

            //destroy_Bars(handle_3D);
            //handle_3D = NULL;
        }
    }

    event->accept();     

}
//-----------------------------------------------------------------------------
//--- slot Get_Image()
//-----------------------------------------------------------------------------
void Video::Get_Image()
{    
    int ch = Set_Fluor->currentIndex();
    int expo = Set_Expo->value();

    Clear_Image();
    emit MakeMeasure_Request(ch, expo, 2);
    //QTimer::singleShot(1000,this,SLOT(Camera_Sound()));
}
//-----------------------------------------------------------------------------
//--- slot Camera_Sound()
//-----------------------------------------------------------------------------
void Video::Camera_Sound()
{
    //qDebug() << qApp->applicationDirPath();
    //QString text = qApp->applicationDirPath() + "/sound/CAMERA.wav";
    //QSound::play(text);
}
//-----------------------------------------------------------------------------
//--- slot Toggle_digit(bool)
//-----------------------------------------------------------------------------
void Video::Toggle_digit(bool dt_digit)
{
    if(dt_digit && Digit.size() == type_dev) {data_Bars(Digit.data(), type_dev);}
    else
    {
        if(Digit_soft.size() == type_dev) {data_Bars(Digit_soft.data(), type_dev);}
    }
}

//-----------------------------------------------------------------------------
//--- slot Get_Image()
//-----------------------------------------------------------------------------
void Video::Get_Picture(QMap<QString,QByteArray> *map)
{

    if(map->size() == 0) return;

    int num_sec = div(type_dev,96).quot;
    if(type_dev == 48) num_sec = 1;

    QByteArray buf = map->value(GETPIC_VIDEO, NULL);
    int count = buf.size();

    //qDebug() << "Buf_size: " << count;

    int COUNT = 256*1023*2;
    if(FHW == 4.0) COUNT = 640*480*2;

    if(count != COUNT)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.setText(QString("Attention! Invalid Buf_Video size: %1 (%2)").arg(count).arg(COUNT));
        message.exec();

        buf.clear();
        map->clear();
        return;
    }

    QByteArray buf_digit = map->value(GETPIC_DATA, NULL);
    int count_digit = buf_digit.size();    

    if(count_digit != 512*num_sec)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.setText(QString("Attention! Invalid Buf_Digit size: %1 (%2)").arg(count_digit).arg(512*num_sec));
        message.exec();

        buf_digit.clear();
        map->clear();
        return;
    }

    //... video ...
    //QVector<int> BUF(H_IMAGE * W_IMAGE);
    BUF_Video.fill(0);
    QVector<ushort> vec(count/2);
    memcpy(vec.data(), buf.data(), count);    

    Treat_VideoImage(vec, BUF_Video);    
    Draw_VideoImage(BUF_Video);    

    image_Widget->update();

    //... difit soft ...    
    Digit_soft.fill(0);    
    Digitise_VideoImage(XY, Rx, Ry, offset_XY, W_IMAGE, H_IMAGE, Set_Fluor->currentIndex(), BUF_Video, Digit_soft);   

    //... digit ...    
    Digit.fill(0);
    Treat_DigitBuf(buf_digit, Digit);

    //... 3D ...
    Toggle_digit(DT_digit->isChecked());
    //data_Bars(Digit.data(), type_dev);    

    buf.clear();    
    buf_digit.clear();    
    vec.clear();    
    //BUF.clear();
    map->clear();

    if(!zoom->isHidden()) Draw_ZoomVideoImage(zoom_tube);
}
//-----------------------------------------------------------------------------
//--- Treat_DigitBuf
//-----------------------------------------------------------------------------
void Video::Treat_DigitBuf(QByteArray &buf, QVector<quint16> &digit)
{
    int i;
    int count_sec = 96;

    int num_sec = div(type_dev,96).quot;
    if(type_dev == 48) {num_sec = 1; count_sec = 48;}

    for(i=0; i<num_sec; i++)
    {
        memcpy(digit.data()+96*i, buf.data()+128+512*i , count_sec*2);
    }    
}

//-----------------------------------------------------------------------------
//--- Treat_VideoImage a->b
//-----------------------------------------------------------------------------
void Video::Treat_VideoImage(QVector<ushort> &a, QVector<int> &b)
{
    int i,j;
    int k,m;
    int left;
    int width_left;

    switch(type_dev)
    {
    default:
    case 96:
    case 384:
                left = LEFT_OFFSET;
                break;
    case 48:
    case 192:
                left = LEFT_OFFSET_DT48;
                break;
    }

    width_left = left + W_IMAGE;

    k=0;
    for(i=0; i<H_REALIMAGE; i++)
    {
        if(FHW == 3.0 && (i<5 || i>296)) continue;
        m=0;
        for(j=0; j<W_REALIMAGE; j++)
        {
            if(j<left || j>width_left-1) continue;
            b.replace(k*W_IMAGE + m, a.at(i*W_REALIMAGE + j));
            m++;
        }
        k++;
    }
}
//-----------------------------------------------------------------------------
//--- Draw_VideoImage
//-----------------------------------------------------------------------------
void Video::Draw_VideoImage(QVector<int> &a)
{
    int i,j;
    int num = 0;
    int size = W_IMAGE * H_IMAGE;
    QRgb pb = QColor(255,0,0).rgba();
    int pixel;

    QImage image(W_IMAGE, H_IMAGE, QImage::Format_RGB32);

    for(i=0; i<H_IMAGE; i++)
    {
        QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(i));
        for(j=0; j<W_IMAGE; j++)
        {
            pixel = a.at(num);
            pixel = pixel >> 4;
            pb = qRgb(pixel,pixel,pixel);
            line[j] = pb;
            num++;
        }
    }

    // Scaled image: ...
    *Image = image.scaled(W_IMAGE_COEF, H_IMAGE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

}
//-----------------------------------------------------------------------------
//--- Draw_ZoomVideoImage(int n)
//-----------------------------------------------------------------------------
void Video::Draw_ZoomVideoImage(int n)
{
    QPen pen(Qt::red,1,Qt::SolidLine);
    QImage *img = zoom->image;
    QPainter painter;
    QString text = "Zoom";
    rt_Plate Plate;
    int col,row;
    int x,y;
    Plate.PlateSize(type_dev, row, col);

    double coef = 300./(double)zoom_h;

    if(zoom->isHidden()) return;

    if(n >= 0)
    {
       x = XY.at(2*n);
       y = XY.at(2*n+1);
    }

    if(n >= 0) text += " " + Convert_IndexToName(n,col);
    zoom->setWindowTitle(text);

    if(n < 0) zoom->image->fill(Qt::white);
    else
    {
        QRect rect;
        rect.setLeft((x - zoom_w/2)/COEF_IMAGE);
        rect.setTop(y - zoom_h/2);
        rect.setWidth(zoom_w/COEF_IMAGE);
        rect.setHeight(zoom_h);

        //qDebug() << "rect: " << rect << x << y;
        //*zoom->image = Image->copy(20,20,50, 50).scaled(300,300,Qt::KeepAspectRatio, Qt::SmoothTransformation);
        *zoom->image = Image->copy(rect).scaled(300,300,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter.begin(img);
        painter.setPen(pen);
        painter.drawLine(0,150,300,150);
        painter.drawLine(150+3,0,150+3,300);

        rect.setLeft(150 - (double)Rx*coef/COEF_IMAGE);
        rect.setTop(150 - (double)Rx*coef/COEF_IMAGE);
        rect.setRight(150 + (double)Rx*coef/COEF_IMAGE + 7);
        rect.setBottom(150 + (double)Rx*coef/COEF_IMAGE);
        painter.drawEllipse(rect);
        painter.end();
    }
    zoom->repaint();

}

//-----------------------------------------------------------------------------
//--- Draw_Mask(int rep)
//-----------------------------------------------------------------------------
void Video::Draw_Mask(QPainter *p, int rep)
{
    int i;    
    QPen pen;
    QPoint center;
    QString text;
    int x,y;
    int select = -1;

    if(!Paint_Mask->isChecked()) return;

    if(Rep >= 0) select = rep_Point.at(Rep);

    pen.setColor(Qt::red);
    pen.setWidth(1);
    p->setPen(pen);

    int id = Set_Fluor->currentIndex();
    short offset_x = offset_XY.at(id*2);
    short offset_y = offset_XY.at(id*2+1);

    //... draw mask ...
    for(i=0; i<type_dev; i++)
    {
        if(select == i)
        {
            pen.setWidth(2);
            p->setPen(pen);
        }
        center.setX(Round((double)(XY.at(2*i) + offset_x)/COEF_IMAGE));
        center.setY(XY.at(2*i+1) + offset_y);
        p->drawEllipse(center,Round((double)(Rx)/COEF_IMAGE),Ry);
        if(pen.width() > 1)
        {
            pen.setWidth(1);
            p->setPen(pen);
        }
    }

    //... draw Label ...
    p->setFont(QFont("Times New Roman", 8, QFont::Bold));
    for(i=0; i<4; i++)
    {
        text = list_Label.at(i);

        x = XY.at(2*rep_Point.at(i));
        y = XY.at(2*rep_Point.at(i)+1);
        if(div(i,2).rem == 0) x -= Rx*3.5;
        else x += Rx*1.8;
        x /= COEF_IMAGE;
        center.setX(x);
        center.setY(y);
        p->drawText(center, text);
    }

}
//-----------------------------------------------------------------------------
//--- Change_Radius()
//-----------------------------------------------------------------------------
void Video::Change_Radius()
{
    Rx = Rx_spot->value();
    Ry = Ry_spot->value();

    //... digit soft ...
    Digitise_VideoImage(XY, Rx, Ry, offset_XY, W_IMAGE, H_IMAGE, Set_Fluor->currentIndex(), BUF_Video, Digit_soft);
    if(Soft_digit->isChecked() && Digit_soft.size() == type_dev) data_Bars(Digit_soft.data(), type_dev);

    image_Widget->update();     // repaint -> draw Mask

    Check_GeometryChange();
 }
//-----------------------------------------------------------------------------
//--- Return_To_Initial()
//-----------------------------------------------------------------------------
void Video::Return_To_Initial()
{
    Get_InfoDevice(&map_DeviceInfo, false);
    image_Widget->update();

    //... digit soft ...
    Digitise_VideoImage(XY, Rx, Ry, offset_XY, W_IMAGE, H_IMAGE, Set_Fluor->currentIndex(), BUF_Video, Digit_soft);
    if(Soft_digit->isChecked() && Digit_soft.size() == type_dev) data_Bars(Digit_soft.data(), type_dev);
}
//-----------------------------------------------------------------------------
//--- Save_Parameters()
//-----------------------------------------------------------------------------
void Video::Save_Parameters()
{
    int i;
    QString data;
    QString str_median;

    short value;
    int left;
    int count = XY.size();    

    union
    {
        Device_ParMedian Device_ParamMedian;
        unsigned char byte_buf[512];
    }DEVICE_PAR_MEDIAN;

    left = LEFT_OFFSET;
    if(type_dev == 48 || type_dev == 192) left = LEFT_OFFSET_DT48;

    //... geometry ...
    for(i=0; i<count; i++)
    {
        if(!div(i,2).rem) value = XY.at(i) + left;      // X
        else value = XY.at(i) + TOP_OFFSET;             // Y

        data += QString("%1 ").arg(value);
    }

    //... radius ...
    data += QString("%1 ").arg(Rx);
    data += QString("%1 ").arg(Ry);

    //... median ...
    for(i=0; i<512; i++) DEVICE_PAR_MEDIAN.byte_buf[i] = 0;
    strcpy(DEVICE_PAR_MEDIAN.Device_ParamMedian.id_median, "median");
    for(i=0; i<4; i++)
    {
        Median* median = median_Mask.at(i);
        median->x_median->blockSignals(true);
        median->y_median->blockSignals(true);
        if(!Median_Box->isChecked())
        {
            median->x_median->setValue(0);
            median->y_median->setValue(0);
        }

        DEVICE_PAR_MEDIAN.Device_ParamMedian.median_xy[i] = median->x_median->value();
        DEVICE_PAR_MEDIAN.Device_ParamMedian.median_xy[i+4] = median->y_median->value();

        median->x_median->blockSignals(false);
        median->y_median->blockSignals(false);
    }    
    QByteArray ba = QByteArray((char*)(DEVICE_PAR_MEDIAN.byte_buf), 512);
    str_median = ba.toBase64();

    emit SaveParMedian("median", str_median);

    emit SavePar(0,data);

    close();
}

//-----------------------------------------------------------------------------
//--- Change_XY
//-----------------------------------------------------------------------------
void Video::Change_XY(int rep)
{
    QPoint center;

    Corner *p_center = corners_Mask.at(rep);
    center.setX(p_center->x_corner->value());
    center.setY(H_IMAGE - p_center->y_corner->value());

    XY_corner.replace(rep, center);
    Recalculate_Geometry();

    if(Median_Box->isChecked())
    {
        Calculate_Median();
    }

    image_Widget->update();

    if(!zoom->isHidden()) Draw_ZoomVideoImage(zoom_tube);

    Check_GeometryChange();
}

//-----------------------------------------------------------------------------
//--- Recalculate_Geometry()
//-----------------------------------------------------------------------------
void Video::Recalculate_Geometry()
{
    int i;
    QPoint center;

    //... linear transformation ...
    Change_Corner(XY);


    for(i=0; i<4; i++)
    {
        //center.setX(XY.at(2*rep_Point.at(i)));
        //center.setY(XY.at(2*rep_Point.at(i)+1));
        center = XY_corner.at(i);

        Corner *p_corner = corners_Mask.at(i);

        p_corner->x_corner->blockSignals(true);
        p_corner->y_corner->blockSignals(true);

        p_corner->x_corner->setValue(center.x());
        p_corner->y_corner->setValue(H_IMAGE - center.y());

        p_corner->x_corner->blockSignals(false);
        p_corner->y_corner->blockSignals(false);
    }


    //... digit soft ...
    Digitise_VideoImage(XY, Rx, Ry, offset_XY, W_IMAGE, H_IMAGE, Set_Fluor->currentIndex(), BUF_Video, Digit_soft);
    if(Soft_digit->isChecked() && Digit_soft.size() == type_dev) data_Bars(Digit_soft.data(), type_dev);

}
//-----------------------------------------------------------------------------
//--- Change_Corner()
//-----------------------------------------------------------------------------
void Video::Change_Corner(QVector<short> &xy)
{
    int i,j;
    QPoint first,second;
    rt_Plate Plate;
    int col,row;

    Plate.PlateSize(type_dev, row, col);

    QVector<short> left_vec(2*row), right_vec(2*row);


    //... left ...
    first = XY_corner.at(0);
    second = XY_corner.at(2);
    Calculate_SingleLength(first, second, row, left_vec.data());

    //... right ...
    first = XY_corner.at(1);
    second = XY_corner.at(3);
    Calculate_SingleLength(first, second, row, right_vec.data());

    //... all ...
    for(i=0; i<row; i++)
    {
        first.setX(left_vec.at(2*i));
        first.setY(left_vec.at(2*i+1));
        second.setX(right_vec.at(2*i));
        second.setY(right_vec.at(2*i+1));
        Calculate_SingleLength(first, second, col, xy.data() + i*col*2);
    }
}

//-----------------------------------------------------------------------------
//--- Calculate_SingleLength()
//-----------------------------------------------------------------------------
void Video::Calculate_SingleLength(QPoint p1, QPoint p2, int N, short *vec)
{
    int i;
    double dx, dy;    

    dx = (double)(p2.x() - p1.x())/(double)(N-1);
    dy = (double)(p2.y() - p1.y())/(double)(N-1);

    for(i=0; i<N; i++)
    {        
        vec[2*i] =    (short)(p1.x() + Round(i*dx));
        vec[2*i+1] =  (short)(p1.y() + Round(i*dy));
    }    
    vec[2*(N-1)] =    (short)(p2.x());
    vec[2*(N-1)+1] =  (short)(p2.y());
}

//-----------------------------------------------------------------------------
//--- Clear_Image()
//-----------------------------------------------------------------------------
void Video::Clear_Image()
{
    BUF_Video.fill(0);

    Image->fill(Qt::white);
    image_Widget->update();

    // 3D
    channel_Bars(Set_Fluor->currentIndex());


}
//-----------------------------------------------------------------------------
//--- Load_Fluor()
//-----------------------------------------------------------------------------
void Video::Load_Fluor()
{
    int i;
    QIcon icon;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    int first_ch = -1;

    Set_Fluor->blockSignals(true);
    Set_Fluor->clear();

    for(i=0; i<COUNT_CH; i++)
    {
        Set_Fluor->addItem(fluor_name[i]);

        if(StyleApp == "fusion")
        {
            switch(i)
            {
            case 0:  icon.addFile(":/images/flat/fam_flat.png");   break;
            case 1:  icon.addFile(":/images/flat/hex_flat.png");   break;
            case 2:  icon.addFile(":/images/flat/rox_flat.png");   break;
            case 3:  icon.addFile(":/images/flat/cy5_flat.png");   break;
            case 4:  icon.addFile(":/images/flat/cy55_flat.png");   break;
            default: icon.addFile(NULL);   break;
            }
        }
        else
        {
            switch(i)
            {
            case 0:  icon.addFile(":/images/fam.png");   break;
            case 1:  icon.addFile(":/images/hex.png");   break;
            case 2:  icon.addFile(":/images/rox.png");   break;
            case 3:  icon.addFile(":/images/cy5.png");   break;
            case 4:  icon.addFile(":/images/cy55.png");   break;
            default: icon.addFile(NULL);   break;
            }
        }

        Set_Fluor->setItemIcon(i,icon);

        if(!(active_channels & (1<<i)))
        {
           Set_Fluor->setItemData(i,0,Qt::UserRole - 1);
        }
        else
        {
            if(first_ch < 0) first_ch = i;
        }
    }

    Set_Fluor->blockSignals(false);
    Set_Fluor->setCurrentIndex(first_ch);
}

//-----------------------------------------------------------------------------
//--- Get_InfoDevice
//-----------------------------------------------------------------------------
void Video::Get_InfoDevice(QMap<QString, QString> *map, bool save_map)
{
    int i,j;
    QString text, label;
    QString str_median;
    QByteArray buf;
    QByteArray buf_median;
    int num_sec;
    std::vector<short> std_vec;
    int left;
    rt_Plate Plate;
    int col,row;
    QPoint center;
    QPoint point;
    int count_sec = 96;
    bool sts;

    Plate.PlateSize(type_dev, row, col);
    QVector<short> XY_temp;
    XY_temp.fill(0,row*col*2);


    //... FHW ...
    qDebug() << "map_DeviceInfo: " << map->value(INFODEV_devHW, "...");
    FHW = 3.0;
    if(map->value(INFODEV_devHW, "...").contains("v4.0")) FHW = 4.0;

    BUF_Video.resize(H_IMAGE*W_IMAGE);
    BUF_Video.fill(0);
    image_Widget->setFixedSize(W_IMAGE_COEF, H_IMAGE);
    *Image = Image->scaled(W_IMAGE_COEF, H_IMAGE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    Image->fill(Qt::white);

    zoom_h = 27;
    if(FHW == 4.0) zoom_h = 52;


    image_Widget->COEF_IMAGE = COEF_IMAGE;
    foreach(Corner *corner, corners_Mask)
    {
        corner->W_IMAGE = W_IMAGE;
        corner->H_IMAGE = H_IMAGE;
    }

    Video_Box->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    Video_Box->update();
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    update();
    repaint();
    qApp->processEvents();

    //qDebug() << "Video:" << LEFT_OFFSET << W_IMAGE << H_IMAGE << COEF_IMAGE << W_IMAGE_COEF << W_REALIMAGE << H_REALIMAGE << TOP_OFFSET;
    //...


    union
    {
        Device_Par Device_Param;
        unsigned char byte_buf[512];
    }DEVICE_PAR;

    union
    {
        Device_ParMedian Device_ParamMedian;
        unsigned char byte_buf[512];
    }DEVICE_PAR_MEDIAN;

    text = map->value(INFODEV_parameters,"");
    if(text.isEmpty())
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Critical);
        message.setText("Attention! Is absent information about device!");
        message.exec();
        return;
    }

    buf = QByteArray::fromBase64(text.toStdString().c_str());
    num_sec = div(buf.size(),512).quot;   

    str_median = map->value("median", "");
    buf_median = QByteArray::fromBase64(str_median.toStdString().c_str());
    //qDebug() << "Get_InfoDevice: buf_median = " << buf_median;

    left = LEFT_OFFSET;
    if(type_dev == 48 || type_dev == 192) left = LEFT_OFFSET_DT48;
    if(type_dev == 48) count_sec = 48;

    //... clear all ...
    XY_initial.clear();
    XY_initial.reserve(count_sec*2*num_sec);
    XY.clear();
    XY.reserve(count_sec*2*num_sec);
    offset_XY.clear();
    list_Label.clear();
    rep_Point.clear();
    XY_corner.clear();
    XY_median.clear();

    if(save_map)
    {
        map_DeviceInfo.clear();
        map_DeviceInfo.insert(INFODEV_parameters, map->value(INFODEV_parameters,""));
        map_DeviceInfo.insert("median", map->value("median",""));
    }

    //... Geometry ...
    for(i=0; i<num_sec; i++)
    {
        for(j=0; j<512; j++) DEVICE_PAR.byte_buf[j] = buf.at(j+i*512);

        std_vec.clear();
        std_vec.assign(DEVICE_PAR.Device_Param.X_Y, DEVICE_PAR.Device_Param.X_Y + count_sec*2);

        for(j=0; j<count_sec; j++)
        {            
            XY.append(std_vec.at(2*j) - left);            
            XY.append(std_vec.at(2*j+1) - TOP_OFFSET);
        }
        if(i == 0)  // only for first sector
        {            
            Rx = DEVICE_PAR.Device_Param.Rx;
            Ry = DEVICE_PAR.Device_Param.Ry;
            for(j=0; j<COUNT_CH; j++)
            {                
                offset_XY.append(DEVICE_PAR.Device_Param.d_xy[j*2+1]);// * (-1));
                offset_XY.append(DEVICE_PAR.Device_Param.d_xy[j*2]);
            }
        }
    }
    //qDebug() << "XY: " << XY;
    std_vec.clear();

    //... Copy to Initial buffer ...
    XY_initial = XY.mid(0);
    Rx_initial = Rx;
    Ry_initial = Ry;

    //... Labels ...    
    rep_Point << 0 << (col-1) << (row-1)*col << row*col - 1;

    for(i=0; i<4; i++)
    {
        center.setX(XY.at(2*rep_Point.at(i)));
        center.setY(XY.at(2*rep_Point.at(i)+1));
        XY_corner.append(center);
    }

    switch(type_dev)
    {
    default:
    case 96:
                label = LABEL_96;
                break;
    case 384:
                label = LABEL_384;
                break;
    case 48:
                label = LABEL_48;
                break;
    case 192:
                label = LABEL_192;
                break;
    }
    list_Label = label.split(',');
    Rep = -1;
    drag_Mode = false;

    //... Corners ...
    for(i=0; i<4; i++)
    {
        center = XY_corner.at(i);
        Corner *p_corner = corners_Mask.at(i);

        p_corner->x_corner->blockSignals(true);
        p_corner->y_corner->blockSignals(true);

        p_corner->x_corner->setRange(0+Rx, W_IMAGE-Rx);
        p_corner->y_corner->setRange(0+Ry, H_IMAGE-Ry);

        p_corner->x_corner->setValue(center.x());
        p_corner->y_corner->setValue(H_IMAGE - center.y());        

        p_corner->x_corner->blockSignals(false);
        p_corner->y_corner->blockSignals(false);

        text = list_Label.at(i) + ":";
        p_corner->name_corner->setText(text);
    }

    //... Radius ...
    Rx_spot->blockSignals(true);
    Ry_spot->blockSignals(true);
    Rx_spot->setValue(Rx);
    Ry_spot->setValue(Ry);
    Rx_spot->blockSignals(false);
    Ry_spot->blockSignals(false);

    //... Digit Buffers ...
    Digit.clear();
    Digit.resize(type_dev);
    Digit_soft.clear();
    Digit_soft.resize(type_dev);

    //... Median ...
    sts = false;
    if(buf_median.size() >= 512)
    {
        for(i=0; i<512; i++) DEVICE_PAR_MEDIAN.byte_buf[i] = buf_median.at(i);
        str_median = QString(DEVICE_PAR_MEDIAN.Device_ParamMedian.id_median);
        //qDebug() << "str_median: " << str_median.size() << str_median;

        if(str_median.contains("median"))
        {
            for(i=0; i<4; i++)
            {
                point.setX(DEVICE_PAR_MEDIAN.Device_ParamMedian.median_xy[i]);
                point.setY(DEVICE_PAR_MEDIAN.Device_ParamMedian.median_xy[i+4]);
                XY_median.append(point);
            }
            sts = true;
        }
    }
    if(!sts)
    {
        for(i=0; i<4; i++)
        {
            point = QPoint(0,0);
            XY_median.append(point);
        }
    }

    sts = false;
    for(i=0; i<4; i++)
    {
        Median *median = median_Mask.at(i);

        median->x_median->blockSignals(true);
        median->y_median->blockSignals(true);

        median->x_median->setValue(XY_median.at(i).x());
        median->y_median->setValue(XY_median.at(i).y());
        if(median->x_median->value() != 0 || median->y_median->value() != 0) sts = true;

        median->x_median->blockSignals(false);
        median->y_median->blockSignals(false);
    }

    Median_Box->blockSignals(true);
    Median_Box->setChecked(sts);
    if(sts) Preference_Box->setTabIcon(2, QIcon(":/images/flat/checkbox.png"));
    else Preference_Box->setTabIcon(2, QIcon(":/images/flat/checkbox_null.ico"));
    Median_Box->blockSignals(false);

    if(!zoom->isHidden()) Draw_ZoomVideoImage(zoom_tube);
    Digitise_VideoImage(XY, Rx, Ry, offset_XY, W_IMAGE, H_IMAGE, Set_Fluor->currentIndex(), BUF_Video, Digit_soft);
    if(Soft_digit->isChecked() && Digit_soft.size() == type_dev) data_Bars(Digit_soft.data(), type_dev);

    Check_GeometryChange();
}
//-----------------------------------------------------------------------------
//--- Round(double) -> int
//-----------------------------------------------------------------------------
int Video::Round(double val)
{
    int res;

    res = qFloor(val+0.5);

    return(res);
}

//-----------------------------------------------------------------------------
//--- Show_DigitPlate()
//-----------------------------------------------------------------------------
void Video::Show_DigitPlate()
{
    if(dll_handle)
    {
        if(handle_3D)
        {
            ::SetWindowPos(handle_3D,HWND_TOPMOST,2,2,400,300,SWP_SHOWWINDOW);

            //... Draw 3D bars ...
            plate_Bars(type_dev);
            channel_Bars(Set_Fluor->currentIndex());
            Toggle_digit(DT_digit->isChecked());
            show_Bars();
        }
    }
}
//-----------------------------------------------------------------------------
//--- Show_ZoomDialog()
//-----------------------------------------------------------------------------
void Video::Show_ZoomDialog()
{
    if(zoom->isHidden())
    {
        zoom->show();
        Draw_ZoomVideoImage(zoom_tube);
    }
}

//-----------------------------------------------------------------------------
//--- Digitise_VideoImage(...)
//-----------------------------------------------------------------------------
void Video::Digitise_VideoImage(QVector<short> &xy,
                                qint16 rx,
                                qint16 ry,
                                QVector<qint8> &offset,
                                int width,
                                int height,
                                int ch,
                                QVector<int> &video_buf,
                                QVector<quint16> &digit)
{

    int i,j,k;
    int X,Y;
    double sum;
    double x,y;
    int N, ovf_num, value;
    quint16 val_16;

    int value_Overflow = 3700;
    int count = xy.size()/2;
    //count = 48;
    int dx = offset.at(2*ch);
    int dy = offset.at(2*ch+1);

    for(i=0; i<count; i++)
    {
        sum = 0.;
        ovf_num = 0;
        N = 0;
        X = xy.at(i*2) + dx;
        Y = /*height - */(xy.at(i*2+1) + dy);

        for(j = Y - ry; j <= Y + ry; j++)       // cycle for height tube
        {
            y = j - Y;
            for(k = X - rx; k <= X + rx; k++)   // cycle for width tube
            {
                x = k - X;                

                if(((x*x)/(rx*rx) + (y*y)/(ry*ry)) <= 1.)
                {                    
                    value = video_buf.at(j*width+k) & 0x0fff;
                    sum += value;
                    N++;
                    if((value & 0x0fff) > value_Overflow) ovf_num++;
                }
            }
        }
        if(N)
        {
            value = qRound(sum/N);
            if(ovf_num > N*0.1)     // nonlinear region: 10%
            {
                value |= 0x8000;
            }
        }
        else value = sum;

        val_16 = value & 0x8fff;
        //val_16 = qrand() & 0x0fff;
        digit.replace(i,val_16);
    }

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void FluorVideo_ItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QRect rect = option.rect;
    QString text = index.data().toString();
    QFont font = qApp->font();
    QPixmap pixmap(":/images/fam.png");
    int row = index.row();

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(255,255,255)); //QColor(255,255,225)
    }
    painter->setFont(font);

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    //font = GraphPlot::font_plot;
    //font = painter->font();
    //font.setBold(true);
    //painter->setFont(QFont("Comic Sans MS", 10, QFont::Normal));

    if(option.state & QStyle::State_Enabled)
    {
        if(style == "fusion")
        {
            switch(row)
            {
            case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
            case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
            case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
            case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
            case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
            }
        }
        else
        {
            switch(row)
            {
            case 0:  pixmap.load(":/images/fam.png");    break;
            case 1:  pixmap.load(":/images/hex.png");    break;
            case 2:  pixmap.load(":/images/rox.png");    break;
            case 3:  pixmap.load(":/images/cy5.png");    break;
            case 4:  pixmap.load(":/images/cy55.png");   break;
            }
        }

    }
    else
    {
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
        if(style == "fusion") pixmap.load(":/images/flat/disable_flat.png");
        else pixmap.load(":/images/disable.png");
    }

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);


    painter->drawPixmap(0, rect.y()+2, pixmap);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Image_Widget::Image_Widget(QImage *picture, QWidget *parent): QWidget(parent)
{
    p_video = (Video*)parent;
    image = picture;

    setMouseTracking(true);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Widget::paintEvent(QPaintEvent *e)
{
    QPoint center;

    QPainter painter(this);

    //... image ...
    painter.drawImage(0,0,*image);

    //... mask ...
    p_video->Draw_Mask(&painter, p_video->Rep);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Widget::mousePressEvent(QMouseEvent *e)
{
    //qDebug() << e->pos() << this->size() << p_video->XY_corner;

    int i;
    int x = e->pos().x();
    int y = e->pos().y();
    int X,Y;
    int count = p_video->type_dev;

    QVector<QPoint> *p_xy = &(p_video->XY_corner);
    QVector<short>  *XY = &(p_video->XY);
    QPoint xy;
    short rx = p_video->Rx;
    short ry = p_video->Ry;

    rx = qFloor((double)(rx)/COEF_IMAGE + 0.5);

    p_video->Rep = -1;

    for(i=0; i<4; i++)
    {
        xy = p_xy->at(i);
        X = qFloor((double)(xy.x())/COEF_IMAGE);
        Y = xy.y();

        if(x >= (X - rx) &&
           x <= (X + rx) &&
           y >= (Y - ry) &&
           y <= (Y + ry))
        {
            p_video->drag_Mode = true;
            p_video->Rep = i;
            break;
        }
    }

    p_video->zoom_tube = -1;
    for(i=0; i<count; i++)
    {
        X = qFloor((double)(XY->at(i*2))/COEF_IMAGE);
        Y = XY->at(i*2+1);

        if(x >= (X - rx) &&
           x <= (X + rx) &&
           y >= (Y - ry) &&
           y <= (Y + ry))
        {
            p_video->zoom_tube = i;
            p_video->Draw_ZoomVideoImage(p_video->zoom_tube);
            break;
        }
        else p_video->Draw_ZoomVideoImage(p_video->zoom_tube);
    }

    update();   // call paintEvent()
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Widget::mouseReleaseEvent(QMouseEvent *e)
{
    p_video->drag_Mode = false;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Widget::mouseMoveEvent(QMouseEvent *e)
{
    int rep = p_video->Rep;
    int x = e->pos().x();
    int y = e->pos().y();
    QPoint point(x,y);

    short rx = p_video->Rx;
    short ry = p_video->Ry;

    if(p_video->drag_Mode && e->buttons() == Qt::LeftButton &&
       rep >= 0 && rep < 4 &&
       x - rx/2 >= 0 && x + rx/2 <= width() &&
       y - ry >= 0 && y + ry <= height())
    {
        x = qFloor((double)(x) * COEF_IMAGE);
        point.setX(x);

        p_video->XY_corner.replace(rep, point);
        p_video->Recalculate_Geometry();
        if(p_video->Median_Box->isChecked())
        {
            p_video->Calculate_Median();
        }

        update();   // call paintEvent()

        if(!p_video->zoom->isHidden()) p_video->Draw_ZoomVideoImage(p_video->zoom_tube);

        p_video->Check_GeometryChange();
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Corner::Corner(int id, QWidget *parent): QGroupBox(parent)
{

    Index = id;
    Parent = parent;

    name_corner = new QLabel(this);
    x_corner = new QSpinBox(this);
    y_corner = new QSpinBox(this);

    x_corner->setRange(0,W_IMAGE);
    y_corner->setRange(0,H_IMAGE);

    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);
    layout->setMargin(2);
    layout->setSpacing(2);
    layout->addWidget(name_corner);
    layout->addStretch(1);
    layout->addWidget(x_corner);
    layout->addWidget(y_corner);

    //setStyleSheet("QGroupBox {border: 0px solid gray;}");
    //name_corner->setFont(QFont("Times New Roman", 10, QFont::Bold));
    x_corner->setFixedWidth(70);
    y_corner->setFixedWidth(70);
    //x_corner->setStyleSheet("QSpinBox {background-color: rgb(128,128,128)}");

    x_corner->setFont(QFont("Times New Roman", 10, QFont::Bold));
    y_corner->setFont(QFont("Times New Roman", 10, QFont::Bold));

    connect(this, SIGNAL(sChange_XY(int)), (Video*)Parent, SLOT(Change_XY(int)));
    connect(x_corner, SIGNAL(valueChanged(int)), this, SLOT(Change_center()));
    connect(y_corner, SIGNAL(valueChanged(int)), this, SLOT(Change_center()));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Corner::~Corner()
{
    delete name_corner;
    delete x_corner;
    delete y_corner;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Corner::Change_center()
{
    emit sChange_XY(Index);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Median::Median(int id, QWidget *parent): QGroupBox(parent)
{
    Index = id;
    Parent = parent;
    QImage image;
    QString fn = QString(":/images/flat/median_%1.png").arg(id);

    image.load(fn);

    id_image = new QLabel(this);
    id_image->setPixmap(QPixmap::fromImage(image));

    x_median = new QSpinBox(this);
    y_median = new QSpinBox(this);



    x_median->setRange(-20,20);
    y_median->setRange(-20,20);

    QHBoxLayout *layout = new QHBoxLayout;
    setLayout(layout);
    layout->setMargin(2);
    layout->setSpacing(2);
    layout->addWidget(id_image);
    layout->addStretch(1);
    layout->addWidget(x_median);
    layout->addWidget(y_median);

    //setStyleSheet("QGroupBox {border: 0px solid gray;}");
    //name_corner->setFont(QFont("Times New Roman", 10, QFont::Bold));
    x_median->setFixedWidth(70);
    y_median->setFixedWidth(70);
    //x_corner->setStyleSheet("QSpinBox {background-color: rgb(128,128,128)}");

    x_median->setFont(QFont("Times New Roman", 10, QFont::Bold));
    y_median->setFont(QFont("Times New Roman", 10, QFont::Bold));

    connect(this, SIGNAL(sChange_Median(int)), (Video*)Parent, SLOT(Change_Median(int)));
    connect(x_median, SIGNAL(valueChanged(int)), this, SLOT(Change_Median()));
    connect(y_median, SIGNAL(valueChanged(int)), this, SLOT(Change_Median()));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Median::~Median()
{
    delete id_image;
    delete x_median;
    delete y_median;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Median::Change_Median()
{
    emit sChange_Median(Index);
}


//-----------------------------------------------------------------------------
//--- Calculate_Median()
//-----------------------------------------------------------------------------
void Video::Calculate_Median()
{
    int i;
    int num;
    rt_Plate Plate;
    int col,row;
    QPoint point;
    Plate.PlateSize(type_dev, row, col);

    median_Point.clear();

    // 0.
    num = col/2-1;
    point.setX(XY.at(num*2));
    point.setY(XY.at(num*2+1));
    //ReCalculate_MedianPoint(col/2-1, 0, 0, (col-1)*2, col, row, point);
    median_Point.append(point);

    // 1.
    num = col/2-1 + col*(row-1);
    point.setX(XY.at(num*2));
    point.setY(XY.at(num*2+1));
    //ReCalculate_MedianPoint(col/2-1, 1, col*(row-1)*2, col*(row-1)*2+(col-1)*2, col, row, point);
    median_Point.append(point);

    // 2.
    num = col*(row/2);
    point.setX(XY.at(num*2));
    point.setY(XY.at(num*2+1));
    //ReCalculate_MedianPoint(row/2-1, 2, (row-1)*col*2, 0, col, row, point);
    median_Point.append(point);

    // 3.
    num = col*(row/2) + col - 1;
    point.setX(XY.at(num*2));
    point.setY(XY.at(num*2+1));
    //ReCalculate_MedianPoint(row/2-1, 3, col*(row-1)*2+(col-1)*2, (col-1)*2, col, row, point);
    median_Point.append(point);

    if(Median_Box->isChecked()) Recalculate_MedianPoint();
    Recalculate_GeometryMedian();

}
//-----------------------------------------------------------------------------
//--- ReCalculate_MedianPoint()
//-----------------------------------------------------------------------------
void Video::Recalculate_MedianPoint()
{
    int i;
    QPoint point;

    for(i=0; i<4; i++)
    {
        point.setX(median_Point.at(i).x() + median_Mask.at(i)->x_median->value());
        point.setY(median_Point.at(i).y() - median_Mask.at(i)->y_median->value());
        median_Point.replace(i, point);
    }
}

//-----------------------------------------------------------------------------
//--- ReCalculate_MedianPoint(...)
//-----------------------------------------------------------------------------
void Video::ReCalculate_MedianPoint(int num_point,
                                    int index_point,
                                    int id_0,
                                    int id_1,
                                    int col,
                                    int row,
                                    QPoint &Point)
{
    int i,j=0;
    int DX,dx = 0;
    int res = 0;
    int x;
    double s,q,dvalue,b1,ds;
    int delta = 1;
    int lim = 200;



    switch(index_point)
    {
    case 0:     // top & bottom   ===========================================
    case 1:
                x = Point.x();
                s = XY.at(id_1) - XY.at(id_0);
                DX = Round((double)s/(double)(col-1)*num_point) + XY.at(id_0) - x;

                if(DX == 0) return;

                if(DX < 0) delta = -delta;

                while(res != DX && j<lim)
                {
                    dx += delta;
                    q = (double)dx/s + 1.;
                    b1 = s*(1.-q)/(1.-pow(q,(col-1)));
                    dvalue = b1*(1.-pow(q,(col/2-1)))/(1.-q);
                    ds = s/(double)(col-1)*num_point - dvalue;
                    res = Round(ds);
                    j++;
                }
                if(j >= lim) return;
                Point.setX(Round((double)s/(double)(col-1)*num_point) + XY.at(id_0) - dx);

                break;

    case 2:     // left & right   =============================================
    case 3:

                x = Point.y();
                s = XY.at(id_1+1) - XY.at(id_0+1);
                DX = Round((double)s/(double)(row-1)*num_point) + XY.at(id_0+1) - x;

                if(DX == 0) return;

                if(DX < 0) delta = -delta;

                while(res != DX && j<lim)
                {
                    dx += delta;
                    q = (double)dx/s + 1.;
                    b1 = s*(1.-q)/(1.-pow(q,(row-1)));
                    dvalue = b1*(1.-pow(q,(row/2-1)))/(1.-q);
                    ds = s/(double)(row-1)*num_point - dvalue;
                    res = Round(ds);
                    j++;
                }
                if(j >= lim) return;
                Point.setY(Round((double)s/(double)(row-1)*num_point) + XY.at(id_0+1) - x);

                break;
          }
}

//-----------------------------------------------------------------------------
//--- Recalculate_GeometryMedian()
//-----------------------------------------------------------------------------
void Video::Recalculate_GeometryMedian()
{
    int i,j,jj,k;
    int dy, Y, y, dY, Yc, DY;
    int dx, X, Xc, s, x;
    double q,b1,Sn,dSn,bn;
    double value;
    QPoint Point;
    int id_0, id_1;
    int type_p;
    int num;
    int DX;
    double Tg_a;
    double dvalue;
    double s_tmp;
    int res;
    double S,B1,B2;
    rt_Plate Plate;
    int row, col;

    Plate.PlateSize(type_dev, row, col);

    for(k=0; k<4; k++)
    {
        type_p = k;

        switch(type_p)
        {
        default:
        case 0:                                     // Top
                    id_0 = 0;
                    id_1 = (col-1)*2;
                    Point = median_Point.at(0);
                    break;

        case 1:                                     // Bottom
                    id_0 = col*(row-1)*2;
                    id_1 = id_0 + (col-1)*2;
                    Point = median_Point.at(1);
                    break;

        case 2:                                     // Left
                   id_0 = (row-1)*col*2;
                   id_1 = 0;
                   Point = median_Point.at(2);
                   break;

        case 3:                                     // Right
                   id_0 = col*(row-1)*2 + (col-1)*2;
                   id_1 = (col-1)*2;
                   Point = median_Point.at(3);
                   break;
        }

        //... Top & Bottom ...
        if(type_p < 2)
        {
            for(j=0; j<row; j++)
            {
                jj = j;
                if(div(type_p,2).rem == 1) jj = row - 1 - j;

                if(!j)
                {
                    s = XY.at(id_1) - XY.at(id_0);
                    dx = Round((double)s/(double)(col-1)*(col/2-1)) + XY.at(id_0) - Point.x();
                    dy = Point.y() - (Round((double)(XY.at(id_1+1) - XY.at(id_0+1))/(double)(col-1)*(col/2-1)) + XY.at(id_0+1));
                    DX = dx;
                }

                s = XY.at((jj+1)*col*2-2) - XY.at(jj*col*2);
                q = (double)dx/(double)s + 1.;
                q = (1.-q)*j/(row-1.) + q;
                if(q == 1) b1 = (double)s/(col-1);
                else b1 = s*(1.-q)/(1.-pow(q,(col-1)));

                value = dy;
                value = value - value/(row-1)*j;
                DY = Round(value);

                for(i=0; i<col/2; i++)
                {
                    dY = Round(((-1.)*(double)DY)/pow(col/2-1,2)*i*i) + DY;        // 5 points 5*5=25

                    XY.replace(col+2*i+1+jj*col*2, XY.at(col+2*i+1+jj*col*2) + dY);
                    XY.replace(col-2*i-1+jj*col*2, XY.at(col-2*i-1+jj*col*2) + dY);
                }


                for(i=0; i<col; i++)
                {
                    if(i != 0 && i < col-1)
                    {
                        if(q == 1)  Sn = b1*i;
                        else Sn = b1*(1.-pow(q,i))/(1.-q);
                        dSn = (double)s/(double)(col-1.)*double(i) - Sn;
                        XY.replace(2*i+jj*col*2, XY.at(2*i+jj*col*2) - Round(dSn));
                    }
                }
            }
        }
        else
        {
            for(j=0; j<col; j++)
            {
                jj = j;
                if(div(type_p,2).rem == 1) jj = col - 1 - j;

                if(!j)
                {
                    s = XY.at(id_1+1) - XY.at(id_0+1);
                    dx = Round((double)s/(double)(row-1)*(row/2-1)) + XY.at(id_0+1) - Point.y();
                    dy = Point.x() - (Round((double)(XY.at(id_1) - XY.at(id_0))/(double)(row-1)*(row/2-1)) + XY.at(id_0));
                    DX = dx;
                }

                s = XY.at(jj*2+1) - XY.at(jj*2+(row-1)*col*2+1);
                q = (double)dx/(double)s + 1.;
                q = (1.-q)*j/(col-1.) + q;
                if(q == 1) b1 = (double)s/(row-1);
                else b1 = s*(1.-q)/(1.-pow(q,(row-1)));

                value = dy;
                value = value - value/(col-1)*j;
                DY = Round(value);

                for(i=0; i<row/2; i++)
                {
                    dY = Round(((-1.)*(double)DY)/pow(row/2-1,2)*i*i) + DY;    // 3 points 3*3=9
                    XY.replace((row/2-1-i)*col*2+jj*2, XY.at((row/2-1-i)*col*2+jj*2) + dY);
                    XY.replace((row/2+i)*col*2+jj*2, XY.at((row/2+i)*col*2+jj*2) + dY);
                }

                for(i=0; i<row; i++)
                {
                    if(i != 0 && i < row-1)
                    {
                        if(q == 1)  Sn = b1*i;
                        else Sn = b1*(1.-pow(q,i))/(1.-q);
                        dSn = (double)s/(double)(row-1)*(double)i - Sn;
                        XY.replace((row-1-i)*col*2+1+jj*2, XY.at((row-1-i)*col*2+1+jj*2) - Round(dSn));
                    }
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- Change_MedianRejime()
//-----------------------------------------------------------------------------
void Video::Change_MedianRejime()
{
    Change_Corner(XY);
    Calculate_Median();
    image_Widget->update();

    bool sts = Median_Box->isChecked();
    if(sts) Preference_Box->setTabIcon(2, QIcon(":/images/flat/checkbox.png"));
    else Preference_Box->setTabIcon(2, QIcon(":/images/flat/checkbox_null.ico"));

    if(!zoom->isHidden()) Draw_ZoomVideoImage(zoom_tube);
    Digitise_VideoImage(XY, Rx, Ry, offset_XY, W_IMAGE, H_IMAGE, Set_Fluor->currentIndex(), BUF_Video, Digit_soft);
    if(Soft_digit->isChecked() && Digit_soft.size() == type_dev) data_Bars(Digit_soft.data(), type_dev);

    Check_GeometryChange();
}
//-----------------------------------------------------------------------------
//--- Change_Median(int index)
//-----------------------------------------------------------------------------
void Video::Change_Median(int index)
{
    //qDebug() << "Change_Median: index = " << index;

    Change_Corner(XY);
    Calculate_Median();
    image_Widget->update();

    if(!zoom->isHidden()) Draw_ZoomVideoImage(zoom_tube);
    Digitise_VideoImage(XY, Rx, Ry, offset_XY, W_IMAGE, H_IMAGE, Set_Fluor->currentIndex(), BUF_Video, Digit_soft);
    if(Soft_digit->isChecked() && Digit_soft.size() == type_dev) data_Bars(Digit_soft.data(), type_dev);

    Check_GeometryChange();
}
//-----------------------------------------------------------------------------
//--- slot_ReadSector()
//-----------------------------------------------------------------------------
void Video::slot_ReadSector()
{
    int sector = 0x0419;

    emit ReadSector(sector, "median");
}
//-----------------------------------------------------------------------------
//--- slot_WriteSector()
//-----------------------------------------------------------------------------
void Video::slot_WriteSector()
{
    int sector = 0x0419;
    QString tim = QDateTime::currentDateTime().toString(" d MMMM dddd yyyy, h:mm:ss ");

    QByteArray ba;
    ba.resize(512);
    ba.fill(0);
    QByteArray ba_data = tim.toUtf8();
    ba.replace(0,ba_data.size(), ba_data);

    tim = ba.toBase64();

    emit WriteSector(sector, tim);
}
//-----------------------------------------------------------------------------
//--- Check_GeometryChange()
//-----------------------------------------------------------------------------
void Video::Check_GeometryChange()
{
    if(XY != XY_initial || Rx != Rx_initial || Ry != Ry_initial) SaveGeometry_Button->setDisabled(false);
    else SaveGeometry_Button->setDisabled(true);

    if(*flag_DeviceSettings == false) SaveGeometry_Button->setDisabled(true);
}
//-----------------------------------------------------------------------------
//--- Load_MASK()
//-----------------------------------------------------------------------------
void Video::Load_MASK()
{
    int i;
    QString FileName;
    QString text;
    short value;
    QStringList fields;
    QString dirName = qApp->applicationDirPath();
    bool ok;
    QPoint center;


    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    foreach(QString str, locations)
    {
        if(str.contains("ProgramData"))
        {
            dirName = str;
            break;
        }
    }

    FileName = QFileDialog::getOpenFileName(this,
                                            tr("Open file mask ..."),
                                            dirName,
                                            tr("Mask File (*.mrt)"));
    if(FileName.isEmpty()) return;

    // 1.
    Median_Box->setChecked(false);
    Change_MedianRejime();


    QFile file(FileName);

    if(file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);

        while(!in.atEnd())
        {
            QString line = in.readLine();

            if(line.startsWith("radius:"))
            {
                fields = line.split(QRegExp("\\s+"));
                if(fields.size() >= 3)
                {
                    Rx = QString(fields.at(1)).toShort(&ok);
                    Ry = QString(fields.at(2)).toShort(&ok);
                }
                else
                {
                    Rx = 10;
                    Ry = 5;
                }
            }
            if(line.startsWith("centers:"))
            {
                fields = line.split(QRegExp("\\s+"));

            }
            if(fields.size() >= 2*type_dev + 1)
            {
                for(i=0; i<2*type_dev; i++)
                {
                    value = QString(fields.at(i+1)).toShort(&ok);
                    XY.replace(i,value);
                }
            }

        }

        file.close();
    }




    // 2.
    Rx_spot->blockSignals(true);
    Rx_spot->setValue(Rx);
    Rx_spot->blockSignals(false);
    Ry_spot->blockSignals(true);
    Ry_spot->setValue(Ry);
    Ry_spot->blockSignals(false);

    // 3.
    for(i=0; i<4; i++)
    {
        center = XY_corner.at(i);
        center.setX(XY.at(2*rep_Point.at(i)));
        center.setY(XY.at(2*rep_Point.at(i)+1));
        Corner *p_corner = corners_Mask.at(i);

        p_corner->x_corner->blockSignals(true);
        p_corner->y_corner->blockSignals(true);

        p_corner->x_corner->setValue(center.x());
        p_corner->y_corner->setValue(H_IMAGE - center.y());

        p_corner->x_corner->setRange(0+Rx, W_IMAGE-Rx);
        p_corner->y_corner->setRange(0+Ry, H_IMAGE-Ry);

        p_corner->x_corner->blockSignals(false);
        p_corner->y_corner->blockSignals(false);
    }

    // 4. replot
    Rep = 0;
    image_Widget->update();
    Check_GeometryChange();

}
