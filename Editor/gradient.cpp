#include "gradient.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Gradient::Gradient(QWidget *parent) : QDialog(parent)
{
    int i,j;
    QStringList name_dev;    
    name_dev  = QString(List_DEVICENAME).split(",");

    setFont(qApp->font());

    setStyleSheet(
                "QGroupBox {margin-top: 1ex; background: #FAFAFA; border: 1px solid #ddd; border-radius: 0px;}"
                "QGroupBox#Transparent {border: 1px solid transparent;}"
                "QDialog {background-color: #FAFAFA; border: 1px solid #ddd;}"
                "QComboBox {background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;}"
                //"QComboBox {border: 1px solid #ddd; border-radius: 0px;}"
                "QPushButton::hover {background-color: #fff;}"
                "QPushButton::disabled {background-color: #ccc;}"
                "QPushButton::enabled {background-color: #FAFAFA;}"
                "QPushButton::pressed {background-color: #ccc;}"
                "QPushButton {text-align: center;}"
                "QPushButton {min-width: 4em; margin:0; padding:5;}"
                "QPushButton {border: 1px solid #aaa; border-radius: 0px;}"
                "QSpinBox {selection-background-color: #d7d7ff; selection-color: black;}"
                "QDoubleSpinBox {selection-background-color: #d7d7ff; selection-color: black;}"
                "QTableWidget {selection-background-color: #FAFAFA;}"
                "QToolTip {background: #FFFFFF; border: 1px solid #ddd;}"
                //"QDoubleSpinBox {border: 1px solid #aaa; border-radius: 0px;}"
    );

    QHBoxLayout *layout = new QHBoxLayout();
    this->setLayout(layout);

    dll_handle = NULL;
    handle_3D = NULL;


    Image_Box = new QGroupBox(this);
    Image_Box->setObjectName("Transparent");
    //Image_Box->setMinimumSize(400,300);
    Control_Box = new QGroupBox(this);
    Control_Box->setFixedWidth(280);
    layout->addWidget(Image_Box);
    layout->addWidget(Control_Box);

    QVBoxLayout *layout_control = new QVBoxLayout();
    Control_Box->setLayout(layout_control);
    Device = new QComboBox(this);
    Device->setToolTip(tr("Choice device"));
    Device->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    Device->addItems(name_dev);
    for(i=1; i<name_dev.size(); i++) Device->setItemData(i, 0, Qt::UserRole - 1);
    Type_gradient = new QComboBox(this);
    Type_gradient->setToolTip(tr("Choice type of gradient"));
    Type_gradient->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    Type_gradient->addItem(tr("None"));
    Type_gradient->addItem(QIcon(":/images/grad.png"), tr("Vertical"));
    Type_gradient->addItem(QIcon(":/images/grad.png"), tr("Horizontal"));
    Type_gradient->addItem(QIcon(":/images/grad.png"), tr("Horizontal(8)"));
    Type_gradient->addItem(QIcon(":/images/grad_step.png"), tr("Differential gradient"));

    Tab_Temperature = new QTabWidget(this);
    Tab_Temperature->setFixedHeight(150);

    Box_Single = new QGroupBox(this);
    Box_Single->setObjectName("Transparent");
    QVBoxLayout *layout_single = new QVBoxLayout();
    Box_Single->setLayout(layout_single);
    set_SingleTemp = new QDoubleSpinBox(this);
    set_SingleTemp->setSuffix(tr("  °C"));
    set_SingleTemp->setDecimals(1);
    set_SingleTemp->setRange(-8.,8.);
    set_SingleTemp->setSingleStep(0.1);
    set_SingleTemp->setValue(0.);
    set_SingleTemp->setFont(QFont("Times New Roman", 14, QFont::Bold));
    set_SingleTemp->setFixedSize(100,40);
    layout_single->addWidget(set_SingleTemp, 0, Qt::AlignCenter);

    Diff_table = new QTableWidget(2,3,this);
    Diff_table->verticalHeader()->hide();
    Diff_table->horizontalHeader()->hide();
    Diff_table->setSelectionMode(QAbstractItemView::NoSelection);
    Diff_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Diff_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    Diff_table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    for(i=0; i<2; i++)
    {
        for(j=0; j<3; j++)
        {
            QWidget *box = new QWidget(this);
            QVBoxLayout *layout_box = new QVBoxLayout(box);
            layout_box->setMargin(3);

            Block_Temperature *block_Temp = new Block_Temperature(i*3+j, this);
            layout_box->addWidget(block_Temp, 0, Qt::AlignCenter);
            Diff_table->setCellWidget(i,j,box);
            connect(block_Temp, SIGNAL(sChange_TemperatureBlock(void*)), this, SLOT(Change_BlockTemperature(void*)));
        }
    }


    Tab_Temperature->addTab(Box_Single, tr("Gradient"));
    Tab_Temperature->addTab(Diff_table, tr("Differential gradient"));

    apply = new QPushButton(tr("Apply"), this);
    apply->setFocusPolicy(Qt::NoFocus);
    cancel = new QPushButton(tr("Cancel"), this);
    cancel->setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *layout_button = new QHBoxLayout();
    layout_button->setMargin(0);
    layout_button->addWidget(apply);
    layout_button->addWidget(cancel);

    layout_control->addWidget(Device);
    layout_control->addWidget(Type_gradient);
    layout_control->addWidget(Tab_Temperature);
    layout_control->addStretch(1);
    layout_control->addLayout(layout_button);


    Base_Temperature = new QLabel("100 °C", this);
    Base_Temperature->setToolTip(tr("Base Temperature"));
    Base_Temperature->setFont(QFont("Times New Roman", 14, QFont::Bold));
    Base_Temperature->setAlignment(Qt::AlignCenter);
    Base_Temperature->setFrameShape(QFrame::StyledPanel);
    Base_Temperature->setStyleSheet("QLabel {background-color: #88FAFAFA; color: black; border: 1px solid gray;}");
    Base_Temperature->move(layout->margin(), layout->margin());

    //QVBoxLayout *layout_image = new QVBoxLayout();
    //Image_Box->setLayout(layout_image);
    //layout_image->addWidget(Base_Temperature, 1, Qt::AlignTop | Qt::AlignLeft);


    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Gradient"));
    setWindowIcon(QIcon(":/images/DTm.ico"));
    //setWindowIcon(QIcon(":/images/RT.ico"));
    resize(700,350);

    connect(cancel, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(apply, SIGNAL(clicked(bool)), this, SLOT(Apply()));
    connect(set_SingleTemp, SIGNAL(valueChanged(double)), this, SLOT(Draw_Gradient()));
    connect(Type_gradient, SIGNAL(currentIndexChanged(int)), this, SLOT(Set_TabGradient(int)));
    connect(Type_gradient, SIGNAL(currentIndexChanged(int)), this, SLOT(Draw_Gradient()));
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Gradient::~Gradient()
{
    delete cancel;
    delete apply;
    delete Base_Temperature;
    delete set_SingleTemp;
    delete Box_Single;
    delete Diff_table;
    delete Tab_Temperature;
    delete Type_gradient;
    delete Device;
    delete Image_Box;
    delete Control_Box;
}
//-----------------------------------------------------------------------------
//--- Load_lib(HINSTANCE)
//-----------------------------------------------------------------------------
void Gradient::Apply()
{
    int i;
    double device_buf[3];
    int grad_buf[6];    
    QString temp_blocks = "";    
    int type = Type_gradient->currentIndex();
    std::vector<int> buf;

    gradient_Info->gradient = set_SingleTemp->value()*10.;
    gradient_Info->type = (gradient_Type)type;

    // ... calculate temperature blocks ...

    if(type < 4 && type > 0) GetDevVal(type-1, base, base+set_SingleTemp->value(), device_buf);
    switch(type)
    {
    default:    break;
    case 1:                         // Vertical
                grad_buf[0] = grad_buf[1] = grad_buf[2] = roundTo(device_buf[0] - base, 1) * 10;
                grad_buf[3] = grad_buf[4] = grad_buf[5] = roundTo(device_buf[1] - base, 1) * 10;
                break;
    case 2:                         // Horizontal
    case 3:                         // Horizontal(8)
                grad_buf[0] = grad_buf[3] = roundTo(device_buf[0] - base, 1) * 10;
                grad_buf[2] = grad_buf[5] = roundTo(device_buf[1] - base, 1) * 10;
                grad_buf[1] = grad_buf[4] = roundTo(device_buf[2] - base, 1) * 10;
                break;

    case 4:                         // Differential Gradient
                i=0;
                foreach(double value, BLOCKs)
                {
                    grad_buf[i] = roundTo(value - base, 1) * 10;
                    buf.push_back(abs(grad_buf[i]));
                    i++;
                }
                gradient_Info->gradient = *std::max_element(buf.begin(), buf.end());
                break;
    }
    for(i=0; i<6; i++) temp_blocks += QString("%1 ").arg(grad_buf[i]);
    gradient_Info->gradient_block = temp_blocks;

    //qDebug() << "grad: " << gradient_Info->gradient_block << gradient_Info->type << gradient_Info->gradient;

    setResult(QDialog::Accepted);
    close();
}
//-----------------------------------------------------------------------------
//--- GetDevVal(int, double, double, double *)
//-----------------------------------------------------------------------------
void Gradient::GetDevVal(int type, double t0, double t1, double *out)
{
    const double kd[3][2] = {{0.159,0.072},
                             {0.105,0.072},
                             {-0.061,-0.025}};

    out[0] = t0+(t1-t0)*kd[type][0];
    out[1] = t1-(t1-t0)*kd[type][1];

    out[2] = (out[0] + out[1])/2.;
}
//---------------------------------------------------------------------------
//  real temperature in block
//---------------------------------------------------------------------------
void Gradient::GetTubeVal(int type, double t0, double t1, double* out)
{
        int i;
        const int Tubes[3] =  {8,12,12};
        const double kt[3][12] = {
        {-0.207,-0.090,0.090,0.319,0.613,0.829,1.014,1.093,1.093,1.093,1.093,1.093},
        {-0.128,-0.062,0.056,0.185,0.345,0.505,0.645,0.763,0.875,0.977,1.059,1.087},
        {-0.128,-0.062,0.056,0.185,0.345,0.505,0.645,0.763,0.875,0.977,1.059,1.087}};

        for(i=0; i<Tubes[type]; i++)  out[i] = t0+(t1-t0)*kt[type][i];
}
//---------------------------------------------------------------------------
//  Change_BlockTemperature(double)
//---------------------------------------------------------------------------
void Gradient::Change_BlockTemperature(void *obj)
{
    Block_Temperature *block = (Block_Temperature*)obj;
    int id = block->Index;
    BLOCKs.replace(id, base + block->value());

    Draw_Gradient();
}

//-------------------------------------------------------------------------------
//--- Set_TabGradient(int)
//-------------------------------------------------------------------------------
void Gradient::Set_TabGradient(int index)
{
    int i;
    int id;

    for(i=0; i<Tab_Temperature->count(); i++) Tab_Temperature->setTabEnabled(i, false);
    switch(index)
    {
    default:    id = 0; break;
    case 4:     id = 1; break;
    }

    Tab_Temperature->setTabEnabled(id, true);
    Tab_Temperature->setCurrentIndex(id);

}

//-------------------------------------------------------------------------------
//--- Draw_Gradient()
//-------------------------------------------------------------------------------
void Gradient::Draw_Gradient()
{
    int i,j;
    int row = 8;
    int col = 12;
    int id;
    int color;
    int type = Type_gradient->currentIndex();
    double value = base;
    double max_value = base, min_value = base;
    double single_step = set_SingleTemp->value();
    double dval_v = single_step/(row-1);
    double dval_h = single_step/(col-1);

    double device_buf[3];
    double tube_buf[col];
    vector<double> buffer;
    vector<double>::iterator it = buffer.begin();


    set_SingleTemp->blockSignals(true);
    if(type == 0) set_SingleTemp->setValue(0.);
    set_SingleTemp->blockSignals(false);

    //... digit ...
    Digit.clear();
    Color.clear();


    switch(type)
    {
    default:    break;
    case 1:
    case 2:
    case 3:
                GetDevVal(type-1, base, base + set_SingleTemp->value(), device_buf);
                GetTubeVal(type-1, device_buf[0], device_buf[1], tube_buf);
                buffer.assign(tube_buf, tube_buf+col);
                break;

    case 4:     buffer = BLOCKs.toStdVector();
                //qDebug() << "BLOCKs: " << BLOCKs;
                break;
    }


    switch(type)
    {
    default:    break;
    case 1:
                max_value = *std::max_element(buffer.begin(), buffer.begin()+8);
                min_value = *std::min_element(buffer.begin(), buffer.begin()+8);
                break;
    case 2:
    case 4:
                max_value = *std::max_element(buffer.begin(), buffer.end());
                min_value = *std::min_element(buffer.begin(), buffer.end());
                break;

    case 3:
                max_value = *std::max_element(buffer.begin()+2, buffer.begin()+10);
                min_value = *std::min_element(buffer.begin()+2, buffer.begin()+10);
                break;
    }

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            switch(type)
            {
            default:    break;

            case 1:                             // Vertical
                        value = tube_buf[i];
                        break;

            case 2:                             // Horizontal
            case 3:                             // Horizontal(8)
                        value = tube_buf[j];
                        break;

            case 4:
                        // only for DT-96
                        id = div(j,4).quot + div(i,4).quot*3;
                        value = BLOCKs.at(id);
                        break;
            }
            Digit.append(value);

            color = RGB_Value(max_value, min_value, value);
            if(type == 3)
            {
                if(j < 2 || j > 9) color = 0xcccccc;
            }
            Color.append(color);
        }
    }

    data_Gradient(Digit.data(), Color.data(), 96);
}
//-----------------------------------------------------------------------------
//--- RGB_Value(double, double, double)
//-----------------------------------------------------------------------------
int Gradient::RGB_Value(double max, double min, double value)
{
    int i,j;
    int delta = (int)(fabs(max - min)/5.*1000);
    BYTE red, green, blue;
    double val;
    int color;

    if(delta == 0)
    {
        color = qRgb(0xff,0x00,0x00) & 0xffffff;
        return(color);
    }

    int num = div((int)(fabs(value - min)*1000),delta).quot;
    int rem = div((int)(fabs(value - min)*1000),delta).rem;

    if(num == 5) {num--; rem = delta;}

    switch(num)
    {
                case 0:		// B(0) G(0-255) R(255)
                         red = 0xff;
                         blue = 0x00;
                         val = (double)rem/delta * 255;
                         green = (BYTE)((int)val);
                         break;
                case 1:		// B(0) G(255) R(255-0)
                         blue = 0x00;
                         green = 0xff;
                         val = (1. - (double)rem/delta) * 255;
                         red = (BYTE)((int)val);
                         break;
                case 2:		// B(0-255) G(255) R(0)
                         green = 0xff;
                         red = 0x00;
                         val = (double)rem/delta * 255;
                         blue = (BYTE)((int)val);
                         break;
                case 3:		// B(255) G(255-0) R(0)
                         red = 0x00;
                         blue = 0xff;
                         val = (1. - (double)rem/delta) * 255;
                         green = (BYTE)((int)val);
                         break;
                case 4:		// B(255) G(0) R(0-255)
                         green = 0x00;
                         blue = 0xff;
                         val = (double)rem/delta * 255;
                         red = (BYTE)((int)val);
                         break;
                //////////////////////////////////////////////////////
                case 5:		// B(127) G(127-255) R(255)
                         red = 0xff;
                         blue = 0x80;
                         val = (double)rem/delta * (255 - 127) + 128;
                         green = (BYTE)((int)val);
                         break;
                case 6:		// B(0) G(128) R(128-0)
                         blue = 0x80;
                         green = 0xff;
                         val = (1. - (double)rem/delta) * (255 - 127) + 128;
                         red = (BYTE)((int)val);
                         break;
                case 7:		// B(0-128) G(128) R(0)
                         green = 0xff;
                         red = 0x80;
                         val = (double)rem/delta * (255 - 127) + 128;
                         blue = (BYTE)((int)val);
                         break;
                case 8:		// B(128) G(128-0) R(0)
                         red = 0x80;
                         blue = 0xff;
                         val = (1. - (double)rem/delta) * (255 - 127) + 128;
                         green = (BYTE)((int)val);
                         break;
                case 9:		// B(128) G(0) R(0-128)
                         green = 0x80;
                         blue = 0xff;
                         val = (double)rem/delta * (255 - 127) + 128;
                         red = (BYTE)((int)val);
                         break;

    }
    color = qRgb(red,green,blue) & 0xffffff;

    return(color);
}

