#include "analysis.h"

static void* Analysis_Obj = NULL;

extern "C" ANALYSISSHARED_EXPORT Analysis* __stdcall create_page()
{
    return(new Analysis());
}

extern "C" ANALYSISSHARED_EXPORT bool __stdcall is_exit(void *p)
{
    return(((Analysis*)p)->Is_Exit());
}

extern "C" ANALYSISSHARED_EXPORT void __stdcall load_Protocol(void *p_analysis, char *p_str)
{
    QString text(p_str);
    ((Analysis*)p_analysis)->open_Protocol(text);
}
extern "C" ANALYSISSHARED_EXPORT void __stdcall load_User(void *p_analysis, QAxObject* user)
{
    ((Analysis*)p_analysis)->create_User(user);
    Analysis_Obj = p_analysis;
}
extern "C" ANALYSISSHARED_EXPORT void __stdcall load_TestVerification(void *p_analysis, QAxObject* axgp_resource)
{
    ((Analysis*)p_analysis)->create_TestVerification(axgp_resource);
}
extern "C" ANALYSISSHARED_EXPORT void __stdcall load_Menu(void *p_analysis, void *list_pinfo)
{
    ((Analysis*)p_analysis)->Create_MenuActions(list_pinfo);
}

extern "C" ANALYSISSHARED_EXPORT void __stdcall action_Menu(void *p_analysis, QString type_action)
{
    ((Analysis*)p_analysis)->execution_MenuAction(type_action);
}

extern "C" ANALYSISSHARED_EXPORT void __stdcall enable_Menu(void *p_analysis, void *enable_action)
{
    ((Analysis*)p_analysis)->Enable_MenuAction(enable_action);
}


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Analysis::Analysis(QWidget *parent): QMainWindow(parent)
{    
    readCommonSettings();

    ax_user = NULL;
    axgp = NULL;
    setFont(qApp->font());


    //setStyleSheet("QGroupBox{padding-top:15px; margin-top:-15px}");

    prot = Create_Protocol();
    action_event = NULL;
    flag_ActivePoint = false;

    MainGroupBox = new QGroupBox();
    MainGroupBox->setObjectName("Transparent");
    setCentralWidget(MainGroupBox);
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(5);
    main_layout->setSpacing(4);
    MainGroupBox->setLayout(main_layout);

    //app_font = qApp->font(); //QFont("Comic Sans MS", 10, QFont::Bold);
    //setFont(app_font);

    createActions();    
    createChartGroupBox();    
    createSelectTubes();        // create SelectTubes as DockWidget

    createStatusBar();    
    createToolBars();

    connect(main_spl, SIGNAL(splitterMoved(int,int)), this, SLOT(resize_splitter(int,int)));
    connect(GridBox_allcp, SIGNAL(Resize()), this, SLOT(resize_CpAllResults()));
    //connect(GridTab, SIGNAL(currentChanged(int)), this, SLOT(resize_extResults()));
    connect(GridBox_ext_all, SIGNAL(Resize()), this, SLOT(resize_extResults()));

    //... Apply Settings ...
    QString dir_path = qApp->applicationDirPath();
    ApplSettings = new QSettings(dir_path + "/tools/ini/preference_analysis.ini", QSettings::IniFormat);
    readSettings();

    main_layout->addWidget(main_spl);

    //qDebug() << "create_NetWork():";
    //create_NetWork();           // create NetworkAccessManger



    //... create Info ...    
    Info_Pro = new Info(this);

    //... Email ...    
    email = new Email(this);
    email->dir_temp = &dir_temp;
    email->Pro = prot;
    //...................

    alg = NULL;
    dll_handle = NULL;

    analyser = NULL;
    analyser_handle = NULL;
    Load_InfoAnalyser();

    //ext_alg = NULL;
    ext_dll_handle = NULL;
    ext_dll_handle_VALID = false;
    Load_InfoEXTAnalyser();

    initialize = NULL;
    set_font = NULL;
    set_folder = NULL;

    show_results = NULL;
    close_results = NULL;
    resize_results = NULL;
    enable_tubes = NULL;
    select_curve = NULL;
    get_color = NULL;

    //report_XMLfile = NULL;
    //show_report = NULL;
    //close_report = NULL;

    label_gif = new QLabel(GridWindow);
    label_gif->setFixedSize(48,48);
    label_gif->raise();
    obj_gif = new QMovie(":/images/wait_1.gif");
    label_gif->setMovie(obj_gif);
    label_gif->setVisible(false);

    fn_IN  = qApp->applicationDirPath() + "/data/in.rt";
    //fn_OUT = qApp->applicationDirPath() + "/data/out.rt";
    //fn_OUTTMP = qApp->applicationDirPath() + "/data/out_tmp.rt";
    fn_OUT = dir_temp.path() + "/out.rt";
    fn_OUTTMP = dir_temp.path() + "/out_tmp.rt";

    message.setWindowIcon(QIcon(":/images/DTm.ico"));
    if(qApp->applicationDisplayName().contains("DTcheck")) message.setWindowIcon(QIcon(":/images/check.ico"));
    message.setFont(qApp->font());

    connect(this, SIGNAL(sActionPoint(Action_Event*)), this, SLOT(slot_ReceivedActionPoint(Action_Event*)));

    connect(plot->Fluor_Box, SIGNAL(currentIndexChanged(int)), this, SLOT(ColorMelting_Default()));

    // RDML
    dll_rdml = NULL;
    dll_rdml = ::LoadLibraryW(L"rdml.dll");
    if(!dll_rdml)
    {
        RDML_import->setDisabled(true); // setVisible(false);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Analysis::~Analysis()
{
    writeSettings();
    Delete_Graph(plot);

    delete obj_gif;
    delete label_gif;

    delete Info_Pro;
    delete email;

    delete Attention_Browser;
    delete cb_MarkerAttention;

    if(alg)
    {
        alg->Destroy_Win();
        alg->Destroy();
        alg = NULL;
    }

    delete layout_ChartTool;

    if(dll_handle) ::FreeLibrary(dll_handle);
    if(ext_dll_handle)
    {
        if(close_results) close_results();
        if(close_report) close_report();
        ::FreeLibrary(ext_dll_handle);
        ext_dll_handle_VALID = false;
    }
    Map_EXTanalyserNAME.clear();

    //...
    Analysis_Interface *analysis;
    HINSTANCE handle;
    QWidget *tab_obj;

    foreach(analysis, Map_analyser)
    {
        analysis->Destroy_Win();
        analysis->Destroy();
    }
    Map_analyserNAME.clear();
    Map_analyser.clear();

    foreach(handle, List_analyserHandle)
    {
        if(handle) ::FreeLibrary(handle);
    }
    List_analyserHandle.clear();


    foreach(tab_obj, Map_TabAnalyser)
    {
        if(tab_obj) delete tab_obj;
    }    
    Map_TabAnalyser.clear();
    Tab_AnalyserGlobe->clear();

    if(dll_rdml) ::FreeLibrary(dll_rdml);

    //...

    //delete net_manager_GET;
    //delete net_manager_POST;
    delete ApplSettings;

}
//-----------------------------------------------------------------------------
//--- Is_Exit()
//-----------------------------------------------------------------------------
bool Analysis::Is_Exit()
{
    int res;
    bool sts = true;
    QString text;    

    //flag_SaveChangesProtocol = Color_IsChanged();
    if(!flag_SaveChangesProtocol && !Color_IsChanged() && !Comments_IsChanged()) return(sts);

    //flag_SaveChangesProtocol = false;
    message.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    text = tr("Attention! Do you want to save last changes of the current protocol in ANALYSIS");
    message.setIcon(QMessageBox::Warning);
    message.setText(text);
    res = message.exec();
    if(res == QMessageBox::Yes)
    {
        save();
        flag_SaveChangesProtocol = false;
    }
    if(res == QMessageBox::Cancel)
    {
        sts = false;
    }


    return(sts);
}
//-----------------------------------------------------------------------------
//--- create_User
//-----------------------------------------------------------------------------
bool Analysis::create_User(QAxObject *user)
{
    ax_user = user;
    user_Dir.setPath(qApp->applicationDirPath() + "/user");
    QString temp;
    QString tooltip;

    if(ax_user)
    {
        QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
        foreach(QString str, locations)
        {
            if(str.contains("ProgramData"))
            {                
                QDir dir;
                QString path = QString("%1/Users/%2").arg(str).arg(ax_user->property("userName").toString());                
                dir.setPath(path);
                if(!dir.exists()) dir.mkpath(dir.absolutePath());
                if(dir.exists()) user_Dir.setPath(dir.absolutePath());
                break;
            }
        }
    }

    // set user properties
    flag_ReOpenProtocol = false;
    flag_CommonSettings = false;
    flag_EditTest = false;
    flag_SaveLocationWin = false;
    flag_SaveChangesProtocol = false;

    if(ax_user)
    {
        flag_ReOpenProtocol = ax_user->dynamicCall("getPriv(QString)", "MASK_DATA").toBool();
        flag_CommonSettings = ax_user->dynamicCall("getPriv(QString)", "CHANGE_ASYS_PREF").toBool();
        flag_EditTest = ax_user->dynamicCall("getPriv(QString)", "EDIT_TEST").toBool();
        flag_SaveLocationWin = ax_user->dynamicCall("getPriv(QString)", "SAVE_LOCATION_WIN").toBool();

        temp = ax_user->dynamicCall("getAttr(QString)","dir:PR_HOME").toString().trimmed();
        if(!temp.isEmpty() && QDir(temp).exists()) user_Dir.setPath(temp);
    }

    //... ToolTip ...
    tooltip = tr("edit preference tests"); if(!flag_EditTest) tooltip += tr(" (unavailable)");
    editTestAct->setToolTip(tooltip);
    tooltip = tr("edit Protocols preference"); if(!flag_CommonSettings) tooltip += tr(" (unavailable)");
    edit_PreferenceProtocol->setToolTip(tooltip);
    tooltip = tr("reopen protocol with another Plate"); if(!flag_ReOpenProtocol) tooltip += tr(" (unavailable)");
    reopenXmlAct->setToolTip(tooltip);


    select();   // sel show()
    return(true);
}

//-----------------------------------------------------------------------------
//--- create_TestVerification
//-----------------------------------------------------------------------------
bool Analysis::create_TestVerification(QAxObject *axgp_resource)
{
    axgp = axgp_resource;

    return(true);
}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Analysis::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    ID_LANG = ID_LANG_RU;

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/analysis_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        if(text == "en") ID_LANG = ID_LANG_EN;

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analysis::resize_splitter(int pos, int index)
{
    main_progress->setFixedWidth(pos);
    Name_Protocol->setFixedWidth(pos - Label_Protocol->width());
}

//-----------------------------------------------------------------------------
//--- NetWork
//-----------------------------------------------------------------------------
void Analysis::create_NetWork()
{
    net_manager_GET = new QNetworkAccessManager(this);
    connect(net_manager_GET, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyFinished(QNetworkReply*)));

    net_manager_POST = new QNetworkAccessManager(this);
    connect(net_manager_POST, SIGNAL(finished(QNetworkReply*)),this, SLOT(replyFinished_POST(QNetworkReply*)));

    //net_adress = "http://172.16.0.1//tisprotocol.php";
}

//-----------------------------------------------------------------------------
//--- Slot replyFinished() for QNetworkAccessManager
//-----------------------------------------------------------------------------

void Analysis::replyFinished(QNetworkReply *reply)
{
    int i;
    QMenu menu;
    QAction *act = NULL;
    QString text, str;
    bool save = false;
    QStringList list;
    QVector<xml_info*> vec_info;

    QString fn = fn_IN; //qApp->applicationDirPath() + "/data/in.rt";
    QFile file(fn);


    QString request_cmd = reply->request().url().toString();
    QByteArray data = reply->readAll();

    //qDebug() << request_cmd;
    //qDebug() << data;

    if(data.isEmpty()) return;


    //.........................................................................
    if(request_cmd.contains(net_adress.toString()) &&      //--- GET XML Protocols(Bace64) ---
       request_cmd.contains("uin=") &&
       request_cmd.contains("sender="))
  {

        //qDebug() << "single xml";
        //text = QString::fromUtf8(QByteArray::fromBase64(data));
        //qDebug() << "size = " << text.size();
        if(file.open(QIODevice::WriteOnly))
        {
            file.write(QByteArray::fromBase64(data));
            file.close();

            if(current_XML.sender  == "amplif") save = true;
            //saveXmlAct->setEnabled(true);

            if(file.exists()) open_Protocol(fn,save);
        }

        return;
  }

    //.........................................................................
    if(request_cmd == net_adress.toString())               //--- GET LIST XML Protocols ---
  {
    //qDebug() << "list xml";

    QXmlStreamReader xml(data);
    while(!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::StartDocument) continue;
        if(token == QXmlStreamReader::StartElement)
        {
           if(xml.name() == "li")
           {
                QXmlStreamAttributes attrib = xml.attributes();

                //list.append(attrib.value("name").toString() + " (" +
                //            attrib.value("sender").toString() + ")");

                xml_info *li_info = new xml_info;
                vec_info.append(li_info);

                li_info->idtask = attrib.value("idtask").toString();
                li_info->uin = attrib.value("uin").toString();
                li_info->barcode = attrib.value("barcode").toString();
                li_info->income = attrib.value("income").toString();
                li_info->name = attrib.value("name").toString();
                li_info->target_dt = attrib.value("target_dt").toString();
                li_info->suin = attrib.value("suin").toString();
                li_info->sent = attrib.value("sent").toString();
                li_info->sender = attrib.value("sender").toString();
                li_info->datasize = attrib.value("datasize").toString();
                li_info->filename = "";
           }
        }
    }

    for(i=0; i<vec_info.size(); i++)
    {
        text = vec_info.at(i)->name + " (" + vec_info.at(i)->sender + ")";
        if(text.isEmpty()) continue;

        str = "";
        if(vec_info.at(i)->target_dt == "384") str = ":/images/Device/384.png";
        if(vec_info.at(i)->target_dt == "192") str = ":/images/Device/192.png";
        if(vec_info.at(i)->target_dt == "96") str = ":/images/Device/96.png";
        if(vec_info.at(i)->target_dt == "48") str = ":/images/Device/48.png";
        act = new QAction(QIcon(str), text, this);
        menu.addAction(act);

        text = vec_info.at(i)->sender;
        if(text == "amplif" ||
           text == "analysis" ||
           text == "reanalysis")  act->setEnabled(true);
        else act->setEnabled(false);
    }

    if(menu.actions().size())
    {
        act = menu.exec(QCursor::pos());
        if(act)
        {
            text = act->text();

            // do something...
            list = text.split(" ");
            for(i=0; i<vec_info.size(); i++)
            {
                if(list.at(0) == vec_info.at(i)->name &&
                   list.at(1).contains(vec_info.at(i)->sender))
                {
                    get_XmlProtocol(vec_info.at(i));
                    break;
                }
            }
        }
    }

    menu.clear();

    for(i=0; i<vec_info.size(); i++)
    {
        xml_info *p = vec_info.at(i);
        delete p;
    }
    vec_info.clear();

    return;
  } 
}

//-----------------------------------------------------------------------------
//--- Slot replyFinished_POST() for QNetworkAccessManager
//-----------------------------------------------------------------------------

void Analysis::replyFinished_POST(QNetworkReply *reply)
{
    bool sts_uin;
    bool res = false;
    QString text;

    QByteArray data = reply->readAll();
    //qDebug() << data;
    QString request_cmd = reply->request().url().toString();


    if(request_cmd == net_adress.toString())
    {
        QXmlStreamReader xml(data);
        while(!xml.atEnd() && !xml.hasError())
        {
            QXmlStreamReader::TokenType token = xml.readNext();
            if(token == QXmlStreamReader::StartDocument) continue;
            if(token == QXmlStreamReader::StartElement)
            {
               if(xml.name() == "li")
               {
                    QXmlStreamAttributes attrib = xml.attributes();

                    if(!current_XML.uin.isEmpty())
                    {
                        if(attrib.value("uin").toString() == current_XML.uin) sts_uin = true;
                        else sts_uin = false;
                    }
                    else sts_uin = true;
                    if(attrib.value("name").toString() == current_XML.name && sts_uin)
                    {
                        res = true;
                        break;
                    }
               }
            }
        }
    }

    message.setStandardButtons(QMessageBox::Ok);
    if(res)
    {
        text = "Protocol (" + current_XML.name + ") is successfully loaded into the database...";
        message.setIcon(QMessageBox::Information);
        message.button(QMessageBox::Ok)->animateClick(5000);
    }
    else
    {
        text = "an error was detected when loading the file to the database...";
        message.setIcon(QMessageBox::Warning);
    }
    message.setText(text);
    message.exec();

}
//-----------------------------------------------------------------------------
//--- Slot get_XmlProtocol(xml_info *)
//-----------------------------------------------------------------------------
void Analysis::get_XmlProtocol(xml_info *p_xml)
{
    QNetworkRequest request;

    QString url = net_adress.toString() + "?uin=" + p_xml->uin +
                                          "&sender=" + p_xml->sender;

    QUrl adress = QUrl(url);

    request.setUrl(adress);
    net_manager_GET->get(request);

    current_XML = *p_xml;
    //qDebug() << "xml_info = " << current_XML.name;

}

//-----------------------------------------------------------------------------
//--- Slot send_XmlProtocol(xml_info *)
//-----------------------------------------------------------------------------
void Analysis::send_XmlProtocol(xml_info *p_info)
{
    QNetworkRequest request;
    QUrlQuery postData;
    QByteArray data;

    QString sender_str = "analysis";
    if(p_info->sender == sender_str) sender_str = "reanalysis";

    request.setUrl(net_adress);

    //--- file ---
    QString fn = p_info->filename;
    QFile file(fn);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        data = file.readAll();
        file.close();
    }
    else return;
    //------------

    postData.addQueryItem("uin", current_XML.uin);
    postData.addQueryItem("sender", sender_str);
    postData.addQueryItem("data", data.toBase64());

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    net_manager_POST->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

}

