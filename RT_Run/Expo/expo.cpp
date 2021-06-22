#include "expo.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Expo::Expo(QWidget *parent): QMainWindow(parent)
{
    readCommonSettings();
    setFont(qApp->font());

    type_dev = 96;
    active_channels = 0x0f;

    dll_handle = NULL;
    handle_3D = NULL;

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
    Main_Box->setLayout(Main_layout);
    Main_layout->setMargin(0);
    Main_layout->addWidget(main_Box);
    Main_layout->addWidget(buttons_Box);

    QHBoxLayout *main_layout = new QHBoxLayout;    
    main_Box->setLayout(main_layout);
    main_layout->setMargin(0);
    main_layout->setSpacing(4);

    //--- Expo_Box ---
    Image_Box = new QGroupBox(this);
    Image_Box->setMinimumSize(400,300);
    //Image_Box->installEventFilter(this);
    //Image_Box->setStyleSheet("QGroupBox {background-color: red}");
    ExpoControl_Box = new QGroupBox(this);

    Set_Fluor = new QComboBox(this);
    fluor_delegate = new FluorExpo_ItemDelegate;
    fluor_delegate->style = StyleApp;
    Set_Fluor->setItemDelegate(fluor_delegate);

    if(StyleApp == "fusion")
    {
        Make_Measure = new QPushButton(QIcon(":/images/flat/timer_flat.png"),tr("   Measure"), this);
        Make_Measure->setIconSize(QSize(24,24));
    }
    else
    {
        Make_Measure = new QPushButton(QIcon(":/images/foto.png"),tr("   Measure"), this);
        Make_Measure->setIconSize(QSize(33,24));
    }

    Set_Expo = new QSpinBox(this);
    Set_Expo->setSuffix(tr("  ms"));
    Set_Expo->setRange(0,10000);
    Set_Expo->setSingleStep(50);
    Set_Expo->setValue(1000);    
    Set_Expo->setFont(QFont("Times New Roman", 14, QFont::Bold));

    expo_TestCoefficient = new QLabel("()", this);
    expo_TestCoefficient->setFont(QFont("Times New Roman", 12, QFont::Normal));
    expo_TestCoefficient->setToolTip(tr("Exposure conversion factor: Test editor->Common->Device settings->Exposure"));

    main_layout->addWidget(Image_Box,1);
    main_layout->addWidget(ExpoControl_Box);

    QVBoxLayout *layout_VideoControl = new QVBoxLayout;
    ExpoControl_Box->setLayout(layout_VideoControl);
    layout_VideoControl->addWidget(Set_Fluor);
    layout_VideoControl->addWidget(Set_Expo);
    layout_VideoControl->addWidget(expo_TestCoefficient);
    layout_VideoControl->addStretch();
    layout_VideoControl->addWidget(Make_Measure);


    //... buttons_Box ...
    SaveExpo_Button = new QPushButton(tr("Save Expo"), this);
    SaveExpo_Button->setVisible(false);
    Cancel_Button = new QPushButton(tr("Cancel"), this);
    SaveAsDefault = new QCheckBox(tr("save as default"), this);
    SaveAsDefault->setVisible(false);
    QHBoxLayout *layout_buttons = new QHBoxLayout;
    layout_buttons->setMargin(4);
    buttons_Box->setLayout(layout_buttons);
    layout_buttons->addWidget(SaveAsDefault, 1, Qt::AlignLeft);
    layout_buttons->addStretch(1);
    layout_buttons->addWidget(SaveExpo_Button);
    layout_buttons->addWidget(Cancel_Button);

    setWindowFlags(this->windowFlags() & ~Qt::WindowMinimizeButtonHint
                                       & ~Qt::WindowMaximizeButtonHint);

    connect(Make_Measure, SIGNAL(clicked(bool)), this, SLOT(Get_Digit()));
    connect(SaveExpo_Button, SIGNAL(clicked(bool)), this, SLOT(Save_Parameters()));
    connect(Cancel_Button, SIGNAL(clicked(bool)), this, SLOT(Close_ByCancel()));
    connect(Set_Fluor, SIGNAL(currentIndexChanged(int)), this, SLOT(Change_Channel(int)));
    connect(Set_Expo, SIGNAL(valueChanged(int)), this, SLOT(Change_Expo(int)));
    connect(SaveAsDefault, SIGNAL(clicked(bool)), this, SLOT(Change_AsDefault(bool)));

    message.setFont(qApp->font());

    //... losd 3D_bars ...    
    //dll_handle = ::LoadLibraryW(L"plugins\\bars_3D.dll");


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Expo::~Expo()
{
    if(dll_handle)
    {
        if(handle_3D) destroy_Bars(handle_3D);
    }

    delete main_Box;
    delete buttons_Box;
    delete Main_Box;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Expo::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/expo_" + text + ".qm"))
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
//--- Close_ByCancel()
//-----------------------------------------------------------------------------
void Expo::Close_ByCancel()
{
    close_Cancel = true;
    close();

}
//-----------------------------------------------------------------------------
//--- closeEvent
//-----------------------------------------------------------------------------
void Expo::closeEvent(QCloseEvent *event)
{
    if(!close_Cancel)
    {
        event->ignore();
        QTimer::singleShot(100, this, SLOT(Close_ByCancel()));
        return;
    }

    if(dll_handle)
    {
        if(handle_3D)
        {
            //ShowWindow(handle_3D, SW_HIDE);            
            destroy_Bars(handle_3D);
            Sleep(100);
            handle_3D = NULL;
            Sleep(100);
        }
    }

    close_Cancel = false;
    event->accept();
}
//-------------------------------------------------------------------------------
//--- showEvent
//-------------------------------------------------------------------------------
void Expo::showEvent(QShowEvent *e)
{
    if(dll_handle)
    {
        if(handle_3D)
        {
            ::SetParent(handle_3D,(HWND)Image_Box->winId());
            ::SetWindowPos(handle_3D,HWND_TOP,2,2,Image_Box->width()-4,Image_Box->height()-4,SWP_SHOWWINDOW);
            show_Bars();
        }
    }    
    SaveAsDefault->setChecked(false);
    close_Cancel = false;    
}
//-------------------------------------------------------------------------------
//--- resizeEvent
//-------------------------------------------------------------------------------
void Expo::resizeEvent(QResizeEvent *e)
{
    if(handle_3D)
    {
        ::SetWindowPos(handle_3D,HWND_TOP,2,2,Image_Box->width()-4,Image_Box->height()-4,SWP_SHOWWINDOW);
    }
}
//-----------------------------------------------------------------------------
//--- Get_Digitization
//-----------------------------------------------------------------------------
void Expo::Get_Digitization(QMap<QString, QByteArray> *map)
{
    int i;

    if(map->size() == 0) return;

    int num_sec = div(type_dev,96).quot;
    if(type_dev == 48) num_sec = 1;

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

    //... digit ...
    Digit.clear();
    Digit.resize(type_dev);
    Digit.fill(0);
    for(i=0; i<num_sec; i++)
    {
        memcpy(Digit.data()+96*i, buf_digit.data()+128+512*i, 96*2);
    }

    data_Bars(Digit.data(), type_dev);

}

//-----------------------------------------------------------------------------
//--- Get_Digit
//-----------------------------------------------------------------------------
void Expo::Get_Digit()
{
    int ch = Set_Fluor->currentIndex();
    int expo = Set_Expo->value();

    channel_Bars(Set_Fluor->currentIndex());    // clear
    emit MakeMeasure_Request(ch, expo, 1);      // get digit data
}

//-----------------------------------------------------------------------------
//--- Get_InfoDevice
//-----------------------------------------------------------------------------
void Expo::Get_InfoDevice(QMap<QString, QString> *map)
{
    int i,j;
    QString text;
    QByteArray buf;

    std::vector<short> std_vec;


    union
    {
        Device_Par Device_Param;
        unsigned char byte_buf[512];
    }DEVICE_PAR;

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

    if(buf.size() < 512)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Critical);
        message.setText("Attention! Invalid size (512*x)!");
        message.exec();
        buf.clear();
        return;
    }

    //... read data ...
    for(i=0; i<512; i++) DEVICE_PAR.byte_buf[i] = buf.at(i);

    std_vec.clear();
    std_vec.assign((short int*)DEVICE_PAR.Device_Param.optics_ch,(short int*)DEVICE_PAR.Device_Param.optics_ch + 8*6);
    Expozition.clear();
    for(i=0; i<COUNT_CH; i++)
    {
        Expozition.append(qRound((double)(std_vec.at(i*6+4))*COEF_EXPO));   // first expo for i-channel
        Expozition.append(qRound((double)(std_vec.at(i*6+5))*COEF_EXPO));   // seecond expo for i-channel
    }
    Set_Expo->setValue(Expozition.at(Set_Fluor->currentIndex()*2));
    Set_ExpoTestCoeff();

    Expozition_initial.clear();
    Expozition_initial = Expozition.mid(0);
    Check_ExpoChange();

    //qDebug() << "expo" << Expozition;

    //... Draw 3D bars ...
    plate_Bars(type_dev);
    channel_Bars(Set_Fluor->currentIndex());
}
//-----------------------------------------------------------------------------
//--- Change_AsDefault
//-----------------------------------------------------------------------------
void Expo::Change_AsDefault(bool state)
{
    if(!state) return;

    QString text = tr("Attention! You can change the basic settings of the device! Continue?");

    message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    message.setDefaultButton(QMessageBox::No);
    message.setIcon(QMessageBox::Warning);
    message.setText(text);
    int res = message.exec();

    if(res == QMessageBox::No)
    {
        SaveAsDefault->blockSignals(true);
        SaveAsDefault->setChecked(false);
        SaveAsDefault->blockSignals(false);
    }
}