//-----------------------------------------------------------------------------
//--- Load_lib(HINSTANCE)
//-----------------------------------------------------------------------------
void Gradient::Load_lib(HINSTANCE handle)
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
        setMinMax = reinterpret_cast<leftaxis>(::GetProcAddress(dll_handle,"left_axis_MinMax"));
        data_Gradient = reinterpret_cast<data_Grad>(::GetProcAddress(dll_handle,"data_Gradient3D"));

        if(create_Bars)
        {
            handle_3D = create_Bars(0);
            plate_Bars(96);
            channel_Bars(-1);
            //setMinMax(0,110);
        }
    }
}
//-------------------------------------------------------------------------------
//--- Set_DiffTemperature(QString)
//-------------------------------------------------------------------------------
void Gradient::Set_DiffTemperature(QString text)
{
    QVector<double> Value;
    double value;
    int id = 0;
    int row,col;
    QWidget *item ;
    QStringList list = text.split(QRegExp("\\s+"));

    foreach(QString str, list)
    {
        if(str.trimmed().isEmpty()) continue;
        value = str.toDouble();
        value = roundTo(value/10., 1);
        Value.append(value);
    }
    if(Value.size() != 6) return;

    foreach(value, Value)
    {
        row = div(id,3).quot;
        col = div(id,3).rem;
        item = Diff_table->cellWidget(row,col);
        Block_Temperature *block = qobject_cast <Block_Temperature*> (item->layout()->itemAt(0)->widget());
        block->blockSignals(true);
        block->setValue(value);
        block->blockSignals(false);
        BLOCKs.replace(id, base + value);
        id++;
    }
}

