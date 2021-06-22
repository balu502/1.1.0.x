#include "mask.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Mask::Mask(QWidget *parent): QGroupBox(parent)
{
    setObjectName("Transparent");
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(2);
    setLayout(main_layout);

    Control_Group = new QGroupBox(this);
    Control_Group->setObjectName("Transparent");
    QHBoxLayout *control_layout = new QHBoxLayout();
    control_layout->setMargin(0);
    Control_Group->setLayout(control_layout);
    Image_Group = new QGroupBox(this);
    Image_Group->setObjectName("Transparent");
    QVBoxLayout *layout_image = new QVBoxLayout;
    Image_Group->setLayout(layout_image);
    Image = new QImage(W_IMAGE_COEF, H_IMAGE, QImage::Format_RGB32);
    Image->fill(Qt::white);
    image_Widget = new Image_Widget(Image, this);
    image_Widget->setFixedSize(W_IMAGE_COEF, H_IMAGE);
    Video_BUF = new QVector<int>(W_IMAGE*H_IMAGE);
    view_mask = new QCheckBox(this);
    label_mask = new QLabel(tr("view mask"), this);
    Fluor_Box = new QComboBox(this);
    fluor_delegate = new FluorVideo_ItemDelegate;
    Fluor_Box->setItemDelegate(fluor_delegate);
    save_button = new QToolButton(this);
    save_button->setIcon(QIcon(":/images/save_flat.png"));
    save_button->setDisabled(true);

    QHBoxLayout *mask_layout = new QHBoxLayout();
    mask_layout->setMargin(0);
    mask_layout->addWidget(Fluor_Box, 0, Qt::AlignLeft);
    mask_layout->addWidget(save_button, 1, Qt::AlignLeft);
    mask_layout->addWidget(view_mask, 0, Qt::AlignRight);
    mask_layout->addWidget(label_mask, 0, Qt::AlignRight);
    mask_layout->addSpacing(10);


    //prev_image = new QPushButton(this);
    //next_image = new QPushButton(this);
    layout_image->addLayout(mask_layout);
    layout_image->addWidget(image_Widget);
    layout_image->addStretch(1);
    //layout_image->addSpacing(10);


    Result_Group = new QGroupBox(this);
    Result_Group->setObjectName("Transparent");
    QVBoxLayout *layout_result = new QVBoxLayout;
    Result_Group->setLayout(layout_result);
    Table_Result = new QTableWidget(0,0, this);
    Table_Result->setWordWrap(true);
    results_Delegate = new ResultsItemDelegate();
    Table_Result->setItemDelegate(results_Delegate);
    //Open_Dir = new QPushButton(tr("Open video Catalogue..."), this);
    layout_result->addWidget(Table_Result, 1);
    //layout_result->addWidget(Open_Dir, 0, Qt::AlignLeft);

    control_layout->addWidget(Result_Group);
    control_layout->addWidget(Image_Group);

    main_progress = new QProgressBar(this);
    main_progress->setTextVisible(false);
    main_progress->setRange(0,100);
    PrBar_status = new QLabel(this);
    PrBar_status->setAlignment(Qt::AlignCenter);
    QHBoxLayout *probar_layout = new QHBoxLayout();
    main_progress->setLayout(probar_layout);
    probar_layout->addWidget(PrBar_status);

    main_layout->addWidget(Control_Group, 1);
    main_layout->addWidget(main_progress);    


    OptMask_status = true;
    Exist_Mask = false;    
    dir_temp = NULL;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Mask::~Mask()
{
    /*int i;
    QStringList *list;
    QImage *img;
    QVector<QPoint> *vec;

    for(i=0; i<map_FN.size(); i++)
    {
        list = map_FN.values().at(i);
        list->clear();
        delete list;
    }
    map_FN.clear();    

    map_RESULTs.clear();

    for(i=0; i<map_IMAGE.size(); i++)
    {
        img = map_IMAGE.values().at(i);
        delete img;
    }
    map_IMAGE.clear();

    for(i=0; i<map_MASK.size(); i++)
    {
        vec = map_MASK.values().at(i);
        vec->clear();
        delete vec;
    }
    map_MASK.clear();
*/
    if(dir_temp) {delete dir_temp; dir_temp = NULL;}

    Clear_AllMaps();

    if(Img) delete Img;
    qDeleteAll(imgs);
    imgs.clear();

    Video_BUF->clear();
    delete Video_BUF;

    delete Image;
    delete image_Widget;
    delete view_mask;
    delete save_button;
    delete fluor_delegate;
    delete Fluor_Box;
    delete results_Delegate;
    delete Table_Result;
    //delete Open_Dir;
    delete Image_Group;
    delete Result_Group;
    delete Control_Group;
    delete PrBar_status;
    delete main_progress;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Clear_AllMaps()
{
    int i;
    QStringList *list;
    QImage *img;
    QVector<QPoint> *vec;
    QVector<int> *vec_int;


    for(i=0; i<map_FN.size(); i++)
    {
        list = map_FN.values().at(i);
        list->clear();
        delete list;
    }
    map_FN.clear();

    map_RESULTs.clear();

    for(i=0; i<map_IMAGE.size(); i++)
    {
        img = map_IMAGE.values().at(i);
        delete img;
    }
    map_IMAGE.clear();

    for(i=0; i<map_MASK.size(); i++)
    {
        vec = map_MASK.values().at(i);
        vec->clear();
        delete vec;
    }
    map_MASK.clear();

    for(i=0; i<map_VALUE.size(); i++)
    {
        vec_int = map_VALUE.values().at(i);
        vec_int->clear();
        delete vec_int;
    }
    map_VALUE.clear();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::SaveRawDataToProperty()
{
    int i,j,k;

    QVector<int> *raw_data;
    QVector<ushort> buffer;
    QByteArray data;
    QString text, name;
    int count_tubes = Prot->count_Tubes;
    int count_meas = Prot->count_MC;
    int count_ch = 0;

    for(i=0; i<count_CH; i++)
    {
        if(Prot->active_Channels & (0x0f<<4*i)) count_ch++;
    }

    int Count = count_ch*count_tubes*count_meas;

    buffer.reserve(Count);
    data.resize(Count*2);

    foreach(i, map_VALUE.keys())
    {
        raw_data = map_VALUE.value(i);
        for(j=0; j<raw_data->size(); j++)
        {
            buffer.append((ushort)raw_data->at(j));
        }
    }
    memcpy(data.data(), buffer.data(), Count*2);
    text = QString(data.toBase64());

    rt_Preference *property = new rt_Preference();
    Prot->preference_Pro.push_back(property);

    property->name = QString("RawData").toStdString();
    property->value = text.toStdString();

    //qDebug() << "raw data: " << text.size();

    // Temporary...
    //Load_map_VALUE();
    //...

    buffer.clear();
    data.clear();
    text.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Load_map_VALUE()
{
    int i,j, k;
    int count_tubes = Prot->count_Tubes;
    int count_meas = Prot->count_MC;
    QString text, value;
    QByteArray ba;
    QVector<ushort> buffer;
    QVector<int> *data;

    rt_Preference *property;

    foreach(property, Prot->preference_Pro)
    {
        if(property->name == "RawData")
        {
            ba = QByteArray::fromBase64(QString::fromStdString(property->value).toLatin1());
            buffer.resize(ba.size()/2);
            memcpy(buffer.data(), ba.data(), ba.size());
            break;
        }
    }

    int count = buffer.size();
    int count_ch = map_VALUE.keys().size();
    int num = count/count_ch;

    k = 0;
    foreach(i, map_VALUE.keys())
    {
        data = map_VALUE.value(i);
        data->clear();
        for(j=0; j<num; j++) data->append((int)buffer.at(j + k*num));
        k++;
    }

    //qDebug() << "Buffer: " << count << ba.size() << ba.mid(0,10) << buffer.mid(0,10) << map_VALUE.value(0)->mid(0,10);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::SaveImageToProperty(int ch, int first, int second)
{
    int i,j,k;
    Image_Obj *image;
    uint value;
    ushort pixel;
    QVector<ushort> buffer;
    QByteArray data;
    int count = H_IMAGE*W_IMAGE;
    QString fluor_name[count_CH] = fluor_NAME;
    QString text, name, str;
    rt_Preference *property;

    while(k<2)
    {
        switch(k)
        {
        default:
        case 0: image = imgs.at(first); break;
        case 1: image = imgs.at(second); break;
        }

        buffer.reserve(count);
        data.resize(count*2);

        for(i=0; i<H_IMAGE; i++)
        {
            for(j=0; j<W_IMAGE; j++)
            {
                value = image->Data[i][j];
                pixel = (ushort)(value & 0xffff);
                pixel = (pixel << 4) & 0xfff0;
                buffer.append(pixel);
            }
        }
        memcpy(data.data(), buffer.data(), count*2);
        text = QString(data.toBase64());

        property = new rt_Preference();
        Prot->preference_Pro.push_back(property);

        str = "max";
        if(k == 1) str = "min";
        name = QString("ImageData_%1_%2").arg(fluor_name[ch]).arg(str);
        property->name = name.toStdString();
        property->value = text.toStdString();

        data.clear();
        buffer.clear();

        k++;
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Create_FilesImage()
{
    QString text, name, value;
    QString fn;
    QStringList list;
    QByteArray ba;
    rt_Preference   *property;

    dir_temp = new QTemporaryDir();

    foreach(property, Prot->preference_Pro)
    {
        name = QString::fromStdString(property->name);
        if(name.startsWith("ImageData_"))
        {
            list = name.split("_");
            if(list.at(2) == "min") text = "1";
            else text = "2";
            name = QString("video_%1_%2_2.dat").arg(list.at(1)).arg(text);
            fn = dir_temp->path() + "/" + name;

            QFile file(fn);
            if(file.open(QIODevice::WriteOnly))
            {
                ba = QString::fromStdString(property->value).toLatin1();
                file.write(QByteArray::fromBase64(ba));
                file.close();

                //qDebug() << "file image: " << file.fileName();
            }
        }
    }


}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Mask::Analyser(rt_Protocol *prot)
{
    int i,j,k;
    int i_first = -1;
    QString fn;
    QString key, text;
    QString fluor_name[count_CH] = fluor_NAME;
    int res = 0;
    QStringList *list;
    int num = 0;
    int min,max,nmin,nmax,sat;
    double qDet;
    double mean,delta;
    double dvalue;
    Image_Obj *img;
    int count = prot->count_MC;
    int count_mc = count;    
    bool sts;
    QTableWidgetItem *item;
    QString str_0, str_1;
    QPoint p;
    QVector<double> temp_Value;
    int id_attention = 0;
    int id = 0;
    int error_Mask = 0x00;

    Prot = prot;
    type_dev = prot->count_Tubes;


    if(Exist_Mask)  // create image files in temporary dir
    {
        Create_FilesImage();
        Catalogue = dir_temp->path();
        count = 2;
        //qDebug() << "Catalogue: " << Catalogue;
    }

    int count_ActiveCh = 0;
    for(j=0; j<count_CH; j++)
    {
        if(prot->active_Channels & (0x0f<<4*j))
        {
            count_ActiveCh++;
            if(i_first < 0) i_first = j;
        }
    }
    results_Delegate->count_ActiveCh = count_ActiveCh;


    Clear_AllMaps();

    qDeleteAll(imgs);
    imgs.clear();
    imgs.reserve(count_ActiveCh*count);
    Img = new Image_Obj(type_dev);

    LoadFluor(prot);
    create_MapFN(count);    
    Fill_ResultsTable();


    main_progress->setRange(0, map_FN.value(fluor_name[i_first])->size()*count_ActiveCh);
    for(i=0; i<count_CH; i++)
    {
        if(!(prot->active_Channels & (0x0f<<4*i))) continue;

        QImage *image = new QImage(W_IMAGE, H_IMAGE, QImage::Format_RGB32);
        map_IMAGE.insert(i,image);
        QVector<QPoint> *MASK = new QVector<QPoint>();
        MASK->reserve(type_dev);
        map_MASK.insert(i,MASK);
        QVector<int> *VALUE = new QVector<int>();
        VALUE->reserve(type_dev*count_mc);
        map_VALUE.insert(i,VALUE);

        Fluor_Box->setCurrentIndex(i);

        list = map_FN.value(fluor_name[i]);
        foreach(fn, *list)
        {
            Draw_Image(fn, type_dev);

            num++;
            Display_ProgressBar(num, fn);
        }

        min = 10000;
        max = 0;
        sat = 0;
        for(j=0; j<list->size(); j++)
        {
            img = imgs.at(id*count + j);

            if(img->Contrast < min) {min = img->Contrast; nmin = j;}
            if(img->Contrast > max) {max = img->Contrast; nmax = j;}
            if(img->Sat > sat) sat = img->Sat;
        }
        chanPar[i].max = max;   // for further reference
        chanPar[i].min = min;
        chanPar[i].nmax = nmax;
        chanPar[i].nmin = nmin;
        chanPar[i].sat = sat;

        //... save to property ...
        if(!Exist_Mask) SaveImageToProperty(i, id*count + nmax, id*count + nmin);
        //...

        //qDebug() << "MaxMin: " << fluor_name[i] << nmin << nmax;

        Img->copy(imgs.at(id*count + nmax));
        Img->minus(imgs.at(id*count + nmin));
        Img->lookPrep();
        for(j=0; j<Img->getNT()+1; j++) Img->look4Blot(j);  // find spots in brigthness order

        Img->doSeq();   // order spots
        for(j=0; j<type_dev; j++) MASK->append(Img->sSpot[j]);
        Img->data2picI(image);

        // 1.
        qDet = Img->recognition_quality();                  // recognition quality        
        if(qDet < border_MaskQuality) sts = true;           // 0.75
        else sts = false;

        if(sts) sts = Validate_Mask(i, Img);                //... Validate Mask ...

        if(OptMask_status) OptMask_status = sts;
        str_0 = QString("%1").arg(qDet,0,'f',2);
        str_1 = QString("%1").arg(sts);
        map_RESULTs.insert(QString("Quality_%1").arg(fluor_name[i]), str_0);
        map_RESULTs.insert(QString("QualityStatus_%1").arg(fluor_name[i]), str_1);
        Table_Result->item(i,1)->setText(str_0);
        Table_Result->item(i,2)->setText(str_1);
        if(!sts)
        {
            key = QString(QChar(0x41 + id_attention)) + ".";
            text = tr("Bad quality in channel: ") + fluor_name[i];
            map_ATTENTION.insert(key, text);
            id_attention++;
            error_Mask |= 0x01;
        }
        //

        // Scaled image: ...
        *Image = image->scaled(W_IMAGE_COEF, H_IMAGE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        view_mask->setChecked(true);
        image_Widget->update();
        Sleep(1000);
        view_mask->setChecked(false);

        id++;

        //qDebug() << "mask_count_CH: " << i;
    }



    // Distance between channel
    dvalue = Calculate_DeltaMask();
    if(dvalue < border_DistanceChannels) sts = true;                // 1.5
    else sts = false;
    if(OptMask_status) OptMask_status = sts;
    str_0 = QString("%1").arg(dvalue,0,'g',3);
    str_1 = QString("%1").arg(sts);
    map_RESULTs.insert("Distance", str_0);
    map_RESULTs.insert("DistanceStatus", str_1);
    Table_Result->item(count_ActiveCh + 1,1)->setText(str_0);
    Table_Result->item(count_ActiveCh + 1,2)->setText(str_1);
    if(!sts)
    {
        key = QString(QChar(0x41 + id_attention)) + ".";
        text = tr("Bad Distance...");
        map_ATTENTION.insert(key, text);
        id_attention++;
        error_Mask |= 0x10;
    }
    //qDebug() << "mask_Distance between channel: ";

    // Distance between real mask from device and calculated mask
    Distance_RealCalcMask(prot, mean, delta);
    if(delta > 0 && mean > 0) sts = true;
    else sts = false;
    if(OptMask_status) OptMask_status = sts;
    str_0 = QString("%1 (%2)").arg(delta,0,'g',3).arg(mean,0,'g',3);
    str_1 = QString("%1").arg(sts);
    Table_Result->item(count_ActiveCh + 3,1)->setText(str_0);
    Table_Result->item(count_ActiveCh + 3,2)->setText(str_1);
    map_RESULTs.insert("Difference_Device", str_0);
    if(sts && OptMask_status && delta > border_DifferenceDevice)
    {
        key = QString(QChar(0x41 + id_attention)) + ".";
        text = tr("It is recommended to load the mask into the device!");
        map_ATTENTION.insert(key, text);
        id_attention++;
    }
    //qDebug() << "mask_Distance_RealCalcMask: ";

    // Value...
    temp_Value.clear();



    if(Exist_Mask)
    {
        Load_map_VALUE();   // from protocol property RawData
    }
    else
    {
        id = 0;
        for(i=0; i<count_CH; i++)
        {
            if(!(prot->active_Channels & (0x0f<<4*i))) continue;

            for(j=0; j<type_dev; j++)
            {
                p = map_MASK.value(i)->at(j);
                for(k=0; k<count_mc; k++)
                {
                    dvalue = (double)imgs.at(id*count_mc+k)->pData[p.y()][p.x()]/(double)imgs.at(id*count_mc+k)->Sqr;
                    map_VALUE.value(i)->append((int)dvalue);
                }
            }
            id++;
        }
    }    

    if(!Exist_Mask) SaveRawDataToProperty();    // copy map_VALUE (raw data) to property of protocol    

    Sleep(1000);

    Display_ProgressBar(0, "");
    main_progress->setVisible(false);

    connect(Fluor_Box, SIGNAL(currentIndexChanged(int)), this, SLOT(Change_Image(int)));
    connect(view_mask, SIGNAL(clicked(bool)), image_Widget, SLOT(update()));
    connect(save_button, SIGNAL(clicked(bool)), this, SLOT(Save_Mask()));

    save_button->setDisabled(false);

    //qDebug() << "mask_Analyser: " << res;

    //... error_Mask
    if(error_Mask) res = -2;
    //...

    return(res);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Save_Mask()
{
    QString FileName;
    QString text;
    QPoint p;
    QVector<QPoint> *map = map_MASK.value(0);
    QString dirName = qApp->applicationDirPath();

    Image_Obj *image = imgs.at(0);
    int Sx = image->Sx;
    int Sy = image->Sy;


    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    foreach(QString str, locations)
    {
        if(str.contains("ProgramData"))
        {
            dirName = str;
            break;
        }
    }

    FileName = QFileDialog::getSaveFileName(this,
                                            tr("Save as ..."),
                                            dirName,
                                            tr("Mask File (*.mrt)"));
    if(FileName.isEmpty()) return;

    QFile file(FileName);
    if(file.open(QIODevice::WriteOnly))
    {
        text = QString("radius: %1 %2\r\ncenters: ").arg(Sx).arg(Sy);
        foreach(p, *map)
        {
            text += QString(" %1 %2").arg(p.x()).arg(p.y());
        }

        QTextStream(&file) << text;


        file.close();
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Change_Image(int index)
{
    QImage *image = map_IMAGE.value(index);

    *Image = image->scaled(W_IMAGE_COEF, H_IMAGE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    image_Widget->update();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::LoadFluor(rt_Protocol *p)
{
    int i;
    QIcon icon;
    QString fluor_name[count_CH] = fluor_NAME;

    for(i=0; i<count_CH; i++)
    {
        Fluor_Box->addItem(fluor_name[i]);
        switch(i)
        {
        case 0:  icon.addFile(":/images/fam_flat.png");   break;
        case 1:  icon.addFile(":/images/hex_flat.png");   break;
        case 2:  icon.addFile(":/images/rox_flat.png");   break;
        case 3:  icon.addFile(":/images/cy5_flat.png");   break;
        case 4:  icon.addFile(":/images/cy55_flat.png");   break;
        default: icon.addFile(NULL);   break;
        }
        Fluor_Box->setItemIcon(i,icon);
        if(!(p->active_Channels & (0x0f<<i*4)))
        {
           Fluor_Box->setItemData(i,0,Qt::UserRole - 1);
        }

    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::create_MapFN(int count_MC)
{
    int i,j;
    QString fn;
    QStringList fn_list;
    QStringList *list;
    QString fluor_name[count_CH] = fluor_NAME;
    QDir Dir(Catalogue);

    //count_MC /= 2;

    for(i=0; i<map_FN.size(); i++)
    {
        list = map_FN.values().at(i);
        list->clear();
        delete list;
    }
    map_FN.clear();

    fn_list = Dir.entryList(QDir::Files);

    for(i=0; i<count_CH; i++)
    {
        if(!(Prot->active_Channels & (0x0f<<4*i))) continue;
        list = new QStringList();
        for(j=0; j<count_MC; j++)
        {
            fn = QString("video_%1_%2_2.dat").arg(fluor_name[i]).arg(j+1);
            if(fn_list.contains(fn)) list->append(fn);
        }
        map_FN.insert(fluor_name[i], list);
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Draw_Image(QString fname, int type_dev)
{
    int i,j;
    int num = 0;
    int pixel;
    QRgb pb = QColor(255,0,0).rgba();
    QByteArray data;
    QVector<ushort> buffer;
    QString text, str;
    int index;

    fname = Catalogue + "/" + fname;

    QFile fn(fname);
    QFileInfo fi(fn);
    if(!fi.exists()) return;
    QDir dir(fi.absoluteDir());
    //QString name = dir.rootPath() + "../" + fi.fileName();
    //file_open->setText(name);


    Video_BUF->fill(0);
    QImage image(W_IMAGE, H_IMAGE, QImage::Format_RGB32);
    Image_Obj *img = new Image_Obj(type_dev);
    imgs.append(img);

    if(fn.open(QFile::ReadOnly))
    {
        data = fn.readAll();

        buffer.resize(data.size()/2);
        memcpy(buffer.data(), data.data(), data.size());

        for(i=0; i<buffer.size(); i++)
        {
            Video_BUF->replace(i, (int)((buffer.at(i) >> 4) & 0xfff));
        }
        fn.close();
    }

    for(i=0; i<H_IMAGE; i++)
    {
        QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(i));
        for(j=0; j<W_IMAGE; j++)
        {
            pixel = Video_BUF->at(num);

            img->import_Data(i,j, pixel);   // Import DATA

            pixel = (pixel >> 4) & 0xff;
            pb = qRgb(pixel,pixel,pixel);
            line[j] = pb;
            num++;
        }
    }

    // IMG
    for(i=0; i<H_IMAGE; i++) for(j=0; j<W_IMAGE; j++) img->Mask[i][j] = 0;

    img->calcHist();        // pixels distribution
    img->lookPrep();        // calculate spot signal


    // Scaled image: ...
    *Image = image.scaled(W_IMAGE_COEF, H_IMAGE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    image_Widget->update();

    data.clear();
    buffer.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Draw_Mask(QPainter *p)
{
    int i,j;
    QPen pen;
    QPoint center;
    int Rx,Ry;
    int index = Fluor_Box->currentIndex();

    int Dmx = Img->Dmx;
    int Dmy = Img->Dmy;

    if(!view_mask->isChecked()) return;

    QVector<QPoint> *mask = map_MASK.value(index);

    //qDebug() << "mask: " << map_MASK.size();

    //return;


    pen.setColor(Qt::red);
    pen.setWidth(1);
    p->setPen(pen);

    Rx = Img->Sx;
    Ry = Img->Sy;

    for(i=0; i<Dmy; i++)
    {
        for(j=0; j<Dmx; j++)
        {
            center.setX(Round((double)(mask->at(i*Dmx+j).x())/COEF_IMAGE));
            center.setY(mask->at(i*Dmx+j).y());
            p->drawEllipse(center,Round((double)(Rx)/COEF_IMAGE),Ry);
        }
    }

}
//-----------------------------------------------------------------------------
//--- Round(double) -> int
//-----------------------------------------------------------------------------
int Mask::Round(double val)
{
    int res;

    res = qFloor(val+0.5);

    return(res);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Display_ProgressBar(int percent, QString text)
{
   main_progress->setValue(percent);
   PrBar_status->setText(text);
   PrBar_status->setMinimumSize(PrBar_status->sizeHint());
   qApp->processEvents();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Mask::Fill_ResultsTable()
{
    int i,j;
    QStringList header;
    int row_current = 0;
    QTableWidgetItem *item;

    header << tr("Name") << tr("Value") << tr("Result");

    Table_Result->clear();
    Table_Result->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Table_Result->setSelectionMode(QAbstractItemView::NoSelection);
    Table_Result->setColumnCount(3);
    Table_Result->setHorizontalHeaderLabels(header);

    Table_Result->verticalHeader()->setVisible(false);
    Table_Result->verticalHeader()->setMinimumSectionSize(1);

    Table_Result->setColumnWidth(0, 100);
    Table_Result->setColumnWidth(1, 110);
    Table_Result->setColumnWidth(2, 70);
    for(i=0; i<Table_Result->columnCount(); i++) Table_Result->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    Table_Result->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    int count_ch = 0;
    for(i=0; i<count_CH; i++)
    {
        if(Prot->active_Channels & (0x0f<<4*i)) count_ch++;
    }

    Table_Result->setRowCount(count_ch + 4);   // Quality(4+1) + (1) + Distance(1) + (1) + Distance(mean)Device(1)...


    // 1. Recognition Quality (4+1)
    for(i=0; i<count_ch; i++)
    {
        for(j=0; j<Table_Result->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            switch(j)
            {
            case 0:     item->setText(tr("Recognition Quality"));    break;
            default:
            case 1:
            case 2:     item->setText("");  break;
            }
            Table_Result->setItem(row_current, j, item);
        }
        Table_Result->setRowHeight(row_current, 20);
        row_current++;
    }
    Table_Result->setSpan(0,0,count_ch,1);

    // 2.0 separator
    for(j=0; j<Table_Result->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        item->setText(" ");
        Table_Result->setItem(row_current, j, item);
    }
    Table_Result->setRowHeight(row_current, 2);
    row_current++;

    // 2. Distance
    for(j=0; j<Table_Result->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        if(!j) item->setText(tr("Distance"));
        else item->setText("");
        Table_Result->setItem(row_current, j, item);
    }
    Table_Result->setRowHeight(row_current, 26);
    row_current++;

    // 3.0 separator
    for(j=0; j<Table_Result->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        item->setText("");
        Table_Result->setItem(row_current, j, item);
    }
    Table_Result->setRowHeight(row_current, 2);
    row_current++;

    // 3. Distance Device
    for(j=0; j<Table_Result->columnCount(); j++)
    {
        item = new QTableWidgetItem();
        if(!j) item->setText(tr("Difference with Device"));
        else item->setText("");
        Table_Result->setItem(row_current, j, item);
    }
    Table_Result->setRowHeight(row_current, 26);
    row_current++;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool Mask::Validate_Mask(int ch, Image_Obj *img)
{
    int i,j;
    QVector<QPoint> *mask;
    int row, col, pos;
    QPoint P, P_prev;
    bool res = true;
    int dx, dy;

    int Rx = img->Sx;
    int Ry = img->Sy;

    Prot->Plate.PlateSize(type_dev, row, col);
    mask = map_MASK.value(ch);

    //qDebug() << "Dmx,Dmy: " << Rx << Ry << row << col;

    for(i=0; i<row; i++)
    {
        if(i)
        {
            P = mask->at(i*col);
            P_prev = mask->at((i-1)*col);
            dx = fabs(P.x() - P_prev.x());
            dy = fabs(P.y() - P_prev.y());
            if(dx > Rx) {res = false; break;}
            if(dy < Ry || dy > Ry*3) {res = false; break;}
        }
        for(j=0; j<col; j++)
        {
            pos = i*col + j;
            P = mask->at(pos);
            if(P.x() <= 0 || P.y() <= 0) {res = false; break;}
            if(!j) continue;

            P_prev = mask->at(pos-1);
            dx = fabs(P.x() - P_prev.x());
            dy = fabs(P.y() - P_prev.y());

            //qDebug() << "tube: " << i << j << P << P_prev << dx << dy;

            if(dx < Rx || dx > Rx*3) {res = false; break;}
            if(dy > Ry) {res = false; break;}
        }
        if(!res) break;
    }

    //qDebug() << "Validate_Mask: " << ch << res;
    return(res);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double Mask::Calculate_DeltaMask()
{
    int i,j;
    int J;
    QVector<double> dist;
    QVector<QPoint> mask;
    QVector<double> Res;
    QPoint p,p0;
    double value,max,min;
    double res = 0.;
    int num = map_MASK.keys().size();

    for(i=0; i<type_dev; i++)
    {
        dist.clear();
        mask.clear();
        for(j=0; j<num; j++)
        {
            J = map_MASK.keys().at(j);
            p = map_MASK.value(J)->at(i);
            if(j == 0) p0 = p;
            p -= p0;
            value = qSqrt((double)(p.x()*p.x() + p.y()*p.y()));
            dist.append(value);
            mask.append(p);
        }
        max = *std::max_element(dist.constBegin(),dist.constEnd());
        min = *std::min_element(dist.constBegin(),dist.constEnd());
        res = max - min;
        Res.append(res);
        //qDebug() << "dist: " << i << res << mask;
    }
    res = *std::max_element(Res.constBegin(),Res.constEnd());

    return(res);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool Mask::Distance_RealCalcMask(rt_Protocol *prot, double &mean, double &delta)
{
    int i,j;
    QString text;
    QStringList list;
    int id, count;
    int x,y;
    bool sts = false;
    bool ok;
    QPoint p;
    QVector<double> mask_Dist;
    double value;

    mean = 0.;
    delta = 0.;

    if(!prot) return(sts);
    text = QString::fromStdString(prot->DeviceParameters);
    if(text.isEmpty()) return(sts);

    list = text.split(QRegExp("\\s+"));
    id = list.indexOf("centers:");
    if(id < 0) return(sts);

    count = prot->count_Tubes;
    mask_Dist.reserve(count);

    for(i=0; i<count; i++)
    {
        j = i*2 + id + 1;        
        if(list.size() <= j+1) break;

        x = QString(list.at(j)).toInt(&ok);
        if(!ok) break;
        y = QString(list.at(j+1)).toInt(&ok);
        if(!ok) break;

        p.setX(x);
        p.setY(y);

        p -= map_MASK.value(0)->at(i);        
        value = qSqrt((double)(p.x()*p.x() + p.y()*p.y()));
        //qDebug() << "i: " << i << "  val: " << value << p;
        mask_Dist.append(value);
    }
    if(mask_Dist.size() == count) sts = true;
    if(!sts) {mask_Dist.clear(); list.clear(); return(sts);}

    delta = *std::max_element(mask_Dist.constBegin(),mask_Dist.constEnd());
    mean = gsl_stats_mean(&mask_Dist.at(0), 1, mask_Dist.size());

    return(true);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Image_Widget::Image_Widget(QImage *picture, QWidget *parent): QWidget(parent)
{
    p_mask = (Mask*)parent;
    image = picture;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Widget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    painter.setPen(Qt::black);

    //... image ...
    painter.drawImage(0,0,*image);

    //... border ...
    painter.drawRect(0,0, width()-1, height()-1);

    //... mask ...
    p_mask->Draw_Mask(&painter);


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Image_Obj::Image_Obj(int type, QWidget *parent): QWidget(parent)
{    
    switch(type)
    {
    default:
    case 384:   Sx = 11; Sy = 6; Dmx = 24; Dmy = 16;    break;
    case 96:    Sx = 24; Sy = 12; Dmx = 12; Dmy = 8;     break;
    case 48:    Sx = 32; Sy = 16; Dmx = 8; Dmy = 6;      break;
    case 192:   Sx = 16; Sy = 8; Dmx = 16; Dmy = 12;    break;
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Image_Obj::~Image_Obj()
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Image_Obj::import_Data(int h, int w, int pixel)
{
    Data[h][w] = pixel;

    return (0);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Obj::calcHist()
{
#define DARK_CTY 30000
#define BRIGHT_CTY 20000
#define SAT_CTY 200

    int i,j,nd,dc,hAcc;
    long mAcc;
    uint ival;

    for(i=0; i<256; i++) hData[i] = 0;
    for(i=0; i<H_IMAGE; i++)
    {
        for(j=0; j<W_IMAGE; j++)
        {
            ival = (Data[i][j] >> 4) & 0xff;
            hData[ival]++;
        }
    }
    Tot = H_IMAGE*W_IMAGE;
    hAcc = 0;
    for(i=0;i<256;i++)
    {
        hAcc += hData[i];
        if(hAcc > DARK_CTY) break;
    }
    //...
    nd = i;
    mAcc = 0L;
    dc = 0;
    for(i=0; i<=nd; i++)
    {
        mAcc += (long)hData[i] * (long)i;
        dc += hData[i];
    }
    Dark = (mAcc*10) / dc;
    hAcc = 0;
    for(i=255; i>=0; i--)
    {
        hAcc += hData[i];
        if(hAcc > BRIGHT_CTY) break;
    }
    //...
    nd = i;
    mAcc = 0L;
    dc = 0;
    for(i=255; i>=nd ;i--)
    {
        mAcc += (long)hData[i] * (long)i; dc += hData[i];
    }
    Bright = (mAcc*10) / dc;
    hAcc = 0;
    for(i=255; i>=0; i--)
    {
        hAcc += hData[i];
        if(hAcc > SAT_CTY) break;
    }
    //...
    nd = i;
    mAcc = 0L;
    dc = 0;
    for(i=255; i>=nd; i--)
    {
        mAcc += (long)hData[i] * (long)i; dc += hData[i];
    }
    Sat = (mAcc*10) / dc;
    Contrast = Bright - Dark;

    //qDebug() << "Drk=" << Dark << " Brt=" << Bright << " Contrast=" << Contrast << " Sat=" << Sat;

    return;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Obj::lookPrep()
{
    uint i,j;
    for(i=0; i<H_IMAGE; i++) for(j=0; j<W_IMAGE; j++) Mask[i][j] = 0;
    for(i=Sy; i<H_IMAGE-Sy; i++)
    {
        for(j=Sx; j<W_IMAGE-Sx; j++)
        {
            pData[i][j] = calcSpot(QPoint(j,i));
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
uint Image_Obj::calcSpot(QPoint pnt)
{
    uint i,j,iRad;
    uint acc;
    uint np;
    double Fy;
    uint cy,cx,dx,dy;

    iRad = Sx*Sx;
    acc = 0;
    cx = pnt.x();
    cy = pnt.y();
    np = 0;
    for(i=1; i<=Sx; i++)
    {
        dx = i;
        Fy = sqrt((double)(iRad - i*i));
        dy = (uint)(Fy * (double)Sy / (double)Sx);
        acc += Data[cy][cx-i];
        acc += Data[cy][cx+i];
        np += 2;
        if(dy) for(j=1; j<=dy; j++)
        {
            acc += Data[cy-j][cx-i];
            acc += Data[cy+j][cx-i];
            acc += Data[cy-j][cx+i];
            acc += Data[cy+j][cx+i];
            np += 4;
        }
    }
    for(j=1; j<=Sy; j++)
    {
        acc += Data[cy-j][cx];
        acc += Data[cy+j][cx];
        np += 2;
    }
    acc += Data[cy][cx];
    np += 1;
    Sqr = np;

    return acc;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Image_Obj::copy(Image_Obj *img)
{
    int i,j;
    uint *pdata = &img->Data[0][0];

    for(i=0; i<H_IMAGE; i++)
    {
        for(j=0; j<W_IMAGE; j++)
        {
            Data[i][j] = *(pdata + i*W_IMAGE + j);
        }
    }
    for(i=0; i<H_IMAGE; i++) for(j=0;j<W_IMAGE;j++) Mask[i][j] = 0;

    return(0);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Image_Obj::minus(Image_Obj *img)
{
    int i,j;
    uint *pdata = &img->Data[0][0];

    for(i=0; i<H_IMAGE; i++)
    {
        for(j=0; j<W_IMAGE; j++)
        {
            Data[i][j] -= *(pdata + i*W_IMAGE + j);
            Data[i][j] += 50;
        }
    }
    for(i=0; i<H_IMAGE; i++) for(j=0; j<W_IMAGE; j++) Mask[i][j] = 0;
    return 0;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Image_Obj::look4Blot(int n)
{
    uint i,j;
    uint max,val;
    int lx0,lx1;
    int ly0,ly1;
    QPoint pmax,pnt;
    max = 0;

    for(i=Sy; i<H_IMAGE-Sy; i++)
    {
        for(j=Sx; j<W_IMAGE-Sx; j++)
        {
            if(Mask[i][j]) continue;
            pnt = QPoint(j,i);
            val = pData[i][j];
            if(val > max)
            {
                max = val;
                pmax = pnt;
            }
        }
    }
    ly0 = pmax.y() - 2*(int)Sy+1; if(ly0 < 0) ly0 = 0;
    ly1 = pmax.y() + 2*(int)Sy-1; if(ly1 > H_IMAGE) ly1 = H_IMAGE;
    lx0 = pmax.x() - 2*(int)Sx+1; if(lx0 < 0) ly0 = 0;
    lx1 = pmax.x() + 2*(int)Sx-1; if(lx1 > W_IMAGE) lx1 = W_IMAGE;
    for(i=(uint)ly0; i<=(uint)ly1; i++) for(j=(uint)lx0; j<=(uint)lx1; j++) Mask[i][j] = 1;
    setSpot(n,pmax);
    Blot[n] = max;

    return 0;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Image_Obj::doSeq()
{
    uint i,j;

    sSeq[0] = look4LeftTop();
    sSpot[0] = Spot[sSeq[0]];
    for(i=0; i<Dmy; i++)
    {
        for(j=1; j<Dmx; j++)
        {
            sSeq[Dmx*i+j] = look4Right(Dmx*i+j-1);
            sSpot[Dmx*i+j] = Spot[sSeq[Dmx*i+j]];
        }
        if(i < (Dmy-1))
        {
            sSeq[Dmx*(i+1)] = look4Bottom(Dmx*i);
            sSpot[Dmx*(i+1)] = Spot[sSeq[Dmx*(i+1)]];
        }
    }
    return 0;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
uint Image_Obj::look4LeftTop()
{
    uint i;
    double min,dst;
    uint targ;
    min = 1000000;

    for(i=0; i<Dmx*Dmy; i++)
    {
        dst = dist(QPoint(0,0),Spot[i]);
        if(dst < min) {min = dst; targ = i;}
    }
    return targ;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double Image_Obj::dist(QPoint p0, QPoint p1)
{
    double dx,dy;

    dx = (double)(p1.x()-p0.x());
    dy = (double)(p1.y()-p0.y());

    return sqrt(dx*dx+dy*dy);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
int Image_Obj::look4Right(uint n)
{
    uint tot;
    uint i,ref;
    int targ;
    int dx,dy;
    double min,dst;
    QPoint base,p;
    tot = Dmx*Dmy;
    ref = sSeq[n];
    base = sSpot[n];
    min = 1000000.;
    targ = -1;

    for(i=0; i<tot; i++)
    {
        if(i == ref) continue;
        p = Spot[i];
        dx = p.x() - base.x();
        dy = p.y() - base.y();
        if(dx < 0) continue;
        if(abs(dy) > dx) continue;
        dst = dist(p,base);
        if(dst < min) {targ = i; min = dst;}
    }
    return targ;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
uint Image_Obj::look4Bottom(uint n)
{
    uint tot;
    uint i,targ,ref;
    int dx,dy;
    double min,dst;
    QPoint base,p;
    tot = Dmx*Dmy;
    ref = sSeq[n];
    base = sSpot[n];
    min = 1000000.;

    for(i=0; i<tot; i++)
    {
        if(i == ref) continue;
        p = Spot[i];
        dx = p.x() - base.x();
        dy = p.y() - base.y();
        if(dy < 0) continue;
        if(abs(dx) > dy) continue;
        dst = dist(p,base);
        if(dst < min) {targ = i; min = dst;}
    }
    return targ;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Obj::data2picI(QImage *image)
{
    int i,j;
    int pixel;
    QRgb pb = QColor(255,0,0).rgba();

    for(i=0; i<H_IMAGE; i++)
    {
        QRgb *line = reinterpret_cast<QRgb*>(image->scanLine(i));
        for(j=0; j<W_IMAGE; j++)
        {
            pixel = Data[i][j];
            pixel = (pixel >> 4) & 0xff;
            pb = qRgb(pixel,pixel,pixel);
            line[j] = pb;
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double Image_Obj::recognition_quality()
{
    double v0,v1,v2, res;

    v0 = getBlot0(0);
    v1 = getBlot0(getNT()-1);
    v2 = getBlot0(getNT());

    res = v2/(v1 - (v0-v1)/getNT());

    return(res);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
double Image_Obj::getBlot0(int n)
{
    return((double)pData[Spot[n].y()][Spot[n].x()])/(double)Sqr;
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
    QFont font;
    QPixmap pixmap(":/images/fam_flat.png");
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

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    //font = GraphPlot::font_plot;
    //font = painter->font();
    //font.setBold(true);
    //painter->setFont(QFont("Comic Sans MS", 10, QFont::Normal));

    if(option.state & QStyle::State_Enabled)
    {
        switch(row)
        {
        case 0:  pixmap.load(":/images/fam_flat.png");    break;
        case 1:  pixmap.load(":/images/hex_flat.png");    break;
        case 2:  pixmap.load(":/images/rox_flat.png");    break;
        case 3:  pixmap.load(":/images/cy5_flat.png");    break;
        case 4:  pixmap.load(":/images/cy55_flat.png");   break;
        }
    }
    else
    {
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
        pixmap.load(":/images/disable_flat.png");
    }

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter->drawPixmap(0, rect.y()+2, pixmap);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ResultsItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text,str;
    int flag;
    bool ok;
    QStringList list;

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;
    text = index.data().toString();
    int row = index.row();
    int col = index.column();

    QPixmap pixmap(":/images/error_16.png");
    QPixmap pixmap_ch(":/images/fam_flat.png");
    QColor bg_color = Qt::white;

    // Background
    painter->fillRect(rect, bg_color);

    // 1. Recognition Quality Data & Standart Deviation
    if(row < count_ActiveCh)
    {
        switch(col)
        {
        case 0: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 1:
                switch(row)
                {
                default:                    
                case 0: pixmap_ch.load(":/images/fam_flat.png");    break;
                case 1: pixmap_ch.load(":/images/hex_flat.png");    break;
                case 2: pixmap_ch.load(":/images/rox_flat.png");    break;
                case 3: pixmap_ch.load(":/images/cy5_flat.png");    break;
                case 4: pixmap_ch.load(":/images/cy55_flat.png");    break;
                }
                painter->drawPixmap(rect.left(), rect.top() + (rect.height()-8)/2 - 2, pixmap_ch);
                rect.setLeft(rect.left() + 10);
                painter->drawText(rect, Qt::AlignCenter, text); break;

        case 2:
                if(text.isEmpty()) break;
                id = text.toInt(&ok);
                if(ok && id > 0) pixmap.load(":/images/check_16.png");
                else  pixmap.load(":/images/error_16.png");
                painter->drawPixmap(option.rect.x() + (option.rect.width()-16)/2,
                                    option.rect.y() + (option.rect.height()-16)/2,
                                    pixmap);
                    break;
        default:    break;
        }
    }

    // 1. Distination
    if(row == count_ActiveCh + 1)
    {
        switch(col)
        {
        case 0: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 1: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 2: if(text.isEmpty()) break;
                id = text.toInt(&ok);
                if(ok && id > 0) pixmap.load(":/images/check_16.png");
                else  pixmap.load(":/images/error_16.png");
                painter->drawPixmap(option.rect.x() + (option.rect.width()-16)/2,
                                    option.rect.y() + (option.rect.height()-16)/2,
                                    pixmap);
                    break;
        default:    break;
        }
    }

    // 1. Distination with device
    if(row == count_ActiveCh + 3)
    {
        switch(col)
        {
        case 0: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 1: painter->drawText(rect, Qt::AlignCenter, text); break;

        case 2: if(text.isEmpty()) break;
                id = text.toInt(&ok);
                if(ok && id > 0) pixmap.load(":/images/check_16.png");
                else  pixmap.load(":/images/error_16.png");
                painter->drawPixmap(option.rect.x() + (option.rect.width()-16)/2,
                                    option.rect.y() + (option.rect.height()-16)/2,
                                    pixmap);
                    break;
        default:    break;
        }
    }
}
