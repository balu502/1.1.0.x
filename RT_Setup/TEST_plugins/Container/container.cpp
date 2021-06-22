#include "container.h"

extern "C" CONTAINERSHARED_EXPORT Test_Interface* __stdcall createTest_plugin()
{
    return(new Container());
}

/*extern "C" CONTAINERSHARED_EXPORT void __stdcall LoadTest_ForContainer(void *p_container, QVector<rt_Test*>*tests)
{
    ((Container*)p_container)->LoadTests(tests);
}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Container::Container()
{
    TabGrid = NULL;

    readCommonSettings();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Container::Create_Win(void *pobj, void *main)
{
    QWidget *parent = (QWidget*)pobj;
    parent_widget = parent;    

    TabGrid = new QTabWidget(parent);

    create_HeaderTest(parent);
    create_Program(parent);
    create_Common(parent);

    main_Spl = new QSplitter(Qt::Horizontal, parent);
    main_Spl->setHandleWidth(4);
    main_Spl->setChildrenCollapsible(true);
    list_Spl = new QSplitter(Qt::Horizontal, parent);
    list_Spl->setHandleWidth(4);
    list_Spl->setChildrenCollapsible(true);

    List_Tests = new QTreeWidget(parent);
    List_Tests->setFont(qApp->font());
    List_Container = new QListWidget(parent);
    List_Container->setFont(qApp->font());
    List_Container->setSelectionMode(QAbstractItemView::SingleSelection);
    List_Container->setDragEnabled(true);
    List_Container->viewport()->setAcceptDrops(true);
    List_Container->setDropIndicatorShown(true);
    List_Container->setDragDropMode(QAbstractItemView::InternalMove);

    Item_Delegate = new ItemListDelegate;
    List_Container->setItemDelegate(Item_Delegate);
    Item_Delegate->map_tests = &map_Tests;

    scheme_Container = new Scheme_ProgramAmpl(QColor(1,1,1,1), parent);

    list_Spl->addWidget(List_Tests);
    list_Spl->addWidget(List_Container);

    main_Spl->addWidget(list_Spl);
    main_Spl->addWidget(scheme_Container);

    int width = parent->width();
    QList<int> spl;
    spl << 2*width/3 << width/3;
    main_Spl->setSizes(spl);
    spl.clear();
    spl << width/3 << width/3;
    list_Spl->setSizes(spl);

    TabGrid->addTab(main_Spl, tr("Container"));
    TabGrid->addTab(header_Test, tr("Header"));
    TabGrid->addTab(program_group, tr("Program"));
    TabGrid->addTab(common_Tree, tr("Common"));


    TabGrid->removeTab(3);
    TabGrid->removeTab(2);
    TabGrid->removeTab(1);

    connect(List_Container, SIGNAL(itemSelectionChanged()), this, SLOT(ReDraw_Program()));
    connect(List_Tests, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(ChangedItem(QTreeWidgetItem*,int)));

    return(TabGrid);
}



//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Container::Destroy()
{

    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Container::Destroy_Win()
{
    if(TabGrid != NULL)
    {
        List_Tests->clear();
        List_Container->clear();
        map_Tests.clear();
        map_TESTs.clear();
        checked_Tests.clear();

        delete List_Tests;
        delete scheme_Container;
        delete List_Container;
        delete list_Spl;
        delete main_Spl;

        delete program_group;

        header_Test->clear();
        delete header_Test;

        common_Tree->clear();
        delete common_Tree;

        delete TabGrid;
        TabGrid = NULL;
    }
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Container::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/container_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;

    logo = QIcon(":/images/logotype.png");
    logonull = QIcon(":/images/logotype_null.png");

}
//-----------------------------------------------------------------------------
//--- Show()
//-----------------------------------------------------------------------------
void Container::Show()
{

    TabGrid->show();    

}
//-----------------------------------------------------------------------------
//--- Disable_Action()
//-----------------------------------------------------------------------------
void Container::Disable_Action()
{


}
//-----------------------------------------------------------------------------
//--- Reserve(void*)
//-----------------------------------------------------------------------------
void Container::Reserve(void *p)
{
    TESTs = (QVector<rt_Test*>*)p;

    //qDebug() << "TESTs:" << TESTs->size();

    Load_listTests();
}
//-----------------------------------------------------------------------------
//--- Load_listTests()
//-----------------------------------------------------------------------------
void Container::Load_listTests()
{
    int i,j;
    int id;
    int type;
    QString text;
    bool sector_BasicTests = true;
    bool find;
    bool checked = true;
    bool use_in_container;
    QString key_str, value_str;

    rt_Test *p_test;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_cur;
    QTreeWidgetItem *item_parent;

    QMultiMap<int, rt_Test*> TESTs_Temporary;
    QStringList list;
    QList<QTreeWidgetItem*> list_item;

    QFont f = qApp->font();
    f.setItalic(true);
    QBrush b(Qt::red);

    QStringList pp;
    pp << tr("List of Tests") << tr("ID");

    List_Tests->blockSignals(true);

    List_Tests->clear();
    List_Tests->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    List_Tests->setColumnCount(2);
    List_Tests->setHeaderLabels(pp);
    List_Tests->setColumnWidth(0, 700);

    QMap<QString, rt_Test*> temp_Map;
    QMultiMap<QString, rt_Test*> TESTs_dtr;
    QString catalog;
    QVector<rt_Test*> List_all;

    for(i=0; i<TESTs->size(); i++)
    {
        p_test = TESTs->at(i);
        type = p_test->header.Type_analysis;
        if(type == 0x0033) continue;        
        if(type < 0x2000) TESTs_Temporary.insert(type, p_test);
        else
        {
            catalog = QString::fromStdString(p_test->header.Catalog);
            TESTs_dtr.insert(catalog, p_test);
        }
    }

    foreach(p_test, TESTs_Temporary.values())
    {
        List_all.append(p_test);
    }
    foreach(text, TESTs_dtr.keys())
    {
        if(list.contains(text)) continue;
        else list.append(text);

        temp_Map.clear();
        QMultiMap<QString, rt_Test*>::iterator it = TESTs_dtr.find(text);
        while(it != TESTs_dtr.end() && it.key() == text)
        {
            p_test = (rt_Test*)it.value();
            temp_Map.insert(QString::fromStdString(p_test->header.Name_Test), p_test);
            ++it;
        }

        foreach(p_test, temp_Map.values())
        {
            List_all.append(p_test);
        }
    }


    for(i=0; i<List_all.size(); i++)
    {
        p_test = List_all.at(i);
        type = p_test->header.Type_analysis;
        text = QString::fromStdString(p_test->header.Catalog).trimmed();

        key_str = QString::fromStdString(p_test->header.ID_Test).trimmed();
        value_str = QString::fromStdString(p_test->header.Name_Test).trimmed();
        map_Tests.insert(key_str, value_str);
        map_TESTs.insert(key_str, p_test);

        use_in_container = false;
        if(checked_Tests.contains(QString::fromStdString(p_test->header.ID_Test))) use_in_container = true;

        if(text.isEmpty()) text = tr("Unknown");

        if(type == 0x0033 || type == 0) continue;

        if(type >= 0x2000 && sector_BasicTests && i)
        {
            sector_BasicTests = false;
            item = new QTreeWidgetItem(List_Tests);
            item->setText(0, "-----");
            item->setDisabled(true);
        }

        list.clear();
        list = text.split("\\");

        list_item.clear();
        for(j=0; j<List_Tests->topLevelItemCount(); j++)
        {
            list_item.append(List_Tests->topLevelItem(j));
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
                if(!id) item_cur = new QTreeWidgetItem(List_Tests);
                else    item_cur = new QTreeWidgetItem(item_parent);
                if(text == tr("Unknown"))
                {
                    item_cur->setFont(0,f);
                    item_cur->setForeground(0,b);
                }
                item_cur->setText(0, text);
                if(checked)
                {
                    item_cur->setCheckState(0, Qt::Unchecked);
                    item_cur->setFlags(item_cur->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
                }

                item_cur->setExpanded(true);
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
        item->setText(0, QString::fromStdString(p_test->header.Name_Test));
        //item->setText(1, QString::number((int)p_test));
        item->setText(1, QString::fromStdString(p_test->header.ID_Test));
        if(checked)
        {
            item->setCheckState(0, Qt::Unchecked);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        }

        if(use_in_container) item->setCheckState(0, Qt::Checked);


        // Icons
        if(p_test->header.Hash == "ok") item->setIcon(0, logo);
        else item->setIcon(0, logonull);
        //...

    }
    TESTs_Temporary.clear();
    List_all.clear();

    List_Tests->blockSignals(false);

    //... Check on valid ptest ...
    for(i=checked_Tests.size()-1; i>=0; i--)
    {
        text = checked_Tests.at(i);
        p_test = map_TESTs.value(text,NULL);
        if(p_test == NULL) checked_Tests.remove(i);
    }

    //... Load List_Container ...
    foreach(text, checked_Tests)
    {
        List_Container->addItem(text);
    }
    if(checked_Tests.size()) List_Container->setCurrentRow(List_Container->count()-1);


}
//-----------------------------------------------------------------------------
//--- ReDraw_Program()
//-----------------------------------------------------------------------------
void Container::ReDraw_Program()
{    
    QString text;
    rt_Test *p_test;    
    QVector<string> program;
    QVector<string> program_pro;
    string str;

    scheme_Container->clear_scheme();
    QListWidgetItem *item = List_Container->currentItem();
    if(!item) return;
    text = item->text();
    p_test = map_TESTs.value(text);

    //qDebug() << "ptest: " << QString::fromStdString(p_test->header.Name_Test);

    program = QVector<string>::fromStdVector(p_test->header.program);

    foreach(str, program)
    {
        //qDebug() << "... " << QString::fromStdString(str);
        text = QString::fromStdString(str).trimmed();
        if(text.isEmpty()) continue;
        program_pro.append(str);
    }

    if(!program_pro.isEmpty() && program_pro.size() > 2 &&
       QString::fromStdString(program_pro.at(0)).contains("XPRG"))
    {
        //qDebug() << "draw: ";
        scheme_Container->draw_Program(&program_pro);
    }


}

//-----------------------------------------------------------------------------
//--- GetInformation
//-----------------------------------------------------------------------------
void Container::GetInformation(QVector<QString> *info)
{
    info->append("0x0033");
    info->append(tr("Container"));
}
//-----------------------------------------------------------------------------
//--- Set_Test
//-----------------------------------------------------------------------------
void Container::Set_Test(std::string xml, std::string name)
{
    int i,j;
    int value;    
    bool ok;

    QString text;
    rt_Protocol *prot_temp;
    QStringList list;
    rt_Test *ptest;

    QDomDocument doc;
    QDomElement root;
    QDomNode    child;
    QDomNode    child_node;    
    QDomElement child_element;

    //qDebug() << "set_Test:" << TESTs;

    // Name of Test, IDTest, Type of Analysis
    header_Test->Load_NewTest(QString::fromStdString(name), GetRandomString(), 0x0033);
    checked_Tests.clear();

    // XML
    if(xml.length() && doc.setContent(QString::fromStdString(xml)))
    {
        root = doc.documentElement();

        // Header
        header_Test->Load_XML(root);

        for(i=0; i<root.childNodes().size(); i++)
        {
            child = root.childNodes().at(i);

            if(child.nodeName() == "activeChannels")
            {
                value = child.toElement().text().toInt(&ok,16);
                active_ch = value;

                continue;
            }
            // Program
            if(child.nodeName() == "amProgramm")
            {
                prot_temp = Create_Protocol();
                text = child.toElement().text();
                list = text.split("\r\n");
                for(j=0; j<list.size(); j++)
                {
                    text = list.at(j);
                    prot_temp->program.push_back(text.toStdString());
                }
                Parsing_ProgramAmplification(prot_temp);
                plot_Program->clear_Program();
                prot_temp->volume = 0;
                plot_Program->draw_Program(prot_temp);
                scheme_Program->draw_Program(prot_temp);
                Level_Program = QVector<string>::fromStdVector(prot_temp->program);
                prot_temp->Clear_Protocol();
                delete prot_temp;
                continue;
            }

            // Properties
            if(child.nodeName() == "properties")
            {
                common_Tree->Load_XML(child);

                //... Checked Tests ...
                for(j=0; j<child.childNodes().size(); j++)
                {
                    child_node = child.childNodes().at(j);
                    child_element = child_node.firstChildElement("name");
                    if(!child_element.isNull() && child_element.text() == "Checked_Tests")
                    {
                        text = child_node.firstChildElement("value").text();
                        list = text.split("\r\n");

                        foreach(text, list)
                        {
                            //qDebug() << "checked test: " << text;
                            checked_Tests.append(text);
                            //ptest = (rt_Test*)(text.toInt(&ok));
                            //if(ok) checked_Tests.append(QString::fromStdString(ptest->header.ID_Test));

                            //List_Container->addItem(text);
                        }

                    }
                    continue;
                }

                //qDebug() << "Checked Tests_IN: " << checked_Tests;
            }
        }
    }
}

//-----------------------------------------------------------------------------
//--- Get_Test(string *xml, string *name)
//-----------------------------------------------------------------------------
void Container::Get_Test(string *xml, string *name)
{    

    QDomDocument doc_temp;
    QDomElement  item;
    QString text;

    rt_Test *ptest = new rt_Test();         // create Test
    fill_Test(ptest);                       // fill Test

    item = SaveXML_Test(doc_temp, ptest);   // create xml text
    doc_temp.appendChild(item);             // ...
    text = doc_temp.toString();             // ...

    *xml = text.toStdString();              // xml Test
    *name = ptest->header.Name_Test;        // name Test

    delete ptest;                           // delete Test


}
//-----------------------------------------------------------------------------
//--- fill_Test(rt_Test *ptest)
//-----------------------------------------------------------------------------
void Container::fill_Test(rt_Test *ptest)
{
    rt_Preference   *property;
    QString text;
    QStringList list;
    rt_Test *p;

    QString fluor_name[COUNT_CH] = FLUOR_NAME;

    if(ptest == NULL) return;

    int i;
    int active_ch = 0;

    // 1. Header
    header_Test->Fill_Properties(ptest);

    // 2. Program
    ptest->header.program = Level_Program.toStdVector();

    // 3. Common
    common_Tree->Fill_Properties(ptest);

    // 4. Templates
    rt_TubeTest *tube = new rt_TubeTest();
    ptest->tubes.push_back(tube);

    active_ch = ptest->header.Active_channel;
    for(i=0; i<COUNT_CH; i++)
    {
        if(((active_ch >> 4*i) & 0x0f) == 0) continue;
        rt_ChannelTest *ch = new rt_ChannelTest();
        tube->channels.push_back(ch);
        ch->ID_Channel = i;
        ch->name = fluor_name[i].toStdString();
        ch->number = i;
    }

    // 5. Checked Tests

    //CheckedTests(checked_Tests);    // clear and load: checked_Tests

    property = new rt_Preference;
    property->name = "Checked_Tests";    
    //foreach(text, checked_Tests)
    //{
        //list.append(text);
    //}
    for(i=0; i<List_Container->count(); i++)
    {
        list.append(List_Container->item(i)->text());
    }


    if(list.size()) text = list.join("\r\n");
    else text = "";
    property->value = text.toStdString();
    ptest->preference_Test.push_back(property);

    //qDebug() << "Checked Tests_OUT: " << checked_Tests;

}
//-----------------------------------------------------------------------------
//--- CheckedTests()
//-----------------------------------------------------------------------------
void Container::CheckedTests(QVector<QString> &list)
{
    QTreeWidgetItemIterator it(List_Tests);
    QString text;
    rt_Test *p_test;
    bool ok;

    list.clear();

    while(*it)
    {
        if((*it)->isDisabled()) {++it; continue;}

        text = (*it)->text(1);
        if((*it)->checkState(0) == Qt::Checked && text.length())
        {
            //p_test = (rt_Test*)(text.toInt(&ok));
            //if(ok) list.append(p_test);
            list.append(text);
        }
        ++it;
    }

}
//-----------------------------------------------------------------------------
//--- create_Common()
//-----------------------------------------------------------------------------
void Container::create_Common(QWidget *parent)
{
    common_Tree = new Common(parent);
    common_Tree->active_channel = active_ch;
}
//-----------------------------------------------------------------------------
//--- create_HeaderTest()
//-----------------------------------------------------------------------------
void Container::create_HeaderTest(QWidget *parent)
{
    header_Test = new Header_Test(parent);
    active_ch = header_Test->active_Channels;
    header_Test->method_analysis = tr(Method);
}
//-----------------------------------------------------------------------------
//--- create_Program()
//-----------------------------------------------------------------------------
void Container::create_Program(QWidget *parent)
{
    Level_Program.clear();

    program_group = new QMainWindow(parent);
    program_tool = program_group->addToolBar("file");
    //program_tool->setIconSize(QSize(16,16));

    open_program = new QAction(QIcon(":/images/open_new.png"), tr("Open program"), parent);
    if(StyleApp == "fusion") open_program->setIcon(QIcon(":/images/flat/open_Program_flat.png"));
    connect(open_program,SIGNAL(triggered()), this, SLOT(Open_Program()));

    edit_program = new QAction(QIcon(":/images/edit_program.png"), tr("Edit program"), parent);
    if(StyleApp == "fusion") edit_program->setIcon(QIcon(":/images/flat/edit_Program_flat.png"));
    connect(edit_program,SIGNAL(triggered()), this, SLOT(Edit_Program()));
    program_tool->addAction(open_program);
    program_tool->addAction(edit_program);

    program_spl = new QSplitter(Qt::Horizontal, parent);
    program_spl->setHandleWidth(4);
    program_spl->setChildrenCollapsible(true);
    program_group->setCentralWidget(program_spl);

    plot_Program = new Plot_ProgramAmpl(parent);
    scheme_Program = new Scheme_ProgramAmpl(QColor(1,1,1,1), parent);

    program_spl->addWidget(plot_Program);
    program_spl->addWidget(scheme_Program);

}

//-----------------------------------------------------------------------------
//--- Open_Program(QString fn)
//-----------------------------------------------------------------------------
void Container::Open_Program(QString fn)
{
    int i;
    QList<QTreeWidgetItem*> list_item;
    rt_Protocol *prot_temp;
    QString text;
    QStringList list;
    QString fileName = "";
    QString selectedFilter;

    if(fn.isEmpty())
    {
        fileName = QFileDialog::getOpenFileName(parent_widget, tr("Open Protocol"),
                                "/home",
                                tr("Programs file (*.rta);;Protocols File (*.rt)"),
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

    Parsing_ProgramAmplification(prot_temp);
    plot_Program->clear_Program();
    prot_temp->volume = 0;
    plot_Program->draw_Program(prot_temp);
    scheme_Program->draw_Program(prot_temp);

    header_Test->Change_NameProgramm(QString::fromStdString(prot_temp->PrAmpl_name));

    Level_Program = QVector<string>::fromStdVector(prot_temp->program);

    prot_temp->Clear_Protocol();
    delete prot_temp;
}
//-----------------------------------------------------------------------------
//--- Edit_Program()
//-----------------------------------------------------------------------------
void Container::Edit_Program()
{
    int i;
    QString text = "";
    bool ok = false;
    std::string s;
    HINSTANCE dll_editor = NULL;
    rt_Protocol *prot_temp;
    QStringList list;
    QList<QTreeWidgetItem*> list_item;

    for(i=0; i<Level_Program.size(); i++)
    {
        if(i) text += "\r\n";
        text += QString::fromStdString(Level_Program.at(i));
    }

#ifdef QT_DEBUG
    dll_editor = ::LoadLibraryW(L"editor_rtd.dll");
#else
    dll_editor = ::LoadLibraryW(L"editor_rt.dll");
#endif

    if(dll_editor)
    {
        Interface_Editor editor_func = reinterpret_cast<Interface_Editor>(
                           ::GetProcAddress(dll_editor,"create_editor@32"));
        if(editor_func)
        {
            editor_func(text.toStdString(), ok, s);
        }

        if(ok)
        {
            prot_temp = Create_Protocol();
            text = QString::fromStdString(s);

            prot_temp->program.clear();
            list = text.split("\r\n");
            for(i=0; i<list.size(); i++)
            {
                text = list.at(i);
                prot_temp->program.push_back(text.toStdString());
            }
            Parsing_ProgramAmplification(prot_temp);
            plot_Program->clear_Program();
            prot_temp->volume = 0;
            plot_Program->draw_Program(prot_temp);
            scheme_Program->draw_Program(prot_temp);

            header_Test->Change_NameProgramm(QString::fromStdString(prot_temp->PrAmpl_name));

            Level_Program = QVector<string>::fromStdVector(prot_temp->program);

            prot_temp->Clear_Protocol();
            delete prot_temp;
        }



        ::FreeLibrary(dll_editor);
    }
}
//-----------------------------------------------------------------------------
//--- ChangedItem()
//-----------------------------------------------------------------------------
void Container::ChangedItem(QTreeWidgetItem *item, int col)
{
    QList<QListWidgetItem*> list;
    int id, row;
    QString text;

    if(!item->text(1).isEmpty() && item->childCount() == 0)
    {
        text = item->text(1);

        if(item->checkState(0) == Qt::Checked)
        {
            List_Container->addItem(text);
            List_Container->setCurrentRow(List_Container->count()-1);
            checked_Tests.append(text);
        }
        else
        {
            if(item->checkState(0) == Qt::Unchecked)
            {
                list = List_Container->findItems(text, Qt::MatchContains);
                if(list.size())
                {
                    row = List_Container->row(list.at(0));
                    List_Container->takeItem(row);
                    delete list.at(0);
                }
                id = checked_Tests.indexOf(text);
                if(id >= 0) checked_Tests.remove(id);
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- SamplesItemDelegate
//-----------------------------------------------------------------------------
void ItemListDelegate::paint(QPainter *painter,
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


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);


    rect = option.rect;
    row = index.row();

    text = index.data().toString();

    //__1. Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(option.rect, QColor(255,255,255));
    }

    text = map_tests->value(text, "");
    text = QString(" %1   %2 ").arg(row+1).arg(text);

    painter->drawText(rect, Qt::AlignLeft, text);
}