//-----------------------------------------------------------------------------
//--- ChartGroupBox
//-----------------------------------------------------------------------------
void Analysis::createChartGroupBox()
{

    main_spl = new QSplitter(Qt::Horizontal);
    main_spl->setHandleWidth(6);
    main_spl->setChildrenCollapsible(false);



    //--- chart ---
    ChartWindow = new QMainWindow();

    ChartBox = new QGroupBox(ChartWindow);
    ChartBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    ChartBox->setMinimumWidth(200);
    ChartWindow->setCentralWidget(ChartBox);
    main_spl->addWidget(ChartWindow);
    layout_Chart = new QHBoxLayout(ChartBox);
    layout_Chart->setMargin(1);
    layout_ChartTool = new QVBoxLayout();

    plot = Create_Graph(this);
    layout_Chart->addWidget(plot);
    connect(plot, SIGNAL(clear_rejimes(int)), this, SLOT(Clear_Rejimes(int)));

    //... chartToolBar
    chartToolBar = new QToolBar((tr("ChartBar")), ChartBox);
    chartToolBar->setOrientation(Qt::Vertical);    
    chartToolBar->addAction(CtAct);
    chartToolBar->addAction(LgAct);
    chartToolBar->addAction(NormalizationAct);    
    //chartToolBar->addSeparator();
    chartToolBar->addAction(MeltingAct);
    chartToolBar->addAction(ColorAct);
    chartToolBar->addAction(dFdT_negative);
    chartToolBar->setVisible(false);
    //chartToolBar->addAction(curveEdit);
    //chartToolBar->addAction(refreshEdit);

    curveEdit->setEnabled(false);
    refreshEdit->setEnabled(false);
    //chartToolBar->addSeparator();

    //... helpToolBar
    helpToolBar = new QToolBar((tr("HelpBar")), ChartBox);
    helpToolBar->setOrientation(Qt::Vertical);
    //UpDown_Marker = new QSpinBox(this);

    helpToolBar->addAction(Increase_Scale);
    helpToolBar->addAction(Decrease_Scale);
    helpToolBar->addAction(Auto_Scale);
    helpToolBar->addSeparator();
    helpToolBar->addAction(UpMarker);
    helpToolBar->addAction(DownMarker);
    helpToolBar->addAction(UpThickness);
    helpToolBar->addAction(DownThickness);
    helpToolBar->addSeparator();
    helpToolBar->addAction(helpChartAct);

    layout_ChartTool->addWidget(chartToolBar);
    layout_ChartTool->addWidget(helpToolBar, 1, Qt::AlignBottom);
    layout_Chart->addLayout(layout_ChartTool);


    //--- grid ---
    GridWindow = new QMainWindow();

    GridTab = new QTabWidget(GridWindow);
    GridTab->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Ignored);
    GridTab->setMinimumWidth(200);
    GridWindow->setCentralWidget(GridTab);
    main_spl->addWidget(GridWindow);    

    GridBox_allcp = new GroupBox(GridTab);
    GridBox_allcp->setObjectName("Transparent");
    GridBox_allcp->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    /*report_CpAll = new QPushButton(QIcon(":/images/report.png"), "", GridBox_allcp);
    if(StyleApp == "fusion") report_CpAll->setIcon(QIcon(":/images/flat/report_preanalysis.png"));
    report_CpAll->setObjectName("Transparent");
    report_CpAll->setToolTip(tr("report"));
    report_CpAll->setIconSize(QSize(24,24));
    report_CpAll->setFixedSize(32,26);
    report_CpAll->setFlat(true);
    connect(report_CpAll, SIGNAL(clicked(bool)), this, SLOT(report_Param()));*/
    //GridBox_allcp->setMinimumWidth(200);
    layout_Grid = new QVBoxLayout(GridBox_allcp);
    GridTab->addTab(GridBox_allcp,QIcon(":/images/report.png"),tr("Parameterization DATA"));    

    GridBox_ext_globe = new GroupBox(GridTab);
    GridBox_ext_globe->setObjectName("Transparent");
    GridBox_ext_globe->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    layout_Grid_globe = new QVBoxLayout(GridBox_ext_globe);
    Tab_AnalyserGlobe = new QTabWidget(GridTab);
    layout_Grid_globe->addWidget(Tab_AnalyserGlobe);
    Tab_AnalyserGlobe->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    GridTab->addTab(GridBox_ext_globe, QIcon(":/images/globe_0.png"),tr("Bace Analysis"));

    GridBox_ext_all = new GroupBox(GridTab);
    GridBox_ext_all->setObjectName("Transparent");
    GridBox_ext_all->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    layout_Grid_ext = new QVBoxLayout(GridBox_ext_all);
    GridBox_ext_dna = new GroupBox(GridTab);
    GridBox_ext_dna->setObjectName("Transparent");
    //Tab_Analyser = new QTabWidget(GridTab);
    layout_Grid_ext->addWidget(GridBox_ext_dna);
    //Tab_Analyser->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

    //GridBox_ext = new GroupBox(GridTab);
    //layout_Grid_ext->addWidget(GridBox_ext);
    //GridBox_ext->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

    GridTab->addTab(GridBox_ext_all, QIcon(":/images/dna_logo.png"),tr("Analysis"));


    Attention_Browser = new QTextEdit();
    Attention_Browser->setReadOnly(true);

    message_Marker.setWindowIcon(QIcon(":/images/DTm.ico"));
    message_Marker.setFont(qApp->font());
    cb_MarkerAttention = new QCheckBox(tr("Don't show again"));
    message_Marker.setCheckBox(cb_MarkerAttention);
    MarkerAttention = false;
    connect(cb_MarkerAttention, SIGNAL(stateChanged(int)), this, SLOT(change_MarkerAttention(int)));


    for(int i=0; i<GridTab->count(); i++) GridTab->setTabEnabled(i,false);
    connect(GridTab, SIGNAL(currentChanged(int)), this, SLOT(Change_IndexTabGrid(int)));

    if(StyleApp == "fusion")
    {
        GridTab->setTabIcon(0, QIcon(":/images/flat/chart_flat.png"));
        GridTab->setTabIcon(1, QIcon(":/images/flat/global_disable_flat.png"));
        GridTab->setTabIcon(2, QIcon(":/images/flat/dna_logo_disable_flat.png"));
    }
}

//-----------------------------------------------------------------------------
//--- Select Tubes
//-----------------------------------------------------------------------------
void Analysis::createSelectTubes()
{
    sel = NULL;     // pointer for Select_Tubes object
    sel = new Select_tubes(tr("Test&&Sample&&Select"),this);    
    sel->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    //...
    sel->setFeatures(QDockWidget::NoDockWidgetFeatures);

    sel->map_TestTranslate = &Map_TestTranslate;
    //sel->setFloating(true);


    sel->hide();
    sel->mTitleBar->list_MainWindow.append(ChartWindow);
    //sel->mTitleBar->list_MainWindow.append(GridWindow);
    sel->mTitleBar->updateStyleChange();

    connect(sel->mTitleBar, SIGNAL(close_Signal()), this, SLOT(ShowHidden_Select()));
}

//-----------------------------------------------------------------------------
//--- Actions
//-----------------------------------------------------------------------------
void Analysis::createActions()
{
    open_ListXML = new QAction(QIcon(":/images/open_db.png"), tr("Open list XML protocol"), this);
    connect(open_ListXML, SIGNAL(triggered()), this, SLOT(view_listXmlProtocols()));

    saveXmlAct = new QAction(QIcon(":/images/save_db.png"), tr("Save XML..."), this);
    connect(saveXmlAct, SIGNAL(triggered()), this, SLOT(save_XML()));
    saveXmlAct->setEnabled(false);

    openXmlAct = new QAction(QIcon(":/images/open_new.png"), tr("Open protocol"), this);
    if(StyleApp == "fusion") openXmlAct->setIcon(QIcon(":/images/flat/open_flat.png"));
    connect(openXmlAct, SIGNAL(triggered()), this, SLOT(open()));

    open_LastProtocols = new QAction(QIcon(":/images/list.png"), tr("Open list of protocol"), this);
    if(StyleApp == "fusion") open_LastProtocols->setIcon(QIcon(":/images/flat/list_flat.png"));
    connect(open_LastProtocols, SIGNAL(triggered()), this, SLOT(view_listProtocols()));

    saveAct = new QAction(QIcon(":/images/save_new.png"), tr("Save as..."), this);
    if(StyleApp == "fusion") saveAct->setIcon(QIcon(":/images/flat/save_flat.png"));
    saveAct->setShortcuts(QKeySequence::Save);
    //saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    saveAct->setEnabled(false);

    selectAct = new QAction(QIcon(":/images/color.png"), tr("View"), this);
    if(StyleApp == "fusion") selectAct->setIcon(QIcon(":/images/flat/selection.png"));
    //exitAct->setStatusTip(tr("Exit the application"));
    connect(selectAct, SIGNAL(triggered()), this, SLOT(select()));
    selectAct->setEnabled(false);

    infoAct = new QAction(QIcon(":/images/info.png"), tr("info..."), this);
    if(StyleApp == "fusion") infoAct->setIcon(QIcon(":/images/flat/info_flat.png"));
    connect(infoAct, SIGNAL(triggered(bool)), this, SLOT(info_Protocol()));
    infoAct->setEnabled(false);

    reportAct = new QAction(QIcon(":/images/report.png"), tr("report..."), this);
    if(StyleApp == "fusion") reportAct->setIcon(QIcon(":/images/flat/report_flat.png"));
    connect(reportAct, SIGNAL(triggered(bool)), this, SLOT(report()));
    reportParamAct = new QAction(QIcon(":/images/flat/report_preanalysis.png"), tr("report Parameterization..."), this);
    connect(reportParamAct, SIGNAL(triggered(bool)), this, SLOT(report_Param()));
    excelAct = new QAction(QIcon(":/images/save_xml_.png"), tr("excel..."), this);
    reportAct->setEnabled(false);
    reportParamAct->setEnabled(false);

    exportResults = new QAction(QIcon(":/images/flat/export_LIS.png"), tr("export results"), this);
    connect(exportResults, SIGNAL(triggered(bool)), this, SLOT(Export_Results()));
    exportResults->setEnabled(false);

    helpChartAct = new QAction(QIcon(":/images/help_chart.png"), tr("help Chart..."), this);
    connect(helpChartAct, SIGNAL(triggered(bool)), this, SLOT(helpChart()));

    CtAct = new QAction(QIcon(":/images/Ct.png"), tr("Ct"), this);
    CtAct->setCheckable(true);
    CtAct->setVisible(false);
    connect(CtAct, SIGNAL(triggered(bool)), this, SLOT(Ct_Chart()));

    LgAct = new QAction(QIcon(":/images/Lg.png"), tr("Lg"), this);
    LgAct->setCheckable(true);
    LgAct->setVisible(false);
    connect(LgAct, SIGNAL(triggered(bool)), this, SLOT(Lg_Chart()));

    NormalizationAct = new QAction(QIcon(":/images/Norm.png"), tr("Normalization"), this);
    NormalizationAct->setCheckable(true);
    NormalizationAct->setVisible(false);
    connect(NormalizationAct, SIGNAL(triggered(bool)), this, SLOT(Norm_Chart()));

    MeltingAct = new QAction(QIcon(":/images/MeltingMarker.png"), tr("Melting Marker"), this);
    MeltingAct->setCheckable(true);
    MeltingAct->setVisible(false);
    connect(MeltingAct, SIGNAL(triggered(bool)), this, SLOT(Melting_Chart()));

    ColorAct = new QAction(QIcon(":/images/ColorResult.png"), tr("Color result"), this);
    ColorAct->setCheckable(true);
    ColorAct->setVisible(false);
    connect(ColorAct, SIGNAL(triggered(bool)), this, SLOT(ColorMelting_Chart()));

    dFdT_negative = new QAction(QIcon(":/images/dFdT_negative.png"), tr("-(dF)/(dT)"), this);
    dFdT_negative->setCheckable(true);
    dFdT_negative->setVisible(false);
    connect(dFdT_negative, SIGNAL(triggered(bool)), this, SLOT(dFdT_Negative()));

    Increase_Scale = new QAction(QIcon(":/images/flat/increase.png"), tr("increase scale"), this);
    connect(Increase_Scale, SIGNAL(triggered(bool)), this, SLOT(Increase_PlotScale()));

    Decrease_Scale = new QAction(QIcon(":/images/flat/decrease.png"), tr("decrease scale"), this);
    connect(Decrease_Scale, SIGNAL(triggered(bool)), this, SLOT(Decrease_PlotScale()));

    Auto_Scale = new QAction(QIcon(":/images/flat/auto_scale.png"), tr("auto scale"), this);
    connect(Auto_Scale, SIGNAL(triggered(bool)), this, SLOT(Auto_PlotScale()));

    UpMarker = new QAction(QIcon(":/images/marker_up.png"), tr("marker up"), this);
    if(StyleApp == "fusion") UpMarker->setIcon(QIcon(":/images/flat/marker_up_flat.png"));
    connect(UpMarker, SIGNAL(triggered(bool)), this, SLOT(Up_sizeMarker()));

    DownMarker = new QAction(QIcon(":/images/marker_down.png"), tr("marker down"), this);
    if(StyleApp == "fusion") DownMarker->setIcon(QIcon(":/images/flat/marker_down_flat.png"));
    connect(DownMarker, SIGNAL(triggered(bool)), this, SLOT(Down_sizeMarker()));

    UpThickness = new QAction(QIcon(":/images/flat/brightness_up_new.png"), tr("thickness up"), this);
    connect(UpThickness, SIGNAL(triggered(bool)), this, SLOT(Up_Thickness()));

    DownThickness = new QAction(QIcon(":/images/flat/brightness_down_new.png"), tr("thickness down"), this);
    connect(DownThickness, SIGNAL(triggered(bool)), this, SLOT(Down_Thickness()));

    curveEdit = new QAction(QIcon(":/images/curve_edit.png"), tr("curve_edit"), this);
    curveEdit->setCheckable(true);
    connect(curveEdit, SIGNAL(triggered(bool)), this, SLOT(Curve_Edit(bool)));

    refreshEdit = new QAction(QIcon(":/images/refresh.ico"), tr("refresh_edit"), this);
    connect(refreshEdit, SIGNAL(triggered(bool)), this, SLOT(Refresh_Edit()));

    load_ToSetup = new QAction(QIcon(":/images/ToSetup.png"), tr("load protocol to Setup"), this);
    if(StyleApp == "fusion") load_ToSetup->setIcon(QIcon(":/images/flat/To_Protocol.png"));
    connect(load_ToSetup, SIGNAL(triggered()), this, SLOT(load_to_setup()));
    load_ToSetup->setEnabled(false);

    reopenXmlAct = new QAction(QIcon(":/images/reopen.png"), tr("reopen protocol with another Plate"), this);
    if(StyleApp == "fusion") reopenXmlAct->setIcon(QIcon(":/images/flat/reopen_flat.png"));
    connect(reopenXmlAct, SIGNAL(triggered()), this, SLOT(reopen()));
    reopenXmlAct->setEnabled(false);

    editTestAct = new QAction(QIcon(":/images/edit_test.png"), tr("edit preference tests"), this);
    if(StyleApp == "fusion") editTestAct->setIcon(QIcon(":/images/flat/edit_Test_flat.png"));
    connect(editTestAct, SIGNAL(triggered()), this, SLOT(editTest()));
    editTestAct->setEnabled(false);

    edit_PreferenceProtocol = new QAction(QIcon(":/images/preference_prot.png"), tr("edit Protocols preference"), this);
    if(StyleApp == "fusion") edit_PreferenceProtocol->setIcon(QIcon(":/images/flat/edit_Protocol_flat.png"));
    connect(edit_PreferenceProtocol, SIGNAL(triggered()), this, SLOT(editPreferencePro()));
    edit_PreferenceProtocol->setEnabled(false);

    preview_videoarchive = new QAction(tr("Preview Video Archive"), this);
    connect(preview_videoarchive, SIGNAL(triggered()), this, SLOT(preview_archive()));

    EMail_send = new QAction(QIcon(":/images/email_32_24.png"), tr("Send message via Email"), this);
    connect(EMail_send, SIGNAL(triggered()), this, SLOT(email_Send()));
    //EMail_send->setEnabled(false);

    RDML_import = new QAction(QIcon(":/images/flat/rdml.png"), tr("RDML import"), this);
    connect(RDML_import, &QAction::triggered, this, &Analysis::rdml_import);

    Validity_Hash = new QAction(QIcon(":/images/flat/star_null.png"), tr("data safety"), this);

}
//-----------------------------------------------------------------------------
//--- Refresh_Edit()
//-----------------------------------------------------------------------------
void Analysis::Refresh_Edit()
{

}
//-----------------------------------------------------------------------------
//--- Curve_Edit()
//-----------------------------------------------------------------------------
void Analysis::Curve_Edit(bool checked)
{
    plot->curve_edit = checked;
    refreshEdit->setEnabled(checked);
}
//-----------------------------------------------------------------------------
//--- helpChart()
//-----------------------------------------------------------------------------
void Analysis::helpChart()
{
    plot->helpChart();
}
//-----------------------------------------------------------------------------
//--- Ct_Chart()
//-----------------------------------------------------------------------------
void Analysis::Ct_Chart()
{
    if(!plot->Ct_Chart(CtAct->isChecked())) CtAct->setChecked(false);
}
//-----------------------------------------------------------------------------
//--- Norm_Chart()
//-----------------------------------------------------------------------------
void Analysis::Norm_Chart()
{    
    if(LgAct->isChecked() && NormalizationAct->isChecked()) {LgAct->setChecked(false); Lg_Chart();}
    if(!plot->Norm_Chart(NormalizationAct->isChecked())) NormalizationAct->setChecked(false);
}
//-----------------------------------------------------------------------------
//--- Melting_Chart()
//-----------------------------------------------------------------------------
void Analysis::Melting_Chart()
{
    if(!plot->Melt_Chart(MeltingAct->isChecked())) MeltingAct->setChecked(false);
}
//-----------------------------------------------------------------------------
//--- dFdT_Negative()
//-----------------------------------------------------------------------------
void Analysis::dFdT_Negative()
{
    if(!prot || prot->count_MC <= 0 || plot->list_Curve.size() == 0) return;

    int ch = plot->current_ch;
    int rejime = plot->current_rejime;
    int type = plot->current_type;
    bool state = false;
    //Analysis_Interface *analyser;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    label_gif->setVisible(true);
    obj_gif->start();

    if(dFdT_negative->isChecked()) state = true;
    dFdT_MCData(this, Analysis::Wrapper_To_Display_ProgressBar, prot, tr("Filtered & dF/Dt Melting Curve Data"), state);


    plot->Draw_Chart(plot->prot, rejime, ch, type);
    plot->replot();

    if(alg)
    {
        alg->Analyser(prot);        
    }
    foreach(analyser, Map_analyser.values())
    {
        analyser->Analyser(prot);
    }

    label_gif->setVisible(false);
    obj_gif->stop();
    QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
//--- ColorMelting_Default()
//-----------------------------------------------------------------------------
void Analysis::ColorMelting_Default()
{
    if(plot->current_type == 0x02)
    {
        ColorAct->setChecked(false);
        emit ColorAct->triggered();
    }
}

//-----------------------------------------------------------------------------
//--- ColorMelting_Chart()
//-----------------------------------------------------------------------------
void Analysis::ColorMelting_Chart()
{
    int i;
    char *p_ch;
    QString text;
    int ch = plot->current_ch;
    int id = -1;
    for(i=0; i<COUNT_CH; i++)
    {
        if(prot->active_Channels & (0x0f <<4*i)) id++;
        if(id == ch)
        {
            ch = i;
            break;
        }
    }


    int rejime = plot->current_rejime;
    int type = plot->current_type;

    bool state = false;
    if(ColorAct->isChecked()) state = true;

    //qDebug() << "ColorMelting_Chart(): " << ch << state;
    if(state && plot->Fluor_Box->currentIndex() >= COUNT_CH)
    {
        state = false;
        ColorAct->blockSignals(true);
        ColorAct->setChecked(false);
        ColorAct->blockSignals(false);
    }

    if(state)
    {
        if(ext_dll_handle && get_color && ext_dll_handle_VALID)
        {            
            get_color(&p_ch);            
            text = QString(p_ch);

            ResultColor_Melting(text, ch);
            plot->Draw_Chart(plot->prot, rejime, ch, type);
            plot->replot();
            //if(!sel->isHidden() && sel->Tab_Select->currentIndex() == 1) sel->Tab_Select->setCurrentIndex(0);
            //sel->Select_Grid->update();
            sel->Select_Grid->viewport()->update();
        }
    }
    else
    {
        for(i=0; i<prot->count_Tubes; i++)
        {
            prot->color_tube[i] = sel->ColorTube_Current.at(i); //sel->ColorTube_Initial.at(i);
        }
        plot->Draw_Chart(plot->prot, rejime, ch, type);
        plot->replot();
        sel->Select_Grid->viewport()->update();
    }


}

//-----------------------------------------------------------------------------
//--- ResultColor_Melting(QString str)
//-----------------------------------------------------------------------------
void Analysis::ResultColor_Melting(QString str, int ch)
{
    int i,j,k;
    int channel;
    int value, id;
    int col,row;
    QString text, s;
    bool ok;
    int r,g,b;
    QColor color;

    QDomDocument doc;
    QDomNode     child;
    QDomElement  item;
    QDomNode     item_group;

    QMap<QString,int> map;


    if(doc.setContent(str))
    {
        QDomElement root = doc.documentElement();

        for(i=0; i<root.childNodes().size(); i++)
        {
            child = root.childNodes().at(i);
            if(child.attributes().contains("Position"))
            {
                text = child.toElement().attribute("Position", "");
                if(text.trimmed().isEmpty()) continue;

                for(k=0; k<child.childNodes().size(); k++)
                {
                    item_group = child.childNodes().at(k);
                    for(j=0; j<item_group.childNodes().size(); j++)
                    {
                        item = item_group.childNodes().at(j).toElement();
                        if(item.nodeName() == "Channel")
                        {
                            s = item.attribute("ID","");
                            channel = s.toInt(&ok);
                            if(!ok) continue;
                            if(channel == ch)
                            {
                                s = item.attribute("Color","$00000000");
                                s = s.replace("$","");
                                value = s.toInt(&ok,16);
                                if(ok) map.insert(text,value);
                                break;
                            }
                        }
                    }
                }
            }

        }
        //qDebug() << "map: " << map << ch;
        doc.clear();

        //... replace color ...
        prot->Plate.PlateSize(prot->count_Tubes, row, col);
        foreach(text, map.keys())
        {
            id = Convert_NameToIndex(text,col);
            color = map.value(text);
            color.getRgb(&b,&g,&r);
            color.setRgb(r,g,b);

            if(r > 0xE0) color = Qt::red;
            if(b > 0xE0) color = Qt::blue;
            if(g > 0xE0) color = Qt::darkGreen;
            if(r > 0xE0 && g > 0xE0) color = Qt::yellow;
            value = color.rgb();

            prot->color_tube.at(id) = value;
        }


        map.clear();
    }
}

//-----------------------------------------------------------------------------
//--- Lg_Chart()
//-----------------------------------------------------------------------------
void Analysis::Lg_Chart()
{
    if(LgAct->isChecked() && NormalizationAct->isChecked()) {NormalizationAct->setChecked(false); Norm_Chart();}
    if(!plot->Lg_Chart(LgAct->isChecked())) LgAct->setChecked(false);
}
//-----------------------------------------------------------------------------
//--- Clear_Rejimes(int type)
//-----------------------------------------------------------------------------
void Analysis::Clear_Rejimes(int type)
{
    bool sts = false;
    bool sts_m = false;

    CtAct->setChecked(false);
    LgAct->setChecked(false);
    NormalizationAct->setChecked(false);
    MeltingAct->setChecked(false);
    ColorAct->setChecked(false);
    if(type == -1) dFdT_negative->setChecked(false);

    switch(type)
    {
    default:    sts = false; break;
    case 0:     sts = true; break;
    case 1:     sts_m = true; break;
    }

    if(type == 0 || type == 1) chartToolBar->setVisible(true);

    //qDebug() << "chartToolBar: " << chartToolBar->isVisible();

    CtAct->setVisible(sts); // setEnabled(sts);
    LgAct->setVisible(sts); // setEnabled(sts);
    NormalizationAct->setVisible(sts); // setEnabled(sts);

    MeltingAct->setVisible(sts_m); // setEnabled(sts_m);
    ColorAct->setVisible(sts_m); // setEnabled(sts_m);
    dFdT_negative->setVisible(sts_m); // setEnabled(sts_m);
}

//-----------------------------------------------------------------------------
//--- Increase_PlotScale()
//-----------------------------------------------------------------------------
void Analysis::Increase_PlotScale()
{
    if(!plot || !prot) return;

    plot->Increase_Scale();
}
//-----------------------------------------------------------------------------
//--- Decrease_PlotScale()
//-----------------------------------------------------------------------------
void Analysis::Decrease_PlotScale()
{
    if(!plot || !prot) return;

    plot->Decrease_Scale();
}
//-----------------------------------------------------------------------------
//--- Auto_PlotScale()
//-----------------------------------------------------------------------------
void Analysis::Auto_PlotScale()
{
    if(!plot || !prot) return;

    plot->Auto_Scale();
}
//-----------------------------------------------------------------------------
//--- Up_sizeMarker()
//-----------------------------------------------------------------------------
void Analysis::Up_sizeMarker()
{
    if(!plot || !prot) return;

    plot->size_symbol++;
    plot->Change_SizeMarker();
}
//-----------------------------------------------------------------------------
//--- Down_sizeMarker()
//-----------------------------------------------------------------------------
void Analysis::Down_sizeMarker()
{
    if(!plot || !prot) return;
    if(plot->list_Curve.size() == 0) return;

    if(plot->size_symbol > 0)
    {
        plot->size_symbol--;
        plot->Change_SizeMarker();
    }
}
//-----------------------------------------------------------------------------
//--- Up_Thickness()
//-----------------------------------------------------------------------------
void Analysis::Up_Thickness()
{
    if(!plot || !prot) return;
    if(plot->list_Curve.size() == 0) return;

    plot->thickness_line++;
    plot->Change_ThicknessLine();
}
//-----------------------------------------------------------------------------
//--- Down_Thickness()
//-----------------------------------------------------------------------------
void Analysis::Down_Thickness()
{
    if(!plot || !prot) return;

    if(plot->thickness_line > 1)
    {
        plot->thickness_line--;
        plot->Change_ThicknessLine();
    }
}
//-----------------------------------------------------------------------------
//--- Action view_listXmlProtocols()
//-----------------------------------------------------------------------------
void Analysis::view_listXmlProtocols()
{
    QNetworkRequest request;

    request.setUrl(net_adress);
    net_manager_GET->get(request);

}
//-----------------------------------------------------------------------------
//--- Action save_XML()
//-----------------------------------------------------------------------------
void Analysis::save_XML()
{
    QNetworkRequest request;
    QUrlQuery postData;
    QByteArray data;

    QDateTime local;
    QString text = QString::fromStdString(prot->regNumber) + "_" +
                   local.currentDateTime().toString("hh:mm:ss_dd.MM.yyyy");

    request.setUrl(net_adress);

    //--- file ---
    QFile file(fn_OUT);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        data = file.readAll();
        file.close();
    }
    else return;
    //------------
    current_XML.name = text;
    current_XML.uin = "";

    postData.addQueryItem("name", text);
    postData.addQueryItem("barcode", "");
    postData.addQueryItem("target", QString::number(prot->count_Tubes));
    postData.addQueryItem("sender", "reanalysis");
    postData.addQueryItem("data", data.toBase64());

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    net_manager_POST->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

}
//-----------------------------------------------------------------------------
//--- Action open()
//-----------------------------------------------------------------------------
void Analysis::open()
{
    Save_ChangesProtocol();

    open_Protocol();
}
//-----------------------------------------------------------------------------
//--- Action save()
//-----------------------------------------------------------------------------
void Analysis::save()
{
    rt_Sample       *psample;
    rt_GroupSamples *pgroup;
    rt_Tube         *ptube;
    int pos;
    bool save_color = false;
    bool save_Comments = false;


    //qDebug() << "save: " << prot->preference_Pro.size() << prot;

    //QString FileName = QString("Protocol(%1)_%2.rt").arg(prot->count_Tubes).arg(QDateTime::currentDateTime().toString("d-M-yy_H-m-s"));
    QString FileName = Check_ValidNameFile(QString::fromStdString(prot->name));
    QString FileName_Unique = Unique_FileName(FileName);
    QString dirName = user_Dir.absolutePath() + "/" + FileName_Unique;
    dirName = Original_FileName(dirName);

    bool sts = true;
    QString text = tr(" didn't save!");

    if(!prot || prot->Plate.groups.size() == 0) return;

    QFile file(fn_OUT);
    if(!file.exists()) return;

    FileName = QFileDialog::getSaveFileName(this,
                                            tr("Save as ..."),
                                            dirName,
                                            tr("Protocols File (*.rt)"));

    if(FileName.isEmpty()) return;
    if(FileName == fn_OUT)
    {
        QMessageBox::critical(this, tr("Error Write file!"), FileName + text);
        return;
    }


    if(Map_analyser.size() || alg)      // save result after "analyser_plugins"(my tests) and Parameterization DATA(All_Cp)
    {
        // 1. save common results...
        //SaveResultsXML(this, Analysis::Wrapper_To_Display_ProgressBar, prot, fn_OUT, fn_OUT);

        // 1.1 Check color change
        foreach(pgroup, prot->Plate.groups)
        {
            foreach(psample, pgroup->samples)
            {
                foreach(ptube, psample->tubes)
                {
                    pos = ptube->pos;
                    if(ptube->color != prot->color_tube.at(pos))
                    {
                        ptube->color = prot->color_tube.at(pos);
                        save_color = true;
                    }
                }
            }
        }

        // 1.2 Comments
        save_Comments = Comments_IsChanged();
        //qDebug() << "save_color: " << save_color << save_Comments;

        if(save_color || save_Comments)
        {
            Save_ColorComments(prot, fn_OUT, save_color, save_Comments);
        }
        /*if(save_color)*/ //SaveAsXML(this, Analysis::Wrapper_To_Display_ProgressBar, prot, fn_OUT, tr("Save as XML..."), false); //must save colors change?!


        // 2. save results from analysers (my plugins)
        //qDebug() << "Map_analyser: " << Map_analyser.size();
        foreach(analyser, Map_analyser)
        {
            //analyser->Save_Results(fn_OUT.toLatin1().data());
            analyser->Save_Results(fn_OUT.toUtf8().data());
        }
    }


    QFile file_out(FileName);
    if(file_out.exists())
    {
        sts = file_out.remove();
    }
    if(sts) sts = file.copy(FileName);
    if(!sts) QMessageBox::critical(this, tr("Error Write file!"), FileName + text);

    if(sts) FileName_Protocol->setText(FileName);

    if(sts) flag_SaveChangesProtocol = false;

}
//-----------------------------------------------------------------------------
//--- Save_ColorComments
//-----------------------------------------------------------------------------
void Analysis::Save_ColorComments(rt_Protocol *P, QString fn, bool save_color, bool save_comments)
{
    int i,j,k;
    int pos;
    QString text;
    bool state = false;
    bool find;
    rt_Preference   *preference;
    QColor color;
    int r,g,b,key;


    QDomDocument    doc;
    QDomElement     root;
    QDomElement     properties;
    QDomElement     source;
    QDomElement     element;
    QDomElement     samples;
    QDomElement     tubes;
    QDomElement     pos_element;
    QDomElement     color_element;
    QDomNode        node;
    QDomNode        child;
    QDomNode        item;

    QFile file(fn);

    // 1. Read File
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();
            state = true;

            root = doc.documentElement();

            if(save_comments)
            {
                text = "";
                foreach(preference, P->preference_Pro)
                {
                    if(preference->name == "Comments")
                    {
                        text = QString::fromStdString(preference->value);
                        break;
                    }
                }

                properties = root.firstChildElement("properties");
                if(!properties.isNull())
                {
                    find = false;
                    for(j=0; j<properties.childNodes().size(); j++)
                    {
                        node = properties.childNodes().at(j);
                        child = node.firstChildElement("name");
                        if(child.toElement().text() == "Comments")
                        {
                            child = node.firstChildElement("value");
                            node.replaceChild(MakeElement(doc,"value", text), child);
                            find = true;
                            break;
                        }
                    }
                    if(!find)
                    {
                        element = doc.createElement("item");
                        element.appendChild(MakeElement(doc,"name","Comments"));
                        element.appendChild(MakeElement(doc,"value",text));
                        element.appendChild(MakeElement(doc,"unit",""));
                        properties.appendChild(element);
                    }
                }
            }

            if(save_color)
            {
                source = root.firstChildElement("sourceSet");
                if(!source.isNull())
                {
                    for(i=0; i<source.childNodes().size(); i++)
                    {
                        node = source.childNodes().at(i);
                        samples = node.firstChildElement("samples");
                        if(samples.isNull()) break;
                        for(j=0; j<samples.childNodes().size(); j++)
                        {
                            child = samples.childNodes().at(j);
                            tubes = child.firstChildElement("tubes");
                            if(tubes.isNull()) break;
                            for(k=0; k<tubes.childNodes().size(); k++)
                            {
                                item = tubes.childNodes().at(k);
                                pos_element = item.firstChildElement("pos");
                                pos = pos_element.text().toInt();
                                if(pos >= 0 && pos < P->count_Tubes)
                                {
                                    color_element = item.firstChildElement("color");

                                    color = P->color_tube.at(pos);
                                    color.getRgb(&r,&g,&b);
                                    key = QColor(b,g,r).rgb() & 0xffffff;
                                    item.replaceChild(MakeElement(doc,"color",QString::number(key,16)), color_element);
                                }
                            }
                        }
                    }
                }
            }
        }
        else file.close();

        // 2. Write
        if(file.open(QIODevice::WriteOnly) && state)
        {
            QTextStream(&file) << doc.toString();
            file.close();
        }

        doc.clear();
    }
}
//-----------------------------------------------------------------------------
//--- Action select()
//-----------------------------------------------------------------------------
void Analysis::select()
{

    if(!sel->isHidden()) return;

    sel->show();
    sel->raise();
    sel->activateWindow();
    selectAct->setEnabled(false);

    ChartWindow->addDockWidget(Qt::TopDockWidgetArea, sel);
}
//-----------------------------------------------------------------------------
//--- info_Protocol()
//-----------------------------------------------------------------------------
void Analysis::info_Protocol()
{
    Info_Pro->show();
    //Info_Pro->refresh_Info(prot);
}
//-----------------------------------------------------------------------------
//--- Export_Results()
//-----------------------------------------------------------------------------
void Analysis::Export_Results()
{
    if(export_results) export_results();

    qDebug() << "export_results: " << export_results;
}

