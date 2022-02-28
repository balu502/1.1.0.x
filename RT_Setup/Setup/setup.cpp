#include "setup.h"

extern "C" SETUPSHARED_EXPORT Setup* __stdcall create_page()
{
    return(new Setup());
}
extern "C" SETUPSHARED_EXPORT void __stdcall load_UserTESTs(void *p_setup, std::string s)
{
    QString fn = QString::fromStdString(s);
    ((Setup*)p_setup)->create_ListTESTs(fn);
}
extern "C" SETUPSHARED_EXPORT void __stdcall load_User(void *p_setup, QAxObject* user)
{
    ((Setup*)p_setup)->create_User(user);
}

extern "C" SETUPSHARED_EXPORT void __stdcall load_TestVerification(void *p_setup, QAxObject* axgp_resource)
{
    ((Setup*)p_setup)->create_TestVerification(axgp_resource);
}

extern "C" SETUPSHARED_EXPORT void __stdcall load_PointerDBase(void *p_setup, QAxObject* axRita_resource)
{
    ((Setup*)p_setup)->create_TestDBase(axRita_resource);
}

extern "C" SETUPSHARED_EXPORT void __stdcall load_Menu(void *p_setup, void *list_pinfo)
{
    ((Setup*)p_setup)->Create_MenuActions(list_pinfo);
}

extern "C" SETUPSHARED_EXPORT void __stdcall action_Menu(void *p_setup, QString type_action)
{
    ((Setup*)p_setup)->execution_MenuAction(type_action);
}
extern "C" SETUPSHARED_EXPORT void __stdcall enable_Menu(void *p_setup, void *enable_action)
{
    ((Setup*)p_setup)->Enable_MenuAction(enable_action);
}
extern "C" SETUPSHARED_EXPORT void* __stdcall Get_pPro(void *p_setup)
{
    return(((Setup*)p_setup)->Get_pProtocol());
}
extern "C" SETUPSHARED_EXPORT void __stdcall Splash_Object(void *p_setup, SplashScreen* s_object)
{
    ((Setup*)p_setup)->splash = s_object;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Setup::Setup(QWidget *parent): QMainWindow(parent)
{
    readCommonSettings();
    readTemplateSettings();
    setFont(qApp->font());


    //qDebug() << "Template_Name:" << template_NAME.name << template_NAME.type << template_NAME.date_time;

    /*QMessageBox::information(NULL, "QTemporaryDir",  dir_temp.path(), QMessageBox::Ok);
    QString error = dir_temp.errorString();
    QString state  = QString("%1 %2").arg(dir_temp.isValid()).arg(error);
    QMessageBox::information(NULL, "QTemporaryDir",  state, QMessageBox::Ok);
    */
    //setStyleSheet("QMainWindow{background-color: white;}"
    //              "QToolBar{background-color: white;}");

    //setStyleSheet("QGroupBox{padding-top:15px; margin-top:-15px; border: 1px solid transparent;}");

    prot = Create_Protocol();    
    prot->Clear_Protocol();    
    Operator = "guest";


    current_UNDO = -1;
    //count_UNDO = 0;

    MainGroupBox = new QGroupBox();
    MainGroupBox->setObjectName("Transparent");
    setCentralWidget(MainGroupBox);
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(5);
    main_layout->setSpacing(4);
    MainGroupBox->setLayout(main_layout);

    main_spl = new QSplitter(Qt::Horizontal, this);
    main_spl->setHandleWidth(6);
    main_spl->setChildrenCollapsible(false);
    //main_spl->setStyleSheet("QSplitter::handle {background-color: rgba(40,40,40,230);}"
    //                        "QSplitter::handle:hover {background-color: rgba(240,240,240,230);}");

    //main_spl->setStyleSheet("QSplitter::handle:vertical   {height: 6px; image: (:/images/vsplitter.png);}");

    main_layout->addWidget(main_spl);

    setContextMenuPolicy(Qt::NoContextMenu);

    createActions();
    createToolBars();    
    create_SampleProgramPlate();

    cursor_FREE = QCursor(QPixmap(":/images/cursor_free.png"));
    cursor_ColorFill = QCursor(QPixmap(":/images/cursor_ColorFill.png"));

    //... Apply Settings ...    
    QString dir_path = qApp->applicationDirPath();
    ApplSettings = new QSettings(dir_path + "/tools/ini/preference_setup.ini", QSettings::IniFormat);
    readSettings();

    //... Create Names Research ...    
    create_NamesResearch();

    //... Load TESTs ...
    add_tests = new Add_TESTs(this);
    add_tests->TESTs = &TESTs;
    add_tests->map_research = &Map_Research;
    add_tests->map_TestTranslate = &Map_TestTranslate;
    connect(add_tests, SIGNAL(signal_addTest(rt_Test*,int,int,int,int,int,int)), this, SLOT(add_test(rt_Test*,int,int,int,int,int,int)));
    connect(add_tests, SIGNAL(signal_addSample(QString,QVector<rt_Test*>*,int,int)), this, SLOT(add_ForSample(QString,QVector<rt_Test*>*,int,int)));
    connect(this, SIGNAL(Operation_Complited()), add_tests, SLOT(CheckState_AddButton()));

    //... Load Test_Editor ...
    //test_editor = new Test_editor(this);
    //test_editor->TESTs = &TESTs;

    connect(main_spl, SIGNAL(splitterMoved(int,int)), this, SLOT(resize_splitter(int,int)));

    /*qApp->setStyleSheet(
        "QSplitter::handle:vertical   {height: 6px; image: url(:/images/vsplitter.png);}"
        "QSplitter::handle:horizontal {width:  6px; image: url(:/images/hsplitter.png);}"
        );*/

    dll_editor = NULL;
    ax_user = NULL;
    axgp = NULL;
    ax_rita = NULL;

    message.setWindowIcon(QIcon(":/images/DTm.ico"));
    //message.setWindowIcon(QIcon(":/images/RT.ico"));
    message.setFont(qApp->font());

    msgbox_ToAnalysis.setWindowIcon(QIcon(":/images/DTm.ico"));
    //msgbox_ToAnalysis.setWindowIcon(QIcon(":/images/RT.ico"));
    msgbox_ToAnalysis.setText(tr("You are trying to change the analysis of current protocol! Continue?"));
    msgbox_ToAnalysis.setFont(qApp->font());

    msgbox_ToAnalysis.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgbox_ToAnalysis.setIcon(QMessageBox::Warning);
    msgbox_ToAnalysis.setDefaultButton(QMessageBox::Cancel);
    cb_ToAnalysis = new QCheckBox(tr("Don't show again"), this);
    msgbox_ToAnalysis.setCheckBox(cb_ToAnalysis);
    ToAnalysis = false;
    connect(cb_ToAnalysis, SIGNAL(stateChanged(int)), this, SLOT(change_ToAnalysis(int)));

    label_gif = new QLabel(this);
    label_gif->setFixedSize(48,48);
    label_gif->raise();
    obj_gif = new QMovie(":/images/flat/wait_1.gif");
    label_gif->setMovie(obj_gif);
    label_gif->setVisible(false);

    splash = NULL;



#ifdef QT_DEBUG
    dll_editor = ::LoadLibraryW(L"editor_rtd.dll");
#else    
    dll_editor = ::LoadLibraryW(L"editor_rt.dll");    

#endif

    // Style:
    if(StyleApp.trimmed() == "fusion")
    {
        //MainGroupBox->setStyleSheet("QGroupBox#Transparent {border: 1px solid transparent;}");

    }

#ifndef TIS
    send_protocol_to_web->setVisible(false);
    look_TIS_Tests->setVisible(false);
#endif

    Clear_UNDO();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Setup::~Setup()
{
    writeSettings();
    delete ApplSettings;

    if(dll_editor) ::FreeLibrary(dll_editor);

    delete obj_gif;
    delete label_gif;

    //delete LoadToRun;
    delete cb_ToAnalysis;
    delete LabelTests_ADDSample;

    delete Samples_Delegate->nameTest;
    delete Samples_Delegate;

    delete plot_Program;
    delete scheme_Program;
    delete program_spl;
    delete Samples_Table;
    delete TabSamples;
    delete Type_Plate;

    delete MainProgramWindow;
    delete MainPlateWindow;
    delete MainListWindow;
    delete MainControlWindow;

    delete MainGroupBox;

    Tubes_key.clear();
    Tubes_kind.clear();
    Map_Research.clear();
    Map_Settings.clear();

    //delete test_editor;

    delete add_tests;
    qDeleteAll(TESTs.begin(),TESTs.end());
    TESTs.clear();
    qDeleteAll(Pro_TESTs.begin(),Pro_TESTs.end());
    Pro_TESTs.clear();

    Map_TestTranslate.clear();

    prot->Clear_Protocol();
    delete prot;

}
//-----------------------------------------------------------------------------
//--- create_NamesResearch()
//-----------------------------------------------------------------------------
void Setup::create_NamesResearch()
{
    QVector<QString> info_test;
    QString fn;
    QFileInfo fi;
    QString text,str;
    int id;
    bool ok;
    int ID_base = 0x20000;
    int method_ID;
    QByteArray ba;
    quint16 crc16;
    HINSTANCE dll_plugin;
    Test_Interface  *alg_editor;
    char *Settings;
    char *DisplayName;
    char *Path;
    int release, build;
    QString dir = qApp->applicationDirPath() + "/forms";
    QString version = "9.0";
    QFont f = qApp->font();

    //qDebug() << "font (set_font): " << f.family() << f.style() << f.pointSize() << f.pixelSize();


    // 1. My Plugins (tests plugins)
    QDir pluginsDir(qApp->applicationDirPath());
    pluginsDir.cd("test_plugins");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files))
    {
        dll_plugin = NULL;

        fn = "test_plugins\\" + fileName;

        dll_plugin = ::LoadLibraryW(fn.toStdWString().c_str());

        if(dll_plugin)
        {
            InterfaceTest_factory factory_func = reinterpret_cast<InterfaceTest_factory>(
                                        ::GetProcAddress(dll_plugin,"createTest_plugin@0"));
            alg_editor = factory_func();
            if(alg_editor)
            {
                alg_editor->GetInformation(&info_test);
                id = info_test.at(0).toInt(&ok,16);
                Map_Research.insert(id, info_test.at(1));
                delete alg_editor;
            }
            alg_editor = NULL;
            info_test.clear();

            ::FreeLibrary(dll_plugin);
        }
    }

    // 2. External Plugins (dtr modules)
    pluginsDir.cd("..");
    pluginsDir.cd("forms");

    ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");    
    if(ext_dll_handle)
    {
        initialize = (Init)(::GetProcAddress(ext_dll_handle,"Initialize"));
        set_font = (SetFont)(::GetProcAddress(ext_dll_handle,"SetFont"));
        get_Info = (GetDTRDisplayInfo)(::GetProcAddress(ext_dll_handle,"GetDTRDisplayInfo"));
        get_Settings = (GetSampleSettings)(::GetProcAddress(ext_dll_handle,"GetSampleSettings"));        
        if(get_Info && initialize)
        {            
            initialize(NULL, dir.toLatin1().data(), version.toLatin1().data(), ID_LANG, NULL,NULL,NULL,NULL,NULL,NULL);

            if(set_font) set_font(f.family().toLatin1().data(), "", f.pointSize());

            foreach (QString fileName, pluginsDir.entryList(QDir::Files))
            {
                fi.setFile(pluginsDir, fileName);
                if(fi.exists() && fi.suffix() == "dtr")
                {
                    text = fi.baseName();
                    if(text == "Default" || text == "ReportSummary" || text == "ReportTitle" || text == "DTReport Studio project") continue;
                    ba.clear();
                    ba.append(text.toLatin1());
                    crc16 = qChecksum(ba.data(),ba.size());
                    method_ID = ID_base + crc16;                    
                    get_Info(text.toLatin1().data(), &DisplayName, &Path, &release, &build);                    
                    text = QString::fromUtf8(DisplayName);                    
                    if(text.trimmed().isEmpty()) continue;
                    Map_Research.insert(method_ID, text);

                    text = fi.baseName();
                    if(get_Settings)
                    {
                        get_Settings(text.toLatin1().data(), &Settings);
                        text = QString::fromUtf8(Settings);
                        if(!text.isEmpty())
                        {
                            str = GroupSettings_XML2String(text);
                            if(!str.isEmpty()) Map_Settings.insert(method_ID, str);
                        }
                    }
                }
            }
        }

        ::FreeLibrary(ext_dll_handle);
    }

    qDebug() << "Map_Settings: " << Map_Settings;

}
//-----------------------------------------------------------------------------
//--- GroupSettings_XML2String(QString xml)
//-----------------------------------------------------------------------------
QString Setup::GroupSettings_XML2String(QString xml)
{
    int i,j;
    QDomDocument doc;
    QDomElement  root;
    QDomElement  child;
    QDomNode     item;

    QStringList list;
    list << "name" << "DisplayName" << "edType" << "value" << "Values";

    QString text = "";

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");

    doc.setContent(xml);
    doc.insertBefore(xmlNode, doc.firstChild());

    root = doc.documentElement();

    for(i=0; i<root.childNodes().size(); i++)
    {
        if(!text.isEmpty()) text += "#13#10";

        item = root.childNodes().at(i);
        for(j=0; j<item.childNodes().size(); j++)
        {
            if(j >= list.size()) break;
            child = item.firstChildElement(list.at(j));

            if(!child.isNull()) text += child.text() + "||";
            else text += " ||";
        }
    }


    return(text);
}

//-----------------------------------------------------------------------------
//--- readTemplateSettings()
//-----------------------------------------------------------------------------
void Setup::readTemplateSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *TemplateNameSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    TemplateNameSettings->beginGroup("Template_NameProtocol");

    template_NAME.name = tr("Protocol");
    template_NAME.type = true;
    template_NAME.date_time = ""; //"ddMMyy_hhmmss";

    text = TemplateNameSettings->value("use_template","no").toString();
    if(text.startsWith("yes", Qt::CaseInsensitive))
    {
        template_NAME.name = TemplateNameSettings->value("name_template","").toString();
        if(TemplateNameSettings->value("use_type","no").toString() == "yes") template_NAME.type = true;
        else template_NAME.type = false;
        template_NAME.date_time = TemplateNameSettings->value("date_template","").toString();
    }

    TemplateNameSettings->endGroup();
    delete TemplateNameSettings;

}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Setup::readCommonSettings()
{
    QString text;
    QString dir_SysTranslate;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    ID_LANG = ID_LANG_RU;

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/setup_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);            
        }
        if(text == "en") ID_LANG = ID_LANG_EN;

        // ... System Language ...
        dir_SysTranslate = dir_path + "/tools/translations";
        //dir_SysTranslate = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

        //qDebug() << "Setup System Language: " << text << dir_SysTranslate;
        //qDebug() << "Location translate: " << QLibraryInfo::location(QLibraryInfo::TranslationsPath);

        if(qt_translator.load("qt_" + text, dir_SysTranslate))
        {
            qApp->installTranslator(&qt_translator);
            qDebug() << "Ok Setup System Language";
        }

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;

    logo = QIcon("logotype.png");
    logonull = QIcon("logotype_null.png");
}

//-----------------------------------------------------------------------------
//--- addProtocol_toList
//-----------------------------------------------------------------------------
void Setup::addProtocol_toList(QString fn)
{
    int i,k;
    QString text;
    int count_border = 10;
    QStringList list_fn;
    QFileInfo fi;

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
//--- addProgram_toList
//-----------------------------------------------------------------------------
void Setup::addProgram_toList(QString fn)
{
    int i,k;
    QString text;
    int count_border = 10;
    QStringList list_fn;
    QFileInfo fi;

    ApplSettings->beginGroup("LastPrograms");

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
    ApplSettings->remove("LastPrograms");

    ApplSettings->beginGroup("LastPrograms");
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
//--- execution_MenuAction
//-----------------------------------------------------------------------------
void Setup::execution_MenuAction(QString type)
{
    if(type == "open") {open_protocol(); return;}
    if(type == "save") {save_protocol(); return;}
    if(type == "clear") {clear_protocol(); return;}
    if(type == "to_run") {load_torun(); return;}
    if(type == "to_analysis") {load_toanalysis(); return;}
    if(type == "edit_test") {edit_tests(); return;}
    if(type == "edit_protocol") {edit_preferencePro(); return;}
    if(type == "copy_block_tests") {copy_block_tests(); return;}
    if(type == "open_xml_7ver") {open_XMLprotocol(); return;}
}

//-----------------------------------------------------------------------------
//--- Create_MenuActions(void *)
//-----------------------------------------------------------------------------
void Setup::Create_MenuActions(void *list_pinfo)
{
    QStringList *list = (QStringList*)list_pinfo;

    //qDebug() << "Create_MenuActions: " << *list;

    list->append(open_Protocol->toolTip() + "\t" + "open" + "\t" + "1");
    list->append(open_XML->toolTip() + "\t" + "open_xml_7ver" + "\t" + "1" );
    list->append(save_Protocol->toolTip() + "\t" + "save" + "\t" + "1");
    list->append(clear_Protocol->toolTip() + "\t" + "clear" + "\t" + "1");
    list->append("...");
    list->append(load_ToRun->toolTip() + "\t" + "to_run" + "\t" + "1");
    list->append(load_ToAnalysis->toolTip() + "\t" + "to_analysis" + "\t" + QString::number((int)flag_ReOpenProtocol));
    list->append("...");

    list->append(copy_BlockTests->toolTip() + "\t" + "copy_block_tests" + "\t" + QString::number((int)copy_BlockTests->isEnabled()));
    list->append(edit_Test->toolTip() + "\t" + "edit_test" + "\t" + QString::number((int)edit_Test->isEnabled()));
    list->append(edit_PreferenceProtocol->toolTip() + "\t" + "edit_protocol" + "\t" + QString::number((int)edit_PreferenceProtocol->isEnabled()));
}
//-----------------------------------------------------------------------------
//--- Enable_MenuAction(void *)
//-----------------------------------------------------------------------------
void Setup::Enable_MenuAction(void *map_enable)
{
    QMap<QString, int> *map = (QMap<QString, int>*)map_enable;

    map->insert("open", open_Protocol->isEnabled());
    //map->insert("open_xml_7ver", open_XML->isEnabled());
    map->insert("open_xml_7ver", open_Protocol->isEnabled());
    map->insert("save", save_Protocol->isEnabled());
    map->insert("clear", clear_Protocol->isEnabled());
    map->insert("to_run", load_ToRun->isEnabled());
    map->insert("to_analysis", load_ToAnalysis->isEnabled());
    map->insert("copy_block_tests", copy_BlockTests->isEnabled());
    map->insert("edit_test", edit_Test->isEnabled());
    map->insert("edit_protocol", edit_PreferenceProtocol->isEnabled());

    //qDebug() << "Enable_Menu: edit_test-> " << edit_Test->isEnabled();

}
//-----------------------------------------------------------------------------
//--- CreateCopy_Test
//-----------------------------------------------------------------------------
void* Setup::Get_pProtocol()
{
    void *p;

    p = (void*)prot;
    //qDebug() << "get pPro:" << prot << p;

    return(p);
}
//-----------------------------------------------------------------------------
//--- CreateCopy_Test
//-----------------------------------------------------------------------------
void Setup::CreateCopy_Test(rt_Test *source_test, rt_Test *target_test)
{
    int i;
    QString text = "";
    rt_Preference *pre;
    QDomDocument doc;
    QDomElement  root;

    for(i=0; i<source_test->preference_Test.size(); i++)
    {
        pre = source_test->preference_Test.at(i);
        if(pre->name == "xml_node")
        {
            text = QString::fromStdString(pre->value);
            break;
        }
    }

    //qDebug() << "CreateCopy_Test: " << text;


    doc.setContent(text);

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    root = doc.documentElement();

    LoadXML_Test(root, target_test);

    // copy info about Signature
    target_test->header.Hash = source_test->header.Hash;

}

//-----------------------------------------------------------------------------
//--- create_TestDBase
//-----------------------------------------------------------------------------
bool Setup::create_TestDBase(QAxObject *axRita_resource)
{
    ax_rita = axRita_resource;

    qDebug() << "ax_rita: " << ax_rita;

    return(true);
}


//-----------------------------------------------------------------------------
//--- create_User
//-----------------------------------------------------------------------------
bool Setup::create_TestVerification(QAxObject *axgp_resource)
{
    axgp = axgp_resource;

    qDebug() << "axgp: " << axgp;

    return(true);
}

//-----------------------------------------------------------------------------
//--- create_User
//-----------------------------------------------------------------------------
bool Setup::create_User(QAxObject *user)
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

                path = QString("%1/Placement_Templates").arg(str);
                templates_Dir.setPath(path);
                if(!templates_Dir.exists()) templates_Dir.mkdir(templates_Dir.absolutePath());

                break;
            }
        }
    }


    // set user properties
    flag_EditTest = false;
    flag_CommonSettings = false;
    flag_CopyBlockTests = false;
    flag_ReOpenProtocol = false;
    flag_EditProtocol = false;
    flag_SaveLocationWin = false;
    if(ax_user)
    {
        flag_EditTest = ax_user->dynamicCall("getPriv(QString)", "EDIT_TEST").toBool();
        flag_CommonSettings = ax_user->dynamicCall("getPriv(QString)", "CHANGE_ASYS_PREF").toBool();
        flag_CopyBlockTests = ax_user->dynamicCall("getPriv(QString)", "COPY_BLOCK_TEST").toBool();
        flag_EditProtocol = ax_user->dynamicCall("getPriv(QString)", "EDIT_PROTOCOL").toBool();
        flag_ReOpenProtocol = ax_user->dynamicCall("getPriv(QString)", "MASK_DATA").toBool();
        flag_SaveLocationWin = ax_user->dynamicCall("getPriv(QString)", "SAVE_LOCATION_WIN").toBool();

        temp = ax_user->dynamicCall("getAttr(QString)","dir:PR_HOME").toString().trimmed();
        if(!temp.isEmpty() && QDir(temp).exists()) user_Dir.setPath(temp);
        else
        {
            if(user_Dir.exists())
            {
                QVariant var_dir(user_Dir.absolutePath());
                ax_user->dynamicCall("setAttr(QString,QString)","dir:PR_HOME",var_dir);
            }
        }


        //qDebug() << "user_Dir: " << temp << user_Dir;
    }

    edit_Test->setEnabled(flag_EditTest);
    edit_PreferenceProtocol->setEnabled(flag_CommonSettings);
    copy_BlockTests->setEnabled(flag_CopyBlockTests);
    load_ToAnalysis->setEnabled(flag_ReOpenProtocol);

    //... ToolTip ...
    tooltip = tr("Edit Test"); if(!flag_EditTest) tooltip += tr(" (unavailable)");
    edit_Test->setToolTip(tooltip);
    tooltip = tr("Edit protocol preference"); if(!flag_CommonSettings) tooltip += tr(" (unavailable)");
    edit_PreferenceProtocol->setToolTip(tooltip);
    tooltip = tr("Copy block tests"); if(!flag_CopyBlockTests) tooltip += tr(" (unavailable)");
    copy_BlockTests->setToolTip(tooltip);
    tooltip = tr("Load to Analysis"); if(!flag_ReOpenProtocol) tooltip += tr(" (unavailable)");
    load_ToAnalysis->setToolTip(tooltip);

    //qDebug() << "flags(S): " << ax_user << flag_EditTest << flag_CommonSettings << flag_CopyBlockTests << flag_EditProtocol << flag_ReOpenProtocol;

    //test_editor->tool_editor->setDisabled(!flag_EditTest);
    //if(!flag_EditTest) test_editor->tests_list->setContextMenuPolicy(Qt::NoContextMenu);

    return(true);
}

//-----------------------------------------------------------------------------
//--- create_ListTESTs
//-----------------------------------------------------------------------------
bool Setup::create_ListTESTs(QString fn)
{
    int i,j;
    bool res = false;
    int count_tests;

    QDomDocument    doc;
    QDomElement     root;
    QDomNode        child;
    rt_Test         *p_test;
    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;
    QFile file(fn);

    QStringList list;

    QString text, sts;
    QString xml_str;
    bool verification;

    rt_Preference   *preference_test;

    if(splash) connect(this, SIGNAL(Send_SplashPercent(QString)), splash, SLOT(Get_SplashPercent(QString)));
    //qDebug() << "create_ListTESTs:  Start " << QTime::currentTime();

    qDeleteAll(TESTs.begin(),TESTs.end());
    TESTs.clear();

    Map_TestTranslate.clear();

    Simple_Test = new SIMPLE_TEST();

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            TESTs.insert(0,Simple_Test);
            return(res);
        }        

        root = doc.documentElement();
        if(root.nodeName() == "TESTs")
        {
            if(root.hasAttribute("operator"))
            {
                Operator = root.attribute("operator","guest");
            }            

            count_tests = root.childNodes().size();
            for(i=0; i<root.childNodes().size(); i++)
            {
                qApp->processEvents();

                child = root.childNodes().at(i);
                p_test = new rt_Test();
                LoadXML_Test(child, p_test);
                TESTs.push_back(p_test);

                //qDebug() << "tests: " << i;
                emit Send_SplashPercent(QString("%1,%2").arg(i+1).arg(count_tests));

                //... Check&Load Catalog Property ...
                Load_CatalogProperties(p_test);

                //... create Translate_Service ...
                if(p_test->header.Type_analysis >= 0x20000 && ID_LANG != ID_LANG_RU)
                {
                    Create_TranslateList(p_test, list);
                }                
                //...................................

                text = QString::fromStdString(p_test->header.Name_Test);
                    for(j=0; j<p_test->preference_Test.size(); j++)
                    {
                        qApp->processEvents();

                        preference_test = p_test->preference_Test.at(j);
                        if(preference_test->name == "xml_node")
                        {
                            xml_str = QString::fromStdString(preference_test->value);
                            //qDebug() << "xml_node: " << xml_str;
                            verification = false;
                            if(axgp)
                            {
                                verification = axgp->dynamicCall("loadXmlTest(QString)", xml_str).toBool();
                                //qDebug() << "ver: " << verification << text;
                            }
                            if(verification) sts = "ok";
                            else sts = "none";
                            p_test->header.Hash = sts.toStdString();
                            //qDebug() << "ver: " << QString::fromStdString(p_test->header.Hash);
                            break;
                        }
                    }
            }

            //qDebug() << "list translate:" << list;
            if(list.size()) Translate_Tests(list);      // Translate names and research
            //qDebug() << "Translate: " << Map_TestTranslate;

        }

        file.close();
    }

    TESTs.insert(0,Simple_Test);    


    //... Load DropperTableTests ...
    Load_DropperTableTests();
    //..............................

    //... Translate Catalog ...
    Translate_Catalog(&TESTs);
    //...

    //qDebug() << "create_ListTESTs: Finish " << QTime::currentTime();
    if(splash) disconnect(this, SIGNAL(Send_SplashPercent(QString)), splash, SLOT(Get_SplashPercent(QString)));
    splash = NULL;

    if(TESTs.size()) res = true;
    return(res);
}
//-----------------------------------------------------------------------------
//--- Create_TranslateList(rt_Test *, QStringList &)
//-----------------------------------------------------------------------------
void Setup::Create_TranslateList(rt_Test *p_test, QStringList &list)
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
//-------------------------------------------------------------------------------
//--- Translate_Catalog()
//-------------------------------------------------------------------------------
void Setup::Translate_Catalog(QVector<rt_Test*> *tests)
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
void Setup::Translate_Tests(QStringList &list)
{
    QString text, str = "";
    char *p;
    QByteArray ba;
    QStringList list_in, list_out;
    int id;
    QString key, value;

    foreach(text, list)
    {
        if(!Map_TestTranslate.keys().contains(text))
        {
            if(str.length()) str += "\r\n";
            str += text;
        }
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
//--- Load_CatalogProperties()
//-----------------------------------------------------------------------------
void Setup::Load_CatalogProperties(rt_Test *ptest)
{
    rt_Preference   *preference_test;
    int method;
    QString name_Method;    

    method = ptest->header.Type_analysis;
    name_Method = Map_Research.value(method, tr("Unknown"));

    if(ptest->header.Catalog.empty() || name_Method == tr("Unknown")) ptest->header.Catalog = name_Method.toStdString();

    //qDebug() << "Catalog: " << QString::fromStdString(ptest->header.Name_Test) << QString::fromStdString(ptest->header.Catalog);


}

//-----------------------------------------------------------------------------
//--- Sample & Program & Plate
//-----------------------------------------------------------------------------
void Setup::create_SampleProgramPlate()
{


    //---
    create_SampleBlock();

    //---
    MainControlWindow = new QMainWindow();
    ControlBox = new QGroupBox(MainControlWindow);
    ControlBox->setObjectName("Transparent");
    ControlBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    ControlBox->setMinimumWidth(200);
    //ControlBox->setStyleSheet("border:0;");
    MainControlWindow->setCentralWidget(ControlBox);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(5);
    layout->setSpacing(4);
    ControlBox->setLayout(layout);

    program_plate_spl = new QSplitter(Qt::Vertical);
    program_plate_spl->setHandleWidth(6);
    //QString splitterSheet = "QSplitter::handle {image: url(:/images/vsplitter.png);}";
    //program_plate_spl->setStyleSheet(splitterSheet);
    layout->addWidget(program_plate_spl);

    create_ProgramBlock();
    create_PlateBlock();

    program_plate_spl->addWidget(MainProgramWindow);
    program_plate_spl->addWidget(MainPlateWindow);
    //---

    main_spl->addWidget(MainListWindow);
    main_spl->addWidget(MainControlWindow);
}
//-----------------------------------------------------------------------------
//--- Sample Block
//-----------------------------------------------------------------------------
void Setup::create_SampleBlock()
{

    MainListWindow = new QMainWindow();
    ListSampleBox = new QGroupBox(MainListWindow);
    ListSampleBox->setObjectName("Transparent");
    ListSampleBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    ListSampleBox->setMinimumWidth(200);
    //ListSampleBox->setStyleSheet("border:0;");
    MainListWindow->setCentralWidget(ListSampleBox);
    MainListWindow->setContextMenuPolicy(Qt::NoContextMenu);

    sampleToolBar = new QToolBar(tr("Samples"), MainListWindow);
    MainListWindow->addToolBar( Qt::BottomToolBarArea,sampleToolBar); // (tr("Samples"));

    LabelTests_ADDSample = new QLabel("", MainListWindow);
    LabelTests_ADDSample->setStyleSheet("QLabel {background-color: rgba(250, 250, 250, 220); color: red;}");
    LabelTests_ADDSample->raise();
    LabelTests_ADDSample->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    LabelTests_ADDSample->setAlignment(Qt::AlignCenter);
    LabelTests_ADDSample->setVisible(false);
    ADD_Sample = new Add_PushButton(QIcon(":/images/add_sample.png"), "", this);
    ADD_Sample->setFixedSize(65,65);
    ADD_Sample->setToolTip(tr("Add Samples/Tests"));
    ADD_Sample->user_Tests = true;
    if(StyleApp == "fusion") ADD_Sample->setIcon(QIcon(":/images/flat/add_samples_flat.png"));
    if(StyleApp == "fusion") ADD_Sample->setIconSize(QSize(32,32));
    ADD_Tube = new QPushButton(QIcon(":/images/add_tube.png"), "", this);
    ADD_Tube->setFixedSize(65,65);
    ADD_Tube->setToolTip(tr("Add tube"));
    if(StyleApp == "fusion") ADD_Tube->setIcon(QIcon(":/images/flat/add_tube_flat.png"));
    if(StyleApp == "fusion") ADD_Tube->setIconSize(QSize(32,32));
    DELETE_Sample = new QPushButton(QIcon(":/images/delete_sample.png"), "", this);
    DELETE_Sample->setFixedSize(65,65);
    DELETE_Sample->setToolTip(tr("Delete sample"));
    if(StyleApp == "fusion") DELETE_Sample->setIcon(QIcon(":/images/flat/delete_flat.png"));
    if(StyleApp == "fusion") DELETE_Sample->setIconSize(QSize(32,32));
    DELETE_AllSamples = new QPushButton(QIcon(":/images/clear_16.png"), "", this);
    DELETE_AllSamples->setFixedSize(65,65);
    DELETE_AllSamples->setToolTip(tr("Delete all samples"));
    if(StyleApp == "fusion") DELETE_AllSamples->setIcon(QIcon(":/images/flat/clear_flat_32.png"));
    if(StyleApp == "fusion") DELETE_AllSamples->setIconSize(QSize(32,32));
    UNDO_Action = new QPushButton(QIcon(":/images/undo.png"), "", this);
    if(StyleApp == "fusion") UNDO_Action->setIcon(QIcon(":/images/flat/undo_flat.png"));
    if(StyleApp == "fusion") UNDO_Action->setIconSize(QSize(32,32));
    UNDO_Action->setFixedSize(65,65);
    UNDO_Action->setToolTip(tr("Undo"));

    connect(ADD_Sample, SIGNAL(clicked(bool)), this, SLOT(add_sample()));
    connect(ADD_Tube, SIGNAL(clicked(bool)), this, SLOT(add_tube()));
    connect(DELETE_Sample, SIGNAL(clicked(bool)), this, SLOT(delete_sample()));
    connect(DELETE_AllSamples, SIGNAL(clicked(bool)), this, SLOT(delete_all_samples()));
    connect(UNDO_Action, SIGNAL(clicked(bool)), this, SLOT(undo()));

    sampleToolBar->addWidget(ADD_Sample);
    sampleToolBar->addWidget(ADD_Tube);

    sampleToolBar->addWidget(DELETE_Sample);
    sampleToolBar->addWidget(DELETE_AllSamples);

    sampleToolBar->addWidget(UNDO_Action);

    DELETE_Sample->setEnabled(false);
    DELETE_AllSamples->setEnabled(false);
    UNDO_Action->setEnabled(false);


    QVBoxLayout *layout = new QVBoxLayout;
    ListSampleBox->setLayout(layout);
    layout->setMargin(2);

    TabSamples = new QTabWidget(MainListWindow);    
    layout->addWidget(TabSamples);

    Samples_Table = new SampleTableWidget(0,0,MainListWindow);
    Samples_Table->setFont(qApp->font());

    //Samples_Table->setSelectionBehavior(QAbstractItemView::SelectItems);
    Samples_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    Samples_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    //Samples_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Samples_Table->setContextMenuPolicy(Qt::CustomContextMenu);
    Samples_Delegate = new SamplesItemDelegate;
    Samples_Delegate->nameTest = new QString("");
    Samples_Delegate->current_row = -1;
    Samples_Table->setItemDelegate(Samples_Delegate);
    Samples_Delegate->list_ID = &list_ID;
    Samples_Delegate->list_NAME = &list_NAME;    
    Samples_Delegate->enable_editor = true;
    Samples_Delegate->style = StyleApp;
    Samples_Delegate->open_sample = &Samples_Table->open_samples;
    Samples_Delegate->disable_Standarts = true;
    //Samples_Delegate->list_editors = new QVector<QWidget*>();

    connect(Samples_Table->corner_btn, SIGNAL(clicked(bool)), this, SLOT(OpenCloseSamples()));
    connect(Samples_Table, SIGNAL(itemSelectionChanged()), this, SLOT(cellSample_Selected()));    
    connect(Samples_Table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_SamplesTable()));
    connect(Samples_Delegate, SIGNAL(change_Type()), this, SLOT(ChangeTypeSample()));
    connect(Samples_Delegate, SIGNAL(change_Copies()), this, SLOT(ChangeCopiesSample()));
    connect(Samples_Delegate, SIGNAL(change_NameSample()), this, SLOT(ChangeNameSample()));

    TestSampleBox = new QGroupBox(MainListWindow);    
    test_sample_spl = new QSplitter(Qt::Horizontal, MainListWindow);
    QHBoxLayout *layout_testsample = new QHBoxLayout;
    layout_testsample->setMargin(0);
    TestSampleBox->setLayout(layout_testsample);
    test_sample_spl->setHandleWidth(6);
    layout_testsample->addWidget(test_sample_spl);
    Test_Tree = new QTreeWidget(MainListWindow);
    Test_Tree->setContextMenuPolicy(Qt::CustomContextMenu);
    Test_Tree->setHeaderLabel(tr("Tests"));
    Sample_Tree = new QTreeWidget(MainListWindow);
    Sample_Tree->setFont(qApp->font());
    Sample_Tree->setHeaderLabel(tr("Sample"));
    Test_Tree->setFont(qApp->font());
    Samples_EditorDelegate = new SampleEditorDelegate(this);
    Samples_EditorDelegate->enable_editor = true;
    Sample_Tree->setItemDelegate(Samples_EditorDelegate);
    connect(Sample_Tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(SampleItemChange(QTreeWidgetItem*,int)));
    test_sample_spl->addWidget(Test_Tree);
    test_sample_spl->addWidget(Sample_Tree);

    connect(Test_Tree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_Test_Tree()));

    Info_Tree = new Info_Protocol(MainListWindow);
    Info_Tree->setFont(qApp->font());
    Info_Tree->setHeaderLabel(tr(""));

    Sample_Properties = new QTreeWidget(MainListWindow);
    Sample_Properties->setContextMenuPolicy(Qt::CustomContextMenu);
    Sample_Properties->setFont(qApp->font());
    property_Delegate = new PropertyDelegate(this);
    Sample_Properties->setItemDelegate(property_Delegate);
    Sample_Properties->setHeaderLabel(tr(""));

    connect(Sample_Properties, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(PropertyItemChange(QTreeWidgetItem*,int)));
    connect(Sample_Properties, &QTreeWidget::customContextMenuRequested, this, &Setup::contextMenu_SampleProperties);

    TabSamples->addTab(Samples_Table,tr("List of Samples"));
    TabSamples->addTab(TestSampleBox,tr("Test&&Sample"));
    TabSamples->addTab(Info_Tree,tr("Information"));
    TabSamples->addTab(Sample_Properties,tr("Information about Samples"));

}

