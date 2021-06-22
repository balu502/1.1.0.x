#include "report_hrm.h"

extern "C" REPORT_HRMSHARED_EXPORT Report_Interface* __stdcall createReport_plugin()
{
    return(new Report_Hrm());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Report_Hrm::Report_Hrm()
{
    readCommonSettings();

    report = NULL;
    type_test = 0x10;                   // HRM type
    test_description = tr("HRM");   // description
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Report_Hrm::Create_Report(rt_Protocol *P)
{
    int i,j,k;
    rt_GroupSamples *group;
    rt_Sample       *sample;    
    rt_Test         *test;

    int row,col;
    int num_sample = -1;    

    QString logotype;
    QString text, str_result;
    QStringList list_header;
    QString date = QDate::currentDate().toString("dd.MM.yyyy");
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QStringList list;

    QStringList list_result;
    QString Pos = tr("Pos");
    //list_result << tr("\r\nPos\r\n\t\r\nIdentificator\r\n\t\r\nTpeak,°C\r\n\t\r\nCluster\r\n\t\r\n%\r\n");
    list_result << QString("\r\n%1\r\n\t\r\n%2\r\n\t\r\n%3\r\n\t\r\n%4\r\n\t\r\n%5\r\n").
    //list_result << QString("%1\t%2\t%3\t%4\t%5").
                   arg(Pos).
                   arg(tr("Identificator")).
                   arg(tr("Tpeak,°C")).
                   arg(tr("Cluster")).
                   arg("%");

    prot = P;
    prot->Plate.PlateSize(prot->count_Tubes,row,col);

    //qDebug() << "reserve:" << *prot->Map_Reserve.value("Diagnostic_CARD");
    vector<string> vec;
    /*foreach(group, prot->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            vec = sample->result_Sample;
            if(vec.empty()) continue;
            text = QString::fromStdString(vec.at(0));
            qDebug() << "hrm: " << text;
        }
    }*/

    report = new NCReport(this);

    // 0. LogoType
    if(lang == "ru") logotype = ":/images/dna_logo_rus_3.png";
    else logotype = ":/images/dna_logo_eng_3.png";
    report->addParameter("Logotype", logotype);

    // 1. Titul
    report->addParameter("Titul_Report",tr("HRM Analysis"));
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
            if(test->header.Type_analysis != 0x10) continue;

            //...
            num_sample++;
            if(!sample_enable.at(num_sample)) continue;
            //...

            vec = sample->result_Sample;
            if(vec.empty()) continue;

            str_result = QString::fromStdString(vec.at(0));
            if(!str_result.startsWith("HRM_Results=")) continue;
            str_result = str_result.remove("HRM_Results=");
            list = str_result.split("\t");
            if(list.size() >= 5)
            {
                text = list.at(3);
                k = text.indexOf(" ");
                if(k >= 0)
                {
                    text = text.mid(0,k);
                    list.replace(3,text);
                    str_result = list.join("\t");
                }
            }
            list_result.append(str_result);            
        }        
    }

    report->addStringList(list_result, "HRM_Result");



    // 4.
    report->setReportFile(":/report/report_hrm.xml");
    report->runReportToPreview();

    bool error = report->hasError();
    QString errormsg = report->lastErrorMsg();
    qDebug() << "error: " << error << errormsg << report->reportFile();

    return(report);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Hrm::Destroy_Report()
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
void Report_Hrm::Type_Report(int *type, QString *description)
{
    *type = type_test;
    *description = test_description;

    return;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Hrm::Set_SamplesEnable(QVector<short> *vec)
{
    sample_enable.clear();
    sample_enable = *vec;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Report_Hrm::readCommonSettings()
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
        if(translator_report.load(":/translations/report_hrm_" + text + ".qm"))
        {
            qApp->installTranslator(&translator_report);
        }

    lang = text;
    CommonSettings->endGroup();
    delete CommonSettings;
}
