#include "report_parameterization.h"



//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Report_Parameterization::Report_Parameterization(rt_Protocol *protocol, QString folder, QWidget *parent): QDialog(parent)
{
    int i,j,k;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_TubeTest     *tube;
    rt_ChannelTest  *channel;
    QString text;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItem *item_ch_ch;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QStringList pp;
    //pp << tr("Table of Results / Analysis Curves");

    prot = protocol;

    Create_MapTranslate();  // Map_TestsTranslate

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
    User_Folder = folder;

    // Charts
    box_graph = new QGroupBox(this);
    box_graph->setStyleSheet("background: #FFFFFF; border: 1px solid transparent;");
    box_graph->setFixedSize(520,200);
    QVBoxLayout *layout_graph = new QVBoxLayout();
    box_graph->setLayout(layout_graph);
    layout_graph->setMargin(0);
    graph = new QwtPlot();
    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(graph);
    layout_graph->addWidget(graph);
    Create_Curve();

    pv = new ReportPreview(this);

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

    QAction *nc_toXLS = pv->actionXls();
    nc_toXLS->setIcon(QIcon(":/images/toXLS.png"));
    nc_toXLS->setVisible(false);
    QAction *nc_increase = pv->actionZoomIn();
    nc_increase->setIcon(QIcon(":/images/increase.png"));
    nc_increase->setToolTip(tr("Increase page"));
    QAction *nc_decrease = pv->actionZoomOut();
    nc_decrease->setIcon(QIcon(":/images/decrease.png"));
    nc_decrease->setToolTip(tr("decrease page"));
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
    nc_lastPage->setToolTip(tr("Go to final page"));
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

    client_toolbar = new QToolBar(tr("client"), this);
    client_toolbar->setObjectName("client_toolbar");
    client_box = new QGroupBox(this);
    //client_box->setFixedSize(150,350);

    // charts_tree
    charts_tree = new TreeWidget_Chart(this);
    charts_tree->setFont(qApp->font());
    charts_tree->setColumnCount(1);
    pp << tr("CONTENT:");
    charts_tree->setHeaderLabels(pp);
    charts_tree->header()->setFont(qApp->font());
    item = new QTreeWidgetItem(charts_tree);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Checked);
    item->setText(0, tr("Header of Results:"));
    item->setDisabled(true);
    item = new QTreeWidgetItem(charts_tree);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Checked);
    item->setText(0, tr("Table of Results:"));
    item->setDisabled(true);
    item = new QTreeWidgetItem(charts_tree);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    item->setCheckState(0, Qt::Checked);
    item->setText(0, tr("All Graphs:"));
    for(i=0; i<COUNT_CH; i++)
    {
        if(prot->active_Channels & (0x0f<<4*i))
        {
            item_child = new QTreeWidgetItem(item);
            item_child->setFlags(item_child->flags() | Qt::ItemIsUserCheckable);
            item_child->setCheckState(0, Qt::Checked);
            item_child->setText(0, fluor_name[i]);
        }
    }
    item->setExpanded(true);

    // type_box: pcr & mc
    type_box = new QGroupBox(tr("Graph: Type of measurements: "),this);
    type_box->setStyleSheet("background: #FFFFFF;");
    QVBoxLayout *layout_type = new QVBoxLayout();
    layout_type->setSpacing(0);
    layout_type->setMargin(15);
    pcr_type = new QRadioButton(tr("PCR"),this);
    pcr_type->setChecked(true);
    mc_type = new QRadioButton(tr("MC"), this);
    type_box->setLayout(layout_type);
    layout_type->addWidget(pcr_type);
    layout_type->addWidget(mc_type);
    if(prot->count_PCR == 0 && prot->count_MC) mc_type->setChecked(true);
    if(prot->count_PCR == 0) pcr_type->setDisabled(true);
    if(prot->count_MC == 0) mc_type->setDisabled(true);

    // rejime_box: raw data & filtered & analysis
    rejime_box = new QGroupBox(tr("Graph: Rejime of measurements: "),this);
    rejime_box->setStyleSheet("background: #FFFFFF;");
    QVBoxLayout *layout_rejime = new QVBoxLayout();
    layout_rejime->setSpacing(0);
    layout_rejime->setMargin(15);
    rawdata_rejime = new QRadioButton(tr("Raw Data"),this);
    filtered_rejime = new QRadioButton(tr("Filtered data"), this);
    analysis_rejime = new QRadioButton(tr("Analysis data"), this);
    analysis_rejime->setChecked(true);
    rejime_box->setLayout(layout_rejime);
    layout_rejime->addWidget(rawdata_rejime);
    layout_rejime->addWidget(filtered_rejime);
    layout_rejime->addWidget(analysis_rejime);

    // samples_tree
    samples_tree = new QTreeWidget(this);
    samples_tree->setFont(qApp->font());
    samples_tree->setColumnCount(1);
    pp.clear();
    pp << tr("SAMPLES");
    samples_tree->setHeaderLabels(pp);
    samples_tree->header()->setFont(qApp->font());
    item = new QTreeWidgetItem(samples_tree);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
    item->setCheckState(0, Qt::Checked);
    item->setText(0, tr("All samples:"));
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            item_child = new QTreeWidgetItem(item);
            item_child->setFlags(item_child->flags() | Qt::ItemIsUserCheckable);
            item_child->setCheckState(0, Qt::Checked);
            item_child->setText(0, QString::fromStdString(sample->Unique_NameSample));
        }
    }
    item->setExpanded(true);


    ApplyChange = new QPushButton(tr("APPLY CHANGE..."), this);    
    ApplyChange->setDisabled(true);

    QVBoxLayout *layout = new QVBoxLayout();
    client_box->setLayout(layout);
    layout->setMargin(1);    
    layout->addWidget(charts_tree);
    layout->addWidget(samples_tree);
    //layout->addWidget(tests_tree);
    layout->addWidget(type_box);
    layout->addWidget(rejime_box);

    //layout->addStretch(10);
    layout->addWidget(ApplyChange, 10, Qt::AlignBottom);

    pv->addToolBar(Qt::LeftToolBarArea, client_toolbar);
    client_toolbar->addWidget(client_box);
    client_toolbar->setMovable(false);

    pv->actionClose()->setVisible(true);
    pv->statusBar()->setVisible(false);

    //connect(tests_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(edit_param()));
    connect(samples_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(edit_param()));
    connect(charts_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(edit_param_Graph()));    
    connect(pcr_type, SIGNAL(clicked(bool)), this, SLOT(edit_param()));
    connect(mc_type, SIGNAL(clicked(bool)), this, SLOT(edit_param()));
    connect(rawdata_rejime, SIGNAL(clicked(bool)), this, SLOT(edit_param()));
    connect(filtered_rejime, SIGNAL(clicked(bool)), this, SLOT(edit_param()));
    connect(analysis_rejime, SIGNAL(clicked(bool)), this, SLOT(edit_param()));
    connect(ApplyChange, SIGNAL(clicked(bool)), this, SLOT(apply_change()));


    Generate_Report();      // REPORT!!!

    //pv->setReport(report);
    //pv->setOutput((NCReportPreviewOutput*)report->output());
    pv->setWindowModality(Qt::ApplicationModal);
    pv->setAttribute(Qt::WA_DeleteOnClose);
    pv->setWindowTitle(tr("Parameterization DATA"));
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
Report_Parameterization::~Report_Parameterization()
{
    pv->removeToolBar(client_toolbar);
    charts_tree->clear();
    delete charts_tree;
    samples_tree->clear();
    delete samples_tree;
    //tests_tree->clear();
    //delete tests_tree;
    delete ApplyChange;

    delete pcr_type;
    delete mc_type;
    delete type_box;

    delete filtered_rejime;
    delete rawdata_rejime;
    delete analysis_rejime;

    delete client_box;    
    delete client_toolbar;

    qDeleteAll(list_Curve.begin(), list_Curve.end());
    list_Curve.clear();
    delete grid;
    delete graph;
    delete box_graph;

    delete report;    
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Report_Parameterization::readCommonSettings()
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
        if(translator_report.load(":/translations/report_param_" + text + ".qm"))
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
void Report_Parameterization::Create_Curve()
{
    int i;
    int count = prot->count_Tubes;

    for(i=0; i<count; i++)
    {
        //if(!prot->enable_tube.at(i)) continue;

        curve = new QwtPlotCurve();
        curve->setStyle(QwtPlotCurve::Lines);
        curve->attach(graph);
        list_Curve.append(curve);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::Clear_Graph()
{
    int i;

    for(i=0; i<list_Curve.count(); i++)
    {
        curve = list_Curve.at(i);
        curve->setData(NULL);
    }

    graph->replot();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::Fill_ChartsInfo(QString &charts_string)
{
    int i,j;
    int act_ch = prot->active_Channels;
    int ch = 0;    
    int count_meas, count_tubes;
    QTreeWidgetItem *item_sample, *item_sampleCh;
    QTreeWidgetItem *item_top;
    QTreeWidgetItem *item_child;
    QString str_b64;
    QString text;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QColor bg_color[] ={0xD8E2F1,0xD8E9D8,0xFEF0CB,0xFFE6FF,0xFFE6FF};
    QColor color;
    QStringList list_result;
    list_result << "name;chart";    
    QByteArray bytes;
    QBuffer buffer;
    double *x,*y;
    QwtText text_x, text_y;
    QwtText title;
    QFont font_title = qApp->font(); //font_plot;
    font_title.setBold(false);
    font_title.setPointSize(font_title.pointSize()+1);
    title.setFont(font_title);


    QFont font_axis = qApp->font();
    font_axis.setPointSize(font_axis.pointSize()-3);



    int current_type = 1;           // 1-pcr, 2-mc
    int current_rejime = 0x03;      // 0x01-raw data, 0x02-filtered data, 0x03-analysis data

    if(rawdata_rejime->isChecked()) current_rejime = 0x01;
    if(filtered_rejime->isChecked()) current_rejime = 0x02;
    if(analysis_rejime->isChecked()) current_rejime = 0x03;

    if(pcr_type->isChecked()) current_type = 1;
    if(mc_type->isChecked()) current_type = 2;

    count_tubes = prot->count_Tubes;    

    switch(current_type)
    {
    case 1:     count_meas = prot->count_PCR;
                if(count_meas <= 0) break;
                x = &prot->X_PCR.at(0);
                text = tr("PCR");
                text_x.setText(tr("Cycle number"));
                //text_y.setText(tr("Fluorescence"));
                text_y.setText(tr(" "));
                break;

    case 2:     count_meas = prot->count_MC;
                if(count_meas <= 0) break;
                x = &prot->X_MC.at(0);
                text = tr("Melting Curve");
                text_x.setText(tr("Temperature, C"));
                if(current_rejime == 0x03) text_y.setText("dF/dT");
                //else text_y.setText(tr("Fluorescence"));
                else text_y.setText(tr(" "));
                break;

    default:    text = "";   break;
    }    

    if(count_meas <= 0) {charts_string = ""; return;}   // COUNT_MEAS == 0!!!!!!!


    item_top = charts_tree->topLevelItem(2);            // item All_charts

    for(i=0; i<COUNT_CH; i++)
    {
        Clear_Graph();

        if(act_ch & (0x0f<<4*i))
        {
            item_child = item_top->child(ch);
            if(item_child->checkState(0) != Qt::Checked) {ch++; continue;}

            title.setText(text + "  " + fluor_name[i]);
            graph->setTitle(title);
            graph->setCanvasBackground(bg_color[i]);


            j = count_meas*count_tubes*ch;

            switch(current_type)
            {
            default:
            case 1:
                    switch(current_rejime)
                    {
                    default:    y = NULL;                       break;
                    case 0x01:  y = &prot->PCR_RawData.at(j);   break;
                    case 0x02:  y = &prot->PCR_Filtered.at(j);  break;
                    case 0x03:  y = &prot->PCR_Bace.at(j);
                                /*if(rejime_Normalization)
                                {
                                    if(rejime_Ct) y = &prot->NormCt_Value.at(j);
                                    else y = &prot->NormCp_Value.at(j);
                                }*/
                                break;
                    }
                    break;

            case 2:
                    switch(current_rejime)
                    {
                    default:    y = NULL;                      break;
                    case 0x01:  y = &prot->MC_RawData.at(j);   break;
                    case 0x02:  y = &prot->MC_Filtered.at(j);  break;
                    case 0x03:  y = &prot->MC_dF_dT.at(j);      break;
                    }
                    break;
            }

            for(j=0; j<count_tubes; j++)
            {
                curve = list_Curve.at(j);
                color = prot->color_tube.at(j);
                curve->setPen(color, 1);
                curve->setSamples(x, y+count_meas*j, count_meas);
                if(prot->enable_tube.at(j)) curve->setVisible(true);
                else curve->setVisible(false);

                if(Sample_PosEnable.at(j) == 0) curve->setVisible(false);
            }



            graph->setAxisAutoScale(QwtPlot::xBottom, false);
            graph->setAxisScale(QwtPlot::xBottom,x[0]-1,x[count_meas-1]+1);
            //graph->setAxisAutoScale(QwtPlot::xBottom, true);
            //graph->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine());
            graph->setAxisAutoScale(QwtPlot::yLeft, true);

            graph->setAxisTitle(QwtPlot::xBottom, text_x);
            graph->setAxisTitle(QwtPlot::yLeft, text_y);
            graph->setAxisFont(QwtPlot::yLeft, font_axis);
            graph->setAxisFont(QwtPlot::xBottom, font_axis);




            graph->updateAxes();
            graph->replot();


            QPixmap pixmap(box_graph->grab());
            //QPixmap pixmap(graph->grab());
            //pixmap.save(QString("%1pp_%2.png").arg(fn).arg(i), "PNG");

            buffer.setBuffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG");
            str_b64 = bytes.toBase64();
            list_result.append("fam;" + str_b64);

            buffer.close();
            bytes.clear();

            ch++;
        }        
    }


    charts_string = list_result.join("\r\n");

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::Fill_PCRResults(QString &pcr_string)
{
    int i,j,k,m,l,n;
    int row,col,pos;
    double value;
    bool ok;
    bool color_gray = false;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_Tube         *tube;
    rt_Channel      *channel;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    int num_sample = 0;
    QString text;
    QString name_test;
    QString Group, Pos, Fluor, Name, Cp, Ct, S, aFF, K_plus, MC, Color, Channel;
    int id_channel, id_tube;
    QStringList list;
    QStringList list_channel;
    QStringList list_result;
    list_result << "group;pos;fluor;name;Cp;Ct;S;aFF;K+;MC;color;channel";

    list_channel << ":/images/fam_flat.png" << ":/images/hex_flat.png" << ":/images/rox_flat.png" << ":/images/cy5_flat.png" << ":/images/cy55_flat.png";

    int active_ch = prot->active_Channels;
    prot->Plate.PlateSize(prot->count_Tubes,row,col);

    //... Sample_PosEnable ...
    Sample_PosEnable.clear();
    Sample_PosEnable.resize(prot->count_Tubes);
    Sample_PosEnable.fill(0);
    //...

    pcr_string.clear();

    item = samples_tree->topLevelItem(0);
    if(item->checkState(0) == Qt::Unchecked) return;    // ALL Samples are OFF

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            item_child = item->child(num_sample);
            sample = group->samples.at(j);
            num_sample++;

            //... Unchecked sample ...
            if(item_child->checkState(0) == Qt::Unchecked &&
               item_child->text(0) == QString::fromStdString(sample->Unique_NameSample)) continue;
            //...

            test = sample->p_Test;
            name_test = QString::fromStdString(test->header.Name_Test);
            if(Map_TestTranslate.size()) name_test = Map_TestTranslate.value(name_test, name_test);
            // ... Group ...
            Group = QString("%1  (%2)").arg(QString::fromStdString(sample->Unique_NameSample))
                                       .arg(name_test);
            color_gray = false;

            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                id_tube = tube->ID_Tube;
                pos = tube->pos;
                Sample_PosEnable.replace(pos,1);    // for charts
                // ... Pos ...
                Pos = Convert_IndexToName(pos,col);
                color_gray = !color_gray;

                for(m=0; m<tube->channels.size(); m++)
                {
                    channel = tube->channels.at(m);
                    id_channel = channel->ID_Channel;

                    // ... Fluor ...
                    Fluor = QString("%1").arg(id_channel);
                    Channel = list_channel.at(id_channel);

                    // ... Name ...
                    Name = QString::fromStdString(channel->Unique_NameChannel);
                    if(Map_TestTranslate.size()) Name = Map_TestTranslate.value(Name, Name);
                    for(n=0; n<channel->result_Channel.size(); n++)
                    {
                        text = QString::fromStdString(channel->result_Channel.at(n));
                        //qDebug() << "res: " << text;
                        list = text.split("=");
                        if(list.at(0) == "Cp")                  // ... Cp ...
                        {
                            Cp = list.at(1);
                            if(Cp.contains("0.0")) Cp = "-";
                            else
                            {
                                value = Cp.toDouble(&ok);
                                if(ok) Cp = QString::number(value,'f',1);
                            }
                            continue;
                        }
                        if(list.at(0) == "Ct")                  // ... Ct ...
                        {
                            Ct = list.at(1);
                            if(Ct.contains("0.0")) Ct = "-";
                            else
                            {
                                value = Ct.toDouble(&ok);
                                if(ok) Ct = QString::number(value,'f',1);
                            }
                            continue;
                        }
                        if(list.at(0) == "Sigmoid_validity")    // ... S ...
                        {
                            S = list.at(1);
                            continue;
                        }
                        if(list.at(0) == "aFF")                 // ... aFF ...
                        {
                            aFF = list.at(1);
                            if(aFF.contains("true", Qt::CaseInsensitive)) aFF = "+";
                            if(aFF.contains("false", Qt::CaseInsensitive)) aFF = "X";
                            if(aFF.contains("none", Qt::CaseInsensitive)) aFF = "-";
                            if(aFF == "warning") aFF = "?";
                            continue;
                        }

                        if(list.at(0) == "Relative_validity")   // ... K_plus ...
                        {
                            K_plus = list.at(1);
                            if(K_plus.contains("true", Qt::CaseInsensitive)) K_plus = "+";
                            if(K_plus.contains("false", Qt::CaseInsensitive)) K_plus = "X";
                            if(K_plus.contains("none", Qt::CaseInsensitive)) K_plus = "-";
                            if(K_plus == "warning") K_plus = "?";
                            continue;
                        }
                        if(list.at(0) == "mc_peaks")            // ... MC ...
                        {
                            MC = list.at(1);
                            if(MC.isEmpty()) MC = "-";
                            continue;
                        }
                    }
                    if(m) Pos = " ";

                    if(color_gray) Color = "#dcdcdc";
                    else Color = "#ffffff";

                    text = QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10;%11;%12").arg(Group).arg(Pos).arg(Fluor).arg(Name)
                                                           .arg(Cp).arg(Ct).arg(S).arg(aFF).arg(K_plus).arg(MC).arg(Color).arg(Channel);
                    list_result.append(text);
                    //qDebug() << "list: " << text;
                }
            }
        }
    }
    pcr_string = list_result.join("\r\n");

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::edit_param_Graph()
{
    bool sts = false;
    QTreeWidgetItem *item = charts_tree->topLevelItem(2);
    if(item->checkState(0) == Qt::Unchecked) sts = true;
    type_box->setDisabled(sts);
    rejime_box->setDisabled(sts);

    edit_param();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::edit_param()
{
    if(ignore_slot) return;

    ignore_slot = true;

    ApplyChange->setDisabled(false);
    QFont f = ApplyChange->font();
    int font_size = f.pointSize();
    font_size += 1;
    f.setPointSize(font_size);
    f.setBold(true);
    ApplyChange->setFont(f);
    Sleep(500);
    font_size -= 1;
    f.setPointSize(font_size);
    f.setBold(false);
    ApplyChange->setFont(f);
    //apply_change();

    QTimer::singleShot(300, this, SLOT(change_ignore()));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::change_ignore()
{
    ignore_slot = false;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::apply_change()
{
    ApplyChange->setDisabled(true);
    if(report)
    {
        report->reset(true);
        delete report;
    }
    Generate_Report();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::Generate_Report()
{
    QString logotype;
    QString Cp_Table;
    QString Header_PCR;
    QString Chart_Info;
    QStringList list_header;
    QString date = QDate::currentDate().toString("dd.MM.yyyy");    
    QTreeWidgetItem *item;
    bool sts;

    report = new NCReport();

    item = charts_tree->topLevelItem(0);
    if(item->checkState(0) == Qt::Unchecked) sts = false;
    else sts = true;

    // 0. LogoType
    if(lang == "ru") logotype = ":/images/dna_logo_rus_3.png";
    else logotype = ":/images/dna_logo_eng_3.png";
    if(sts) report->addParameter("Logotype", logotype);

    // 1. Titul
    report->addParameter("Titul_Report",tr("Report on PCR results"));
    report->addParameter("CurrentDate", date);

    // 2. Header
    QString fn = QString::fromStdString(prot->xml_filename);
    QFileInfo fi(fn);
    QStringList list;
    list = fn.split("/");
    QString fname = fi.fileName();
    if(list.size() > 3) fn = QString("%1/.../%2").arg(list.at(0)).arg(fname);

    list_header << tr("Date:") + "\t" + QString::fromStdString(prot->time_created);
    list_header << tr("ID Protocol:") + "\t" + QString::fromStdString(prot->regNumber);
    list_header << tr("Name of Protocol:") + "\t" + QString::fromStdString(prot->name);
    list_header << tr("Operator:") + "\t" + QString::fromStdString(prot->owned_by);
    list_header << tr("File with results:") + "\t" + fn;    //QString::fromStdString(prot->xml_filename);
    list_header << tr("Device:") + "\t" + QString::fromStdString(prot->SerialName);
    if(sts) report->addStringList(list_header, "Header_Source");

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


    report->setReportFile(":/report/report_param_new.xml");
    report->runReportToPreview();    
    pv->setReport(report);
    pv->setOutput((NCReportPreviewOutput*)report->output());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::SaveTo_Image()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save to PNG File(s)"), User_Folder, tr("PNG files (*.png)"));
    if(fileName.isEmpty())  return;

    report->runReportToImage(fileName);    
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Report_Parameterization::SaveTo_PDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save to PDF File(s)"), User_Folder, tr("PDF files (*.pdf)"));
    if(fileName.isEmpty())  return;

    report->runReportToPDF(fileName);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ReportPreview::ReportPreview(QWidget *parent): NCReportPreviewWindow(parent)
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ReportPreview::~ReportPreview()
{

}
//-----------------------------------------------------------------------------
//--- Create_MapTranslate()
//-----------------------------------------------------------------------------
void Report_Parameterization::Create_MapTranslate()
{
    QString key_Tr = "Map_TestsTranslate";
    QVector<QString> *vec_Tr;
    QString text;
    QStringList list_tr;

    Map_TestTranslate.clear();

    if(prot->Map_Reserve.keys().contains(key_Tr))
    {
        vec_Tr = prot->Map_Reserve.value(key_Tr);

        foreach(text, *vec_Tr)
        {
            list_tr = text.split("~");
            Map_TestTranslate.insert(list_tr.at(0), list_tr.at(1));
        }
    }

    qDebug() << "Map_Translate: " << Map_TestTranslate;
}



