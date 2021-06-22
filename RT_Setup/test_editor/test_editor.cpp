#include "test_editor.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Test_editor::Test_editor(QWidget *parent, bool disable): QDialog(parent)
{    
    QPalette palette;
    QFont label_font = qApp->font(); //("Times New Roman", 12, QFont::Bold);
    label_font.setBold(true);
    setFont(qApp->font());    

    readCommonSettings();
    //TESTs = tests;

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Tests Editor"));    
    resize(1300,550);    

    QVBoxLayout *main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setMargin(2);

    box_editor = new QGroupBox(this);
    box_editor->setObjectName("Transparent");
    close_button = new QPushButton(tr("Close"), this);
    apply_button = new QPushButton(tr("Apply"), this);
    apply_button->setEnabled(false);
    if(!disable) apply_button->setVisible(false);
    QHBoxLayout *control_layout = new QHBoxLayout();
    control_layout->setMargin(4);
    control_layout->addWidget(apply_button,1,Qt::AlignRight);
    control_layout->addWidget(close_button,0,Qt::AlignRight);
    main_layout->addWidget(box_editor,1);
    main_layout->addLayout(control_layout);
    close_button->setFocusPolicy(Qt::NoFocus);
    apply_button->setFocusPolicy(Qt::NoFocus);

    QVBoxLayout *editor_layout = new QVBoxLayout();
    editor_layout->setMargin(0);
    box_editor->setLayout(editor_layout);

    main_spl = new QSplitter(Qt::Horizontal, this);
    main_spl->setHandleWidth(4);
    main_spl->setChildrenCollapsible(false);

    tests_list = new MyTreeWidget(this);
    tests_list->setFont(qApp->font());
    tests_list->setContextMenuPolicy(Qt::CustomContextMenu);
    //tests_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(tests_list, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_ListTests(QPoint)));


    //edit_test = new QPushButton(tr("Edit test"));
    //edit_test->setEnabled(false);
    //box_TestList = new QGroupBox(this);
    //QVBoxLayout *TestList_layout = new QVBoxLayout();
    //box_TestList->setLayout(TestList_layout);
    //TestList_layout->addWidget(tests_list);
    //TestList_layout->addWidget(edit_test,0,Qt::AlignRight);

    //tab_editor = new QTabWidget(this);
    group_editor = new QGroupBox(this);
    group_editor->setObjectName("Transparent");
    main_spl->addWidget(tests_list);
    main_spl->addWidget(group_editor);

    QVBoxLayout *tool_layout = new QVBoxLayout();
    group_editor->setLayout(tool_layout);
    tool_layout->setMargin(0);    

    tool_editor = new QToolBar();

    edit_TestButton = new QPushButton(QIcon(":/images/edit_TestNew.png"), tr("Edit Test..."), this);
    if(StyleApp == "fusion") edit_TestButton->setIcon(QIcon(":/images/flat/edit_flat.png"));
    edit_TestButton->setMinimumWidth(220);
    //edit_TestButton->setFixedWidth(220);
    edit_TestButton->setFocusPolicy(Qt::NoFocus);
    connect(edit_TestButton, SIGNAL(clicked()), this, SLOT(Click_edit_test()));
    edit_TestButton->setEnabled(false);


    new_Container = new QAction(QIcon(":/images/flat/container.png"), tr("new Container"), this);
    connect(new_Container, SIGNAL(triggered(bool)), this, SLOT(create_newContaner()));
    new_Test = new QAction(QIcon(":/images/new_new.png"), tr("new Test"), this);
    if(StyleApp == "fusion") new_Test->setIcon(QIcon(":/images/flat/new_flat.png"));
    connect(new_Test, SIGNAL(triggered(bool)), this, SLOT(create_newTest()));
    copy_Test = new QAction(QIcon(":/images/copy_new.png"), tr("copy Test"), this);
    if(StyleApp == "fusion") copy_Test->setIcon(QIcon(":/images/flat/copy_new.png"));
    connect(copy_Test, SIGNAL(triggered(bool)), this, SLOT(Copy_Test()));
    save_Test = new QAction(QIcon(":/images/save_new.png"), tr("save Test"), this);
    if(StyleApp == "fusion") save_Test->setIcon(QIcon(":/images/flat/save_flat.png"));
    connect(save_Test, SIGNAL(triggered(bool)), this, SLOT(Save_Test()));
    clear_Test = new QAction(QIcon(":/images/clear_editor.png"), tr("clear Editor"), this);
    if(StyleApp == "fusion") clear_Test->setIcon(QIcon(":/images/flat/clear_flat.png"));
    connect(clear_Test, SIGNAL(triggered(bool)), this, SLOT(Clear_editor()));
    menu_deleteTest = new QAction(tr("delete test"), this);
    connect(menu_deleteTest, SIGNAL(triggered(bool)), this, SLOT(DeleteTest_menu()));
    menu_copyTest = new QAction(tr("copy test"), this);
    connect(menu_copyTest, SIGNAL(triggered(bool)), this, SLOT(CopyTest_menu()));
    menu_editTest = new QAction(tr("edit test"), this);
    connect(menu_editTest, SIGNAL(triggered(bool)), this, SLOT(EditTest_menu()));
    editor_Name = new QLineEdit(this);

    menu_AddItem = new QAction(tr("add item"), this);
    connect(menu_AddItem, SIGNAL(triggered(bool)), this, SLOT(AddItem_menu()));
    menu_DeleteItem = new QAction(tr("delete item"), this);
    connect(menu_DeleteItem, SIGNAL(triggered(bool)), this, SLOT(DeleteItem_menu()));

    catalog_Test = new QAction(QIcon(":/images/flat/catalog_24.png"), tr("edit catalog test"), this);
    connect(catalog_Test, SIGNAL(triggered(bool)), this, SLOT(Catalog_editor()));
    catalog_Test->setVisible(false);

    //menu_Move = new QAction(tr("move item"), this);
    //connect(menu_Move, SIGNAL(triggered(bool)), this, SLOT(Item_Move()));

    tool_editor->addWidget(edit_TestButton);
    obj = new QWidget(this);
    obj->setFixedWidth(20);    
    tool_editor->addWidget(obj);
    tool_editor->addAction(new_Container);
    tool_editor->addSeparator();
    tool_editor->addAction(new_Test);
    tool_editor->addAction(copy_Test);
    tool_editor->addAction(save_Test);    
    tool_editor->addSeparator();
    tool_editor->addAction(clear_Test);    
    spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tool_editor->addWidget(spacer);
    tool_editor->addAction(catalog_Test);
    tool_editor->addWidget(editor_Name);    
    //tool_editor->setIconSize(QSize(16,16));

    palette.setColor(QPalette::Text,Qt::black);//Qt::darkBlue);
    palette.setColor(QPalette::Base, QColor(255,255,225)); //QColor(255,255,225));
    palette.setColor(QPalette::Highlight,QColor(180,180,255));
    palette.setColor(QPalette::HighlightedText,Qt::black);
    //editor_Name->setPalette(palette);
    editor_Name->setFont(label_font);
    editor_Name->setReadOnly(true);
    editor_Name->setFocusPolicy(Qt::NoFocus);
    editor_Name->setAlignment(Qt::AlignCenter);
    connect(editor_Name, SIGNAL(textChanged(QString)), this, SLOT(auto_resize(QString)));


    //tool_editor->setStyleSheet("background-color: rgb(255, 255, 255);");
    /*tool_editor->setStyleSheet(
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

    widget_editor = new groupBox(this);
    layout_plugin = new QVBoxLayout(widget_editor);

    tool_layout->addWidget(tool_editor,0,Qt::AlignTop);
    tool_layout->addWidget(widget_editor, 1);

    editor_layout->addWidget(main_spl);

    connect(close_button, SIGNAL(clicked(bool)), this, SLOT(Close_Editor()));
    connect(apply_button, SIGNAL(clicked(bool)), this, SLOT(Apply_Editor()));

    //connect(tests_list, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(Click_edit_test()));
    connect(tests_list, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(Check_EditStatus(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(tests_list, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(Item_changed(QTreeWidgetItem*,int)));
    //connect(edit_test, SIGNAL(clicked(bool)), this, SLOT(Click_edit_test()));
    connect(widget_editor, SIGNAL(Resize()), this, SLOT(resize_extTests()));

    // from protocol
    From_Protocol = false;
    prot = NULL;

    if(disable)
    {
        new_Container->setDisabled(true);
        new_Test->setDisabled(true);
        copy_Test->setDisabled(true);
        menu_copyTest->setDisabled(true);
        menu_deleteTest->setDisabled(true);        
    }
    disable_action = disable;


    //... Load Plugins ...
    LoadTest_Plugins();
    ax_user = NULL;
    User_Folder = qApp->applicationDirPath();

    //... Load Ext Plugins ...
    //LoadTest_ExtPlugins();    
    Ext_Editor = false;
    ext_dll_handle = NULL;  // DTReport2.dll

    //... Clear IN OUT buffers
    IN_buffer.clear();
    OUT_buffer.clear();

    //...
    //message.setWindowIcon(QIcon(":/images/DTm.ico"));
    //message.setWindowIcon(QIcon(":/images/RT.ico"));
    message.setFont(qApp->font());

    //...
    label_gif = new QLabel(this);
    label_gif->setFixedSize(48,48);
    label_gif->raise();
    obj_gif = new QMovie(":/images/flat/wait_1.gif");
    label_gif->setMovie(obj_gif);
    label_gif->setVisible(false);

}
//-----------------------------------------------------------------------------
//--- ~Test_editor()
//-----------------------------------------------------------------------------
Test_editor::~Test_editor()
{
    HINSTANCE handle;
    Test_Interface  *editor;

    foreach(editor, Map_editor)
    {
        editor->Destroy_Win();
        editor->Destroy();
    }
    Map_editor.clear();
    Map_editorNAME.clear();

    foreach(handle, List_handle)
    {
        if(handle) ::FreeLibrary(handle);
    }
    List_handle.clear();

    Map_editorEXT.clear();
    Map_editorEXT_formID.clear();
    if(ext_dll_handle)
    {
        if(Ext_Editor)
        {
            Ext_Editor = false;
            close_Editor();
        }
        //::FreeLibrary(ext_dll_handle);
    }

    delete obj_gif;
    delete label_gif;

    delete  tool_editor;
    delete  tests_list;
    //delete  edit_test;
    //delete  box_TestList;
    delete  layout_plugin;
    delete  widget_editor;
    delete  group_editor;
    delete  main_spl;
    delete  close_button;
    delete  box_editor;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Test_editor::readCommonSettings()
{
    QString text;
    QString dir_SysTranslate;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        Lang_editor = text;
        if(translator.load(":/translations/test_editor_" + text + ".qm"))
        {            
            qApp->installTranslator(&translator);
        }

        // ... System Language ...
        dir_SysTranslate = dir_path + "/tools/translations";
        if(qt_translator.load("qt_" + text + ".qm", dir_SysTranslate))
        {
            qApp->installTranslator(&qt_translator);
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
//--- showEvent
//-----------------------------------------------------------------------------
void Test_editor::showEvent(QShowEvent *event)
{    
    QList<int> list_spl;
    list_spl << 300 << 1000;

    Clear_editor();    
    Fill_ListTests();    

    main_spl->setSizes(list_spl);

    //int w = main_spl->sizes().at(0);
    //tests_list->header()->resizeSection(0, w*0.8);

    if(!disable_action && ax_user == NULL)
    {
        new_Container->setDisabled(true);
        new_Test->setDisabled(true);
        copy_Test->setDisabled(true);
        save_Test->setDisabled(true);        
    }
    if(!From_Protocol)
    {
        tests_list->setDragEnabled(true);
        tests_list->viewport()->setAcceptDrops(true);
        tests_list->setDropIndicatorShown(true);
        tests_list->setDragDropMode(QAbstractItemView::InternalMove);

        tests_list->message = &message;

    }
    else
    {
        close_button->setText(tr("Cancel"));
    }
}
//-----------------------------------------------------------------------------
//--- closeEvent
//-----------------------------------------------------------------------------
void Test_editor::closeEvent(QCloseEvent *event)
{
    //qDebug() << "close editor: (From_Protocol) " << From_Protocol;
    if(!From_Protocol)
    {
        if(!Clear_editor())
        {
            //qDebug() << "Clear_editor: false...";
            event->ignore();
        }
    }
    event->accept();

}
//-----------------------------------------------------------------------------
//--- eventFilter
//-----------------------------------------------------------------------------
/*bool Test_editor::eventFilter(QObject *obj, QEvent *e)
{

    qDebug() << "Drag&Drop: " << e->type();


    return QDialog::eventFilter(obj, e);
}*/

//-----------------------------------------------------------------------------
//--- Fill_ListTests
//-----------------------------------------------------------------------------
void Test_editor::Fill_ListTests(QString name)
{
    int i,j;
    int id;
    QString name_str;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_cur;
    QTreeWidgetItem *item_parent;
    QTreeWidgetItem *item_root;
    QTreeWidgetItem *item_up;
    rt_Test         *ptest;
    QString text,str;
    int type;
    bool find;
    bool sector_BasicTests = true;
    int count_test = TESTs->size();
    QList<int> list_methods;
    QStringList list;
    QList<QTreeWidgetItem*> list_item;
    QVector<rt_Test*> List_Container;

    QMap<QString, rt_Test*> temp_Map;
    QMultiMap<QString, rt_Test*> TESTs_dtr;
    QString catalog;
    QVector<rt_Test*> List_all;


    QString userName = "...";
    if(ax_user && !From_Protocol) userName = ax_user->property("userName").toString();
    if(From_Protocol && prot)
    {
        userName = QString::fromStdString(prot->owned_by);
    }

    QMultiMap<int, rt_Test*> TESTs_Temporary;
    for(i=0; i<count_test; i++)
    {
        ptest = TESTs->at(i);
        type = ptest->header.Type_analysis;

        if(type < 0x2000) TESTs_Temporary.insert(type, ptest);
        else
        {
            catalog = QString::fromStdString(ptest->header.Catalog);
            TESTs_dtr.insert(catalog, ptest);
        }

        //TESTs_Temporary.insert(type, ptest);
        //qDebug() << type << QString::fromStdString(ptest->header.Name_Test);
    }

    foreach(ptest, TESTs_Temporary.values())
    {
        List_all.append(ptest);
        //qDebug() << "Base_test: " << QString::fromStdString(ptest->header.Name_Test) << QString::fromStdString(ptest->header.Catalog);
    }

    foreach(text, TESTs_dtr.keys())
    {
        if(list.contains(text)) continue;
        else list.append(text);

        temp_Map.clear();
        QMultiMap<QString, rt_Test*>::iterator it = TESTs_dtr.find(text);
        while(it != TESTs_dtr.end() && it.key() == text)
        {
            ptest = (rt_Test*)it.value();
            temp_Map.insert(QString::fromStdString(ptest->header.Name_Test), ptest);
            ++it;
        }

        foreach(ptest, temp_Map.values())
        {
            List_all.append(ptest);
        }
    }

    /*foreach(ptest, TESTs_dtr.values())
    {
        List_all.append(ptest);
    }*/


    tests_list->blockSignals(true);

    QFont f = qApp->font();
    f.setItalic(true);
    QBrush b (Qt::red);

    QStringList pp;
    pp << tr("List of Tests") << " " << tr("ID");

    tests_list->clear();
    tests_list->setColumnCount(3);
    tests_list->setHeaderLabels(pp);
    tests_list->setColumnHidden(2, true);    
    tests_list->header()->setStretchLastSection(false);
    tests_list->header()->resizeSection(1, 50);
    tests_list->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tests_list->header()->setSectionResizeMode(1, QHeaderView::Fixed);

    list_methods = Map_editorNAME.keys();           // My tests
    list_methods.append(Map_editorEXT.keys());      // CpALL tests

    //qDebug() << "list_methods: " << list_methods  << Map_editorNAME << Map_editorEXT;

    item_root = new QTreeWidgetItem(tests_list);
    item_root->setText(0, tr("Tests from ") + userName);
    item_root->setIcon(0, QIcon(":/images/flat/folder_new_16.png"));
    item_root->setFlags(item_root->flags() ^ Qt::ItemIsDragEnabled);
    item_root->setFlags(item_root->flags() ^ Qt::ItemIsDropEnabled);

    //for(i=0; i<TESTs_Temporary.size(); i++)
    for(i=0; i<List_all.size(); i++)
    {
        //ptest = TESTs_Temporary.values().at(i);
        ptest = List_all.at(i);
        type = ptest->header.Type_analysis;
        text = "...";

        if(type == 0x0033)  // Container
        {
            List_Container.append(ptest);
            continue;
        }

        if(Map_editorNAME.contains(type)) text = Map_editorNAME.value(type);
        if(Map_editorEXT.contains(type)) text = Map_editorEXT.value(type);
        if(text == "...") text = tr("Unknown");

        if(type < 0x2000)   //my tests
        {
            if(QString::fromStdString(ptest->header.Catalog).trimmed().isEmpty() && !text.isEmpty())
            {
                ptest->header.Catalog = text.toStdString();
            }
        }

        //qDebug() << "Catalog edit_test: " << QString::fromStdString(ptest->header.Name_Test) << QString::fromStdString(ptest->header.Catalog) << text;

        // Temporary
        str = QString::fromStdString(ptest->header.Catalog);
        if(!str.isEmpty()) text = str;
        if(type == 0) text = tr("Simple method");


        if(type >= 0x2000 && sector_BasicTests && i)
        {
            sector_BasicTests = false;
            item = new QTreeWidgetItem(item_root);
            item->setText(0, "-----");
            item->setDisabled(true);
            item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
        }

        list.clear();
        list = text.split("\\");

        list_item.clear();
        for(j=0; j<item_root->childCount(); j++)
        {
            list_item.append(item_root->child(j));
        }

        id = 0;
        foreach(text, list)
        {
            find = false;
            foreach(item, list_item)
            {
                if(item->text(0) == text)
                {
                    find = true;
                    break;
                }
            }
            if(find)
            {
                item_cur = item;
            }
            else
            {
                if(!id) item_cur = new QTreeWidgetItem(item_root);
                else    item_cur = new QTreeWidgetItem(item_parent);
                //item_cur->setExpanded(true);
                item_cur->setIcon(0, QIcon(":/images/flat/folder_new_16.png"));
                item_cur->setFlags(item_cur->flags() ^ Qt::ItemIsDragEnabled);

                if(From_Protocol || type < 0x2000)
                {
                    item_cur->setFlags(item_cur->flags() ^ Qt::ItemIsDropEnabled);
                    item_cur->setIcon(0, QIcon(":/images/flat/folder_new_fix.png"));
                }

                if(text == tr("Unknown"))
                {
                    item_cur->setFont(0,f);
                    item_cur->setForeground(0,b);
                    item_cur->setFlags(item_cur->flags() ^ Qt::ItemIsDropEnabled);
                }
                item_cur->setText(0, text);

            }
            item_parent = item_cur;
            list_item.clear();
            for(j=0; j<item_cur->childCount(); j++)
            {
                list_item.append(item_cur->child(j));
            }

            id++;
        }

        item = new QTreeWidgetItem(item_cur);
        name_str = QString::fromStdString(ptest->header.Name_Test);
        if(map_TestTranslate->size()) name_str = map_TestTranslate->value(name_str, name_str);
        item->setText(0, name_str);
        item->setText(1, QString::fromStdString(ptest->header.version));
        item->setText(2, QString::number((int)ptest));
        item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
        if(ptest->header.Type_analysis < 0x1000) item->setFlags(item->flags() ^ Qt::ItemIsDragEnabled);

        if(item->text(0) == name)
        {
            tests_list->setCurrentItem(item);
            item_up = item->parent();
            while(item_up)
            {
                item_up->setExpanded(true);
                item_up = item_up->parent();
            }
        }

        if(From_Protocol)
        {
            item->setFlags(item->flags() ^ Qt::ItemIsDragEnabled);
        }


        // Icons
        if(ptest->header.Hash == "ok") item->setIcon(0, logo);
        else item->setIcon(0, logonull);
        //...

        if(!list_methods.contains(type)) item->setDisabled(true);
    }

    if(List_Container.size() && !From_Protocol)   // Container
    {
        temp_Map.clear();
        foreach(ptest, List_Container)
        {
            temp_Map.insert(QString::fromStdString(ptest->header.Name_Test), ptest);
        }

        item = new QTreeWidgetItem(item_root);
        item->setText(0, "-----");
        item->setDisabled(true);
        item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);

        item_cur = new QTreeWidgetItem(item_root);
        item_cur->setFlags(item_cur->flags() ^ Qt::ItemIsDragEnabled);
        item_cur->setFlags(item_cur->flags() ^ Qt::ItemIsDropEnabled);
        item_cur->setIcon(0, QIcon(":/images/flat/folder_new_fix.png"));
        item_cur->setText(0, tr("Container"));

        foreach(ptest, temp_Map.values())
        {
            item = new QTreeWidgetItem(item_cur);
            item->setText(0, QString::fromStdString(ptest->header.Name_Test));
            item->setText(1, QString::fromStdString(ptest->header.version));
            item->setText(2, QString::number((int)ptest));
            item->setFlags(item->flags() ^ Qt::ItemIsDropEnabled);
            item->setFlags(item->flags() ^ Qt::ItemIsDragEnabled);
        }
        List_Container.clear();
        temp_Map.clear();
    }

    item_root->setExpanded(true);
    TESTs_Temporary.clear();
    List_all.clear();
    TESTs_dtr.clear();

    //tests_list->setColumnWidth(0, tests_list->width()*2);

    //tests_list->setSortingEnabled(true);
    //tests_list->sortItems(0, Qt::AscendingOrder);

    tests_list->blockSignals(false);
}

