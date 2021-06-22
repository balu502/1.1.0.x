#include "techreport_calibration.h"

extern "C" TECHREPORT_CALIBRATIONSHARED_EXPORT Report_Interface* __stdcall createReport_plugin()
{
    return(new TechReport_Calibration());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
TechReport_Calibration::TechReport_Calibration()
{
    readCommonSettings();

    report = NULL;
    type_test = 0x20;                       // calibration type
    test_description = tr("Calibration");   // description
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* TechReport_Calibration::Create_Report(rt_Protocol *P)
{
    QString logotype;
    QString text;
    QStringList list_header;
    QString date = QDate::currentDate().toString("dd.MM.yyyy");
    QString Result_Table;
    QStringList list, list_temp;
    bool att = false;
    int status;
    rt_Preference *property;
    QStringList list_result;
    list_result << tr("Функциональные nпоказатели\tХарактеристика и норма\tРезультат\tРезультат");

    prot = P;

    //qDebug() << "reserve:" << *prot->Map_Reserve.value("Diagnostic_CARD");

    QString sn_LOT = "";
    foreach(property, prot->preference_Pro)
    {
        if(property->name == "DTcheck_LOT")
        {
            sn_LOT = QString::fromStdString(property->value);
            break;
        }
    }


    report = new NCReport(this);

    // 0. LogoType
    if(lang == "ru") logotype = ":/images/dna_logo_rus_3.png";
    else logotype = ":/images/dna_logo_eng_3.png";
    report->addParameter("Logotype", logotype);

    // 1. Titul
    report->addParameter("Titul_Report",tr("Протокол приемо-сдаточных испытаний №: "));
    report->addParameter("Titul_ID", QString::fromStdString(prot->regNumber));
    report->addParameter("Titul_date", tr("дата:"));
    report->addParameter("Titul_date_1", QString::fromStdString(prot->time_created));

    report->addParameter("Titul_next",tr("набора реагентов для верификации детектирующих амплификаторов серии ДТ"));

    text = QString("%1-%2").arg(tr("(ДТчек) в комплектации ДТчек")).arg(prot->count_Tubes);
    report->addParameter("Titul_next_1", text);
    report->addParameter("Titul_next_2", tr("Серия"));

    text = "*******-**";
    if(!sn_LOT.isEmpty()) text = sn_LOT;
    report->addParameter("Titul_next_3", text);

    report->addParameter("Titul_next_4", tr("Проверка проводилась по регламенту № 134-01-ОТК-2021"));

    // 2. Header
    report->addParameter("Caption_Header", tr("Проверка состава набора и внешнего вида компонентов"));

    list_header << tr("Компонент") + "\t" + tr("Количество") + "\t" + tr("Характеристика инорма") + "\t" + tr("Результат");
    list_header << tr("Смесь с флуоресцентными температурно-зависимыми зондами") + "\t" +
                   tr("1 микропланшет") + "\t" +
                   tr("микропланшет 96 лунок, запаянный прозрачной пленкой") + "\t" +
                   tr("Соответствует");

    report->addStringList(list_header, "Header_Source");

    // 3. Table TechReport

    report->addParameter("Caption_TechTable", tr("Проверка функциональных показателей"));

    foreach(text, *prot->Map_Reserve.value("TechReportCalibration"))
    {
        //qDebug() << "text: " << text;
        if(text.contains(tr("Не соответствует"))) att = true;

        list_result.append(text);
    }
    report->addStringList(list_result, "Total_Result");


    // 4. Conclusion
    report->addParameter("Caption_Conclusion", tr("Заключение:"));
    report->addParameter("Conclusion_data_0", tr("Набор реагентов для верификации детектирующих амплификаторов серии ДТ"));
    report->addParameter("Conclusion_data_1", tr("(ДТчек)  Серия"));

    text = "*******-**";
    if(!sn_LOT.isEmpty()) text = sn_LOT;

    report->addParameter("Conclusion_data_2", text);

    text = tr("соответствует регламенту №134-01-ОТК-2021.");
    if(att) text = tr("не ") + text;
    report->addParameter("Conclusion_data_3", text);
    report->addParameter("Conclusion_data_4", tr("Проверку провел"));



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
void TechReport_Calibration::Destroy_Report()
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
void TechReport_Calibration::Type_Report(int *type, QString *description)
{
    *type = type_test;
    *description = test_description;

    return;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TechReport_Calibration::Set_SamplesEnable(QVector<short> *vec)
{

}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void TechReport_Calibration::readCommonSettings()
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