//-----------------------------------------------------------------------------
//--- report()
//-----------------------------------------------------------------------------
void Analysis::report_Param()
{   
    if(!prot) return;

    //QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    //label_gif->setVisible(true);
    //obj_gif->start();

    Report_Param = new Report_Parameterization(prot, user_Dir.absolutePath(), this);
    //Report_Param->exec();
    delete Report_Param;

    //label_gif->setVisible(false);
    //obj_gif->stop();
    //QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
//--- report()
//-----------------------------------------------------------------------------
void Analysis::report()
{
    int type_analysis;
    QString text;
    char *p_ch;
    QByteArray ba;
    QWidget *w_obj = GridTab->currentWidget();
    bool modify = false;

    bool *p_mod = &modify;


    QString fn = fn_OUT;    //qApp->applicationDirPath() + "/data/out.rt";
    QFile file(fn);

    // 1. Dna_Technology Analysis
    if(w_obj == GridBox_ext_all)
    {
        /*text = QString("DTReport status:  report_XMLfile:%1  show_report:%2  close_report:%3").arg((int)report_XMLfile)
                                                                                              .arg((int)show_report)
                                                                                              .arg((int)close_report);
        message.setIcon(QMessageBox::Information);
        message.button(QMessageBox::Ok)->animateClick(15000);
        message.setText(text);
        message.exec();*/

        if(report_XMLfile && show_report && close_report) // && !text.isEmpty())
        {
            //report_XMLfile(fn.toLatin1().data(), text.toLatin1().data(), false, Wrapper_To_ReportStatus, this, &p_ch);            
            report_XMLfile(fn.toUtf8().data(), NULL, false, Wrapper_To_ReportStatus, this, &p_ch);

            ba = QByteArray(p_ch);
            text = "";
            text = QString(ba);
            if(!text.isEmpty())
            {
                message.setIcon(QMessageBox::Critical);
                message.button(QMessageBox::Ok)->animateClick(15000);
                message.setText(text);
                message.exec();
            }

            Display_ProgressBar(0, "");            
            show_report(NULL, true, p_mod, &p_ch);

            qDebug() << "modify: " << *p_mod << modify;
            if(modify) flag_SaveChangesProtocol = true;

            ba.clear();
            ba = QByteArray(p_ch);
            text = "";
            text = QString(ba);
            if(!text.isEmpty())
            {
                message.setIcon(QMessageBox::Critical);
                message.button(QMessageBox::Ok)->animateClick(15000);
                message.setText(text);
                message.exec();
            }
        }

        /*
        w_obj = Tab_Analyser->currentWidget();
        type_analysis = Map_TabAnalyser.key(w_obj);

        if(type_analysis >= 0x20000)    // DTR Report
        {
            text = Map_EXTanalyserNAME.value(type_analysis,"");
            qDebug() << "report: " << text;

        }
        else                            //
        {

        }*/

    }

    // 2. All_Cp Analysis
    if(w_obj == GridBox_ext_globe)
    {
        Report_All = new Report(prot, user_Dir.absolutePath(), "", this);
        delete Report_All;
    }


}

//-----------------------------------------------------------------------------
//--- Action view_listProtocols()
//-----------------------------------------------------------------------------
void Analysis::view_listProtocols()
{
    int i;
    QMenu menu;
    QString text;
    QAction *act = NULL;
    QFileInfo fi;

    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");

    menu.setFont(qApp->font());

    ApplSettings->beginGroup("LastProtocols");
    QStringList list = ApplSettings->childKeys();

    for(i=0; i<list.count(); i++)
    {
        text = ApplSettings->value(list.at(i),"").toString();
        if(text.isEmpty()) continue;
        // check of file exists...
        fi.setFile(text);
        if(!fi.exists()) continue;
        //...
        act = new QAction(text, this);
        menu.addAction(act);
    }
    text = "";
    if(menu.actions().size())
    {
        //qDebug() << "menu_Font:" <<  menu.font();
        act = menu.exec(QCursor::pos());
        if(act) text = act->text();
    }
    ApplSettings->endGroup();

    if(act)
    {
        QFile file(text);
        if(file.exists())
        {
            Save_ChangesProtocol();

            original_FileName = true;
            open_Protocol(text);
        }
        else
        {
            text = tr("FILE (") + text + tr(")  IS ABSENT!");
            QMessageBox::warning(this, tr("Attention"), text);
        }
    }

    menu.clear();

}
//-----------------------------------------------------------------------------
//--- Action load_to_setup()
//-----------------------------------------------------------------------------
void Analysis::load_to_setup()
{
    QString fn = fn_OUT;

    if(!prot || !load_ToSetup->isEnabled()) return;

    parentWidget()->setProperty("analysis_to_setup", QVariant(fn));
    QEvent *e = new QEvent((QEvent::Type)3005);
    QApplication::sendEvent(this->parentWidget(), e);
}

//-----------------------------------------------------------------------------
//--- Action reopen()
//-----------------------------------------------------------------------------
void Analysis::reopen(QString new_plate)
{
    if(!prot) return;

    int i;

    //qDebug() << "reopen prot->xml_filename: " << QString::fromStdString(prot->xml_filename);

    QFile file(QString::fromStdString(prot->xml_filename));
    if(!file.exists()) return;

    QDomDocument    doc;
    QDomElement     root;
    QDomElement     tests;
    QDomElement     sourceSet;
    QDomElement     name_pro;
    QDomElement     operator_pro;
    QDomElement     id_pro;
    QDomNode        xmlNode_instruction;
    bool            ok;
    bool            status = false;
    QVector<int>    vec_RemovedCh;

    QString selectedFilter;
    QString fn_out = qApp->applicationDirPath() + "/data/reopen.rt";
    QFile file_out(fn_out);
    QDomDocument    doc_plate;
    QDomElement     root_plate;
    QDomElement     tests_plate;
    QDomElement     sourceSet_plate;
    QDomElement     name_pro_plate;
    QDomElement     operator_pro_plate;
    QDomElement     id_pro_plate;
    QDomElement     x_size, y_size;
    QDomElement     active_ch;
    QDomElement     temp_element;
    int             act_ch = 0;
    int             x_count, y_count, tubes_count = 0;

    QString dirName = user_Dir.absolutePath();

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return;
        }
        root = doc.documentElement();
        name_pro = root.firstChildElement("name");
        id_pro = root.firstChildElement("regNumber");
        operator_pro = root.firstChildElement("nameOperator");
        tests = root.firstChildElement("tests");
        sourceSet = root.firstChildElement("sourceSet");

        // delete DTReport and DTReportHash
        temp_element = root.firstChildElement("DTReportHash");
        if(!temp_element.isNull()) root.removeChild(temp_element);
        temp_element = root.firstChildElement("DTReport");
        if(!temp_element.isNull()) root.removeChild(temp_element);


       //root.replaceChild(tests,sourceSet);
        if(new_plate.isEmpty())
        {
            new_plate = QFileDialog::getOpenFileName(this,
                                                    tr("Reopen Protocol"),
                                                    dirName,
                                                    tr("Protocols File (*.rt)"),
                                                    &selectedFilter);
        }
        QFile file_plate(new_plate);
        if(file_plate.exists() && file_plate.open(QIODevice::ReadOnly))
        {
            if(doc_plate.setContent(&file_plate))
            {
                root_plate = doc_plate.documentElement();
                tests_plate = root_plate.firstChildElement("tests");
                sourceSet_plate = root_plate.firstChildElement("sourceSet");
                name_pro_plate = root_plate.firstChildElement("name");
                operator_pro_plate = root_plate.firstChildElement("nameOperator");
                id_pro_plate = root_plate.firstChildElement("regNumber");
                x_size = root_plate.firstChildElement("xsize");
                y_size = root_plate.firstChildElement("ysize");
                if(!x_size.isNull() && !y_size.isNull())
                {
                    x_count = x_size.text().toInt(&ok);
                    y_count = y_size.text().toInt(&ok);
                    tubes_count = x_count * y_count;
                }
                if(tubes_count != prot->count_Tubes)
                {
                    file_plate.close();
                    file.close();
                    QMessageBox::warning(this, tr("ReOpen Protocol"), tr("InValid the size of the plate..."));
                    return;
                }

                // check on active_channels
                active_ch = root_plate.firstChildElement("activeChannels");
                if(!active_ch.isNull())
                {
                    act_ch = active_ch.text().toInt(&ok, 16);
                }


                vec_RemovedCh.clear();
                for(i=0; i<COUNT_CH; i++)
                {
                    if((act_ch & (0x0f<<4*i)) && !(prot->active_Channels & (0x0f<<4*i)))
                    {
                        vec_RemovedCh.append(i);
                    }
                }
                //qDebug() << "Active_ch: " << act_ch << prot->active_Channels << vec_RemovedCh;

                if(vec_RemovedCh.size()) RemoveChannels_InSourceset(&sourceSet_plate, &vec_RemovedCh);

                /*for(i=0; i<COUNT_CH; i++)
                {
                    if((act_ch & (0x0f<<4*i)) && !(prot->active_Channels & (0x0f<<4*i)))
                    {
                        file_plate.close();
                        file.close();
                        QMessageBox::warning(this, tr("ReOpen Protocol"), tr("InValid the optical mask..."));
                        return;
                    }
                }*/


                // Replace
                root.replaceChild(tests_plate, tests);
                root.replaceChild(sourceSet_plate, sourceSet);
                root.replaceChild(name_pro_plate, name_pro);
                root.replaceChild(operator_pro_plate, operator_pro);
                root.replaceChild(id_pro_plate, id_pro);

                //id_pro = root.firstChildElement("regNumber");
                //qDebug() << "id pro: " << id_pro.text();

                //xmlNode_instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
                //doc.insertBefore(xmlNode_instruction, doc.firstChild());

                if(file_out.open(QIODevice::WriteOnly))
                {
                    QTextStream(&file_out) << doc.toString();
                    file_out.close();
                    status = true;

                    //qDebug() << "reopen: " << QString::fromStdString(prot->xml_filename) << file_out.fileName();
                }
            }
            file_plate.close();
        }
        file.close();

        if(status)
        {
            open_Protocol(file_out.fileName());
            flag_SaveChangesProtocol = true;
        }
    }
}