//-------------------------------------------------------------------------------
//--- showEvent
//-------------------------------------------------------------------------------
void Gradient::showEvent(QShowEvent *e)
{
    int type = gradient_Info->type;


    if(dll_handle)
    {
        if(handle_3D)
        {
            // type, gradient
            if(type <= 0) type = 1;
            Type_gradient->blockSignals(true);
            Type_gradient->setCurrentIndex(type);
            Set_TabGradient(type);
            Type_gradient->blockSignals(false);

            if((base - 10.) < 0.) set_SingleTemp->setMinimum(-base);
            if((base + 10.) > 100.) set_SingleTemp->setMaximum(100 - base);
            setMinMax(base - 12, base + 12);

            ::SetParent(handle_3D,(HWND)Image_Box->winId());
            ::SetWindowPos(handle_3D,HWND_TOP,0,0,Image_Box->width()-0,Image_Box->height()-0,SWP_SHOWWINDOW);
            show_Bars();

            switch(type)
            {
            default:
                        set_SingleTemp->blockSignals(true);
                        set_SingleTemp->setValue(gradient_Info->gradient/10.);
                        set_SingleTemp->blockSignals(false);
                        Set_DiffTemperature("0 0 0 0 0 0");
                        break;
            case 4:
                        Set_DiffTemperature(gradient_Info->gradient_block);
                        set_SingleTemp->setValue(0.0);
                        break;
            }

            Draw_Gradient();
        }
    }
}
//-----------------------------------------------------------------------------
//--- closeEvent
//-----------------------------------------------------------------------------
void Gradient::closeEvent(QCloseEvent *event)
{
    if(dll_handle)
    {
        if(handle_3D)
        {
            //ShowWindow(handle_3D, SW_HIDE);
            destroy_Bars(handle_3D);
            handle_3D = NULL;
        }
    }
    event->accept();
}
//-------------------------------------------------------------------------------
//--- resizeEvent
//-------------------------------------------------------------------------------
void Gradient::resizeEvent(QResizeEvent *e)
{
    if(handle_3D)
    {
        ::SetWindowPos(handle_3D,HWND_TOP,0,0,Image_Box->width()-0,Image_Box->height()-0,SWP_SHOWWINDOW);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Block_Temperature::Block_Temperature(int id, QWidget *parent): QDoubleSpinBox(parent)
{
    Index = id;
    Parent = parent;

    setSuffix(tr("  °C"));
    setRange(-8.,8.);
    setDecimals(1);
    setSingleStep(0.1);
    setValue(0.);
    setFont(QFont("Times New Roman", 10, QFont::Bold));
    setFixedSize(70,30);

    connect(this, SIGNAL(valueChanged(double)), this, SLOT(Change_Temperature()));

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Block_Temperature::~Block_Temperature()
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Block_Temperature::Change_Temperature()
{
    emit sChange_TemperatureBlock(this);
}
