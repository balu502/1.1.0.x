#include "report.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Report::Report(rt_Protocol *protocol, QString user_folder, QString report_folder, QWidget *parent): QDialog(parent)
{
    int i,j,k;
    rt_GroupSamples *group;
    rt_Sample       *sample;    
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;    
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QStringList pp;
    int type;

    prot = protocol;

    int width_screen = qApp->desktop()->width();
    int height_screen = qApp->desktop()->height();
    int width_window = width_screen * 0.7;
    int height_window = height_screen * 0.9;
    int X = (width_screen - width_window)/2;
    int Y = (height_screen - height_window)/2;

    report = NULL;
    readCommonSettings();
    ignore_slot = false;

    setFont(qApp->font());
    User_Folder = user_folder;

    pv = new ReportPreview_ALL(this);

    QAction *nc_print = pv->actionPrint();
    nc_print->setIcon(QIcon(":/images/print.png"));
    QAction *nc_to_PDF = pv->actionPdf();
    nc_to_PDF->setVisible(false);
    //nc_toPDF->setIcon(QIcon(":/images/toPDF.png"));
    nc_toPDF = new QAction(QIcon(":/images/toPDF.png"), tr("Print to PDF file"), this);
    pv->toolBarFile()->insertAction(nc_to_PDF, nc_toPDF);
    connect(nc_toPDF, SIGNAL(triggered(bool)), this, SLOT(SaveTo_PDF()));
    QAction *nc_toSVG = pv->actionSvg();
    nc_toSVG->setIcon(QIcon(":/images/toSVG.png"));
    nc_toSVG->setVisible(false);

    nc_toPNG = new QAction(QIcon(":/images/toPNG.png"), tr("Print to PNG file"), this);
    pv->toolBarFile()->insertAction(nc_toSVG, nc_toPNG);
    connect(nc_toPNG, SIGNAL(triggered(bool)), this, SLOT(SaveTo_Image()));

    box_Report = new QComboBox(this);
    QWidget *spacer_1 = new QWidget(this);
    spacer_1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    pv->toolBarNavigation()->addWidget(spacer_1);
    pv->toolBarNavigation()->addWidget(box_Report);

    QAction *nc_toXLS = pv->actionXls();
    nc_toXLS->setIcon(QIcon(":/images/toXLS.png"));
    nc_toXLS->setVisible(false);
    QAction *nc_increase = pv->actionZoomIn();
    nc_increase->setIcon(QIcon(":/images/increase.png"));
    QAction *nc_decrease = pv->actionZoomOut();
    nc_decrease->setIcon(QIcon(":/images/decrease.png"));
    QAction *nc_scale_off = pv->actionZoomOff();
    nc_scale_off->setIcon(QIcon(":/images/scale_off.png"));
    QAction *nc_nextPage = pv->actionNextPage();
    nc_nextPage->setIcon(QIcon(":/images/next_page.png"));
    QAction *nc_firstPage = pv->actionFirstPage();
    nc_firstPage->setIcon(QIcon(":/images/first_page.png"));
    QAction *nc_prevPage = pv->actionPreviousPage();
    nc_prevPage->setIcon(QIcon(":/images/prev_page.png"));
    QAction *nc_lastPage = pv->actionLastPage();
    nc_lastPage->setIcon(QIcon(":/images/last_page.png"));
    QAction *nc_onePage = pv->actionGoToPage();
    nc_onePage->setIcon(QIcon(":/images/rejime1pages.png"));
    QAction *nc_singlePage = pv->actionViewSingleMode();
    nc_singlePage->setIcon(QIcon(":/images/rejime1pages.png"));
    QAction *nc_twoPage = pv->actionViewDoubleMode();
    nc_twoPage->setIcon(QIcon(":/images/rejime2pages.png"));
    QAction *nc_continiusPage = pv->actionViewContinousMode();
    nc_continiusPage->setIcon(QIcon(":/images/rejime_without.png"));
    QAction *nc_exit = pv->actionClose();
    nc_exit->setIcon(QIcon(":/images/log_out.png"));

    QAction *nc_prevReport = pv->actionPreviousReport();
    nc_prevReport->setVisible(false);
    QAction *nc_nextReport = pv->actionNextReport();
    nc_nextReport->setVisible(false);

    client_toolbar = new QToolBar("client", this);
    client_toolbar->setObjectName("client_toolbar");
    client_box = new QGroupBox(this);

    //
    Load_Reports(report_folder);

    // samples_tree
    samples_Enable.clear();
    samples_tree = new QTreeWidget(this);
    samples_tree->setColumnCount(1);
    samples_tree->setFont(qApp->font());


    ApplyChange = new QPushButton(tr("APPLY CHANGE..."), this);    
    ApplyChange->setDisabled(true);

    QVBoxLayout *layout = new QVBoxLayout();
    client_box->setLayout(layout);
    layout->setMargin(1);
    layout->addWidget(samples_tree);

    //layout->addStretch(10);
    layout->addWidget(ApplyChange, 10, Qt::AlignBottom);

    pv->addToolBar(Qt::LeftToolBarArea, client_toolbar);
    client_toolbar->addWidget(client_box);
    client_toolbar->setMovable(false);


    pv->actionClose()->setVisible(true);
    pv->statusBar()->setVisible(false);    


    //Generate_Report();      // REPORT!!!
    //Load_Reports();
    current_ReportObj = NULL;

    if(Map_Reports.size())
    {
        change_Report(0);
        //PreView_Report(Map_Reports.values().at(0));
        //Samples_ForTest(Map_Reports.values().at(0));
    }

    connect(samples_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(edit_param(QTreeWidgetItem*,int)));
    connect(ApplyChange, SIGNAL(clicked(bool)), this, SLOT(apply_change()));
    connect(box_Report, SIGNAL(currentIndexChanged(int)), this, SLOT(change_Report(int)));

    pv->setWindowModality(Qt::ApplicationModal);
    pv->setAttribute(Qt::WA_DeleteOnClose);
    pv->setWindowTitle(tr("Report"));
    pv->setWindowIcon(QIcon(":/images/report.png"));
    pv->setWindowFlags(Qt::Dialog);
    pv->setWindowFlags(pv->windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMaximizeButtonHint);
    pv->showNormal();
    pv->setGeometry(X,Y,width_window,height_window);

    pv->exec();    

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Report::~Report()
{
    foreach(report_obj, Map_Reports)
    {
        report_obj->Destroy_Report();
    }
    Map_Reports.clear();

    foreach(report_handle, List_reportHandle)
    {
        if(report_handle) ::FreeLibrary(report_handle);
    }
    List_reportHandle.clear();


    pv->removeToolBar(client_toolbar);    
    samples_tree->clear();
    delete samples_tree;    
    delete ApplyChange;

    delete client_box;    
    delete client_toolbar;

    delete box_Report;

    //delete report;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Report::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/ncreport_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        if(translator_report.load(":/translations/report_" + text + ".qm"))
        {
            qApp->installTranslator(&translator_report);
        }

    lang = text;
    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::change_Report(int num)
{
    int i,j;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    QStringList pp;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    int type;

    samples_tree->clear();
    samples_Enable.clear();

    pp.clear();
    pp << tr("SAMPLES");
    samples_tree->setHeaderLabels(pp);
    samples_tree->header()->setFont(qApp->font());
    item = new QTreeWidgetItem(samples_tree);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    item->setCheckState(0, Qt::Checked);
    item->setText(0, tr("All samples:"));

    type = Map_Reports.keys().at(num);    

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);

            if(sample->p_Test->header.Type_analysis != type) continue;

            item_child = new QTreeWidgetItem(item);
            item_child->setFlags(item_child->flags() | Qt::ItemIsUserCheckable);
            item_child->setCheckState(0, Qt::Checked);
            item_child->setText(0, QString::fromStdString(sample->Unique_NameSample));
            samples_Enable.append(1);
        }
    }
    item->setExpanded(true);

    PreView_Report(Map_Reports.values().at(num));

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::edit_param(QTreeWidgetItem* item, int col)
{
    if(current_ReportObj)
    {
        //if(item->isDisabled() && item->checkState(0) == Qt::Checked)
        //{
            //item->setCheckState(0, Qt::Unchecked);
            //qDebug() << item->text(0);
        //}
    }

    if(ignore_slot) return;

    ignore_slot = true;

    ApplyChange->setDisabled(false);
    QFont f = ApplyChange->font();
    int font_size = f.pointSize();
    font_size += 1;
    f.setPointSize(font_size);
    f.setBold(true);
    ApplyChange->setFont(f);
    Sleep(100);
    font_size -= 1;
    f.setPointSize(font_size);
    f.setBold(false);
    ApplyChange->setFont(f);
    //apply_change();

    QTimer::singleShot(100, this, SLOT(change_ignore()));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::change_ignore()
{
    ignore_slot = false;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::apply_change()
{
    int i;
    bool check;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    int num = box_Report->currentIndex();

    ApplyChange->setDisabled(true);
    if(report)
    {
        report->reset(true);
        delete report;
    }

    samples_Enable.clear();
    item = samples_tree->topLevelItem(0);
    for(i=0; i<item->childCount(); i++)
    {
        item_child = item->child(i);
        if(item_child->checkState(0) == Qt::Checked) check = true;
        else check = false;
        samples_Enable.append(check);
    }

    if(Map_Reports.size()) PreView_Report(Map_Reports.values().at(num));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::SaveTo_PDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save to PDF File(s)"), User_Folder, tr("PDF files (*.pdf)"));
    if(fileName.isEmpty())  return;

    if(report) report->runReportToPDF(fileName);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::SaveTo_Image()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save to PNG File(s)"), User_Folder, tr("PNG files (*.png)"));
    if(fileName.isEmpty())  return;

    if(report) report->runReportToImage(fileName);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::PreView_Report(Report_Interface *r_obj)
{
    if(r_obj)
    {
        r_obj->Set_SamplesEnable(&samples_Enable);
        report = (NCReport*)(r_obj->Create_Report(prot));
        if(report)
        {

            pv->setReport(report);
            pv->setOutput((NCReportPreviewOutput*)report->output());
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::Samples_ForTest(Report_Interface *r_obj)
{
    int i,j;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    int num_sample = -1;
    bool status;

    current_ReportObj = r_obj;
    int key = Map_Reports.key(r_obj, -1);
    item = samples_tree->topLevelItem(0);
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            num_sample++;
            sample = group->samples.at(j);
            test = sample->p_Test;
            status = false;
            if(test->header.Type_analysis == key) status = true;

            if(num_sample < item->childCount())
            {
                item_child = item->child(num_sample);
                if(status)
                {
                    item_child->setCheckState(0,Qt::Checked);
                    item_child->setDisabled(false);
                }
                else
                {
                    item_child->setCheckState(0,Qt::Unchecked);
                    item_child->setDisabled(true);                    
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::Generate_Report()
{
    QString logotype;
    QString Cp_Table;
    QString Header_PCR;
    QString Chart_Info;
    QStringList list_header;
    QString date = QDate::currentDate().toString("dd.MM.yyyy");
    QTreeWidgetItem *item;
    bool sts = true;

    report = new NCReport();

    /*item = charts_tree->topLevelItem(0);
    if(item->checkState(0) == Qt::Unchecked) sts = false;
    else sts = true;
*/
    // 0. LogoType
    if(lang == "ru") logotype = ":/images/dna_logo_rus_3.png";
    else logotype = ":/images/dna_logo_eng_3.png";
    if(sts) report->addParameter("Logotype", logotype);


    // 1. Titul
    report->addParameter("Titul_Report",tr("Report on PCR results"));
    report->addParameter("CurrentDate", date);

    // 2. Header
    list_header << tr("Date:") + "\t" + QString::fromStdString(prot->time_created);
    list_header << tr("ID Protocol:") + "\t" + QString::fromStdString(prot->regNumber);
    list_header << tr("Name of Protocol:") + "\t" + QString::fromStdString(prot->name);
    list_header << tr("Operator:") + "\t" + QString::fromStdString(prot->owned_by);
    list_header << tr("File with results:") + "\t" + QString::fromStdString(prot->xml_filename);
    list_header << tr("Device:") + "\t" + QString::fromStdString(prot->SerialName);
    if(sts) report->addStringList(list_header, "Header_Source");
/*
    item = charts_tree->topLevelItem(1);
    if(item->checkState(0) == Qt::Unchecked) sts = false;
    else sts = true;
    // 3. Cp & MeltingCurve
    Header_PCR = tr("PCR Results:") + "\t" + tr("Pos") + "\t" + tr("F") + "\t" + tr("Research Name") + "\t" +
                 tr("Cp") + "\t" + tr("Ct") + "\t" + tr("S(%)") + "\t" + tr("aFF") + "\t" + tr("K+") + "\t" + tr("Temp.Peaks");
    if(sts)
    {
        Fill_PCRResults(Cp_Table);
        //if(!Cp_Table.isEmpty())
        //{
            report->addParameter("Header_PCR", Header_PCR);
            report->addParameter("p1", Cp_Table);
        //}
    }

    // 4. Charts
    Fill_ChartsInfo(Chart_Info);
    if(Chart_Info.length()) report->addParameter("Charts", Chart_Info);
*/

    report->setReportFile(":/report/report.xml");
    report->runReportToPreview();

    bool error = report->hasError();
    QString errormsg = report->lastErrorMsg();

    //qDebug() << "error: " << error << errormsg << report->reportFile();

    pv->setReport(report);
    pv->setOutput((NCReportPreviewOutput*)report->output());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report::Load_Reports(QString report_dll)
{
    int i;
    int type;
    QString description;    
    rt_Test *test;
    QFileInfo fi;
    QString dir_dll;
    QDir pluginsDir(qApp->applicationDirPath());
    pluginsDir.cd("report_plugins");

    if(!report_dll.isEmpty())
    {
        fi.setFile(report_dll);
        if(fi.exists())
        {
            dir_dll = fi.absoluteDir().absolutePath();
            pluginsDir.setPath(dir_dll);
        }
    }


    QVector<int> Type_Tests;
    for(i=0; i<prot->tests.size(); i++)
    {
        test = prot->tests.at(i);
        Type_Tests.append(test->header.Type_analysis);
    }



    foreach(QString fname, pluginsDir.entryList(QDir::Files))
    {
        //qDebug() << "fn: " << fname;

        report_handle = NULL;
        report_obj = NULL;

        //fname = "report_plugins\\" + fname;
        fname = pluginsDir.absolutePath() + "\\" + fname;
        report_handle = ::LoadLibraryW(fname.toStdWString().c_str());

        //qDebug() << "handle :" << report_handle;

        if(report_handle)
        {
            Report_factory report_func = reinterpret_cast<Report_factory>(
                                            ::GetProcAddress(report_handle,"createReport_plugin@0"));

            if(report_func)
            {
                report_obj = report_func();
                report_obj->Type_Report(&type, &description);

                if(Type_Tests.contains(type))
                {
                    Map_Reports.insert(type, report_obj);
                    List_reportHandle.append(report_handle);
                    Map_Description.insert(type,description);
                }
                else
                {
                    if(report_obj) report_obj->Destroy_Report();
                    ::FreeLibrary(report_handle);
                }
            }
            else ::FreeLibrary(report_handle);
        }
    }

    // only for Calibration
    if(Type_Tests.contains(0x20) && Map_Reports.keys().contains(0x20))
    {
        client_toolbar->setVisible(false);
    }    

    Type_Tests.clear();

    foreach(description, Map_Description.values())
    {
        box_Report->addItem(description);
    }

    //qDebug() << "map_Reports: " << Map_Reports << Map_Reports.keys() << Map_Reports.values();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ReportPreview_ALL::ReportPreview_ALL(QWidget *parent): NCReportPreviewWindow(parent)
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ReportPreview_ALL::~ReportPreview_ALL()
{

}