//-----------------------------------------------------------------------------
//--- Save_Parameters()
//-----------------------------------------------------------------------------
void Expo::Check_ExpoChange()
{
    bool sts = true;

    if(Expozition != Expozition_initial) sts = false;

    if(*flag_DeviceSettings == false) sts = true;
    SaveExpo_Button->setDisabled(sts);
    SaveAsDefault->setDisabled(sts);
}

//-----------------------------------------------------------------------------
//--- Save_Parameters()
//-----------------------------------------------------------------------------
void Expo::Save_Parameters()
{
    int i,j;
    QString data = "", text;
    quint32 value;

    union
    {
        Device_ParDefault Device_ParamDefault;
        unsigned char byte_buf[512];
    }DEVICE_PAR_DEFAULT;

    for(i=0; i<Expozition.size(); i++)
    {
        value = qRound((double)(Expozition.at(i))/(double)(COEF_EXPO));
        j = div(i,2).quot;
        if(!(active_channels & (1<<j))) value = 0;

        text = QString(" %1").arg(value);
        data += text;
    }
    data = data.trimmed();


    if(SaveAsDefault->isChecked())      // default exposition
    {
        for(i=0; i<512; i++) DEVICE_PAR_DEFAULT.byte_buf[i] = 0;
        strcpy(DEVICE_PAR_DEFAULT.Device_ParamDefault.id_default, "default");
        for(i=0; i<Expozition.size(); i++)
        {
            DEVICE_PAR_DEFAULT.Device_ParamDefault.Expo_def[i] = Expozition.at(i);
        }
        QByteArray ba = QByteArray((char*)(DEVICE_PAR_DEFAULT.byte_buf), 512);
        text = ba.toBase64();
        emit SaveParDefault("default", text);
    }

    emit SavePar(1,data);

    //qDebug() << "Save Expo: " << data << Expozition;

    close();
}

