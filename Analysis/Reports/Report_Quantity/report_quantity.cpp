#include "report_quantity.h"

extern "C" REPORT_QUANTITYSHARED_EXPORT Report_Interface* __stdcall createReport_plugin()
{
    return(new Report_Quantity());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Report_Quantity::Report_Quantity()
{
    readCommonSettings();

    report = NULL;
    type_test = 0x01;                       // Quantity type
    test_description = tr("Quantity");      // description
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Report_Quantity::Create_Report(rt_Protocol *P)
{
    int i,j,k,m,n;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Channel      *channel;
    rt_Test         *test;
    rt_Preference   *preference;
    bool use_Ct, ok;
    int row,col,pos;
    int num_ch;
    int num_sample = -1;
    double dvalue;
    QString Pos,Name,Ch,Cp,Result_Ch,Result;

    QString logotype;
    QString text, str, str_result;
    QStringList list_header;
    QString date = QDate::currentDate().toString("dd.MM.yyyy");
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QStringList list_result;    
    list_result << tr("Pos\tIdentificator\tChannel\tCp(Ct)\tResults\tConcentration");

    prot = P;
    prot->Plate.PlateSize(prot->count_Tubes,row,col);

    //qDebug() << "reserve:" << *prot->Map_Reserve.value("Diagnostic_CARD");

    report = new NCReport(this);

    // 0. LogoType
    if(lang == "ru") logotype = ":/images/dna_logo_rus_3.png";
    else logotype = ":/images/dna_logo_eng_3.png";
    report->addParameter("Logotype", logotype);

    // 1. Titul
    report->addParameter("Titul_Report",tr("Quantity PCR Analysis"));
    report->addParameter("CurrentDate", date);

    // 2. Header
    QString fn = QString::fromStdString(prot->xml_filename);
    QFileInfo fi(fn);
    QStringList lists;
    lists = fn.split("/");
    QString fname = fi.fileName();
    if(lists.size() > 3) fn = QString("%1/.../%2").arg(lists.at(0)).arg(fname);

    list_header << tr("Date:") + "\t" + QString::fromStdString(prot->time_created);
    //list_header << tr("ID Protocol:") + "\t" + QString::fromStdString(prot->regNumber);
    //list_header << tr("Name of Protocol:") + "\t" + QString::fromStdString(prot->name);
    list_header << tr("Operator:") + "\t" + QString::fromStdString(prot->owned_by);
    list_header << tr("File with results:") + "\t" + fn;    //QString::fromStdString(prot->xml_filename);
    list_header << tr("Device:") + "\t" + QString::fromStdString(prot->SerialName) +
                                          QString(" (%1)").arg(QString::fromStdString(prot->Type_ThermoBlock));
    report->addStringList(list_header, "Header_Source");

    // 3. Results
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {            
            sample = group->samples.at(j);            
            test = sample->p_Test;
            if(test->header.Type_analysis != 0x01) continue;

            //...
            num_sample++;
            if(!sample_enable.at(num_sample)) continue;
            //...

            Name = QString::fromStdString(sample->Unique_NameSample);
            use_Ct = false;
            for(n=0; n<test->preference_Test.size(); n++)
            {
                preference = test->preference_Test.at(n);
                if(preference->name == "use_Threshold")
                {
                    text = QString::fromStdString(preference->value);
                    if(text.contains("true", Qt::CaseInsensitive)) use_Ct = true;
                    break;
                }
            }

            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);                
                Pos = Convert_IndexToName(tube->pos,col);
                Ch = "";
                Cp = "";
                Result_Ch = "";
                Result = "";
                for(m=0; m<tube->channels.size(); m++)
                {
                    channel = tube->channels.at(m);
                    if(!Ch.isEmpty()) Ch += "\r\n";
                    Ch += QString("%1").arg(fluor_name[channel->ID_Channel]);
                    str = "Cp=";
                    if(use_Ct) str = "Ct=";
                    dvalue = 0.;
                    for(n=0; n<channel->result_Channel.size(); n++)
                    {
                        str_result = QString::fromStdString(channel->result_Channel.at(n));
                        if(str_result.contains(str))
                        {
                            pos = str_result.indexOf("=");
                            str_result = str_result.mid(pos+1);
                            dvalue = str_result.toDouble(&ok);
                            if(!ok) dvalue = 0.;
                            break;
                        }
                    }
                    if(dvalue == 0.) str_result = "-";
                    else str_result = QString("%1").arg(dvalue,0,'f',1);
                    if(!Cp.isEmpty()) Cp += "\r\n";
                    Cp += str_result;
                    if(!Result_Ch.isEmpty()) Result_Ch += "\r\n";
                    if(dvalue <= 0.) Result_Ch += "-";
                    else Result_Ch += "+";

                    // Quantity results

                    str = "Calculated_Concentration";
                    for(n=0; n<channel->result_Channel.size(); n++)
                    {
                        str_result = QString::fromStdString(channel->result_Channel.at(n));
                        if(str_result.contains(str))
                        {
                            pos = str_result.indexOf("=");
                            str_result = str_result.mid(pos+1);
                            dvalue = str_result.toDouble(&ok);
                            if(!ok) dvalue = 0.;
                            break;
                        }
                    }
                    if(dvalue <= 0.) str_result = "-";
                    else
                    {
                        if(dvalue < 0.01 || dvalue > 99999) str_result = QString("%1").arg(dvalue, 0, 'e', 3);
                        else
                        {
                            if(dvalue >= 100.) str_result = QString("%1").arg(dvalue, 0, 'f', 0);
                            if(dvalue >= 10. && dvalue < 100.) str_result = QString("%1").arg(dvalue, 0, 'f', 1);
                            if(dvalue < 10.) str_result = QString("%1").arg(dvalue, 0, 'f', 2);
                        }

                        //str_result = QString("%1").arg(dvalue,0,'f',0);
                    }
                    if(!Result.isEmpty()) Result += "\r\n";
                    Result += str_result;

                }

                /*for(m=0; m<tube->result_Tube.size(); m++)
                {
                    str_result = QString::fromStdString(tube->result_Tube.at(m));
                    if(str_result.contains("Quality_Result"))
                    {
                        pos = str_result.indexOf("=");
                        if(pos > 0)
                        {
                            Result = str_result.mid(pos+1).trimmed();
                        }
                        break;
                    }
                }*/

                // additional rows:
                num_ch = div(tube->channels.size(),2).quot;
                str = "\r\n";
                text = "";
                for(m=0; m<num_ch; m++) text += str;
                Pos = text+Pos+text;
                Name = text+Name+text;
                //Result = text+Result+text;

            }

            if(Pos.isEmpty()) continue;

            Ch = QString(" \r\n%1\r\n ").arg(Ch);
            Cp = QString(" \r\n%1\r\n ").arg(Cp);
            Result_Ch = QString(" \r\n%1\r\n ").arg(Result_Ch);
            Result = QString(" \r\n%1\r\n ").arg(Result);

            str = QString("%1\t%2\t%3\t%4\t%5\t%6").arg(Pos).arg(Name).arg(Ch).arg(Cp).arg(Result_Ch).arg(Result);
            list_result.append(str);
        }        
    }

    report->addStringList(list_result, "Quantity_Result");



    // 4.
    report->setReportFile(":/report/report_quantity_new.xml");
    report->runReportToPreview();

    bool error = report->hasError();
    QString errormsg = report->lastErrorMsg();
    //qDebug() << "error: " << error << errormsg << report->reportFile();

    return(report);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Quantity::Destroy_Report()
{
    if(report)
    {
        report->reset(true);
        delete report;
    }
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Quantity::Type_Report(int *type, QString *description)
{
    *type = type_test;
    *description = test_description;

    return;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Quantity::Set_SamplesEnable(QVector<short> *vec)
{
    sample_enable.clear();
    sample_enable = *vec;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Report_Quantity::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        //if(translator.load(":/translations/ncreport_" + text + ".qm"))
        //{
        //    qApp->installTranslator(&translator);
        //}
        if(translator_report.load(":/translations/report_quantity_" + text + ".qm"))
        {
            qApp->installTranslator(&translator_report);
        }

    lang = text;
    CommonSettings->endGroup();
    delete CommonSettings;
}
