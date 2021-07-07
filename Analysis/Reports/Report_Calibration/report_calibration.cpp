#include "report_calibration.h"

extern "C" REPORT_CALIBRATIONSHARED_EXPORT Report_Interface* __stdcall createReport_plugin()
{
    return(new Report_Calibration());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Report_Calibration::Report_Calibration()
{
    readCommonSettings();

    report = NULL;
    type_test = 0x20;                       // calibration type
    test_description = tr("Calibration");   // description
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Report_Calibration::Create_Report(rt_Protocol *P)
{
    QString logotype, stamp, signature, certificate;
    QString text;
    QStringList list_header;
    QString date = QDate::currentDate().toString("dd.MM.yyyy");
    QString Result_Table;
    QStringList list, list_temp;
    bool sts;
    int status;
    QStringList list_result;
    list_result << tr("Name research\tStatus\t                                                 Notice");

    prot = P;

    //qDebug() << "reserve:" << *prot->Map_Reserve.value("Diagnostic_CARD");

    report = new NCReport(this);

    // 0. LogoType
    if(lang == "ru") logotype = ":/images/dna_logo_rus_3.png";
    else logotype = ":/images/dna_logo_eng_3.png";
    report->addParameter("Logotype", logotype);

    // 1. Titul
    report->addParameter("Titul_Report",tr("Diagnostic DT(%1) card").arg(QString::fromStdString(prot->SerialName)));
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

    //list_header << tr("Operator:") + "\t" + QString::fromStdString(prot->owned_by);


    list_header << tr("File with results:") + "\t" + fn;    //QString::fromStdString(prot->xml_filename);
    list_header << tr("Device:") + "\t" + QString::fromStdString(prot->SerialName) +
                                          QString(" (%1)").arg(QString::fromStdString(prot->Type_ThermoBlock));
    report->addStringList(list_header, "Header_Source");

    // 3. Table
    foreach(text, *prot->Map_Reserve.value("Diagnostic_CARD"))
    {
        //qDebug() << "text: " << text;

        list = text.split("\t");
        if(list.at(1) == "0") status = 0;
        else status = 1;
        if(list.at(1) == "...") status = 2;
        if(status == 2) {text = "\r\n..."; list.replace(1,text);}
        else  list.replace(1,"");
        text = list.at(0);        
        text = " \r\n" + text + "\r\n  ";
        list.replace(0,text);
        text = list.at(2);
        list_temp = text.split("\r\n");
        //if(!text.contains("\r\n")) text = " \r\n" + text;
        text = " \r\n" + text + "\r\n  ";
        //qDebug() << "list_temp.size(): " << list_temp.size() << list_temp;
        if(list_temp.size() > 4) text = text + " \r\n   ";

        list.replace(2,text);
        text = list.join("\t");

        switch(status)
        {
        case 0:     text += "\t:/images/error_16.png";  break;
        case 1:     text += "\t:/images/check_16.png";  break;
        case 2:
        default:    break;
        }

        //if(sts) text += "\t:/images/check_16.png";
        //else text += "\t:/images/error_16.png";
        list_result.append(text);
    }
    //qDebug() << "List result: " << list_result;

    report->addStringList(list_result, "Total_Result");

    // 4. Conclusion
    text = "";
    QVector<QString> *vec = prot->Map_Reserve.value("Conclusion_VERIFICATION");
    if(vec && vec->size())
    {
        text = vec->at(0);
    }
    //qDebug() << "text: " <<  text;

    report->addParameter("Conclusion_data", text);

    // 5. Stamp
    stamp = ":/images/stamp.png";
    signature = "                                                                            / Комаров Д.И. /";
    certificate = "This certificate is auto-generated and serves as an original. No signature is required.";
    if(lang == "ru")
    {
        report->addParameter("Stamp", stamp);
        report->addParameter("signature", signature);
    }
    else report->addParameter("certificate", certificate);


    report->setReportFile(":/report/report_calibration.xml");
    report->runReportToPreview();

    bool error = report->hasError();
    QString errormsg = report->lastErrorMsg();
    //qDebug() << "error: " << error << errormsg << report->reportFile();

    return(report);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Calibration::Destroy_Report()
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
void Report_Calibration::Type_Report(int *type, QString *description)
{
    *type = type_test;
    *description = test_description;

    return;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Calibration::Set_SamplesEnable(QVector<short> *vec)
{

}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Report_Calibration::readCommonSettings()
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
        if(translator_report.load(":/translations/report_calibration_" + text + ".qm"))
        {
            qApp->installTranslator(&translator_report);
        }

    lang = text;
    CommonSettings->endGroup();
    delete CommonSettings;
}