//-----------------------------------------------------------------------------
//--- LoadTest_Plugins()
//-----------------------------------------------------------------------------
bool Test_editor::LoadTest_Plugins()
{
    QString fn;
    QVector<QString> info_test;
    //std::vector<std::string> info_test;
    QVector<std::string> info;
    QString text;
    int method_ID;

    bool ok;

    QDir pluginsDir(qApp->applicationDirPath());
    pluginsDir.cd("test_plugins");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files))
    {
        dll_handle = NULL;
        alg_editor = NULL;
        info_test.clear();

        fn = "test_plugins\\" + fileName;
        dll_handle = ::LoadLibraryW(fn.toStdWString().c_str());        

        if(dll_handle)
        {
            InterfaceTest_factory factory_func = reinterpret_cast<InterfaceTest_factory>(
                                            ::GetProcAddress(dll_handle,"createTest_plugin@0"));
            if(factory_func)
            {
                alg_editor = factory_func();
                if(alg_editor)
                {
                    alg_editor->GetInformation(&info_test);                    
                    //info = QVector<std::string>::fromStdVector(info_test);
                    //text = QString::fromStdString(info.at(0));
                    text = info_test.at(0);
                    method_ID = text.toInt(&ok,16);
                    //text = QString::fromStdString(info.at(1));
                    text = info_test.at(1);
                    Map_editorNAME.insert(method_ID, text);
                    Map_editor.insert(method_ID, alg_editor);
                    List_handle.append(dll_handle);
                    //qDebug() << method_ID << text;                    
                }
                else
                {
                    if(dll_handle) ::FreeLibrary(dll_handle);
                }
                alg_editor = NULL;
            }            
        }        
    }

    return(true);
}
//-----------------------------------------------------------------------------
//--- LoadTest_ExtPlugins()
//-----------------------------------------------------------------------------
bool Test_editor::LoadTest_ExtPlugins()
{
    QFileInfo fn;
    QString text,str;
    int method_ID;
    int id = 0x20000;
    QByteArray ba;
    quint16 crc16;
    char *DisplayName;
    char *Path;
    int release, build;

    show_Editor = NULL;
    close_Editor = NULL;
    resize_Editor = NULL;
    save_Editor = NULL;
    get_Info = NULL;

    if(!ext_dll_handle) {Map_editorEXT.clear(); return(false);}

    if(ext_dll_handle)
    {
        show_Editor = (ShowEditor)(::GetProcAddress(ext_dll_handle,"ShowServiceEditor"));
        close_Editor = (CloseEditor)(::GetProcAddress(ext_dll_handle,"CloseServiceEditor"));
        resize_Editor = (ResizeEditor)(::GetProcAddress(ext_dll_handle,"ResizeServiceEditorForm"));
        save_Editor = (SaveEditor)(::GetProcAddress(ext_dll_handle,"SaveService"));
        get_Info = (GetDTRDisplayInfo)(::GetProcAddress(ext_dll_handle,"GetDTRDisplayInfo"));

        if(!show_Editor || !close_Editor || !resize_Editor || !save_Editor || !get_Info) {Map_editorEXT.clear();  return(false);}
    }


    QDir pluginsDir(qApp->applicationDirPath());
    pluginsDir.cd("forms");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files))
    {
        fn.setFile(pluginsDir, fileName);
        if(fn.exists() && fn.suffix() == "dtr")
        {
            text = fn.baseName();
            if(text == "Default" || text == "ReportSummary" || text == "ReportTitle" || text == "DTReport Studio project") continue;

            ba.clear();
            ba.append(text.toLatin1());
            crc16 = qChecksum(ba.data(),ba.size());
            method_ID = id + crc16;
            get_Info(text.toLatin1().data(), &DisplayName, &Path, &release, &build);
            str = QString::fromUtf8(DisplayName);
            if(str.trimmed().isEmpty()) continue;
            Map_editorEXT.insert(method_ID, str);
            Map_editorEXT_formID.insert(method_ID, text);

            //qDebug() << "text,str: " << method_ID << text << str;
        }        
    }

    //qDebug() << "Map_editorEXT: " << Map_editorEXT;
    return(true);
}