//-----------------------------------------------------------------------------
//--- RemoveChannels_InSourceset
//-----------------------------------------------------------------------------
void Analysis::RemoveChannels_InSourceset(QDomElement *sourceset, QVector<int> *vec_removed)
{
    int i,j,k,m,n,p,q;
    QString text;
    int id;
    bool ok;

    QDomNode    item_group;
    QDomNode    item_Samples;
    QDomNode    item_Tubes;
    QDomNode    item_Channels;
    QDomNode    item_sample;
    QDomNode    item_tube;
    QDomNode    item_channel;

    QDomElement id_channel;
    QVector<QDomNode> vec_item;


    for(i=0; i<sourceset->childNodes().size(); i++)
    {
        item_group = sourceset->childNodes().at(i);
        if(item_group.nodeName() != "item") continue;

        for(j=0; j<item_group.childNodes().size(); j++)
        {
            item_Samples = item_group.childNodes().at(j);
            if(item_Samples.nodeName() != "samples") continue;

            for(k=0; k<item_Samples.childNodes().size(); k++)
            {
                item_sample = item_Samples.childNodes().at(k);
                if(item_sample.nodeName() != "item") continue;

                for(m=0; m<item_sample.childNodes().size(); m++)
                {
                    item_Tubes = item_sample.childNodes().at(m);
                    if(item_Tubes.nodeName() != "tubes") continue;

                    for(n=0; n<item_Tubes.childNodes().size(); n++)
                    {
                        item_tube = item_Tubes.childNodes().at(n);
                        if(item_tube.nodeName() != "item") continue;

                        for(p=0; p<item_tube.childNodes().size(); p++)
                        {
                            item_Channels = item_tube.childNodes().at(p);
                            if(item_Channels.nodeName() != "channels") continue;

                            vec_item.clear();
                            for(q=0; q<item_Channels.childNodes().size(); q++)
                            {
                                item_channel = item_Channels.childNodes().at(q);

                                id_channel = item_channel.firstChildElement("IDChannel");
                                if(!id_channel.isNull())
                                {
                                    text = id_channel.text();
                                    id = text.toInt(&ok);

                                    if(ok)
                                    {
                                        if(vec_removed->contains(id)) vec_item.append(item_channel);
                                    }
                                }
                            }
                            // remove channels
                            for(q=0; q<vec_item.size(); q++)
                            {
                                item_Channels.removeChild(vec_item.at(q));
                            }
                        }
                    }
                }
            }
        }
    }

    vec_item.clear();
}

//-----------------------------------------------------------------------------
//--- Action editTest()
//-----------------------------------------------------------------------------
void Analysis::editTest()
{        
    QString fn_temp;

    if(!prot || !editTestAct->isEnabled()) return;

    //...
    EnableTubes_Prev = QVector<short>::fromStdVector(prot->enable_tube);
    //qDebug() << "enable_Tubes: " << EnableTubes_Prev;

    //... Load Test_Editor ...
    QVector<rt_Test*> TESTs = QVector<rt_Test*>::fromStdVector(prot->tests);

    test_editor = new Test_editor(this, true);

    test_editor->From_Protocol = true;
    test_editor->TESTs = &TESTs;
    test_editor->ax_user = ax_user;
    test_editor->axgp = axgp;
    test_editor->User_Folder = user_Dir.absolutePath();
    test_editor->prot = prot;
    //test_editor->Translate_Catalog();
    test_editor->map_TestTranslate = map_TestTranslate;

    if(ext_dll_handle)
    {
        test_editor->ext_dll_handle = ext_dll_handle;
        test_editor->LoadTest_ExtPlugins();
    }    
    int res = test_editor->exec();
    delete test_editor;    

    if(res)
    {
        qApp->processEvents();

        replaceTests(fn_temp);
        open_Protocol(fn_temp);
        flag_SaveChangesProtocol = true;

        if(EnableTubes_Prev.size())
        {
            sel->Previous_EnableTubes(&EnableTubes_Prev);
        }
    }
    EnableTubes_Prev.clear();

    /*if(res && Map_analyser.size())
    {
        foreach(analyser, Map_analyser)
        {
            analyser->Analyser(prot);
        }
    }*/
}
//-----------------------------------------------------------------------------
//--- Action preview_archive()
//-----------------------------------------------------------------------------
void Analysis::preview_archive()
{
    Archive = new Video_Archive();    
    Archive->exec();

    delete Archive;
}

//-----------------------------------------------------------------------------
//--- Action email_Send()
//-----------------------------------------------------------------------------
void Analysis::email_Send()
{
    QString address = "";

    if(ax_user)
    {
        address = ax_user->dynamicCall("getAttr(QString)","str:PR_EMAIL").toString();
        email->Email_address = address.trimmed();
    }

    if(email->isHidden()) email->show();
}
//-----------------------------------------------------------------------------
//--- Action rdml_export()
//-----------------------------------------------------------------------------
void Analysis::rdml_import()
{
    QString fileName;
    QString selectedFilter;
    QString dirName = user_Dir.absolutePath();
    bool start_rdml = false;

    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open RDML Protocol"),
                                            dirName,
                                            tr("RDML Protocols File (*.xml)"),
                                            &selectedFilter);

    std::string str = fileName.toStdString();


    QFileInfo file_Info(fileName);    

    if(file_Info.exists())
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        label_gif->setVisible(true);
        obj_gif->start();

        Display_ProgressBar(50, tr("Converting RDML format..."));

        qApp->processEvents();

        if(dll_rdml)
        {
            RDML_IMPORT rdml_to_rt = reinterpret_cast<RDML_IMPORT>(
                           ::GetProcAddress(dll_rdml,"RDML_2_RT@4"));

            if(rdml_to_rt)
            {
                start_rdml = rdml_to_rt((char*)(str.c_str()));

                if(!start_rdml)
                {
                    QMessageBox::warning(this, tr("RDML import"), tr("Attention! Invalid RDML file format..."));
                }
            }

        }

        Display_ProgressBar(0, tr(""));
        label_gif->setVisible(false);
        obj_gif->stop();
        QApplication::restoreOverrideCursor();

        if(start_rdml) open_Protocol("rdml/rdml.rt");
    }
}
//-----------------------------------------------------------------------------
//--- Action editPreferencePro()
//-----------------------------------------------------------------------------
void Analysis::editPreferencePro()
{
    int res;
    QString fn_temp;

    if(!prot || !edit_PreferenceProtocol->isEnabled()) return;

    Pro_editor = new Common_Pro(this, prot);
    res = Pro_editor->exec();
    delete Pro_editor;

    if(res)
    {
        qApp->processEvents();

        replaceProperties(fn_temp);        
        open_Protocol(fn_temp);
        flag_SaveChangesProtocol = true;
    }
}

//-----------------------------------------------------------------------------
//--- replaceProperties
//-----------------------------------------------------------------------------
void Analysis::replaceProperties(QString &fn)
{
    int i,j;
    rt_Preference *preference;
    QString text;

    fn = "";
    if(!prot) return;

    QFile file(QString::fromStdString(prot->xml_filename));
    if(!file.exists()) return;

    QDomDocument    doc;
    QDomElement     root;
    QDomElement     property;
    QDomElement     property_new;
    QDomElement     tests;
    QDomElement     temp_element;

    QString fn_out = fn_OUTTMP; //qApp->applicationDirPath() + "/data/out_temp.rt";
    QFile file_out(fn_out);

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return;
        }
        file.close();

        root = doc.documentElement();
        tests = root.firstChildElement("tests");
        property = root.firstChildElement("properties");
        property_new = doc.createElement("properties");

        if(property.isNull()) root.insertBefore(property_new, tests);
        else root.replaceChild(property_new, property);

        for(i=0; i<prot->preference_Pro.size(); i++)
        {
            preference = prot->preference_Pro.at(i);
            QDomElement item_property = doc.createElement("item");
            item_property.appendChild(MakeElement(doc,"name",QString::fromStdString(preference->name)));
            item_property.appendChild(MakeElement(doc,"value",QString::fromStdString(preference->value)));
            item_property.appendChild(MakeElement(doc,"unit",QString::fromStdString(preference->unit)));
            property_new.appendChild(item_property);
        }

        // delete DTReport and DTReportHash
        temp_element = root.firstChildElement("DTReportHash");
        if(!temp_element.isNull()) root.removeChild(temp_element);
        temp_element = root.firstChildElement("DTReport");
        if(!temp_element.isNull()) root.removeChild(temp_element);


        if(file_out.open(QIODevice::WriteOnly))
        {
            QTextStream(&file_out) << doc.toString();
            file_out.close();
            fn = fn_out;
        }        
    }
}

//-----------------------------------------------------------------------------
//--- replaceTests
//-----------------------------------------------------------------------------
void Analysis::replaceTests(QString &fn)
{
    int i,j;
    rt_Test *ptest;
    rt_Preference *prop;
    QString text;

    fn = "";
    if(!prot) return;

    QFile file(QString::fromStdString(prot->xml_filename));
    if(!file.exists()) return;

    QDomDocument    doc;
    QDomElement     root;
    QDomElement     tests;
    QDomElement     tests_new;
    QDomElement     temp_element;

    QString fn_out = fn_OUTTMP; //qApp->applicationDirPath() + "/data/out_temp.rt";
    QFile file_out(fn_out);

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return;
        }
        file.close();

        root = doc.documentElement();        
        tests = root.firstChildElement("tests");
        tests_new = doc.createElement("tests");
        root.replaceChild(tests_new, tests);

        for(i=0; i<prot->tests.size(); i++)
        {
            ptest = prot->tests.at(i);
            tests_new.appendChild(SaveXML_Test(doc,ptest));
        }

        // delete DTReport and DTReportHash
        temp_element = root.firstChildElement("DTReportHash");
        if(!temp_element.isNull()) root.removeChild(temp_element);
        temp_element = root.firstChildElement("DTReport");
        if(!temp_element.isNull()) root.removeChild(temp_element);


        if(file_out.open(QIODevice::WriteOnly))
        {
            QTextStream(&file_out) << doc.toString();
            file_out.close();
            fn = fn_out;
        }        
    }
}