//-----------------------------------------------------------------------------
//--- Program Block
//-----------------------------------------------------------------------------
void Setup::create_ProgramBlock()
{
    QFont f = qApp->font();

    MainProgramWindow = new QMainWindow();
    ProgramBox = new QGroupBox(MainProgramWindow);
    ProgramBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    MainProgramWindow->setCentralWidget(ProgramBox);
    MainProgramWindow->setContextMenuPolicy(Qt::NoContextMenu);

    programToolBar = MainProgramWindow->addToolBar("Program");

    Label_Program = new QLabel(tr("Program:"), this);
    Name_Program = new QLabel("", this);
    f.setBold(true);
    Name_Program->setFont(f);
    QWidget *spacer = new QWidget(this);
    spacer->setFixedWidth(10);
    programToolBar->addWidget(Label_Program);
    programToolBar->addWidget(spacer);
    programToolBar->addWidget(Name_Program);
    spacer = new QWidget(this);
    //spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    spacer->setFixedWidth(25);
    programToolBar->addWidget(spacer);
    programToolBar->addSeparator();
    programToolBar->addAction(open_Program);
    programToolBar->addAction(last_Programs);
    programToolBar->addAction(edit_Program);
    programToolBar->setIconSize(QSize(16,16));

    //QHBoxLayout *layout_toolbar = new QHBoxLayout;
    //programToolBar->setLayout(layout_toolbar);
    //QLabel *label = new QLabel("Program");
    //layout_toolbar->addWidget(label,0,Qt::AlignHCenter);
    //label->setVisible(true);
    //programToolBar->addWidget(label);

    QWidget *p = programToolBar->widgetForAction(last_Programs);
    p->setMaximumWidth(13);
    //programToolBar->setIconSize(QSize(20,20));


    QHBoxLayout *layout_program = new QHBoxLayout;
    ProgramBox->setLayout(layout_program);
    program_spl = new QSplitter(Qt::Horizontal, this);
    program_spl->setHandleWidth(6);
    program_spl->setChildrenCollapsible(false);
    layout_program->addWidget(program_spl);
    plot_Program = new Plot_ProgramAmpl(this);
    scheme_Program = new Scheme_ProgramAmpl(QColor(1,1,1,1), this);

    layout_program->setSpacing(2);
    layout_program->setMargin(2);
    program_spl->addWidget(plot_Program);
    program_spl->addWidget(scheme_Program);
}
//-----------------------------------------------------------------------------
//--- Plate Block
//-----------------------------------------------------------------------------
void Setup::create_PlateBlock()
{
    QStringList name_dev;
    //name_dev  = QString(List_DEVICENAME).split(",");
    name_dev << tr("DTprime_96") << tr("DTprime_384") << tr("DTlime_48") << tr("DTlime_192");
    QFont f = qApp->font();
    QPalette palette;

    MainPlateWindow = new QMainWindow();
    PlateBox = new QGroupBox(MainPlateWindow);
    PlateBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);    
    MainPlateWindow->setCentralWidget(PlateBox);
    MainPlateWindow->setContextMenuPolicy(Qt::NoContextMenu);

    plateToolBar = MainPlateWindow->addToolBar(tr("Plate"));

    label_Plate = new QLabel(tr("ThermoBlock:  "), this);
    plateToolBar->addWidget(label_Plate);
    type_plate = new QComboBox(this);
    type_plate->addItems(name_dev);
    type_plate->setCurrentIndex(-1);    
    /*if(StyleApp == "fusion")*/ type_plate->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    f.setBold(false);
    type_plate->setFont(f);

    plateToolBar->addWidget(type_plate);


    //...
    spacer_fix = new QWidget(this);
    spacer_fix->setFixedWidth(20);
    plateToolBar->addWidget(spacer_fix);

    Color_Group = new QButtonGroup(this);
    Color_Group->setExclusive(true);

    first_ColorButton = new Color_ToolButton(QColor(255,0,0), this);
    second_ColorButton = new Color_ToolButton(QColor(255,128,0), this);
    third_ColorButton = new Color_ToolButton(QColor(255,255,0), this);
    fourth_ColorButton = new Color_ToolButton(QColor(0,255,0), this);
    fifth_ColorButton = new Color_ToolButton(QColor(0,128,255), this);
    sixth_ColorButton = new Color_ToolButton(QColor(0,0,255), this);
    seventh_ColorButton = new Color_ToolButton(QColor(255,0,255), this);

    first_ColorButton->setCheckable(true);
    second_ColorButton->setCheckable(true);
    third_ColorButton->setCheckable(true);
    fourth_ColorButton->setCheckable(true);
    fifth_ColorButton->setCheckable(true);
    sixth_ColorButton->setCheckable(true);
    seventh_ColorButton->setCheckable(true);

    Color_Group->addButton(first_ColorButton,0);
    Color_Group->addButton(second_ColorButton,1);
    Color_Group->addButton(third_ColorButton,2);
    Color_Group->addButton(fourth_ColorButton,3);
    Color_Group->addButton(fifth_ColorButton,4);
    Color_Group->addButton(sixth_ColorButton,5);
    Color_Group->addButton(seventh_ColorButton,6);



    Color_Box = new QGroupBox(this);
    Color_Box->setObjectName("Transparent");
    Color_Box->setFixedSize(24,180);
    Color_Box->setVisible(false);
    QVBoxLayout *layout_color = new QVBoxLayout;
    Color_Box->setLayout(layout_color);
    layout_color->setMargin(0);
    layout_color->setSpacing(1);
    layout_color->addWidget(first_ColorButton);
    layout_color->addWidget(second_ColorButton);
    layout_color->addWidget(third_ColorButton);
    layout_color->addWidget(fourth_ColorButton);
    layout_color->addWidget(fifth_ColorButton);
    layout_color->addWidget(sixth_ColorButton);
    layout_color->addWidget(seventh_ColorButton);

    //...

    plateToolBar->addAction(zoom_Plate);

    spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    plateToolBar->addWidget(spacer);

    plateToolBar->addAction(auto_Filling);
    plateToolBar->addAction(free_Filling);
    plateToolBar->addAction(user_Filling);
    plateToolBar->addSeparator();
    plateToolBar->addAction(clear_Plate);
    plateToolBar->addSeparator();
    plateToolBar->addSeparator();
    plateToolBar->addAction(order_Filling);
    plateToolBar->addAction(type_View);
    plateToolBar->addSeparator();
    plateToolBar->addSeparator();
    plateToolBar->setIconSize(QSize(16,16));

    Type_Plate = new PlateTableWidget(0, 0, this);
    Type_Plate->setSelectionMode(QAbstractItemView::NoSelection);
    Type_Plate->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Type_Plate->setContextMenuPolicy(Qt::CustomContextMenu);
    Type_Plate->ability_select = true;
    Type_Plate->selected = false;
    Type_Plate->moving = false;
    Type_Plate->p_prot = prot;
    Plate_Delegate = new PlateItemDelegate();
    Plate_Delegate->style = StyleApp;
    Type_Plate->setItemDelegate(Plate_Delegate);
    Plate_Delegate->get_prot(prot);
    Type_Plate->delegate = Plate_Delegate;
    Type_Plate->enable_editor = true;
    Plate_Delegate->A1 = &Type_Plate->A1;
    Type_Plate->Color_Group = Color_Group;
    header_hor = new MyHeader(Qt::Horizontal, this);
    header_hor->setFont(f);
    Type_Plate->setHorizontalHeader(header_hor);
    header_ver = new MyHeader(Qt::Vertical, this);
    header_ver->setFont(f);
    Type_Plate->setVerticalHeader(header_ver);


    QHBoxLayout *layout_plate = new QHBoxLayout;
    PlateBox->setLayout(layout_plate);
    layout_plate->setMargin(0);
    layout_plate->setSpacing(1);
    layout_plate->addWidget(Type_Plate,1);
    layout_plate->addWidget(Color_Box,1,Qt::AlignBottom);

    connect(Type_Plate, SIGNAL(cellClicked(int,int)), this, SLOT(cellSelected(int,int)));    

    //connect(Type_Plate->horizontalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(columnSelected(int)));
    //connect(Type_Plate->verticalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(rowSelected(int)));
    //connect(Type_Plate->horizontalHeader(), SIGNAL(sectionEntered(int)), this, SLOT(columnMoved(int)));
    //connect(Type_Plate->verticalHeader(), SIGNAL(sectionEntered(int)), this, SLOT(rowMoved(int)));
    connect(header_hor, SIGNAL(sSectionsSelect(QVector<int>)), this, SLOT(ColumnsSelect(QVector<int>)));
    connect(header_ver, SIGNAL(sSectionsSelect(QVector<int>)), this, SLOT(RowsSelect(QVector<int>)));

    connect(Type_Plate, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_Plate(QPoint)));

    connect(Type_Plate->corner_btn,SIGNAL(clicked(bool)), this,SLOT(allSelected()));

    connect(Type_Plate, SIGNAL(ReLoadColor(QMap<int,int>)), this, SLOT(ChangeColor(QMap<int,int>)));

    connect(type_plate, SIGNAL(currentIndexChanged(int)), this, SLOT(change_plate(int)));
    type_plate->setCurrentIndex(0);     //default -> 96

    connect(free_Filling, SIGNAL(changed()), this, SLOT(changED_free()));


    connect(Color_Group, SIGNAL(buttonClicked(int)), this, SLOT(Color_Button(int)));
    Color_Group->buttons().at(0)->setChecked(true);
    emit Color_Group->buttonClicked(0);
}

//-----------------------------------------------------------------------------
//--- Actions
//-----------------------------------------------------------------------------
void Setup::createActions()
{
    //... Main ...
    open_DBProtocol = new QAction(QIcon(":/images/open_db.png"), tr("Open DBASE Protocol"), this);
    connect(open_DBProtocol, SIGNAL(triggered()), this, SLOT(open_DBprotocol()));

    open_Protocol = new QAction(QIcon(":/images/open_new.png"), tr("Open Protocol"), this);
    if(StyleApp == "fusion") open_Protocol->setIcon(QIcon(":/images/flat/open_flat.png"));
    connect(open_Protocol, SIGNAL(triggered()), this, SLOT(open_protocol()));

    open_XML = new QAction(QIcon(":/images/flat/open_xml_flat.png"), tr("Open XML"), this);
    connect(open_XML, SIGNAL(triggered()), this, SLOT(open_XMLprotocol()));
    open_XML->setVisible(false);

    last_Protocols = new QAction(QIcon(":/images/list.png"), tr("Last Protocols"), this);
    if(StyleApp == "fusion") last_Protocols->setIcon(QIcon(":/images/flat/list_flat.png"));
    connect(last_Protocols, SIGNAL(triggered()), this, SLOT(last_protocols()));

    save_Protocol = new QAction(QIcon(":/images/save_new.png"), tr("Save as Template"), this);
    if(StyleApp == "fusion") save_Protocol->setIcon(QIcon(":/images/flat/save_flat.png"));
    connect(save_Protocol, SIGNAL(triggered()), this, SLOT(save_protocol()));

    clear_Protocol = new QAction(QIcon(":/images/clear_new.png"), tr("Clear Protocol"), this);
    if(StyleApp == "fusion") clear_Protocol->setIcon(QIcon(":/images/flat/clear_flat.png"));
    connect(clear_Protocol, SIGNAL(triggered()), this, SLOT(clear_protocol()));

    edit_Test = new QAction(QIcon(":/images/edit_test.png"), tr("Edit Test"), this);
    if(StyleApp == "fusion") edit_Test->setIcon(QIcon(":/images/flat/edit_Test_flat.png"));
    connect(edit_Test, SIGNAL(triggered()), this, SLOT(edit_tests()));

    edit_PreferenceProtocol = new QAction(QIcon(":/images/preference_prot.png"), tr("Edit protocol preference"), this);
    if(StyleApp == "fusion") edit_PreferenceProtocol->setIcon(QIcon(":/images/flat/edit_Protocol_flat.png"));
    connect(edit_PreferenceProtocol, SIGNAL(triggered()), this, SLOT(edit_preferencePro()));

    copy_BlockTests = new QAction(QIcon(":/images/DataBace.png"), tr("Copy block tests"), this);
    if(StyleApp == "fusion") copy_BlockTests->setIcon(QIcon(":/images/flat/copyBlockTests_flat.png"));
    connect(copy_BlockTests, SIGNAL(triggered()), this, SLOT(copy_block_tests()));

    load_ToRun = new QAction(QIcon(":/images/ToRun.png"), tr("Load to Run"), this);
    if(StyleApp == "fusion") load_ToRun->setIcon(QIcon(":/images/flat/To_Run.png"));
    connect(load_ToRun, SIGNAL(triggered()), this, SLOT(load_torun()));

    load_ToAnalysis = new QAction(QIcon(":/images/ToPipet.png"), tr("Load to Analysis"), this);
    if(StyleApp == "fusion") load_ToAnalysis->setIcon(QIcon(":/images/flat/To_Analysis.png"));
    connect(load_ToAnalysis, SIGNAL(triggered()), this, SLOT(load_toanalysis()));

    load_ToPipet = new QAction(QIcon(":/images/ToPipet.png"), tr("Load to Pipet"), this);
    connect(load_ToPipet, SIGNAL(triggered()), this, SLOT(load_topipet()));
    //load_ToPipet->setDisabled(true);

    //... Program ...
    open_Program = new QAction(QIcon(":/images/open_new.png"), tr("Open Program"), this);
    if(StyleApp == "fusion") open_Program->setIcon(QIcon(":/images/flat/open_Program_flat.png"));
    connect(open_Program, SIGNAL(triggered()), this, SLOT(open_program()));

    last_Programs = new QAction(QIcon(":/images/list.png"), tr("Last Programs"), this);
    connect(last_Programs, SIGNAL(triggered()), this, SLOT(last_programs()));

    edit_Program = new QAction(QIcon(":/images/edit_program.png"), tr("Edit Program"), this);
    if(StyleApp == "fusion") edit_Program->setIcon(QIcon(":/images/flat/edit_Program_flat.png"));
    connect(edit_Program, SIGNAL(triggered()), this, SLOT(edit_program()));

    //... Sample ...
    add_Sample = new QAction(QIcon(":/images/add_sample.png"), tr("Add Samples/Tests"), this);
    connect(add_Sample, SIGNAL(triggered()), this, SLOT(add_sample()));

    add_Tube = new QAction(QIcon(":/images/add_tube.png"), tr("Add tube"), this);
    connect(add_Tube, SIGNAL(triggered()), this, SLOT(add_tube()));

    delete_Sample = new QAction(QIcon(":/images/delete_sample.png"), tr("Delete sample"), this);
    connect(delete_Sample, SIGNAL(triggered()), this, SLOT(delete_sample()));
    delete_Sample->setEnabled(false);

    delete_Tube = new QAction(QIcon(":/images/delete_tube.png"), tr("Delete tube"), this);
    connect(delete_Tube, SIGNAL(triggered()), this, SLOT(delete_tube()));
    delete_Tube->setEnabled(false);

    delete_AllSamples = new QAction(QIcon(":/images/clear_16.png"), tr("Delete all samples"), this);
    connect(delete_AllSamples, SIGNAL(triggered()), this, SLOT(delete_all_samples()));
    delete_AllSamples->setEnabled(false);

    Undo = new QAction(QIcon(":/images/undo.png"), tr("Undo"), this);
    connect(Undo, SIGNAL(triggered()), this, SLOT(undo()));
    Undo->setEnabled(false);

    //... Plate ...
    zoom_Plate = new QAction(QIcon(":/images/flat/magnifier-24.png"), tr("zoom Plate"), this);
    connect(zoom_Plate, SIGNAL(triggered(bool)), this, SLOT(zoom_plate()));

    user_Filling = new QAction(QIcon(":/images/open_new.png"), tr("user locate"), this);
    if(StyleApp == "fusion") user_Filling->setIcon(QIcon(":/images/flat/open_Program_flat.png"));
    connect(user_Filling, SIGNAL(triggered(bool)), this,SLOT(change_user()));

    auto_Filling = new QAction(QIcon(":/images/auto.png"), tr("auto"), this);
    if(StyleApp == "fusion") auto_Filling->setIcon(QIcon(":/images/flat/auto_flat.png"));
    connect(auto_Filling, SIGNAL(triggered(bool)), this,SLOT(change_auto()));
    auto_Filling->setCheckable(true);

    free_Filling = new QAction(QIcon(":/images/free.png"), tr("free"), this);
    if(StyleApp == "fusion") free_Filling->setIcon(QIcon(":/images/flat/free_flat.png"));
    connect(free_Filling, SIGNAL(triggered(bool)), this,SLOT(change_free()));

    free_Filling->setCheckable(true);

    order_Filling = new QAction(QIcon(":/images/horiz.png"), tr("order"), this);
    connect(order_Filling, SIGNAL(triggered(bool)), this,SLOT(change_order()));
    order_Filling->setCheckable(true);

    clear_Plate = new QAction(QIcon(":/images/clear_16.png"), tr("clear"), this);
    if(StyleApp == "fusion") clear_Plate->setIcon(QIcon(":/images/flat/clear_flat_16.png"));
    connect(clear_Plate, SIGNAL(triggered(bool)), this,SLOT(clear_plate()));

    type_View = new QAction(QIcon(":/images/numeration_view.png"), tr("Numeration&Color_view"), this);
    connect(type_View, SIGNAL(triggered(bool)), this,SLOT(change_view()));
    type_View->setCheckable(true);

    paste_from_clipboard = new QAction(QIcon(":/images/paste_clipboard.png"),tr("paste samples names from ClipBoard"), this);
    connect(paste_from_clipboard, SIGNAL(triggered(bool)), this, SLOT(from_ClipBoard()));

    increment_digits = new QAction(QIcon(":/images/increment_dgitits.png"),tr("copy samples names with digits increment"), this);
    connect(increment_digits, SIGNAL(triggered(bool)), this, SLOT(Increment_Digits()));

    as_first_sample = new QAction(QIcon(":/images/as_FirstSample.png"),tr("rename as first sample"), this);
    connect(as_first_sample, SIGNAL(triggered(bool)), this, SLOT(As_First_Sample()));

    send_protocol_to_web = new QAction(QIcon(":/images/database-24.png"), tr("send current protocol to WebServer"), this);
    connect(send_protocol_to_web, SIGNAL(triggered(bool)), this, SLOT(send_Protocol_To_Web()));

    look_TIS_Tests = new QAction(tr("TIS tests"), this);
    connect(look_TIS_Tests, SIGNAL(triggered(bool)), this, SLOT(look_TISTests()));

    invert_Row = new QAction(QIcon(":/images/reverse_row.png"),tr(""), this);
    //connect(invert_Row, SIGNAL(triggered(bool)), this, SLOT(InvertRow()));

    invert_Column = new QAction(QIcon(":/images/reverse_col.png"),tr(""), this);
    //connect(invert_Column, SIGNAL(triggered(bool)), this, SLOT(InvertColumn()));

    invert_Plate = new QAction(QIcon(":/images/retweet_24.png"),tr(""), this);
    //connect(invert_Plate, SIGNAL(triggered(bool)), this, SLOT(InvertPlate()));

    tests_layout = new QAction(QIcon(":/images/layout_test_24.png"),tr(""), this);

    paste_SampleProperties = new QAction(QIcon(":/images/flat/load_SampleProperties.png"),tr("load properties of samples from another protocol"), this);
    connect(paste_SampleProperties, SIGNAL(triggered(bool)), this, SLOT(SampleProperties_FromAnotherProtocol()));
}
//-----------------------------------------------------------------------------
//--- ToolBar
//-----------------------------------------------------------------------------
void Setup::createToolBars()
{
    QPalette palette;
    QFont f = qApp->font();

    fileToolBar = addToolBar(tr("Protocol"));
    /*fileToolBar->setStyleSheet(
                    "QToolBar {"
                    "background-color: qlineargradient(spread:pad, x1:1, y1:0.455686, x2:1, y2:1, stop:0 rgba(196, 196, 196, 255), stop:1 rgba(96, 96, 96, 255));"
                    "color:rgb(255, 255, 255);"
                    "}"
                    "QToolButton { "
                    "border: none;"
                    "padding: 5;"
                    "}"
                    "QToolButton:hover { "
                    "background-color: rgba(96, 96, 96, 255);" //white;"
                    "}"
                    "QToolButton:checked:pressed { "
                    "background-color: white;"
                    "}"
                    "QToolButton:pressed { "
                    "background-color: rgba(196, 196, 196, 255);" //red;"
                    "}"
                    "QToolButton:checked { "
                    "background-color: red;"
                    "}"
                    //"QToolBar {height: 20px;}"
                    );*/

    //fileToolBar->addAction(open_DBProtocol);
    //fileToolBar->addSeparator();
    Label_Protocol = new QLabel(tr("Protocol: "), this);
    Name_Protocol = new QLineEdit(this);
    Name_Protocol->setFixedWidth(250);
    f.setBold(true);    
    Name_Protocol->setFont(f);    
    Name_Protocol->setAlignment(Qt::AlignHCenter);

    //QRegExp rx("[A-Za-z0-9_-)(]{1,14}");
    //Name_Protocol->setValidator(new QRegExpValidator(rx, this));

    Name_Protocol->setText(Generate_NameProtocol(96));
    Generate_IDProtocol(prot);
    Name_Protocol->clearFocus();

    connect(Name_Protocol,SIGNAL(textChanged(QString)), this,SLOT(change_NameProtocol(QString)));


    fileToolBar->addWidget(Label_Protocol);
    fileToolBar->addWidget(Name_Protocol);

    QWidget *spacer = new QWidget(this);
    spacer->setFixedWidth(25);
    fileToolBar->addWidget(spacer);

    //LoadToRun = new QPushButton(QIcon(":/images/ToRun.png"), tr("Load To Run"), this);

    //fileToolBar_pro = addToolBar(tr("Protocol_pro"));

    fileToolBar->addAction(open_Protocol);
    fileToolBar->addAction(last_Protocols);
    fileToolBar->addSeparator();
    fileToolBar->addAction(open_XML);
    if(open_XML->isVisible()) fileToolBar->addSeparator();
    fileToolBar->addAction(save_Protocol);
    fileToolBar->addAction(clear_Protocol);

    fileToolBar->addSeparator();
    fileToolBar->addSeparator();

    QWidget *spacer_0 = new QWidget(this);
    spacer_0->setFixedWidth(25);
    fileToolBar->addWidget(spacer_0);

    fileToolBar->addAction(load_ToRun);
    fileToolBar->addSeparator();
    fileToolBar->addAction(load_ToAnalysis);
    //fileToolBar->addAction(load_ToPipet);

    QWidget *p = fileToolBar->widgetForAction(last_Protocols);
    p->setMaximumWidth(13);

    QWidget *spacer_1 = new QWidget(this);
    spacer_1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fileToolBar->addWidget(spacer_1);

    fileToolBar->addAction(copy_BlockTests);
    fileToolBar->addAction(edit_Test);    
    fileToolBar->addAction(edit_PreferenceProtocol);
    fileToolBar->addSeparator();

    /*
    QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(9.0);
    shadowEffect->setColor(QColor(0, 0, 0, 160));
    shadowEffect->setOffset(4.0);
    fileToolBar_pro->setGraphicsEffect(shadowEffect);
    */
}
//-----------------------------------------------------------------------------
//--- fill_SampleTable
//-----------------------------------------------------------------------------
void Setup::fill_SampleTable()
{
    int i,j,k,m,n,l;
    int count;
    QString text,str;
    QString name_str;
    QStringList header, list_temp;
    QTableWidgetItem *newItem;
    int num_row=0;
    int count_ch;
    int heigth;
    int pos,color;
    int col,row;
    int id_channel, id_tube;
    int id;
    int current_Num = 1;
    int current_Sample = 0;
    int num_samples = 0;
    int active_ch = prot->active_Channels;
    bool isopen = Samples_Table->open_samples;
    if(!isopen) Samples_Table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    else Samples_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    Samples_Table->setSelectionBehavior(QAbstractItemView::SelectRows);

    int kind = 0;  //sample(0),K+(1),K-(2),St(3)

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_Tube         *tube;
    rt_Channel      *channel;
    rt_Preference   *preference;

    QVector<QTableWidgetItem*> Items_Copies;
    QVector<int> header_row;

    header_row.clear();

    prot->Plate.PlateSize(prot->count_Tubes,row,col);
    for(i = Samples_Table->rowCount(); i>0; i--) Samples_Table->removeRow(i-1);
    header << tr("Pos") << "" << tr("Identificator") << tr("Type") << tr("Copies");

    Tubes_key.clear();
    Tubes_kind.clear();
    list_ID.clear();
    list_NAME.clear();

    Samples_Table->clear();
    Samples_Table->setColumnCount(5);
    Samples_Table->setHorizontalHeaderLabels(header);
    Samples_Table->horizontalHeader()->setDefaultSectionSize(10);
    Samples_Table->horizontalHeader()->setMinimumSectionSize(10);
    Samples_Table->setColumnWidth(0,50);
    Samples_Table->setColumnWidth(1,10);
    Samples_Table->setColumnWidth(3,50);
    Samples_Table->setColumnWidth(4,60);
    Samples_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    Samples_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    Samples_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    Samples_Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    Samples_Table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);

    Samples_Table->horizontalHeader()->setSectionHidden(4,true);


    //... count rows ...

    count = prot->count_Tubes;
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            count += sample->tubes.size();
        }
    }
    Samples_Table->setRowCount(count);
    header.clear();

    //... cycle of groups ...

    //qDebug() << "groups: " << prot->Plate.groups.size();
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        //qDebug() << "samples: " << group->samples.size();
        for(j=0; j<group->samples.size(); j++)
        {
            current_Sample++;
            sample = group->samples.at(j);
            test = sample->p_Test;
            kind = 0;

            if(sample->ID_Sample == "")
            {
                Sleep(5);
                sample->ID_Sample = GetRandomString(10).toStdString();
            }
            list_ID.append(QString::fromStdString(sample->ID_Sample));
            list_NAME.append(QString::fromStdString(sample->Unique_NameSample));            

            //... Header ...
            if(isopen) Samples_Table->setRowHeight(num_row, 30);
            else Samples_Table->setRowHeight(num_samples, 30);
            for(k=0; k<Samples_Table->columnCount(); k++)
            {
                newItem = new QTableWidgetItem();
                text = "Header_Sample:";
                if(k == 0 && sample->tubes.size() > 1) text += "(multi)";
                if(k == 0) text += list_ID.last();
                if(k == 0 && !isopen)
                {
                    tube = sample->tubes.at(0);
                    pos = tube->pos;
                    if(pos >= 0)
                    {
                        text += "(pos)" + Convert_IndexToName(pos,col);
                        if(sample->tubes.size() > 1) text += ",...";
                    }
                }
                if(k == 1 && !isopen)
                {
                    tube = sample->tubes.at(0);
                    pos = tube->pos;
                    text += "(color)";
                    if(pos >=0 && pos < prot->count_Tubes) color = prot->color_tube.at(pos);
                    else color = tube->color;
                    if(sample->tubes.size() > 1) color = -1;
                    text += QString::number(color);
                }

                if(k == 2)
                {
                    text = "Header_Sample:";
                    text += QString::fromStdString(sample->Unique_NameSample);
                    name_str = QString::fromStdString(test->header.Name_Test);
                    if(Map_TestTranslate.size()) name_str = Map_TestTranslate.value(name_str, name_str);
                    text += "  (" + name_str + ")";
                }
                if(k == 3)
                {
                    text = "-1";
                    for(m=0; m<sample->preference_Sample.size(); m++)
                    {
                        preference = sample->preference_Sample.at(m);
                        if(preference->name == "kind")
                        {
                            if(preference->value == "ControlPositive") text = "0";
                            if(preference->value == "ControlNegative") text = "1";
                            if(preference->value == "Standart") text = "2";
                            break;
                        }
                    }
                    kind = text.toInt() + 1;
                }
                if(k == 4)
                {
                    Items_Copies.append(newItem);
                    text = "-";
                    for(m=0; m<sample->preference_Sample.size(); m++)
                    {
                        preference = sample->preference_Sample.at(m);
                        if(preference->name == "copies")
                        {
                            text = QString::fromStdString(preference->value);
                            if(text == QString::fromStdString(sample->ID_Sample)) text += QString("\tmain_copy");
                            //text += QString("\t%1").arg(num_samples);
                            break;
                        }
                    }
                }

                newItem->setText(text);
                if(k != 2 && k != 3 && k != 4) newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                if(isopen) Samples_Table->setItem(num_row,k,newItem);
                else Samples_Table->setItem(num_samples,k,newItem);

                if(!k) header_row.append(1);
            }
            num_samples++;
            num_row++;

            //header.append(" ");
            header.append(QString("%1").arg(current_Sample));

            //... Tubes ...
            //qDebug() << "tubes: " << sample->tubes.size();
            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                id_tube = tube->ID_Tube;                
                Tubes_key.insert(num_row, tube);
                Tubes_kind.insert(num_row, kind);

                //... 0. current numeration ...
                //header.append(QString::number(current_Num));
                header.append(QString("%1-%2").arg(current_Sample).arg(current_Num));
                current_Num++;

                //... 1. position of tube ...
                pos = tube->pos;
                if(pos >=0 && pos < prot->count_Tubes) color = prot->color_tube.at(pos);
                else color = tube->color;

                //... 2. count active channel in tube ...
                count_ch = tube->channels.size();
                heigth = 18;
                if(isopen)
                {
                    if(count_ch > 1) Samples_Table->setRowHeight(num_row, heigth*0.9*count_ch);
                    else Samples_Table->setRowHeight(num_row, heigth);
                }

                //... 3. load data ...
                for(m=0; m<Samples_Table->columnCount(); m++)
                {
                    text = "";
                    for(n=0; n<count_ch; n++)
                    {
                        if((m == 0 || m == 1) && n) continue;   // for pos and color
                        if(n) text += "\r\n";

                        channel = tube->channels.at(n);
                        id_channel = channel->ID_Channel;

                        id = 0;
                        for(l=0; l<=id_channel; l++)
                        {
                           if(active_ch & (0x0f<<4*l)) id++;
                        }

                        id = (id-1)*prot->count_Tubes + pos;


                        switch(m)
                        {
                        case 2: // Identificator
                                    name_str = FindNameByTest(test,id_tube,id_channel);
                                    if(Map_TestTranslate.size()) name_str = Map_TestTranslate.value(name_str, name_str);

                                    text += QString::number(id_channel) + " " + name_str;

                                            //FindNameByTest(test,id_tube,id_channel);
                                    break;

                        case 1: // color
                                    text = QString::number(color);
                                    //qDebug() << text;
                                    break;

                        case 0: // position
                                    if(pos >= 0) text = Convert_IndexToName(pos,col);
                                    else text = "";
                                    break;

                        default:    text = "";
                                    break;
                        }

                    }
                    if(!isopen) continue;

                    newItem = new QTableWidgetItem();
                    newItem->setText(text);
                    newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                    Samples_Table->setItem(num_row,m,newItem);                    
                }
                header_row.append(0);
                num_row++;
            }
        }
    }

    // Check for validity info about copies

    for(i=0; i<Items_Copies.size(); i++)                // if absent main copies -> all will not be copies
    {
        text = Items_Copies.at(i)->text().trimmed();
        list_temp = text.split("\t");
        if(list_temp.size())
        {
            text = list_temp.at(0);
            if(list_ID.indexOf(text) < 0) Items_Copies.at(i)->setText("-");
        }
    }

    for(i=0; i<Items_Copies.size(); i++)                // if only single copies -> it is not copies
    {
        text = Items_Copies.at(i)->text().trimmed();
        if(text.contains("\tmain_copy"))
        {
            id = text.indexOf("\t");
            str = text.mid(0,id);
            if(i == (Items_Copies.size()-1) ||
               !Items_Copies.at(i+1)->text().startsWith(str))
            {
                Items_Copies.at(i)->setText("-");
            }
        }
    }
    //...


    if(isopen) Samples_Table->setRowCount(num_row);
    else Samples_Table->setRowCount(num_samples);

    if(!isopen)
    {
        header.clear();
        for(i=0; i<num_samples; i++) header.append(QString(" %1").arg(i+1));
    }
    Samples_Table->setVerticalHeaderLabels(header);
    Samples_Table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    if(free_currentTube > 0)
    {
        //newItem = Samples_Table->takeItem(free_currentTube,1);
        //Samples_Table->scrollToItem(newItem,QAbstractItemView::EnsureVisible);
        Samples_Table->setCurrentCell(free_currentTube,0);
        Samples_Table->setFocus();
    }

    if(Tubes_key.size()) {delete_AllSamples->setEnabled(true); DELETE_AllSamples->setEnabled(true);}
    else {delete_AllSamples->setEnabled(false); DELETE_AllSamples->setEnabled(false);}

    //Samples_Table->resizeRowsToContents();
    i = 0;
    foreach(k, header_row)
    {
        if(!k) Samples_Table->resizeRowToContents(i);
        i++;
    }

    //qDebug() << "header_row: " << header_row.size() << header_row;
    header_row.clear();    

}
//-----------------------------------------------------------------------------
//--- cellSample_Selected
//-----------------------------------------------------------------------------
void Setup::cellSample_Selected()
{
    int num;
    QString text;
    QTableWidgetItem *item;

    /*if(!Samples_Table->open_samples)
    {
        Samples_Table->blockSignals(true);
        Samples_Table->setCurrentCell(-1,-1);
        Samples_Table->blockSignals(false);
        return;
    }*/

    QList<QTableWidgetItem*> list = Samples_Table->selectedItems();
    if(list.size())
    {
        delete_Sample->setEnabled(true);
        DELETE_Sample->setEnabled(true);
        //delete_AllSamples->setEnabled(true);
        item = list.at(0);
        num = item->row();

        //qDebug() << "row,col = " << item->row() << item->column();

        if(item->text().indexOf("Header_Sample:") >= 0 && Samples_Table->open_samples)
        {
            if(item->row() < Samples_Table->rowCount()-1)
            {
                num++;
                Samples_Table->setCurrentCell(item->row()+1,0);
                Samples_Table->setFocus();
            }
        }
        else
        {
            while(num)
            {
                num--;
                item = Samples_Table->item(num,0);
                text = item->text();
                if(text.indexOf("Header_Sample:") >= 0)
                {
                    if(text.contains("multi")) delete_Tube->setEnabled(false);
                    else delete_Tube->setEnabled(true);
                    break;
                }
            }
        }
    }
    else
    {
        delete_Tube->setEnabled(false);
        delete_Sample->setEnabled(false);
        DELETE_Sample->setEnabled(false);
        //DELETE_AllSamples->setEnabled(false);
        //delete_AllSamples->setEnabled(false);
    }

    // current row -> what is the TEST?
    Samples_Delegate->disable_Standarts = true;

    rt_Test *ptest;
    int pos;
    int cur_row = Samples_Table->currentRow();
    cur_row--;
    if(cur_row >= 0)
    {
        item = Samples_Table->item(cur_row, 2);
        text = item->text();
        if(text.contains("Header_Sample:"))
        {
            pos = text.indexOf("  (");
            if(pos >= 0)
            {
                pos += 3;
                text.remove(0,pos);
                text.replace(")","");
                foreach(ptest, prot->tests)
                {
                    if(ptest->header.Name_Test == text.toStdString())
                    {
                        if(ptest->header.Type_analysis == 0x0001 ||
                           ptest->header.Type_analysis == 0x0003 ||
                           ptest->header.Type_analysis == 155077)     Samples_Delegate->disable_Standarts = false;
                        break;
                    }
                }
            }
        }
    }

}
//-----------------------------------------------------------------------------
//--- cellSample_Changed
//-----------------------------------------------------------------------------
void Setup::cellSample_Changed(int row, int col)
{
    //qDebug() << "change row: " << row;
}