//-----------------------------------------------------------------------------
//--- Load_lib(HINSTANCE)
//-----------------------------------------------------------------------------
void Expo::Load_lib(HINSTANCE handle)
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
            handle_3D = create_Bars(0);            
        }        
    }
}

//-----------------------------------------------------------------------------
//--- Load_Fluor()
//-----------------------------------------------------------------------------
void Expo::Load_Fluor()
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
//--- Change_Channel
//-----------------------------------------------------------------------------
void Expo::Change_Channel(int ch)
{
    channel_Bars(ch);
    Set_Expo->setValue(Expozition.at(ch*2));
    Set_ExpoTestCoeff();
}
//-----------------------------------------------------------------------------
//--- Change_Expo
//-----------------------------------------------------------------------------
void Expo::Change_Expo(int value)
{
    int ch = Set_Fluor->currentIndex();    

    Expozition.replace(ch*2, value);
    Expozition.replace(ch*2+1, qRound((double)(value)/COEF_EXPO_1));    

    Check_ExpoChange();
    Set_ExpoTestCoeff();
}
//-----------------------------------------------------------------------------
//--- Set_ExpoTestCoeff()
//-----------------------------------------------------------------------------
void Expo::Set_ExpoTestCoeff()
{
    int ch = Set_Fluor->currentIndex();
    int value = Set_Expo->value();
    double coeff = (double)value/(double)expozition_Def->at(ch*2);

    expo_TestCoefficient->setText(QString("(%1)").arg(coeff,4,'f',2));

    //qDebug() << "change Expo:" << value << coeff << *expozition_Def;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void FluorExpo_ItemDelegate::paint(QPainter *painter,
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