//-----------------------------------------------------------------------------
//--- Save_SimpleTest(rt_Test *ptest)
//-----------------------------------------------------------------------------
bool Test_editor::Save_SimpleTest(rt_Test *ptest)
{
    QString text;
    bool ok;
    int i;
    rt_Preference *preference;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Simple_Test");

    // active_channels
    text = QString::number(ptest->header.Active_channel, 16);

    CommonSettings->setValue("active_channels", text);

    // volumeTube
    text = QString::number(ptest->header.Volume_Tube);
    CommonSettings->setValue("volumeTube", text);

    // comments
    text = QString::fromStdString(ptest->header.comment);
    CommonSettings->setValue("comments", text);

    // Property
    for(i=0; i<ptest->preference_Test.size(); i++)
    {
        preference = ptest->preference_Test.at(i);
        text = QString::fromStdString(preference->name);

        if(text == CRITERION_POSRES)
        {
            CommonSettings->setValue("CriterionPositiveResult", QString::fromStdString(preference->value));
            continue;
        }
        if(text == CRITERION_VALIDITY)
        {
            CommonSettings->setValue("CriterionValidity", QString::fromStdString(preference->value));
            continue;
        }
        if(text == USE_AFF)
        {
            CommonSettings->setValue("Use_AFF", QString::fromStdString(preference->value));
            continue;
        }
        if(text == CRITERION_AFF)
        {
            CommonSettings->setValue("CriterionAFF", QString::fromStdString(preference->value));
            continue;
        }
        if(text == CRITERION_AFF_MIN)
        {
            CommonSettings->setValue("CriterionAFF_MIN", QString::fromStdString(preference->value));
            continue;
        }
        if(text == CRITERION_AFF_MAX)
        {
            CommonSettings->setValue("CriterionAFF_MAX", QString::fromStdString(preference->value));
            continue;
        }
        if(text == SIGMOIDVAL_MIN)
        {
            CommonSettings->setValue("BordersSigmoidValidityMin", QString::fromStdString(preference->value));
            continue;
        }
        if(text == SIGMOIDVAL_MAX)
        {
            CommonSettings->setValue("BordersSigmoidValidityMax", QString::fromStdString(preference->value));
            continue;
        }
        if(text == USE_THRESHOLD)
        {
            CommonSettings->setValue("Use_Threshold", QString::fromStdString(preference->value));
            continue;
        }
        if(text == VALUE_THRESHOLD)
        {
            CommonSettings->setValue("Value_Threshold", QString::fromStdString(preference->value));
            continue;
        }
        if(text == EXPOSURE)
        {
            CommonSettings->setValue("Exposure", QString::fromStdString(preference->value));
            continue;
        }
        if(text == FLUOROFORS)
        {
            CommonSettings->setValue("Fluorofors", QString::fromStdString(preference->value));
            continue;
        }
        if(text == MC_FWHM_BORDER)
        {
            CommonSettings->setValue("FWHM", QString::fromStdString(preference->value));
            continue;
        }
        if(text == MC_PEAKS_BORDER)
        {
            CommonSettings->setValue("peaks_Border", QString::fromStdString(preference->value));
            continue;
        }
    }



    // Program
    QVector<string> pro = QVector<string>::fromStdVector(ptest->header.program);
    QStringList list;
    for(int i=0; i<pro.size(); i++) list.append(QString::fromStdString(pro.at(i)));
    text = list.join('#');
    CommonSettings->setValue("program", text);

    CommonSettings->endGroup();
    delete CommonSettings;

    return(true);
}