//-----------------------------------------------------------------------------
//--- create StatusBar
//-----------------------------------------------------------------------------
void Analysis::createStatusBar()
{
    QString text;

    main_progress = new QProgressBar(this);

    if(StyleApp != "fusion")
    {
        main_progress->setStyleSheet("QProgressBar:horizontal {border: 1px solid gray; border-radius: 3px; background: #E8E8E8; padding: 2px;}"
                "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #C4FFC4, stop: 1 lime);}");
    }
    main_progress->setFixedWidth(600);

    FileName_Protocol = new QLabel(this);
    FileName_Protocol->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    DirLabel_Protocol = new QLabel(this);
    DirLabel_Protocol->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    DirLabel_Protocol->setPixmap(QPixmap(":/images/flat/open_from.png"));
    DirLabel_Protocol->setVisible(false);
    original_FileName = false;


    //QLabel *version_label = new QLabel(this);
    //version_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    //text = VERS;
    //version_label->setText(""); //("ver. " + text);
    statusBar()->addWidget(main_progress);
    statusBar()->addWidget(DirLabel_Protocol,1);
    statusBar()->addWidget(FileName_Protocol);
    statusBar()->setFont(qApp->font());

    //statusBar()->addWidget(version_label,1);



    main_progress->setTextVisible(false);
    main_progress->setRange(0,100);
    PrBar_status = new QLabel();
    PrBar_status->setAlignment(Qt::AlignCenter);

    QHBoxLayout *probar_layout = new QHBoxLayout();

    main_progress->setLayout(probar_layout);
    probar_layout->addWidget(PrBar_status);


    /*QFont font = app_font;
    font.setBold(false);
    font.setPointSizeF(8);
    statusBar()->setFont(font);*/

    statusBar()->setSizeGripEnabled(false);

}
//-----------------------------------------------------------------------------
//--- create ToolBar
//-----------------------------------------------------------------------------
void Analysis::createToolBars()
{    
    QPalette palette;
    QFont f = qApp->font();

    //...fileToolBar
    fileToolBar = addToolBar(tr("File"));    
    setContextMenuPolicy(Qt::NoContextMenu);

    Label_Protocol = new QLabel(tr("Protocol: "), this);
    Name_Protocol = new QLineEdit(this);
    Name_Protocol->setReadOnly(true);    
    Name_Protocol->setFixedWidth(250);
    Name_Protocol->setAlignment(Qt::AlignHCenter);
    Name_Protocol->setFocusPolicy(Qt::NoFocus);
    //Name_Protocol->addAction(QIcon(":/images/flat/star.png"), QLineEdit::TrailingPosition);
    Name_Protocol->addAction(Validity_Hash, QLineEdit::TrailingPosition);
    f.setBold(true);
    Name_Protocol->setFont(f);

    fileToolBar->addWidget(Label_Protocol);
    fileToolBar->addWidget(Name_Protocol);

    QWidget *spacer = new QWidget(this);
    spacer->setFixedWidth(25);
    fileToolBar->addWidget(spacer);

    fileToolBar->addAction(openXmlAct);
    fileToolBar->addAction(open_LastProtocols);    

    QWidget *p = fileToolBar->widgetForAction(open_LastProtocols);
    p->setMaximumWidth(13);

    fileToolBar->addSeparator();
    fileToolBar->addAction(saveAct);
    fileToolBar->addSeparator();
    fileToolBar->addSeparator();
    fileToolBar->addAction(infoAct);
    fileToolBar->addAction(selectAct);
    fileToolBar->addSeparator();

    QWidget *spacer_0 = new QWidget(this);
    spacer_0->setFixedWidth(25);
    fileToolBar->addWidget(spacer_0);
    fileToolBar->addAction(reportParamAct);
    fileToolBar->addAction(reportAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(exportResults);

    fileToolBar->addSeparator();
    QWidget *spacer_2 = new QWidget(this);
    spacer_2->setFixedWidth(25);
    fileToolBar->addWidget(spacer_2);
    fileToolBar->addAction(EMail_send);
    //fileToolBar->addAction(RDML_import);

    QWidget *spacer_1 = new QWidget(this);
    spacer_1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileToolBar->addWidget(spacer_1);

    fileToolBar->addAction(load_ToSetup);    
    fileToolBar->addAction(reopenXmlAct);
    fileToolBar->addAction(editTestAct);
    fileToolBar->addAction(edit_PreferenceProtocol);
    fileToolBar->addSeparator();


/*    //... viewToolBar
    viewToolBar = addToolBar((tr("View")));
    viewToolBar->addAction(selectAct);
    viewToolBar->addSeparator();
    viewToolBar->addAction(infoAct);
    viewToolBar->addAction(reportAct);
    viewToolBar->addAction(excelAct);    
*/
}
//-----------------------------------------------------------------------------
//--- Read Settings
//-----------------------------------------------------------------------------
void Analysis::readSettings()
{
    int i;
    int num;
    QMainWindow *p_mw;
    QSize size;
    QPoint pos;
    int first_pos;
    QAbstractButton *a_button;
    Color_ToolButton *button;
    QColor color;
    unsigned int val;
    bool ok;

    //... MainWindow ..........................................................
    ApplSettings->beginGroup("MainWindow");

    int width = (main_spl->size().width() - 4)/2;
    first_pos = width;
    QList<int> list_spl;
    QString text = ApplSettings->value("splitter","").toString();
    if(text.isEmpty()) list_spl << width << width;
    else
    {
        QStringList list_str = text.split(" ");
        for(i=0; i<list_str.count(); i++) list_spl.append(list_str.at(i).toInt());
        first_pos = list_str.at(0).toInt();
    }

    main_spl->setSizes(list_spl);
    main_spl->splitterMoved(first_pos,1);
    ApplSettings->endGroup();
    //.........................................................................

    //... Select Panel ........................................................

    ApplSettings->beginGroup("SelectPanel");
    if(!ApplSettings->value("Hidden", true).toBool())
    {
        if(ApplSettings->value("Floating", false).toBool())
        {
            //sel->show();
            sel->hide();
            sel->setAllowedAreas(Qt::NoDockWidgetArea);
            sel->setFloating(true);
            size = ApplSettings->value("size",QSize(300, 200)).toSize();
            sel->resize(size);
            pos = ApplSettings->value("pos", QPoint(1,1)).toPoint();
            sel->move(pos);
        }
        else
        {
            num = ApplSettings->value("dock_MainWin",1).toInt();
            sel->mTitleBar->type_MainWin = (MainWinDock)num;
            switch(num)
            {
            case 1: p_mw = ChartWindow; break;
            case 2: p_mw = GridWindow; break;
            default: p_mw = NULL;   break;
            }

            if(p_mw)
            {
                //sel->show();
                sel->setFloating(false);
                num = ApplSettings->value("dock_area",8).toInt();
                sel->dock_area = (Qt::DockWidgetArea)(num);
                p_mw->addDockWidget(sel->dock_area, sel);



                size = ApplSettings->value("size",QSize(300, 200)).toSize();
                //setDockSize(sel->mTitleBar, size.width(), size.height());
                sel->resize(size.width(), size.height());
                //sel->setMinimumHeight(size.height());
                //sel->setMaximumHeight(size.height());
            }
         }

    }
    ApplSettings->endGroup();

    //.........................................................................

    //... Server ..............................................................
    ApplSettings->beginGroup("Server");
        text = ApplSettings->value("address","").toString();
        if(!text.isEmpty())
        {
           net_adress = text;
           //qDebug() << net_adress;
        }
    ApplSettings->endGroup();

    //... ColorRejime ................................................................
    ApplSettings->beginGroup("ColorRejime");
    i = 0;
    foreach(a_button, sel->Fluor_Buttons->buttons())
    {
        button = (Color_ToolButton*)a_button;
        text = ApplSettings->value(QString("color_%1").arg(i), "").toString();
        i++;
        if(text.isEmpty()) continue;
        val = text.toInt(&ok,16);
        if(!ok) val = 0x0;
        color = val;
        if(color.isValid()) button->color = color;
    }
    emit sel->Fluor_Buttons->buttonClicked(0);
    ApplSettings->endGroup();

    //... FirstPCR_TESTs ......................................................................
    ApplSettings->beginGroup("FirstPCR_TESTs");
    text = ApplSettings->value("tests","").toString();
    FirstPCR_TESTs = text.split(QRegExp("\\s+"));
    ApplSettings->endGroup();

}

//-----------------------------------------------------------------------------
//--- Write Settings
//-----------------------------------------------------------------------------
void Analysis::writeSettings()
{
    QString text = "";
    int i;
    QAbstractButton *a_button;
    Color_ToolButton *button;
    unsigned int val;

    if(!flag_SaveLocationWin) return;

    //--- MainWindow ---
    ApplSettings->beginGroup("MainWindow");

    QList<int> list_spl = main_spl->sizes();
    for(int i=0; i<list_spl.count(); i++) text += QString::number(list_spl.at(i)) + " ";
    ApplSettings->setValue("splitter", text);

    ApplSettings->endGroup();


    //--- Select Panel ---
    ApplSettings->beginGroup("SelectPanel");
    ApplSettings->setValue("Hidden", sel->isHidden());
    if(!sel->isHidden())
    {
        ApplSettings->setValue("Floating", sel->isFloating());
        if(sel->isFloating())
        {
            ApplSettings->setValue("size", sel->size());
            ApplSettings->setValue("pos", sel->pos());
        }
        else
        {
            ApplSettings->setValue("dock_MainWin",sel->mTitleBar->type_MainWin);
            ApplSettings->setValue("dock_area",sel->dock_area);
            ApplSettings->setValue("size", sel->size());
        }
    }
    ApplSettings->endGroup();

    //--- ColorRejime ---
    ApplSettings->beginGroup("ColorRejime");
    i=0;
    foreach(a_button, sel->Fluor_Buttons->buttons())
    {
        button = (Color_ToolButton*)a_button;
        val = button->color.rgb() & 0xffffff;
        text = QString::number(val,16);
        ApplSettings->setValue(QString("color_%1").arg(i), text);
        i++;
    }
    ApplSettings->endGroup();

 }


//-----------------------------------------------------------------------------
//--- Slot open_Protocol()
//-----------------------------------------------------------------------------
void Analysis::open_Protocol(QString fileName, bool send, bool create_Hash)
{
    int i,j,k,id;
    int sts;
    QDir ApplDir(qApp->applicationDirPath());
    QDir dir_Device;
    QString selectedFilter;
    QString suffix;
    QStringList list_suffix;
    QString text, str;
    bool res;
    bool ok;
    int type = 0;
    char *p_str;
    char *p_ch;
    QByteArray ba;
    bool sts_ExternalLib = false;
    bool visible;
    rt_Test *p_test;
    QString version;
    std::string string_temp;
    int type_analysis;
    QWidget *widget_obj;
    int method_ID;
    QVector<QString> info_analyser;
    bool negative = false;
    bool melting = false;
    QString validate_str;
    QVector<QString> DTR_Names;

    QString fn = fn_OUT;    //qApp->applicationDirPath() + "/data/out.rt";
    QFile file(fn);    

    QString dirName = user_Dir.absolutePath();


    if(fileName.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this,
                                                tr("Open Protocol"),
                                                dirName,
                                                tr("Protocols File (*.r96 *.r48 *.384 *.rt)"),
                                                &selectedFilter);//,
                                                //options);
        if(!fileName.isEmpty()) original_FileName = true;
    }

    QFileInfo file_Info(fileName);

    //qDebug() << "open protocol = " << fileName;
    //qDebug() << "fn_OUT = " << fn_OUT;

    if(file_Info.exists())
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        label_gif->setVisible(true);
        obj_gif->start();

        Display_ProgressBar(10, tr("Clear Protocol..."));

        qApp->processEvents();

        Attention_Browser->clear();
        if(GridTab->count() > 3) GridTab->removeTab(3);     // remove Attention Tab

        saveAct->setEnabled(false);
        //selectAct->setEnabled(false);
        infoAct->setEnabled(false);
        reportAct->setEnabled(false);
        exportResults->setEnabled(false);
        reportParamAct->setEnabled(false);
        load_ToSetup->setEnabled(false);
        reopenXmlAct->setEnabled(false);
        editTestAct->setEnabled(false);
        edit_PreferenceProtocol->setEnabled(false);
        //EMail_send->setEnabled(false);
        //saveXmlAct->setEnabled(false);

        addProtocol_toList(fileName);

        //--- Clear All Rejimes ---
        Clear_Rejimes(-1);
        //CtAct->setChecked(false);
        //LgAct->setChecked(false);
        //NormalizationAct->setChecked(false);

        //--- Clear Graph ---
        plot->Clear_Graph();
        plot->Delete_Curve();

        //--- 0. Clear Protocol ---
        prot->Clear_Protocol();

        sel->clear_SelectGrid();
        sel->load_SelectGrid(NULL);


        // ... All_Cp plugin ...
        if(alg)
        {            
            alg->Destroy_Win();
            alg->Destroy();
            alg = NULL;            
        }
        if(dll_handle)
        {
            ::FreeLibrary(dll_handle);
            dll_handle = NULL;
        }

        // ... DTR modules ...
        if(ext_dll_handle)
        {
            if(close_results) close_results();
            if(close_report) close_report();
            ::FreeLibrary(ext_dll_handle);
            ext_dll_handle = NULL;
            show_results = NULL;
            close_results = NULL;
            enable_tubes = NULL;
            select_curve = NULL;
            get_color = NULL;
            resize_results = NULL;
            report_XMLfile = NULL;
            show_report = NULL;
            close_report = NULL;
            export_results = NULL;
            ext_dll_handle_VALID = false;
        }

        // ... My plugins ...
        // ... Clear List Analyser ...
        foreach(analyser, Map_analyser)
        {
            analyser->Destroy_Win();
            analyser->Destroy();
        }
        analyser = NULL;
        //Map_analyserNAME.clear();
        Map_analyser.clear();


        foreach(analyser_handle, List_analyserHandle)
        {
            if(analyser_handle) ::FreeLibrary(analyser_handle);
        }
        List_analyserHandle.clear();



        // ... Clear Tabs analysers ...
        foreach(widget_obj, Map_TabAnalyser)
        {
            if(widget_obj) delete widget_obj;
        }        
        Map_TabAnalyser.clear();        
        Tab_AnalyserGlobe->clear();

        GridTab->setTabEnabled(0,false);
        GridTab->setTabEnabled(1,false);
        GridTab->setTabEnabled(2,false);        

        if(StyleApp == "fusion")
        {
            GridTab->setTabIcon(1, QIcon(":/images/flat/global_disable_flat.png"));
            GridTab->setTabIcon(2, QIcon(":/images/flat/dna_logo_disable_flat.png"));
            GridTab->setTabIcon(3, QIcon(":/images/flat/calibration_disable_1.png"));
        }
        //............................

        suffix = file_Info.suffix();
        list_suffix << "r96" << "384" << "r48" << "192";
        Validity_Hash->setIcon(QIcon(":/images/flat/star_null.png"));

        GridTab->update();

        qApp->processEvents();
        qDebug() << "clear:";

        //--- 1. Read ---        
        if(list_suffix.indexOf(suffix) < 0)
        {
              sts = Read_XML(this, Analysis::Wrapper_To_Display_ProgressBar, prot, fileName, tr("Read XML protocol..."));
        }
        else  sts = Read_r96(this, Analysis::Wrapper_To_Display_ProgressBar, prot, fileName, tr("Read r96 protocol..."));

        qDebug() << "fileName: " << fileName << sts << prot->count_PCR << prot->count_MC << prot->meas.size();

        if(!Validate_OpenProtocol(prot, validate_str))
        {
            label_gif->setVisible(false);
            obj_gif->stop();
            QApplication::restoreOverrideCursor();

            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(10000);
            message.setIcon(QMessageBox::Warning);
            message.setText(validate_str);
            message.exec();

            prot->Clear_Protocol();
            Display_ProgressBar(0, "");
            return;
        }


        if(prot->meas.size() == 0)
        {
            label_gif->setVisible(false);
            obj_gif->stop();
            QApplication::restoreOverrideCursor();
            text = tr("There are no optical measurements in the protocol!") + "\r\n" +
                   tr("You can open it in the Protocol section...");            
            QMessageBox::warning(this,tr("Open Protocol"),text);

            prot->Clear_Protocol();
            Display_ProgressBar(0, "");
            return;
        }

        if(sts < 0)
        {
            label_gif->setVisible(false);
            obj_gif->stop();
            QApplication::restoreOverrideCursor();
            QMessageBox::warning(this,tr("Open Protocol"),tr("Unable open protocol..."));

            prot->Clear_Protocol();
            return;
        }

        //--- 1.0 Validate Hash ---
        //Display_ProgressBar(50, tr("Validate Protocol, Tests..."));
        //QApplication::processEvents();

        if(prot->validity_hash | create_Hash) Validity_Hash->setIcon(QIcon(":/images/flat/star.png"));

        //--- 1.1 Validate Tests ---
        foreach(p_test, prot->tests)
        {
            Validate_Test(p_test);
        }

        //--- 1.11 Translate Tests&Research ---  load Map_TestsTranslate
        if(ID_LANG != ID_LANG_RU)
        {
            QStringList list_tr;
            foreach(p_test, prot->tests)
            {
                if(p_test->header.Type_analysis > 0x20000)
                {
                    Create_TranslateList(p_test, list_tr);
                }
            }
            if(list_tr.size()) Translate_Tests(list_tr);        // create Map_TestsTranslate
            list_tr.clear();
        }

        //--- 1.2 Load Exposure ---
        //Display_ProgressBar(80, tr("Validate Protocol, Tests..."));
        //QApplication::processEvents();
        k = 0;
        for(i=0; i<COUNT_CH; i++)
        {
            id = k*2;
            if(!(prot->active_Channels & (0x0f<<4*i)) || prot->meas.size() <= id)
            {
                prot->Exposure.push_back((short)0);
            }
            else
            {
                rt_Measurement *meas =  prot->meas.at(id);
                prot->Exposure.push_back(meas->exp_value);
                k++;
            }
        }

        Name_Protocol->setText(QString::fromStdString(prot->name));
        //prot->xml_filename = file_Info.fileName().toStdString();
        prot->xml_filename = file_Info.absoluteFilePath().toStdString();
        if(original_FileName ||(!original_FileName && FileName_Protocol->text().trimmed().isEmpty()))
        {
            FileName_Protocol->setText(file_Info.absoluteFilePath()+ "   ");
            original_FileName = false;
        }
        DirLabel_Protocol->setVisible(true);

        //--- 1.3 Check on negative: -dF/dT ---
        foreach(p_test, prot->tests)
        {
            if(p_test->header.Type_analysis == 0x0010)
            {
                dFdT_negative->blockSignals(true);
                dFdT_negative->setChecked(true);
                dFdT_negative->blockSignals(false);
                negative = true;
                melting = true;
                break;
            }
        }

        //--- 2. Transpose RawData ---
        sts = Transpose_RawData(this, Analysis::Wrapper_To_Display_ProgressBar, prot, tr("Transpose_RawData"));
        //qDebug() << "Transpose RawData: " << sts;
        //qDebug() << "Raw: " << QVector<double>::fromStdVector(prot->PCR_RawData).mid(0, prot->count_PCR);
        if(sts < 0)
        {
            label_gif->setVisible(false);
            obj_gif->stop();
            QApplication::restoreOverrideCursor();
            QMessageBox::warning(this,tr("Open Protocol"),tr("Unable open protocol..."));
            return;
        }

        sts = CrossTalk_Correction(prot);        
        //qDebug() << "Raw:(crosstalk) " << sts;//<< QVector<double>::fromStdVector(prot->PCR_RawData).mid(0, prot->count_PCR);

        //--- 3. Transpose Filtered RawDate ---
        sts = Filtered_RawData(this, Analysis::Wrapper_To_Display_ProgressBar, prot, tr("Filtered PCR_RawData"));
        //qDebug() << "Filtered: " << QVector<double>::fromStdVector(prot->PCR_Filtered).mid(0, prot->count_PCR);
        //qDebug() << "Transpose Filtered RawDate: " << sts;

        //--- 4. Bace&Fitting Data ---
        sts = Bace_Data(this, Analysis::Wrapper_To_Display_ProgressBar, prot, tr("Bace&Fitting Data"));
        //qDebug() << "Bace&Fitting Data: " << sts;

        //--- 5. dF/dT MC Data ---
        sts = dFdT_MCData(this, Analysis::Wrapper_To_Display_ProgressBar, prot, tr("Filtered & dF/Dt Melting Curve Data"), negative);
        if(prot->count_MC && prot->X_MC.size()) plot->value_MeltingMarker = (prot->X_MC.front() + prot->X_MC.back())/2.;
        else plot->value_MeltingMarker = 0.;
        //qDebug() << "dF/dT MC Data: " << QVector<double>::fromStdVector(prot->MC_TPeaks);


        //plot->Create_Curve(prot);
        //qDebug() << "Create_Curve: ";
        //plot->Draw_Chart(prot,0x03,0,2);


        sel->load_SelectGrid(prot);        
        Info_Pro->refresh_Info(prot);


        //... change protocol status ...
        switch (prot->state)
        {
        default:
        case mData:         prot->state = mAnalysis;
                            break;
        case mAnalysis:
        case mReAnalysis:   prot->state = mReAnalysis;
                            break;
        }
        //...

        //obj_gif->stop();

#ifdef QT_DEBUG
        dll_handle = ::LoadLibraryW(L"plugins\\all_cpd.dll");
#else
        dll_handle = ::LoadLibraryW(L"plugins\\all_cp.dll");        
#endif

        //qDebug() << "1. Parameterization DATA " << prot->active_Channels;

        // 1. Parameterization DATA
        GridTab->setCurrentWidget(GridBox_allcp);

        if(dll_handle)
        {
            Interface_factory factory_func = reinterpret_cast<Interface_factory>(
                    ::GetProcAddress(dll_handle,"create_plugin@0"));

            if(factory_func)
            {
                alg = factory_func();

                if(alg)
                {                    
                    GridTab->setTabEnabled(0,true);

                    void *obj = alg->Create_Win(GridBox_allcp, this, ax_user);

                    QWidget *w_obj = reinterpret_cast<QWidget*>(obj);
                    if(w_obj->isWidgetType())
                    {
                        layout_Grid->addWidget(w_obj);
                    }

                    //qDebug() << "Analyser: ";
                    alg->Analyser(prot,0x00);
                    //qDebug() << "Show: ";
                    alg->Show();

                    void *map = alg->ReferenceForMap();
                    map_TestTranslate = reinterpret_cast<QMap<QString,QString>*>(map);
                    //sel->map_TestTranslate = map_TestTranslate;
                    //qDebug() << "map_Analysis: " << *map_TestTranslate;

                    //qDebug() << "suffix: ";
                    if(suffix != "rt")
                    {
                        res = SaveAsXML(this, Analysis::Wrapper_To_Display_ProgressBar, prot, fn, tr("Save as XML..."));
                        if(res) prot->xml_filename = fn.toStdString();
                    }
                    else res = true;


                    //qDebug() << "res: true " << res << send;
                    if(res)
                    {
                        saveAct->setEnabled(true);
                        //selectAct->setEnabled(true);
                        infoAct->setEnabled(true);
                        EMail_send->setEnabled(true);
                        //reportParamAct->setEnabled(true);                        
                        load_ToSetup->setEnabled(true);

                        //--- user preference: axUser
                        if(flag_ReOpenProtocol)
                        {                            
                            reopenXmlAct->setEnabled(true);
                        }
                        if(flag_CommonSettings)
                        {                            
                            edit_PreferenceProtocol->setEnabled(true);
                        }
                        if(flag_EditTest)
                        {
                            editTestAct->setEnabled(true);
                        }
                    }

                    if(res && send)
                    {
                        current_XML.filename = fn;
                        send_XmlProtocol(&current_XML);
                    }
                }
            }
        }

        // 1.0.1 Chart
        //qDebug() << "2. Chart " << prot->active_Channels;
        //plot->Calculate_NormCoefficients(prot);
        plot->Create_Curve(prot);        
        for(i=0; i<COUNT_CH; i++)
        {
           if(prot->active_Channels & (0x0f<<4*i))
           {               
               plot->Fluor_Box->setCurrentIndex(i);
               break;
           }
        }


        // 1.1 Check for validity markers:

        //qDebug() << "3. Check_Marker ";
        Check_Marker();        

        //qDebug() << "4. Attention_Analysis ";
        Attention_Analysis();               // Attention !!!


        // 2. Find External Analyser
        //qDebug() << "5. Find External Analyser ";
        widget_obj = NULL;
        for(i=0; i<prot->Plate.groups.size(); i++)
        {
            rt_GroupSamples *group = prot->Plate.groups.at(i);
            for(j=0; j<group->samples.size(); j++)
            {
                rt_Sample *sample = group->samples.at(j);
                type_analysis = sample->p_Test->header.Type_analysis;
                if(!Map_TabAnalyser.keys().contains(type_analysis))
                {
                    if(type_analysis < 0x20000) // My Tests
                    {                        
                        if(!Map_analyserNAME.keys().contains(type_analysis)) continue;
                        else text = Map_analyserNAME.value(type_analysis);
                    }
                    else                        // "dtr" Tests
                    {
                        if(!Map_EXTanalyserNAME.keys().contains(type_analysis)) continue;
                        else
                        {
                            text = Map_EXTanalyserNAME.value(type_analysis);
                            if(!DTR_Names.contains(text)) DTR_Names.append(text);
                            sts_ExternalLib = true;
                        }
                        continue;
                    }

                    if(type_analysis < 0x0064) // 100
                    {
                        widget_obj = GridBox_ext_globe;
                        QGroupBox *box = new QGroupBox(widget_obj);
                        box->setObjectName("Transparent");
                        QVBoxLayout *layout_analyser = new QVBoxLayout();
                        box->setLayout(layout_analyser);
                        Map_TabAnalyser.insert(type_analysis, box);
                        Tab_AnalyserGlobe->addTab(box, text);
                        if(type_analysis == 0x0020)
                        {
                            Tab_AnalyserGlobe->setTabIcon(Tab_AnalyserGlobe->count()-1, QIcon(":/images/flat/calibration_1.png"));
                            plot->Fluor_Box->setCurrentIndex(COUNT_CH+1);                            
                        }
                    }
                }
            }
        }

        //qDebug() << "6. My tests ";
        if(Tab_AnalyserGlobe->count())                      // My tests
        {
            GridTab->setCurrentWidget(GridBox_ext_globe);
            Tab_AnalyserGlobe->setCurrentIndex(0);
            GridTab->setTabEnabled(1,true);
            if(StyleApp == "fusion") GridTab->setTabIcon(1, QIcon(":/images/flat/global_flat.png"));
        }
        //qDebug() << "7. DTR tests ";
        if(sts_ExternalLib) //Tab_Analyser->count())        // DTR tests
        {
            GridTab->setCurrentWidget(GridBox_ext_all);
            //Tab_Analyser->setCurrentIndex(0);
            GridTab->setTabEnabled(2,true);            
            if(StyleApp == "fusion") GridTab->setTabIcon(2, QIcon(":/images/flat/dna_logo_flat.png"));

            if(!DTR_Names.isEmpty())  // LIST of DTR moduls + DTReport2.dll
            {
                Load_DTRVersion(prot, &DTR_Names);
            }
        }



        // 3. Load External Analyser (My tests)
        QDir pluginsDir(qApp->applicationDirPath());
        pluginsDir.cd("analyser_plugins");
        QString fn_plugin;        

        foreach(QString fname, pluginsDir.entryList(QDir::Files))
        {
            analyser_handle = NULL;
            analyser = NULL;

            fn_plugin = "analyser_plugins\\" + fname;

            analyser_handle = ::LoadLibraryW(fn_plugin.toStdWString().c_str());
            if(analyser_handle)
            {
                Analysis_factory analyser_func = reinterpret_cast<Analysis_factory>(
                                                ::GetProcAddress(analyser_handle,"createAnalyser_plugin@0"));                
                if(analyser_func)
                {
                    analyser = analyser_func();

                    if(analyser)
                    {
                        info_analyser.clear();
                        analyser->GetInformation(&info_analyser);                        

                        method_ID = info_analyser.at(0).toInt(&ok,16);

                        if(Map_TabAnalyser.keys().contains(method_ID))
                        {
                            if(method_ID == 0x20)
                            {
                                if(!Validate_Calibration())
                                {
                                    analyser->Destroy();
                                    analyser = NULL;
                                    ::FreeLibrary(analyser_handle);
                                    continue;
                                }
                            }

                            QWidget *w = Map_TabAnalyser.value(method_ID);
                            void *obj = analyser->Create_Win(w, this);                            
                            QWidget *w_obj = reinterpret_cast<QWidget*>(obj);
                            if(w_obj->isWidgetType())
                            {
                                w->layout()->setMargin(0);
                                w->layout()->addWidget(w_obj);
                                //qDebug() << "objects: " << w << w_obj;
                            }
                            analyser->Analyser(prot);
                            analyser->Show();

                             //qDebug() << "Analyser: ";

                            Map_analyser.insert(method_ID, analyser);
                            List_analyserHandle.append(analyser_handle);
                        }
                        else
                        {
                            analyser->Destroy();
                            analyser = NULL;
                            ::FreeLibrary(analyser_handle);
                        }
                    }
                }
            }
        }


        if(Map_analyser.size() || alg)      // save result after "analyser_plugins"(my tests) and Parameterization DATA(All_Cp)
        {            
            // 1. save common results...
            SaveResultsXML(this, Analysis::Wrapper_To_Display_ProgressBar, prot, fileName, tr("Save Results XML..."), fn);

            // 2. save results from analysers (my plugins)
            foreach(analyser, Map_analyser)
            {
                //analyser->Save_Results(fn.toLatin1().data());
                analyser->Save_Results(fn.toUtf8().data());
            }
        }


        label_gif->setVisible(false);
        obj_gif->stop();
        QApplication::restoreOverrideCursor();

        // 2. EXT algorithm...

        if(sts_ExternalLib)
        {
            ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");
            if(ext_dll_handle)
            {

                initialize = (Init)(::GetProcAddress(ext_dll_handle,"Initialize"));
                set_font = (SetFont)(::GetProcAddress(ext_dll_handle,"SetFont"));
                set_folder = (SetUserFolder)(::GetProcAddress(ext_dll_handle,"SetUserDefaultFolder"));

                show_results = (ShowResults)(::GetProcAddress(ext_dll_handle,"ShowResults"));
                close_results = (CloseResults)(::GetProcAddress(ext_dll_handle,"CloseResults"));
                resize_results = (ResizeResults)(::GetProcAddress(ext_dll_handle,"ResizeResultsForm"));
                enable_tubes = (ChangeStatusTubes)(::GetProcAddress(ext_dll_handle,"ChangeStatusTubes"));
                select_curve = (SelectCurve)(::GetProcAddress(ext_dll_handle,"SelectTubeResults"));
                get_color = (GetGroupColor)(::GetProcAddress(ext_dll_handle,"GetGroupChannelsColor"));

                report_XMLfile = (Cr_Report)(::GetProcAddress(ext_dll_handle,"PrepareReportsForXMLFile"));
                show_report = (ShowReport)(::GetProcAddress(ext_dll_handle,"ShowPreview"));
                close_report = (CloseReport)(::GetProcAddress(ext_dll_handle,"ClosePreview"));

                export_results = (ExportResultsEx)(::GetProcAddress(ext_dll_handle,"ExportResultsEx"));

                text = qApp->applicationDirPath() + "/forms";
                version = "9.0";

                /*QFile file_before(qApp->applicationDirPath() + "/before.rt");
                QFile file_after(qApp->applicationDirPath() + "/after.rt");
                if(file_before.exists()) file_before.remove();
                if(file_after.exists()) file_after.remove();
                file.copy(file_before.fileName());*/

                if(initialize && show_results && close_results && resize_results)
                {                    
                    widget_obj = GridBox_ext_dna;

                    // 1. Init
                    initialize(NULL, text.toUtf8().data(), version.toLatin1().data(), ID_LANG, NULL,NULL,NULL,Callback_SetProgressBar,NULL,NULL);

                    if(set_font) set_font(qApp->font().family().toLatin1().data(), "", qApp->font().pointSize());
                    if(set_folder) set_folder(user_Dir.absolutePath().toUtf8().data());
                    //qDebug() << "set_folder: " << set_folder << user_Dir.absolutePath().toUtf8().data();

                    // 2. Show
                    show_results((HWND)widget_obj->winId(), fn.toUtf8().data(), NULL, NULL, Callback_SelectTube, &visible, &p_ch);
                    ba = QByteArray(p_ch);
                    text = "";
                    text = QString(ba);

                    //qDebug() << "ba: " << ba << "text: " << text << QString::fromLocal8Bit(ba) << QString::fromUtf8(ba) << QString::fromLatin1(ba);
                    //text = QString(p_ch);
                    if(!text.isEmpty())
                    {
                        message.setIcon(QMessageBox::Critical);
                        message.setStandardButtons(QMessageBox::Ok);
                        message.button(QMessageBox::Ok)->animateClick(15000);
                        message.setText(text);
                        message.exec();
                    }
                    else ext_dll_handle_VALID = true;

                    // 3. Resize
                    resize_results((HWND)widget_obj->winId());

                    //file.copy(file_after.fileName());

/*
                    foreach(type_analysis, Map_TabAnalyser.keys())
                    {                        
                        if(type_analysis >= 0x20000)
                        {
                            text = Map_EXTanalyserNAME.value(type_analysis);
                            //widget_obj = Map_TabAnalyser.value(type_analysis);
                            //Tab_Analyser->setCurrentWidget(widget_obj);

                            //if(text == "multiplex") continue;

                            qDebug() << "analysis: " << text << type_analysis << widget_obj;


                            // 2. Show
                            //show_results((HWND)widget_obj->winId(), fn.toLatin1().data(), text.toLatin1().data(), NULL, NULL, &visible, &p_ch);
                            show_results((HWND)widget_obj->winId(), fn.toLatin1().data(), NULL, NULL, NULL, &visible, &p_ch);
                            // 3. Resize
                            resize_results((HWND)widget_obj->winId());

                            string_temp.assign(p_ch);
                            text = QString::fromStdString(string_temp);
                            //qDebug() << "p_ch: " << text;

                        }
                    }*/

                    Display_ProgressBar(0, "");
                }
            }
        }

        //qDebug() << "prot_state: (after) " << prot->state;

        if(prot->state == mAnalysis && create_Hash)    // measure -> analysis
        {
            ok = true;
            text = qApp->applicationDirPath() + "/device/";
            dir_Device.setPath(text);
            if(!dir_Device.exists()) ok = dir_Device.mkdir(text);
            if(ok)
            {
                text += QString::fromStdString(prot->SerialName);
                dir_Device.setPath(text);
                if(!dir_Device.exists()) ok = dir_Device.mkdir(text);
                if(ok && file.exists())
                {
                    QFile::remove(dir_Device.path() + "/analysis.rt");
                    file.copy(dir_Device.path() + "/analysis.rt");
                }
            }

            //... send message to Web
            QString message_ToWeb = QString::fromStdString(prot->regNumber) + ";Analyzed";
            parentWidget()->setProperty("ProtocolInfo_ToWeb", QVariant(message_ToWeb));
            QEvent *e = new QEvent((QEvent::Type)3013);
            QApplication::sendEvent(this->parentWidget(), e);
            //...


            ok = true;
            message.setStandardButtons(QMessageBox::Ok);
            //message.button(QMessageBox::Ok)->animateClick(15000);

            text = QString::fromStdString(prot->name);

            // check users filename...
            QFileInfo users_catalog;

            for(i=0; i<prot->preference_Pro.size(); i++)
            {
                rt_Preference *preference = prot->preference_Pro.at(i);
                //qDebug() << "Users_Catalog: " << QString::fromStdString(preference->name) << QString::fromStdString(preference->value);
                if(preference->name == "Users_Catalog")
                {
                    str = QString::fromStdString(preference->value);
                    QFileInfo users_catalog(str);

                    user_Dir.setPath(users_catalog.absoluteDir().absolutePath());
                    text = users_catalog.baseName();
                    break;
                }
            }
            //

            if(!user_Dir.exists()) ok = user_Dir.mkdir(user_Dir.absolutePath());
            if(ok)
            {
                text = Check_ValidNameFile(text);
                text = Unique_FileName(text);
                text = user_Dir.absolutePath() + "/" + text + ".rt";

                text = Original_FileName(text);                

                QFile::remove(text);
                ok = file.copy(text);
                if(create_Hash) ok = Create_HashProtocol(text);

                if(ok)
                {
                    //... send Protocol to Web
                    for(i=0; i<prot->preference_Pro.size(); i++)
                    {
                        rt_Preference *preference = prot->preference_Pro.at(i);
                        if(preference->name == "Web_Protocol" && preference->value == prot->regNumber)
                        {
                            message_ToWeb = QString::fromStdString(prot->regNumber) + ";" + text;
                            parentWidget()->setProperty("ProtocolResult_ToWeb", QVariant(message_ToWeb));
                            QEvent *e = new QEvent((QEvent::Type)3014);
                            QApplication::sendEvent(this->parentWidget(), e);
                            break;
                        }
                    }
                    //...
                    addProtocol_toList(text);
                    str = text;
                    text = tr("The Analysis File is successfully created!") + "\r\n" + text;
                    message.setIcon(QMessageBox::Information);
                    message.button(QMessageBox::Ok)->animateClick(15000);
                    FileName_Protocol->setText(str);
                    DirLabel_Protocol->setVisible(true);

                    prot->xml_filename = str.toStdString();

                    QTreeWidgetItem *item_fn = Info_Pro->info_pro->topLevelItem(8); // FileName of the protocol
                    if(item_fn)
                    {
                        item_fn->setText(1, str);
                    }

                    if(!ArchiveFile_path.trimmed().isEmpty())
                    {
                        QFile file_archive(ArchiveFile_path);
                        if(file_archive.exists()) file_archive.remove();
                    }
                }
                else
                {
                    text = tr("The Analysis File is NOT created!") + "\r\n" + text;
                    message.setIcon(QMessageBox::Warning);                    
                    QPushButton* pb = (QPushButton*)message.button(QMessageBox::Ok);
                    pb->setDefault(true);
                }
            }
            else
            {
                text = tr("InValid User Directory!");
                message.setIcon(QMessageBox::Critical);
                QPushButton* pb = (QPushButton*)message.button(QMessageBox::Ok);
                pb->setDefault(true);
                //message.button(QMessageBox::Ok)->animateClick(60000);
            }
            message.setText(text);
            message.exec();
        }        
    }

    qDebug() << "The END Analysis...";   

    if(sts_ExternalLib) sel->restore_Cursor();
    ArchiveFile_path = "";

    if(prot->count_MC && melting) plot->Type_Box->setCurrentIndex(1);
    foreach(p_test, prot->tests)
    {
        text = QString::fromStdString(p_test->header.Name_Test).trimmed();
        if(FirstPCR_TESTs.contains(text) && prot->count_PCR)
        {
            plot->Type_Box->setCurrentIndex(0);
            break;
        }
    }

    // add Map_TestsTranslate to Protocol
    if(Map_TestTranslate.size())
    {
        QVector<QString> *vec_Tr = new QVector<QString>();
        foreach(QString key_tr, Map_TestTranslate.keys())
        {
            text = QString("%1~%2").arg(key_tr).arg(Map_TestTranslate.value(key_tr));
            vec_Tr->append(text);
        }
        prot->Map_Reserve.insert("Map_TestsTranslate", vec_Tr);

        //qDebug() << "Map_Translate: " << *prot->Map_Reserve.value("Map_TestsTranslate");
    }

    // Catalog
    QVector<rt_Test*> pro_tests;
    pro_tests = QVector<rt_Test*>::fromStdVector(prot->tests);
    Translate_Catalog(&pro_tests);
    pro_tests.clear();
}
//-----------------------------------------------------------------------------
//--- Create_TranslateList(rt_Test *, QStringList &)
//-----------------------------------------------------------------------------
void Analysis::Create_TranslateList(rt_Test *p_test, QStringList &list)
{
    QString text;
    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;

    // 1. name test
    text = QString::fromStdString(p_test->header.Name_Test);
    if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text))
    {
        list.append(text);
        //qDebug() << "name: " << text;
    }

    // 2. name research
    foreach(tube_test, p_test->tubes)
    {
        foreach(channel_test, tube_test->channels)
        {
            text = QString::fromStdString(channel_test->name);
            if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text))
            {
                list.append(text);
                //qDebug() << "name_research: " << text;
            }
        }
    }

    // перевод каталогов делаем сами...
    // 3. name Catalog
    //text = QString::fromStdString(p_test->header.Catalog);
    //if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text)) list.append(text);
}
//-----------------------------------------------------------------------------
//--- Translate_Tests
//-----------------------------------------------------------------------------
void Analysis::Translate_Catalog(QVector<rt_Test *> *tests)
{
    if(ID_LANG == ID_LANG_RU) return;

    QString fn = qApp->applicationDirPath() + "/tools/catalog_translate/catalog_en.xml";
    QFile file(fn);

    int i;
    rt_Test         *ptest;
    QDomDocument    doc;
    QDomElement     root;
    QDomNode        child;
    QDomElement     source;
    QDomElement     target;

    QMap <QString,QString> map_Catalog;

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return;
        }

        root = doc.documentElement();
        if(root.nodeName() == "Catalog")
        {
            for(i=0; i<root.childNodes().size(); i++)
            {
                child = root.childNodes().at(i);
                source = child.firstChildElement("source");
                if(!source.isNull())
                {
                    target = child.firstChildElement("target");
                    if(!target.isNull())
                    {
                        map_Catalog.insert(source.text(), target.text());
                    }
                }
            }
        }

        file.close();
    }

    //qDebug() << "map_Catalog: " << map_Catalog;

    foreach(ptest, *tests)
    {
        //qDebug() << "test: " << QString::fromStdString(ptest->header.Name_Test) << QString::fromStdString(ptest->header.Catalog);

        if(map_Catalog.keys().contains(QString::fromStdString(ptest->header.Catalog)))
        {
            ptest->header.Catalog = map_Catalog.value(QString::fromStdString(ptest->header.Catalog)).toStdString();
            //qDebug() << "test: " << QString::fromStdString(ptest->header.Name_Test) << QString::fromStdString(ptest->header.Catalog);
        }
    }
}
//-----------------------------------------------------------------------------
//--- Translate_Tests
//-----------------------------------------------------------------------------
void Analysis::Translate_Tests(QStringList &list)
{
    QString text, str = "";
    char *p;
    QByteArray ba;
    QStringList list_in, list_out;
    int id;
    QString key, value;

    Map_TestTranslate.clear();

    foreach(text, list)
    {
        if(str.length()) str += "\r\n";
        str += text;
    }

    if(str.length())
    {
        ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");
        if(ext_dll_handle)
        {
            translate_service = (TranslateService)(::GetProcAddress(ext_dll_handle,"TranslateStr"));
            if(translate_service)
            {
                //text = "Balu... - Я";
                list_in = str.split("\r\n");
                translate_service(ID_LANG, str.toUtf8().data(), &p);

                ba.clear();
                ba.append(p);

                text = QString::fromLocal8Bit(ba);                      // ok
                //text = QString::fromStdString(ba.toStdString());      // ok
                list_out = text.split("\r\n");


                id = 0;
                foreach(text, list_in)
                {
                    key = text;
                    if(id >= list_out.size()) break;
                    value= list_out.at(id);
                    Map_TestTranslate.insert(key, value);

                    id++;
                }
            }
            ::FreeLibrary(ext_dll_handle);
        }
    }

    //qDebug() << "TranslateService - ok" << Map_TestTranslate;

}
//-----------------------------------------------------------------------------
//--- Validate_Calibration()
//-----------------------------------------------------------------------------
bool Analysis::Validate_Calibration()
{
    int i;
    int num = 0;
    QString text;
    bool find = false;
    bool mask = false;
    QDir VideoData;
    QFileInfoList list_Info;
    QFileInfo fi;
    rt_Preference   *property;

    VideoData.setPath("");
    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        property = prot->preference_Pro.at(i);
        if(QString::fromStdString(property->name) == "VideoData_Catalogue")
        {
            VideoData.setPath(QString::fromStdString(property->value));
            find = true;
            continue;
        }
        if(QString::fromStdString(property->name).startsWith("ImageData_"))
        {
            mask = true;
            continue;
        }
    }

    if((!find || !VideoData.exists() || VideoData.count() < COUNT_CH) && !mask)
    {
        QApplication::restoreOverrideCursor();

        message.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        //message.button(QMessageBox::Ok)->animateClick(10000);
        text = tr("Attention! Video catalogue don't found! (or empty)");
        text += "\r\n";
        text += tr("Do you want to select correct path for this directory?");
        message.setIcon(QMessageBox::Warning);
        message.setText(text);
        if(message.exec() == QMessageBox::Cancel) return(false);

        text = QFileDialog::getExistingDirectory(this, tr("Select directory"), "", QFileDialog::ShowDirsOnly);
        if(text.isEmpty()) return(false);

        VideoData.setPath(text);
        list_Info = VideoData.entryInfoList(QDir::Files);
        foreach(fi, list_Info)
        {
            if(fi.baseName().startsWith("video_") && fi.suffix().contains("dat")) num++;
        }

        if(!VideoData.exists() || VideoData.count() < COUNT_CH || num < COUNT_CH)
        {
            message.setStandardButtons(QMessageBox::Ok);
            text = "Invalid choice!";
            message.setIcon(QMessageBox::Warning);
            message.setText(text);
            message.exec();
            return(false);
        }

        if(!find)
        {
            property = new rt_Preference;
            prot->preference_Pro.push_back(property);
        }
        property->name = QString("VideoData_Catalogue").toStdString();
        property->value = VideoData.absolutePath().toStdString();

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    }

    return(true);
}