//-----------------------------------------------------------------------------
//--- fill_Information
//-----------------------------------------------------------------------------
void Setup::fill_Information()
{
    if(prot == NULL) return;
    prot->name = Name_Protocol->text().trimmed().toStdString();
    prot->owned_by = Operator.toStdString();    

    Info_Tree->clear_Info();
    Info_Tree->fill_Info(prot);
}

//-----------------------------------------------------------------------------
//--- fill_TestSample
//-----------------------------------------------------------------------------
void Setup::fill_TestSample()
{
    int i,j,k;
    int count_test;
    int count_sample;
    int count_tube;
    int count_channel;
    QString text, name_str;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    int active_channels = 0;

    rt_Test         *test;
    rt_TubeTest     *tube;
    rt_ChannelTest  *channel;
    rt_GroupSamples *source;
    rt_Sample       *sample;

    if(prot == NULL) return;
    count_test = prot->tests.size();
    count_sample = prot->Plate.groups.size();
    if(count_sample == 0) count_test = 0;

    //... Tests ...
    QStringList list_header;
    list_header << tr("Tests") << " ";
    Test_Tree->clear();
    Test_Tree->setColumnCount(2);
    Test_Tree->setHeaderLabels(list_header);

    Test_Tree->header()->setMinimumSectionSize(1);
    Test_Tree->header()->setStretchLastSection(false);
    Test_Tree->header()->resizeSection(1, 50);
    Test_Tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    Test_Tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);

    for(i=0; i<count_test; i++)
    {
        test = prot->tests.at(i);
        text = QString::fromStdString(test->header.Name_Test);
        item = new QTreeWidgetItem(Test_Tree);
        //item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        //item->setCheckState(0, Qt::Checked);
        if(Map_TestTranslate.size()) text = Map_TestTranslate.value(text, text);
        item->setText(0, text);
        item->setText(1, QString::fromStdString(test->header.version));
        if(test->header.Hash == "ok") item->setIcon(0, logo);
        else item->setIcon(0, logonull);

        active_channels |= test->header.Active_channel;

        count_tube = test->tubes.size();
        if(count_tube < 2) continue;

        for(j=0; j<count_tube; j++)
        {
            tube = test->tubes.at(j);
            count_channel = tube->channels.size();
            text = "";
            for(k=0; k<count_channel; k++)
            {
                channel = tube->channels.at(k);
                if(text.length()) text += "; ";
                name_str = QString::fromStdString(channel->name);
                if(Map_TestTranslate.size()) name_str = Map_TestTranslate.value(name_str, name_str);
                text += name_str;
            }
            item_child = new QTreeWidgetItem(item);
            //item_child->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            //item_child->setCheckState(0, Qt::Checked);
            item_child->setText(0, text);
            item_child->setIcon(0, logonull);
        }
    }
    prot->active_Channels = active_channels;

    //... Samples ...
    Sample_Tree->blockSignals(true);

    Sample_Tree->clear();
    Sample_Tree->setColumnCount(2);

    QStringList pp;
    pp << tr("Sample") << tr("Test");
    //Sample_Tree->setHeaderLabel(tr("Samples"));
    Sample_Tree->setHeaderLabels(pp);

    for(i=0; i<count_sample; i++)
    {
        source = prot->Plate.groups.at(i);
        for(j=0; j<source->samples.size(); j++)
        {
            sample = source->samples.at(j);
            text = QString::fromStdString(sample->Unique_NameSample);
            item = new QTreeWidgetItem(Sample_Tree);            
            item->setText(0, text);
            text = QString::fromStdString(sample->p_Test->header.Name_Test);
            if(Map_TestTranslate.size()) text = Map_TestTranslate.value(text, text);
            item->setText(1, text);
            item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
    }
    Sample_Tree->resizeColumnToContents(0);

    Sample_Tree->blockSignals(false);
}
//-----------------------------------------------------------------------------
//--- fill_SampleProperties()
//-----------------------------------------------------------------------------
void Setup::fill_SampleProperties()
{
    QString text;
    QMap <QString,QString> map_property;
    map_property.insert("patient", tr("patient"));
    map_property.insert("sex", tr("sex"));
    map_property.insert("age", tr("age"));
    map_property.insert("organization", tr("organization"));
    map_property.insert("phisician", tr("phisician"));
    map_property.insert("date", tr("date"));
    map_property.insert("note", tr("note"));

    map_property.insert("additional", tr("additional"));

    rt_GroupSamples *group;
    rt_Sample       *sample;

    rt_Preference   *property;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;

    Sample_Properties->blockSignals(true);

    Sample_Properties->clear();
    Sample_Properties->setColumnCount(2);

    QStringList pp;
    pp << tr("Samples properties") << tr("Value");
    Sample_Properties->setHeaderLabels(pp);

    foreach(group, prot->Plate.groups)
    {
        if(group->samples.size() == 0) continue;

        sample = group->samples.at(0);
        item = new QTreeWidgetItem(Sample_Properties);
        //text = QString::fromStdString(group->Unique_NameGroup);
        text = QString::fromStdString(sample->Unique_NameSample);
        item->setText(0, text);

        foreach(property, group->preference_Group)
        {
            item_child = new QTreeWidgetItem(item);
            item_child->setFlags(item_child->flags() | Qt::ItemIsEditable);
            text = QString::fromStdString(property->name);
            if(!map_property.keys().contains(text)) item_child->setText(0, text);
            item_child->setText(0, map_property.value(text,""));
            text = QString::fromStdString(property->value);
            item_child->setText(1, text);
        }

        item->setExpanded(false);
    }

    Sample_Properties->resizeColumnToContents(0);
    Sample_Properties->blockSignals(false);
}

//-----------------------------------------------------------------------------
//--- Check_TestsVolumeTube(rt_Protocol*)
//-----------------------------------------------------------------------------
void Setup::Check_TestsVolumeTube(rt_Protocol *p)
{
    int i;
    QString text;
    int volume;

    rt_Test *ptest;

    if(!p) return;
    if(p->tests.size() <= 1) return;

    for(i=0; i<p->tests.size(); i++)
    {
        ptest = p->tests.at(i);
        if(!i)
        {
            volume = ptest->header.Volume_Tube;
            continue;
        }

        if(volume != ptest->header.Volume_Tube)
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(10000);
            message.setIcon(QMessageBox::Warning);
            text = tr("Attention! Discovered different tubes volume for multiple tests!");
            text += "\r\n" + tr("The current protocol cannot be run!");
            message.setText(text);
            message.exec();
            break;
        }
    }
}
//-----------------------------------------------------------------------------
//--- Check_ProtocolProgramm_TestsProgramm(rt_Protocol *p)
//-----------------------------------------------------------------------------
void Setup::Check_ProtocolProgramm_TestsProgramm(rt_Protocol *p)
{
    if(!p) return;
    if(p->tests.size() == 0) return;

    QVector<string> Protocol_Pro;
    QVector<string> Tests_Pro;
    rt_Test         *ptest;

    int i;
    QString text;
    QStringList list;
    int vol = p->volume;

    //qDebug() << "vol: " << vol;

    Protocol_Pro = QVector<string>::fromStdVector(p->program);
    text = QString::fromStdString(Protocol_Pro.at(0));
    if(text.startsWith("XPRG "))
    {
        list = text.split(" ");
        if(list.size() > 2)
        {
            list.replace(2,QString::number(vol));
            text = list.join(" ");
            Protocol_Pro.replace(0, text.toStdString());
        }
    }

    for(i=0; i<p->tests.size(); i++)
    {
        ptest = p->tests.at(i);
        Tests_Pro.clear();
        Tests_Pro = QVector<string>::fromStdVector(ptest->header.program);
        text = QString::fromStdString(Tests_Pro.at(0));
        if(text.startsWith("XPRG "))
        {
            list = text.split(" ");
            if(list.size() > 2)
            {
                list.replace(2,QString::number(vol));
                text = list.join(" ");
                Tests_Pro.replace(0, text.toStdString());
            }
        }

        if(Protocol_Pro != Tests_Pro)
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(10000);
            message.setIcon(QMessageBox::Warning);
            text = tr("Attention! The Protocol program differs from the Test Program!");
            text += "\r\n" + tr("The current protocol cannot be run!");
            message.setText(text);
            message.exec();
            break;
        }
    }
}

//-----------------------------------------------------------------------------
//--- Check_TestsPtrogramm(rt_Protocol*)
//-----------------------------------------------------------------------------
void Setup::Check_TestsPtrogramm(rt_Protocol *p)
{
    int i;

    if(!p) return;
    if(p->tests.size() <= 1) return;

    QVector<string> First_Pro;
    QVector<string> Next_Pro;
    string str;
    QString text;
    QStringList list;
    int vol = p->volume;

    rt_Test         *ptest;

    for(i=0; i<p->tests.size(); i++)
    {
        ptest = p->tests.at(i);
        if(!i)
        {
            First_Pro = QVector<string>::fromStdVector(ptest->header.program);
            text = QString::fromStdString(First_Pro.at(0));
            if(text.startsWith("XPRG "))
            {
                list = text.split(" ");
                if(list.size() > 2)
                {
                    list.replace(2,QString::number(vol));
                    text = list.join(" ");
                    First_Pro.replace(0, text.toStdString());
                }
            }
            continue;
        }

        Next_Pro = QVector<string>::fromStdVector(ptest->header.program);
        text = QString::fromStdString(Next_Pro.at(0));
        if(text.startsWith("XPRG "))
        {
            list = text.split(" ");
            if(list.size() > 2)
            {
                list.replace(2,QString::number(vol));
                text = list.join(" ");
                Next_Pro.replace(0, text.toStdString());
            }
        }
        if(Next_Pro != First_Pro)
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(10000);
            message.setIcon(QMessageBox::Warning);
            text = tr("Attention! Discovered different programs for multiple tests!");
            text += "\r\n" + tr("The current protocol cannot be run!");
            message.setText(text);
            message.exec();
            break;
        }
    }
}

//-----------------------------------------------------------------------------
//--- Check_TestsExposition(rt_Protocol*)
//-----------------------------------------------------------------------------
void Setup::Check_TestsExposition(rt_Protocol *p)
{
    if(!p) return;
    if(p->tests.size() <= 1) return;

    int i,j,k;
    QStringList list;
    QString text;
    QVector<double> expo, expo_mask;
    double value;
    bool ok;
    int act_ch = p->active_Channels;

    rt_Test         *ptest;
    rt_Preference   *preference;

    for(i=0; i<p->tests.size(); i++)
    {
        ptest = p->tests.at(i);

        for(j=0; j<ptest->preference_Test.size(); j++)
        {
            preference = ptest->preference_Test.at(j);

            if(preference->name == EXPOSURE)
            {
                text = QString::fromStdString(preference->value);
                list = text.split(QRegExp("\\s+"));
                k = 0;
                foreach(text, list)
                {
                    if(text == "0") list.replace(k,"1");
                    k++;
                }

                //qDebug() << "Expo: " << QString::fromStdString(ptest->header.Name_Test) << list;
                expo.clear();

                for(k=0; k<COUNT_CH; k++)
                {                    
                    if(act_ch & (0x0f << k*4))
                    {
                        value = QString(list.at(k)).toDouble(&ok);
                        if(!ok) value = 1.;
                        expo.push_back(value);
                    }
                }
                if(!i) expo_mask = expo.mid(0);
                //qDebug() << "expo: " << expo << expo_mask;
            }
        }
        if(expo_mask.size() == expo.size())
        {
            ok = qEqual(expo.begin(), expo.end(), expo_mask.begin());
        }
        else ok = false;

        if(!ok)
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(10000);
            message.setIcon(QMessageBox::Warning);
            text = tr("Attention! Discovered different values of exposures for multiple tests!");
            text += "\r\n" + tr("The current protocol cannot be run!");
            message.setText(text);
            message.exec();
            break;
        }
    }
}
//-----------------------------------------------------------------------------
//--- Check_ValidNameFile
//-----------------------------------------------------------------------------
QString Setup::Check_ValidNameFile(QString name)
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
//--- Generate_NameProtocol
//-----------------------------------------------------------------------------
QString Setup::Generate_NameProtocol(int num_tubes)
{
    QString text = "";
    QString type = "";
    QString date = "";

    if(template_NAME.type) type = QString("_(%1)").arg(num_tubes);
    if(template_NAME.date_time.size()) date = QString("_%1").arg(QDateTime::currentDateTime().toString(template_NAME.date_time));

    text = QString("%1%2%3").arg(template_NAME.name).arg(type).arg(date);

    return(text);
}
//-----------------------------------------------------------------------------
//--- Generate_NameProtocol
//-----------------------------------------------------------------------------
void Setup::Generate_IDProtocol(rt_Protocol *p)
{
    if(!p) return;
    if(!QString::fromStdString(p->regNumber).trimmed().isEmpty()) return;

    QString text = QString("%1_%2").arg(GetRandomString(5)).arg(QDateTime::currentDateTime().toString("ddMMyy_hmmss"));
    p->regNumber = text.toStdString();
}

//-----------------------------------------------------------------------------
//--- open_DBprotocol
//-----------------------------------------------------------------------------
void Setup::open_DBprotocol()
{
    //...
    ApplSettings->beginGroup("Pipet");
    QString dir = ApplSettings->value("templates_dir","").toString();
    if(dir.isEmpty()) dir = qApp->applicationDirPath();
    ApplSettings->endGroup();
    //...

    QString filename = open_protocol(dir);
    if(filename.isEmpty()) return;

    QDomDocument doc;
    QDomElement  root;
    QDomElement  item;
    QDomElement  first;
    bool res = false;
    QString text;

    QFile file(filename);

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();
            first = root.firstChild().toElement();
            item = MakeElement(doc,"protocol_type","Balu");
            root.insertBefore(item,first);

            filename = dir_temp.path() + "/DB_setup.rt";
            QFile file_out(filename);
            if(file_out.open(QIODevice::WriteOnly))
            {
                QTextStream(&file_out) << doc.toString();
                file_out.close();
                res = true;
            }
        }
        else file.close();
    }

    message.setStandardButtons(QMessageBox::Ok);
    message.button(QMessageBox::Ok)->animateClick(5000);
    if(res)
    {
        message.setIcon(QMessageBox::Information);
        text = "File successfully written!\r\n" + filename;
        load_ToPipet->setDisabled(false);
    }
    else
    {
        message.setIcon(QMessageBox::Warning);
        text = "File is NOT saved!\r\n" + filename;
        message.setText(text);
        message.exec();
    }
}

//-----------------------------------------------------------------------------
//--- open_XMLprotocol()
//-----------------------------------------------------------------------------
void Setup::open_XMLprotocol(QString fn)
{
    int i,j,k;

    QString fileName = "";
    QString dirName = user_Dir.absolutePath();
    QString selectedFilter;

    if(fn.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this, tr("Open XML-Protocol"),
                                dirName,
                                tr("Protocols File (*.xml)"),
                                &selectedFilter);

        if(fileName.isEmpty()) return;
    }
    else fileName = fn;

    QString error = "";
    QString text, name, str;
    QStringList list;
    int num,x,y;
    int id;
    bool ok;
    QDomDocument    doc;
    QDomElement     root;
    QDomElement     cr_plate;
    QDomNode        child;    
    QDomNode        item;
    QDomNamedNodeMap map_attributes;
    QMap<QString, QString> map_SamplesProperties;

    rt_Test         *ptest, *pTEST;
    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;
    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Channel      *channel;
    rt_Preference   *property;
    bool load_program = false;
    bool test_in_user;
    bool find_group;
    bool find_sample;
    QString current_test = "";
    QStringList list_research;
    QStringList list_currentCell;
    QMap<QString,QString> list_TESTs;

    QMap<QString,QString> map_ID_Name_tests;
    foreach(ptest, TESTs)
    {
        text = QString::fromStdString(ptest->header.ID_Test);
        name = QString::fromStdString(ptest->header.Name_Test);
        map_ID_Name_tests.insert(text, name);
    }

    int row=0,col=0;

    QFile file(fileName);

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        // clear protocol
        clear_protocol();
        // ...

        if(!doc.setContent(&file))
        {
            file.close();            
            return;
        }

        file.close();

        while(1)
        {
            root = doc.documentElement();
            cr_plate = root.firstChildElement("create_plate");
            if(cr_plate.isNull()) {error = tr("don't find create_plate"); break;}

            col = cr_plate.attribute("size_x","0").toInt(&ok);
            row = cr_plate.attribute("size_y","0").toInt(&ok);            
            prot->count_Tubes = col*row;
            if(prot->count_Tubes <= 0 || prot->count_Tubes > 384) {error = tr("size of plate"); break;}
            prot->enable_tube.clear();
            prot->color_tube.clear();
            for(i=0; i<prot->count_Tubes; i++)
            {
                prot->color_tube.push_back(0x808080);       // Qt::gray
                prot->enable_tube.push_back(0);
            }
            prot->regNumber = cr_plate.attribute("plate","").toStdString();
            prot->Plate.Unique_NamePlate = prot->regNumber;

            for(i=0; i<cr_plate.childNodes().size(); i++)   // cell
            {
                if(!error.isEmpty()) break;

                child = cr_plate.childNodes().at(i);
                if(child.nodeName() != "cell") continue;

                num = child.toElement().attribute("n","0").toInt(&ok);
                x = child.toElement().attribute("x","0").toInt(&ok);
                y = child.toElement().attribute("y","0").toInt(&ok);
                name = child.toElement().attribute("name","sample");
                list_currentCell.clear();

                //qDebug() << "cell: " << x << y << name;

                for(j=0; j<child.childNodes().size(); j++)      // cycle inside cell: test...
                {
                    if(!error.isEmpty()) break;

                    item = child.childNodes().at(j);
                    if(item.nodeName() != "test") continue;
                    text = item.toElement().attribute("id","\\");
                    list = text.split("\\");

                    //if(list.size() < 2) {error = QString("%1 <test id=\"%2\"").arg(tr("invalid format: ")).arg(text); break;}

                    //... replace IDTest on Name of the Test ...
                    /*str = list.at(0);
                    if(!map_ID_Name_tests.values().contains(str) && map_ID_Name_tests.keys().contains(str))
                    {
                        str = map_ID_Name_tests.value(str);
                        list.replace(0, str);
                    }*/
                    //...

                    if(j==0) current_test = list.at(0);
                    if(current_test != list.at(0))
                    {
                        error = QString("%1 %2 - <test id=\"%3\\%4").arg(tr("invalid current test: ")).arg(current_test).arg(list.at(0)).arg(list.at(1));
                        break;
                    }

                    //qDebug() << "current_test: " << current_test;

                    if(j==0 && !list_TESTs.contains(current_test))
                    {
                        test_in_user = false;
                        foreach(pTEST, TESTs)
                        {
                            if(pTEST->header.Name_Test == current_test.toStdString())
                            {
                                ptest = new rt_Test();
                                prot->tests.push_back(ptest);
                                CreateCopy_Test(pTEST, ptest);
                                test_in_user = true;
                                list_research.clear();
                                foreach(tube_test, ptest->tubes)
                                {
                                    str = "";
                                    foreach(channel_test, tube_test->channels)
                                    {
                                        if(!str.isEmpty()) str += ";";
                                        str += QString::fromStdString(channel_test->name);
                                    }
                                    list_research.append(str);
                                }
                                list_TESTs.insert(current_test, list_research.join("\r\n"));
                                //qDebug() << "current: " << list_TESTs;
                                break;
                            }
                        }
                        if(!test_in_user)
                        {
                            error = QString("%1 %2").arg(tr("don't find test: ")).arg(current_test);
                            break;
                        }
                    }
                    if(list.size() > 1) list_currentCell.append(list.at(1));
                }
                if(!error.isEmpty()) break;

                //Check valid state for single cell
                text = list_currentCell.join(";");                
                str = list_TESTs.value(current_test, "");
                list_research = str.split("\r\n");
                id = list_research.indexOf(text);

                if(id < 0 && list.size() == 1 && list_TESTs.keys().contains(current_test)) id = 0;  // for single_tubes tests

                //qDebug() << "cell: " << i << current_test << text << id;
                if(id >= 0)
                {
                    // find test in protocol
                    ptest = NULL;
                    foreach(ptest, prot->tests)
                    {
                        if(ptest->header.Name_Test == current_test.toStdString())
                        {
                            // load program
                            if(!load_program)
                            {
                                if(!ptest->header.program.empty())
                                {
                                    prot->program.clear();
                                    prot->program = ptest->header.program;
                                    Parsing_ProgramAmplification(prot);
                                    //Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
                                    //plot_Program->draw_Program(prot);
                                    //scheme_Program->draw_Program(prot);

                                    load_program = true;
                                }
                            }

                            break;
                        }
                    }
                    if(!ptest) {error = QString("%1 %2").arg(tr("don't find test: ")).arg(current_test); break;}

                    // add tube to sample
                    find_group = false;
                    foreach(group, prot->Plate.groups)
                    {
                        if(group->Unique_NameGroup == name.toStdString())
                        {
                            find_group = true;
                            break;
                        }
                    }
                    if(!find_group)
                    {
                        group = new rt_GroupSamples();
                        group->Unique_NameGroup = name.toStdString();
                        //Create_GroupSamplesProperties(group);
                        {Sleep(5); group->ID_Group = GetRandomString(10).toStdString();}
                        prot->Plate.groups.push_back(group);
                    }

                    find_sample = false;
                    foreach(sample, group->samples)
                    {
                        if(sample->Unique_NameSample == name.toStdString() &&
                           sample->ID_Test == ptest->header.ID_Test)
                        {
                            find_sample = true;
                            break;
                        }
                    }
                    if(!find_sample)
                    {
                        sample = new rt_Sample();
                        {Sleep(5); sample->ID_Sample = GetRandomString(10).toStdString();}
                        sample->Unique_NameSample = group->Unique_NameGroup;
                        sample->ID_Test = ptest->header.ID_Test;
                        sample->p_Test = ptest;
                        group->samples.push_back(sample);
                    }

                    if(!find_group) Create_GroupSamplesProperties(group);

                    tube = new rt_Tube();
                    sample->tubes.push_back(tube);

                    //qDebug() << "create tube: " << QString::fromStdString(sample->Unique_NameSample) << QString::fromStdString(sample->ID_Test);


                    tube->ID_Tube = id;
                    tube->pos = (x-1) + (y-1)*col;
                    rt_TubeTest *tube_test = ptest->tubes.at(id);
                    tube->color = tube_test->color;
                    if(tube->pos >= 0) prot->enable_tube.at(tube->pos) = 1;
                    if(tube->pos >= 0) prot->color_tube.at(tube->pos) = tube->color;
                    foreach(channel_test, tube_test->channels)
                    {
                        channel = new rt_Channel();
                        channel->ID_Channel = channel_test->ID_Channel;
                        channel->Unique_NameChannel = channel_test->name;
                        tube->channels.push_back(channel);
                    }
                }
                else
                {
                    error = QString("%1 %2 - %3").arg(tr("don't find tube with this tests: ")).arg(current_test).arg(text);
                    break;
                }
            }

            // Find Sample
            for(i=0; i<root.childNodes().size(); i++)
            {
                item = root.childNodes().at(i);
                //qDebug() << "nodeName: " << item.nodeName();
                if(item.nodeName() == "sample")
                {
                    if(item.toElement().hasAttributes())
                    {
                        map_SamplesProperties.clear();
                        map_attributes = item.toElement().attributes();
                        for(j=0; j<map_attributes.count(); j++)
                        {
                            child = map_attributes.item(j);
                            //qDebug() << "map_attributes: " << i << j << child.nodeName() << item.toElement().attribute(child.nodeName());
                            map_SamplesProperties.insert(child.nodeName(), item.toElement().attribute(child.nodeName()));
                        }

                        name = map_SamplesProperties.value("name", "");
                        if(name.isEmpty()) continue;

                        foreach(group, prot->Plate.groups)
                        {
                            if(group->Unique_NameGroup == name.toStdString())
                            {
                                foreach(property, group->preference_Group)
                                {
                                    if(map_SamplesProperties.keys().contains(QString::fromStdString(property->name)))
                                    {
                                        property->value = map_SamplesProperties.value(QString::fromStdString(property->name)).toStdString();
                                    }
                                }

                                if(map_SamplesProperties.keys().contains("type"))
                                {
                                    str = "";
                                    text = map_SamplesProperties.value("type");
                                    if(text == "pcr+") str = "ControlPositive";
                                    if(text == "pcr-") str = "ControlNegative";
                                    if(!str.isEmpty())
                                    {
                                        foreach(sample, group->samples)
                                        {
                                            property = new rt_Preference();
                                            property->name = "kind";
                                            property->value = str.toStdString();
                                            sample->preference_Sample.push_back(property);
                                        }
                                    }
                                }

                                break;
                            }
                        }
                    }
                }
            }

            break;
        }

        map_ID_Name_tests.clear();

        if(!error.isEmpty())
        {
            clear_protocol();   // clear

            message.setStandardButtons(QMessageBox::Ok);
            //message.button(QMessageBox::Ok)->animateClick(15000);
            message.setIcon(QMessageBox::Warning);
            text = tr("Error found while opening XML file: ") + error;
            message.setText(text);
            message.exec();
        }
        else        // load protocol
        {
            Load_Protocol();
        }
    }
}

