#include "copy_blocktests.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Copy_BlockTests::Copy_BlockTests(QWidget *parent): QDialog(parent)
{
    readCommonSettings();
    setFont(qApp->font());

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Copy a group of tests"));
    resize(800,400);

    QVBoxLayout *main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setMargin(2);
    main_layout->setSpacing(0);

    box_tests = new QGroupBox(this);
    box_tests->setObjectName("Transparent");
    box_control = new QGroupBox(this);
    box_control->setObjectName("Transparent");
    //box_control->setFixedHeight(40);
    main_layout->addWidget(box_tests,1);
    main_layout->addWidget(box_control);

    QHBoxLayout *layout_control = new QHBoxLayout();
    layout_control->setMargin(1);
    box_control->setLayout(layout_control);
    copy_button = new QPushButton(tr("Copy"), this);
    copy_button->setEnabled(false);
    delete_button = new QPushButton(tr("Delete"), this);
    delete_button->setEnabled(false);
    delete_button->setVisible(false);
    close_button = new QPushButton(tr("Close"), this);
    connect(close_button, SIGNAL(clicked(bool)), this, SLOT(close()));
    layout_control->addWidget(copy_button, 0, Qt::AlignLeft);
    layout_control->addWidget(delete_button, 1, Qt::AlignLeft);
    layout_control->addWidget(close_button, 0, Qt::AlignRight);

    QHBoxLayout *layout_tests = new QHBoxLayout();
    layout_tests->setMargin(1);
    box_tests->setLayout(layout_tests);
    main_spl = new QSplitter(Qt::Horizontal, this);
    main_spl->setHandleWidth(4);
    main_spl->setChildrenCollapsible(false);
    layout_tests->addWidget(main_spl);
    box_source = new QGroupBox(this);
    box_source->setObjectName("Transparent");
    box_target = new QGroupBox(this);
    box_target->setObjectName("Transparent");
    main_spl->addWidget(box_source);
    main_spl->addWidget(box_target);

    items_source = new Tests_ComboBox();
    items_source->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    items_source->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    items_target = new Tests_ComboBox();
    items_target->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    items_target->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    items_target->setDisabled(true);
    tree_source = new QTreeWidget(this);
    tree_source->setContextMenuPolicy(Qt::CustomContextMenu);
    tree_source->setFont(qApp->font());
    connect(tree_source, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_ListTests(QPoint)));
    tree_target = new QTreeWidget(this);
    tree_target->setFont(qApp->font());
    label_source = new QLabel(" ...", this);
    //label_source->setFrameStyle(QFrame::Panel | QFrame::Plain);
    label_source->setStyleSheet("QLabel {border: 1px solid #aaa; padding: 3px;}");
    label_target = new QLabel(" ...", this);
    //label_target->setFrameStyle(QFrame::Panel | QFrame::Plain);
    label_target->setStyleSheet("QLabel {border: 1px solid #aaa; padding: 3px;}");
    label_target->setMinimumWidth(100);
    QVBoxLayout *layout_source = new QVBoxLayout();
    layout_source->setMargin(0);
    box_source->setLayout(layout_source);
    QVBoxLayout *layout_target = new QVBoxLayout();
    layout_target->setMargin(0);
    box_target->setLayout(layout_target);
    layout_source->addWidget(items_source, 0, Qt::AlignLeft);
    layout_source->addWidget(tree_source, 1);
    layout_source->addWidget(label_source);
    layout_target->addWidget(items_target, 0, Qt::AlignRight);
    layout_target->addWidget(tree_target, 1);
    layout_target->addWidget(label_target);

    FileName_To = "";
    //message.setWindowIcon(QIcon(":/images/DTm.ico"));
    //message.setWindowIcon(QIcon(":/images/RT.ico"));

    label_gif = new QLabel(this);
    label_gif->setFixedSize(48,48);
    label_gif->raise();
    obj_gif = new QMovie(":/images/wait_1.gif");
    label_gif->setMovie(obj_gif);
    label_gif->setVisible(false);

    menu_MoveToCommon = new QAction(tr("move to Common"), this);
    connect(menu_MoveToCommon, SIGNAL(triggered(bool)), this, SLOT(MoveToCommon()));
    menu_MoveToPrivate = new QAction(tr("move to Private"), this);
    connect(menu_MoveToPrivate, SIGNAL(triggered(bool)), this, SLOT(MoveToPrivate()));
    ax_NameTest = "";
    ax_OtherUser = NULL;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Copy_BlockTests::~Copy_BlockTests()
{
    qDeleteAll(TESTs_Source_temporary.begin(),TESTs_Source_temporary.end());
    TESTs_Source_temporary.clear();
    qDeleteAll(TESTs_Target_temporary.begin(),TESTs_Target_temporary.end());
    TESTs_Target_temporary.clear();

    map_AllUsers.clear();

    if(ax_OtherUser) delete ax_OtherUser;

    delete obj_gif;
    delete label_gif;

    delete menu_MoveToCommon;
    delete menu_MoveToPrivate;

    delete items_source;
    delete items_target;
    delete tree_source;
    delete tree_target;
    delete label_source;
    delete label_target;
    delete box_source;
    delete box_target;

    delete copy_button;
    delete delete_button;
    delete close_button;

    delete box_tests;
    delete box_control;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Copy_BlockTests::readCommonSettings()
{
    QString text;
    QString dir_SysTranslate;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        ID_LANG = ID_LANG_RU;
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/Copy_BlockTests_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
            if(text == "en") ID_LANG = ID_LANG_EN;
        }

        // ... System Language ...
        dir_SysTranslate = dir_path + "/tools/translations";
        if(qt_translator.load("qt_" + text + ".qm", dir_SysTranslate))
        {
            qApp->installTranslator(&qt_translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;

    logo = QIcon("logotype.png");
    logonull = QIcon("logotype_null.png");
}
//-----------------------------------------------------------------------------
//--- showEvent
//-----------------------------------------------------------------------------
void Copy_BlockTests::showEvent(QShowEvent *event)
{
    QStringList list;
    QList<QVariant> list_OtherUsers;
    int id;
    QString user_name;
    bool ok;
    if(!ax_user) return;
    QString text = ax_user->property("userName").toString();

    list << text << tr("from xml_file:") << tr("from (*.rt,*.trt) protocol:");
    if(!ax_user->property("isAdmin").toBool()) list.append("COMMON");
    items_source->addItems(list);
    items_source->setCurrentIndex(-1);
    Fill_SourseTests();

    list.clear();
    list << text << tr("to xml_file:");
    if(ax_user->property("isAdmin").toBool() && ax_rita)
    {
        list.append(tr("--- Other users ---"));
        list_OtherUsers = ax_rita->property("usersIndex").toList();
        foreach(QVariant value, list_OtherUsers)
        {
            id = value.toInt(&ok);
            QAxObject*  ax_Other = ax_rita->querySubObject("userObject(int)", id);
            user_name = ax_Other->property("userName").toString();
            if(user_name != text)
            {
                list.append("   " + user_name);
                map_AllUsers.insert(id, user_name);
            }

            delete ax_Other;
        }

    }
    items_target->addItems(list);
    items_target->setCurrentIndex(-1);
    items_target->setItemData(2,0,Qt::UserRole - 1);
    Fill_TargetTests();

    int w = width();
    int h = height();
    label_gif->move(w/2 - label_gif->width()/2, h/2 - label_gif->height()/2);

    connect(items_source, SIGNAL(currentIndexChanged(int)), this, SLOT(Fill_SourseTests()));
    connect(items_target, SIGNAL(currentIndexChanged(int)), this, SLOT(Fill_TargetTests()));
    //connect(items_source, SIGNAL(signalPopupHidden()), this, SLOT(Fill_SourseTests()));
    //connect(items_target, SIGNAL(signalPopupHidden()), this, SLOT(Fill_TargetTests()));
    connect(tree_source, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(Change_CheckedSource()));
    connect(tree_target, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(Change_CheckedSource()));
    connect(copy_button, SIGNAL(clicked(bool)), this, SLOT(Copy_Tests()));
}
//-------------------------------------------------------------------------------
//--- resizeEvent
//-------------------------------------------------------------------------------
void Copy_BlockTests::resizeEvent(QResizeEvent *e)
{
    int w = width();
    int h = height();
    label_gif->move(w/2 - label_gif->width()/2, h/2 - label_gif->height()/2);
}

//-----------------------------------------------------------------------------
//--- Change_CheckedSource()
//-----------------------------------------------------------------------------
void Copy_BlockTests::Change_CheckedSource()
{
    int i,j;
    QString text;

    bool enable = false;
    QTreeWidgetItem *item_main = tree_source->topLevelItem(0);
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    int count_test = item_main->childCount();
    int count = 0;
    int count_test_real = 0;

    // 1.
    for(i=0; i<count_test; i++)
    {
        item = item_main->child(i);
        if(item->text(0) == "Common:")
        {
            for(j=0; j<item->childCount(); j++)
            {
                item_child = item->child(j);
                if(item_child->checkState(0) == Qt::Checked) {enable = true; count++;}
                count_test_real++;
            }
            continue;
        }
        count_test_real++;
        if(item->checkState(0) == Qt::Checked) {enable = true; count++;}
    }    

    // 2.
    if(items_source->currentIndex() == 0 && items_target->currentIndex() == 0) enable = false;
    if(items_target->currentIndex() < 0) enable = false;

    copy_button->setEnabled(enable);

    text = QString("%1/%2 tests").arg(count).arg(count_test_real);
    label_source->setText(text);
}
//-----------------------------------------------------------------------------
//--- MoveToCommon()
//-----------------------------------------------------------------------------
void Copy_BlockTests::MoveToCommon()
{
    bool ok;
    if(!ax_user) return;

    int id_user = ax_user->property("userId").toInt(&ok);
    QAxObject* axItemCache = ax_user->querySubObject("getCacheEntryIndex(QString,QString,int)",ax_NameTest,"test",id_user);
    axItemCache->dynamicCall("setPermitionsAll(int)", 1);
    delete axItemCache;

    Fill_SourseTests();
}
//-----------------------------------------------------------------------------
//--- MoveToPrivate()
//-----------------------------------------------------------------------------
void Copy_BlockTests::MoveToPrivate()
{
    bool ok;
    if(!ax_user) return;

    int id_user = ax_user->property("userId").toInt(&ok);
    QAxObject* axItemCache = ax_user->querySubObject("getCacheEntryIndex(QString,QString)",ax_NameTest,"test",id_user);
    axItemCache->dynamicCall("setPermitionsAll(int)", 0);
    delete axItemCache;

    Fill_SourseTests();
}
//-----------------------------------------------------------------------------
//--- contextMenu_ListTests(QPoint pos)
//-----------------------------------------------------------------------------
void Copy_BlockTests::contextMenu_ListTests(QPoint pos)
{
    item_action = NULL;
    ptest_action = NULL;
    ax_NameTest = "";
    QTreeWidgetItem *item = tree_source->itemAt(pos);

    if(!ax_user || !ax_user->property("isAdmin").toBool()) return;
    if(items_source->currentIndex() != 0) return;
    if(!item) return;
    if(item->isDisabled() || item->text(0) == "Common:") return;
    if(!item->parent()) return;

    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    item_action = item;

    if(item_action->parent()->text(0) != "Common:") menu.addAction(menu_MoveToCommon);
    if(item_action->parent()->text(0) == "Common:") menu.addAction(menu_MoveToPrivate);
    ax_NameTest = item_action->text(0);

    menu.exec(QCursor::pos());

    menu.clear();
}
//-----------------------------------------------------------------------------
//--- Fill_SourseTests
//-----------------------------------------------------------------------------
void Copy_BlockTests::Fill_SourseTests()
{
    int i;
    int count_test;
    QString text, name_test;
    int id_name;
    int id_user;
    bool ok;
    bool find;
    QTreeWidgetItem *item_main;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_common;
    QMap<int, QVector<rt_Test*>*> map_CommonTests;
    QVector<rt_Test*> *vec_CommonTests;
    rt_Test *ptest;
    QStringList list_header;
    QMap<QString, rt_Test*> map_temporary;


    bool IsAdmin = false;
    if(ax_user)  IsAdmin = ax_user->property("isAdmin").toBool();

    switch(items_source->currentIndex())
    {
    case 0:     source_Tests = TESTs;   break;                  // user
    case 1:     source_Tests = Open_fromFile();   break;        // from file:
    case 2:     source_Tests = Open_fromProtocol();   break;    // from protocol:
    case 3:     source_Tests = Open_fromCommon();   break;      // COMMON
    default:    source_Tests = NULL;   break;
    }

    //--- update map_TestsTranslate --
    if(items_source->currentIndex() > 0 && source_Tests && ID_LANG != ID_LANG_RU)    // if another source...
    {
        Update_MapTranslate(source_Tests);
    }
    //---

    tree_source->blockSignals(true);

    label_source->setText("");
    tree_source->clear();
    tree_source->setColumnCount(2);
    list_header << tr("Source tests:") << " ";
    tree_source->setHeaderLabels(list_header);
    tree_source->header()->setStretchLastSection(false);
    tree_source->header()->resizeSection(1, 50);
    tree_source->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree_source->header()->setSectionResizeMode(1, QHeaderView::Fixed);


    label_target->setText("");
    tree_target->clear();
    tree_target->setColumnCount(2);
    list_header.clear();
    list_header << tr("Target tests:") << " ";
    tree_target->setHeaderLabels(list_header);
    tree_target->header()->setStretchLastSection(false);
    tree_target->header()->resizeSection(1, 50);
    tree_target->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree_target->header()->setSectionResizeMode(1, QHeaderView::Fixed);


    items_target->blockSignals(true);
    items_target->setCurrentIndex(-1);
    items_target->blockSignals(false);
    items_target->setDisabled(true);

    if(source_Tests == NULL || source_Tests->size() <= 0)
    {
        items_source->blockSignals(true);
        items_source->setCurrentIndex(-1);        
        items_source->blockSignals(false);
        tree_source->blockSignals(false);
        return;
    }    

    if(source_Tests)
    {
        label_gif->setVisible(true);
        obj_gif->start();

        count_test = source_Tests->size();

        item_main = new QTreeWidgetItem(tree_source);
        item_main->setFlags(item_main->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
        item_main->setCheckState(0, Qt::Unchecked);
        item_main->setText(0, tr("All tests:"));
        if(items_source->currentIndex() == 3) item_main->setText(0, "Common");

        foreach(ptest, *source_Tests)
        {
            name_test = QString::fromStdString(ptest->header.Name_Test);
            if(map_TestTranslate->size())  name_test = map_TestTranslate->value(name_test, name_test);
            map_temporary.insert(name_test, ptest);
        }

        foreach(ptest, map_temporary.values())
        {
            qApp->processEvents();
            if(ptest->header.Type_analysis == 0)  continue;

            text = QString::fromStdString(ptest->header.Name_Test);

            if((items_source->currentIndex() == 0 || items_source->currentIndex() == 3) && IsAdmin)
            {
                find = false;
                id_user = ax_user->property("userId").toInt(&ok);
                QAxObject* axItemCache = ax_user->querySubObject("getCacheEntryIndex(QString,QString,int)",text,"test",id_user);
                if(axItemCache->property("permitionsAll").toInt(&ok) == 1)
                {
                    id_name = axItemCache->property("idOwner").toInt(&ok);
                    if(!map_CommonTests.contains(id_name))
                    {
                        vec_CommonTests = new QVector<rt_Test*>();
                        map_CommonTests.insert(id_name, vec_CommonTests);
                    }
                    else vec_CommonTests = map_CommonTests.value(id_name);
                    vec_CommonTests->append(ptest);
                    find = true;
                }                
                delete axItemCache;

                if(find) continue;
            }


            item = new QTreeWidgetItem(item_main);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, Qt::Unchecked);
            name_test = QString::fromStdString(ptest->header.Name_Test);
            if(map_TestTranslate->size())  name_test = map_TestTranslate->value(name_test, name_test);
            item->setText(0, name_test);
            item->setText(1, QString::fromStdString(ptest->header.version));            
            if(ptest->header.Hash == "ok") item->setIcon(0, logo);
            else item->setIcon(0, logonull);
        }

        if(map_CommonTests.size())   // COMMON DIR...
        {            
            foreach(id_name, map_CommonTests.keys())
            {
                item_common = new QTreeWidgetItem(item_main);
                item_common->setFlags(item_common->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
                item_common->setCheckState(0, Qt::Unchecked);
                item_common->setText(0, "Common:");
                foreach(ptest, *map_CommonTests.value(id_name))
                {
                    item = new QTreeWidgetItem(item_common);
                    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                    item->setCheckState(0, Qt::Unchecked);
                    name_test = QString::fromStdString(ptest->header.Name_Test);
                    if(map_TestTranslate->size())  name_test = map_TestTranslate->value(name_test, name_test);
                    item->setText(0, name_test);
                    if(ptest->header.Hash == "ok") item->setIcon(0, logo);
                    else item->setIcon(0, logonull);
                }

                item_common->setExpanded(true);
            }
        }

        item_main->setExpanded(true);

        label_gif->setVisible(false);
        obj_gif->stop();
    }

    tree_source->blockSignals(false);


    foreach(vec_CommonTests, map_CommonTests.values())
    {
        delete vec_CommonTests;
    }
    map_CommonTests.clear();
    map_temporary.clear();

    if(items_source->currentIndex() >= 0) items_target->setDisabled(false);

    Change_CheckedSource(); // label_source->setText(...)

}
//-----------------------------------------------------------------------------
//--- Fill_TargetTests
//-----------------------------------------------------------------------------
void Copy_BlockTests::Fill_TargetTests(QVector<rt_Test*> *target)
{
    int i;
    int count_test;
    QString text, name_test;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_common;
    QMap<int, QVector<rt_Test*>*> map_CommonTests;
    QVector<rt_Test*> *vec_CommonTests;
    rt_Test *ptest;
    bool ok;
    bool find;
    int id_name;
    int id_user;
    QStringList list_header;
    QMap<QString, rt_Test*> map_temporary;


    bool IsAdmin = false;
    if(ax_user)  IsAdmin = ax_user->property("isAdmin").toBool();

    //qDebug() << "Target start:" << target;

    label_target->setText("");

    if(target == NULL)
    {
        switch(items_target->currentIndex())
        {
        case 0:     target_Tests = TESTs;   break;                  // user
        case 1:     target_Tests = Open_toFile();  break;           // to file:
        case -1:    target_Tests = NULL;   break;
        default:    target_Tests = Open_toOtherUser();   break;     // to other user
        }
    }
    else target_Tests = target;

    tree_target->blockSignals(true);

    tree_target->clear();
    tree_target->setColumnCount(2);
    list_header << tr("Target tests:") << " ";
    tree_target->setHeaderLabels(list_header);
    tree_target->header()->setStretchLastSection(false);
    tree_target->header()->resizeSection(1, 50);
    tree_target->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    tree_target->header()->setSectionResizeMode(1, QHeaderView::Fixed);

    if(target_Tests == NULL) // || target_Tests->size() <= 0)
    {
        items_target->blockSignals(true);
        items_target->setCurrentIndex(-1);
        items_target->blockSignals(false);
        tree_target->blockSignals(false);
        FileName_To = "";
        return;
    }

    //--- update map_TestsTranslate --
    if(items_target->currentIndex() > 0 && target_Tests && ID_LANG != ID_LANG_RU)    // if another source...
    {
        Update_MapTranslate(target_Tests);
    }
    //---

    //qDebug() << "Target_1:" << *target_Tests;

    if(target_Tests)
    {
        label_gif->setVisible(true);
        obj_gif->start();


        count_test = target_Tests->size();

        foreach(ptest, *target_Tests)
        {
            name_test = QString::fromStdString(ptest->header.Name_Test);
            if(map_TestTranslate->size())  name_test = map_TestTranslate->value(name_test, name_test);
            map_temporary.insert(name_test, ptest);
        }

        foreach(ptest, map_temporary.values())
        {
            qApp->processEvents();

            if(ptest->header.Type_analysis == 0) continue;

            text = QString::fromStdString(ptest->header.Name_Test);

            if(items_target->currentIndex() == 0 && IsAdmin)
            {
                find = false;
                id_user = ax_user->property("userId").toInt(&ok);
                QAxObject* axItemCache = ax_user->querySubObject("getCacheEntryIndex(QString,QString,int)",text,"test",id_user);
                if(axItemCache->property("permitionsAll").toInt(&ok) == 1)
                {
                    id_name = axItemCache->property("idOwner").toInt(&ok);
                    if(!map_CommonTests.contains(id_name))
                    {
                        vec_CommonTests = new QVector<rt_Test*>();
                        map_CommonTests.insert(id_name, vec_CommonTests);
                    }
                    else vec_CommonTests = map_CommonTests.value(id_name);
                    vec_CommonTests->append(ptest);
                    find = true;
                }
                delete axItemCache;

                if(find) continue;
            }

            item = new QTreeWidgetItem(tree_target);

            if(map_TestTranslate->size())  text = map_TestTranslate->value(text, text);
            item->setText(0, text);
            item->setText(1, QString::fromStdString(ptest->header.version));

            if(ptest->header.Hash == "ok") item->setIcon(0, logo);
            else item->setIcon(0, logonull);
        }

        if(map_CommonTests.size())   // COMMON DIR...
        {
            foreach(id_name, map_CommonTests.keys())
            {
                item_common = new QTreeWidgetItem(tree_target);
                item_common->setText(0, "Common:");
                foreach(ptest, *map_CommonTests.value(id_name))
                {
                    item = new QTreeWidgetItem(item_common);
                    name_test = QString::fromStdString(ptest->header.Name_Test);
                    if(map_TestTranslate->size())  name_test = map_TestTranslate->value(name_test, name_test);
                    item->setText(0, name_test);
                    if(ptest->header.Hash == "ok") item->setIcon(0, logo);
                    else item->setIcon(0, logonull);
                }

                item_common->setExpanded(true);
            }
        }


        text = QString("%1 tests").arg(count_test);
        if(target_Tests == &TESTs_Target_temporary)     // Target == file
        {
            text += QString("  (%1)").arg(FileName_To);
        }
        label_target->setText(text);

        label_gif->setVisible(false);
        obj_gif->stop();
    }

    tree_target->blockSignals(false);

    //qDebug() << "Target stop:" << target;

    foreach(vec_CommonTests, map_CommonTests.values())
    {
        delete vec_CommonTests;
    }
    map_CommonTests.clear();
    map_temporary.clear();

    // check: items_target and items_source for enable/disable copy_button
    if(items_target->currentIndex() >= 0) Change_CheckedSource();
}
//-----------------------------------------------------------------------------
//--- Update_MapTranslate(QVector<rt_Test*> *tests)
//-----------------------------------------------------------------------------
void Copy_BlockTests::Update_MapTranslate(QVector<rt_Test*> *tests)
{
    if(!tests)  return;
    if(ID_LANG == ID_LANG_RU) return;


    QList<QString>  List_Tr;
    QStringList     list;
    QString         text, str;
    rt_Test         *ptest;
    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;

    char *p;
    QByteArray ba;
    QStringList list_in, list_out;
    int id;
    QString key, value;

    HINSTANCE ext_dll_handle;

    List_Tr = map_TestTranslate->keys();

    foreach(ptest, *tests)      // only name tests and research
    {
        text = QString::fromStdString(ptest->header.Name_Test);
        if(text.contains(QRegularExpression("[А-Яа-я]+")) && !List_Tr.contains(text) && !list.contains(text))
        {
            list.append(text);
        }

        foreach(tube_test, ptest->tubes)
        {
            foreach(channel_test, tube_test->channels)
            {
                text = QString::fromStdString(channel_test->name);
                if(text.contains(QRegularExpression("[А-Яа-я]+")) && !List_Tr.contains(text) && !list.contains(text))
                {
                    list.append(text);
                }
            }
        }
    }

    if(list.size())         // update
    {
        str = list.join("\r\n");

        ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");
        if(ext_dll_handle)
        {
            translate_service = (Translate_Service)(::GetProcAddress(ext_dll_handle,"TranslateStr"));
            if(translate_service)
            {
                list_in = str.split("\r\n");
                translate_service(ID_LANG, str.toUtf8().data(), &p);
                ba.clear();
                ba.append(p);
                text = QString::fromLocal8Bit(ba);                      // ok
                list_out = text.split("\r\n");

                id = 0;
                foreach(text, list_in)
                {
                    key = text;
                    if(id >= list_out.size()) break;
                    value= list_out.at(id);
                    map_TestTranslate->insert(key, value);

                    id++;
                }
            }
            ::FreeLibrary(ext_dll_handle);
        }

    }

}
//-----------------------------------------------------------------------------
//--- Open_toOtherUser()
//-----------------------------------------------------------------------------
QVector<rt_Test*>* Copy_BlockTests::Open_toOtherUser()
{
    QString text;
    int id_test;
    int id_owner;
    bool ok;

    QDomDocument    doc;
    QDomElement     root;
    rt_Test         *p_test;

    qDeleteAll(TESTs_Target_temporary.begin(),TESTs_Target_temporary.end());
    TESTs_Target_temporary.clear();

    QString name_user = items_target->currentText().trimmed();
    int id = map_AllUsers.key(name_user);

    if(ax_OtherUser) {delete ax_OtherUser; ax_OtherUser = NULL;}

    ax_OtherUser = ax_rita->querySubObject("userObject(int)", id);
    QStringList list_id = ax_OtherUser->dynamicCall("getCache(QString)", "test").toStringList();

    foreach(text, list_id)
    {
        id_test = text.toInt(&ok);
        QAxObject* axItemCache = ax_OtherUser->querySubObject("getCacheEntry(int)", id_test);
        id_owner = axItemCache->property("idOwner").toInt(&ok);
        if(id == id_owner)
        {
            text = axItemCache->property("dataEntry").toString();
            doc.clear();
            doc.setContent(text);
            root = doc.documentElement();
            QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
            doc.insertBefore(xmlNode, doc.firstChild());

            p_test = new rt_Test();
            LoadXML_Test(root, p_test);
            Validate_Test(p_test);
            TESTs_Target_temporary.push_back(p_test);
        }

        delete axItemCache;        
    }

    //delete ax_OtherUser;


    return(&TESTs_Target_temporary);
}

//-----------------------------------------------------------------------------
//--- Open_toFile()
//-----------------------------------------------------------------------------
QVector<rt_Test*>* Copy_BlockTests::Open_toFile()
{
    int i;
    QString fileName = "";
    QString dirName = qApp->applicationDirPath();
    QString selectedFilter;

    QDomDocument    doc;
    QDomElement     root;
    QDomNode        child;
    rt_Test         *p_test;

    FileName_To = "";

    //... ProgramData ...
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    foreach(QString str, locations)
    {
        if(str.contains("ProgramData"))
        {
            QDir dir;
            QString path = QString("%1/INI").arg(str);
            dir.setPath(path);
            if(!dir.exists()) dir.mkpath(dir.absolutePath());
            if(dir.exists()) dirName = dir.absolutePath();
            break;
        }
    }


    //...

    fileName = QFileDialog::getSaveFileName(this,
                                            tr("Get File with tests..."),
                                            dirName,
                                            tr("Tests File (*.xml)"),
                                            Q_NULLPTR,
                                            QFileDialog::DontConfirmOverwrite);

    qDeleteAll(TESTs_Target_temporary.begin(),TESTs_Target_temporary.end());
    TESTs_Target_temporary.clear();    

    if(fileName.isEmpty()) return(NULL);    

    QFile file(fileName);
    FileName_To = fileName;   

    if(file.exists())
    {
        if(file.open(QIODevice::ReadOnly))
        {
            if(file.size() == 0) {file.close(); return(&TESTs_Target_temporary);}

            if(!doc.setContent(&file))
            {
                file.close();
                QMessageBox::warning(this, tr("Attention!"), tr("Invalid format file!"));
                return(NULL);
            }
            file.close();

            root = doc.documentElement();
            if(root.nodeName() == "TESTs")
            {
                label_gif->setVisible(true);
                obj_gif->start();

                for(i=0; i<root.childNodes().size(); i++)
                {
                    qApp->processEvents();
                    child = root.childNodes().at(i);
                    p_test = new rt_Test();
                    LoadXML_Test(child, p_test);
                    Validate_Test(p_test);
                    TESTs_Target_temporary.push_back(p_test);
                }

                label_gif->setVisible(false);
                obj_gif->stop();
            }
            else
            {
                QMessageBox::warning(this, tr("Attention!"), tr("Invalid format file!"));
                return(NULL);
            }

        }
    }
    else
    {
        if(file.open(QIODevice::WriteOnly))
        {

            file.close();
        }        
    }

    return(&TESTs_Target_temporary);

}
//-----------------------------------------------------------------------------
//--- Open_fromProtocol()
//-----------------------------------------------------------------------------
QVector<rt_Test*>* Copy_BlockTests::Open_fromProtocol()
{
    int i;
    rt_Protocol *prot_temp;
    rt_Test     *p_test, *p_test_copy;
    QString fileName = "";
    QString dirName = user_dir->absolutePath();
    QString selectedFilter;

    fileName = QFileDialog::getOpenFileName(this, tr("Open Protocol..."),
                            dirName,
                            tr("Protocol File (*.rt *.trt)"),
                            &selectedFilter);

    qDeleteAll(TESTs_Source_temporary.begin(),TESTs_Source_temporary.end());
    TESTs_Source_temporary.clear();

    if(fileName.isEmpty()) return(NULL);

    QFile file(fileName);

    if(file.exists())
    {
        prot_temp = Create_Protocol();
        Read_XML(this,NULL,prot_temp,fileName,"",true);

        foreach(p_test, prot_temp->tests)
        {
            p_test_copy = new rt_Test();
            CreateCopy_Test(p_test, p_test_copy);
            Validate_Test(p_test_copy);
            TESTs_Source_temporary.push_back(p_test_copy);
        }

        prot_temp->Clear_Protocol();
        delete prot_temp;
    }

    return(&TESTs_Source_temporary);
}

//-----------------------------------------------------------------------------
//--- Open_fromFile()
//-----------------------------------------------------------------------------
QVector<rt_Test*>* Copy_BlockTests::Open_fromFile()
{
    int i;
    QString fileName = "";
    QString dirName = qApp->applicationDirPath();
    QString selectedFilter;

    QDomDocument    doc;
    QDomElement     root;
    QDomNode        child;
    rt_Test         *p_test;

    //... ProgramData ...
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    foreach(QString str, locations)
    {
        if(str.contains("ProgramData"))
        {
            QDir dir;
            QString path = QString("%1/INI").arg(str);
            dir.setPath(path);
            if(!dir.exists()) dir.mkpath(dir.absolutePath());
            if(dir.exists()) dirName = dir.absolutePath();
            break;
        }
    }


    //...



    fileName = QFileDialog::getOpenFileName(this, tr("Open File with tests..."),
                            dirName,
                            tr("Tests File (*.xml)"),
                            &selectedFilter);

    qDeleteAll(TESTs_Source_temporary.begin(),TESTs_Source_temporary.end());
    TESTs_Source_temporary.clear();

    if(fileName.isEmpty()) return(NULL);

    QFile file(fileName);

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            QMessageBox::warning(this, tr("Attention!"), tr("Invalid format file!"));
            return(NULL);
        }
        file.close();

        root = doc.documentElement();
        if(root.nodeName() == "TESTs")
        {
            label_gif->setVisible(true);
            obj_gif->start();

            for(i=0; i<root.childNodes().size(); i++)
            {
                qApp->processEvents();
                child = root.childNodes().at(i);
                p_test = new rt_Test();
                LoadXML_Test(child, p_test);
                Validate_Test(p_test);
                TESTs_Source_temporary.push_back(p_test);
            }

            label_gif->setVisible(false);
            obj_gif->stop();
        }
        else
        {
            QMessageBox::warning(this, tr("Attention!"), tr("Invalid format file!"));
            return(NULL);
        }

    }


    return(&TESTs_Source_temporary);

}
//-----------------------------------------------------------------------------
//--- Open_fromCommon()
//-----------------------------------------------------------------------------
QVector<rt_Test*>* Copy_BlockTests::Open_fromCommon()
{
    QString text;
    int id;
    int id_owner, id_user;
    bool ok;

    QDomDocument    doc;
    QDomElement     root;
    rt_Test         *p_test;

    qDeleteAll(TESTs_Source_temporary.begin(),TESTs_Source_temporary.end());
    TESTs_Source_temporary.clear();

    if(!ax_user) return(NULL);

    id_user = ax_user->property("userId").toInt(&ok);

    QStringList	list_Tests = ax_user->dynamicCall("getCache(QString)", "test").toStringList();
    foreach(text, list_Tests)
    {
        id = text.toInt(&ok);
        QAxObject* axItemCache = ax_user->querySubObject("getCacheEntry(int)", id);
        id_owner = axItemCache->property("idOwner").toInt(&ok);

        if(axItemCache->property("permitionsAll") == 1 && id_user != id_owner)
        {
            text = axItemCache->property("dataEntry").toString();
            doc.clear();
            doc.setContent(text);
            root = doc.documentElement();
            QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
            doc.insertBefore(xmlNode, doc.firstChild());

            p_test = new rt_Test();
            LoadXML_Test(root, p_test);
            Validate_Test(p_test);
            TESTs_Source_temporary.push_back(p_test);
        }

        delete axItemCache;
    }

    return(&TESTs_Source_temporary);
}

//-----------------------------------------------------------------------------
//--- Copy_Tests()
//-----------------------------------------------------------------------------
void Copy_BlockTests::Copy_Tests()
{
    int i,j,k;
    QString text;
    QTreeWidgetItem *item_main;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    rt_Test *ptest_s;
    rt_Test *ptest_t;
    bool load_to_Target = false;
    bool replace;
    bool sts;
    bool ok;
    int ret;
    int id_user;
    int count=0;
    QMap<rt_Test*,bool> DBase_Tests;
    QAxObject* axItemCache;
    rt_Preference   *preference_test;
    QString xml = "";
    QString ID_test;
    QString name_Test;
    QString name_Source, name_Target;
    QStringList list_Tests;

    QDomDocument doc;

    if(!source_Tests || !target_Tests || tree_source->topLevelItemCount() == 0) {copy_button->setEnabled(false); return;}

    label_gif->setVisible(true);
    obj_gif->start();
    label_gif->repaint();
    repaint();
    qApp->processEvents();
    Sleep(200);


    // 1. copy to target_Tests
    item_main = tree_source->topLevelItem(0);

    for(i=0; i<item_main->childCount(); i++)            // Create List_TESTS
    {
        item = item_main->child(i);
        if(!item->checkState(0) == Qt::Unchecked)
        {
            name_Test = item->text(0);
            if(name_Test == "Common:")
            {
                for(j=0; j<item->childCount(); j++)
                {
                    item_child = item->child(j);
                    name_Test = item_child->text(0);
                    if(!item_child->checkState(0) == Qt::Unchecked) list_Tests.append(name_Test);
                }
            }
            else list_Tests.append(name_Test);
        }
    }    

    //qDebug() << "List tests:" << list_Tests;

    foreach(name_Test, list_Tests)
    {
        for(j=0; j<source_Tests->size(); j++)
        {
            ptest_s = source_Tests->at(j);
            if(ptest_s->header.Type_analysis == 0) continue;

            name_Source = QString::fromStdString(ptest_s->header.Name_Test);
            if(map_TestTranslate->size()) name_Source = map_TestTranslate->value(name_Source,name_Source);
            if(name_Source == name_Test)
            {
                replace = true;
                sts = false;
                for(k=0; k<target_Tests->size(); k++)
                {
                    ptest_t = target_Tests->at(k);
                    name_Target = QString::fromStdString(ptest_t->header.Name_Test);
                    if(map_TestTranslate->size()) name_Target = map_TestTranslate->value(name_Target,name_Target);
                    if(name_Source == name_Target)
                    {
                        text = tr("Do You want to replace the test: ") + name_Source + "?";
                        message.setText(text);
                        message.setIcon(QMessageBox::Question);
                        message.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
                        message.setDefaultButton(QMessageBox::No);
                        ret = message.exec();
                        if(ret == QMessageBox::No) replace = false;
                        if(ret == QMessageBox::Cancel)
                        {
                            item_main->setCheckState(0, Qt::Unchecked);
                            label_gif->setVisible(false);
                            obj_gif->stop();
                            return;
                        }
                        if(replace)
                        {
                            sts = true;
                            delete ptest_t;
                        }
                        break;
                    }
                }
                if(!replace) break;

                ptest_t = new rt_Test();
                CreateCopy_Test(ptest_s, ptest_t);
                if(replace && sts) {target_Tests->replace(k, ptest_t); DBase_Tests.insert(ptest_t, true);}
                else {target_Tests->append(ptest_t); DBase_Tests.insert(ptest_t, false);}
                load_to_Target = true;
                count++;
            }
            //label_gif->repaint();
            label_gif->update();
            qApp->processEvents();
            Sleep(1);
        }
    }    

    // 2. Load to TARGET
    if(load_to_Target)
    {

        sts = false;

        if(target_Tests == &TESTs_Target_temporary && items_target->currentIndex() >= 2 && ax_OtherUser)     // save to Other User:
        {
            for(i=0; i<DBase_Tests.keys().size(); i++)
            {
                ptest_t = DBase_Tests.keys().at(i);
                replace = DBase_Tests.value(ptest_t, false);

                for(j=0; j<ptest_t->preference_Test.size(); j++)
                {
                    preference_test = ptest_t->preference_Test.at(j);
                    if(preference_test->name == "xml_node")
                    {
                        xml = QString::fromStdString(preference_test->value);
                        break;
                    }
                }
                if(xml.isEmpty()) break;

                ID_test = QString::fromStdString(ptest_t->header.Name_Test);
                id_user = ax_OtherUser->property("userId").toInt(&ok);
                if(replace) axItemCache = ax_OtherUser->querySubObject("getCacheEntryIndex(QString,QString,int)", ID_test, "test",id_user);
                else        axItemCache = ax_OtherUser->querySubObject("addCacheEntry(QString,QString)", ID_test, "test");
                if(axItemCache) axItemCache->setProperty("dataEntry", xml);
                if(axItemCache) axItemCache->dynamicCall("setPermitionsAll(int)", 0);
                delete axItemCache;
                sts = true;

                label_gif->update();
                qApp->processEvents();
                Sleep(1);
            }

            delete ax_OtherUser;
            ax_OtherUser = NULL;
        }

        if(target_Tests == &TESTs_Target_temporary && items_target->currentIndex() == 1)     // save to FILE:
        {
            QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
            doc.insertBefore(xmlNode, doc.firstChild());
            QDomElement  root = doc.createElement("TESTs");
            doc.appendChild(root);
            foreach(ptest_t, *target_Tests)
            {
                root.appendChild(SaveXML_Test(doc, ptest_t));

                label_gif->update();
                qApp->processEvents();
                Sleep(1);
            }
            QFile file(FileName_To);
            if(file.exists() && file.open(QIODevice::WriteOnly))
            {
                QTextStream(&file) << doc.toString();
                file.close();
                sts = true;
            }
        }
        if(target_Tests == TESTs && ax_user)                       // save to User dir (ax_rita)
        {
            for(i=0; i<DBase_Tests.keys().size(); i++)
            {
                ptest_t = DBase_Tests.keys().at(i);
                replace = DBase_Tests.value(ptest_t, false);

                for(j=0; j<ptest_t->preference_Test.size(); j++)
                {
                    preference_test = ptest_t->preference_Test.at(j);
                    if(preference_test->name == "xml_node")
                    {
                        xml = QString::fromStdString(preference_test->value);
                        break;
                    }
                }
                if(xml.isEmpty()) break;

                ID_test = QString::fromStdString(ptest_t->header.Name_Test);
                id_user = ax_user->property("userId").toInt(&ok);
                if(replace) axItemCache = ax_user->querySubObject("getCacheEntryIndex(QString,QString,int)", ID_test, "test",id_user);
                else        axItemCache = ax_user->querySubObject("addCacheEntry(QString,QString)", ID_test, "test");
                if(axItemCache) axItemCache->setProperty("dataEntry", xml);
                if(axItemCache) axItemCache->dynamicCall("setPermitionsAll(int)", 0);
                delete axItemCache;
                sts = true;

                label_gif->update();
                qApp->processEvents();
                Sleep(1);
            }
            //DBase_Tests.clear();
        }

        Sleep(200);
        label_gif->setVisible(false);
        obj_gif->stop();

        if(sts)
        {
            text = QString("%1 ").arg(count) + tr("file(s) successfully copied!");
            message.setText(text);
            message.setIcon(QMessageBox::Information);
            message.setStandardButtons(QMessageBox::Ok);
            message.button(QMessageBox::Ok)->animateClick(5000);
            message.exec();

            Fill_TargetTests(target_Tests);
            item_main->setCheckState(0, Qt::Unchecked);
        }

    }

    label_gif->setVisible(false);
    obj_gif->stop();

}
//-----------------------------------------------------------------------------
//--- CreateCopy_Test
//-----------------------------------------------------------------------------
void Copy_BlockTests::CreateCopy_Test(rt_Test *source_test, rt_Test *target_test)
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

    doc.setContent(text);

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    root = doc.documentElement();
    LoadXML_Test(root, target_test);

    // copy info about Signature
    target_test->header.Hash = source_test->header.Hash;

}
//-----------------------------------------------------------------------------
//--- Validate_Test(rt_Test*)
//-----------------------------------------------------------------------------
bool Copy_BlockTests::Validate_Test(rt_Test *ptest)
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
//--- Tests_ComboBox::showPopup()
//-----------------------------------------------------------------------------
void Tests_ComboBox::showPopup()
{
    blockSignals(true);
    setCurrentIndex(-1);
    blockSignals(false);

    QComboBox::showPopup();
}