//-----------------------------------------------------------------------------
//--- Save_DBaseTests(rt_Test*, bool)
//-----------------------------------------------------------------------------
bool Test_editor::Save_DBaseTests(rt_Test* ptest, bool new_test)
{
    int i;
    QString ID_test = QString::fromStdString(ptest->header.Name_Test);// ID_Test);
    QString xml = "";
    rt_Preference   *preference_test;
    QAxObject* axItemCache;
    int id_user;

    if(!ax_user) return(false);

    id_user = ax_user->property("userId").toInt();

    for(i=0; i<ptest->preference_Test.size(); i++)
    {
        preference_test = ptest->preference_Test.at(i);
        if(preference_test->name == "xml_node")
        {
            xml = QString::fromStdString(preference_test->value);
            //qDebug() << "xml: " << xml;
            //qDebug() << "ok saveDB";
            break;
        }
    }
    if(xml.isEmpty()) return(false);

    if(new_test) axItemCache = ax_user->querySubObject("addCacheEntry(QString,QString)", ID_test, "test");
    else         axItemCache = ax_user->querySubObject("getCacheEntryIndex(QString,QString,int)", ID_test, "test", id_user);

    //qDebug() << "new_test: " << new_test << axItemCache;

    if(axItemCache) axItemCache->setProperty("dataEntry", xml);

    delete axItemCache;

    return(true);
}
//-----------------------------------------------------------------------------
//--- Save_Test()
//-----------------------------------------------------------------------------
void Test_editor::Save_Test(bool confirm)
{
    int i;
    string xml = "";
    string name = "";
    rt_Test *ptest;
    QDomDocument doc;
    QDomElement root;
    QDomElement element;
    QDomElement ref;
    rt_Preference   *preference;
    QString xml_str;
    bool sts;
    QString status;
    bool new_test = true;
    bool simple_test = false;
    QString text;
    QString text_xml;
    int res;
    char *pchar;
    QString name_test;
    int method;
    bool modified;
    bool NotEqual = false;
    message.setWindowTitle(tr("Save test"));
    QStringList list = editor_Name->text().split(":  ");

    QVector<QString> alg_editor_Info;

    if(alg_editor)
    {
        alg_editor->Get_Test(&xml,&name);
        name_test = QString::fromStdString(name);
        text_xml = QString::fromStdString(xml);

        /*if(Current_Catalog.isEmpty())
        {
            alg_editor->GetInformation(&alg_editor_Info);
            if(alg_editor_Info.size() > 1) Current_Catalog = alg_editor_Info.at(1);
        }*/
    }
    if(Ext_Editor)
    {        
        save_Editor(&pchar, &modified);
        if(!confirm) modified = true;
        //qDebug() << "modified: (save) " << modified;
        text_xml = QString::fromUtf8(pchar);
        name_test = QString(list.at(1)).trimmed();
        method = Map_editorEXT.key(list.at(0), 0);

        name = name_test.toStdString();

        //qDebug() << "method: " << method << name_test;
    }

    Add_CatalogXML(text_xml);

    OUT_buffer = text_xml;
    if(IN_buffer != OUT_buffer) NotEqual = true;
    if(Ext_Editor) NotEqual = modified;    

    if(!NotEqual)    // Do not save!
    {
        if(!From_Protocol) QMessageBox::information(this, tr("Save test"),  tr("No changes to file!"), QMessageBox::Ok);
        return;
    }

    if(doc.setContent(text_xml))
    {
        root = doc.documentElement();

        if(!XML_Signature.isNull())
        {
            root.appendChild(XML_Signature);
        }

        // check Contaneir-test on equal program
        if(alg_editor && Map_editor.key(alg_editor, 0x0000) == 0x0033)
        {
            if(!Check_Programm_inContainer(root))
            {
                text = tr("Attention! Unable to save the container!") + "\r\n" +
                       tr("Different amplification programs were found for tests...");
                message.setStandardButtons(QMessageBox::Ok);
                message.setIcon(QMessageBox::Warning);
                message.setText(text);
                message.exec();
                return;
            }
        }

        //

        foreach(ptest, *TESTs)
        {
            if(ptest->header.Name_Test == name_test.toStdString())
            {
                new_test = false;
                if(ptest->header.Name_Test == "simple" &&
                   ptest->header.ID_Test == "-1" &&
                   ptest->header.Type_analysis == 0x0000) simple_test = true;

                break;
            }
        }

        if(confirm && alg_editor) // only for my tests
        {
            message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            if(new_test) text = tr("Do You want to save the new test: ") + name_test + "?";
            else text = text = tr("Do You want to save changes in the test: ") + name_test + "?";
            message.setText(text);
            message.setIcon(QMessageBox::Question);
            res = message.exec();
            if(res == QMessageBox::No) return;
        }

        if(new_test) ptest = new rt_Test();                

        LoadXML_Test(root, ptest);

        if(axgp)
        {
            foreach(preference, ptest->preference_Test)
            {
                if(preference->name == "xml_node")
                {
                    xml_str = QString::fromStdString(preference->value);
                    sts = axgp->dynamicCall("loadXmlTest(QString)", xml_str).toBool();
                    if(sts) status = "ok";
                    else status = "none";
                    ptest->header.Hash = status.toStdString();
                    break;
                }
            }
        }


        if(new_test) TESTs->append(ptest);
        Fill_ListTests(QString::fromStdString(name));

        if(ax_user && !simple_test) Save_DBaseTests(ptest, new_test);
        if(ax_user && simple_test) Save_SimpleTest(ptest);

        IN_buffer = OUT_buffer;
        //ptest->header.Hash = "none";    // Clear verification

        /*QFile fn_out(qApp->applicationDirPath() + "/OUT_save.xml");
        if(fn_out.open(QIODevice::WriteOnly))
        {
            QTextStream(&fn_out) << doc.toString();
            fn_out.close();
            //qDebug() << "ok - Save test!";
        }*/


        if(Ext_Editor)  // Clear and reOpen test
        {
            Clear_and_Open_Editor(ptest);
            QCursor::setPos(mapToGlobal(QPoint(width()/2, height()/2)));
        }

        if(apply_button->isVisible()) apply_button->setEnabled(true);

    }
}
//-----------------------------------------------------------------------------
//--- Clear_and_Open_Editor()
//-----------------------------------------------------------------------------
void Test_editor::Clear_and_Open_Editor(rt_Test *ptest)
{
    // 1. Clear
    Clear_editor(false);

    // 2. reOpen
    edit_Test(ptest);
}

//-----------------------------------------------------------------------------
//--- Temporary_Insert
//-----------------------------------------------------------------------------
void Test_editor::Temporary_Insert(QString &xml, QDomDocument &doc, QDomElement &root, int method)
{
    QDomElement element;
    QDomElement ref;

    element = MakeElement(doc, "method", QString("%1").arg(method));
    ref = root.firstChildElement("nameTest");
    root.insertAfter(element,ref);

    element = MakeElement(doc, "properties", "");
    ref = root.firstChildElement("DTReport");
    root.insertAfter(element,ref);

    xml = doc.toString();
}