//-----------------------------------------------------------------------------
//--- Check_Marker()
//-----------------------------------------------------------------------------
void Analysis::Check_Marker()
{
    Attention_Marker = "";
    if(!prot || prot->count_PCR == 0) return;

    int i,j,k;
    int id, id_channel;
    int num_id, pos, key;
    int num, num_channel;
    QString text;
    bool exist;
    double value, value_Mean;

    QVector<int> ID_Tubes;
    QVector<int> ID_NULL;
    QVector<double> NULL_tubes;
    QVector<double> ALL_tubes;
    QMap<int,double> Marker_values;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;
    rt_Tube         *tube;

    int col,row;
    prot->Plate.PlateSize(prot->count_Tubes, row, col);

    //... cycle of groups ...
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            id_channel = -1;
            ID_Tubes.clear();
            ID_NULL.clear();
            sample = group->samples.at(j);
            test = sample->p_Test;
            foreach(tube_test, test->tubes)
            {
                foreach(channel_test, tube_test->channels)
                {
                    text = QString::fromStdString(channel_test->name);
                    if(text.trimmed() == tr("Marker"))
                    {
                        id_channel = channel_test->ID_Channel;
                        ID_Tubes.append(tube_test->ID_Tube);
                    }
                }
            }
            // ID_NULL ...
            if(id_channel >= 0)
            {
                foreach(tube_test, test->tubes)
                {
                    exist = false;
                    foreach(channel_test, tube_test->channels)
                    {
                        if(channel_test->ID_Channel == id_channel)
                        {
                            exist = true;
                            break;
                        }
                    }
                    if(!exist) ID_NULL.append(tube_test->ID_Tube);
                }
            }


            // ...
            num_channel = -1;
            for(k=0; k<COUNT_CH; k++)
            {
                if(prot->active_Channels & 0xf<<k*4) num_channel++;
                if(k == id_channel) break;
            }

            if(num_channel >= 0 && ID_Tubes.size())  // check value
            {
                NULL_tubes.clear();
                ALL_tubes.clear();
                Marker_values.clear();

                for(k=0; k<sample->tubes.size(); k++)
                {
                    tube = sample->tubes.at(k);
                    id = tube->ID_Tube;
                    pos = tube->pos;
                    num = prot->count_PCR*prot->count_Tubes*num_channel + pos*prot->count_PCR + 5;
                    num_id = ID_Tubes.indexOf(id);

                    if(num_id >= 0)   // find Marker
                    {
                        Marker_values.insert(pos, prot->PCR_Filtered.at(num));
                        ID_Tubes.remove(num_id);
                        continue;
                    }

                    ALL_tubes.append(prot->PCR_Filtered.at(num));

                    // NULL_tubes:
                    num_id = ID_NULL.indexOf(id);
                    if(num_id >= 0)
                    {
                        NULL_tubes.append(prot->PCR_Filtered.at(num));
                        ID_NULL.remove(num_id);
                    }
                }
                if(NULL_tubes.size())
                {
                    value_Mean = std::accumulate(NULL_tubes.begin(),NULL_tubes.end(),0)/NULL_tubes.size();
                }
                else
                {
                    value_Mean = std::accumulate(ALL_tubes.begin(),ALL_tubes.end(),0)/ALL_tubes.size();
                }

                for(k=0; k<Marker_values.keys().size(); k++)
                {
                    key = Marker_values.keys().at(k);
                    value = Marker_values.value(key);
                    if(value < value_Mean*3)
                    {
                        if(Attention_Marker.length()) Attention_Marker += ",";
                        Attention_Marker += Convert_IndexToName(key, col);
                    }
                }
            }
        }
    }


    if(!Attention_Marker.isEmpty())
    {
        Attention_Marker = tr("Position of the marker is incorrect or it is absent in the tubes: ") + Attention_Marker;
        if(!MarkerAttention)
        {
            QApplication::restoreOverrideCursor();
            message_Marker.setText(Attention_Marker);
            message_Marker.setStandardButtons(QMessageBox::Ok);
            message_Marker.setIcon(QMessageBox::Warning);
            message_Marker.button(QMessageBox::Ok)->animateClick(10000);
            message_Marker.exec();
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        }
    }
}
//-----------------------------------------------------------------------------
//--- Check_OverflowData(...)
//-----------------------------------------------------------------------------
void Analysis::Check_OverflowData(QString &overflow)
{
    int id;
    short value;
    QString text = "";
    rt_Measurement *measure;
    QVector<short> limit(COUNT_CH);
    QString fluor_name[COUNT_CH] = FLUOR_NAME;

    limit.fill(0,COUNT_CH);

    foreach(measure, prot->meas)
    {
        if(measure->num_exp == 0) continue;     // only for second expo

        id = measure->optical_channel;
        if(limit.at(id)) continue;              // if channel overflow -> continue;

        foreach(value, measure->measurements)
        {
            if(value & 0x8000)
            {
                limit.replace(id,1);
                break;
            }
        }
    }
    //qDebug() << "Check_OverflowData: " << limit;

    id = 0;
    foreach(value, limit)
    {
        if(value)
        {
            if(!text.isEmpty()) text += ",";
            text += fluor_name[id];
        }
        id++;
    }
    if(!text.isEmpty()) overflow = tr("the optical signal has exceeded the maximum allowed level in the channels: ") + text;

}