//-----------------------------------------------------------------------------
//--- open_protocol
//-----------------------------------------------------------------------------
QString Setup::open_protocol(QString dir, QString fn, bool temporary_file)
{
    int i,j;
    int sts;
    QString fileName = "";    
    QString dirName = user_Dir.absolutePath();    
    QString selectedFilter;    
    rt_Test *ptest, *ptest_copy;
    rt_Preference *preference;
    rt_GroupSamples *group;
    QString xml_str;
    bool verification;


    if(!dir.isEmpty()) dirName = dir;    

    if(fn.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this, tr("Open Protocol(Template)"),
                                dirName,
                                tr("Protocols File (*.rt *.trt)"),
                                &selectedFilter/*,
                                QFileDialog::DontUseNativeDialog*/);
    }
    else fileName = fn;

    if(fileName.isEmpty()) return("");

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    label_gif->setVisible(true);
    obj_gif->start();
    QApplication::processEvents();


    Tubes_key.clear();
    Tubes_kind.clear();
    prot->Clear_Protocol();
    //clear_protocol(temporary_file);
    sts = Read_XML(this,NULL,prot,fileName,"",true);

    label_gif->setVisible(false);
    obj_gif->stop();
    QApplication::restoreOverrideCursor();

    if(sts == -1)
    {
        QMessageBox::warning(this,tr("Open Protocol"),tr("Unable open protocol..."));
        return("");
    }

    //... Samples Properties
    foreach(group, prot->Plate.groups)
    {
        if(group->preference_Group.size() == 0) Create_GroupSamplesProperties(group);
    }


    //... Clear Web_Protocol property
    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        preference = prot->preference_Pro.at(i);
        if(preference->name == "Web_Protocol")
        {
            delete preference;
            prot->preference_Pro.erase(prot->preference_Pro.begin() + i);
            break;
        }
    }

    //... Tests Validation
    for(i=0; i<prot->tests.size(); i++)
    {
        ptest = prot->tests.at(i);
        for(j=0; j<ptest->preference_Test.size(); j++)
        {
            preference = ptest->preference_Test.at(j);
            if(preference->name == "xml_node")
            {
                xml_str = QString::fromStdString(preference->value);
                verification = false;
                if(axgp)
                {
                    verification = axgp->dynamicCall("loadXmlTest(QString)", xml_str).toBool();
                }
                if(verification) ptest->header.Hash = "ok";
                else ptest->header.Hash = "none";

                break;
            }
        }
    }

    // Check Tests on Expositions...
    if(!temporary_file) Check_TestsExposition(prot);

    // Check Tests on Program
    if(!temporary_file) Check_TestsPtrogramm(prot);

    // Check Tests on Volume
    if(!temporary_file) Check_TestsVolumeTube(prot);    

    // Load protocol
    Load_Protocol();

    //... add Protocol to List
    if(!temporary_file) addProtocol_toList(fileName);

    if(!temporary_file)
    {
        Clear_UNDO();

        if(ADD_Sample->user_Tests) open_Label(tr("Tests from protocol..."));
        ADD_Sample->user_Tests = false;
        qDeleteAll(Pro_TESTs.begin(),Pro_TESTs.end());
        Pro_TESTs.clear();
        foreach(ptest, prot->tests)
        {
            ptest_copy = new rt_Test();
            CreateCopy_Test(ptest, ptest_copy);
            Pro_TESTs.append(ptest_copy);
        }

        //... Translate Catalog ...
        Translate_Catalog(&Pro_TESTs);
        //...


    }

    return(fileName);
}
//-----------------------------------------------------------------------------
//--- close_Label()
//-----------------------------------------------------------------------------
void Setup::close_Label()
{
    LabelTests_ADDSample->setVisible(false);
}
//-----------------------------------------------------------------------------
//--- open_Label(QString s)
//-----------------------------------------------------------------------------
void Setup::open_Label(QString s)
{
    LabelTests_ADDSample->setText(s);
    QFontMetrics fm(LabelTests_ADDSample->font());
    int width = fm.width(LabelTests_ADDSample->text()) * 1.2;
    LabelTests_ADDSample->setFixedWidth(width);
    QPoint P = sampleToolBar->pos();
    P.ry()+= 22;
    LabelTests_ADDSample->move(P);
    LabelTests_ADDSample->setVisible(true);
    QTimer::singleShot(2000, this, SLOT(close_Label()));
    ADD_Sample->repaint();
}

//-----------------------------------------------------------------------------
//--- Load_Protocol()
//-----------------------------------------------------------------------------
void Setup::Load_Protocol()
{
    QString text = LIST_DEVICE;
    QStringList list_dev = text.split(",");
    int id;
    rt_Test *ptest;
    QStringList list_tr;


    //... date/time if create Protocol
    if(prot->time_created.empty())
    {
        prot->time_created = QDateTime::currentDateTime().toString("d-M-yy_hh:mm:ss").toStdString();
    }

    //... Name of Protocol    
    if(prot->name.empty()) Name_Protocol->setText(Generate_NameProtocol(prot->count_Tubes));
    else Name_Protocol->setText(QString::fromStdString(prot->name));
    Generate_IDProtocol(prot);

    //... Translate Tests and Research
    if(ID_LANG != ID_LANG_RU)
    {
        foreach(ptest, prot->tests)
        {
            if(ptest->header.Type_analysis > 0x20000)
            {
                Create_TranslateList(ptest, list_tr);
            }
        }
        if(list_tr.size()) Translate_Tests(list_tr);
    }    
    //...

    //... Samples ...
    //Name_Protocol->setText(QString::fromStdString(prot->name));
    fill_SampleTable();
    fill_TestSample();
    fill_Information();
    fill_SampleProperties();

    //... Program ...
    Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
    plot_Program->draw_Program(prot);
    scheme_Program->draw_Program(prot);

    //...Plate ...
    fill_Plate();
    text = QString::number(prot->count_Tubes);
    //id = type_plate->findText(text);
    id = list_dev.indexOf(text);
    type_plate->blockSignals(true);
    type_plate->setCurrentIndex(id);
    type_plate->blockSignals(false);

    //... change status editable protocol
    change_StatusEditProtocol(false);

    //... status of the protocol
    prot->state = mWait;
}

//-----------------------------------------------------------------------------
//--- save_protocol
//-----------------------------------------------------------------------------
void Setup::save_protocol()
{
    int i;
    QString text_attention = "";
    rt_Tube *tube;

    message.setStandardButtons(QMessageBox::Ok);
    message.setIcon(QMessageBox::Warning);
    message.button(QMessageBox::Ok)->animateClick(8000);

    // Check valid protocol...
    if(!prot->Plate.groups.size())                              // check samples
    {
        text_attention += tr("Protocol is Empty!");
    }

    if(prot->program.size() < 2 && text_attention.isEmpty())       // check program
    {
        text_attention += tr("Program is Empty!");
    }
    if(text_attention.isEmpty())                                // check plate
    {
        for(i=0; i<Tubes_key.size(); i++)
        {
            tube = Tubes_key.values().at(i);
            if(tube->pos < 0)
            {
                text_attention += tr("Tubes location is not valid!");
                break;
            }
        }
    }

    if(!text_attention.isEmpty())
    {
        message.setText(text_attention);
        message.exec();
        return;
    }



    //QString fileName = QString("Protocol(%1)_%2.rt").arg(prot->count_Tubes).arg(QDateTime::currentDateTime().toString("d-M-yy_H-m-s"));
    QString fileName;
    if(QString::fromStdString(prot->name).trimmed().isEmpty()) prot->name = Generate_NameProtocol(prot->count_Tubes).toStdString();
    fileName = Check_ValidNameFile(QString::fromStdString(prot->name));


    QString dirName = user_Dir.absolutePath() + "/" + fileName;

    QString selectedFilter;

    /*Template_FileDialog *dialog = new Template_FileDialog(this);
    dialog->template_protocol->setVisible(true);
    dialog->getSaveFileName(this, "SAVE", dirName, tr("Protocols File (*.rt)"), &selectedFilter);
    */

    fileName = QFileDialog::getSaveFileName(this, tr("Save as Template of Protocol"),
                            dirName,
                            tr("Protocols File (*.trt)"),
                            &selectedFilter);

    //delete dialog;

    /*QFileDialog *mydialog_ = new QFileDialog(this);
    mydialog_->setOptions(QFileDialog::DontUseNativeDialog);
    QGridLayout *grid = qobject_cast<QGridLayout*>(mydialog_->layout());
    QCheckBox *cb = new QCheckBox(this);

    int row = grid->rowCount();
    qDebug() << "row: " << row;
    grid->addWidget(cb,row,0);
    cb->setVisible(true);
    //mydialog_->layout()->addWidget(cb);


    fileName = mydialog_->getSaveFileName(this, tr("Save Protocol"),
                                          dirName,
                                          tr("Protocols File (*.rt)"),
                                          &selectedFilter);
    fileName = mydialog_->exec();

    delete cb;
    delete mydialog_;*/


    if(fileName.isEmpty()) return;

    SaveAsXML(this,NULL,prot,fileName,"",true);
}

//-----------------------------------------------------------------------------
//--- clear_protocol
//-----------------------------------------------------------------------------
void Setup::clear_protocol(bool temporary_file)
{
    if(!temporary_file) save_undo();        // UNDO clear protocol

    Tubes_key.clear();
    Tubes_kind.clear();
    prot->Clear_Protocol();
    change_plate(type_plate->currentIndex(), true);   // in this call fill_Plate()

    fill_SampleTable();
    fill_TestSample();
    fill_Information();
    fill_SampleProperties();

    Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
    //plot_Program->draw_Program(prot);
    //scheme_Program->draw_Program(prot);
    plot_Program->clear_Program();
    scheme_Program->clear_scheme();

    change_StatusEditProtocol(true);

    Clear_UNDO();   // Clear ALL

    if(!ADD_Sample->user_Tests) open_Label(tr("Tests from User..."));
    ADD_Sample->user_Tests = true;
    ADD_Sample->repaint();

}
//-----------------------------------------------------------------------------
//--- copy_block_tests
//-----------------------------------------------------------------------------
void Setup::copy_block_tests()
{
    if(prot->Plate.groups.size())
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(5000);
        QString text_attention = tr("You must clean Protocol...");
        message.setText(text_attention);
        message.exec();
        return;
    }

    copy_block = new Copy_BlockTests(this);
    copy_block->TESTs = &TESTs;
    copy_block->ax_user = ax_user;
    copy_block->ax_rita = ax_rita;
    copy_block->axgp = axgp;
    copy_block->user_dir = &user_Dir;
    copy_block->map_TestTranslate = &Map_TestTranslate;

    copy_block->exec();
    delete copy_block;

    Translate_Catalog(&TESTs);  // update Catalog if create new tests

}

//-----------------------------------------------------------------------------
//--- edit tests
//-----------------------------------------------------------------------------
void Setup::edit_tests()
{
    int i,j;
    rt_Test *ptest;
    rt_Preference *ppref;
    QString dir = qApp->applicationDirPath() + "/forms";
    QString version = "9.0";
    QFont f = qApp->font();

    if(prot->Plate.groups.size())
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(5000);
        QString text_attention = tr("You must clean Protocol...");
        message.setText(text_attention);
        message.exec();
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    label_gif->setVisible(true);
    obj_gif->start();
    QApplication::processEvents();

    test_editor = new Test_editor(this);    
    test_editor->From_Protocol = false;
    test_editor->TESTs = &TESTs;
    test_editor->ax_user = ax_user;
    test_editor->axgp = axgp;
    test_editor->User_Folder = user_Dir.absolutePath();
    //test_editor->Translate_Catalog();
    ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");


    initialize = (Init)(::GetProcAddress(ext_dll_handle,"Initialize"));
    set_font = (SetFont)(::GetProcAddress(ext_dll_handle,"SetFont"));
    set_folder = (SetUserFolder)(::GetProcAddress(ext_dll_handle,"SetUserDefaultFolder"));

    if(initialize)
    {
        initialize(NULL, dir.toLatin1().data(), version.toLatin1().data(), ID_LANG, NULL,NULL,NULL,NULL,NULL,NULL);
    }
    if(set_font) set_font(f.family().toLatin1().data(), "", f.pointSize());
    if(set_folder) set_folder(user_Dir.absolutePath().toUtf8().data());
    QApplication::processEvents();

    if(ext_dll_handle)
    {
        test_editor->ext_dll_handle = ext_dll_handle;
        test_editor->LoadTest_ExtPlugins();
    }

    //setWindowOpacity(0.5);
    test_editor->map_TestTranslate = &Map_TestTranslate;

    label_gif->setVisible(false);
    obj_gif->stop();
    QApplication::restoreOverrideCursor();

    test_editor->exec();
    delete test_editor;
    //setWindowOpacity(1);

    if(ext_dll_handle) ::FreeLibrary(ext_dll_handle);


    /*for(i=0; i<TESTs.size(); i++)
    {
        ptest = TESTs.at(i);
        qDebug() << "setup test:" << QString::fromStdString(ptest->header.Name_Test) << ptest->preference_Test.size();
        for(j=0; j<ptest->preference_Test.size(); j++)
        {
            ppref = ptest->preference_Test.at(j);
            qDebug() << "setup prop:" << QString::fromStdString(ppref->name);
        }
    }*/

}
//-----------------------------------------------------------------------------
//--- edit_preferencePro
//-----------------------------------------------------------------------------
void Setup::edit_preferencePro()
{
    int res;

    Pro_editor = new Common_Pro(this, prot);

    res = Pro_editor->exec();

    delete Pro_editor;
}

//-----------------------------------------------------------------------------
//--- load to Run
//-----------------------------------------------------------------------------
void Setup::load_torun()
{
    int i;    
    QString text_attention = "";
    QString fn;
    rt_Tube *tube;

    message.setStandardButtons(QMessageBox::Ok);
    message.setIcon(QMessageBox::Warning);
    message.button(QMessageBox::Ok)->animateClick(8000);

    // Check valid protocol...
    if(!prot->Plate.groups.size())                              // check samples
    {
        text_attention += tr("Protocol is Empty!");
    }

    if(prot->program.size() < 2 && text_attention.isEmpty())       // check program
    {
        text_attention += tr("Program is Empty!");
    }
    if(text_attention.isEmpty())                                // check plate
    {        
        for(i=0; i<Tubes_key.size(); i++)
        {
            tube = Tubes_key.values().at(i);
            if(tube->pos < 0)
            {
                text_attention += tr("Tubes location is not valid!");
                break;
            }
        }
    }   

    if(!text_attention.isEmpty())
    {
        message.setText(text_attention);
        message.exec();
        return;
    }

    fn = dir_temp.path() + "/manual_setup.rt";
    if(SaveAsXML(this,NULL,prot,fn,"",true))
    {
        parentWidget()->setProperty("pcrdocument", QVariant(fn));
        QEvent *e = new QEvent((QEvent::Type)3001);
        QApplication::sendEvent(this->parentWidget(), e);
    }
    else
    {

    }
}

//-----------------------------------------------------------------------------
//--- change_ToAnalysis(int)
//-----------------------------------------------------------------------------
void Setup::change_ToAnalysis(int state)
{
    if(static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) ToAnalysis = true;
    else ToAnalysis = false;
}

//-----------------------------------------------------------------------------
//--- load to Analysis
//-----------------------------------------------------------------------------
void Setup::load_toanalysis()
{
    int i;
    int res;
    QString text_attention = "";
    QString fn;
    rt_Tube *tube;

    message.setStandardButtons(QMessageBox::Ok);
    message.setIcon(QMessageBox::Warning);
    message.button(QMessageBox::Ok)->animateClick(8000);

    // Check valid protocol...
    if(!prot->Plate.groups.size())                              // check samples
    {
        text_attention += tr("Protocol is Empty!");
    }
    if(!prot->program.size() && text_attention.isEmpty())       // check program
    {
        text_attention += tr("Program is Empty!");
    }
    if(text_attention.isEmpty())                                // check plate
    {
        for(i=0; i<Tubes_key.size(); i++)
        {
            tube = Tubes_key.values().at(i);
            if(tube->pos < 0)
            {
                text_attention += tr("Tubes location is not valid!");
                break;
            }
        }
    }

    if(!text_attention.isEmpty())
    {
        message.setText(text_attention);
        message.exec();
        return;
    }

    if(!ToAnalysis)
    {
        res = msgbox_ToAnalysis.exec();
        if(res == QMessageBox::Cancel) return;
    }

    fn = dir_temp.path() + "/from_setup_to_analysis.rt";
    if(SaveAsXML(this,NULL,prot,fn,"",true))
    {
        parentWidget()->setProperty("setup_to_analysis", QVariant(fn));
        QEvent *e = new QEvent((QEvent::Type)3007);
        QApplication::sendEvent(this->parentWidget(), e);
    }
    else
    {

    }
}

//-----------------------------------------------------------------------------
//--- load to Pipet
//-----------------------------------------------------------------------------
void Setup::load_topipet()
{
    QString text;
    QString fn, fn_in;
    bool sts;

    if(!prot->Plate.groups.size())
    {
        message.setStandardButtons(QMessageBox::Ok);
        text = tr("Protocol is Empty!");
        message.setIcon(QMessageBox::Warning);
        message.button(QMessageBox::Ok)->animateClick(8000);
        message.setText(text);
        message.exec();
        return;
    }

    fn_in = dir_temp.path() + "/DB_setup.rt";
    SaveAsXML(this,NULL,prot,fn_in,"",true);

    QFile file(fn_in);
    //qDebug() << file.exists();


    //... MainWindow ...
    ApplSettings->beginGroup("Pipet");

    text = ApplSettings->value("path_dir","").toString();

    if(!text.isEmpty())
    {
        fn = text + "/DB_setup.rt";
    }
    else fn = dir_temp.path() + "/DB_setup.rt";

    ApplSettings->endGroup();
    //...
    //qDebug() << fn;


    /*QString selectedFilter;

    fn = QFileDialog::getSaveFileName(this, tr("Save Protocol"),
                            qApp->applicationDirPath(),
                            tr("Protocols File (*.rt)"),
                            &selectedFilter);


    qDebug() << fn;*/

    sts = CopyFile(fn_in.toStdWString().c_str(), fn.toStdWString().c_str(), false);

    message.setStandardButtons(QMessageBox::Ok);
    //message.button(QMessageBox::Ok)->animateClick(8000);
    if(file.exists() && sts)
    {
        message.setIcon(QMessageBox::Information);
        text = tr("File successfully written!") + "\r\n" + fn;
    }
    else
    {
        message.setIcon(QMessageBox::Warning);
        text = tr("File is NOT saved!") + "\r\n" + fn;
    }

    message.setText(text);
    message.exec();

}