//-----------------------------------------------------------------------------
//--- Close_Editor()
//-----------------------------------------------------------------------------
void Test_editor::Close_Editor()
{
    //qDebug() << "close editor: (From_Protocol) " << From_Protocol;
    if(!From_Protocol)
    {
        if(!Clear_editor())
        {
            //qDebug() << "Clear_editor: false...";
            return;
        }
    }

    setResult(QDialog::Rejected);
    close();
}
//-----------------------------------------------------------------------------
//--- Apply_Editor()
//-----------------------------------------------------------------------------
void Test_editor::Apply_Editor()
{
    Save_Test(false);
    Clear_editor(false);
    //if(!Clear_editor()) return;

    setResult(QDialog::Accepted);
    close();
}
//-----------------------------------------------------------------------------
//--- Copy_Test()
//-----------------------------------------------------------------------------
void Test_editor::Copy_Test()
{
    bool ok;
    int method_id;
    QString name, text, name_test;
    QInputDialog inputDialog;
    rt_Test *ptest;
    rt_Test *ptest_old = NULL;
    QStringList list = editor_Name->text().split(":  ");

    QDomDocument doc_temp;
    QDomElement  xml_item;
    QDomElement  xml_signature;
    QDomElement  xml_element;

    if(alg_editor)
    {
        method_id = Map_editor.key(alg_editor);
        text = Map_editorNAME.value(method_id);
    }
    if(Ext_Editor)
    {
        text = QString(list.at(0)).trimmed();
        method_id = Map_editorEXT.key(text, 0);
    }
    //qDebug() << "Map_editorEXT.key" << Map_editorEXT << text << method_id << list;

    if(method_id == 0) return;

    name = QString(list.at(1)).trimmed();    
    foreach(ptest_old, *TESTs)
    {
        name_test = QString::fromStdString(ptest_old->header.Name_Test);
        if(map_TestTranslate->size()) name_test = map_TestTranslate->value(name_test,name_test);
        if(name_test == name) break;
    }
    if(!ptest_old) return;

    inputDialog.setWindowFlags(inputDialog.windowFlags() & (~Qt::WindowContextHelpButtonHint));
    name = inputDialog.getText(this, tr("Choose name of the Test..."),
                                         tr("Name of the Test:"), QLineEdit::Normal,
                                         "", &ok, inputDialog.windowFlags());

    if(ok && !name.isEmpty())
    {
        // check on unique name test
        foreach(ptest, *TESTs)
        {
            if(ptest->header.Name_Test == name.toStdString())
            {
                QMessageBox::warning(this, tr("New Test"), tr("The test with this name already exists! Select other name..."));
                return;
            }
        }

        text = QString("%1:  %2").arg(text).arg(name);
        editor_Name->setText(text);

        if(alg_editor)
        {
            alg_editor->Set_Test("", name.toStdString());
        }
        if(Ext_Editor)
        {
            xml_item = SaveXML_Test(doc_temp, ptest_old);                           // create xml text

            xml_signature = xml_item.firstChildElement("Signature");                // Remove Signature item
            if(!xml_signature.isNull()) xml_item.removeChild(xml_signature);

            xml_element = xml_item.firstChildElement("IDTest");                     // new ID
            if(!xml_element.isNull())
            {
                xml_item.replaceChild(MakeElement(doc_temp,"IDTest",GetRandomString()), xml_element);
            }
            xml_element = xml_item.firstChildElement("nameTest");                   // new name test
            if(!xml_element.isNull())
            {
                xml_item.replaceChild(MakeElement(doc_temp,"nameTest",name), xml_element);
            }

            doc_temp.appendChild(xml_item);
            doc_temp.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
            Create_DefaultCommonProperties(doc_temp, xml_item);

            text = doc_temp.toString();
            IN_buffer = text;

            QFile fn_out("OUT_copy.xml");
            if(fn_out.open(QIODevice::WriteOnly))
            {
                QTextStream(&fn_out) << text;
                fn_out.close();
                //qDebug() << "ok - Copy!";
                //qDebug() << text;
            }

            close_Editor();
            //Sleep(50);
            show_Editor((HWND)widget_editor->winId(), text.toUtf8().data(), From_Protocol);
            resize_Editor((HWND)widget_editor->winId());
        }
    }
}
//-----------------------------------------------------------------------------
//--- create_newContaner()
//-----------------------------------------------------------------------------
void Test_editor::create_newContaner()
{
    bool ok;
    int method_id;
    QString text;
    QStringList items;
    rt_Test *ptest;
    rt_Preference *preference;
    QDomDocument doc_temp;
    QDomElement  xml_item;
    QList<QString> list = Map_editorNAME.values();
    QMap<int, QString> map_AllEditor;

    QString name_Container = tr("Container");
    int id_Container = 0x0033;

    if(!Clear_editor()) return;

    // 1. name of Container
    QInputDialog inputDialog;
    inputDialog.setMinimumWidth(1000);
    inputDialog.setFont(qApp->font());
    inputDialog.setWindowFlags(inputDialog.windowFlags() & (~Qt::WindowContextHelpButtonHint));
    QString name = inputDialog.getText(this, tr("Choose name of the Container..."),
                                         tr("Name of the Container:"), QLineEdit::Normal,
                                         "", &ok, inputDialog.windowFlags());
    if(ok && !name.isEmpty())
    {
        foreach(ptest, *TESTs)
        {
            if(ptest->header.Name_Test == name.toStdString())
            {
                message.setStandardButtons(QMessageBox::Ok);
                text = tr("The test with this name already exists! Select other name...");
                message.setText(text);
                message.setIcon(QMessageBox::Warning);
                message.exec();
                //QMessageBox::warning(this, tr("New Container"), tr("The test with this name already exists! Select other name..."));
                return;
            }
        }

        text = QString("%1:  %2").arg(name_Container).arg(name);
        editor_Name->setText(text);

        alg_editor = Map_editor.value(id_Container);
        Load_editor("", name);


        alg_editor->Reserve(TESTs);
        save_Test->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
//--- create_newTest()
//-----------------------------------------------------------------------------
void Test_editor::create_newTest()
{
    bool ok;
    int method_id;
    QString text;
    QStringList items;
    rt_Test *ptest;
    rt_Preference *preference;
    QDomDocument doc_temp;
    QDomElement  xml_item;
    QList<QString> list = Map_editorNAME.values();
    QMap<int, QString> map_AllEditor;

    if(!Clear_editor()) return;

    // My tests
    foreach (text, list)
    {
        //qDebug() << "My tests: " << text;
        if(Map_editorNAME.key(text,0) == 0) continue;           // Simple
        if(Map_editorNAME.key(text,0) == 0x0033) continue;      // Container
        items.append(text);
        map_AllEditor.insert(Map_editorNAME.key(text,0), text);
    }
    //qDebug() << "list: " << items << ;

    // EXT tests
    list.clear();
    list = Map_editorEXT.values();
    if(list.size()) items.append("-----");
    foreach (text, list)
    {
        if(Map_editorEXT.key(text,0) == 0) continue;
        items.append(text);
        map_AllEditor.insert(Map_editorEXT.key(text,0), text);
    }    

    // 1. type method
    QInputDialog inputDialog;
    inputDialog.setMinimumWidth(1000);
    inputDialog.setFont(qApp->font());

    //connect(&inputDialog, SIGNAL(textValueSelected(QString)), this, SLOT(InputDialog_Select(QString)));

    inputDialog.setWindowFlags(inputDialog.windowFlags() & (~Qt::WindowContextHelpButtonHint));
    QString item = inputDialog.getItem(this, tr("Choose type of the Test..."),
                                        tr("List of type Tests:"), items, 0, false, &ok, inputDialog.windowFlags());    

    if(item.startsWith("-----"))
    {
        ok = false;
        message.setStandardButtons(QMessageBox::Ok);
        text = tr("Attention! Select the correct test type");
        message.setText(text);
        message.setIcon(QMessageBox::Warning);
        message.exec();

        return;
    }


    //qDebug() << "inputDialog: " << item << ok << Map_editorNAME;

    if(ok && !item.isEmpty())
    {
        // 2. name test
        QString name = inputDialog.getText(this, tr("Choose name of the Test..."),
                                             tr("Name of the Test:"), QLineEdit::Normal,
                                             "", &ok, inputDialog.windowFlags());

        if(name.trimmed().isEmpty())
        {
            message.setStandardButtons(QMessageBox::Ok);
            text = tr("Attention! Enter a name for the test");
            message.setText(text);
            message.setIcon(QMessageBox::Warning);
            message.exec();
            return;
        }

        if(ok && !name.isEmpty())
        {
            // 3. check on unique name test
            foreach(ptest, *TESTs)
            {
                if(ptest->header.Name_Test == name.toStdString())
                {
                    message.setStandardButtons(QMessageBox::Ok);
                    text = tr("The test with this name already exists! Select other name...");
                    message.setText(text);
                    message.setIcon(QMessageBox::Warning);
                    message.exec();
                    //QMessageBox::warning(this, tr("New Test"), tr("The test with this name already exists! Select other name..."));
                    return;
                }
            }

            method_id = map_AllEditor.key(item,0);

            if(method_id > 0)
            {
                text = QString("%1:  %2").arg(item).arg(name);
                editor_Name->setText(text);                


                if(method_id < 0x20000)         // My tests
                {
                    alg_editor = Map_editor.value(method_id);
                    Load_editor("", name);
                }
                else                            // Ext tests (dtr)
                {
                    ptest = new rt_Test();

                    ptest->header.Name_Test = name.toStdString();
                    ptest->header.ID_Test = GetRandomString().toStdString();
                    ptest->header.Type_analysis = method_id;
                    ptest->header.Catalog = item.toStdString();
                    preference =  new rt_Preference();
                    ptest->preference_Test.push_back(preference);
                    preference->name = "FormID";
                    preference->value = Map_editorEXT_formID.value(method_id,"").toStdString();
                    xml_item = SaveXML_Test(doc_temp, ptest);   // create xml text
                    delete ptest;
                    doc_temp.appendChild(xml_item);
                    doc_temp.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");

                    Create_DefaultCommonProperties(doc_temp, xml_item);

                    text = doc_temp.toString();
                    Ext_Editor = true;

                    QFile outfile("new_EXT.xml");
                    if(outfile.open(QIODevice::WriteOnly))
                    {
                        QTextStream(&outfile) << doc_temp.toString();
                        outfile.close();
                    }


                    show_Editor((HWND)widget_editor->winId(), text.toUtf8().data(), From_Protocol);
                    resize_Editor((HWND)widget_editor->winId());
                }
            }
            save_Test->setEnabled(true);            
        }
    }
}
//-----------------------------------------------------------------------------
//--- Check_Programm_inContainer(QDomElement &node)
//-----------------------------------------------------------------------------
bool Test_editor::Check_Programm_inContainer(QDomElement &node)
{
    int i;
    QString Name, Value;
    QString text;
    QStringList list;
    bool res = true;
    QVector<rt_Test*> tests;
    rt_Test *ptest;
    QVector<string> First_Pro;
    QVector<string> Next_Pro;
    int vol = 35;

    QDomElement element;
    QDomNode    properties_item;

    if(node.isElement())
    {
        element = node.firstChildElement("properties");
        if(element.isElement())
        {
            for(i=0; i<element.childNodes().size(); i++)
            {
                properties_item = element.childNodes().at(i);
                Name = properties_item.firstChildElement("name").toElement().text();
                Value = properties_item.firstChildElement("value").toElement().text();
                if(Name == "Checked_Tests")
                {
                    list = Value.split("\r\n");

                    foreach(text, list)
                    {
                        foreach(ptest, *TESTs)
                        {
                            if(ptest->header.ID_Test == text.toStdString())
                            {
                                tests.append(ptest);
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if(tests.size())
    {
        i = 0;
        foreach(ptest, tests)
        {
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
                i++;
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
                res = false;
                break;
            }

            i++;
        }
    }

    //qDebug() << "Res tests: " << res;
    tests.clear();

    return(res);
}

//-----------------------------------------------------------------------------
//--- InputDialog_Select(QString str)
//-----------------------------------------------------------------------------
void Test_editor::InputDialog_Select(QString str)
{
    //qDebug() << "InputDialog_Select: " << str;
}

//-----------------------------------------------------------------------------
//--- auto_resize
//-----------------------------------------------------------------------------
void Test_editor::auto_resize(QString text)
{
    QFont f = editor_Name->font();
    QFontMetrics fm(f);

    int pixelsWide = fm.width(text);
    int pixelsHigh = fm.height();


    editor_Name->setFixedWidth(pixelsWide + 100);
}
//-----------------------------------------------------------------------------
//--- Add_CatalogXML
//-----------------------------------------------------------------------------
void Test_editor::Add_CatalogXML(QString &str)
{
     QDomDocument doc_temp;
     QDomElement  item_root;
     QDomElement  item;
     QDomElement  result;

     if(doc_temp.setContent(str))
     {
        item_root = doc_temp.documentElement();
        item = item_root.firstChildElement("Catalog");
        result = MakeElement(doc_temp,"Catalog",Current_Catalog);
        if(item.isNull())
        {
            item_root.insertBefore(result, item_root.firstChildElement("amProgramm"));
        }
        else item_root.replaceChild(result,item);

        str = doc_temp.toString();
     }

}

//-----------------------------------------------------------------------------
//--- Clear_editor()
//-----------------------------------------------------------------------------
bool Test_editor::Clear_editor(bool check_Modify)
{
    bool result = true;
    bool NotEqual = false;
    int res;
    QString text_xml, text;
    char *pchar;
    bool modified = false;    

    if(check_Modify)
    {
        // 1. Check
        if(alg_editor)
        {
            string xml = "";
            string name = "";
            alg_editor->Get_Test(&xml,&name);
            text_xml = QString::fromStdString(xml);
        }
        if(Ext_Editor)
        {
            save_Editor(&pchar, &modified);
            //qDebug() << "modified: (clear) " << modified;
            text_xml = QString::fromUtf8(pchar);
        }

        Add_CatalogXML(text_xml);   // add catalog to xml

        OUT_buffer = text_xml;
        if(IN_buffer != OUT_buffer) NotEqual = true;
        if(Ext_Editor) NotEqual = modified;

        //...
        /*QFile file_in("IN.xml");
        if(file_in.open(QIODevice::WriteOnly))
        {
            QTextStream(&file_in) << IN_buffer;
            file_in.close();
        }

        QFile file_out("OUT.xml");
        if(file_out.open(QIODevice::WriteOnly))
        {
            QTextStream(&file_out) << OUT_buffer;
            file_out.close();
        }*/

        //...


        if(NotEqual)
        {
            message.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            if(Ext_Editor) message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            message.setWindowTitle(tr("Attention"));
            //text = tr("You don't save change of test!\r\nWill you want to save this change?");
            text = tr("You don't save change of test! Will you want to save this change?");
            message.setText(text);
            message.setIcon(QMessageBox::Information);
            message.setButtonText(QMessageBox::Yes, tr("Yes"));
            message.setButtonText(QMessageBox::No, tr("No"));
            message.setButtonText(QMessageBox::Cancel, tr("Cancel"));
            res = message.exec();
            switch(res)
            {
            case QMessageBox::Yes:       Save_Test(false);
                                         break;
            case QMessageBox::No:        break;

            case QMessageBox::Cancel:    result = false;
                                         break;
            default: break;
            }
        }

        if(!result) return(false);
    }


    // 2. Clear
    if(alg_editor)
    {
        alg_editor->Destroy_Win();
        //alg_editor->Destroy();
        alg_editor = NULL;
    }
    if(Ext_Editor)
    {
        close_Editor();
        Ext_Editor = false;
    }
    editor_Name->setText("");
    IN_buffer = OUT_buffer = "";
    Current_Catalog = "";
    if(!disable_action) copy_Test->setEnabled(false);
    save_Test->setEnabled(false);

    if(apply_button->isVisible()) apply_button->setEnabled(false);

    return(result);
}
//-----------------------------------------------------------------------------
//--- Load_editor()
//-----------------------------------------------------------------------------
void Test_editor::Load_editor(QString xml, QString name)
{    
    if(alg_editor)
    {
        void *obj = alg_editor->Create_Win(widget_editor, this);

        QWidget *w_obj = reinterpret_cast<QWidget*>(obj);
        if(w_obj->isWidgetType())
        {
            layout_plugin->addWidget(w_obj);
        }        

        if(disable_action) alg_editor->Disable_Action();
        alg_editor->Set_Test(xml.toStdString(), name.toStdString());

        alg_editor->Show();        
    }
}
//-----------------------------------------------------------------------------
//--- Click_edit_test()
//-----------------------------------------------------------------------------
void Test_editor::Click_edit_test()
{
    rt_Test *ptest;
    int p;
    bool ok;
    QTreeWidgetItem *parent_item;
    QTreeWidgetItem *current_item = tests_list->currentItem();
    if(current_item)
    {
        p = current_item->text(2).toInt(&ok);
        if(!ok) p = 0;
        ptest = (rt_Test*)p;

        if(TESTs->contains(ptest) && (current_item->flags() & Qt::ItemIsEnabled))
        {
            edit_Test(current_item);
            parent_item = current_item->parent();
            if(parent_item)
            {
                //tests_list->setCurrentItem(parent_item);
            }
            if(apply_button->isVisible()) apply_button->setEnabled(true);
        }
    }
}

//-----------------------------------------------------------------------------
//--- Check_EditStatus
//-----------------------------------------------------------------------------
void Test_editor::Check_EditStatus(QTreeWidgetItem *current_item, QTreeWidgetItem *prev_item)
{
    rt_Test *ptest;
    bool ok;
    bool status = false;

    int p = current_item->text(2).toInt(&ok);
    if(!ok) p = 0;

    ptest = (rt_Test*)p;


    //if(tests_list->indexOfTopLevelItem(current_item) < 0 &&
    //  (current_item->flags() & Qt::ItemIsEnabled)) status = true;

    if(TESTs->contains(ptest) && (current_item->flags() & Qt::ItemIsEnabled)) status = true;

    edit_TestButton->setEnabled(status);
}

//-----------------------------------------------------------------------------
//--- edit_Test(QTreeWidgetItem *item)
//-----------------------------------------------------------------------------
void Test_editor::edit_Test(QTreeWidgetItem *item)
{
    QTreeWidgetItem *item_parent;
    int method_id;
    int id;
    QString text, name;
    bool ok;    
    rt_Test *ptest;

    qDebug() << "edit_Test(QTreeWidgetItem *item) item: " << item;

    id = item->text(2).toInt(&ok);
    if(!ok) id = 0;
    ptest = (rt_Test*)id;

    if(!(item->flags() & Qt::ItemIsEnabled) || !TESTs->contains(ptest)) return;


    /*name = item->text(0);
    item_parent = item->parent();    
    text = item_parent->text(2);
    text.replace(" ","");
    method_id = text.toInt(&ok, 16);

    foreach(ptest, *TESTs)
    {
        if(ptest->header.Name_Test == name.toStdString()) break;
        else ptest = NULL;
    }*/

    if(!Clear_editor()) return;

    name = item->text(0);
    if(map_TestTranslate->size()) map_TestTranslate->key(name, name);

    if(ptest && (QString::fromStdString(ptest->header.Name_Test) == name)); //item->text(0))
    {
        edit_Test(ptest);
    }
}
//-----------------------------------------------------------------------------
//--- edit_Test(QTreeWidgetItem *item)
//-----------------------------------------------------------------------------
void Test_editor::edit_Test(rt_Test *ptest)
{
    if(!ptest) return;

    QDomDocument doc_temp;
    QDomElement  xml_item;
    QDomElement  xml_signature;
    QString text, xml;
    QVector<QString> user_info;

    QString name = QString::fromStdString(ptest->header.Name_Test);
    int method_id = ptest->header.Type_analysis;

    Current_Catalog = QString::fromStdString(ptest->header.Catalog);

    qDebug() << "Current_Catalog: " << Current_Catalog;

    if(Map_editor.contains(method_id))  // My Editor
    {
        alg_editor = Map_editor.value(method_id);
        if(alg_editor && ptest)
        {
            xml_item = SaveXML_Test(doc_temp, ptest);   // create xml text

            XML_Signature.clear();

            xml_signature = xml_item.firstChildElement("Signature");    // Remove Signature item
            if(!xml_signature.isNull())
            {
                XML_Signature = xml_signature;
                xml_item.removeChild(xml_signature);
            }

            doc_temp.appendChild(xml_item);             // ...
            xml = doc_temp.toString();
            Add_CatalogXML(xml);
            IN_buffer = xml;            

            //...

            /*QFile file_out("IN.xml");
            if(file_out.open(QIODevice::WriteOnly))
            {
                QTextStream(&file_out) << IN_buffer;
                file_out.close();
            }*/

            //...

            Load_editor(xml, name);

            //qDebug() << "Edit test, XML: " << QString::fromStdString(ptest->header.Catalog) << xml;

            text = QString("%1:  %2").arg(Map_editorNAME.value(method_id)).arg(name);
            if(method_id == 0) text = QString("%1:  %2").arg(tr("Preliminary_analysis")).arg(name);

            editor_Name->setText(text);
        }
        if(alg_editor && method_id == 0x0033) alg_editor->Reserve(TESTs);
        else
        {
            if(alg_editor)
            {
                user_info.append(User_Folder);
                alg_editor->Reserve(&user_info);
            }
        }

    }
    if(Map_editorEXT.contains(method_id))   // CpALL Editor
    {
        if(ptest)
        {

            //qDebug() << "Edit_test: " << QString::fromStdString(ptest->header.Name_Test);

            xml_item = SaveXML_Test(doc_temp, ptest);   // create xml text

            XML_Signature.clear();

            xml_signature = xml_item.firstChildElement("Signature");    // Remove Signature item
            if(!xml_signature.isNull())
            {
                XML_Signature = xml_signature;
                xml_item.removeChild(xml_signature);
            }

            doc_temp.appendChild(xml_item);
            doc_temp.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");

            Create_DefaultCommonProperties(doc_temp, xml_item);

            text = doc_temp.toString();

            Add_CatalogXML(text);

            IN_buffer = text;
            Ext_Editor = true;

            //QFile outfile(qApp->applicationDirPath() + "/IN_dtr.xml");
            //if(outfile.open(QIODevice::WriteOnly))
            //{
            //    QTextStream(&outfile) << text; //doc_temp.toString();
            //    outfile.close();
            //}

            //qDebug() << "Edit_test: show_Editor start";
            show_Editor((HWND)widget_editor->winId(), text.toUtf8().data(), From_Protocol);
            //qDebug() << "Edit_test: show_Editor stop";
            resize_Editor((HWND)widget_editor->winId());

            if(map_TestTranslate->size()) name = map_TestTranslate->value(name, name);

            text = QString("%1:  %2").arg(Map_editorEXT.value(method_id)).arg(name);                       
            editor_Name->setText(text);
        }
    }
    if(!disable_action) copy_Test->setEnabled(true);
    save_Test->setEnabled(true);    

}
//-----------------------------------------------------------------------------
//--- Item_Move()
//-----------------------------------------------------------------------------
void Test_editor::Item_Move()
{

}

//-----------------------------------------------------------------------------
//--- Item_changed(QTreeWidgetItem *item, int col)
//-----------------------------------------------------------------------------
void Test_editor::Item_changed(QTreeWidgetItem *item, int col)
{
    tests_list->setEditTriggers(0);
    tests_list->setCurrentItem(item);
}

//-----------------------------------------------------------------------------
//--- AddItem_menu()
//-----------------------------------------------------------------------------
void Test_editor::AddItem_menu()
{
    QTreeWidgetItem *item = tests_list->selectedItems().at(0);

    tests_list->blockSignals(true);

    QTreeWidgetItem *item_child = new QTreeWidgetItem(item);
    item_child->setIcon(0, QIcon(":/images/flat/folder_new_16.png"));
    item_child->setText(0, tr("new item"));
    item_child->parent()->setExpanded(true);
    item_child->setFlags(item_child->flags() | Qt::ItemIsEditable);
    tests_list->editItem(item_child);

    tests_list->blockSignals(false);
}
//-----------------------------------------------------------------------------
//--- DeleteItem_menu()
//-----------------------------------------------------------------------------
void Test_editor::DeleteItem_menu()
{
    int i, id;
    bool ok;
    rt_Test *ptest;
    QString text;
    int index;
    QStringList	cache;
    QString ID_test;

    QTreeWidgetItem *item = tests_list->selectedItems().at(0);
    List_Deleted.clear();

    Load_ListDeleted(item);

    message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    message.setDefaultButton(QMessageBox::No);
    message.setWindowTitle(tr("Delete test"));
    text = tr("Do You want to DELETE all tests ") + QString("(%1)?").arg(List_Deleted.size());
    message.setText(text);
    message.setIcon(QMessageBox::Question);
    int res = message.exec();
    if(res == QMessageBox::No) return;

    qApp->processEvents();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    int w = width();
    int h = height();
    label_gif->move(w/2 - label_gif->width()/2, h/2 - label_gif->height()/2);
    label_gif->setVisible(true);
    obj_gif->start();
    qApp->processEvents();

    label_gif->repaint();


    // DELETE
    foreach(ptest, List_Deleted)
    {
        index = TESTs->indexOf(ptest);
        if(index < 0) continue;

        ID_test = QString::fromStdString(ptest->header.Name_Test);
        delete ptest;
        TESTs->remove(index);

        if(ax_user)
        {
            cache = ax_user->dynamicCall("getCache(QString)", "test").toStringList();
            foreach(text, cache)
            {
                id = text.toInt(&ok);
                QAxObject*  axItemCache = ax_user->querySubObject( "getCacheEntry(int)", id);
                text = axItemCache->property("uinIncome").toString();
                delete axItemCache;
                qApp->processEvents();

                if(text == ID_test)
                {
                    ax_user->dynamicCall("delCacheEntry(int)", id);
                    break;
                }
            }
        }

    }
    List_Deleted.clear();

    label_gif->setVisible(false);
    obj_gif->stop();
    QApplication::restoreOverrideCursor();

    Fill_ListTests();

}
//-----------------------------------------------------------------------------
//--- Load_ListDeleted(QTreeWidgetItem *item)
//-----------------------------------------------------------------------------
void Test_editor::Load_ListDeleted(QTreeWidgetItem *item)
{
    int i;
    int id;
    bool ok;
    QString text;
    rt_Test *p_test;
    QTreeWidgetItem *child_item;

    if(item->childCount() == 0)
    {
        text = item->text(2).trimmed();
        if(!text.isEmpty())
        {
            id = text.toInt(&ok);
            if(ok && id > 0)
            {
                p_test = (rt_Test*)id;
                List_Deleted.append(p_test);
            }
        }
    }
    else
    {
        for(i=0; i<item->childCount(); i++)
        {
            child_item = item->child(i);
            Load_ListDeleted(child_item);
        }
    }
}

//-----------------------------------------------------------------------------
//--- contextMenu_Ctalog(QPoint pos)
//-----------------------------------------------------------------------------
void Test_editor::contextMenu_Catalog(QPoint pos)
{
    QTreeWidgetItem *item = tests_list->itemAt(pos);
    bool has_child = false;

    if(From_Protocol) return;

    has_child = (bool)item->childCount();
    //menu_DeleteItem->setDisabled(has_child);

    if(tests_list->indexOfTopLevelItem(item) >= 0) menu_DeleteItem->setDisabled(true);

    if(!Whats_Type(item) && item->parent()) return;
    if(item->isDisabled()) return;

    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(menu_AddItem);
    menu.addAction(menu_DeleteItem);
    menu.exec(QCursor::pos());

    menu.clear();
}

//-----------------------------------------------------------------------------
//--- contextMenu_ListTests(QPoint pos)
//-----------------------------------------------------------------------------
void Test_editor::contextMenu_ListTests(QPoint pos)
{
    QAction *action_exec;
    item_action = NULL;
    ptest_action = NULL;
    QTreeWidgetItem *item = tests_list->itemAt(pos);

    QFont f = qApp->font();
    f.setItalic(true);

    if(From_Protocol) return;

    //qDebug() << item;
    if(!item) return;
    //if(item->isDisabled()) return;
    //if(!item->parent()) return;

    if(item->text(2).isEmpty())     // menu for item - catalog
    {
        contextMenu_Catalog(pos);
        return;
    }


    item_action = item;

    rt_Test *ptest;
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    QString name = item->text(0);
    QString name_test;

    foreach(ptest, *TESTs)
    {
        name_test = QString::fromStdString(ptest->header.Name_Test);
        if(map_TestTranslate->size()) name_test = map_TestTranslate->value(name_test,name_test);

        if(name_test == name) {ptest_action = ptest; break;}
        else ptest_action = NULL;
    }
    if(!ptest_action) {menu.clear(); return;}
    if(ptest_action->header.Type_analysis == 0 && ptest_action->header.Name_Test == "simple") {menu.clear(); return;}


    if(!item->isDisabled())
    {
        menu.addAction(menu_editTest);
        menu.addAction(menu_copyTest);
    }
    menu.addAction(menu_deleteTest);
    menu.addSeparator();
    menu_Move = menu.addMenu(tr("test move"));
    Create_MenuMove(menu_Move);

    if(ptest_action->header.Type_analysis < 0x1000 ||
       item_action->isDisabled()) menu_Move->setDisabled(true);

    action_exec = menu.exec(QCursor::pos());
    Save_Catalog(action_exec, ptest_action);

    Delete_MenuMove();

    menu.clear();
}
//-----------------------------------------------------------------------------
//--- Save_Catalog(QAction *action, rt_Test *ptest)
//-----------------------------------------------------------------------------
void Test_editor::Save_Catalog(QAction *action, rt_Test *ptest)
{
    QString text;
    int res;
    QStringList list;

    if(action && action->whatsThis().contains("move"))
    {
        if(Check_StateEditor(ptest))
        {
            message.setStandardButtons(QMessageBox::Ok);
            message.setIcon(QMessageBox::Warning);
            text = tr("You must clear the editor field!");
            message.setText(text);
            message.exec();
            return;
        }

        message.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        text = tr("Do You want to move selected test to the catalog: ") + action->text() + " ?";
        message.setText(text);
        message.setIcon(QMessageBox::Question);
        res = message.exec();
        if(res == QMessageBox::Yes)
        {
            list = action->toolTip().split("\\");
            list.removeFirst();
            text = list.join("\\");

            ptest->header.Catalog = text.toStdString();
            qDebug() << "ptest->header.Catalog: " << QString::fromStdString(ptest->header.Name_Test) << QString::fromStdString(ptest->header.Catalog);

            Save_PropertyCatalog(ptest);
            Save_DBaseTests(ptest, false);
            Fill_ListTests(QString::fromStdString(ptest->header.Name_Test));
        }
    }
}
//-----------------------------------------------------------------------------
//--- Check_StateEditor(rt_Test *ptest)
//-----------------------------------------------------------------------------
bool Test_editor::Check_StateEditor(rt_Test *ptest)
{
    QString text = editor_Name->text();
    QStringList list;

    list = text.split(":  ");
    if(list.size() >= 2)
    {
        text = QString(list.at(1)).trimmed();
        if(ptest->header.Name_Test == text.toStdString()) return(true);
    }

    return(false);
}

//-----------------------------------------------------------------------------
//--- Save_PropertyCatalog(rt_Test *ptest)
//-----------------------------------------------------------------------------
void Test_editor::Save_PropertyCatalog(rt_Test *ptest)
{
    QDomDocument doc_temp;
    QDomElement  item_root;
    QDomElement  item;
    QDomElement  result;

    QString catalog = QString::fromStdString(ptest->header.Catalog);


    item_root = SaveXML_Test(doc_temp, ptest);   // create xml text

    item = item_root.firstChildElement("Catalog");
    result = MakeElement(doc_temp,"Catalog",catalog);

    if(item.isNull())
    {
        item_root.insertAfter(result, item_root.firstChildElement("amProgramm"));
    }
    else item_root.replaceChild(result,item);

    LoadXML_Test(item_root, ptest);             // replace ptest
}

//-----------------------------------------------------------------------------
//--- Create_MenuMove(QMenu *menu)
//-----------------------------------------------------------------------------
void Test_editor::Create_MenuMove(QMenu *menu)
{
    int i;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_root = tests_list->topLevelItem(0);
    QFont f = qApp->font();
    QMenu *menu_rec;

    menu->setFont(f);
    menu_rec = menu;

    qDeleteAll(List_ActionsMove);
    List_ActionsMove.clear();

    for(i=0; i<item_root->childCount(); i++)
    {
        item = item_root->child(i);
        create_Actions(item, menu_rec);
    }

}
//-----------------------------------------------------------------------------
//--- create_Actions(QTreeWidgetItem *item)
//-----------------------------------------------------------------------------
void Test_editor::create_Actions(QTreeWidgetItem *item, QMenu *menu)
{
    int i;
    QTreeWidgetItem *item_child;
    QList<QTreeWidgetItem*> list_Item;
    bool create_menu = false;
    QMenu *menu_add;
    QFont f = qApp->font();

    menu->setFont(f);

    for(i=0; i<item->childCount(); i++)
    {
        item_child = item->child(i);
        list_Item.append(item_child);
        if(item_child->text(2).isEmpty())
        {
            create_menu = true;
        }
    }

    if(create_menu)
    {
        menu_add = menu->addMenu(item->text(0));

        QAction *action = new QAction(item->text(0), this);
        if(!Whats_Type(item))
        {
            action->setDisabled(true);
            menu_add->setDisabled(true);
        }
        action->setWhatsThis("move");
        action->setToolTip(Catalog_ForItem(item));
        List_ActionsMove.append(action);
        menu_add->addAction(action);
        QAction *separator = new QAction("-----", this);
        separator->setDisabled(true);
        List_ActionsMove.append(separator);
        menu_add->addAction(separator);

        foreach(item_child, list_Item)
        {
            create_Actions(item_child, menu_add);
        }
    }
    else
    {
        QAction *action = new QAction(item->text(0), this);
        if(!Whats_Type(item)) action->setDisabled(true);
        if(item->isDisabled()) action->setDisabled(true);
        action->setWhatsThis("move");
        action->setToolTip(Catalog_ForItem(item));
        List_ActionsMove.append(action);
        menu->addAction(action);
    }
}
//-----------------------------------------------------------------------------
//--- Whats_Type(QTreeWidgetItem *item)
//-----------------------------------------------------------------------------
bool Test_editor::Whats_Type(QTreeWidgetItem *item)
{
    bool res = true;
    int id;
    bool ok;
    rt_Test *ptest;

    while(item->childCount())
    {
        item = item->child(0);
        if(!item->text(2).isEmpty())
        {
            id = item->text(2).toInt(&ok);
            if(!ok) id = 0;
            if(id > 0)
            {
                ptest = (rt_Test*)id;
                if(ptest->header.Type_analysis < 0x1000) res = false;
            }
            break;
        }
    }

    return(res);
}

//-----------------------------------------------------------------------------
//--- Catalog_editor()
//-----------------------------------------------------------------------------
void Test_editor::Catalog_editor()
{
    QMenu menu;
    QAction *action_exec;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu_Move = menu.addMenu(tr("test move"));
    Create_MenuMove(menu_Move);

    action_exec = menu.exec(QCursor::pos());
    //Save_Catalog(action_exec, ptest_action);
}

//-----------------------------------------------------------------------------
//--- Catalog_ForItem(QTreeWidgetItem *item)
//-----------------------------------------------------------------------------
QString Test_editor::Catalog_ForItem(QTreeWidgetItem *item)
{
    QString text;
    QTreeWidgetItem *item_parent;

    text = item->text(0);

    while(item->parent())
    {
        item_parent = item->parent();
        text = item->parent()->text(0) + "\\" + text;
        item = item_parent;
    }

    return(text);
}

//-----------------------------------------------------------------------------
//--- Delete_MenuMove()
//-----------------------------------------------------------------------------
void Test_editor::Delete_MenuMove()
{
    qDeleteAll(List_ActionsMove);
    List_ActionsMove.clear();
}

//-----------------------------------------------------------------------------
//--- DeleteTest_menu()
//-----------------------------------------------------------------------------
void Test_editor::DeleteTest_menu()
{
    int id;
    QString text;
    bool ok;
    QStringList	cache;
    int res;

    if(!ptest_action) return;

    int index = TESTs->indexOf(ptest_action);
    if(index == -1) return;

    QString ID_test = QString::fromStdString(ptest_action->header.Name_Test); // ID_Test);

    message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    message.setDefaultButton(QMessageBox::No);
    message.setWindowTitle(tr("Delete test"));
    text = tr("Do You want to DELETE the test: ") + QString::fromStdString(ptest_action->header.Name_Test) + "?";
    message.setText(text);
    message.setIcon(QMessageBox::Question);
    res = message.exec();
    if(res == QMessageBox::No) return;

    // DELETE...
    delete ptest_action;
    TESTs->remove(index);

    Fill_ListTests();

    if(ax_user)
    {
        cache = ax_user->dynamicCall("getCache(QString)", "test").toStringList();
        foreach(text, cache)
        {
            id = text.toInt(&ok);
            QAxObject*  axItemCache = ax_user->querySubObject( "getCacheEntry(int)", id);
            text = axItemCache->property("uinIncome").toString();

            delete axItemCache;

            if(text == ID_test)
            {
               ax_user->dynamicCall("delCacheEntry(int)", id);
               break;
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- CopyTest_menu()
//-----------------------------------------------------------------------------
void Test_editor::CopyTest_menu()
{

    if(!ptest_action || !item_action) return;

    int index = TESTs->indexOf(ptest_action);
    if(index == -1) return;

    QString name_test = QString::fromStdString(ptest_action->header.Name_Test);
    if(map_TestTranslate->size())  name_test = map_TestTranslate->value(name_test,name_test);

    if(item_action->text(0) != name_test) return;

    // COPY...
    edit_Test(item_action);
    Copy_Test();

}
//-----------------------------------------------------------------------------
//--- EditTest_menu()
//-----------------------------------------------------------------------------
void Test_editor::EditTest_menu()
{

    if(!ptest_action || !item_action) return;

    int index = TESTs->indexOf(ptest_action);
    if(index == -1) return;

    QString name_test = QString::fromStdString(ptest_action->header.Name_Test);
    if(map_TestTranslate->size())  name_test = map_TestTranslate->value(name_test,name_test);

    if(item_action->text(0) != name_test) return;

    // EDIT...
    edit_Test(item_action);
}
//-------------------------------------------------------------------------------
//--- resizeEvent QGroupBox_ext
//-------------------------------------------------------------------------------
void groupBox::resizeEvent(QResizeEvent *e)
{
    emit Resize();
}
//-------------------------------------------------------------------------------
//--- showEvent QGroupBox_ext
//-------------------------------------------------------------------------------
void groupBox::showEvent(QShowEvent *e)
{
    emit Resize();
}
//-------------------------------------------------------------------------------
//--- resize_extReport
//-------------------------------------------------------------------------------
void Test_editor::resize_extTests()
{
    if(Ext_Editor)
    {
        resize_Editor((HWND)widget_editor->winId());
    }
}
//-------------------------------------------------------------------------------
//--- resize_extReport
//-------------------------------------------------------------------------------
void Test_editor::Create_DefaultCommonProperties(QDomDocument &doc, QDomElement &root)
{
    int i,j;
    QString name, text;
    QStringList list;

    QDomElement item_property;

    QDomNode item;
    bool find;

    list << CRITERION_POSRES << CRITERION_VALIDITY << USE_AFF << CRITERION_AFF << CRITERION_AFF_MIN << CRITERION_AFF_MAX << SIGMOIDVAL_MIN << \
            SIGMOIDVAL_MAX << USE_THRESHOLD << VALUE_THRESHOLD << EXPOSURE << FLUOROFORS << MC_FWHM_BORDER << MC_PEAKS_BORDER;

    item_property = root.firstChildElement("properties");
    if(!item_property.isNull())
    {
        for(i=0; i<list.size(); i++)
        {
            name = list.at(i);
            find = false;
            for(j=0; j<item_property.childNodes().size(); j++)
            {
                item = item_property.childNodes().at(j);
                if(item.firstChildElement("name").text() == name)
                {
                    find = true;
                    break;
                }
            }

            if(!find)
            {
                QDomElement child = MakeElement(doc,"item","");

                switch(i)
                {
                case 0:     text = "80";   break;          // CRITERION_POSRES
                case 1:     text = "5";    break;          // CRITERION_VALIDITY
                case 2:     text = "True"; break;          // USE_AFF
                case 3:     text = "1000"; break;          // CRITERION_AFF
                case 4:     text = "10";   break;          // CRITERION_AFF_MIN
                case 5:     text = "90";   break;          // CRITERION_AFF_MAX
                case 6:     text = "7";    break;          // SIGMOIDVAL_MIN
                case 7:     text = "20";   break;          // SIGMOIDVAL_MAX
                case 8:     text = "False";   break;       // USE_THRESHOLD
                case 9:     text = "10";   break;          // VALUE_THRESHOLD
                case 10:    text = "1 1 1 1 1";   break;   // EXPOSURE
                case 11:    text = "Fam Hex Rox Cy5 Cy5.5"; break;// FLUOROFORS
                case 12:    text = "30";    break;         // MC_FWHM_BORDER
                case 13:    text = "20";   break;          // MC_PEAKS_BORDER

                default:    text = "";  break;
                }

                child.appendChild(MakeElement(doc,"name", name));
                child.appendChild(MakeElement(doc,"value", text));
                child.appendChild(MakeElement(doc,"unit", ""));
                item_property.appendChild(child);
            }
        }
    }
}
//-------------------------------------------------------------------------------
//---
//-------------------------------------------------------------------------------
void MyTreeWidget::dropEvent(QDropEvent *event)
{
    QString text;
    QStringList list;
    int res;
    int id;
    bool ok;
    QTreeWidgetItem *item;
    rt_Test *ptest;

    item = this->selectedItems().at(0);
    id = item->text(2).toInt(&ok);
    if(!ok) id = 0;
    ptest = (rt_Test*)id;

    QModelIndex index = indexAt(event->pos());
    if(!index.isValid())
    {
        event->setDropAction(Qt::IgnoreAction);
        return;
    }
    item = itemFromIndex(index);
    if(!item->text(2).isEmpty()) item = item->parent();

    if(widget_parent->Check_StateEditor(ptest))
    {
        message->setStandardButtons(QMessageBox::Ok);
        message->setIcon(QMessageBox::Warning);
        text = tr("You must clear the editor field!");
        message->setText(text);
        message->exec();
        event->setDropAction(Qt::IgnoreAction);
        return;
    }


    message->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    text = tr("Do You want to move selected test to the catalog: ") + item->text(0) + " ?";
    message->setText(text);
    message->setIcon(QMessageBox::Question);
    res = message->exec();

    if(res == QMessageBox::Yes)
    {
        QTreeWidget::dropEvent(event);

        text = item->text(0);
        while(item->parent())
        {
            text = QString("%1\\%2").arg(item->parent()->text(0)).arg(text);
            item = item->parent();
        }
        list = text.split("\\");
        list.removeFirst();
        text = list.join("\\");

        ptest->header.Catalog = text.toStdString();

        widget_parent->Save_PropertyCatalog(ptest);
        widget_parent->Save_DBaseTests(ptest, false);
        widget_parent->Fill_ListTests(QString::fromStdString(ptest->header.Name_Test));
    }
    else
    {
        event->setDropAction(Qt::IgnoreAction);
        return;
    }
}
//-------------------------------------------------------------------------------
//--- Translate_Catalog()
//-------------------------------------------------------------------------------
/*void Test_editor::Translate_Catalog()
{
    if(Lang_editor == "ru") return;

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

    foreach(ptest, *TESTs)
    {
        //qDebug() << "test: " << QString::fromStdString(ptest->header.Name_Test) << QString::fromStdString(ptest->header.Catalog);

        if(map_Catalog.keys().contains(QString::fromStdString(ptest->header.Catalog)))
        {
            ptest->header.Catalog = map_Catalog.value(QString::fromStdString(ptest->header.Catalog)).toStdString();
        }
    }
}*/