//-----------------------------------------------------------------------------
//--- Check_CpValidity(...)
//-----------------------------------------------------------------------------
void Analysis::Check_CpValidity(QString &sig)
{
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Channel      *channel;
    std::string     s;
    QString         result;
    bool            find;
    POINT_TAKEOFF   *sigmoid;
    int             id, ch;
    double          Cp_value;

    int pos;
    int row,col;
    prot->Plate.PlateSize(prot->count_Tubes,row,col);    

    foreach(group, prot->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            foreach(tube, sample->tubes)
            {
                pos = tube->pos;
                ch = 0;
                foreach(channel, tube->channels)
                {
                    find = false;
                    ch++;
                    foreach(s, channel->result_Channel)
                    {
                        id = (ch-1)*prot->count_Tubes + pos;
                        if(id >= prot->list_PointsOff.size()) break;
                        sigmoid = prot->list_PointsOff.at(id);
                        Cp_value = sigmoid->cp_Sigmoid;

                        result = QString::fromStdString(s);
                        if(result.startsWith("Sigmoid_validity="))
                        {
                            if(result.contains("?") && sigmoid->valid) {find = true; break;}
                            continue;
                        }
                        if(result.startsWith("aFF="))
                        {
                            if(result.contains("warning") && Cp_value >= 0) {find = true; break;}
                            continue;
                        }
                        if(result.startsWith("Relative_validity="))
                        {
                            if(result.contains("warning") && Cp_value >= 0) {find = true; break;}
                            continue;
                        }
                    }
                    if(find)
                    {
                        if(sig.length()) sig += ",";
                        sig += Convert_IndexToName(pos,col);
                        break;
                    }
                }
            }
        }
    }

}

//-----------------------------------------------------------------------------
//--- Attention_Analysis()
//-----------------------------------------------------------------------------
void Analysis::Attention_Analysis()
{
    QString Attention_Preference = "";
    QString Attention_CpValidity = "";
    QString Attention_OverFlow = "";
    rt_Preference*  property;
    Attention_Browser->clear();
    QString text;
    double dvalue;
    bool ok;
    bool find = false;

    QStringList list;
    list << USE_CROSSTALK << USE_OPTICALCORRECTION << USE_DIGITFILTER << PARAM_SPLINECUBE;

    // 1. Preference of the protocol
    foreach(property, prot->preference_Pro)
    {
        text = QString::fromStdString(property->name);
        if(list.indexOf(text) < 0) continue;
        if(text == PARAM_SPLINECUBE)
        {
            dvalue = QString::fromStdString(property->value).toDouble(&ok);
            if(!ok || dvalue != 30) {find = true; break;}
            continue;
        }
        if(property->value != "yes")
        {
            find = true;
            break;
        }
    }
    if(find) Attention_Preference = tr("Change preferences of the protocol!");    

    // 2. S,aFF,K+ validation
    if(prot->count_PCR)
    {
        Check_CpValidity(Attention_CpValidity);
        if(!Attention_CpValidity.isEmpty())
        {
            Attention_CpValidity = tr("Incorrect PCR validity in tubes: ") + Attention_CpValidity;
        }
    }

    // 3. OverFlow
    if(prot->meas.size())
    {
        Check_OverflowData(Attention_OverFlow);
    }


    //...
    if(!Attention_Preference.isEmpty() ||
       !Attention_Marker.isEmpty() ||
       !Attention_CpValidity.isEmpty() ||
       !Attention_OverFlow.isEmpty())
    {
        GridTab->addTab(Attention_Browser, QIcon(":/images/flat/attention_flat_red.png"), tr("Attention"));
        GridTab->tabBar()->setTabTextColor(3, Qt::red);
    }
    else return;

    if(Attention_Preference.length())
    {
        Attention_Browser->setTextColor(Qt::red);
        Attention_Browser->insertPlainText("\r\n" + tr("Preferences: "));
        Attention_Browser->setTextColor(Qt::black);
        Attention_Browser->insertPlainText(Attention_Preference);
    }
    if(Attention_Marker.length())
    {
        Attention_Browser->setTextColor(Qt::red);
        Attention_Browser->insertPlainText("\r\n" + tr("Marker: "));
        Attention_Browser->setTextColor(Qt::black);
        Attention_Browser->insertPlainText(Attention_Marker);
    }
    if(Attention_CpValidity.length())
    {
        Attention_Browser->setTextColor(Qt::red);
        Attention_Browser->insertPlainText("\r\n" + tr("Validity(Cp): "));
        Attention_Browser->setTextColor(Qt::black);
        Attention_Browser->insertPlainText(Attention_CpValidity);
    }
    if(Attention_OverFlow.length())
    {
        Attention_Browser->setTextColor(Qt::red);
        Attention_Browser->insertPlainText("\r\n" + tr("Overflow: "));
        Attention_Browser->setTextColor(Qt::black);
        Attention_Browser->insertPlainText(Attention_OverFlow);
    }

    // write to property ...
/*
    find = false;
    foreach(property, prot->preference_Pro)
    {
        if(property->name == "Attention_Analysis")
        {
            find = true;
            break;
        }
    }

    if(find)
    {
        property = new rt_Preference();
        property->name = "Attention_Analysis";
    }
    property->value = Attention_Browser->toPlainText().toStdString();
*/


}
//-----------------------------------------------------------------------------
//--- Validate_Test(rt_Test*)
//-----------------------------------------------------------------------------
bool Analysis::Validate_Test(rt_Test *ptest)
{
    bool res = false;
    QString xml_str;
    QString status;
    rt_Preference   *preference;

    if(!axgp) return(res);

    foreach(preference, ptest->preference_Test)
    {
        if(preference->name == "xml_node")
        {
            xml_str = QString::fromStdString(preference->value);
            res = axgp->dynamicCall("loadXmlTest(QString)", xml_str).toBool();
            if(res) status = "ok";
            else status = "none";
            ptest->header.Hash = status.toStdString();
            break;
        }
    }

    return(res);
}
//-----------------------------------------------------------------------------
//--- Unique_FileName
//-----------------------------------------------------------------------------
QString Analysis::Unique_FileName(QString fn)
{
    QString unique_text = fn + QDateTime::currentDateTime().toString("_ddMMyy_HHmmss");
    return(unique_text);
}

