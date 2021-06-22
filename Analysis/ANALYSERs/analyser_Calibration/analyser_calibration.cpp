#include "analyser_calibration.h"

extern "C" ANALYSER_CALIBRATIONSHARED_EXPORT Analysis_Interface* __stdcall createAnalyser_plugin()
{
    return(new Analyser_Calibration());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Analyser_Calibration::Analyser_Calibration()
{
    MainTab = NULL;
    readCommonSettings();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Analyser_Calibration::Create_Win(void *pobj, void *main)
{
    int i;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QFont f = qApp->font();

    QWidget *parent = (QWidget*)pobj;
    main_widget = (QWidget*)main;

    MainTab = new QTabWidget(parent);
    MainTab->setFont(f);

    Total = new Total_Results(MainTab);
    mask_Optical = new Mask(MainTab);
    melt_curve = new Melt_Curve(MainTab);
    melt_curve->Lang = &Lang;

    Additional_Doc = new QGroupBox(MainTab);
    Additional_Doc->setObjectName("Transparent");
    QVBoxLayout *layout_add = new QVBoxLayout;
    Additional_Doc->setLayout(layout_add);
    Tech_Table = new QTableWidget(0,3, MainTab);
    Tech_Table->setSelectionMode(QAbstractItemView::NoSelection);
    Tech_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Tech_Table->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    Tech_Table->setFont(f);
    delegate_col1 = new Tech_Delegate_1;
    Tech_Table->setItemDelegateForColumn(1, delegate_col1);
    delegate_col2 = new Tech_Delegate_2;
    Tech_Table->setItemDelegateForColumn(2, delegate_col2);
    delegate_col2->TechReport_Item = &TechReport_Item;
    Tech_Report = new QPushButton(QIcon(":/images/note_32.png"), tr("Technical Report"), MainTab);
    Tech_Report->setIconSize(QSize(32,32));
    connect(Tech_Report, SIGNAL(clicked(bool)), this, SLOT(OpenTechReport()));
    layout_add->addWidget(Tech_Table, 1);
    layout_add->addWidget(Tech_Report, 0, Qt::AlignRight);
    //QWidget *UnEqual = new QWidget();

    MainTab->addTab(Total, tr("Total"));
    MainTab->addTab(mask_Optical, QIcon(":/images/mask_w.png"), tr("Optical mask"));
    MainTab->addTab(melt_curve, QIcon(":/images/temperature_spectrum.png"), tr("Temperature/Amplitude calibration"));    
    //MainTab->addTab(UnEqual, QIcon(":/images/Unequal_w.png"), tr("Spectrum crosstalk"));
    MainTab->addTab(Additional_Doc, QIcon(":/images/note_tech_32.png"), tr("Technical document"));


    //message.setWindowIcon(QIcon(":/images/DTm.ico"));
    message.setWindowIcon(QIcon(":/images/RT.ico"));
    MainTab->setIconSize(QSize(32,16));

    return(MainTab);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Calibration::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Calibration::Destroy_Win()
{

    if(MainTab != NULL)
    {
        delete Total;
        delete mask_Optical;
        delete melt_curve;

        delete Tech_Table;
        delete Tech_Report;
        delete Additional_Doc;

        delete MainTab;
        MainTab = NULL;
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Calibration::Show()
{
    MainTab->show();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Calibration::GetInformation(QVector<QString> *info)
{
    info->append("0x0020");
    info->append(tr("Calibration"));
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Analyser_Calibration::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/analyser_calibration_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        Lang = text;

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();

    // Control Department
    CommonSettings->beginGroup("Control_Department");

    text = CommonSettings->value("control","").toString();
    if(text.trimmed() == "yes") flag_ControlDepartment = true;
    else flag_ControlDepartment = false;

    CommonSettings->endGroup();


    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//--- readCalibrationSettings(QString dev_Name)
//-----------------------------------------------------------------------------
void Analyser_Calibration::readCalibrationSettings()
{
    int i,j;
    QString text;
    QStringList list;
    double value;    
    bool ok;
    rt_Preference   *property;

    foreach(property, Prot->preference_Pro)
    //for(i=0; i<Prot->preference_Pro.size(); i++)
    {
        //property = Prot->preference_Pro.at(i);

        if(property->name == "TPeaks")
        {
            text = QString::fromStdString(property->value);
            list = text.trimmed().split(QRegExp("\\s+"));
            foreach(text, list)
            {
                value = text.toDouble(&ok);
                if(!ok) value = 100.;
                melt_curve->TPeaks.append(value);
            }
            continue;
        }

        if(property->name == "Amplitude")
        {
            text = QString::fromStdString(property->value);
            list = text.trimmed().split(QRegExp("\\s+"));
            foreach(text, list)
            {
                value = text.toDouble(&ok);
                if(!ok) value = 1000.;
                melt_curve->Amplitude.append(value);
            }
            continue;
        }

        if(property->name == "TCorrection")
        {
            text = QString::fromStdString(property->value);
            list = text.trimmed().split(QRegExp("\\s+"));
            foreach(text, list)
            {
                value = text.toDouble(&ok);
                if(!ok) value = 1.;
                //qDebug() << "correct: " << value;
                melt_curve->FluorCorrection.append(value);
            }
            continue;
        }

        if(property->name == "Borders_mask")
        {
            text = QString::fromStdString(property->value);
            list = text.trimmed().split(QRegExp("\\s+"));
            for(i=0; i<list.size(); i++)
            {
                text = list.at(i);
                value = text.toDouble(&ok);
                if(!ok) value = 1.;
                switch(i)
                {
                case 0: mask_Optical->border_MaskQuality = value;       break;
                case 1: mask_Optical->border_DistanceChannels = value;  break;
                case 2: mask_Optical->border_DifferenceDevice = value;  break;
                default:    break;
                }
            }
            continue;
        }

        if(property->name == "Borders_temperature")
        {
            text = QString::fromStdString(property->value);
            list = text.trimmed().split(QRegExp("\\s+"));
            for(i=0; i<list.size(); i++)
            {
                text = list.at(i);
                value = text.toDouble(&ok);
                if(!ok) value = 1.;
                switch(i)
                {
                case 0: melt_curve->border_AbsDeviation = value;        break;
                case 1: melt_curve->border_IrregularityPlate = value;   break;
                case 2: melt_curve->border_PlateDeviation = value;      break;
                default:    break;
                }
            }
            continue;
        }

        if(property->name == "Borders_Optic")
        {
            text = QString::fromStdString(property->value);
            list = text.trimmed().split(QRegExp("\\s+"));
            for(i=0; i<list.size(); i++)
            {
                text = list.at(i);
                value = text.toDouble(&ok);
                if(!ok) value = 1.;
                switch(i)
                {
                case 0: melt_curve->border_AbsDeviationOptic = value;        break;
                case 1: melt_curve->border_IrregularityPlateOptic = value;   break;
                case 2: melt_curve->border_PlateDeviationOptic = value;      break;
                default:    break;
                }
            }
            continue;
        }
    }

    //... Coefficients for OTK
    QString dir_path = qApp->applicationDirPath();
    QString fn = dir_path + "/calibration/Calibration.ini";
    TechReport_Borders.fill(1.,7);
    double coeff;
    QString key;
    int num = 0;

    QSettings *CalibrSettings = new QSettings(fn, QSettings::IniFormat);
    qDebug() << "Calibration: " << fn;

    CalibrSettings->beginGroup("Coefficients_OTK");

    for(i=0; i<TechReport_Borders.size(); i++)
    {
        key = QString("coeff_%1").arg(num);
        coeff = CalibrSettings->value(key,"1").toDouble(&ok);
        TechReport_Borders.replace(num,coeff);
        num++;
    }

    CalibrSettings->endGroup();

    delete CalibrSettings;

    qDebug() << "Coefficients_OTK: " << TechReport_Borders;
}
//-----------------------------------------------------------------------------
//--- readCalibrationSettings(QString dev_Name)
//-----------------------------------------------------------------------------
void Analyser_Calibration::readCalibrationSettings(QString dev_Name)
{
    QString text;
    QStringList list;
    double value;
    bool ok;
    QString dir_path = qApp->applicationDirPath();
    QString fn = dir_path + "/device/" + dev_Name + "/Calibration.ini";

    QSettings *CalibrSettings = new QSettings(fn, QSettings::IniFormat);

    qDebug() << "Calibration: " << fn;

    // Coefficients
    CalibrSettings->beginGroup("Coefficients");

    text = CalibrSettings->value("TPeaks", "100 100 100 100 100").toString();

    //qDebug() << "TPeaks: " << text;

    list = text.trimmed().split(QRegExp("\\s+"));
    foreach(text, list)
    {
        value = text.toDouble(&ok);
        if(!ok) value = 100.;
        melt_curve->TPeaks.append(value);
    }

    text = CalibrSettings->value("Amplitude", "1000 1000 1000 1000 1000").toString();
    list = text.trimmed().split(QRegExp("\\s+"));
    foreach(text, list)
    {
        value = text.toDouble(&ok);
        if(!ok) value = 1000.;
        melt_curve->Amplitude.append(value);
    }

    text = CalibrSettings->value("TCorrection", "1 1 1 1 1").toString();
    list = text.trimmed().split(QRegExp("\\s+"));
    foreach(text, list)
    {
        value = text.toDouble(&ok);
        if(!ok) value = 1.;
        //qDebug() << "correct: " << value;
        melt_curve->FluorCorrection.append(value);
    }

    CalibrSettings->endGroup();

    // Borders_mask
    CalibrSettings->beginGroup("Borders_mask");
    mask_Optical->border_MaskQuality = CalibrSettings->value("mask_quality","1").toDouble();
    mask_Optical->border_DistanceChannels = CalibrSettings->value("distance_channels","1").toDouble();
    mask_Optical->border_DifferenceDevice = CalibrSettings->value("difference_device","1").toDouble();
    CalibrSettings->endGroup();

    qDebug() << "Borders_mask: " << mask_Optical->border_MaskQuality <<
                                    mask_Optical->border_DistanceChannels <<
                                    mask_Optical->border_DifferenceDevice;

    // Borders_temperature
    CalibrSettings->beginGroup("Borders_temperature");
    melt_curve->border_AbsDeviation = CalibrSettings->value("abs_deviation","1").toDouble();
    melt_curve->border_IrregularityPlate = CalibrSettings->value("irregularity_plate","1").toDouble();
    melt_curve->border_PlateDeviation = CalibrSettings->value("plate_deviation","1").toDouble();
    CalibrSettings->endGroup();

    // Borders_Optic
    CalibrSettings->beginGroup("Borders_Optic");
    melt_curve->border_AbsDeviationOptic = CalibrSettings->value("abs_deviation","100").toDouble();
    melt_curve->border_IrregularityPlateOptic = CalibrSettings->value("irregularity_plate","100").toDouble();
    melt_curve->border_PlateDeviationOptic = CalibrSettings->value("plate_deviation","100").toDouble();
    CalibrSettings->endGroup();

    delete CalibrSettings;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Calibration::Analyser(rt_Protocol *prot)
{
    int i;
    int res;
    QString text;
    bool find = false;
    QVector<int> error_Buf;

    rt_Preference   *property;

    Prot = prot;

    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        property = prot->preference_Pro.at(i);
        //qDebug() << QString::fromStdString(property->name) << QString::fromStdString(property->value);
    }

    // Calibration Settings
    //readCalibrationSettings(QString::fromStdString(prot->SerialName));
    readCalibrationSettings();    

    for(i=1; i<MainTab->count(); i++) MainTab->setTabEnabled(i,true);
    if(!flag_ControlDepartment) MainTab->setTabVisible(3, false);


    // 1. Optic mask        
    VideoData.setPath("");
    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        property = prot->preference_Pro.at(i);
        //qDebug() << QString::fromStdString(property->name) << QString::fromStdString(property->value);
        if(QString::fromStdString(property->name) == "VideoData_Catalogue")
        {
            VideoData.setPath(QString::fromStdString(property->value));
            find = true;
            continue;
        }        
        if(QString::fromStdString(property->name).startsWith("ImageData_"))
        {
            mask_Optical->Exist_Mask = true;
            continue;
        }
    }

    //qDebug() << "mask exists: " << mask_Optical->Exist_Mask;

    if((find && VideoData.exists()) || mask_Optical->Exist_Mask)
    {
        MainTab->setCurrentWidget(mask_Optical);
        mask_Optical->Catalogue = VideoData.absolutePath();
        res = mask_Optical->Analyser(prot);                                 // res = -2
    }
    else
    {
        for(i=1; i<MainTab->count(); i++) MainTab->setTabEnabled(i,false);

        message.setStandardButtons(QMessageBox::Ok);
        message.button(QMessageBox::Ok)->animateClick(10000);
        text = "Attention! Video catalogue don't found! (or empty)";
        message.setIcon(QMessageBox::Warning);
        message.setText(text);
        message.exec();
        res = -1;                                                           // res = -1
    }

    // 1.1 Filter on (Min,Max)
    /*if(res == 0)
    {
        res = Check_MinMaxAmplitude();                                      // res = -3 (AFF)
    }*/

    // 2. Temperature of Plate
    if(res == 0)
    {
        MainTab->setCurrentWidget(melt_curve);
        melt_curve->map_RawData = &mask_Optical->map_VALUE;
        res = melt_curve->Analyser(prot);                                   // res = -4  or -3 (AFF)
    }

    qDebug() << "Results: " << res;


    // 5. fill Total Results
    Total->mask_ATTENTION = &mask_Optical->map_ATTENTION;
    Total->mask_status = mask_Optical->OptMask_status;

    Total->temperature_ATTENTION = &melt_curve->map_ATTENTION_Temp;
    Total->temperature_status = melt_curve->TemperatureValid_status;

    Total->optic_ATTENTION = &melt_curve->map_ATTENTION_Optic;
    Total->optic_status = melt_curve->OpticValid_status;


    error_Buf.append(res);
    error_Buf.append((int)melt_curve->Overflow_T);
    error_Buf.append((int)melt_curve->Overflow_A);

    Total->fill_TotalResults(prot, &error_Buf);

    Fill_TechDoc(); // additional documentation

    MainTab->setCurrentWidget(Total);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Calibration::Enable_Tube(QVector<short> *e)
{

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Calibration::Select_Tube(int pos)
{

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Calibration::Save_Results(char *fname)
{
    int i,j,k,m,n;
    int id_ch;
    QDomDocument    doc;
    QDomElement     root;
    QDomElement     data;
    QDomElement     item;
    QDomElement     result;    
    QDomElement     calibration_result;
    QDomElement     properties;
    QDomNode        node;
    QDomNode        child;

    bool is_Exists;
    QString text(fname);
    QString text_item, str, vec_str;
    QTableWidgetItem *item_Table;

    rt_Preference *prop;
    QString name, value;
    QStringList list;

    //qDebug() << "Save_Results: " << text;


    // 1. Read & Edit Info
    QFile file(text);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {

        if(doc.setContent(&file))
        {
            file.close();

            QVector<QString> *Results = new QVector<QString>();

            root = doc.documentElement();

            //... Property protocol ...
            if(!mask_Optical->Exist_Mask)
            {
                properties = root.firstChildElement("properties");
                if(!properties.isNull())
                {
                    for(j=0; j<properties.childNodes().size(); j++)
                    {
                        node = properties.childNodes().at(j);
                        for(k=0; k<node.childNodes().size(); k++)
                        {
                            child = node.childNodes().at(k);
                            if(child.nodeName() == "name")
                            {
                                str = child.toElement().text();
                                if(str.startsWith("ImageData_") && !list.contains(str)) list.append(str);
                                if(str.startsWith("RawData") && !list.contains(str)) list.append(str);
                            }
                        }
                    }

                    foreach(prop, Prot->preference_Pro)
                    {
                        name = QString::fromStdString(prop->name);
                        value = QString::fromStdString(prop->value);
                        if(list.contains(name)) continue;
                        if(name.startsWith("ImageData_") || name.startsWith("RawData"))
                        {
                            item = doc.createElement("item");
                            item.appendChild(MakeElement(doc,"name",name));
                            item.appendChild(MakeElement(doc,"value",value));
                            properties.appendChild(item);
                        }
                    }
                }
            }

            //... Analysis Data ...
            data = root.firstChildElement("Analysis_Data");
            if(!data.isNull())
            {
                // 1. Diagnostic_CARD

                is_Exists = false;
                for(i=0; i<data.childNodes().size(); i++)
                {
                    calibration_result = data.childNodes().at(i).toElement();
                    if(calibration_result.hasAttribute("name"))
                    {
                        if(calibration_result.attribute("name") == NAME_METHOD)
                        {
                            //qDebug() << NAME_METHOD;
                            is_Exists = true;
                            break;
                        }
                    }
                }

                item = doc.createElement("item");
                item.setAttribute("name", NAME_METHOD);

                for(i=0; i<Total->Total_Table->rowCount(); i++)
                {
                    result = doc.createElement("item");
                    vec_str = "";
                    for(j=0; j<Total->Total_Table->columnCount(); j++)
                    {
                        item_Table = Total->Total_Table->item(i,j);
                        text_item = item_Table->text();

                        if(!vec_str.isEmpty()) vec_str += "\t";
                        vec_str += text_item;

                        switch(j)
                        {
                        case 0:     str = "name";   break;
                        case 1:     str = "result"; break;
                        case 2:     str = "note";   break;
                        default:    str = "";       break;
                        }

                        result.appendChild(MakeElement(doc, str, text_item));
                    }
                    item.appendChild(result);
                    Results->append(vec_str);
                }

                // Total_Result
                result = doc.createElement("item");
                result.appendChild(MakeElement(doc, "name", "Total result"));
                result.appendChild(MakeElement(doc, "note", Total->Conclusion->toPlainText()));
                result.appendChild(MakeElement(doc, "result", QString("%1").arg(Total->total_result)));
                item.appendChild(result);

                if(is_Exists) data.replaceChild(item, calibration_result);
                else data.appendChild(item);

                QVector<QString> *Conclusion = new QVector<QString>();
                Conclusion->append(Total->Conclusion->toPlainText());
                Prot->Map_Reserve.insert("Conclusion_VERIFICATION", Conclusion);



                // 2. Optic Mask
                Create_OpticResults(doc, data);
                //qDebug() << "Optic Mask: ";

                // 3. Temperature Validation
                Create_TemperatureResults(doc, data);
                //qDebug() << "Temperature Validation: ";

                // 4. Amplitude Validation
                Create_AmplitudeResults(doc, data);
                //qDebug() << "Amplitude Validation: ";

                // 5. RowData:  Peaks and Amplitude for all channels
                Create_RawDataResults(doc, data);
                //qDebug() << "RowData: ";

                // 6.

            }

            Prot->Map_Reserve.insert(NAME_METHOD, Results);
        }
        else file.close();

        // 2. Write info
        if(file.open(QIODevice::WriteOnly))
        {
            QTextStream(&file) << doc.toString();
            file.close();
        }
    }
}
//-----------------------------------------------------------------------------
//--- Create_RawDataResults
//-----------------------------------------------------------------------------
void Analyser_Calibration::Create_RawDataResults(QDomDocument &doc, QDomElement &data)
{
    int i,j;
    QString text_item;
    QString str_peaks, str_ampl;
    QDomElement     item;
    QDomElement     result;
    QDomElement     calibration_result;
    QVector<double> *vec;
    double value;


    bool is_Exists = false;

    for(i=0; i<data.childNodes().size(); i++)
    {
        calibration_result = data.childNodes().at(i).toElement();
        if(calibration_result.hasAttribute("name"))
        {
            if(calibration_result.attribute("name") == "RawData")
            {

                is_Exists = true;
                break;
            }
        }
    }

    item = doc.createElement("item");
    item.setAttribute("name", "RawData");



    for(i=0; i<count_CH; i++)
    {
        if(!(Prot->active_Channels & (0x0f<<4*i))) continue;

        result = doc.createElement("item");
        item.appendChild(result);

        //qDebug() << "i, map_Param.size(): " << i << melt_curve->map_Param.size();

        if(melt_curve->map_Param.size() <= i) break;

        vec = melt_curve->map_Param.value(i);

        str_peaks.clear();
        str_ampl.clear();
        for(j=0; j<Prot->count_Tubes; j++)
        {
            str_ampl.append(QString("%1 ").arg(vec->at(j*COUNT_RESULT_PARAM + 0)));
            str_peaks.append(QString("%1 ").arg(vec->at(j*COUNT_RESULT_PARAM + 2)));
        }
        result.appendChild(MakeElement(doc, "Amplitude", str_ampl));
        result.appendChild(MakeElement(doc, "Peaks", str_peaks));        
    }


    if(is_Exists) data.replaceChild(item, calibration_result);
    else data.appendChild(item);

}

//-----------------------------------------------------------------------------
//--- Create_OpticResults
//-----------------------------------------------------------------------------
void Analyser_Calibration::Create_OpticResults(QDomDocument &doc, QDomElement &data)
{
    int i,j;
    QString text_item;
    QString str;
    QDomElement     item;
    QDomElement     result;
    QDomElement     calibration_result;

    int count_ch = 0;
    for(i=0; i<COUNT_CH; i++)
    {
        if(Prot->active_Channels & (0x0f<<i*4)) count_ch++;
    }

    QString fluor_name[COUNT_CH] = FLUOR_NAME;

    QTableWidgetItem *item_Table;

    bool is_Exists = false;

    for(i=0; i<data.childNodes().size(); i++)
    {
        calibration_result = data.childNodes().at(i).toElement();
        if(calibration_result.hasAttribute("name"))
        {
            if(calibration_result.attribute("name") == "Optic_MASK")
            {
                is_Exists = true;
                break;
            }
        }
    }

    item = doc.createElement("item");
    item.setAttribute("name", "Optic_MASK");

    for(i=0; i<mask_Optical->Table_Result->rowCount(); i++)
    {
        if(mask_Optical->Table_Result->item(i,0)->text().trimmed().isEmpty()) continue;
        result = doc.createElement("item");
        for(j=0; j<mask_Optical->Table_Result->columnCount(); j++)
        {
            item_Table = mask_Optical->Table_Result->item(i,j);
            text_item = item_Table->text();

            if(j == 0 && i < count_ch) text_item += " " + fluor_name[i];

            switch(j)
            {
            case 0:     str = "name";   break;
            case 2:     str = "result"; break;
            case 1:     str = "note";   break;
            default:    str = "";       break;
            }

            result.appendChild(MakeElement(doc, str, text_item));
        }
        item.appendChild(result);
    }

    if(is_Exists) data.replaceChild(item, calibration_result);
    else data.appendChild(item);
}
//-----------------------------------------------------------------------------
//--- Create_TemperatureResults
//-----------------------------------------------------------------------------
void Analyser_Calibration::Create_TemperatureResults(QDomDocument &doc, QDomElement &data)
{
    int i,j;
    int id = 0;
    QString text_item;
    QString str;
    QDomElement     item;
    QDomElement     result;
    QDomElement     calibration_result;

    QString fluor_name[COUNT_CH] = FLUOR_NAME;

    QTableWidgetItem *item_Table;

    bool is_Exists = false;

    for(i=0; i<data.childNodes().size(); i++)
    {
        calibration_result = data.childNodes().at(i).toElement();
        if(calibration_result.hasAttribute("name"))
        {
            if(calibration_result.attribute("name") == "Temperature_VALIDATION")
            {
                is_Exists = true;
                break;
            }
        }
    }

    item = doc.createElement("item");
    item.setAttribute("name", "Temperature_VALIDATION");

    for(i=0; i<melt_curve->Table_Result->rowCount(); i++)
    {
        if(melt_curve->Table_Result->item(i,0)->text().trimmed().isEmpty()) continue;
        result = doc.createElement("item");
        for(j=0; j<melt_curve->Table_Result->columnCount(); j++)
        {
            item_Table = melt_curve->Table_Result->item(i,j);
            text_item = item_Table->text();

            if(j == 0) text_item += " " + fluor_name[div(id,4).rem];

            switch(j)
            {
            case 0:     str = "name";   break;
            case 2:     str = "result"; break;
            case 1:     str = "note";   break;
            default:    str = "";       break;
            }

            result.appendChild(MakeElement(doc, str, text_item));
        }
        item.appendChild(result);
        id++;
    }

    if(is_Exists) data.replaceChild(item, calibration_result);
    else data.appendChild(item);
}
//-----------------------------------------------------------------------------
//--- Create_AmplitudeResults
//-----------------------------------------------------------------------------
void Analyser_Calibration::Create_AmplitudeResults(QDomDocument &doc, QDomElement &data)
{
    int i,j;
    int id = 0;
    QString text_item;
    QString str;
    QDomElement     item;
    QDomElement     result;
    QDomElement     calibration_result;

    QString fluor_name[COUNT_CH] = FLUOR_NAME;

    QTableWidgetItem *item_Table;

    bool is_Exists = false;

    for(i=0; i<data.childNodes().size(); i++)
    {
        calibration_result = data.childNodes().at(i).toElement();
        if(calibration_result.hasAttribute("name"))
        {
            if(calibration_result.attribute("name") == "Amplitude_VALIDATION")
            {
                is_Exists = true;
                break;
            }
        }
    }

    item = doc.createElement("item");
    item.setAttribute("name", "Amplitude_VALIDATION");

    for(i=0; i<melt_curve->Table_ResultSpectrum->rowCount(); i++)
    {
        if(melt_curve->Table_ResultSpectrum->item(i,0)->text().trimmed().isEmpty()) continue;
        result = doc.createElement("item");
        for(j=0; j<melt_curve->Table_ResultSpectrum->columnCount(); j++)
        {
            item_Table = melt_curve->Table_ResultSpectrum->item(i,j);
            text_item = item_Table->text();

            if(j == 0 && div(id,4).quot < 2) text_item += " " + fluor_name[div(id,4).rem];

            switch(j)
            {
            case 0:     str = "name";   break;
            case 2:     str = "result"; break;
            case 1:     str = "note";   break;
            default:    str = "";       break;
            }

            result.appendChild(MakeElement(doc, str, text_item));
        }
        item.appendChild(result);
        id++;
    }

    if(is_Exists) data.replaceChild(item, calibration_result);
    else data.appendChild(item);
}
//-----------------------------------------------------------------------------
//--- to_ClipBoard()
//-----------------------------------------------------------------------------
void Analyser_Calibration::to_ClipBoard()
{
    QString text = "";

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- Check_MinMaxAmplitude()
//-----------------------------------------------------------------------------
int Analyser_Calibration::Check_MinMaxAmplitude()
{
    int i,j;
    int num = 0;
    int res = 0;    
    QVector<double> y;
    double min_value, max_value, value;
    double border;
    int count = Prot->count_Tubes;
    int count_mc = Prot->count_MC;

    for(i=0; i<count_CH; i++)
    {
        if(Prot->active_Channels & (0x0f<<4*i))
        {
            border = melt_curve->Amplitude.at(num);
            for(j=0; j<count; j++)
            {
                y = QVector<double>::fromStdVector(Prot->MC_Filtered).mid(j*count_mc + num*count_mc*count, count_mc);
                min_value = *std::min_element(y.begin(), y.end());
                max_value = *std::max_element(y.begin(), y.end());
                value = fabs(max_value - min_value);
                value /= COEF_EXPO_1;

                if(value < border*0.30)         //30%
                {
                    res = -3;
                    break;
                }
            }
            num++;
        }
        if(res < 0) break;
    }

    return(res);
}
//-----------------------------------------------------------------------------
//--- OpenTechReport()
//-----------------------------------------------------------------------------
void Analyser_Calibration::OpenTechReport()
{
    event_TechReport.report_dll = qApp->applicationDirPath() + "/ControlDepartment/techreport_calibration.dll";
    QApplication::sendEvent(main_widget, &event_TechReport);
}
//-----------------------------------------------------------------------------
//--- Check_MinMaxAmplitude()
//-----------------------------------------------------------------------------
void Analyser_Calibration::Fill_TechDoc()
{
    int i,j,k;
    QString text, str;
    QTableWidgetItem *item;
    QStringList header;

    QVector<double> vec_temp;
    double dvalue;
    double Coeff;
    int value;
    bool ok;
    QString fluor_name[count_CH] = fluor_NAME;

    QVector<QString> *TechResult = new QVector<QString>();
    QString tech_str;


    // coef_0   Число дефектных лунок
    QString coef_0 = "-";
    if(melt_curve->Num_Mistakes.size())
    {
        value = *std::max_element(melt_curve->Num_Mistakes.begin(), melt_curve->Num_Mistakes.end());
        coef_0 = QString("%1").arg(value);
    }



    // coef_1   Коэффициент качества распознавания оптической маски
    QString coef_1 = "-";
    vec_temp.clear();
    for(i=0; i<COUNT_CH; i++)
    {
        text = fluor_name[i];
        text = mask_Optical->map_RESULTs.value(QString("Quality_%1").arg(text), "");
        if(!text.isEmpty())
        {
            dvalue = text.toDouble(&ok);
            if(ok) vec_temp.append(dvalue);
        }
    }
    if(vec_temp.size())
    {
        dvalue = *std::max_element(vec_temp.begin(),vec_temp.end());
        coef_1 = QString("%1").arg(dvalue,0,'f',3);
    }

    // coef_2   Стандартный разброс температур по блоку
    QString coef_2 = "-";
    QVector<double> *vec = &melt_curve->Std_temperature;
    if(vec->size())
    {
        dvalue = *std::max_element(vec->begin(), vec->end());
        coef_2 = QString("%1 °C").arg(dvalue,0,'f',3);
    }

    // coef_3   Разброс температур по ТЭМ
    QString coef_3 = "-";
    vec = &melt_curve->Pick_2_Pick_Block;
    if(vec->size())
    {
        dvalue = *std::max_element(vec->begin(), vec->end());
        coef_3 = QString("%1 °C").arg(dvalue,0,'f',3);
    }

    // coef_4   Стандартный разброс измерений флуоресценции
    QString coef_4 = "-";
    vec = &melt_curve->Std_amplitude;
    if(vec->size())
    {
        value = *std::max_element(vec->begin(), vec->end());
        coef_4 = QString("%1 %").arg(value);
    }

    // coef_5 Отклонение средней температуры плавления
    QString coef_5 = "-";
    vec = &melt_curve->Mean_temperature;
    if(vec->size())
    {
        dvalue = *std::max_element(vec->begin(), vec->end());
        coef_5 = QString("%1 °C").arg(dvalue,0,'f',3);
    }

    // coef_6 Отклонение измеренного сигнала флуоресценции
    QString coef_6 = "-";
    vec = &melt_curve->Deviation_amplitude;
    //qDebug() << "Deviation_amplitude: " << melt_curve->Deviation_amplitude;
    if(vec->size())
    {
        dvalue = *std::max_element(vec->begin(), vec->end());
        coef_6 = QString("%1 %").arg(dvalue,0,'f',0);
    }


    header << tr("Title") << tr("Limit") << tr("Value");

    Tech_Table->clear();
    Tech_Table->setWordWrap(true);
    Tech_Table->setColumnCount(3);
    Tech_Table->setHorizontalHeaderLabels(header);
    Tech_Table->horizontalHeader()->setFont(qApp->font());

    //Tech_Table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    Tech_Table->setColumnWidth(1, 100);
    Tech_Table->setColumnWidth(2, 70);
    Tech_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    Tech_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    Tech_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);

    Tech_Table->setRowCount(7);
    TechReport_Item.fill(1,Tech_Table->rowCount());     // fill all ok(1)

    for(i=0; i<Tech_Table->rowCount(); i++)
    {
        Coeff = TechReport_Borders.at(i);
        tech_str = "";
        for(j=0; j<Tech_Table->columnCount(); j++)
        {
            if(!tech_str.isEmpty()) tech_str.append("\t");

            item = new QTableWidgetItem();

            switch(j)
            {
            default:    text = "";  break;

            case 0:     // Title
                    switch(i)
                    {
                    case 0:     text = tr("Число дефектных лунок"); break;
                    case 1:     text = tr("Коэффициент качества распознавания оптической маски");   break;
                    case 2:     text = tr("Стандартный разброс температур по блоку");   break;
                    case 3:     text = tr("Разброс температур по ТЭМ");     break;
                    case 4:     text = tr("Стандартный разброс измерений флуоресценции");   break;
                    case 5:     text = tr("Отклонение средней температуры плавления");  break;
                    case 6:     text = tr("Отклонение измеренного сигнала флуоресценции");  break;
                    default:    text = "";  break;
                    }

                    //qDebug() << "col_0: " << i << text;
                    break;

            case 2:
                    switch(i)
                    {
                    case 0:     text = coef_0; break;
                    case 1:     text = coef_1;   break;
                    case 2:     text = coef_2;   break;
                    case 3:     text = coef_3;     break;
                    case 4:     text = coef_4;   break;
                    case 5:     text = coef_5;  break;
                    case 6:     text = coef_6;  break;
                    default:    text = "";  break;
                    }

                    break;

            case 1:     // Lim
                    switch(i)
                    {
                    case 0:     text = QString("<%1").arg(Coeff,0,'f',0); break; //"<3"; break;
                    case 1:     text = QString("<%1").arg(Coeff,0,'f',1); break;
                    case 2:     text = QString("<%1°C").arg(Coeff,0,'f',1); break; //"<0.1°C";   break;
                    case 3:     text = QString("<%1°C").arg(Coeff,0,'f',1); break; //"<0.2°C";     break;
                    case 4:     text = QString("<%1%").arg(Coeff,0,'f',0); break; //"<20%";   break;
                    case 5:     text = QString("<%1°C").arg(Coeff,0,'f',1); break; //"<0.2°C";  break;
                    case 6:     text = QString("<%1%").arg(Coeff,0,'f',0); break; //"<30%";  break;
                    default:    text = "";  break;
                    }


                    break;

            }

            item->setText(text);
            Tech_Table->setItem(i, j, item);

            tech_str.append(text);
        }
        //

        str = "";
        for(k=0; k<text.size(); k++)
        {
            QChar ch = text.at(k);
            if(ch.isDigit() || ch == '.') str.append(ch);
        }
        switch(i)
        {
        case 0:     value = str.toInt();       if(value > Coeff) TechReport_Item.replace(i,0);     break;
        case 1:     dvalue = str.toDouble();   if(dvalue > Coeff) TechReport_Item.replace(i,0);  break;
        case 2:     dvalue = str.toDouble();   if(dvalue > Coeff) TechReport_Item.replace(i,0);  break;
        case 3:     dvalue = str.toDouble();   if(dvalue > Coeff) TechReport_Item.replace(i,0);  break;
        case 4:     value = str.toInt();       if(value > Coeff) TechReport_Item.replace(i,0);    break;
        case 5:     dvalue = str.toDouble();   if(dvalue > Coeff) TechReport_Item.replace(i,0);  break;
        case 6:     value = str.toInt();       if(value > Coeff) TechReport_Item.replace(i,0);    break;
        default:    break;
        }

        //qDebug() << "i: " << i << str << Coeff << TechReport_Item;

        text = tr("Соответствует");
        if(TechReport_Item.at(i) == 0 || str.isEmpty()) text = tr("Не соответствует");
        tech_str.append(QString("\t%1").arg(text));

        TechResult->append(tech_str);
    }
    Prot->Map_Reserve.insert("TechReportCalibration", TechResult);

    Tech_Table->viewport()->update();
}
//-----------------------------------------------------------------------------
//--- Check_MinMaxAmplitude()
//-----------------------------------------------------------------------------
void Tech_Delegate_2::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    int     value;
    double  dvalue;
    bool    att = false;

    QRect rect = option.rect;
    QString text = index.data().toString();
    QString str_unit = "";

    QStringList list;
    if(!text.isEmpty())
    {
        list = text.split(" ");
        text = list.at(0);
    }
    if(list.size() > 1) str_unit = list.at(1);


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

    /*switch(row)
    {
    case 0:
                value = text.toInt();
                if(value > 2) att = true;
                break;

    case 1:
                dvalue = text.toDouble();
                if(dvalue > 0.2) att = true;
                break;

    case 2:
                dvalue = text.toDouble();
                if(dvalue > 0.1) att = true;
                break;

    case 3:
                dvalue = text.toDouble();
                if(dvalue > 0.2) att = true;
                break;

    case 4:
                value = text.toInt();
                if(value > 20) att = true;
                break;

    case 5:
                dvalue = text.toDouble();
                if(dvalue > 0.2) att = true;
                break;

    case 6:
                value = text.toInt();
                if(value > 30) att = true;
                break;

    default:    break;
    }*/
    if(TechReport_Item->at(row) == 0) att = true;

    //qDebug() << "row: " << row << att << value << dvalue;

    if(att) painter->setPen(QPen(Qt::red,2,Qt::SolidLine));
    else painter->setPen(QPen(Qt::black,1,Qt::SolidLine));

    text += " " + str_unit;
    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

}
//-----------------------------------------------------------------------------
//--- Check_MinMaxAmplitude()
//-----------------------------------------------------------------------------
void Tech_Delegate_1::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    int     value;
    double  dvalue;
    bool    att = false;

    QRect rect = option.rect;
    QString text = index.data().toString();
    QString str_unit = "";

    QStringList list;
    if(!text.isEmpty())
    {
        list = text.split(" ");
        text = list.at(0);
    }
    if(list.size() > 1) str_unit = list.at(1);


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



    //qDebug() << "row: " << row << att << value << dvalue;

    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

}