//-----------------------------------------------------------------------------
//--- last Protocols
//-----------------------------------------------------------------------------
void Setup::last_protocols()
{
    int i;
    QMenu menu;
    QString text;
    QAction *act = NULL;
    QStringList list_fn;
    QFileInfo fi;

    if(StyleApp == "fusion") menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");

    ApplSettings->beginGroup("LastProtocols");
    QStringList list = ApplSettings->childKeys();

    menu.setFont(qApp->font());

    for(i=0; i<list.count(); i++)
    {
        text = ApplSettings->value(list.at(i),"").toString();
        if(text.isEmpty()) continue;
        // check of file exists...
        fi.setFile(text);
        if(!fi.exists()) continue;
        list_fn.append(text);
        //...
        act = new QAction(text, this);
        menu.addAction(act);        
    }
    ApplSettings->endGroup();

    /*if(list.size() != list_fn.size())
    {
        ApplSettings->remove("LastProtocols");
        ApplSettings->beginGroup("LastProtocols");
        for(i=0; i<list_fn.count(); i++)
        {
            text = list.at(i).trimmed();
            ApplSettings->setValue(QString::number(i), text);
        }
        ApplSettings->endGroup();
    }*/

    text = "";
    if(menu.actions().size())
    {
        act = menu.exec(QCursor::pos());
        if(act) text = act->text();
    }

    if(act)
    {
        QFile file(text);
        if(file.exists())
        {
            open_protocol(NULL, text);
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
//--- open_program
//-----------------------------------------------------------------------------
void Setup::open_program(QString fn)
{
    int i;
    rt_Protocol *prot_temp;
    QString text;
    QStringList list;
    QString fileName = "";
    //QDir ApplDir(qApp->applicationDirPath());
    //QString dirName = ApplSettings->value("OpenDir",ApplDir.dirName()).toString();
    QString dirName = user_Dir.absolutePath();

    QString selectedFilter;

    if(fn.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(this, tr("Open Program"),
                                dirName,
                                tr("Programs file (*.rta);;Protocols File (*.rt *.trt)"),
                                &selectedFilter);
    }
    else fileName = fn;
    if(fileName.isEmpty()) return;

    prot_temp = Create_Protocol();

    QFile file(fileName);
    QFileInfo file_info(fileName);
    if(file_info.suffix() == "rta")
    {
        if(file.open(QIODevice::ReadOnly))
        {
            QTextStream t_str(&file);
            text = t_str.readAll();
            file.close();
            list = text.split("\r\n");
            prot_temp->program.clear();
            for(i=0; i<list.size(); i++)
            {
                text = list.at(i);
                prot_temp->program.push_back(text.toStdString());
            }
        }
    }
    else Read_XML(this,NULL,prot_temp,fileName,"",true);


    prot->program = prot_temp->program;
    Parsing_ProgramAmplification(prot);
    Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
    plot_Program->draw_Program(prot);
    scheme_Program->draw_Program(prot);

    prot_temp->Clear_Protocol();
    delete prot_temp;

    addProgram_toList(fileName);

    //---
    Check_ProtocolProgramm_TestsProgramm(prot);
}
//-----------------------------------------------------------------------------
//--- last Programs
//-----------------------------------------------------------------------------
void Setup::last_programs()
{
    int i;
    QMenu menu;
    QString text;
    QAction *act = NULL;
    QStringList list_fn;
    QFileInfo fi;

    if(StyleApp == "fusion") menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");

    ApplSettings->beginGroup("LastPrograms");
    QStringList list = ApplSettings->childKeys();

    menu.setFont(qApp->font());

    for(i=0; i<list.count(); i++)
    {
        text = ApplSettings->value(list.at(i),"").toString();
        if(text.isEmpty()) continue;
        // check of file exists...
        fi.setFile(text);
        if(!fi.exists()) continue;
        list_fn.append(text);
        //...
        act = new QAction(text, this);
        menu.addAction(act);
    }
    ApplSettings->endGroup();

    /*if(list.size() != list_fn.size())
    {
        ApplSettings->remove("LastPrograms");
        ApplSettings->beginGroup("LastPrograms");
        for(i=0; i<list_fn.count(); i++)
        {
            text = list.at(i).trimmed();
            ApplSettings->setValue(QString::number(i), text);
        }
        ApplSettings->endGroup();
    }*/

    text = "";
    if(menu.actions().size())
    {
        act = menu.exec(QCursor::pos());
        if(act) text = act->text();
    }    

    if(act)
    {
        QFile file(text);
        if(file.exists())
        {
            open_program(text);
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
//--- edit program separate
//-----------------------------------------------------------------------------
void Setup::edit_program_separate()
{
    QString fileName = "";
    QString selectedFilter;
    QString text = "";
    bool ok = false;
    std::string s;

    if(dll_editor)
    {
        Interface_Editor editor_func = reinterpret_cast<Interface_Editor>(
                           ::GetProcAddress(dll_editor,"create_editor@32"));
        if(editor_func)
        {
            editor_func("", ok, s);
            if(ok)
            {
                text = QString::fromStdString(s);
                fileName = QFileDialog::getSaveFileName(this, tr("Save Program in File"),
                                        "",
                                        tr("File with Program (*.rta)"),
                                        &selectedFilter);

                if(fileName.isEmpty()) return;

                QFile file(fileName);
                if(file.open(QIODevice::WriteOnly))
                {
                    QTextStream(&file) << text;
                    file.close();
                }
                else
                {
                    qDebug() << "error in open file...";
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
//--- edit program
//-----------------------------------------------------------------------------
void Setup::edit_program()
{
    int i;
    bool ok = false;
    std::string s;
    QString text = "";
    QVector<string> pro;
    QStringList list;

    pro = QVector<string>::fromStdVector(prot->program);
    for(i=0; i<pro.size(); i++)
    {
        if(i) text += "\r\n";
        text += QString::fromStdString(pro.at(i));
    }


    if(dll_editor)
    {
        Interface_Editor editor_func = reinterpret_cast<Interface_Editor>(
                           ::GetProcAddress(dll_editor,"create_editor@32"));
        if(editor_func)
        {
            editor_func(text.toStdString(), ok, s);

            if(ok)
            {
                text = QString::fromStdString(s);

                prot->program.clear();
                list = text.split("\r\n");

                for(i=0; i<list.size(); i++)
                {
                    text = list.at(i);
                    prot->program.push_back(text.toStdString());                    
                }
                Parsing_ProgramAmplification(prot);
                Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
                plot_Program->clear_Program();
                plot_Program->draw_Program(prot);
                scheme_Program->draw_Program(prot);

                //---
                Check_ProtocolProgramm_TestsProgramm(prot);
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- add test
//-----------------------------------------------------------------------------
void Setup::add_sample()
{    
    if(ADD_Sample->user_Tests) add_tests->TESTs = &TESTs;
    else add_tests->TESTs = &Pro_TESTs;

    add_tests->exec();    
}
//-----------------------------------------------------------------------------
//--- change_NameProtocol
//-----------------------------------------------------------------------------
void Setup::change_NameProtocol(QString text)
{
    int i;
    QChar ch;
    QVector<QChar> vec_char;

    vec_char << '_' << '-' << '(' << ')' << ':';

    if(!prot) return;

    int pos = Name_Protocol->cursorPosition();

    for(i=text.length()-1; i>=0; i--)
    {
        ch = text.at(i);
        if(ch.isLetterOrNumber()) continue;
        if(vec_char.contains(ch)) continue;

        text.remove(i,1);
        pos--;
    }
    if(text.length() > 50) text = text.mid(0,50);

    Name_Protocol->blockSignals(true);
    Name_Protocol->setText(text);
    Name_Protocol->blockSignals(false);

    if(pos >= 0 && pos <= text.size()) Name_Protocol->setCursorPosition(pos);

    prot->name = text.toStdString();
    fill_Information();
}
//-----------------------------------------------------------------------------
//--- add_ForSample
//-----------------------------------------------------------------------------
void Setup::add_ForSample(QString name, QVector<rt_Test*> *list_tests, int count_sample, int kind)
{
    int i,j,k,m;
    int count = 0;
    int id;
    QString base_name;
    QChar ch;
    bool ok;    
    bool cr_test;
    QString text, new_name;
    rt_Test *p_test, *ptest, *ptr;
    rt_GroupSamples *group;
    rt_Sample  *sample;
    QVector<rt_Test*> listProt_tests;
    rt_Preference   *preference;

    foreach(p_test, *list_tests)
    {
        //qDebug() << QString::fromStdString(p_test->header.Name_Test);
        count += p_test->tubes.size();
    }


    if((Tubes_key.size() + count*count_sample) > prot->count_Tubes)
    {
        add_tests->label_gif->setVisible(false);
        add_tests->obj_gif->stop();
        message.setStandardButtons(QMessageBox::Ok);
        message.button(QMessageBox::Ok)->animateClick(8000);
        message.setIcon(QMessageBox::Warning);
        text = tr("Check amount of the added tubes!");
        message.setText(text);
        message.exec();

        emit Operation_Complited();
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    save_undo();        // UNDO add_tests    

    // find test    
    foreach(p_test, *list_tests)
    {
        cr_test = true;
        for(i=0; i<prot->tests.size(); i++)
        {
            ptest = prot->tests.at(i);
            if(p_test->header.Name_Test == ptest->header.Name_Test &&
               p_test->header.Type_analysis == ptest->header.Type_analysis &&
               p_test->header.ID_Test == ptest->header.ID_Test)
            {
                cr_test = false;
                listProt_tests.append(ptest);
                break;
            }
        }
        if(cr_test)
        {
            if(p_test->header.Name_Test == "simple" && p_test->header.Type_analysis == 0 && p_test->header.ID_Test == "-1")
            {
                ptr = new SIMPLE_TEST();
            }
            else
            {
                ptr = new rt_Test();
                CreateCopy_Test(p_test, ptr);
            }

            prot->tests.push_back(ptr);
            listProt_tests.append(ptr);
        }
    }

    //qDebug() << "listProt_tests: " << listProt_tests.size();
    foreach(p_test, listProt_tests)
    {
        if(p_test->header.program.size() > 3)
        {
            //text = QString::fromStdString(p_test->header.program.at(0));
            //if(!text.contains("XPRG")) continue;

            prot->program.clear();
            prot->program = p_test->header.program;
            Parsing_ProgramAmplification(prot);
            Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
            plot_Program->draw_Program(prot);
            scheme_Program->draw_Program(prot);

            break;
        }
    }

    //qDebug() << "tests: " << prot->tests.size();
    //return;
    base_name = name;
    id = 1;
    if(count_sample > 1)
    {
        for(i=name.size()-1; i>=0; i--)
        {
            ch = name.at(i);
            if(ch.isDigit()) continue;
            if(i == name.size()-1) break;

            base_name = name.mid(0,i+1);
            id = name.mid(i+1).toInt(&ok);
            if(!ok) id = 1;
            else id++;
            break;
        }
    }

    // add samples
    for(m=0; m<count_sample; m++)
    {
        new_name = name;
        if(m)
        {
            new_name = QString("%1%2").arg(base_name).arg(id);
            id++;
        }

        group = new rt_GroupSamples();
        group->Unique_NameGroup = new_name.toStdString();
        //Create_GroupSamplesProperties(group);
        {Sleep(5); group->ID_Group = GetRandomString(10).toStdString();}
        prot->Plate.groups.push_back(group);
        foreach(p_test, listProt_tests)
        {
            sample = new rt_Sample();
            group->samples.push_back(sample);
            sample->Unique_NameSample = group->Unique_NameGroup;
            sample->ID_Test = p_test->header.ID_Test;
            sample->p_Test = p_test;

            if(kind && m == count_sample-1)
            {
                preference = new rt_Preference();
                preference->name = "kind";
                preference->value = "ControlPositive";
                if(kind == 2) preference->value = "ControlNegative";
                sample->preference_Sample.push_back(preference);
            }

            {Sleep(5); sample->ID_Sample = GetRandomString(10).toStdString();}

            for(j=0; j<p_test->tubes.size(); j++)
            {
                rt_Tube *tube = new rt_Tube();
                tube->ID_Tube = j;
                tube->pos = -1;
                rt_TubeTest *tube_test = p_test->tubes.at(j);
                tube->color = tube_test->color;

                for(k=0; k<tube_test->channels.size(); k++)
                {
                    rt_ChannelTest *ch_test = tube_test->channels.at(k);
                    rt_Channel *ch = new rt_Channel();
                    ch->ID_Channel = ch_test->ID_Channel;
                    ch->Unique_NameChannel = ch_test->name;
                    tube->channels.push_back(ch);
                }
                sample->tubes.push_back(tube);
            }
        }
        Create_GroupSamplesProperties(group);
    }


    // Sample,Plate tables
    fill_SampleTable();     // sample
    fill_TestSample();      // test&sample
    fill_Information();     // information
    fill_SampleProperties();// information about Samples
    change_auto();          // plate

    // select new sample
    k = Samples_Table->rowCount() - 1;      // last row
    Samples_Table->setCurrentCell(k,1);
    Samples_Table->scrollToBottom();

    QApplication::restoreOverrideCursor();
    add_tests->label_gif->setVisible(false);
    add_tests->obj_gif->stop();

    // Check Tests on Expositions...
    Check_TestsExposition(prot);

    // Check Tests on Program
    Check_TestsPtrogramm(prot);

    // Check Tests on Volume
    Check_TestsVolumeTube(prot);

    //...
    emit Operation_Complited();
}

//-----------------------------------------------------------------------------
//--- add_test
//-----------------------------------------------------------------------------
void Setup::add_test(rt_Test *ptest, int count_samples, int doubles, int Kpos, int Kneg, int St, int St_doubles)
{
    int i,j,k,m;
    int count,num;
    bool ok;
    QString text, name;
    QString ID_Sample;
    rt_Test *p_test;
    rt_Test *p_Tests;
    rt_GroupSamples *group;
    rt_GroupSamples *group_current = NULL;
    rt_Sample  *sample;
    //rt_Tube    *tube;
    rt_Channel *channel;
    rt_Preference *preference;
    bool cr_test;
    bool control;
    int control_countTubes = ptest->tubes.size();
    int num_St = 0;
    int id_Kpos = 0;
    int id_Kneg = 0;
    int id_St;
    int num_sample = 0;
    int N;
    QStringList list_standarts;
    int id_sample = 0;
    bool new_group;
    QString test_name = QString::fromStdString(ptest->header.Name_Test);

    qDebug() << "add_test: " << ptest << count_samples << doubles << Kpos << Kneg << St << St_doubles;
    //qDebug() << "add_test: tubes " << ptest->tubes.size();

    //if(id < 0 || id >= TESTs.size()) return;

    // check count_tubes for control
    foreach(preference, ptest->preference_Test)
    {
        if(preference->name == "ControlTubesCount")
        {
            num = QString::fromStdString(preference->value).toInt(&ok);
            if(ok && num > 0 && num <= ptest->tubes.size()) control_countTubes = num;
        }
    }

    //qDebug() << "control_countTubes: " << control_countTubes;

    //

    TabSamples->setCurrentIndex(0);

    //p_Tests = TESTs.at(id);
    p_Tests = ptest;
    count = p_Tests->tubes.size() * count_samples - (Kpos + Kneg)*(p_Tests->tubes.size() - control_countTubes);

    if((Tubes_key.size() + count) > prot->count_Tubes)
    {
        add_tests->label_gif->setVisible(false);
        add_tests->obj_gif->stop();
        message.setStandardButtons(QMessageBox::Ok);
        message.button(QMessageBox::Ok)->animateClick(8000);
        message.setIcon(QMessageBox::Warning);
        //text = tr("ThermoBlock can be NOT filled!\r\nCheck amount of the added tubes...");
        text = tr("Check amount of the added tubes!");
        message.setText(text);
        message.exec();

        emit Operation_Complited();
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    save_undo();        // UNDO add_tests

    // find test
    cr_test = true;
    for(i=0; i<prot->tests.size(); i++)
    {
        p_test = prot->tests.at(i);
        if(p_test->header.Name_Test == p_Tests->header.Name_Test &&
           p_test->header.Type_analysis == p_Tests->header.Type_analysis &&
           p_test->header.ID_Test == p_Tests->header.ID_Test)
        {
            cr_test = false;
            break;
        }
    }
    if(cr_test)     // add test
    {
        if(p_Tests->header.Name_Test == "simple" && p_Tests->header.Type_analysis == 0 && p_Tests->header.ID_Test == "-1")
        {
           p_test = new SIMPLE_TEST();
        }
        else
        {
            p_test = new rt_Test();
            CreateCopy_Test(p_Tests,p_test);
        }
        prot->tests.push_back(p_test);

        //qDebug()<< "Add test: " << QString::fromStdString(p_test->header.Name_Test);
    }

    // Volume
    int value = ptest->header.Volume_Tube;
    if(value >= 5 && value <= 200)
    {
        prot->volume = value;
    }

    if(!p_test->header.program.empty())
    {
        prot->program.clear();
        prot->program = p_test->header.program;
        Parsing_ProgramAmplification(prot);
        Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
        plot_Program->draw_Program(prot);
        scheme_Program->draw_Program(prot);
    }

    // find last sample
    text = tr("Sample_");
    for(i=prot->Plate.groups.size()-1; i>=0; i--)
    {
        group = prot->Plate.groups.at(i);
        sample = group->samples.at(0);
        name = QString::fromStdString(sample->Unique_NameSample);
        if(name.startsWith(text))
        {
            k = name.indexOf(text);
            name = name.mid(text.length());
            k = name.toInt(&ok);
            if(ok) num_sample = k;
            break;
        }
    }

    // add samples    
    for(i=0; i<count_samples; i++)
    {

        if((Kpos > 0 || Kneg >0) && i >= count_samples - St) name = tr("Control_Group");
        if(St > 0 && i >= count_samples - St - Kpos - Kneg && i < count_samples - Kpos - Kneg) name = tr("Standart_Group");

        N = count_samples - St - Kpos - Kneg - i;
        if(N > 0)   // samples
        {
            if(div(N,doubles).rem == 0) num_sample++;
            name = QString("%1%2").arg(tr("Sample_")).arg(num_sample);
        }

        /* new_group = false;
       if((doubles > 1 &&
           (i < count_samples - Kpos - Kneg - St) &&
            div(count_samples - Kpos - Kneg - St - i, doubles).rem != 0 &&
            prot->Plate.groups.size()) ||
           (St > 0 && St_doubles > 1 &&
           (i >= count_samples - Kpos - Kneg - St && i < count_samples - Kpos - Kneg) &&
            div(count_samples - Kpos - Kneg - i, St_doubles).rem != 0 &&
            prot->Plate.groups.size()))
        {
            // 1. old group
            group = group_current;
        }
        else
        {*/
            // 2. create new group
            group = new rt_GroupSamples();
            group_current = group;
            group->Unique_NameGroup = name.toStdString();
            //Create_GroupSamplesProperties(group);
            {Sleep(5); group->ID_Group = GetRandomString(10).toStdString();}
            prot->Plate.groups.push_back(group);
            new_group = true;
        //}


        sample = new rt_Sample();
        group->samples.push_back(sample);
        sample->Unique_NameSample = group->Unique_NameGroup;
        sample->ID_Test = p_test->header.ID_Test;
        sample->p_Test = p_test;
        if(new_group) Create_GroupSamplesProperties(group);

        Sleep(5);
        sample->ID_Sample = GetRandomString(10).toStdString();

        // Doubles
        if(doubles > 1 && i < (count_samples - Kpos - Kneg - St))
        {
            if(div(count_samples - Kpos - Kneg - St - i, doubles).rem == 0) ID_Sample = QString::fromStdString(sample->ID_Sample);
            //qDebug() << "ID_Sample: " << ID_Sample;

            preference = new rt_Preference();
            preference->name = "copies";
            preference->value = ID_Sample.toStdString();
            sample->preference_Sample.push_back(preference);
        }

        // Standarts
        if(St > 0)
        {
            if(!i)      // only for first
            {
                for(m=0; m<p_test->preference_Test.size(); m++)
                {
                    preference = p_test->preference_Test.at(m);
                    if(preference->name == "Standarts Information")
                    {
                        text = QString::fromStdString(preference->value);
                        list_standarts = text.split("\t");
                        list_standarts = list_standarts.mid(3);
                        break;
                    }
                }
            }

            k = count_samples - i - Kneg - Kpos;
            if(k <= St && k > 0)
            {
                num_St++;
                preference = new rt_Preference();
                preference->name = "kind";
                preference->value = "Standart";
                preference->unit = QString::number(div(num_St-1,St_doubles).quot).toStdString();
                sample->preference_Sample.push_back(preference);

                if(St_doubles > 0)
                {
                    id_St = div(num_St-1,St_doubles).quot+1;
                }
                else id_St = num_St;

                sample->Unique_NameSample = QString(tr("Standart") + QString("_%1").arg(id_St)).toStdString();
                if(St_doubles > 1)
                {
                    if(div(num_St - 1, St_doubles).rem == 0) ID_Sample = QString::fromStdString(sample->ID_Sample);
                    preference = new rt_Preference();
                    preference->name = "copies";
                    preference->value = ID_Sample.toStdString();
                    sample->preference_Sample.push_back(preference);
                }
            }
        }

        // K+,K-
        if(Kpos > 0)
        {
            k = count_samples - Kneg - /*St -*/ i;
            if(k <= Kpos && k > 0)
            {
                //qDebug() << "K+";
                preference = new rt_Preference();
                preference->name = "kind";
                preference->value = "ControlPositive";
                sample->preference_Sample.push_back(preference);
                sample->Unique_NameSample = QString(tr("K+") + "_" + test_name).toStdString();

                id_Kpos++;
                if(Kpos > 1)
                {
                    sample->Unique_NameSample += QString("(%1)").arg(id_Kpos).toStdString();
                }
                //if(sample == group->samples.at(0)) group->Unique_NameGroup = sample->Unique_NameSample;
            }
        }
        if(Kneg > 0)
        {
            k = count_samples - /*St -*/ i;
            if(k <= Kneg && k > 0)
            {
                //qDebug() << "K-";
                preference = new rt_Preference();
                preference->name = "kind";
                preference->value = "ControlNegative";
                sample->preference_Sample.push_back(preference);
                sample->Unique_NameSample = QString(tr("K-") + "_" + test_name).toStdString();

                id_Kneg++;
                if(Kneg > 1)
                {
                    sample->Unique_NameSample += QString("(%1)").arg(id_Kneg).toStdString();
                }
                //if(sample == group->samples.at(0)) group->Unique_NameGroup = sample->Unique_NameSample;
            }
        }        


        //..................
        // for K+ and K- ...
        control = false;
        foreach(preference, sample->preference_Sample)
        {
            if(preference->name == "kind")
            {
                if(preference->value == "ControlNegative" || preference->value == "ControlPositive")
                {
                    control = true;
                }
                else control = false;
                break;
            }
        }
        //..................
        if(control) num = control_countTubes;
        else num = p_test->tubes.size();
        //qDebug() << "i, num: " << i << num << control;


        for(j=0; j<num; j++)
        {
            rt_Tube *tube = new rt_Tube();
            tube->ID_Tube = j;
            tube->pos = -1;             
            rt_TubeTest *tube_test = p_test->tubes.at(j);
            tube->color = tube_test->color;

            for(k=0; k<tube_test->channels.size(); k++)
            {
                rt_ChannelTest *ch_test = tube_test->channels.at(k);
                rt_Channel *ch = new rt_Channel();
                ch->ID_Channel = ch_test->ID_Channel;
                ch->Unique_NameChannel = ch_test->name;
                tube->channels.push_back(ch);

                // Standarts value:
                if(!j)                  // only for first tube
                {
                    if(St > 0 && (count_samples - i - Kneg - Kpos) <= St)
                    {
                        preference = new rt_Preference();
                        preference->name = "Standarts Value";
                        if(div(num_St-1,St_doubles).quot < list_standarts.size())
                        {
                            text = list_standarts.at(div(num_St-1,St_doubles).quot);
                            preference->value = QString(text.split(";").at(k)).toStdString();
                        }
                        else preference->value = "0";

                        ch->preference_Channel.push_back(preference);
                    }
                }
            }
            sample->tubes.push_back(tube);
        }

        //prot->Plate.groups.push_back(group);
    }


    // Sample,Plate tables
    fill_SampleTable();     // sample
    fill_TestSample();      // test&sample
    fill_Information();     // information
    fill_SampleProperties();
    change_auto();          // plate

    // select new sample
    k = Samples_Table->rowCount() - 1;      // last row
    Samples_Table->setCurrentCell(k,1);
    Samples_Table->scrollToBottom();

    QApplication::restoreOverrideCursor();
    add_tests->label_gif->setVisible(false);
    add_tests->obj_gif->stop();

    // Check Tests on Expositions...
    Check_TestsExposition(prot);

    // Check Tests on Program
    Check_TestsPtrogramm(prot);

    // Check Tests on Volume
    Check_TestsVolumeTube(prot);

    //...
    emit Operation_Complited();
}

//-----------------------------------------------------------------------------
//--- add tube
//-----------------------------------------------------------------------------
void Setup::add_tube()
{
    int i,j,k;
    bool sts = false;
    bool ok;
    QString name,text;
    rt_GroupSamples *group;
    rt_Sample *sample;
    rt_Test *simple_test;
    rt_Test *ptest;
    int count = prot->Plate.groups.size();

    TabSamples->setCurrentIndex(0);

    //... find last test in protocol
    if(count > 0)
    {
        group = prot->Plate.groups.at(count-1);
        sample = group->samples.back();
        ptest = sample->p_Test;
        if(ptest->header.Type_analysis != 0)
        {
            add_test(ptest, 1,1,0,0,0,1);
            return;
        }
    }
    //...


    if(Tubes_key.size() >= prot->count_Tubes)
    {
        message.setStandardButtons(QMessageBox::Ok);
        message.button(QMessageBox::Ok)->animateClick(5000);
        message.setIcon(QMessageBox::Warning);
        text = tr("ThermoBlock is already FILLED!");
        message.setText(text);
        message.exec();
        return;
    }

    save_undo();    // UNDO add_tube

    // find test(Simple_Test)
    for(i=0; i<prot->tests.size();i++)
    {
        rt_Test *a = prot->tests.at(i);
        if(/*a->header.Name_Test == "simple" &&*/
           a->header.Type_analysis == 0 &&
           a->header.ID_Test == "-1")
        {
            sts = true;
            simple_test = (SIMPLE_TEST*)a;
            break;
        }
    }

    // add test
    if(!sts)
    {
        simple_test = new SIMPLE_TEST();
        prot->tests.push_back(simple_test);
    }

    // add sample
    name = tr("Sample_");
    text = "1";
    if(count > 0)
    {
        group = prot->Plate.groups.at(count-1);
        text = QString::fromStdString(group->Unique_NameGroup);
        k = text.indexOf(name);
        if(k==0)
        {
            text = text.mid(name.length());
            k = text.toInt(&ok);
            if(ok) {k++; text = QString::number(k);}
            else text = "1";
        }
        else text = "1";
    }
    name += text;

    group = new rt_GroupSamples();
    group->Unique_NameGroup = name.toStdString();    
    {Sleep(5); group->ID_Group = GetRandomString(10).toStdString();}
    sample = new rt_Sample();
    group->samples.push_back(sample);
    sample->Unique_NameSample = group->Unique_NameGroup;
    sample->ID_Test = simple_test->header.ID_Test;
    sample->p_Test = simple_test;
    Create_GroupSamplesProperties(group);

    Sleep(5);
    sample->ID_Sample = GetRandomString(10).toStdString();

    for(i=0; i<simple_test->tubes.size(); i++)
    {
        rt_Tube *tube = new rt_Tube();
        tube->ID_Tube = i;
        tube->pos = -1;
        rt_TubeTest *tube_test = simple_test->tubes.at(i);
        for(j=0; j<tube_test->channels.size(); j++)
        {
            rt_ChannelTest *ch_test = tube_test->channels.at(j);
            rt_Channel *ch = new rt_Channel();
            ch->ID_Channel = ch_test->ID_Channel;
            ch->Unique_NameChannel = ch_test->name;
            tube->channels.push_back(ch);
        }
        sample->tubes.push_back(tube);
    }

    prot->Plate.groups.push_back(group);

    // Volume
    int value = sample->p_Test->header.Volume_Tube;
    if(value >= 5 && value <= 200)
    {
        prot->volume = value;
    }

    // Program
    if(!simple_test->header.program.empty())
    {
        prot->program.clear();
        prot->program = simple_test->header.program;
        Parsing_ProgramAmplification(prot);
        Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
        plot_Program->draw_Program(prot);
        scheme_Program->draw_Program(prot);
    }

    // Sample,Plate tables
    fill_SampleTable();     // sample
    fill_TestSample();      // test&sample
    fill_Information();     // information
    fill_SampleProperties();
    change_auto();          // plate

    // select new sample
    k = Samples_Table->rowCount() - 1;      // last row
    Samples_Table->setCurrentCell(k,1);
    Samples_Table->scrollToBottom();    

    // Check Tests on Expositions...
    Check_TestsExposition(prot);

    // Check Tests on Program
    Check_TestsPtrogramm(prot);

    // Check Tests on Volume
    Check_TestsVolumeTube(prot);
}
//-----------------------------------------------------------------------------
//--- delete sample
//-----------------------------------------------------------------------------
void Setup::delete_sample()
{
    int i,j,k;
    int num = 0;
    bool sts_break = false;
    int count_groups;
    int count_samples;

    save_undo();        // UNDO delete sample

    QString name_Test, id_Test;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Test         *p_test;

    int cur_row = Samples_Table->currentRow();
    int count_g = prot->Plate.groups.size();
    bool isopen = Samples_Table->open_samples;

    TabSamples->setCurrentIndex(0);

    //QMap<QString,QString> active_tests; // Name - key  ID - value;
    QVector<QString> active_tests;

    if(isopen)
    {
        for(i=0; i<count_g; i++)
        {
            group = prot->Plate.groups.at(i);
            for(j=0; j<group->samples.size(); j++)
            {
                num++;
                sample = group->samples.at(j);
                for(k=0; k<sample->tubes.size(); k++)
                {
                    tube = sample->tubes.at(k);
                    if(num == cur_row)
                    {
                        delete sample;
                        group->samples.erase(group->samples.begin()+j);
                        if(group->samples.size() == 0)
                        {
                            delete group;
                            prot->Plate.groups.erase(prot->Plate.groups.begin()+i);
                        }
                        sts_break = true;
                        break;
                    }
                    num++;
                }
                if(sts_break) break;
            }
            if(sts_break) break;
        }
    }
    else
    {
        for(i=0; i<count_g; i++)
        {
            group = prot->Plate.groups.at(i);
            for(j=0; j<group->samples.size(); j++)
            {
                sample = group->samples.at(j);
                if(num == cur_row)
                {
                    delete sample;
                    group->samples.erase(group->samples.begin()+j);
                    if(group->samples.size() == 0)
                    {
                        delete group;
                        prot->Plate.groups.erase(prot->Plate.groups.begin()+i);
                    }
                    sts_break = true;
                    break;
                }
                num++;
            }
            if(sts_break) break;
        }
    }


    //  active tests
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            id_Test = QString::fromStdString(sample->ID_Test);
            name_Test = QString::fromStdString(sample->p_Test->header.Name_Test);
            if(active_tests.contains(name_Test)) continue;
            else active_tests.append(name_Test);
        }
    }
    for(i=prot->tests.size()-1; i>=0; i--)
    {
        p_test = prot->tests.at(i);
        name_Test = QString::fromStdString(p_test->header.Name_Test);
        if(active_tests.contains(name_Test)) continue;
        else
        {
            delete p_test;
            prot->tests.erase(prot->tests.begin()+i);
        }
    }
    active_tests.clear();

    //... Program in the last sample ...
    sts_break = false;
    count_groups = prot->Plate.groups.size();
    for(i=count_groups-1; i>=0; i--)
    {
        group = prot->Plate.groups.at(i);
        count_samples = group->samples.size();
        for(j=count_samples-1; j>=0; j--)
        {
            sample = group->samples.at(j);
            p_test = sample->p_Test;

            // volume
            prot->volume = p_test->header.Volume_Tube;

            if(p_test->header.program.size())
            {
                prot->program.clear();
                prot->program = p_test->header.program;
                Parsing_ProgramAmplification(prot);
                Name_Program->setText(QString::fromStdString(prot->PrAmpl_name));
                plot_Program->draw_Program(prot);
                scheme_Program->draw_Program(prot);
                sts_break = true;
                break;
            }
        }
        if(sts_break) break;
    }
    //...


    // Sample,Plate tables
    fill_SampleTable();     // sample
    fill_TestSample();      // test&sample
    fill_Information();     // information
    fill_SampleProperties();

    // Program
    if(prot->Plate.groups.size() == 0)
    {
        prot->program.clear();
        scheme_Program->clear_scheme();
        plot_Program->clear_Program();
    }

    change_auto();          // plate
    Clear_AfterDelete();

    if(cur_row >= Samples_Table->rowCount()-1) cur_row = Samples_Table->rowCount()-1;
    Samples_Table->setCurrentCell(cur_row,0);
    Samples_Table->setFocus();
}
//-----------------------------------------------------------------------------
//--- delete all sample
//-----------------------------------------------------------------------------
void Setup::delete_all_samples()
{
    int i;

    save_undo();        // UNDO delete all samples

    for(i=0; i<prot->Plate.groups.size(); i++) delete prot->Plate.groups.at(i);
    prot->Plate.groups.clear();
    Tubes_key.clear();
    Tubes_kind.clear();
    for(i=0; i<prot->tests.size(); i++) delete prot->tests.at(i);
    prot->tests.clear();

    // Sample,Plate tables
    fill_SampleTable();     // sample
    fill_TestSample();      // test&sample
    fill_Information();     // information
    fill_SampleProperties();

    // Program
    prot->program.clear();
    plot_Program->clear_Program();
    scheme_Program->clear_scheme();

    change_auto();          // plate
    Clear_AfterDelete();
}

//-----------------------------------------------------------------------------
//--- delete tube
//-----------------------------------------------------------------------------
void Setup::delete_tube()
{
    int i,j,k;
    int num = 0;
    bool sts_break = false;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;

    int cur_row = Samples_Table->currentRow();
    int count_g = prot->Plate.groups.size();

    for(i=0; i<count_g; i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            num++;
            sample = group->samples.at(j);
            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                if(num == cur_row)
                {
                    delete tube;
                    sample->tubes.erase(sample->tubes.begin()+k);
                    if(sample->tubes.size() == 0)
                    {
                        delete sample;
                        group->samples.erase(group->samples.begin()+j);
                        if(group->samples.size() == 0)
                        {
                            delete group;
                            prot->Plate.groups.erase(prot->Plate.groups.begin()+i);
                        }
                    }
                    sts_break = true;
                    break;
                }
                num++;
            }
            if(sts_break) break;
        }
        if(sts_break) break;
    }

    // Sample,Plate tables
    fill_SampleTable();     // sample
    fill_TestSample();      // test&sample
    fill_Information();     // information
    fill_SampleProperties();


    change_auto();          // plate
    Clear_AfterDelete();

    if(cur_row >= Samples_Table->rowCount()-1) cur_row = Samples_Table->rowCount()-1;
    Samples_Table->setCurrentCell(cur_row,0);
    Samples_Table->setFocus();    

}
//-----------------------------------------------------------------------------
//--- save_undo()
//-----------------------------------------------------------------------------
void Setup::save_undo()
{    
    current_UNDO++;
    Undo->setEnabled(true);
    UNDO_Action->setEnabled(true);
    //if(UNDO > count_UNDO) count_UNDO++;

    int num = div(current_UNDO, UNDO).rem;
    QString fn = dir_UNDO.path() + QString("/prot_%1.rt").arg(num);
    SaveAsXML(this,NULL,prot,fn,"",true);

    //qDebug() << "save: " << fn << current_UNDO << num;

}
//-----------------------------------------------------------------------------
//--- undo()
//-----------------------------------------------------------------------------
void Setup::undo()
{
    int num = div(current_UNDO, UNDO).rem;
    QString fn = dir_UNDO.path() + QString("/prot_%1.rt").arg(num);    
    bool remove = false;
    int row;    

    open_protocol(NULL, fn, true);

    QFile file(fn);
    if(file.exists()) remove = file.remove();

    current_UNDO--;
    if(current_UNDO >= 0)
    {
        num = div(current_UNDO, UNDO).rem;
        file.setFileName(dir_UNDO.path() + QString("/prot_%1.rt").arg(num));
    }
    if(!file.exists() || current_UNDO < 0)
    {
        Clear_UNDO();
    }


    if(free_Filling->isChecked() && Samples_Table->open_samples && Samples_Table->currentRow())
    {
        row = Samples_Table->currentRow();
        //qDebug() << "undo: " << free_Filling->isChecked() << row << Samples_Table->open_samples << free_currentTube;

        set_PreviousTube();
    }    

}
//-----------------------------------------------------------------------------
//--- Clear_UNDO()
//-----------------------------------------------------------------------------
void Setup::Clear_UNDO()
{
    QDir dir_temp;
    QStringList list_fn;

    current_UNDO = -1;
    Undo->setEnabled(false);
    UNDO_Action->setEnabled(false);
    dir_temp = QDir(dir_UNDO.path());
    list_fn = dir_temp.entryList(QDir::Files);
    foreach(QString fn, list_fn)
    {
        dir_temp.remove(fn);
    }
}

//-----------------------------------------------------------------------------
//--- All Select
//-----------------------------------------------------------------------------
void Setup::allSelected()
{
    //if(auto_Filling->isChecked()) return;
    if(!Type_Plate->enable_editor) return;
    if(!Type_Plate->ability_select) return;
    if(prot->Plate.groups.size() == 0) return;
    QColor color;
    QMap<int,int> color_Map;

    save_undo();        // UNDO allSelected

    //if(Plate_Delegate->view == v_color) color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
    if(Plate_Delegate->view == v_color)
    {
        Color_ToolButton *button = (Color_ToolButton*)Color_Group->checkedButton();
        color = button->color;
    }

    int  i,j;
    int pos;
    int row, col;
    int count = prot->count_Tubes;
    prot->Plate.PlateSize(count,row,col);


    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            pos = i*col+j;

            switch(Plate_Delegate->view)
            {
            default:
            case v_numeration:
                                prot->enable_tube.at(pos) = !prot->enable_tube.at(pos) & 0x01;
                                break;
            case v_color:
                                if(color.isValid() && prot->enable_tube.at(pos))
                                {
                                    prot->color_tube.at(pos) = color.rgb();
                                    color_Map.insert(pos, color.rgb() & 0xffffff);
                                }
                                break;
            }
        }
    }
    if(color_Map.size()) ChangeColor(color_Map);
    change_Selection();
    Type_Plate->viewport()->update();
}

//-----------------------------------------------------------------------------
//--- Column Moved
//-----------------------------------------------------------------------------
void Setup::columnMoved(int Col)
{
    columnSelected(Col);
    /*Type_Plate->horizontalHeader()->setStyleSheet("::section {" // "QHeaderView::section {"
                                                  "background-color: lightblue;}");
    Type_Plate->horizontalHeader()->viewport()->update();*/
}
//-----------------------------------------------------------------------------
//--- Row Moved
//-----------------------------------------------------------------------------
void Setup::rowMoved(int Col)
{
    rowSelected(Col);
}
//-----------------------------------------------------------------------------
//--- ColumnsSelect
//-----------------------------------------------------------------------------
void Setup::ColumnsSelect(QVector<int> vec_column)
{
    //if(auto_Filling->isChecked()) return;
    if(!Type_Plate->enable_editor) return;
    if(!Type_Plate->ability_select) return;
    if(prot->Plate.groups.size() == 0) return;
    QColor color;
    QMap<int,int> color_Map;

    save_undo();        // UNDO columnSelected

    //if(Plate_Delegate->view == v_color) color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
    if(Plate_Delegate->view == v_color)
    {
        Color_ToolButton *button = (Color_ToolButton*)Color_Group->checkedButton();
        color = button->color;
    }

    int  i;
    int pos;
    int row, col, Col;
    int count = prot->count_Tubes;
    prot->Plate.PlateSize(count,row,col);

    foreach(Col, vec_column)
    {
        for(i=0; i<row; i++)
        {
            pos = div(i,row).rem*col + Col;

            switch(Plate_Delegate->view)
            {
            default:
            case v_numeration:
                                prot->enable_tube.at(pos) = !prot->enable_tube.at(pos) & 0x01;
                                break;
            case v_color:
                                if(color.isValid() && prot->enable_tube.at(pos))
                                {
                                    prot->color_tube.at(pos) = color.rgb();
                                    color_Map.insert(pos, color.rgb() & 0xffffff);
                                }
                                break;
            }
        }
    }
    if(color_Map.size()) ChangeColor(color_Map);
    change_Selection();
    Type_Plate->viewport()->update();
}
//-----------------------------------------------------------------------------
//--- RowsSelect
//-----------------------------------------------------------------------------
void Setup::RowsSelect(QVector<int> vec_row)
{
    //if(auto_Filling->isChecked()) return;
    if(!Type_Plate->enable_editor) return;
    if(!Type_Plate->ability_select) return;
    if(prot->Plate.groups.size() == 0) return;
    QColor color;
    QMap<int,int> color_Map;

    save_undo();        // UNDO rowSelected

    //if(Plate_Delegate->view == v_color) color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
    if(Plate_Delegate->view == v_color)
    {
        Color_ToolButton *button = (Color_ToolButton*)Color_Group->checkedButton();
        color = button->color;
    }

    int  i;
    int pos;
    int row, col, Row;
    int count = prot->count_Tubes;
    prot->Plate.PlateSize(count,row,col);

    foreach(Row, vec_row)
    {
        for(i=0; i<col; i++)
        {
            pos = Row*col + i;

            switch(Plate_Delegate->view)
            {
            default:
            case v_numeration:
                                prot->enable_tube.at(pos) = !prot->enable_tube.at(pos) & 0x01;
                                break;
            case v_color:
                                if(color.isValid() && prot->enable_tube.at(pos))
                                {
                                    prot->color_tube.at(pos) = color.rgb();
                                    color_Map.insert(pos, color.rgb() & 0xffffff);
                                }
                                break;
            }
        }
    }
    if(color_Map.size()) ChangeColor(color_Map);
    change_Selection();
    Type_Plate->viewport()->update();
}

//-----------------------------------------------------------------------------
//--- Column Select
//-----------------------------------------------------------------------------
void Setup::columnSelected(int Col)
{
    //if(auto_Filling->isChecked()) return;
    if(!Type_Plate->enable_editor) return;
    if(!Type_Plate->ability_select) return;
    if(prot->Plate.groups.size() == 0) return;
    QColor color;
    QMap<int,int> color_Map;

    save_undo();        // UNDO columnSelected

    //if(Plate_Delegate->view == v_color) color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
    if(Plate_Delegate->view == v_color)
    {
        Color_ToolButton *button = (Color_ToolButton*)Color_Group->checkedButton();
        color = button->color;
    }

    int  i;
    int pos;
    int row, col;
    int count = prot->count_Tubes;
    prot->Plate.PlateSize(count,row,col);

    for(i=0; i<row; i++)
    {
        pos = div(i,row).rem*col + Col;

        switch(Plate_Delegate->view)
        {
        default:
        case v_numeration:
                            prot->enable_tube.at(pos) = !prot->enable_tube.at(pos) & 0x01;
                            break;
        case v_color:
                            if(color.isValid() && prot->enable_tube.at(pos))
                            {
                                prot->color_tube.at(pos) = color.rgb();
                                color_Map.insert(pos, color.rgb() & 0xffffff);
                            }
                            break;
        }
    }
    if(color_Map.size()) ChangeColor(color_Map);
    change_Selection();
    Type_Plate->viewport()->update();
}

//-----------------------------------------------------------------------------
//--- row Select
//-----------------------------------------------------------------------------
void Setup::rowSelected(int Row)
{
    //if(auto_Filling->isChecked()) return;
    if(!Type_Plate->enable_editor) return;
    if(!Type_Plate->ability_select) return;
    if(prot->Plate.groups.size() == 0) return;
    QColor color;
    QMap<int,int> color_Map;

    save_undo();        // UNDO rowSelected

    //if(Plate_Delegate->view == v_color) color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
    if(Plate_Delegate->view == v_color)
    {
        Color_ToolButton *button = (Color_ToolButton*)Color_Group->checkedButton();
        color = button->color;
    }

    int  i;
    int pos;
    int row, col;
    int count = prot->count_Tubes;
    prot->Plate.PlateSize(count,row,col);

    for(i=0; i<col; i++)
    {
        pos = Row*col + i;

        switch(Plate_Delegate->view)
        {
        default:
        case v_numeration:
                            prot->enable_tube.at(pos) = !prot->enable_tube.at(pos) & 0x01;
                            break;
        case v_color:
                            if(color.isValid() && prot->enable_tube.at(pos))
                            {
                                prot->color_tube.at(pos) = color.rgb();
                                color_Map.insert(pos, color.rgb() & 0xffffff);
                            }
                            break;
        }
    }
    if(color_Map.size()) ChangeColor(color_Map);
    change_Selection();
    Type_Plate->viewport()->update();
}

//-----------------------------------------------------------------------------
//--- cell Select
//-----------------------------------------------------------------------------
void Setup::cellSelected(int Row, int Col)
{
    QColor color;
    QMap<int,int> color_Map;


    if(auto_Filling->isChecked()) return;
    if(prot->Plate.groups.size() == 0) return;
    if(!Type_Plate->enable_editor) return;

    save_undo();        // UNDO cellSelected

    if(Row < 0 || Col < 0)  {change_Selection(); return;}


    int count = prot->count_Tubes;
    int row, col;
    prot->Plate.PlateSize(count,row,col);

    int pos = Col + Row * Type_Plate->columnCount();

    //qDebug() << "pos: " << pos;

    switch(Plate_Delegate->view)
    {
    default:
    case v_numeration:
                        if(free_Filling->isChecked())
                        {
                            if(prot->enable_tube.at(pos) != 1)
                            {
                                prot->enable_tube.at(pos) = 1;
                                set_NextTube(pos);
                            }
                        }
                        else prot->enable_tube.at(pos) = !prot->enable_tube.at(pos) & 0x01;                        
                        break;

    case v_color:
                        if(prot->enable_tube.at(pos))
                        {
                            color = prot->color_tube.at(pos);
                            //color = QColorDialog::getColor(color, this, tr("Select Color"));
                            Color_ToolButton *button = (Color_ToolButton*)Color_Group->checkedButton();
                            color = button->color;

                            if(color.isValid())
                            {
                                prot->color_tube.at(pos) = color.rgb();
                                color_Map.insert(pos, color.rgb() & 0xffffff);
                                ChangeColor(color_Map);
                            }
                        }
                        break;
    }

    change_Selection();
    Type_Plate->viewport()->update();
}
//-----------------------------------------------------------------------------
//--- zoom_plate()
//-----------------------------------------------------------------------------
void Setup::zoom_plate()
{
    QString fn = user_Dir.absolutePath() + "/PlateLocation.png";

    zoom = new ZoomPlate(prot, fn, &Map_TestTranslate, this);
    zoom->exec();

    delete zoom;
}

//-----------------------------------------------------------------------------
//--- change user filling
//-----------------------------------------------------------------------------
void Setup::change_user()
{
    int i,j,k;
    rt_Protocol *prot_temp;    
    QString fileName = "";
    QString selectedFilter;
    QString dirName = user_Dir.absolutePath();    

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;

    if(prot->Plate.groups.size() == 0) return;

    if(templates_Dir.exists()) dirName = templates_Dir.absolutePath();

    fileName = QFileDialog::getOpenFileName(this, tr("Open Protocol(Template)"),
                                    dirName,
                                    tr("Protocols File (*.rt *.trt)"),
                                    &selectedFilter);
    if(fileName.isEmpty()) return;


    prot_temp = Create_Protocol();
    Read_XML(this,NULL,prot_temp,fileName,"",true);

    User_Position.clear();

    if(prot->count_Tubes == prot_temp->count_Tubes)
    {
        save_undo();    // UNDO change_user

        for(i=0; i<prot_temp->Plate.groups.size(); i++)
        {
            group = prot_temp->Plate.groups.at(i);
            for(j=0; j<group->samples.size(); j++)
            {
                sample = group->samples.at(j);
                for(k=0; k<sample->tubes.size(); k++)
                {
                    tube = sample->tubes.at(k);
                    User_Position.append(tube->pos);
                }
            }
        }

        free_Filling->setChecked(false);
        auto_Filling->setChecked(false);
        type_View->setChecked(false);
        change_Position(user_locate);

        Type_Plate->ability_select = true;
    }
    else
    {
        message.setIcon(QMessageBox::Warning);
        message.setStandardButtons(QMessageBox::Ok);
        message.setText(tr("Invalid format(384,96,...) of the protocol!"));
        message.exec();
    }

    delete prot_temp;
}

//-----------------------------------------------------------------------------
//--- change auto filling
//-----------------------------------------------------------------------------
void Setup::change_auto()
{    
    if(auto_Filling->isChecked())
    {
        save_undo();    // UNDO  change_auto

        free_Filling->setChecked(false);
        order_Filling->setEnabled(true);
        change_Position(auto_locate);
    }
    Type_Plate->ability_select = !auto_Filling->isChecked();
}
//-----------------------------------------------------------------------------
//--- change free filling
//-----------------------------------------------------------------------------
void Setup::change_free()
{    
    if(free_Filling->isChecked())
    {
        auto_Filling->setChecked(false);        
        set_FirstTube();
    }
    order_Filling->setEnabled(!free_Filling->isChecked());
    Type_Plate->ability_select = !free_Filling->isChecked();    
}
//-----------------------------------------------------------------------------
//--- change free filling
//-----------------------------------------------------------------------------
void Setup::changED_free()
{
    int i;
    int num;
    rt_Tube *tube;

    //qDebug() << "free_Filling State: " << free_Filling->isChecked() << free_Filling->isEnabled();

    message.setIcon(QMessageBox::Information);
    message.setStandardButtons(QMessageBox::Ok);
    message.button(QMessageBox::Ok)->animateClick(5000);

    bool empty_Tubes = true;
    if(Tubes_key.size()) empty_Tubes = false;
    bool empty_TubesFree = true;
    for(i=0; i<Tubes_key.size(); i++)
    {
        num = Tubes_key.keys().at(i);
        tube = Tubes_key.value(num);
        if(tube->pos < 0)
        {
            empty_TubesFree = false;
            break;
        }
    }

    if(free_Filling->isChecked())
    {
        //Type_Plate->setCursor(Qt::CrossCursor);
        Type_Plate->setCursor(cursor_FREE);
        if(!Samples_Table->open_samples)
        {
            Samples_Table->setSelectionMode(QAbstractItemView::SingleSelection);
            Samples_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
            Samples_Table->corner_btn->click();
        }
        Samples_Table->setCurrentCell(-1,-1);
        Samples_Table->setSelectionMode(QAbstractItemView::NoSelection);
        Samples_Table->setCursor(Qt::ForbiddenCursor);
        Samples_Table->setFocus();        

        if(empty_Tubes)
        {
            message.setIcon(QMessageBox::Warning);
            message.setText(tr("Attention! Tubes are absent!"));
        }
        if(!empty_Tubes && empty_TubesFree)
        {
            message.setIcon(QMessageBox::Warning);
            message.setText(tr("Attention! It is necessary to clear the Plate..."));
        }
        if(!empty_Tubes && !empty_TubesFree) message.setText(tr("Attention! Free fill mode is enabled..."));
        message.exec();
    }
    else
    {
        free_currentTube = -1;
        Samples_Delegate->current_row = -1;

        Samples_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        Samples_Table->setSelectionMode(QAbstractItemView::SingleSelection);
        Samples_Table->clearFocus();
        Type_Plate->setCursor(Qt::ArrowCursor);
        Samples_Table->setCursor(Qt::ArrowCursor);

        message.setText(tr("Attention! Free fill mode is disabled..."));
        if(!empty_Tubes && empty_TubesFree) message.exec();
    }
    Samples_Table->repaint();
}

//-----------------------------------------------------------------------------
//--- change order filling
//-----------------------------------------------------------------------------
void Setup::change_order()
{
    if(order_Filling->isChecked()) order_Filling->setIcon(QIcon(":/images/vertcal.png"));
    else order_Filling->setIcon(QIcon(":/images/horiz.png"));
    if(auto_Filling->isChecked())
    {
        save_undo();        // UNDO change_order
        change_Position(auto_locate);
    }
    if(!auto_Filling->isChecked() && !free_Filling->isChecked())
    {
        save_undo();        // UNDO change_order
        change_Position(simply);
    }
}
//-----------------------------------------------------------------------------
//--- clear plate
//-----------------------------------------------------------------------------
void Setup::clear_plate()
{
    save_undo();        // UNDO clear_plate

    auto_Filling->setChecked(false);
    {free_Filling->setChecked(false); order_Filling->setEnabled(true);}
    Type_Plate->ability_select = true;
    change_Position(clear);
}
//-----------------------------------------------------------------------------
//--- clear view
//-----------------------------------------------------------------------------
void Setup::change_view()
{
    free_Filling->blockSignals(true);

    bool state = type_View->isChecked();

    Color_Box->setVisible(state);

    if(state)
    {
        type_View->setIcon(QIcon(":/images/color_view.png"));
        Plate_Delegate->view = v_color;
        auto_Filling->setChecked(false);

        free_Filling->setChecked(false);
        free_currentTube = -1;
        Samples_Delegate->current_row = -1;
        Samples_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
        Samples_Table->setSelectionMode(QAbstractItemView::SingleSelection);
        Samples_Table->clearFocus();
        Type_Plate->setCursor(Qt::ArrowCursor);
        Samples_Table->setCursor(Qt::ArrowCursor);

        auto_Filling->setDisabled(true);
        free_Filling->setDisabled(true);
        user_Filling->setDisabled(true);
        Type_Plate->ability_select = true;
    }
    else
    {
        type_View->setIcon(QIcon(":/images/numeration_view.png"));
        Plate_Delegate->view = v_numeration;
        auto_Filling->setDisabled(false);
        free_Filling->setDisabled(false);
        user_Filling->setDisabled(false);
    }
    Type_Plate->viewport()->update();



    if(state) Type_Plate->setCursor(cursor_ColorFill);
    else Type_Plate->setCursor(Qt::ArrowCursor);

    free_Filling->blockSignals(false);
}

//-----------------------------------------------------------------------------
//--- change plate
//-----------------------------------------------------------------------------
void Setup::change_plate(int index, bool without_saveUndo)
{
    int i,j,k;
    int key;
    QColor color;
    QStringList list_dev = QString(LIST_DEVICE).split(",");
    //QString text = type_plate->currentText();
    QString text = list_dev.at(index);
    int count_tubes = text.toInt();

    if(!without_saveUndo) save_undo();    // UNDO change plate

    // 1. clear Plate
    prot->count_Tubes = count_tubes;
    prot->enable_tube.clear();
    prot->color_tube.clear();
    color = QColor(Qt::gray);
    key = color.rgb();
    for(i=0; i<prot->count_Tubes; i++)
    {
        prot->color_tube.push_back(key);
        prot->enable_tube.push_back(0);
    }

    /*for(i=0; i<prot->Plate.groups.size(); i++)
    {
        rt_GroupSamples *group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            rt_Sample *sample = group->samples.at(j);
            for(k=0; k<sample->tubes.size(); k++)
            {
                rt_Tube *tube = sample->tubes.at(k);
                tube->pos = -1;
            }
        }
    }*/
    // 2. Clear Samples
        Tubes_key.clear();
        Tubes_kind.clear();
        for(i=0; i<prot->Plate.groups.size(); i++) delete prot->Plate.groups[i];
        prot->Plate.groups.clear();
    //

    auto_Filling->setChecked(false);
    free_Filling->setChecked(false);


    fill_Plate();               //... create Plate ...
    fill_SampleTable();         //...

    Name_Protocol->setText(Generate_NameProtocol(prot->count_Tubes));

    prot->regNumber = "";           // for new generate ID
    Generate_IDProtocol(prot);

}
//-----------------------------------------------------------------------------
//--- change Selection
//-----------------------------------------------------------------------------
void Setup::change_Selection()
{
    int i,pos;
    //int id_tube = 0;
    int count = prot->count_Tubes;
    int row, col;
    prot->Plate.PlateSize(count,row,col);
    QMapIterator<int,rt_Tube*> id_tube(Tubes_key);
    id_tube.toFront();

    if(Plate_Delegate->view == v_color) return;
    if(free_Filling->isChecked()) {change_Position(selected); return;}

    for(i=0; i<count; i++)
    {
        pos = i;
        if(order_Filling->isChecked()) pos = div(i,row).rem*col + div(i,row).quot;
        if(!prot->enable_tube.at(pos)) continue;

        if(!id_tube.hasNext()) {prot->enable_tube.at(pos) = false; continue;}
        id_tube.next();
        id_tube.value()->pos = pos;

        prot->color_tube.at(pos) = id_tube.value()->color;
    }
    while(id_tube.hasNext())
    {
        id_tube.next();
        id_tube.value()->pos = -1;
    }

    change_Position(selected);
}

//-----------------------------------------------------------------------------
//--- change_Position
//-----------------------------------------------------------------------------
void Setup::change_Position(Plate_Action action)
{
    int i,j,k,m,n;
    int count = prot->count_Tubes;
    int col,row;
    int current_col, current_row;
    bool enable;

    prot->Plate.PlateSize(count, row, col);

    rt_GroupSamples *group;
    rt_Sample       *sample;    
    rt_Tube         *tube;

    if(action != clear)
    {
        if(Tubes_key.size() > count)
        {
            action = clear;
            message.setStandardButtons(QMessageBox::Ok);
            message.setIcon(QMessageBox::Warning);
            message.button(QMessageBox::Ok)->animateClick(8000);
            message.setText(tr("Attention! Tubes amount more than places on plate!"));
            message.exec();

            auto_Filling->setChecked(false);
            free_Filling->setChecked(false);
        }
    }

    switch(action)
    {
    default:        break;

    case clear:         // Clear
                    for(i=0; i<count; i++) prot->enable_tube.at(i) = false;
                    for(i=0; i<prot->Plate.groups.size(); i++)
                    {
                        group = prot->Plate.groups.at(i);
                        for(j=0; j<group->samples.size(); j++)
                        {
                            sample = group->samples.at(j);
                            for(k=0; k<sample->tubes.size(); k++)
                            {
                                tube = sample->tubes.at(k);
                                tube->pos = -1;
                            }
                        }
                    }
                    break;

    case auto_locate:   // Auto
                    m = 0;
                    for(i=0; i<count; i++) prot->enable_tube.at(i) = false;
                    for(i=0; i<count; i++) prot->color_tube.at(i) = QColor(Qt::gray).rgb();
                    for(i=0; i<prot->Plate.groups.size(); i++)
                    {
                        group = prot->Plate.groups.at(i);
                        for(j=0; j<group->samples.size(); j++)
                        {
                            sample = group->samples.at(j);
                            for(k=0; k<sample->tubes.size(); k++)
                            {
                                tube = sample->tubes.at(k);
                                n = m;
                                if(order_Filling->isChecked())
                                {
                                    n = div(m,row).rem*col + div(m,row).quot;
                                }
                                tube->pos = n;
                                prot->enable_tube.at(n) = true;
                                prot->color_tube.at(n) = tube->color;
                                m++;
                            }
                        }
                    }
                    break;

    case simply:   // simply locate (Not auto, Not user)
                    n = -1;
                    for(i=0; i<count; i++) prot->color_tube.at(i) = QColor(Qt::gray).rgb();

                    //qDebug() << "enable: " << QVector<short>::fromStdVector(prot->enable_tube);
                    //qDebug() << "orientir: " << order_Filling->isChecked();

                    for(i=0; i<prot->Plate.groups.size(); i++)
                    {
                        group = prot->Plate.groups.at(i);
                        for(j=0; j<group->samples.size(); j++)
                        {
                            sample = group->samples.at(j);
                            for(k=0; k<sample->tubes.size(); k++)
                            {
                                tube = sample->tubes.at(k);
                                if(n >= count-1) break;
                                enable = false;
                                while(1)
                                {
                                    if(n < 0) n = 0;
                                    else
                                    {
                                        if(order_Filling->isChecked())      // vertical
                                        {
                                            current_row = div(n,col).quot;
                                            current_col = div(n,col).rem;
                                            if(current_row >= row-1) {current_row = 0; current_col++;}
                                            else current_row++;
                                            n = current_row*col + current_col;
                                        }
                                        else n++;                           // horizontal
                                    }

                                    //qDebug() << "while: " << n;
                                    if(n >= count) break;
                                    if(prot->enable_tube.at(n)) {enable = true; break;}
                                    if(n == count-1) break;
                                }
                                if(n >= count || !enable) break;

                                //qDebug() << "n: " << n;

                                tube->pos = n;
                                prot->color_tube.at(n) = tube->color;
                            }
                        }
                    }

                    break;

    case selected:
                    if(free_currentTube < 0) free_Filling->setChecked(false);
                    break;

    case user_locate:   // User

                    m = 0;
                    for(i=0; i<count; i++) prot->enable_tube.at(i) = false;
                    for(i=0; i<count; i++) prot->color_tube.at(i) = QColor(Qt::gray).rgb();
                    for(i=0; i<prot->Plate.groups.size(); i++)
                    {
                        group = prot->Plate.groups.at(i);
                        for(j=0; j<group->samples.size(); j++)
                        {
                            sample = group->samples.at(j);
                            for(k=0; k<sample->tubes.size(); k++)
                            {
                                tube = sample->tubes.at(k);
                                if(m < User_Position.size())
                                {
                                    tube->pos = User_Position.at(m);                                    
                                    prot->enable_tube.at(tube->pos) = true;
                                    prot->color_tube.at(tube->pos) = tube->color;
                                }
                                else tube->pos = -1;
                                m++;
                            }
                        }
                    }
                    break;
    }

    fill_Plate();
    fill_SampleTable();
}

//-----------------------------------------------------------------------------
//--- fill_Plate
//-----------------------------------------------------------------------------
void Setup::fill_Plate()
{
    int i,j;
    QStringList header;
    QTableWidgetItem* item;
    int col, row;
    int current_col, current_row;
    int pos;
    int code_ch;
    int kind;
    QString text;
    rt_Tube *tube;

    QMap<int,rt_Tube*>::iterator id_tube;


    prot->Plate.PlateSize(prot->count_Tubes, row, col);

    // 2. create table_plate
    Type_Plate->clear();
    Type_Plate->setRowCount(row);
    Type_Plate->setColumnCount(col);

    Tubes_plate.clear();

    for(i=0; i<row; i++)
    {
        for(j=0; j<col; j++)
        {
            pos = i*col + j;
            item = new QTableWidgetItem();            
            Type_Plate->setItem(i,j,item);
        }
    }

    i = 1;
    for(id_tube = Tubes_key.begin(); id_tube != Tubes_key.end(); ++id_tube)
    {
        if(id_tube.value()->pos >= 0)
        {
            tube = id_tube.value();
            pos = id_tube.value()->pos;

            Tubes_plate.insert(pos, tube);

            current_col = div(pos,col).rem;
            current_row = div(pos,col).quot;
            code_ch = Tube_CodeChannels(id_tube.value());
            kind = Tubes_kind.value(id_tube.key());
            j = i | (code_ch << 16) | (kind << 24);
            text = QString::number(j);
            //qDebug() << "Plate: " << text << current_col << current_row << code_ch << kind;
            Type_Plate->item(current_row,current_col)->setText(text);
        }
        i++;
    }

    //qDebug() << "Tubes_plate: " << Tubes_plate;


    header.clear();
    for(i=0; i<col; i++) header.append(QString::number(i+1));   // Column Header
    Type_Plate->setHorizontalHeaderLabels(header);
    for(i=0; i<col; i++) Type_Plate->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    Type_Plate->horizontalHeader()->setFont(qApp->font());

    header.clear();
    for(i=0; i<row; i++) header.append(QChar(0x41 + i));        // Row Header
    Type_Plate->setVerticalHeaderLabels(header);
    for(i=0; i<row; i++) Type_Plate->verticalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    Type_Plate->verticalHeader()->setFont(qApp->font());

    // calculate A1
    item = Type_Plate->item(0,0);
    Type_Plate->A1 = Type_Plate->visualItemRect(item);

}
//-----------------------------------------------------------------------------
//--- Tube_CodeChannels
//-----------------------------------------------------------------------------
int Setup::Tube_CodeChannels(rt_Tube *tube)
{
    int i,id;
    int res = 0;
    int count = tube->channels.size();
    rt_Channel *channel;

    for(i=0; i<count; i++)
    {
        channel = tube->channels.at(i);
        id = channel->ID_Channel;
        res |= (1<<id);        
    }
    return(res);
}

//-----------------------------------------------------------------------------
//--- setPreviousTube
//-----------------------------------------------------------------------------
void Setup::set_PreviousTube()
{
    bool find = false;
    int num;

    QMap<int,rt_Tube*>::const_iterator id_tube = Tubes_key.find(free_currentTube);

    if(id_tube != Tubes_key.begin() && id_tube.key() == free_currentTube)
    {
        --id_tube;

        num = id_tube.key();
        if(id_tube.value()->pos < 0)
        {
                Samples_Delegate->current_row = num;
                free_currentTube = num;
                find = true;
        }
    }
    if(!find)
    {
        free_currentTube = -1;
        Samples_Delegate->current_row = -1;
        free_Filling->setChecked(false);
    }
}

//-----------------------------------------------------------------------------
//--- set_NextTube
//-----------------------------------------------------------------------------
void Setup::set_NextTube(int pos)
{
    bool find = false;
    int num;


    QMap<int,rt_Tube*>::const_iterator id_tube = Tubes_key.find(free_currentTube);

    if(id_tube != Tubes_key.end() && id_tube.key() == free_currentTube)
    {
        id_tube.value()->pos = pos;        
        prot->color_tube.at(pos) = id_tube.value()->color;
        ++id_tube;

        while(id_tube != Tubes_key.end())
        {
            num = id_tube.key();
            if(id_tube.value()->pos < 0)
            {
                Samples_Delegate->current_row = num;
                free_currentTube = num;
                find = true;
                break;
            }
            id_tube++;
        }

        //Samples_Table->setCurrentCell(free_currentTube,0);
        //Samples_Table->setFocus();        
    }

    if(!find) {free_currentTube = -1; Samples_Delegate->current_row = -1;}
}

//-----------------------------------------------------------------------------
//--- set_FirstTube
//-----------------------------------------------------------------------------
void Setup::set_FirstTube()
{
    bool find = false;    

    int num;
    QMapIterator<int,rt_Tube*> id_tube(Tubes_key);
    id_tube.toFront();

    free_currentTube = -1;

    while(id_tube.hasNext())
    {
        id_tube.next();
        if(id_tube.value()->pos < 0)
        {
            num = id_tube.key();
            Samples_Table->setCurrentCell(num,0);
            Samples_Table->setFocus();
            free_currentTube = num;
            find = true;
            break;
        }
    }
    Samples_Delegate->current_row = free_currentTube;
    if(!find) free_Filling->setChecked(false);
}

//-----------------------------------------------------------------------------
//--- Clear_AfterDelete
//-----------------------------------------------------------------------------
void Setup::Clear_AfterDelete()
{
    int i;
    int pos;

    if(auto_Filling->isChecked()) return;

    int count = prot->count_Tubes;
    QMapIterator<int,rt_Tube*> id_tube(Tubes_key);
    id_tube.toFront();

    for(i=0; i<count; i++) prot->enable_tube.at(i) = false;

    while(id_tube.hasNext())
    {
        id_tube.next();
        pos = id_tube.value()->pos;
        if(pos >= 0) prot->enable_tube.at(pos) = 1;
    }

    fill_Plate();
}
//-----------------------------------------------------------------------------
//--- Load_DropperTableTests()
//-----------------------------------------------------------------------------
void Setup::Load_DropperTableTests()
{
    int i, id;
    bool ok;
    QString text;
    rt_Test *ptest;

    QVector<QString> *vec = new QVector<QString>();

    ApplSettings->beginGroup("DropperTableTests");
    QStringList list = ApplSettings->childKeys();
    for(i=0; i<list.count(); i++)
    {
        text = ApplSettings->value(list.at(i),"").toString();

        foreach(ptest, TESTs)
        {
            if(ptest->header.Name_Test == text.trimmed().toStdString())
            {
                text = QString("%1;%2;%3").arg(list.at(i))
                                          .arg(QString::fromStdString(ptest->header.ID_Test))
                                          .arg(QString::fromStdString(ptest->header.Name_Test));
                vec->append(text);
                break;
            }
        }
    }

    ApplSettings->endGroup();

    prot->Map_Reserve.insert("DropperTableTests", vec);
}

//-----------------------------------------------------------------------------
//--- readSettings
//-----------------------------------------------------------------------------
void Setup::readSettings()
{
    int i;
    QStringList list_str;
    QAbstractButton *a_button;
    Color_ToolButton *button;
    QColor color;
    unsigned int val;
    bool ok;

    //... MainWindow ..........................................................
    ApplSettings->beginGroup("MainWindow");

    int width = (main_spl->size().width() - 4)/2;
    QList<int> list_spl;
    QString text = ApplSettings->value("splitter","").toString();
    if(text.isEmpty()) list_spl << width << width;
    else
    {
        list_str = text.split(" ");
        for(i=0; i<list_str.count(); i++) list_spl.append(list_str.at(i).toInt());
    }
    main_spl->setSizes(list_spl);

    text = ApplSettings->value("program&plate_splitter","").toString();
    if(!text.isEmpty())
    {
        list_str.clear();
        list_spl.clear();
        list_str = text.split(" ");
        for(i=0; i<list_str.count(); i++) list_spl.append(list_str.at(i).toInt());
        program_plate_spl->setSizes(list_spl);
    }

    text = ApplSettings->value("program_splitter","").toString();
    if(!text.isEmpty())
    {
        list_str.clear();
        list_spl.clear();
        list_str = text.split(" ");
        for(i=0; i<list_str.count(); i++) list_spl.append(list_str.at(i).toInt());
        program_spl->setSizes(list_spl);
    }

    text = ApplSettings->value("order_filling","").toString().trimmed();
    if(!text.isEmpty())
    {
        if(text == "ver")
        {
            order_Filling->setChecked(true);
            order_Filling->setIcon(QIcon(":/images/vertcal.png"));
        }
    }


    ApplSettings->endGroup();

    //... ColorRejime ................................................................
    ApplSettings->beginGroup("ColorRejime");
    i = 0;
    foreach(a_button, Color_Group->buttons())
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
    emit Color_Group->buttonClicked(0);
    ApplSettings->endGroup();

}
//-----------------------------------------------------------------------------
//--- writeSettings
//-----------------------------------------------------------------------------
void Setup::writeSettings()
{
    QString text = "";
    int i;
    QAbstractButton *a_button;
    Color_ToolButton *button;
    unsigned int val;


    QColor color;

    if(!flag_SaveLocationWin) return;

    //--- MainWindow ---
    ApplSettings->beginGroup("MainWindow");

    QList<int> list_spl = main_spl->sizes();
    for(i=0; i<list_spl.count(); i++) text += QString::number(list_spl.at(i)) + " ";
    ApplSettings->setValue("splitter", text);

    list_spl.clear();
    text = "";
    list_spl = program_plate_spl->sizes();
    for(i=0; i<list_spl.size(); i++) text += QString::number(list_spl.at(i)) + " ";
    ApplSettings->setValue("program&plate_splitter", text);

    list_spl.clear();
    text = "";
    list_spl = program_spl->sizes();
    for(i=0; i<list_spl.size(); i++) text += QString::number(list_spl.at(i)) + " ";
    ApplSettings->setValue("program_splitter", text);

    text = "hor";
    if(order_Filling->isChecked()) text = "ver";
    ApplSettings->setValue("order_filling", text);

    ApplSettings->endGroup();

    //--- ColorRejime ---
    ApplSettings->beginGroup("ColorRejime");
    i=0;
    foreach(a_button, Color_Group->buttons())
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
//--- ChangeColor(int pos, int color)
//-----------------------------------------------------------------------------
void Setup::ChangeColor(QMap<int, int> color_map)
{

    //qDebug() << "Tubes_key: " << Tubes_key;
    //return;

    int pos, color;

    QMapIterator<int,rt_Tube*> id_tube(Tubes_key);
    id_tube.toFront();

    while(id_tube.hasNext())
    {
        id_tube.next();
        pos = id_tube.value()->pos;

        if(color_map.keys().contains(pos))
        {
            color = color_map.value(pos,0);
            id_tube.value()->color = color;
        }

        /*
        if(id_tube.value()->pos == pos)
        {
            id_tube.value()->color = color;
            break;
        }*/
    }
    fill_SampleTable();
}

//-----------------------------------------------------------------------------
//--- contextMenu_Test_Tree
//-----------------------------------------------------------------------------
void Setup::contextMenu_Test_Tree()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(look_TIS_Tests);
    menu.exec(QCursor::pos());
    menu.clear();
}
//-----------------------------------------------------------------------------
//--- contextMenu_Plate()
//-----------------------------------------------------------------------------
void Setup::contextMenu_Plate(QPoint P)
{
    QTableWidgetItem *item;
    QString text;
    int count = prot->count_Tubes;
    int row,col;

    prot->Plate.PlateSize(count, row, col);

    item = Type_Plate->itemAt(P);
    text = Convert_IndexToName(item->row()*col + item->column(), col);

    //qDebug() << "item: " << item->row() << item->column() << text;

    QMenu menu;
    QAction *act;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    invert_Row->setText(tr("invert row - ") + text.mid(0,1));
    invert_Column->setText(tr("invert column - ") + text.mid(1));
    invert_Plate->setText(tr("invert all Plate"));
    tests_layout->setText(tr("tests layout"));

    menu.addAction(invert_Row);
    menu.addAction(invert_Column);
    menu.addSeparator();
    menu.addAction(invert_Plate);    
    menu.addSeparator();
    menu.addAction(tests_layout);

    act = menu.exec(QCursor::pos());

    if(act == invert_Row) InvertRow(item->row());
    if(act == invert_Column) InvertColumn(item->column());
    if(act == invert_Plate) InvertPlate();
    if(act == tests_layout) Tests_Layout();

    menu.clear();
}

//-----------------------------------------------------------------------------
//--- contextMenu_SamplesTable
//-----------------------------------------------------------------------------
void Setup::contextMenu_SamplesTable()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    bool sts = prot->Plate.groups.size();

    increment_digits->setEnabled(sts);

    QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    if(text.isEmpty()) sts = false;
    paste_from_clipboard->setEnabled(sts);

    send_protocol_to_web->setEnabled(check_ValidProtocol());

    if(Samples_Table->open_samples) as_first_sample->setEnabled(false);
    else as_first_sample->setEnabled(true);

    menu.addAction(paste_from_clipboard);
    menu.addAction(increment_digits);
    menu.addAction(as_first_sample);
    menu.addSeparator();
    menu.addAction(send_protocol_to_web);
    menu.exec(QCursor::pos());
    menu.clear();

}
//-----------------------------------------------------------------------------
//--- contextMenu_SampleProperties()
//-----------------------------------------------------------------------------
void Setup::contextMenu_SampleProperties()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    bool sts = prot->Plate.groups.size();

    paste_SampleProperties->setEnabled(sts);

    menu.addAction(paste_SampleProperties);
    menu.exec(QCursor::pos());
    menu.clear();


}
//-----------------------------------------------------------------------------
//--- SampleProperties_FromAnotherProtocol()
//-----------------------------------------------------------------------------
void Setup::SampleProperties_FromAnotherProtocol()
{
    int id;
    int state;
    QString text, Name;
    QString fileName = "";
    QString dirName = user_Dir.absolutePath();
    QString selectedFilter;

    rt_Preference *property, *property_new;
    rt_GroupSamples *group_source, *group_target;
    rt_Sample       *sample;
    QMap<QString, rt_Preference*> map_property;



    fileName = QFileDialog::getOpenFileName(this, tr("Open Protocol(Template)"),
                                            dirName,
                                            tr("Protocols File (*.rt *.trt)"),
                                            &selectedFilter/*,
                                            QFileDialog::DontUseNativeDialog*/);

    if(fileName.isEmpty()) return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    label_gif->setVisible(true);
    obj_gif->start();
    QApplication::processEvents();


    rt_Protocol *prot_temp = new rt_Protocol();
    state = Read_XML(this,NULL,prot_temp,fileName,"",true);
    if(state == -1)
    {
        label_gif->setVisible(false);
        obj_gif->stop();
        QApplication::restoreOverrideCursor();

        QMessageBox::warning(this,tr("Open Protocol"),tr("Unable open protocol..."));
        delete prot_temp;

        return;
    }

    save_undo();    // save undo...

    id = 0;
    foreach(group_source, prot_temp->Plate.groups)
    {
        if(prot->Plate.groups.size() <= id) break;

        group_target = prot->Plate.groups.at(id);
        map_property.clear();
        foreach(property, group_target->preference_Group)
        {
            map_property.insert(QString::fromStdString(property->name), property);
        }
        Name = QString::fromStdString(group_source->samples.at(0)->Unique_NameSample);

        foreach(property, group_source->preference_Group)
        {
            group_target->Unique_NameGroup = Name.toStdString();
            foreach(sample, group_target->samples)
            {
                sample->Unique_NameSample = Name.toStdString();
            }

            text = QString::fromStdString(property->name);
            if(map_property.contains(text))                     // replace
            {
                *map_property.value(text) = *property;
            }
            else                                                // new
            {
                property_new = new rt_Preference();
                group_target->preference_Group.push_back(property_new);
                *property_new = *property;
            }
        }
        id++;
        QApplication::processEvents();
    }

    map_property.clear();
    delete prot_temp;

    fill_SampleTable();
    fill_TestSample();
    fill_Information();
    fill_SampleProperties();

    label_gif->setVisible(false);
    obj_gif->stop();
    QApplication::restoreOverrideCursor();
}
//-----------------------------------------------------------------------------
//--- check_ValidProtocol()
//-----------------------------------------------------------------------------
bool Setup::check_ValidProtocol()
{
    int i;
    rt_Tube *tube;
    bool sts = true;

    if(prot->Plate.groups.size() && prot->program.size())
    {
        for(i=0; i<Tubes_key.size(); i++)
        {
            tube = Tubes_key.values().at(i);
            if(tube->pos < 0) {sts = false; break;}
        }
    }
    else sts = false;

    return(sts);
}

//-----------------------------------------------------------------------------
//--- look_TISTests
//-----------------------------------------------------------------------------
void Setup::look_TISTests()
{
    int i;
    QVector<QString> *vec;
    QString text;
    QStringList list;
    QTreeWidgetItem *item;
    QMap<int, QString> map;

    QDialog *tis_TESTs = new QDialog(this);
    tis_TESTs->setWindowTitle(tr("TIS tests"));
    tis_TESTs->setWindowFlags(tis_TESTs->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    tis_TESTs->resize(200,300);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(5);
    layout->setSpacing(4);
    tis_TESTs->setLayout(layout);
    QTreeWidget *tests = new QTreeWidget(tis_TESTs);
    layout->addWidget(tests);

    tests->clear();
    tests->setColumnCount(2);

    QStringList pp;
    pp << tr("N") << tr("Name test"); // << tr("ID test");
    tests->setHeaderLabels(pp);

    if(prot->Map_Reserve.contains("DropperTableTests"))
    {
        vec = prot->Map_Reserve.value("DropperTableTests");
        foreach(text, *vec)
        {
            list = text.split(";");
            map.insert(QString(list.at(0)).toInt(), QString(list.at(2)));
        }

        foreach(i, map.keys())
        {
            item = new QTreeWidgetItem(tests);
            item->setText(0, QString("   %1   ").arg(i));
            item->setText(1, map.value(i));
        }

    }

    tests->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tests->header()->setSectionResizeMode(1, QHeaderView::Stretch);


    tis_TESTs->exec();

    map.clear();
    tests->clear();
    delete tests;
    delete tis_TESTs;
}

//prot->Map_Reserve.insert("DropperTableTests", vec);

//-----------------------------------------------------------------------------
//--- InvertRow()
//-----------------------------------------------------------------------------
void Setup::InvertRow(int r)
{
    //qDebug() << "invert row: " << r;

    int i, pos;
    int row,col;
    QVector<int> inv_Row;
    QVector<rt_Tube*> tubes_Row;
    rt_Tube *tube;
    vector<int> color;

    save_undo();

    prot->Plate.PlateSize(prot->count_Tubes, row, col);

    for(i=0; i<col; i++) inv_Row.append(i+r*col);
    i = 0;
    foreach(pos, inv_Row)
    {
        tube = NULL;
        if(Tubes_plate.contains(pos)) tube = Tubes_plate.value(pos);
        tubes_Row.append(tube);
        color.push_back(prot->color_tube.at(pos));
        i++;
    }

    std::reverse(inv_Row.begin(), inv_Row.end());
    //std::reverse(color.begin(), color.end());


    i = 0;
    foreach(pos, inv_Row)
    {
        tube = tubes_Row.at(i);
        if(tube)
        {
            tube->pos = pos;
            prot->enable_tube.at(pos) = true;
        }
        else prot->enable_tube.at(pos) = false;
        prot->color_tube.at(pos) = color.at(i);
        i++;
    }

    fill_Plate();
    fill_SampleTable();
}
//-----------------------------------------------------------------------------
//--- InvertColumn()
//-----------------------------------------------------------------------------
void Setup::InvertColumn(int c)
{
    int i, pos;
    int row,col;
    QVector<int> inv_Col;
    QVector<rt_Tube*> tubes_Col;
    rt_Tube *tube;
    vector<int> color;

    save_undo();

    prot->Plate.PlateSize(prot->count_Tubes, row, col);

    for(i=0; i<row; i++) inv_Col.append(c + i*col);
    i = 0;
    foreach(pos, inv_Col)
    {
        tube = NULL;
        if(Tubes_plate.contains(pos)) tube = Tubes_plate.value(pos);
        tubes_Col.append(tube);
        color.push_back(prot->color_tube.at(pos));
        i++;
    }

    std::reverse(inv_Col.begin(), inv_Col.end());
    //std::reverse(color.begin(), color.end());


    i = 0;
    foreach(pos, inv_Col)
    {
        tube = tubes_Col.at(i);
        if(tube)
        {
            tube->pos = pos;
            prot->enable_tube.at(pos) = true;
        }
        else prot->enable_tube.at(pos) = false;
        prot->color_tube.at(pos) = color.at(i);
        i++;
    }

    fill_Plate();
    fill_SampleTable();
}
//-----------------------------------------------------------------------------
//--- InvertPlate()
//-----------------------------------------------------------------------------
void Setup::InvertPlate()
{
    int i, pos;
    int row,col;
    QVector<int> inv_RowCol;
    QVector<rt_Tube*> tubes_RowCol;
    rt_Tube *tube;
    vector<int> color;

    save_undo();

    prot->Plate.PlateSize(prot->count_Tubes, row, col);

    for(i=0; i<prot->count_Tubes; i++) inv_RowCol.append(i);
    i = 0;
    foreach(pos, inv_RowCol)
    {
        tube = NULL;
        if(Tubes_plate.contains(pos)) tube = Tubes_plate.value(pos);
        tubes_RowCol.append(tube);
        color.push_back(prot->color_tube.at(pos));
        i++;
    }

    std::reverse(inv_RowCol.begin(), inv_RowCol.end());

    i = 0;
    foreach(pos, inv_RowCol)
    {
        tube = tubes_RowCol.at(i);
        if(tube)
        {
            tube->pos = pos;
            prot->enable_tube.at(pos) = true;
        }
        else prot->enable_tube.at(pos) = false;
        prot->color_tube.at(pos) = color.at(i);
        i++;
    }

    fill_Plate();
    fill_SampleTable();

}
//-----------------------------------------------------------------------------
//--- Tests_Layout()
//-----------------------------------------------------------------------------
void Setup::Tests_Layout()
{
    int i,j, pos;
    int row,col;    
    rt_Tube         *tube;
    rt_Sample       *sample;
    rt_GroupSamples *group;
    rt_Test         *ptest;
    vector<int>     color;
    QTableWidgetItem *item;

    QString text;
    QVector<int> Plate_Num;
    QVector<rt_Test*> Plate_Tests;
    QVector<rt_Sample*> Plate_Sample;

    //save_undo();

    prot->Plate.PlateSize(prot->count_Tubes, row, col);    

    // 1. Append vector NUM
    /*for(i=0; i<Samples_Table->rowCount(); i++)
    {
        item = Samples_Table->item(i,0);
        text = item->text();
        if(text.trimmed().isEmpty()) continue;
        if(text.startsWith("Header_Sample:")) continue;
        pos = Convert_NameToIndex(text, col);
        Plate_Num.append(pos);
    }*/
    if(order_Filling->isChecked())      // Vertical
    {
        for(j=0; j<col; j++)
        {
            for(i=0; i<row; i++)
            {
                item = Type_Plate->item(i,j);
                if(item->text().trimmed().isEmpty()) continue;
                pos = j + i*col;
                Plate_Num.append(pos);
            }
        }
    }
    else                                // Horizontal
    {
        for(i=0; i<row; i++)
        {
            for(j=0; j<col; j++)
            {
                item = Type_Plate->item(i,j);
                if(item->text().trimmed().isEmpty()) continue;
                pos = j + i*col;
                Plate_Num.append(pos);
            }
        }
    }
        qDebug() << "Plate_Num: " << Plate_Num;
        //std::sort(Plate_Num.begin(), Plate_Num.end());
        //qDebug() << "Plate_Num_sorted: " << Plate_Num;


    // 2. Clear
    clear_plate();

    // 3. Create vector tests
    foreach(group, prot->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            Plate_Sample.append(sample);
            if(Plate_Tests.contains(sample->p_Test)) continue;
            Plate_Tests.append(sample->p_Test);
            //qDebug() << "test: " << QString::fromStdString(sample->p_Test->header.Name_Test);
        }
    }
    //qDebug() << "Plate_Tests: " << Plate_Tests;

    // 4. Fill Plate
    foreach(ptest, Plate_Tests)
    {
        foreach(sample, Plate_Sample)
        {
            if(sample->p_Test != ptest) continue;

            foreach(tube, sample->tubes)
            {
                if(Plate_Num.empty()) break;

                pos = Plate_Num.at(0);
                tube->pos = pos;
                prot->enable_tube.at(pos) = true;

                Plate_Num.removeFirst();
            }
        }
    }

    Plate_Num.clear();
    Plate_Sample.clear();
    Plate_Tests.clear();

    // 5.
    fill_Plate();
    fill_SampleTable();

    // 6.
    //tests_layout->setDisabled(true);
}
//-----------------------------------------------------------------------------
//--- send_Protocol_To_Web
//-----------------------------------------------------------------------------
void Setup::send_Protocol_To_Web()
{
    QString fn;
    QString id = QString::fromStdString(prot->regNumber);
    QString text;
    if(id.isEmpty()) id = GetRandomString();

    fn = dir_temp.path() + "/order_to_web.rt";
    if(SaveAsXML(this,NULL,prot,fn,"",true))
    {
        text = id + ";" + fn;
        parentWidget()->setProperty("order_to_web", QVariant(text));
        QEvent *e = new QEvent((QEvent::Type)3020);
        QApplication::sendEvent(this->parentWidget(), e);
    }
}
//-----------------------------------------------------------------------------
//--- As_First_Sample()
//-----------------------------------------------------------------------------
void Setup::As_First_Sample()
{
    int i,j;
    int num_row;
    int num = -1;
    int count;

    rt_Sample       *sample_first;
    rt_GroupSamples *group_first;
    rt_Sample       *sample;
    rt_GroupSamples *group;
    QList<QTableWidgetItem*> list_sel;
    QTableWidgetItem *item;
    QVector<int>    list;

    sample_first = NULL;
    group_first = NULL;

    //... selected samples ...
    list_sel = Samples_Table->selectedItems();
    foreach(item, list_sel)
    {
        num_row = item->row();
        if(!list.contains(num_row)) list.append(num_row);
    }
    if(list.isEmpty()) return;

    //qSort(list.begin(),list.end());
    std::sort(list.begin(),list.end());

    save_undo();                        // as first sample

    //... find first sample and group ...
    foreach(group, prot->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            num++;
            if(num == list.at(0))
            {
                sample_first = sample;
                group_first = group;
                continue;
            }

            if(list.contains(num) && sample_first && group_first)
            {
                sample->Unique_NameSample = sample_first->Unique_NameSample;
                group_first->samples.push_back(sample);
                group->samples.erase(std::find(group->samples.begin(), group->samples.end(), sample));
            }
        }
    }

    //... check all groups on isEMPTY ...
    count = prot->Plate.groups.size();    
    for(i=count-1; i>=0; i--)
    {
        group = prot->Plate.groups.at(i);
        if(group->samples.empty())
        {
            prot->Plate.groups.erase(std::find(prot->Plate.groups.begin(), prot->Plate.groups.end(), group));
            delete group;
        }
    }

    //...
    fill_SampleTable();
    fill_TestSample();
    //fill_Information();
    fill_SampleProperties();
    fill_Plate();

}
//-----------------------------------------------------------------------------
//--- Increment_Digits()
//-----------------------------------------------------------------------------
void Setup::Increment_Digits()
{
    int i,j;
    int row;
    int current_sample = -1;
    int num_sample = 0;
    int count_tubes = 0;
    int id = 0;
    QString text;
    QString str_before, str_after, str_digits;
    int digits_val;
    bool ok;
    int pos_s = 0;
    int pos_f = -1;

    rt_GroupSamples *group;
    rt_Sample *sample;

    row = Samples_Table->currentRow();

    if(Samples_Table->open_samples)
    {
        for(i=0; i<prot->Plate.groups.size(); i++)
        {
            group = prot->Plate.groups.at(i);
            for(j=0; j<group->samples.size(); j++)
            {
                num_sample++;
                sample = group->samples.at(j);
                count_tubes += sample->tubes.size() + 1;
                if(row < count_tubes)
                {
                    current_sample = num_sample - 1;
                    text = QString::fromStdString(sample->Unique_NameSample);
                    break;
                }
            }
            if(current_sample >= 0) break;
        }
    }
    else
    {
        current_sample = row;
        for(i=0; i<prot->Plate.groups.size(); i++)
        {
            group = prot->Plate.groups.at(i);
            for(j=0; j<group->samples.size(); j++)
            {
                num_sample++;
                sample = group->samples.at(j);
                if(row == num_sample - 1)
                {
                    text = QString::fromStdString(sample->Unique_NameSample);
                    break;
                }
            }
        }
    }

    //... find digitd info ...
    for(i=text.size()-1; i>=0; i--)
    {
        if(pos_f < 0 && text.at(i).isDigit()) {pos_f = i; continue;}
        if(pos_f >= 0 && !text.at(i).isDigit()) {pos_s = i+1; break;}
    }
    if(pos_s == 0) str_before = "";
    else str_before = text.mid(0, pos_s);
    if(pos_f >= text.size()-1) str_after = "";
    else str_after = text.mid(pos_f+1);
    str_digits = text.mid(pos_s, pos_f - pos_s + 1);
    digits_val = str_digits.toInt(&ok);
    if(!ok) digits_val = 1;
    if(pos_s == 0 && pos_f == -1) {str_before = text; str_after = "";}

    //qDebug() << "Sample TEXT: " << text << str_before << str_digits << str_after << digits_val << pos_s << pos_f;
    //...

    num_sample = -1;
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            num_sample++;
            if(num_sample < current_sample) continue;

            sample = group->samples.at(j);
            text = QString("%1%2%3").arg(str_before).arg(digits_val++).arg(str_after);
            id++;
            if(text.trimmed().isEmpty()) continue;
            sample->Unique_NameSample = text.toStdString();
            if(group->samples.size() == 1) group->Unique_NameGroup = text.toStdString();
        }
    }
    fill_SampleTable();
    fill_TestSample();
    fill_SampleProperties();
}

//-----------------------------------------------------------------------------
//--- from_ClipBoard()
//-----------------------------------------------------------------------------
void Setup::from_ClipBoard()
{
    int i,j;
    int row;
    int current_sample = -1;
    int num_sample = 0;
    int count_tubes = 0;
    int id = 0;

    rt_GroupSamples *group;
    rt_Sample *sample;

    QClipboard *clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    QStringList list = text.split(QRegExp("\n|\r\n|\r"));

    if(!list.size()) return;

    row = Samples_Table->currentRow();

    if(Samples_Table->open_samples)
    {
        for(i=0; i<prot->Plate.groups.size(); i++)
        {
            group = prot->Plate.groups.at(i);
            for(j=0; j<group->samples.size(); j++)
            {
                num_sample++;
                sample = group->samples.at(j);
                count_tubes += sample->tubes.size() + 1;
                if(row < count_tubes)
                {
                    current_sample = num_sample - 1;
                    break;
                }
            }
            if(current_sample >= 0) break;
        }
    }
    else current_sample = row;

    num_sample = -1;
    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            if(id >= list.size()) break;
            num_sample++;
            if(num_sample < current_sample) continue;

            sample = group->samples.at(j);
            text = list.at(id);
            id++;
            if(text.trimmed().isEmpty()) continue;
            sample->Unique_NameSample = text.toStdString();
            if(group->samples.size() == 1) group->Unique_NameGroup = text.toStdString();
        }
    }
    fill_SampleTable();
    fill_TestSample();
    fill_SampleProperties();
}

//-----------------------------------------------------------------------------
//--- PropertyItemChange(QTreeWidgetItem *item, int col)
//-----------------------------------------------------------------------------
void Setup::PropertyItemChange(QTreeWidgetItem *item, int col)
{
    int id;
    QString text, str_value, name;
    rt_GroupSamples *group;    
    rt_Preference   *property;
    QTreeWidgetItem *item_parent = item->parent();

    QStringList list;
    list << PATIENT << SEX << AGE << ORG << PHISIC << DATA << NOTE << ADD_PRO;

    if(col != 1) return;
    if(!item_parent) return;

    id = Sample_Properties->indexOfTopLevelItem(item_parent);
    if(id < 0 || id >= prot->Plate.groups.size()) return;

    text = item->text(0);
    str_value = item->text(1);

    group = prot->Plate.groups.at(id);

    foreach(property, group->preference_Group)
    {
        name = QString::fromStdString(property->name);
        id = list.indexOf(name);
        switch(id)
        {
        case 0: name = tr(PATIENT); break;
        case 1: name = tr(SEX); break;
        case 2: name = tr(AGE); break;
        case 3: name = tr(ORG); break;
        case 4: name = tr(PHISIC); break;
        case 5: name = tr(DATA); break;
        case 6: name = tr(NOTE); break;
        case 7: name = tr(ADD_PRO); break;

        default:    name = "";  break;
        }

        if(name == text)
        {
            property->value = str_value.toStdString();            
            break;
        }
    }
}

//-----------------------------------------------------------------------------
//--- SampleItemChange(QTreeWidgetItem *item, int col)
//-----------------------------------------------------------------------------
void Setup::SampleItemChange(QTreeWidgetItem *item, int col)
{
    int i,j;
    int id = 0;
    int count_gr = prot->Plate.groups.size();
    rt_GroupSamples *group;
    rt_Sample *sample;
    bool change = false;

    qDebug() << "SampleItemChange: ";

    if(col != 0) return;
    int index = Sample_Tree->indexOfTopLevelItem(item);
    QString text = item->text(col).trimmed();
    if(text.isEmpty()) text = "sample";

    //qDebug() << item->text(col) << Sample_Tree->indexOfTopLevelItem(item);

    for(i=0; i<count_gr; i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            if(change) break;
            sample = group->samples.at(j);
            if(id == index)
            {
                sample->Unique_NameSample = text.toStdString();
                if(group->samples.size() == 1) group->Unique_NameGroup = text.toStdString();
                change = true;
                break;
            }
            id++;
        }
    }

    fill_SampleTable();
    fill_TestSample();
    fill_SampleProperties();

}
//-----------------------------------------------------------------------------
//--- ChangeTypeSample()
//-----------------------------------------------------------------------------
void Setup::ChangeTypeSample()
{
    int i,j,k;
    int count_tubes;
    int row = 0;
    int type;
    bool kod;
    bool find;
    QString text;
    bool isopen = Samples_Table->open_samples;
    int scroll_value = Samples_Table->verticalScrollBar()->value();

    //qDebug() << "ChangeTypeSample: ";

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Preference   *preference;
    QTableWidgetItem *item;

    save_undo();        // UNDO ChangeTypeSample: K+,K-,St

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            item = Samples_Table->item(row,3);
            text = item->text();
            type = text.toInt(&kod);
            if(!kod) type = -1;

            count_tubes = sample->tubes.size();
            if(isopen) row += count_tubes + 1;
            else row++;

            find = false;
            for(k=0; k<sample->preference_Sample.size(); k++)
            {
                preference = sample->preference_Sample.at(k);
                if(preference->name == "kind")
                {
                    find = true;
                    switch(type)
                    {
                    case 0:     text = "ControlPositive";   break;
                    case 1:     text = "ControlNegative";   break;
                    case 2:     text = "Standart";  break;
                    default:    text = "";  break;
                    }
                    preference->value = text.toStdString();
                    break;
                }
            }
            if(!find)
            {
                switch(type)
                {
                case 0:     text = "ControlPositive";   break;
                case 1:     text = "ControlNegative";   break;
                case 2:     text = "Standart";  break;
                default:    text = "";  break;
                }
                if(!text.isEmpty())
                {
                    preference = new rt_Preference();
                    preference->name = "kind";
                    preference->value = text.toStdString();
                    sample->preference_Sample.push_back(preference);
                }
            }
        }
    }


    fill_SampleTable();
    fill_Plate();

    Samples_Table->verticalScrollBar()->setValue(scroll_value);

}
//-----------------------------------------------------------------------------
//--- ChangeCopiesSample()
//-----------------------------------------------------------------------------
void Setup::ChangeCopiesSample()
{
    int i,j,k;
    int count_tubes;
    int row = 0;
    int type;
    bool kod;
    bool find;
    QString text;
    bool isopen = Samples_Table->open_samples;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Preference   *preference;
    QTableWidgetItem *item;   

    save_undo();        // UNDO ChangeCopiesSample

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            item = Samples_Table->item(row,4);
            text = item->text();



            count_tubes = sample->tubes.size();
            if(isopen) row += count_tubes + 1;
            else row++;

            find = false;
            for(k=0; k<sample->preference_Sample.size(); k++)
            {
                preference = sample->preference_Sample.at(k);
                if(preference->name == "copies")
                {
                    preference->value = text.toStdString();
                    find = true;
                    break;
                }
            }
            if(!find)
            {
                if(!text.isEmpty() && text != "-")
                {
                    preference = new rt_Preference();
                    preference->name = "copies";
                    preference->value = text.toStdString();
                    sample->preference_Sample.push_back(preference);
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- ChangeNameSample()
//-----------------------------------------------------------------------------
void Setup::ChangeNameSample()
{
    int i,j,k;
    int count_tubes;
    int row = 0;
    QString text, name_item;
    QStringList list;
    bool isopen = Samples_Table->open_samples;
    bool rewrite = false;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    QTableWidgetItem *item;

    //qDebug() << "new name: ";

    save_undo();        // UNDO ChangeNameSample

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);

        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            item = Samples_Table->item(row,2);
            text = item->text();
            text.remove("Header_Sample:");
            list = text.split("  (");
            text = list.at(0);

            //qDebug() << "i,j: " << i << j << text;

            sample->Unique_NameSample = text.toStdString();

            if(!j && text != QString::fromStdString(group->Unique_NameGroup))
            {
                group->Unique_NameGroup = text.toStdString();
            }
            count_tubes = sample->tubes.size();
            if(isopen) row += count_tubes + 1;
            else row++;
        }
    }

    fill_TestSample();
    fill_SampleProperties();
}

//-----------------------------------------------------------------------------
//--- change_StatusEditProtocol(bool)
//-----------------------------------------------------------------------------
void Setup::change_StatusEditProtocol(bool status)
{
    if(flag_EditProtocol) return;

    // 1. Sample
    sampleToolBar->setEnabled(status);
    Samples_Delegate->enable_editor = status;
    if(status)Samples_Table->setContextMenuPolicy(Qt::CustomContextMenu);
    else Samples_Table->setContextMenuPolicy(Qt::NoContextMenu);
    Samples_EditorDelegate->enable_editor = status;

    // 2. Program
    programToolBar->setEnabled(status);

    // 3. Plate
    plateToolBar->setEnabled(status);
    Type_Plate->enable_editor = status;
}

//-----------------------------------------------------------------------------
//--- OpenCloseSamples()
//-----------------------------------------------------------------------------
void Setup::OpenCloseSamples()
{
    Samples_Table->open_samples = !Samples_Table->open_samples;
    fill_SampleTable();
}
//-----------------------------------------------------------------------------
//--- resize_splitter(int pos, int index)
//-----------------------------------------------------------------------------
void Setup::resize_splitter(int pos, int index)
{
    Name_Protocol->setFixedWidth(pos - Label_Protocol->width());
}
//-----------------------------------------------------------------------------
//--- resizeEvent(QResizeEvent *e)
//-----------------------------------------------------------------------------
void Setup::resizeEvent(QResizeEvent *e)
{    
    int value = MainListWindow->width() - Label_Protocol->width();
    if(value > 0) Name_Protocol->setFixedWidth(value);
    Name_Protocol->repaint();
    Name_Protocol->clearFocus();

    int w = width();
    int h = height();
    label_gif->move(w/2 - label_gif->width()/2, h/2 - label_gif->height()/2);
}
//-----------------------------------------------------------------------------
//--- showEvent(QShowEvent *e)
//-----------------------------------------------------------------------------
void Setup::showEvent(QShowEvent *e)
{
    int value = MainListWindow->width() - Label_Protocol->width();
    if(value > 0) Name_Protocol->setFixedWidth(value);
    Name_Protocol->clearFocus();

    //create_NamesResearch();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void Setup::mouseDoubleClickEvent(QMouseEvent *e)
{
    QMainWindow::mouseDoubleClickEvent(e);
}*/

//-------------------------------------------------------------------------------
//--- User events
//-------------------------------------------------------------------------------
bool Setup::event(QEvent *e)
{
    QString text;

    if(e->type() == 3006)       // open protocol (file from Analysis)
    {
        text = parentWidget()->property("analysis_to_setup").toString();
        //qDebug() << text;
        QFile file(text);
        if(file.exists())
        {
            open_protocol(NULL, text);
        }
        return(true);
    }

    if(e->type() == 3012)       // open protocol (file from Web)
    {
        text = parentWidget()->property("Web_to_setup").toString();
        QFile file(text);
        QFileInfo fi(text);
        if(file.exists())
        {
            if(fi.suffix() == "xml") open_XMLprotocol(text);
            else open_protocol(NULL, text);

            qDebug() << "Web to setup: ";

            if(prot->Plate.groups.size())   // if Protocol is not empty
            {
                rt_Preference *preference = new rt_Preference;
                preference->name = "Web_Protocol";
                preference->value = prot->regNumber;
                prot->preference_Pro.push_back(preference);
            }
        }
        return(true);
    }

    if(e->type() == 4004)       // open Editor Programm
    {
        QTimer::singleShot(200, this, SLOT(edit_program_separate()));
        return(true);
    }



    return QWidget::event(e);
}
//-----------------------------------------------------------------------------
//--- SamplesItemDelegate
//-----------------------------------------------------------------------------
void SamplesItemDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    int i,id;
    int num_sample = -1;
    QRect rect;
    QString text;
    QStringList list, list_ch, list_tmp;
    int pos;
    bool ok;
    QFont font = qApp->font();
    int font_size = font.pointSize();
    QFont serifFont("Times New Roman", 12, QFont::Bold);
    int dy;
    double K;
    QColor color;
    int row;

    QPixmap pixmap(":/images/fam.png");


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);


    rect = option.rect;
    row = index.row();

    text = index.data().toString();

    //--- Header sample ---
    pos = text.indexOf("Header_Sample:");
    if(pos >= 0)
    {
        //if(index.column() == 0) qDebug() << "header" << text;

        if((option.state & QStyle::State_Selected)) painter->fillRect(rect, QColor(215,215,255));
        else
        {
            if(style == "fusion")
            {
                if(*open_sample) painter->fillRect(rect, QColor(255,255,255));
                else
                {
                    if(div(row,2).rem == 0) painter->fillRect(rect, QColor(255,255,255));
                    else painter->fillRect(rect, QColor(250,250,250));
                }
            }
            else painter->fillRect(rect, QColor(255,255,225));
        }

        painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
        //font = painter->font();
        if(index.column() == 2) font.setBold(true);
        painter->setFont(font);
        text = text.mid(14).trimmed();
        if(index.column() == 2) painter->drawText(rect, Qt::AlignCenter, text);
        if(index.column() == 0)
        {
            pos = text.indexOf("(pos)");
            if(pos >= 0)
            {
                text = text.mid(pos+5);
                painter->drawText(rect, Qt::AlignCenter, text);
            }
        }
        if(index.column() == 1)
        {
            pos = text.indexOf("(color)");
            if(pos >= 0)
            {               
                text = text.mid(pos+7).trimmed();
                if(text == "-1")
                {
                    painter->fillRect(rect, Qt::DiagCrossPattern); // Qt::Dense7Pattern);
                    painter->setBrush(Qt::SolidPattern);
                }
                else
                {
                    color = text.toInt();
                    rect.setX(rect.x()+1);
                    rect.setY(rect.y()+1);
                    rect.setWidth(rect.width()-1);
                    rect.setHeight(rect.height()-1);
                    painter->fillRect(rect, color);
                }
            }
        }

        return;
    }

    //--- Type ---
    if(index.column() == 3 && !text.isEmpty())
    {
        if((option.state & QStyle::State_Selected)) painter->fillRect(rect, QColor(215,215,255));
        else
        {
            if(style == "fusion")
            {
                if(*open_sample) painter->fillRect(rect, QColor(255,255,255));
                else
                {
                    if(div(row,2).rem == 0) painter->fillRect(rect, QColor(255,255,255));
                    else painter->fillRect(rect, QColor(250,250,250));
                }
            }
            else painter->fillRect(rect, QColor(255,255,225));
        }

        id = text.toInt(&ok);
        if(ok)
        {
            switch(id)
            {
            case 0:     text = tr("C+");    break;
            case 1:     text = tr("C-");    break;
            case 2:     text = tr("St");    break;
            default:    text = "";  break;
            }
        }
        else text = "";
        font.setBold(true);
        //painter->setFont(serifFont);
        //font.setPointSize(font_size + 2);
        painter->setFont(font);

        painter->drawText(rect, Qt::AlignCenter, text);

        //font.setPointSize(font_size);
        font.setBold(false);
        painter->setFont(font);
        return;
    }
    //----------------------

    //--- Copies ---
    if(index.column() == 4 && !text.isEmpty())
    {
        //qDebug() << "Copies paint: " << text;
        if((option.state & QStyle::State_Selected)) painter->fillRect(rect, QColor(215,215,255));
        else
        {
            if(style == "fusion")
            {
                if(*open_sample) painter->fillRect(rect, QColor(255,255,255));
                else
                {
                    if(div(row,2).rem == 0) painter->fillRect(rect, QColor(255,255,255));
                    else painter->fillRect(rect, QColor(250,250,250));
                }
            }
            else painter->fillRect(rect, QColor(255,255,225));
        }

        if(text == "-") text = "";

        //font.setBold(false);
        //painter->setFont(font);
        //painter->drawText(rect, Qt::AlignCenter, text);
        if(!text.isEmpty())
        {

            list = text.split("\t");
            id = list_ID->indexOf(list.at(0));
            /*if(list.size() > 1)
            {
                num_sample = QString(list.at(1)).toInt(&ok);
            }
            if(id != num_sample)
            {
                pixmap.load(":/images/copies_new_16.png");
                painter->drawPixmap(rect.x() + (rect.width() - 16)/2, rect.y() + (rect.height() - 16)/2, 16, 16, pixmap);
            }
            else
            {*/
            if(list.size() > 1 && list.at(1) == "main_copy")
            {
                pixmap.load(":/images/copies_24.png");
                painter->drawPixmap(rect.x() + 13, rect.y() + 4, 24, 24, pixmap);
            }
            else
            {
                pixmap.load(":/images/copies_16_child_new.png");
                //pixmap.load(":/images/copies_3.png");
                painter->drawPixmap(rect.x() + 13, rect.y() + 0, 24, 29, pixmap);
            }
            //}

            /*if(id >= 0)
            {
                text = QString::number(id+1);
                font.setPointSize(font_size - 1);
                painter->setFont(font);
                painter->drawText(rect, Qt::AlignLeft | Qt::AlignBottom, text);
            }*/
        }

        return;
    }

    //__1. Background
    //int row = index.row();
    if((option.state & QStyle::State_Selected) || row == current_row)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        if(div(row,2).rem)  painter->fillRect(option.rect, QColor(245,245,245));
        else painter->fillRect(option.rect, QColor(250,250,250));
    }
    //-----------------------

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    font = qApp->font();
    //font.setBold(false);
    painter->setFont(font);

    QFontMetrics fm(font);

    int col = index.column();
    text = index.data().toString();
    switch(col)
    {
    case 2:                                 // Identificator
            list = text.split("\r\n");
            for(i=0; i<list.size(); i++)
            {
                list_ch.append(list.at(i).mid(0,1));
                text = list.at(i);
                text.remove(0,2);
                list_tmp.append(text);
            }
            text = list_tmp.join("\r\n");
            rect.setX(rect.x()+35);
            painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);

            //K = 0.9;
            //dy = 0;
            //if(list_ch.size() > 1) {K = 0.7; dy = 1*(list_ch.size()-1);}
            //dy = (option.rect.height() - 16*list_ch.size())/(list_ch.size() - 1 + 4);
            dy = option.rect.height()/list_ch.size();
            for(i=0; i<list_ch.size(); i++)
            {
               id = list_ch.at(i).toInt();
               if(style == "fusion")
               {
                   switch(id)
                   {
                   case 0:  pixmap.load(":/images/fam_pro.png");    break;
                   case 1:  pixmap.load(":/images/hex_pro.png");    break;
                   case 2:  pixmap.load(":/images/rox_pro.png");    break;
                   case 3:  pixmap.load(":/images/cy5_pro.png");    break;
                   case 4:  pixmap.load(":/images/cy55_pro.png");   break;
                   default: pixmap.load(":/images/disable_pro.png");   break;
                   }
               }
               else
               {
                   switch(id)
                   {
                   case 0:  pixmap.load(":/images/fam.png");    break;
                   case 1:  pixmap.load(":/images/hex.png");    break;
                   case 2:  pixmap.load(":/images/rox.png");    break;
                   case 3:  pixmap.load(":/images/cy5.png");    break;
                   case 4:  pixmap.load(":/images/cy55.png");   break;
                   default: pixmap.load(":/images/disable.png");   break;
                   }
               }

               font = painter->font();
               font.setBold(false);
               font.setItalic(false);
               painter->setFont(font);
               //painter->drawPixmap(option.rect.x()+10, option.rect.y() + 21*K*i + 0 + dy,pixmap);
               //painter->drawPixmap(option.rect.x()+10, option.rect.y() + (16 + dy)*i + 2*dy,pixmap);
               painter->drawPixmap(option.rect.x()+10, option.rect.y() + dy*i + (dy-16)/2., pixmap);
            }
            break;
    case 0:                                 // Position
            font = painter->font();
            font.setBold(false);
            painter->setFont(font);
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;
    case 1:                                 // Color
            rect.setX(rect.x()+1);
            rect.setY(rect.y()+1);
            rect.setWidth(rect.width()-1);
            rect.setHeight(rect.height()-1);
            color = text.toInt();            
            painter->fillRect(rect, color);
            break;            

    default:
            break;

    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget* SamplesItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget *obj;
    QComboBox *obj_cbox;
    QLineEdit *obj_le;
    int col = index.column();    
    QFont f = qApp->font();
    int font_size = f.pointSize();

    if(!enable_editor) return(nullptr);

    //qDebug() << "current_Row: " << index.row();

    switch(col)
    {
    case 2:

        //obj = QStyledItemDelegate::createEditor(parent, option, index);
        obj_le = new QLineEdit(parent);
        obj_le->setStyleSheet("QLineEdit {selection-background-color: rgb(215,215,255); selection-color: black;}");

        connect(obj_le, SIGNAL(editingFinished()), this, SLOT(Close_LineEditor()));

        obj = obj_le;

        break;

    case 3:

        //obj = nullptr;
        //break;


        obj_cbox = new QComboBox(parent);
        obj_cbox->addItem(tr("C+"));
        obj_cbox->addItem(tr("C-"));
        obj_cbox->addItem(tr("St"));
        obj_cbox->addItem("-");

        if(disable_Standarts) obj_cbox->setItemData(2,0,Qt::UserRole - 1);

        obj_cbox->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
        f.setPointSize(font_size - 1);
        obj_cbox->setFont(f);
        obj = obj_cbox;


        //connect(obj_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(Close_TypeEditor()));


        break;

    case 4:

        obj = nullptr;
        break;

        obj_cbox = new QComboBox(parent);
        obj_cbox->addItems(*list_NAME);
        obj_cbox->addItem("-");        
        obj_cbox->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
        f.setPointSize(font_size - 1);
        obj_cbox->setFont(f);
        obj = obj_cbox;

        connect(obj_cbox, SIGNAL(currentIndexChanged(int)), this, SLOT(Close_CopiesEditor()));

        break;

    default:

        obj = nullptr;
        break;
    }

    return(obj);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void SamplesItemDelegate::Close_LineEditor()
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    if(editor)
    {
        emit commitData(editor);    // fix data: This signal must be emitted when the editor widget has completed editing the data,
                                    // and wants to write it back into the model.
        emit closeEditor(editor);
        emit change_NameSample();
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void SamplesItemDelegate::Close_TypeEditor()
{
    QComboBox *bx = qobject_cast<QComboBox *>(sender());
    if(bx)
    {
        emit commitData(bx);    // fix data: This signal must be emitted when the editor widget has completed editing the data,
                                // and wants to write it back into the model.

        emit closeEditor(bx);   // This signal is emitted when the user has finished editing an item using the specified editor.

        emit change_Type();
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void SamplesItemDelegate::Close_CopiesEditor()
{
    QComboBox *bx = qobject_cast<QComboBox *>(sender());
    if(bx)
    {
        emit commitData(bx);    // fix data: This signal must be emitted when the editor widget has completed editing the data,
                                // and wants to write it back into the model.

        emit closeEditor(bx);   // This signal is emitted when the user has finished editing an item using the specified editor.

        emit change_Copies();       
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void SamplesItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int col = index.column();
    int value;
    bool ok;
    int id;
    QString text;
    QStringList list;


    if(col == 2)
    {
        QLineEdit *LineEdit = static_cast<QLineEdit*>(editor);
        text = index.model()->data(index, Qt::EditRole).toString().trimmed();
        text.remove("Header_Sample:");
        list = text.split("  (");
        if(list.size() > 1)
        {
            *nameTest = QString(list.at(1)).remove(")");
            text = list.at(0);
        }
        else *nameTest = "";
        LineEdit->setText(text);

        return;
    }

    if(col == 3)
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        text = index.model()->data(index, Qt::EditRole).toString().trimmed();
        //qDebug() << "setEditor: " << text;
        if(text.isEmpty()) comboBox->setCurrentIndex(-1);
        else
        {
            value = text.toInt(&ok,10);
            if(ok && value < comboBox->count() && value < 3) comboBox->setCurrentIndex(value);
            else comboBox->setCurrentIndex(-1);
        }

        connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(Close_TypeEditor()));
        return;
    }

    if(col == 4)
    {
        //return;

        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        text = index.model()->data(index, Qt::EditRole).toString().trimmed();
        //qDebug() << "setEditor: " << text;
        if(text.isEmpty() || text == "-") comboBox->setCurrentIndex(-1);
        else
        {
            id = list_ID->indexOf(text);
            comboBox->setCurrentIndex(id);
        }

        //QStyledItemDelegate::setEditorData(editor,index);

        return;
    }

    QStyledItemDelegate::setEditorData(editor,index);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void SamplesItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox;
    QLineEdit *lineEditor;
    int col = index.column();
    int id;
    QString text;

    switch(col)
    {
    case 2:
                lineEditor = static_cast<QLineEdit*>(editor);
                text = "Header_Sample:" + lineEditor->text() + "  (" + *nameTest + ")";
                model->setData(index, text, Qt::EditRole);
                break;
    case 3:                
                comboBox = static_cast<QComboBox*>(editor);
                model->setData(index, comboBox->currentIndex(), Qt::EditRole);
                break;

    case 4:
                break;
                comboBox = static_cast<QComboBox*>(editor);
                id = comboBox->currentIndex();
                if(id >= 0 && id < list_ID->size()) text = list_ID->at(id);
                else text = "-";
                model->setData(index, text, Qt::EditRole);
                break;

    default:
                QStyledItemDelegate::setModelData(editor,model,index);
                break;
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void SamplesItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect;
    int col = index.column();

    switch(col)
    {
    case 4:
                rect = option.rect;
                rect.setLeft(rect.left()-100);
                break;

    default:
                rect = option.rect;
                break;
    }

    editor->setGeometry(rect);
}
//-----------------------------------------------------------------------------
//--- PlateTableWidget::mousePressEvent
//-----------------------------------------------------------------------------
void PlateTableWidget::mousePressEvent(QMouseEvent *event)
{
    int num;

    //const QColorDialog::ColorDialogOptions options = QFlag(colorDialogOptionsWidget->value());
    QColor color;

    QTableWidget::mousePressEvent(event);
    if(!enable_editor) return;

    if(event->button() == Qt::LeftButton && ability_select && p_prot->Plate.groups.size())// && delegate->view == v_numeration)
    {
        point_0 = event->pos();
        point_1 = point_0;

        selected = true;
        coord_0.setX(currentColumn());
        coord_0.setY(currentRow());
    }    
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PlateTableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int i,j;
    int pos;        
    bool zoom = false;
    int row, col;
    QColor color;

    QMap<int,int> color_Map;
    int value;

    if(selected && moving) zoom = true;
    selected = false;
    moving = false;
    if(event->button() == Qt::LeftButton) QTableWidget::mouseReleaseEvent(event);
    if(!enable_editor) return;

    coord_1.setX(currentColumn());
    coord_1.setY(currentRow());

    if(zoom && coord_0 != coord_1 && coord_1.x() >= 0 && coord_1.y() >= 0)// && delegate->view == v_numeration)
    {
        int min_row = qMin(coord_0.y(),coord_1.y());
        int min_col = qMin(coord_0.x(),coord_1.x());
        int max_row = qMax(coord_0.y(),coord_1.y());
        int max_col = qMax(coord_0.x(),coord_1.x());

        if(delegate->view == v_color) //color = QColorDialog::getColor(Qt::gray, this, tr("Select Color"));
        {
            Color_ToolButton *button = (Color_ToolButton*)Color_Group->checkedButton();
            color = button->color;
        }

        p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);       

        for(i=min_row; i<=max_row; i++)
        {
            for(j=min_col; j<=max_col; j++)
            {
                pos = j + i*col;

                switch(delegate->view)
                {
                default:
                case v_numeration:
                                    p_prot->enable_tube.at(pos) = !p_prot->enable_tube.at(pos) & 0x01;
                                    break;
                case v_color:
                                    if(color.isValid() && p_prot->enable_tube.at(pos))
                                    {
                                        p_prot->color_tube.at(pos) = color.rgb();                                        
                                        color_Map.insert(pos, color.rgb() & 0xffffff);
                                    }
                                    break;
                }
            }
        }        
        emit cellClicked(-1,-1);

        if(color_Map.size() && delegate->view == v_color)  emit ReLoadColor(color_Map);
    }

    viewport()->update();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PlateTableWidget::mouseMoveEvent(QMouseEvent *event)
{
    QTableWidget::mouseMoveEvent(event);
    if(!enable_editor) return;

    if(selected)
    {
        point_1 = event->pos();
        moving = true;
        viewport()->update();
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PlateTableWidget::paintEvent(QPaintEvent *e)
{
    QTableWidget::paintEvent(e);
    if(!enable_editor) return;

    if(selected && moving)
    {
        QPen pen(Qt::black,1,Qt::DashLine);
        QPainter painter_table(viewport());

        painter_table.setPen(pen);
        painter_table.drawRect(QRect(point_0,point_1));
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PlateTableWidget::resizeEvent(QResizeEvent *e)
{
    QTableWidget::resizeEvent(e);

    QTableWidgetItem *item = this->item(0,0);
    QRect r = this->visualItemRect(item);
    A1 = r;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PlateItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor color,rgb;
    //int r,g,b;
    int rad;
    int x,y;
    double X,Y,R;
    int pos;
    int width,height;
    int min_value = qMin(option.rect.width(), option.rect.height());
    int min_value_Pro = qMin(A1->width(),A1->height());                 //
    QString text = index.data().toString();
    QRect rect = option.rect;
    QBrush brush;


    int code_ch, num, kind;
    if(text.length())
    {
        num = text.toInt();
        code_ch = (num & 0xff0000) >> 16;
        kind = (num & 0xff000000) >> 24;
        num = num & 0xffff;
        text = QString::number(num);
    }


    switch(view)
    {
        default:
        case v_color:
                        //R = (double)min_value/2. * 0.80;
                        R = (double)min_value_Pro/2. * 0.80;
                        X = (double)option.rect.x() + (double)option.rect.width()/2. - R;
                        Y = (double)option.rect.y() + (double)option.rect.height()/2. - R;

                        rad = (int)(R+0.5);
                        x = (int)(X+0.5);
                        y = (int)(Y+0.5);
                        break;

    case v_numeration:

                        width = (int)((double)option.rect.width()/1.7 + 0.5);
                        height = (int)((double)option.rect.height()/1.7 + 0.5);
                        x = option.rect.x() + (option.rect.width() - width)/2.;
                        y = option.rect.y() + (option.rect.height() - height)/2.;
                        break;
    }

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus


    QStyledItemDelegate::paint(painter, viewOption, index);

    painter->setRenderHint(QPainter::Antialiasing);

    //__1. Background
    if(style == "fusion") painter->fillRect(option.rect, QColor(255,255,255));
    else  painter->fillRect(option.rect, QColor(255,255,225));

    //__2. Enable Tube
    pos = index.column() + index.row() * index.model()->columnCount();

    if(pos >= 0 && pos < p_prot->count_Tubes && p_prot->enable_tube.size())
    {
        if(p_prot->enable_tube.at(pos))
        {
            switch(view)
            {
                default:
            case v_color:
                                color = p_prot->color_tube.at(pos);
                                painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
                                painter->setBrush(QBrush(color, Qt::SolidPattern));
                                painter->drawEllipse(x,y,2*rad,2*rad);
                                break;

            case v_numeration:
                                Paint_ChannelsRegions(painter,rect,code_ch);
                                switch(kind)
                                {
                                default:
                                case 0:
                                case 1:
                                case 2:
                                            painter->fillRect(x,y,width,height,QColor(200,200,200));
                                            painter->drawRect(x,y,width,height);
                                            if(kind == 1) text = tr("CONTROL+");
                                            if(kind == 2) text = tr("CONTROL-");
                                            break;

                                case 3:
                                            brush = painter->brush();
                                            color = QColor(200,200,200);
                                            painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
                                            painter->setBrush(QBrush(color, Qt::SolidPattern));

                                            // ... new geometry ...
                                            R = (double)min_value/2. * 0.80;
                                            X = (double)option.rect.x() + (double)option.rect.width()/2. - R;
                                            Y = (double)option.rect.y() + (double)option.rect.height()/2. - R;

                                            rad = (int)(R+0.5);
                                            x = (int)(X+0.5);
                                            y = (int)(Y+0.5);
                                            // ...

                                            painter->drawEllipse(x,y,2*rad,2*rad);
                                            painter->setBrush(brush);
                                            break;
                                }

                                painter->drawText(option.rect,Qt::AlignCenter,text);
            }
        }        
    }
    //if(view == v_numeration) painter->drawRect(option.rect);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Paint_ChannelsRegions(QPainter *painter, QRect rect, int code)
{
    //QColor bg_color[] ={0xD8E2F1,0xD8E9D8,0xFEF0CB,0xFFE6FF,0xFFE6FF};
    QColor bg_color[] ={0xA5BCDE,0x93C193,0xFCD05F,0xFFC4FF,0xFF8080};
    //QColor bg_color[] ={Qt::blue,Qt::green,Qt::yellow,Qt::magenta,Qt::red};
    QVector<int> list;
    int i,id;
    int new_top;
    QRect rect_1,rect_2,rect_3,rect_4,R;
    QPainterPath path;

    for(i=0; i<5; i++)
    {
        id = code & (1<<i);
        if(id) list.append(i);
    }
    int count = list.size();

    switch(count)
    {
    default:    break;

    case 1:
                painter->fillRect(rect,bg_color[list.at(0)]);
                //painter->drawRect(rect);
                break;

    case 2:
                rect_1 = rect;
                rect_1.setRight(rect.width()/2 + rect.left() + 0);
                rect_2 = rect;
                rect_2.setLeft(rect.width()/2 + rect.left() - 0);
                painter->fillRect(rect_1,bg_color[list.at(0)]);                
                painter->fillRect(rect_2,bg_color[list.at(1)]);
                /*painter->drawRect(rect_1);
                painter->drawRect(rect_2);*/

                break;

    case 3:
    case 4:

                rect_1 = rect;
                rect_1.setRight(rect.width()/2 + rect.left() + 0);
                rect_1.setBottom(rect.height()/2 + rect.top() + 0);
                rect_2 = rect;
                rect_2.setLeft(rect.width()/2 + rect.left() - 0);
                rect_2.setBottom(rect_1.bottom());
                rect_3 = rect;
                rect_3.setRight(rect_1.right());
                rect_3.setTop(rect_1.bottom() - 0);
                rect_4 = rect;
                rect_4.setLeft(rect_2.left());
                rect_4.setTop(rect_3.top());
                painter->fillRect(rect_1,bg_color[list.at(0)]);                
                painter->fillRect(rect_2,bg_color[list.at(1)]);                
                painter->fillRect(rect_3,bg_color[list.at(2)]);                
                if(count == 3) painter->fillRect(rect_4,Qt::white);
                else painter->fillRect(rect_4,bg_color[list.at(3)]);

                break;

     case 5:
                rect_1 = rect;
                rect_1.setRight(rect.width()/2 + rect.left() + 0);
                rect_1.setBottom(rect.height()/2 + rect.top() + 0);
                rect_2 = rect;
                rect_2.setLeft(rect.width()/2 + rect.left() - 0);
                rect_2.setBottom(rect_1.bottom());
                painter->fillRect(rect_1,bg_color[list.at(0)]);
                painter->fillRect(rect_2,bg_color[list.at(1)]);

                R = rect;
                new_top = R.top() + (R.height())/2;
                R.setTop(new_top);

                // 3.
                path = QPainterPath(); //clear
                path.moveTo(R.left() + R.width()/2, R.top());
                path.lineTo(R.left(),R.bottom()+1);
                path.lineTo(R.left(),R.top());
                path.lineTo(R.left() + R.width()/2, R.top());
                painter->fillPath(path, QBrush(bg_color[list.at(2)]));

               // 4.
                path = QPainterPath(); //clear
                path.moveTo(R.left() + R.width()/2, R.top());
                path.lineTo(R.right()+1,R.bottom()+1);
                path.lineTo(R.right()+1,R.top());
                path.lineTo(R.left() + R.width()/2, R.top());
                painter->fillPath(path, QBrush(bg_color[list.at(3)]));

                // 5.
                path = QPainterPath(); //clear
                path.moveTo(R.left() + R.width()/2, R.top());
                path.lineTo(R.left(),R.bottom()+1);
                path.lineTo(R.right()+1,R.bottom()+1);
                path.lineTo(R.left() + R.width()/2, R.top());
                painter->fillPath(path, QBrush(bg_color[list.at(4)]));

                break;
    }

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget* SampleEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget* obj;

    if(!enable_editor) return(nullptr);

    if(index.column() == 0)
    {
        obj = QStyledItemDelegate::createEditor(parent, option, index);
        obj->setStyleSheet("QLineEdit {selection-background-color: rgb(215,215,255); selection-color: black;}");
        return(obj);
    }
    return(nullptr);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PropertyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString text = index.data().toString();
    QString str, text_new;
    QStringList list, list_ch;
    int row = index.row();
    int col = index.column();
    QStyleOptionViewItem  viewOption(option);

    QStyledItemDelegate::paint(painter, viewOption, index);


    if(row == 7 && col > 0)
    {
        //__1. Background
        painter->fillRect(option.rect, QColor(255,255,255));
        if(option.state & QStyle::State_Selected) painter->fillRect(option.rect, QColor(215,215,255));

        //
        text_new = "";
        list = text.split("#13#10");
        foreach(str, list)
        {
            list_ch = str.split("||");
            if(list_ch.size() < 5) continue;
            if(!text_new.isEmpty()) text_new += "   ";
            text_new += QString("%1: %2;").arg(list_ch.at(1)).arg(list_ch.at(3));
        }
        if(text_new.isEmpty()) text_new = " ...";
        painter->drawText(option.rect,Qt::AlignLeft,text_new);
    }


}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget* PropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont f = qApp->font();
    QWidget* obj = nullptr;
    //QComboBox *obj_cbox;
    QComboBox *cbox;
    QLineEdit *ledit;
    QString text;
    QStringList list, list_ch, list_cb;


    QTreeWidget *obj_tree;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_ch;
    int col = index.column();
    int row = index.row();


    if(index.column() == 1)
    {
        //qDebug() << "col/row: " << index.column() << index.row() << index.data();

        switch(row)
        {
        default:
                    obj = QStyledItemDelegate::createEditor(parent, option, index);
                    obj->setStyleSheet("QLineEdit {selection-background-color: rgb(215,215,255); selection-color: black;}");
                    break;
        case 7:
                    text = index.model()->data(index, Qt::EditRole).toString().trimmed();
                    //if(text.isEmpty() || text == "...") break;

                    obj_tree = new QTreeWidget(parent);
                    obj_tree->setColumnCount(2);
                    obj_tree->horizontalScrollBar()->setVisible(false);

                    list = text.split("#13#10");

                    foreach(text, list)
                    {
                        list_ch = text.split("||");
                        if(list_ch.size() < 5) continue;

                        item = new QTreeWidgetItem(obj_tree);
                        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
                        item->setText(0, list_ch.at(1));

                        if(list_ch.at(2) == "etDictValue")
                        {
                            cbox = new QComboBox(parent);
                            cbox->setStyleSheet("QComboBox {background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;}");
                            list_cb = QString(list_ch.at(4)).split(";");
                            cbox->addItems(list_cb);
                            cbox->setCurrentText(list_ch.at(3));
                            obj_tree->setItemWidget(item, 1, cbox);
                            continue;
                        }
                        if(list_ch.at(2) == "etFloat")
                        {
                            ledit = new QLineEdit(parent);
                            ledit->setText(list_ch.at(3));
                            obj_tree->setItemWidget(item, 1, ledit);
                            continue;
                        }
                        if(list_ch.at(2) == "etDateMasked")
                        {
                            ledit = new QLineEdit(parent);
                            ledit->setInputMask("00.00.0000");
                            ledit->setText(list_ch.at(3));
                            obj_tree->setItemWidget(item, 1, ledit);
                            continue;
                        }
                    }

                    QStringList pp;
                    pp << tr("Name") << tr("Value");
                    obj_tree->setHeaderLabels(pp);

                    //f.setPointSize(font_size - 1);
                    obj_tree->header()->resizeSection(0,150);
                    obj_tree->setFont(f);
                    obj = obj_tree;
                    break;
        }
        return(obj);
    }
    return(nullptr);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PropertyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QWidget *wobj;
    QComboBox *cbox;
    QLineEdit *ledit;
    QTreeWidget *obj_tree;
    QTreeWidgetItem *item;
    QString text, str, name_add;
    int count;
    int i,j;
    QStringList list, list_ch, list_name;

    int row = index.row();

    switch(row)
    {
        default:    QStyledItemDelegate::setModelData(editor,model,index);
                    break;

        case 7:
                    text = index.model()->data(index, Qt::EditRole).toString().trimmed();
                    //qDebug() << "setModelData: " << text;

                    list = text.split("#13#10");
                    foreach(text, list)
                    {
                        list_ch = text.split("||");
                        if(list_ch.size()) list_name.append(list_ch.at(0));
                    }

                    //qDebug() << "list add: " << list_name;


                    obj_tree = static_cast<QTreeWidget*>(editor);
                    QTreeWidgetItemIterator it(obj_tree, QTreeWidgetItemIterator::All);

                    j = 0;
                    text = "";
                    while(*it)
                    {
                        //qDebug() << (*it)->text(0);
                        item = *it;
                        it++;

                        if(j <= list_name.size()) name_add = list_name.at(j);
                        else name_add = QString("add_%1").arg(j);
                        j++;


                        if(!text.isEmpty()) text += "#13#10";
                        text += name_add + "||";
                        text += item->text(0) + "||";

                        wobj = obj_tree->itemWidget(item, 1);

                        if(QLineEdit * ledit = qobject_cast<QLineEdit *>(wobj))
                        {
                            text += QString("etDateMasked||%1|| ").arg(ledit->text());
                            continue;
                        }
                        if(QComboBox * cbox = qobject_cast<QComboBox *>(wobj))
                        {
                            str = "";
                            for(i=0; i< cbox->count(); i++)
                            {
                                if(!str.isEmpty()) str += ";";
                                str += cbox->itemText(i);
                            }
                            text += QString("etDictValue||%1||%2").arg(cbox->currentText()).arg(str);
                            continue;
                        }
                    }


                    model->setData(index, text, Qt::EditRole);
                    break;
    }


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void PropertyDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect;
    int col = index.column();
    int row = index.row();

    if(col == 1)
    {
        switch(row)
        {
        case 7:
                    rect = option.rect;
                    //rect.setLeft(rect.left()-100);
                    rect.setWidth(300);
                    rect.setHeight(100);
                    break;

        default:
                    rect = option.rect;
                    break;
        }
    }

    editor->setGeometry(rect);
}


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void SampleEditorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int row,col;
    QString text;
    QFont font = painter->font();

    row = index.row();
    col = index.column();
    text = index.data().toString();


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus


    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background
    painter->fillRect(option.rect, painter->background());

    //__2. Data
    if(col == 0) font.setBold(true);
    else font.setBold(false);
    painter->setFont(font);

    painter->drawText(option.rect, Qt::AlignLeft, text);

}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool PlateTableWidget::eventFilter(QObject *obj, QEvent *e)
{    
    if(!enable_editor) return(false);

    if (e->type() == QEvent::Paint)
    {
        QAbstractButton* btn = qobject_cast<QAbstractButton*>(obj);
        if(btn)
        {
            QStyleOptionHeader opt;
            opt.init(btn);
            QStyle::State styleState = QStyle::State_None;
            if (btn->isEnabled())
                styleState |= QStyle::State_Enabled;
            if (btn->isActiveWindow())
                styleState |= QStyle::State_Active;
            if (btn->isDown())
                styleState |= QStyle::State_Sunken;
            opt.state = styleState;
            opt.rect = btn->rect();

            opt.iconAlignment = Qt::AlignCenter;
            opt.icon = QIcon(":/images/all.png");

            opt.position = QStyleOptionHeader::OnlyOneSection;
            QStylePainter painter(btn);
            painter.drawControl(QStyle::CE_Header, opt);

            return(true);
        }
    }

    return(false);
    //return QTableWidget::eventFilter(obj, e);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool SampleTableWidget::eventFilter(QObject *obj, QEvent *e)
{    
    if (e->type() == QEvent::Paint)
    {
        QAbstractButton* btn = qobject_cast<QAbstractButton*>(obj);
        if(btn)
        {
            QStyleOptionHeader opt;
            opt.init(btn);
            QStyle::State styleState = QStyle::State_None;
            if (btn->isEnabled())
                styleState |= QStyle::State_Enabled;
            if (btn->isActiveWindow())
                styleState |= QStyle::State_Active;
            if (btn->isDown())
                styleState |= QStyle::State_Sunken;
            opt.state = styleState;
            opt.rect = btn->rect();

            opt.iconAlignment = Qt::AlignCenter;

            if(open_samples) opt.icon = QIcon(":/images/open_sample.png");
            else opt.icon = QIcon(":/images/close_sample.png");

            opt.position = QStyleOptionHeader::OnlyOneSection;
            QStylePainter painter(btn);
            painter.drawControl(QStyle::CE_Header, opt);

            return(true);
        }
    }

    return(false);
    //return QTableWidget::eventFilter(obj, e);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Add_PushButton::paintEvent(QPaintEvent *e)
{
    QPushButton::paintEvent(e);

    if(!user_Tests)
    {
        QRect rect(5,5,10,10);
        QPainter painter(this);
        painter.setPen(QPen(Qt::red, 1, Qt::SolidLine));
        painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
        painter.drawEllipse(rect);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void Color_ToolButton::paintEvent(QPaintEvent *e)
{
    //QColor color;

    if(isChecked()) color.setAlpha(255);
    else color.setAlpha(100);

    //QToolButton::paintEvent(e);

    QPainter painter(this);

    QRect rect = painter.viewport();
    rect.setX(2);
    rect.setY(3);
    rect.setWidth(rect.width() - 6);
    rect.setHeight(rect.height() - 4);

    //painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    //painter.drawRect(rect);

    painter.fillRect(rect, color);
    if(isChecked()) painter.drawRect(rect);


}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Color_ToolButton::mouseDoubleClickEvent(QMouseEvent *e)
{
    QToolButton::mouseDoubleClickEvent(e);

    QColor color_temp = QColorDialog::getColor(color, this, tr("Select Color"));
    if(color_temp.isValid())
    {
        color = color_temp;
        unsigned int val = color.rgba();
        QString str_color = QString::number(val,16);

        if(isChecked()) setStyleSheet(QString("QToolButton::checked {background-color: #%1; border: 1px solid black}").arg(str_color));
        else
        {
            color.setAlpha(60);
            val = color.rgba();
            str_color = QString::number(val,16);
            setStyleSheet(QString("QToolButton {background-color: #%1; border: 1px solid gray}").arg(str_color));
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Color_ToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    QString str_color;
    QString str_color_alfa;
    QToolButton::mouseReleaseEvent(e);

    if(e->button() == Qt::RightButton)
    {
        QColor color_temp = QColorDialog::getColor(color, this, tr("Select Color"));
        if(color_temp.isValid())
        {
            color = color_temp;
            unsigned int val = color.rgba();
            QString str_color = QString::number(val,16);

            if(isChecked()) setStyleSheet(QString("QToolButton::checked {background-color: #%1; border: 1px solid black}").arg(str_color));
            else
            {
                color.setAlpha(60);
                val = color.rgba();
                str_color = QString::number(val,16);
                setStyleSheet(QString("QToolButton {background-color: #%1; border: 1px solid gray}").arg(str_color));
            }
        }
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Setup::Color_Button(int id)
{    
    Color_ToolButton *button = (Color_ToolButton*)Color_Group->button(id);
    QColor color = button->color;
    unsigned int val = (color.rgba() | 0xff000000);
    //qDebug() << "color: " << QString::number(val,16);

    QString str_color = QString::number(val,16);
    QString Style = QString("QToolButton::checked {background-color: #%1; border: 1px solid black}").arg(str_color);

    button->setStyleSheet(Style);

    for(int i=0; i<Color_Group->buttons().size(); i++)
    {
        if(i == id) continue;
        button = (Color_ToolButton*)Color_Group->button(i);
        color = button->color;
        color.setAlpha(60);
        val = color.rgba();
        str_color = QString::number(val,16);
        //qDebug() << "str_color:__ " << str_color;
        Style = QString("QToolButton {background-color: #%1; border: 1px solid gray}").arg(str_color);
        button->setStyleSheet(Style);
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Setup::Create_GroupSamplesProperties(rt_GroupSamples *group)
{
    QString text, str;
    rt_Preference *property;
    rt_Sample     *sample;
    QStringList list;
    int method;

    list << PATIENT << SEX << AGE << ORG << PHISIC << DATA << NOTE << ADD_PRO;

    foreach(text, list)
    {
        property = new rt_Preference;
        property->name = text.toStdString();
        property->value = "...";
        property->unit = "";

        if(text == "display_name") property->value = group->Unique_NameGroup;

        group->preference_Group.push_back(property);

        if(property->name == ADD_PRO && group->samples.size())
        {

            sample = group->samples.at(0);
            method = sample->p_Test->header.Type_analysis;
            str = Map_Settings.value(method,"").trimmed();
            if(!str.isEmpty()) property->value = str.toStdString();
        }
    }
}

//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    QString text;
    QRect rect_new; // = rect;
    rect_new.setX(rect.x()-1);
    rect_new.setY(rect.y()-1);
    rect_new.setWidth(rect.width());
    rect_new.setHeight(rect.height());

    /*if(orientation() == Qt::Horizontal && logicalIndex == 0)
    {
        rect_new.setX(rect.x());
        rect_new.setWidth(rect.width()-1);
    }*/

    QPen pen(Qt::black,1,Qt::SolidLine);
    pen.setColor(QColor(50,50,50));

    if(!rect.isValid()) return;

    //qDebug() << "paint: " << logicalIndex << selected;
    painter->save();
    //QHeaderView::paintSection(painter,rect,logicalIndex);
    painter->restore();

    if(logicalIndex >= 0)
    {
        if(selected && Section_Selected.contains(logicalIndex)) painter->fillRect(rect, QColor(210,210,210));
        else painter->fillRect(rect, QColor(255,255,255));
        painter->setPen(pen);
        painter->drawRect(rect_new);
        if(orientation() == Qt::Horizontal) text = QString::number(logicalIndex+1);
        else text = QChar(0x41 + logicalIndex);
        painter->drawText(rect, Qt::AlignCenter, text);
    }
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHeader::mouseReleaseEvent(QMouseEvent *event)
{
    QHeaderView::mouseReleaseEvent(event);
    //qDebug() << "mouseReleaseEvent: " << selected << moving << Section_Selected;

    if(event->button() == Qt::LeftButton && Section_Selected.size())
    {
        emit sSectionsSelect(Section_Selected);
    }

    selected = false;
    moving = false;
    Section_Selected.clear();
    viewport()->update();
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHeader::mousePressEvent(QMouseEvent *event)
{
    int id = -1;

    QHeaderView::mousePressEvent(event);

    Section_Selected.clear();

    if(event->button() == Qt::LeftButton)
    {
        selected = true;
        id = logicalIndexAt(event->pos());
        Section_Selected.append(id);

        viewport()->update();
    }


    //qDebug() << "mousePressEvent: " << id << selected << moving << Section_Selected;
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyHeader::mouseMoveEvent(QMouseEvent *event)
{
    int id = -1;
    QHeaderView::mouseMoveEvent(event);

    id = logicalIndexAt(event->pos());
    //qDebug() << "mouseMoveEvent " << id << selected;

    if(selected)
    {
        if(id >= 0 && !Section_Selected.contains(id))
        {
            Section_Selected.append(id);
            viewport()->update();
        }
    }
}