//-----------------------------------------------------------------------------
//--- Check_ValidNameFile
//-----------------------------------------------------------------------------
QString Analysis::Check_ValidNameFile(QString name)
{
    QString symbol;
    QStringList list;
    list << "\\" << "/" << "?" << "*" << """" << "|" << "<" << ">" << "+" << ":";

    foreach(symbol, list)
    {
        name.replace(symbol,"");
    }

    return(name);
}
//-----------------------------------------------------------------------------
//--- Original_FileName(QString fn)
//-----------------------------------------------------------------------------
QString Analysis::Original_FileName(QString fn)
{    
    QFileInfo fi(fn);
    int id = 0;
    QString file_name;
    QString name, str_add;
    QString suffix = fi.suffix();

    if(!fi.exists()) return(fn);

    QString dir = fi.absolutePath();
    name = dir + "/" + fi.completeBaseName();

    while(id < 100)
    {
        id++;
        str_add = QString("_(%1)").arg(id);
        file_name = name + str_add + "." + suffix;
        fi.setFile(file_name);
        if(!fi.exists()) return(file_name);
    }

    return(fn);
}

//-----------------------------------------------------------------------------
//--- Create_HashProtocol(QString)
//-----------------------------------------------------------------------------
bool Analysis::Create_HashProtocol(QString fn)
{
    QDomDocument    doc;
    QDomElement     root;
    QDomElement     result;
    QDomElement     item;

    bool res = false;

    QString         hash_string;
    QCryptographicHash Hash(QCryptographicHash::Sha256);

    QFile file(fn);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return(false);
        }
        file.close();

        hash_string = doc.toString();
        Hash.addData(hash_string.toUtf8());

        QByteArray hash_compress;
        QByteArray hash(Hash.result());             // QCryptographicHash::Sha256
        std::reverse(hash.begin(), hash.end());     // reverse
        hash_compress = qCompress(hash);            // Compress

        hash_string = hash_compress.toHex();

        root = doc.documentElement();
        item = root.firstChildElement("Analysis_HASH");
        result = doc.createElement("Analysis_HASH");
        result.appendChild(MakeElement(doc, "hash", hash_string));

        if(item.isNull()) root.appendChild(result);
        else root.replaceChild(result,item);

        QFile::remove(fn);
        QFile file_out(fn);
        if(file_out.open(QIODevice::WriteOnly))
        {
            QTextStream(&file_out) << doc.toString();
            file_out.close();
            res = true;
        }        
    }
    return(res);
}
//-----------------------------------------------------------------------------
//--- change_MarkerAttention
//-----------------------------------------------------------------------------
void Analysis::change_MarkerAttention(int state)
{
    if(static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) MarkerAttention = true;
    else MarkerAttention = false;
}
//-----------------------------------------------------------------------------
//--- Change_IndexTabGrid(int)
//-----------------------------------------------------------------------------
void Analysis::Change_IndexTabGrid(int index)
{
    if(index == 2 || index == 1) reportAct->setEnabled(true);
    else reportAct->setEnabled(false);

    if(index == 2) exportResults->setEnabled(true);
    else exportResults->setEnabled(false);

    if(index == 0) reportParamAct->setEnabled(true);
    else reportParamAct->setEnabled(false);
}
//-----------------------------------------------------------------------------
//--- Save_ChangesProtocol()
//-----------------------------------------------------------------------------
void Analysis::Save_ChangesProtocol()
{
    QString text;

    //flag_SaveChangesProtocol = Color_IsChanged();
    if(!flag_SaveChangesProtocol && !Color_IsChanged() && !Comments_IsChanged()) return;

    flag_SaveChangesProtocol = false;
    message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    text = tr("Attention! Do you want to save last changes of the current protocol");
    message.setIcon(QMessageBox::Warning);
    message.setText(text);
    if(message.exec() == QMessageBox::Yes)
    {
        save();
    }
}
//-----------------------------------------------------------------------------
//--- Color_IsChanged()
//-----------------------------------------------------------------------------
bool Analysis::Color_IsChanged()
{
    rt_GroupSamples *pgroup;
    rt_Sample       *psample;
    rt_Tube         *ptube;
    int pos;
    bool change_color = false;

    foreach(pgroup, prot->Plate.groups)
    {
        foreach(psample, pgroup->samples)
        {
            foreach(ptube, psample->tubes)
            {
                pos = ptube->pos;
                if(ptube->color != prot->color_tube.at(pos))
                {
                    ptube->color = prot->color_tube.at(pos);
                    change_color = true;
                }
                break;
            }
            if(change_color) break;
        }
        if(change_color) break;
    }

    return(change_color);
}
//-----------------------------------------------------------------------------
//--- Comments_IsChanged()
//-----------------------------------------------------------------------------
bool Analysis::Comments_IsChanged()
{
    bool change_comment = false;
    rt_Preference   *property;
    bool            find = false;
    QString         text = Info_Pro->comments->toPlainText();

    foreach(property, prot->preference_Pro)
    {
        if(property->name == "Comments")
        {
            if(property->value != text.toStdString())
            {
                property->value = text.toStdString();
                change_comment = true;
            }
            find = true;
            break;
        }
    }

    if(!find && !text.isEmpty())
    {
        property = new rt_Preference;
        property->name = "Comments";
        property->value = text.toStdString();
        prot->preference_Pro.push_back(property);
        change_comment = true;
    }

    //qDebug() << "Comments_IsChanged: " << change_comment <<  prot->preference_Pro.size() << prot;

    return(change_comment);
}
//-----------------------------------------------------------------------------
//--- Load_DTRVersion
//-----------------------------------------------------------------------------
void Analysis::Load_DTRVersion(rt_Protocol *p, QVector<QString> *list)
{
    HINSTANCE ext_handle;
    QString filename, text;
    QString path_DTReport = qApp->applicationDirPath() + "\\DTReport2.dll";
    QFileInfo fi;
    QDir modules_Dir(qApp->applicationDirPath() + "\\forms");

    char *DisplayName;
    char *Path;
    int release, build;
    QVector<QString> *vec;

    UINT Size,Len;
    UINT h;
    BYTE* buf;
    char *ValueBuf;
    int lang_page;
    QString cmd,str;

    get_Info = NULL;

    ext_handle = ::LoadLibraryW(L"DTReport2.dll");

    if(ext_handle)
    {
        // dtr
        get_Info = (GetDTRDisplayInfo)(::GetProcAddress(ext_handle,"GetDTRDisplayInfo"));
        if(get_Info)
        {
            vec = new QVector<QString>();
            p->Map_Reserve.insert("DTR_Version", vec);
            foreach(text, *list)
            {
                filename = QString("%1.dtr").arg(text);
                fi.setFile(modules_Dir, filename);
                get_Info(text.toLatin1().data(), &DisplayName, &Path, &release, &build);

                text = QString("%1.dtr ver:%2.%3").arg(text).arg(release).arg(build);
                vec->append(text);
            }
        }

        // DTReport2.dll
        Size = GetFileVersionInfoSizeA(path_DTReport.toStdString().c_str(), NULL);
        if(Size)
        {
            buf = new BYTE[Size];
            if(GetFileVersionInfoA(path_DTReport.toStdString().c_str(), h, Size, buf) != 0)
            {
                VerQueryValueA(buf,"\\VarFileInfo\\Translation", (LPVOID*)&ValueBuf, &Len);
                if(Len >= 4)
                {
                    lang_page = (int)MAKELONG(*(int *)(ValueBuf+2), *(int *)ValueBuf);
                    str = QString("%1").arg(lang_page,8,16,QLatin1Char('0'));
                    cmd = "\\StringFileInfo\\" + str + "\\LegalCopyright";
                    if(VerQueryValueA(buf, cmd.toStdString().c_str(), (LPVOID*)&ValueBuf, &Len) != 0)
                    {
                        cmd = "\\StringFileInfo\\" + str + "\\FileVersion";
                        if(VerQueryValueA(buf, cmd.toStdString().c_str(), (LPVOID*)&ValueBuf, &Len) != 0)
                        {
                            cmd = QString::fromStdString(ValueBuf);

                            // Add info about version to Map_Reserve (DTR_Version)
                            vec->append(QString("DTReport.dll ver:%1").arg(cmd));
                        }
                    }
                }
            }

            delete buf;
        }

        //...

        ::FreeLibrary(ext_handle);
    }

}

//-----------------------------------------------------------------------------
//--- Load_InfoEXTAnalyser()
//-----------------------------------------------------------------------------
void Analysis::Load_InfoEXTAnalyser()
{
    int id = 0x20000;
    int ID;
    QDir Dtr_Dir(qApp->applicationDirPath());
    Dtr_Dir.cd("forms");
    QFileInfo fn;
    QString text;
    QByteArray ba;
    quint16 crc16;

    foreach(QString fileName, Dtr_Dir.entryList(QDir::Files))
    {
        fn.setFile(fileName);
        text = fn.completeBaseName();
        if(text == "Default" || text == "ReportSummary" || text == "ReportTitle") continue;

        ba.clear();
        ba.append(text.toLatin1());
        crc16 = qChecksum(ba.data(),ba.size());
        ID = id + crc16;

        Map_EXTanalyserNAME.insert(ID, text);
    }    

    //qDebug() << Map_EXTanalyserNAME;
}

//-----------------------------------------------------------------------------
//--- Load_InfoAnalyser()
//-----------------------------------------------------------------------------
void Analysis::Load_InfoAnalyser()
{
    QVector<QString> info_analyser;
    bool ok;
    QString fn;
    QString text;
    int method_ID;

    QDir pluginsDir(qApp->applicationDirPath());
    pluginsDir.cd("analyser_plugins");    

    foreach(QString fileName, pluginsDir.entryList(QDir::Files))
    {
        analyser_handle = NULL;
        analyser = NULL;
        info_analyser.clear();

        fn = "analyser_plugins\\" + fileName;
        analyser_handle = ::LoadLibraryW(fn.toStdWString().c_str());

        if(analyser_handle)
        {
            Analysis_factory factory_func = reinterpret_cast<Analysis_factory>(
                                            ::GetProcAddress(analyser_handle,"createAnalyser_plugin@0"));            

            if(factory_func)
            {
                analyser = factory_func();

                if(analyser)
                {
                    analyser->GetInformation(&info_analyser);
                    method_ID = info_analyser.at(0).toInt(&ok,16);
                    text = info_analyser.at(1);
                    Map_analyserNAME.insert(method_ID, text);
                }
            }
            if(analyser) delete analyser;
            if(analyser_handle) ::FreeLibrary(analyser_handle);
        }
    }    
}

//-----------------------------------------------------------------------------
//--- addProtocol_toList
//-----------------------------------------------------------------------------
void Analysis::addProtocol_toList(QString fn)
{
    int i,k;
    QString text;
    int count_border = 10;
    QStringList list_fn;
    QFileInfo fi;

    if(fn == fn_OUTTMP || fn == fn_OUT) return;

    ApplSettings->beginGroup("LastProtocols");

    QStringList list = ApplSettings->childKeys();

    for(i=0; i<list.count(); i++)
    {
        text = ApplSettings->value(list.at(i),"").toString();
        fi.setFile(text);
        if(!fi.exists()) continue;
        list_fn.append(text);
    }

    k = list_fn.indexOf(fn);

    if(k >= 0) list_fn.removeAt(k);
    list_fn.insert(0,fn);

    ApplSettings->endGroup();
    ApplSettings->remove("LastProtocols");

    ApplSettings->beginGroup("LastProtocols");
    k = 0;
    for(i=0; i<list_fn.count(); i++)
    {
        text = list_fn.at(i).trimmed();
        if(text.isEmpty()) continue;

        ApplSettings->setValue(QString::number(k), text);
        k++;
        if(count_border <= k) break;
    }
    ApplSettings->endGroup();

}
//-----------------------------------------------------------------------------
//--- static wrapper-function to be able to callback
//---    the member function Display_ProgressBar
//-----------------------------------------------------------------------------
void Analysis::Wrapper_To_Display_ProgressBar(void *pt2Object, int percent, QString text)
{
//--- explicitly cast to a pointer to TClassA

    Analysis *mySelf = (Analysis *) pt2Object;

//--- call member

    mySelf->Display_ProgressBar(percent, text);
   // qDebug() << "ok";

}

//-----------------------------------------------------------------------------
//--- static wrapper-function to be able to callback
//---    the member function Display_ProgressBar
//-----------------------------------------------------------------------------
void __stdcall Analysis::Wrapper_To_ReportStatus(int StepNo, int StepCount, void *pt2Object, bool Cancel)
{
    int percent = StepNo * 100./StepCount;
    QString text;

    Analysis *mySelf = (Analysis *) pt2Object;

    text = QString("%1% - Create Report...").arg(percent);
    mySelf->Display_ProgressBar(percent, text);    

}
//-----------------------------------------------------------------------------
//--- callback for progress bar
//---
//-----------------------------------------------------------------------------
void __stdcall Analysis::Callback_SetProgressBar(bool sts, int pos, char** comment, char** err)
{
    char *p;
    p = *comment;
    QByteArray ba(p);
    QString text;
    QTextCodec *codec1 = QTextCodec::codecForName("CP1251");
    Analysis *mySelf = (Analysis*)Analysis_Obj;

    if(sts)
    {
        text = codec1->toUnicode(ba);
        mySelf->Display_ProgressBar(pos, text);
    }
    else
    {
        mySelf->Display_ProgressBar(0, "");
    }
}

//-----------------------------------------------------------------------------
//--- callback for select tube in  graph
//---
//-----------------------------------------------------------------------------
void __stdcall Analysis::Callback_SelectTube(char* tube, char** err)
{
    int pos = 1;
    int row,col;
    int count_tubes;

    QString text(tube);    

    Analysis *mySelf = (Analysis*)Analysis_Obj;
    count_tubes = mySelf->prot->count_Tubes;
    mySelf->prot->Plate.PlateSize(count_tubes,row,col);
    pos = Convert_NameToIndex(text,col);
    mySelf->plot->Select_Curve(pos);
}

//-----------------------------------------------------------------------------
//--- execution_MenuAction
//-----------------------------------------------------------------------------
void Analysis::execution_MenuAction(QString type)
{
    if(type == "open") {open_Protocol(); return;}
    if(type == "save") {save(); return;}
    if(type == "info") {info_Protocol(); return;}
    if(type == "report_param") {report_Param(); return;}
    if(type == "export") {Export_Results(); return;}
    if(type == "report") {report(); return;}
    if(type == "email") {email_Send(); return;}
    if(type == "to_setup") {load_to_setup(); return;}
    if(type == "reopen") {reopen(); return;}
    if(type == "edit_test") {editTest(); return;}
    if(type == "edit_protocol") {editPreferencePro(); return;}
    if(type == "preview_archive") {preview_archive(); return;}
    if(type == "rdml_import") {rdml_import(); return;}

}

//-----------------------------------------------------------------------------
//--- Create_MenuActions(void *)
//-----------------------------------------------------------------------------
void Analysis::Create_MenuActions(void *list_pinfo)
{
    QStringList *list = (QStringList*)list_pinfo;

    list->append(openXmlAct->toolTip() + "\t" + "open" + "\t" + "1");
    list->append(saveAct->toolTip() + "\t" + "save" + "\t" + "1");
    list->append(infoAct->toolTip() + "\t" + "info" + "\t" + "1");
    list->append(reportParamAct->toolTip() + "\t" + "report_param" + "\t" + "1");
    list->append(reportAct->toolTip() + "\t" + "report" + "\t" + "1");
    list->append(exportResults->toolTip() + "\t" + "export" + "\t" + "1");
    list->append(EMail_send->toolTip() + "\t" + "email" + "\t" + "1");
    list->append("...");
    list->append(load_ToSetup->toolTip() + "\t" + "to_setup" + "\t" + QString::number((int)flag_ReOpenProtocol));
    list->append(reopenXmlAct->toolTip() + "\t" + "reopen" + "\t" + QString::number((int)flag_ReOpenProtocol));
    list->append(editTestAct->toolTip() + "\t" + "edit_test" + "\t" + QString::number((int)flag_EditTest));
    list->append(edit_PreferenceProtocol->toolTip() + "\t" + "edit_protocol" + "\t" + QString::number((int)flag_CommonSettings));
    list->append("...");
    if(dll_rdml) list->append(RDML_import->toolTip() + "\t" + "rdml_import" + "\t" + "1");

}
//-----------------------------------------------------------------------------
//--- Enable_MenuAction(void *)
//-----------------------------------------------------------------------------
void Analysis::Enable_MenuAction(void *map_enable)
{
    QMap<QString, int> *map = (QMap<QString, int>*)map_enable;

    map->insert("open", openXmlAct->isEnabled());
    map->insert("save", saveAct->isEnabled());
    map->insert("info", infoAct->isEnabled());
    map->insert("report_param", reportParamAct->isEnabled());
    map->insert("report", reportAct->isEnabled());
    map->insert("export", exportResults->isEnabled());
    map->insert("email", true); //EMail_send->isEnabled());
    map->insert("to_setup", load_ToSetup->isEnabled());
    map->insert("reopen", reopenXmlAct->isEnabled());
    map->insert("edit_test", editTestAct->isEnabled());
    map->insert("edit_protocol", edit_PreferenceProtocol->isEnabled());
    map->insert("preview_archive", preview_videoarchive->isEnabled());
    map->insert("rdml_import", RDML_import->isEnabled());
}

//-------------------------------------------------------------------------------
//--- restore_Cursor()
//-------------------------------------------------------------------------------
void Analysis::restore_Cursor()
{
    QApplication::restoreOverrideCursor();
    qDebug() << "Analysis: restore_Cursor";
    //this->setCursor(Qt::ArrowCursor);
    this->setFocus();
}

//-------------------------------------------------------------------------------
//--- User events
//-------------------------------------------------------------------------------
bool Analysis::event(QEvent *e)
{
    int i;
    int col,row;
    QString text, str;
    //Analysis_Interface *analyser;
    QCursor cur;
    QPoint point;


    if(e->type() == 1100)       // Enable tubes
    {
        EnableTubes *p_event = static_cast<EnableTubes *>(e);
        if(p_event)
        {
            //qDebug() << "Get row:" << p_event->enable_tubes;
            //QMessageBox::information(this,"event","user 1100");
            //change_Enable(&p_event->enable_tubes);
            plot->Change_Enable(prot, &p_event->enable_tubes);
            if(alg)
            {
                alg->Enable_Tube(&p_event->enable_tubes);
            }
            foreach(analyser, Map_analyser.values())
            {
                analyser->Enable_Tube(&p_event->enable_tubes);
            }
            // dtr
            if(ext_dll_handle && enable_tubes && ext_dll_handle_VALID)
            {
                if(prot->count_Tubes == prot->enable_tube.size())
                {
                    text = " ";
                    prot->Plate.PlateSize(prot->count_Tubes,row,col);
                    for(i=0; i<prot->count_Tubes; i++)
                    {
                      if(!prot->enable_tube.at(i))
                      {
                          str = Convert_IndexToName(i, col);
                          if(text.length()) text += ",";
                          text += str;
                      }
                    }
                    enable_tubes(text.toLatin1().data());
                }
            }
            //qApp->processEvents();
            //QApplication::restoreOverrideCursor();
            //QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
            //sel->setCursor(QCursor(Qt::ArrowCursor));
            //qApp->processEvents();

            //Sleep(1000);
            //QTimer::singleShot(1000, this, SLOT(restore_Cursor()));
            //QTimer::singleShot(2000, sel, SLOT(restore_Cursor()));
            if(ext_dll_handle && enable_tubes) sel->restore_Cursor();
            //
            //sel->setCursor(QCursor(Qt::ArrowCursor));
            /*point = QCursor::pos();
            cur = sel->cursor();
            cur.setPos(0,0);
            qApp->processEvents();
            cur.setPos(point);
            sel->setCursor(QCursor(Qt::ArrowCursor));
            sel->repaint();
            qApp->processEvents();*/

            return(true);
        }
    }

    if(e->type() == 1110)       // Select tube
    {
        Select_Tube *p_event = static_cast<Select_Tube *>(e);
        if(p_event)
        {

            if(alg)
            {
                alg->Select_Tube(p_event->pos_tube);
                //QMessageBox::information(this,"event","user 1110");
            }
            foreach(analyser, Map_analyser.values())
            {
                analyser->Select_Tube(p_event->pos_tube);
            }
            // dtr
            if(ext_dll_handle && select_curve && ext_dll_handle_VALID)
            {

                prot->Plate.PlateSize(prot->count_Tubes,row,col);
                text = Convert_IndexToName(p_event->pos_tube, col);
                select_curve(text.toLatin1().data());
                //qDebug() << "select_curve: " << p_event->pos_tube << text;
            }

            return(true);
        }
    }

    if(e->type() == 1111)       // Select plot
    {
        Select_Plot *p_event = static_cast<Select_Plot *>(e);
        if(p_event)
        {
            //qDebug() << p_event->pos_plot;
            plot->Select_Curve(p_event->pos_plot);
        }

        return(true);
    }

    if(e->type() == 1112)       // Open TechReport
    {
        Open_TechReport *p_event = static_cast<Open_TechReport *>(e);
        if(p_event)
        {
            qDebug() << "Open TechReport: " << p_event->report_dll;

            QFile f(p_event->report_dll);
            if(f.exists())
            {
                Report_All = new Report(prot, user_Dir.absolutePath(), p_event->report_dll, this);
                delete Report_All;
            }
        }

        return(true);
    }

    if(e->type() == 1120)       // reAnalysis
    {
        ReAnalysis *p_event = static_cast<ReAnalysis *>(e);
        if(p_event)
        {            
            if(alg)
            {
                alg->Analyser(prot, p_event->param);
            }            
            foreach(analyser, Map_analyser.values())
            {
                analyser->Analyser(prot);
            }            

            return(true);
        }
    }

    if(e->type() == 9001)       // close all chaild windows
    {
        if(!sel->isHidden() & !sel->allowedAreas()) sel->hide();
        if(!Info_Pro->isHidden()) Info_Pro->hide();
        return(true);
    }


    if(e->type() == 3004)       // open protocol (file from Run)
    {
        ArchiveFile_path = this->parentWidget()->property("measurements_ArchiveFile").toString();
        text = this->parentWidget()->property("measurements_file").toString();
        if(!text.isEmpty()) open_Protocol(text, false, true);
        return(true);
    }

    if(e->type() == 3008)       // reopen protocol (file from Setup)
    {
        text = this->parentWidget()->property("setup_to_analysis").toString();
        if(!text.isEmpty())
        {
            reopen(text);
        }
        return(true);
    }

    // Point_Action Event
    if(e->type() == 2002)
    {
        Action_Event *p_event = static_cast<Action_Event *>(e);
        if(p_event)
        {
            //qDebug() << "Analysis p_event: " << p_event->point_action->Unique_Name;
            emit sActionPoint(p_event);
        }

        return(true);
    }

    if(e->type() == 3011)       // open protocol (file from Web)
    {
        text = parentWidget()->property("Web_to_analysis").toString();
        QFile file(text);
        if(file.exists())
        {
            open_Protocol(text);
        }
        return(true);
    }

    if(e->type() == 4005)       // open View VideoArchive
    {
        QTimer::singleShot(200, this, SLOT(preview_archive()));
        return(true);
    }


    return QWidget::event(e);
}
//-------------------------------------------------------------------------------
//--- resizeEvent
//-------------------------------------------------------------------------------
void Analysis::resizeEvent(QResizeEvent *e)
{
    main_progress->setFixedWidth(ChartWindow->width());
    Name_Protocol->setFixedWidth(ChartWindow->width() - Label_Protocol->width());

    int w = GridWindow->width();
    int h = GridWindow->height();
    label_gif->move(w/2 - label_gif->width()/2, h/2 - label_gif->height()/2);
}
//-------------------------------------------------------------------------------
//--- showEvent
//-------------------------------------------------------------------------------
void Analysis::showEvent(QShowEvent *e)
{
    main_progress->setFixedWidth(ChartWindow->width());
    Name_Protocol->setFixedWidth(ChartWindow->width() - Label_Protocol->width());

    selectAct->setEnabled(sel->isHidden());

    int w = GridWindow->width();
    int h = GridWindow->height();
    label_gif->move(w/2 - label_gif->width()/2, h/2 - label_gif->height()/2);

}
//-------------------------------------------------------------------------------
//--- ShowHidden_Select()
//-------------------------------------------------------------------------------
void Analysis::ShowHidden_Select()
{
    selectAct->setEnabled(true);
}
//-------------------------------------------------------------------------------
//--- resizeEvent QGroupBox_ext
//-------------------------------------------------------------------------------
void GroupBox::resizeEvent(QResizeEvent *e)
{
    //qDebug() << e->size().width() << e->size().height();

    emit Resize();
}
//-------------------------------------------------------------------------------
//--- showEvent QGroupBox_ext
//-------------------------------------------------------------------------------
void GroupBox::showEvent(QShowEvent *e)
{
    emit Resize();
}
//-------------------------------------------------------------------------------
//--- resize_extReport
//-------------------------------------------------------------------------------
void Analysis::resize_extResults()
{
    int type_analysis;
    QWidget *w_obj;

    if(resize_results && GridTab->currentWidget() == GridBox_ext_all) // && Tab_Analyser->size())
    {
        resize_results((HWND)GridBox_ext_dna->winId());

        /*w_obj = Tab_Analyser->currentWidget();
        type_analysis = Map_TabAnalyser.key(w_obj);

        if(type_analysis >= 0x20000)
        {            
            resize_results((HWND)w_obj->winId());
        } */
    }
}

//-------------------------------------------------------------------------------
//--- resize_CpAllResults
//-------------------------------------------------------------------------------
void Analysis::resize_CpAllResults()
{
    int width = GridBox_allcp->width();
    //report_CpAll->move(width - report_CpAll->width() - 1, 1);
    //report_CpAll->raise();
    //report_CpAll->show();
}

//-------------------------------------------------------------------------------
//--- slot_ReceivedActionPoint(Action_Event *)
//-------------------------------------------------------------------------------
void Analysis::slot_ReceivedActionPoint(Action_Event *p_event)
{
    QString text;
    QFile file;
    QStringList list_points;
    list_points << LIST_ACTIONPOINTS_ANALYSIS;

    int id = list_points.indexOf(p_event->point_action->Unique_Name);
    action_event = p_event;
    flag_ActivePoint = true;

    switch(id)
    {
    default:    PA_Error("???");
                break;

    case 0:                     // Analysis Protocol
                //qDebug() << "ACTIVE POINT: " << action_event->point_action->Unique_Name << action_event->point_action->Param;
                text = action_event->point_action->Param.value("FileName", "");
                if(!text.isEmpty())
                {
                    file.setFileName(text);
                    if(file.exists())
                    {
                        open_Protocol(text, false, true);
                        PA_AnalysisProtocol();
                    }
                    else PA_Error("File is not exists");
                }
                else PA_Error("FileName is empty");
                break;
    }

}
//-----------------------------------------------------------------------------
//--- PA_Error()
//-----------------------------------------------------------------------------
void Analysis::PA_Error(QString text)
{
    //qDebug() << "PA_Error: ";
    flag_ActivePoint = false;
    action_event->point_action->Param.insert("status","0");
    action_event->point_action->Param.insert("error",text);
    QApplication::sendEvent(action_event->point_action->sender, action_event);
}
//-----------------------------------------------------------------------------
//--- PA_Error()
//-----------------------------------------------------------------------------
void Analysis::PA_AnalysisProtocol()
{
    //qDebug() << "PA_AnalysisProtocol: ";
    flag_ActivePoint = false;
    action_event->point_action->Param.insert("status","1");
    QApplication::sendEvent(action_event->point_action->sender, action_event);
}

//-----------------------------------------------------------------------------
//--- PA_Error()
//-----------------------------------------------------------------------------
bool Analysis::Validate_OpenProtocol(rt_Protocol *Prot, QString &str)
{
    int i,j,k;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    bool res = true;

    if(!Prot) {str = "NULL Protocol!"; return(false);}

    str = "";

    // Tube Position - ???
    for(i=0; i<Prot->Plate.groups.size(); i++)
    {
        group = Prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                if(tube->pos < 0)
                {
                    str = tr("Attention! Invalid tubes position in the Protocol!");
                    res = false;
                    break;
                }
            }
            if(str.length()) break;
        }
        if(str.length()) break;
    }

    //

    return(res);
}
