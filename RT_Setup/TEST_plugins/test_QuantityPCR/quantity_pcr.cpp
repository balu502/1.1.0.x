#include "quantity_pcr.h"

extern "C" QUANTITY_PCRSHARED_EXPORT Test_Interface* __stdcall createTest_plugin()
{
    return(new Quantity_PCR());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Quantity_PCR::Quantity_PCR()
{
    TabGrid = NULL;
    readCommonSettings();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Quantity_PCR::Create_Win(void *pobj, void *main)
{
    QWidget *parent = (QWidget*)pobj;
    parent_widget = parent;    

    TabGrid = new QTabWidget(parent);

    create_HeaderTest(parent);
    create_Program(parent);
    create_Common(parent);
    create_Standarts(parent);
    create_Type(parent);
    //create_Standarts1(parent);


    TabGrid->addTab(header_Test, tr("Header"));
    TabGrid->addTab(program_group, tr("Program"));
    TabGrid->addTab(common_Tree, tr("Common"));
    TabGrid->addTab(group_Standarts, tr("Standarts"));
    TabGrid->addTab(Table_Quality, tr("Secific && IC"));

    //TabGrid->addTab(standarts, tr("standarts1"));

    Change_CountStandarts(2);

    connect(header_Test, SIGNAL(Change_ActiveChannels(int)), this, SLOT(Get_ActiveChannels(int)));
    connect(TabGrid, SIGNAL(currentChanged(int)), this, SLOT(Change_TabIndex(int)));

    return(TabGrid);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Quantity_PCR::Change_TabIndex(int index)
{
    QWidget *obj = TabGrid->currentWidget();
    if(obj) obj->setFocus();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Quantity_PCR::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Quantity_PCR::Destroy_Win()
{
    if(TabGrid != NULL)
    {
        delete program_group;

        header_Test->clear();
        delete header_Test;

        common_Tree->clear();
        delete common_Tree;

        //delete standarts;

        delete count_Standarts;
        delete count_Doubles;
        delete Units_Box;
        delete label_count;
        delete label_Doubles;
        delete label_unit;
        delete standarts_Delegate;
        Standarts_Table->clear();
        delete Standarts_Table;
        delete group_Standarts;

        delete TabGrid;
        TabGrid = NULL;
    }
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Quantity_PCR::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/test_quantity_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//--- Show()
//-----------------------------------------------------------------------------
void Quantity_PCR::Show()
{
    QList<int> spl;
    int value, sum;

    TabGrid->show();

    TabGrid->setCurrentIndex(1);    // ???
    TabGrid->setCurrentIndex(0);    // ???

    spl = program_spl->sizes();
    sum = spl.at(0) + spl.at(1);
    value = sum/2;
    spl.replace(0,value);
    spl.replace(1,sum-value);
    program_spl->setSizes(spl);

    //qDebug() << "show: " << active_ch << Table_Quality->rowCount();
    if(Table_Quality->rowCount() == 0) Fill_TableQuality(active_ch);
}
//-----------------------------------------------------------------------------
//--- Reserve(void *p)
//-----------------------------------------------------------------------------
void Quantity_PCR::Reserve(void *p)
{
    QVector<QString> *info = (QVector<QString>*)p;

    User_Folder = info->at(0);
}

//-----------------------------------------------------------------------------
//--- Disable_Action()
//-----------------------------------------------------------------------------
void Quantity_PCR::Disable_Action()
{
    // Header
    header_Test->setDisabled(true);

    // Program
    open_program->setDisabled(true);
    edit_program->setDisabled(true);

    // Common
    QTreeWidgetItem *item = common_Tree->topLevelItem(1);
    item->setDisabled(true);

    // Standart
    group_Standarts->setDisabled(true);
    standarts_Delegate->Standart_isDisable = true;
    //count_Standarts->setDisabled(true);
    //count_Doubles->setDisabled(true);
    //Units_Box->setDisabled(true);

    // Standarts1
    //standarts->setDisabled(true);

}
//-----------------------------------------------------------------------------
//--- GetInformation
//-----------------------------------------------------------------------------
void Quantity_PCR::GetInformation(QVector<QString> *info)
{    
    info->append("0x0001");
    info->append(tr("Base\\Quantity"));
}
//-----------------------------------------------------------------------------
//--- Set_Test
//-----------------------------------------------------------------------------
void Quantity_PCR::Set_Test(std::string xml, std::string name)
{
    int i,j,k,m,n;
    int value;
    int id_ch;
    int row;
    bool ok;
    bool change_ID = false;
    QByteArray b_array;

    QTreeWidgetItem *item;
    QTableWidgetItem *item_value;
    QList<QTreeWidgetItem*> list_item;
    QString text;
    rt_Protocol *prot_temp;
    QStringList list,list_ch;

    int count_St, count_D, type_unit;
    int count_ch = 0;

    QDomDocument doc;
    QDomElement root;
    QDomNode    child;
    QDomNode    channel_child;
    QDomNode    child_node;
    QDomNode    child_property;
    QDomElement child_element;
    QDomElement child_ch;
    QDomElement property_element;

    QMap<int,QString> Specific_IC;
    int key;


    // Name of Test, IDTest, Type of Analysis
    header_Test->Load_NewTest(QString::fromStdString(name), GetRandomString(), 0x0001);

    //qDebug() << "xml: " << QString::fromStdString(xml);

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

                for(j=0; j<COUNT_CH; j++)
                {
                    if(active_ch & (0x0f<<j*4)) count_ch++;
                }                
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
            }

            // Properties about Standarts
            if(child.nodeName() == "properties")
            {
                for(j=0; j<child.childNodes().size(); j++)
                {
                    child_node = child.childNodes().at(j);
                    child_element = child_node.firstChildElement("name");
                    if(child_element.text() == STANDARTS_INFORMATION)
                    {
                        child_element = child_node.firstChildElement("value");
                        list = child_element.text().split("\t");

                        // 1. count standarts, doubles and type unit
                        text = list.at(0);
                        QTextStream(&text) >> count_St;
                        text = list.at(1);
                        QTextStream(&text) >> count_D;
                        text = list.at(2);
                        QTextStream(&text) >> type_unit;

                        count_Standarts->blockSignals(true);
                        count_Standarts->setValue(count_St);
                        count_Standarts->blockSignals(false);
                        Change_CountStandarts(count_St);

                        count_Doubles->setValue(count_D);
                        Units_Box->setCurrentIndex(type_unit);

                        // 2. load standarts value
                        for(k=0; k<count_St; k++)
                        {
                            if((k+3) < list.size()) text = list.at(k+3);
                            else text = "";
                            list_ch = text.split(";");
                            for(m=0; m<count_ch; m++)
                            {
                                item_value = Standarts_Table->item(m + k*count_ch , 4);
                                item_value->setText(list_ch.at(m));
                            }
                        }
                        //qDebug() << "Standart data: " << list;
                        break;
                    }
                }
                continue;
            }

            /*if(child.nodeName() == "properties")
            {
                standarts->Set_ActiveCh(active_ch);
                standarts->Load_XML(child);
            }*/

            if(child.nodeName() == "templates")
            {
                if(child.childNodes().count() == 1)
                {
                    child_node = child.childNodes().at(0);
                    child_element = child_node.firstChildElement("channels");
                    if(!child_element.isNull())
                    {
                        for(j=0; j<child_element.childNodes().size(); j++)
                        {
                            channel_child = child_element.childNodes().at(j);
                            key = channel_child.firstChildElement("IDChannel").text().toInt(&ok);
                            child_property = channel_child.firstChildElement("properties");
                            for(k=0; k<child_property.childNodes().size(); k++)
                            {
                                property_element = child_property.childNodes().at(k).toElement();
                                if(property_element.firstChildElement("name").text() == "specific&ic")
                                {
                                    Specific_IC.insert(key, property_element.firstChildElement("value").text());
                                }
                            }
                        }
                    }
                }
            }
        }

        Fill_TableQuality(active_ch, &Specific_IC);
    }
}
//-----------------------------------------------------------------------------
//--- Get_Test(string *xml, string *name)
//-----------------------------------------------------------------------------
void Quantity_PCR::Get_Test(string *xml, string *name)
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

    //qDebug() << "Get Test:";
}
//-----------------------------------------------------------------------------
//--- fill_Test(rt_Test *ptest)
//-----------------------------------------------------------------------------
void Quantity_PCR::fill_Test(rt_Test *ptest)
{
    QString fluor_name[COUNT_CH] = FLUOR_NAME;

    if(ptest == NULL) return;

    int i,j;
    int id;
    int value;
    bool ok;
    QString text, str;
    QTableWidgetItem *item_header;
    QTreeWidgetItem *item;
    QTableWidgetItem *item_table;
    rt_Preference *property;
    QByteArray b_array;
    int active_ch = 0;
    int count_ch = 0;

    rt_ChannelTest *ch;
    rt_Preference *preference;

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
        count_ch++;
    }

    // 5. Properties about Standarts
    text = QString("%1\t%2\t%3\t").arg(count_Standarts->value())
                                  .arg(count_Doubles->value())
                                  .arg(Units_Box->currentIndex()); // Number standarts, number doubles, type units
    //qDebug() << "Properties about Standarts: " << text;
    for(i=0; i<count_Standarts->value(); i++)
    {
        if(i) text += "\t";
        for(j=0; j<count_ch; j++)
        {
            item_table = Standarts_Table->item(i*count_ch + j, 4);
            str = item_table->text().trimmed();
            if(str.isEmpty()) str = "0";
            if(j) text += ";";
            text += str;
        }         
    }

    property = new rt_Preference();
    ptest->preference_Test.push_back(property);
    property->name = STANDARTS_INFORMATION;
    property->value = text.toStdString();

    // 6. About Specific & IC
    for(i=0; i<Table_Quality->rowCount(); i++)
    {
        item_table = Table_Quality->item(i, 0);
        id = item_table->text().toInt(&ok);
        item_table = Table_Quality->item(i, 2);
        if(!item_table->text().isEmpty())
        {
            foreach(ch, tube->channels)
            {
                if(ch->ID_Channel == id)
                {
                    preference = new rt_Preference();
                    preference->name = "specific&ic";
                    preference->value = item_table->text().toStdString();
                    ch->preference_ChannelTest.push_back(preference);
                    break;
                }
            }
        }
    }
    Table_Quality->horizontalHeader()->setFont(qApp->font());


}
//-----------------------------------------------------------------------------
//--- create_Type()
//-----------------------------------------------------------------------------
void Quantity_PCR::create_Type(QWidget *parent)
{
    QStringList header;

    Table_Quality = new QTableWidget(0, 3, parent);
    //Table_Quality->setFont(qApp->font());
    Table_Quality->setSelectionBehavior(QAbstractItemView::SelectItems);
    Table_Quality->setSelectionMode(QAbstractItemView::SingleSelection);
    Table_Quality->verticalHeader()->setVisible(false);
    Table_Quality->setColumnWidth(0,70);
    Table_Quality->setColumnWidth(1,100);
    Table_Quality->setColumnWidth(2,150);
    Table_Quality->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    Table_Quality->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    Table_Quality->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    header << tr("channels") << tr("Names") << tr("Values");
    Table_Quality->setHorizontalHeaderLabels(header);
    Table_Quality->horizontalHeader()->setFont(qApp->font());
    quality_Delegate = new QualityItemDelegate();
    Table_Quality->setItemDelegate(quality_Delegate);
}
//-----------------------------------------------------------------------------
//--- create_Common()
//-----------------------------------------------------------------------------
void Quantity_PCR::create_Standarts(QWidget *parent)
{
    QStringList header;
    QPalette palette;

    group_Standarts = new QGroupBox(parent);
    QVBoxLayout *layout = new QVBoxLayout();
    group_Standarts->setLayout(layout);
    QHBoxLayout *layout_count = new QHBoxLayout();

    count_Standarts = new QSpinBox();
    count_Standarts->setRange(2,384);
    count_Standarts->setSingleStep(1);
    count_Standarts->setValue(2);

    palette.setColor(QPalette::HighlightedText,Qt::black);
    //palette.setColor(QPalette::Base,QColor(255,255,225));
    palette.setColor(QPalette::Highlight,QColor(180,180,255));
    count_Standarts->setPalette(palette);
    count_Standarts->setFont(QFont("Times New Roman", 14, QFont::Bold));
    label_count = new QLabel(tr("Count of standarts:  "));

    count_Doubles = new QSpinBox();
    count_Doubles->setRange(1,10);
    count_Doubles->setSingleStep(1);
    count_Doubles->setValue(1);
    count_Doubles->setPalette(palette);
    count_Doubles->setFont(QFont("Times New Roman", 12, QFont::Bold));
    label_Doubles = new QLabel(tr("Count of doubles:  "));

    layout_count->addWidget(label_count);
    layout_count->addWidget(count_Standarts);
    layout_count->addStretch(0);
    layout_count->addWidget(label_Doubles);
    layout_count->addWidget(count_Doubles);

    /*QHBoxLayout *layout_Doubles = new QHBoxLayout();
    layout_Doubles->addWidget(label_Doubles);
    layout_Doubles->addWidget(count_Doubles);
    layout_Doubles->addStretch(0);*/


    Standarts_Table = new QTableWidget(0,5);
    Standarts_Table->setSelectionBehavior(QAbstractItemView::SelectItems);
    Standarts_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    Standarts_Table->verticalHeader()->setVisible(false);
    Standarts_Table->setColumnWidth(0,30);
    Standarts_Table->setColumnWidth(1,15);
    Standarts_Table->setColumnWidth(2,80);
    Standarts_Table->setColumnWidth(4,300);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    standarts_Delegate = new StandartsItemDelegate();    
    Standarts_Table->setItemDelegate(standarts_Delegate);
    standarts_Delegate->active_ch = &active_ch;
    standarts_Delegate->style = StyleApp;
    header << "N°" << "" << tr("ch") << tr("Name") << tr("Values");
    Standarts_Table->setHorizontalHeaderLabels(header);
    Standarts_Table->horizontalHeader()->setFont(qApp->font());

    connect(count_Standarts, SIGNAL(valueChanged(int)), this, SLOT(Change_CountStandarts(int)));

    label_unit = new QLabel(tr("Units of value:  "));
    Units_Box = new QComboBox();
    header.clear();
    header << tr("copies") << tr("pg") << tr("IU");
    Units_Box->addItems(header);

    /*palette.setColor(QPalette::HighlightedText,Qt::black);
    palette.setColor(QPalette::Base,QColor(255,255,225));
    palette.setColor(QPalette::Highlight,QColor(180,180,255));
    Units_Box->setPalette(palette);*/
    Units_Box->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");

    Units_Box->setFont(QFont("Times New Roman", 12, QFont::Bold));
    QHBoxLayout *layout_units = new QHBoxLayout();
    layout_units->addWidget(label_unit);
    layout_units->addWidget(Units_Box);
    layout_units->addStretch(0);

    layout->addLayout(layout_count);
    //layout->addLayout(layout_Doubles);
    layout->addWidget(Standarts_Table,1);
    layout->addLayout(layout_units);
}
//-----------------------------------------------------------------------------
//--- create_HeaderTest()
//-----------------------------------------------------------------------------
void Quantity_PCR::create_HeaderTest(QWidget *parent)
{
    header_Test = new Header_Test(parent);
    active_ch = header_Test->active_Channels;
    header_Test->method_analysis = tr("Quantity");
}
//-----------------------------------------------------------------------------
//--- create_Common()
//-----------------------------------------------------------------------------
void Quantity_PCR::create_Common(QWidget *parent)
{
    common_Tree = new Common(parent);
    common_Tree->active_channel = active_ch;
}
//-----------------------------------------------------------------------------
//--- create_Standarts1
//-----------------------------------------------------------------------------
/*void Quantity_PCR::create_Standarts1(QWidget *parent)
{
    //standarts = new Standarts(parent);
    //common_Tree->active_channel = active_ch;
}*/
//-----------------------------------------------------------------------------
//--- create_Program()
//-----------------------------------------------------------------------------
void Quantity_PCR::create_Program(QWidget *parent)
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
void Quantity_PCR::Open_Program(QString fn)
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
                                User_Folder,
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
void Quantity_PCR::Edit_Program()
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

            qDebug() << text;

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
//--- Fill_TableQuality
//-----------------------------------------------------------------------------
void Quantity_PCR::Fill_TableQuality(int act_ch, QMap<int, QString> *map)
{
    int i,j;
    int id;
    QTableWidgetItem *item;
    QVector<int> channels;
    QString text;

    active_ch = act_ch;

    for(i=0; i<COUNT_CH; i++)
    {
        if(active_ch & (0xf<<4*i))
        {
          channels.append(i);
        }
    }

    //qDebug() << "Fill_TableQuality: " << channels;

    Table_Quality->clearContents();
    Table_Quality->clearSpans();
    Table_Quality->setRowCount(channels.size());

    for(i=0; i<Table_Quality->rowCount(); i++)
    {
        for(j=0; j<Table_Quality->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            Table_Quality->setItem(i,j,item);
            switch(j)
            {
            case 0:     text = QString::number(channels.at(i)); break;
            case 1:     text = tr("Sample");    break;
            case 2:     text = "";
                        if(map)
                        {
                            id = map->keys().indexOf(channels.at(i));
                            if(id >= 0)
                            {
                                text = map->values().at(id);
                            }
                        }
                        break;
            default:    text = ""; break;
            }
            item->setText(text);
        }
    }
    Table_Quality->setSpan(0,1, channels.size(),1);
}

//-----------------------------------------------------------------------------
//--- Get_ActiveChannels()
//-----------------------------------------------------------------------------
void Quantity_PCR::Get_ActiveChannels(int act_ch, QMap<int,QString> *map)

{
    int i,j;
    int id;
    QTableWidgetItem *item;
    QVector<int> channels;
    QString text;

    active_ch = act_ch;

    //qDebug() << "Get_ActiveChannels: " << act_ch;

    for(i=0; i<Standarts_Table->rowCount(); i++)
    {
        item = Standarts_Table->item(i,4);
        item->setText("0");
    }
    Change_CountStandarts(count_Standarts->value());


    for(i=0; i<COUNT_CH; i++)
    {
        if(active_ch & (0xf<<4*i))
        {
          channels.append(i);
        }
    }

    Table_Quality->clearContents();
    Table_Quality->clearSpans();
    Table_Quality->setRowCount(channels.size());

    for(i=0; i<Table_Quality->rowCount(); i++)
    {
        for(j=0; j<Table_Quality->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            Table_Quality->setItem(i,j,item);
            switch(j)
            {
            case 0:     text = QString::number(channels.at(i)); break;
            case 1:     text = tr("Sample");    break;
            case 2:     text = "";
                        if(map)
                        {
                            id = map->keys().indexOf(channels.at(i));
                            if(id >= 0)
                            {
                                text = map->values().at(id);
                            }
                        }
                        break;
            default:    text = ""; break;
            }
            item->setText(text);
        }
    }
    Table_Quality->setSpan(0,1, channels.size(),1);

}
//-----------------------------------------------------------------------------
//--- Change_CountStandarts
//-----------------------------------------------------------------------------
void Quantity_PCR::Change_CountStandarts(int count_tubes)
{
    int i,j;
    QTableWidgetItem *item;
    int count_ch = 0; //COUNT_CH;
    for(i=0; i<COUNT_CH; i++)
    {
        if(active_ch & (0x0f<<4*i)) count_ch++;
    }    

    int count = Standarts_Table->rowCount()*count_ch;
    int row = Standarts_Table->rowCount();
    int col = Standarts_Table->columnCount();
    int row_new = count_tubes*count_ch;
    QStringList list_old;

    for(i=0; i<row; i++)
    {
        item = Standarts_Table->item(i,4);
        if(item)
        {
            list_old << item->text();
        }
    }

    //...
    Standarts_Table->clearContents();
    Standarts_Table->clearSpans();
    Standarts_Table->setRowCount(row_new);


    for(i=0; i<row_new; i++)
    {

        for(j=0; j<col; j++)
        {
            item = new QTableWidgetItem();
            if(j == 4 && i < row) item->setText(list_old.at(i));
            else item->setText("0");
            //item->setText("");
            Standarts_Table->setItem(i,j,item);
            if(j < 4) item->setFlags(Standarts_Table->item(i,j)->flags() & ~Qt::ItemIsEditable & ~Qt::ItemIsSelectable);
        }
    }

    for(i=0; i<row_new; i++) Standarts_Table->setRowHeight(i,20);
    if(count_ch > 1)
    {
        for(i=0; i<count_tubes; i++) Standarts_Table->setSpan(i*count_ch,0,count_ch,1);
        for(i=0; i<count_tubes; i++) Standarts_Table->setSpan(i*count_ch,1,count_ch,1);
        for(i=0; i<count_tubes; i++) Standarts_Table->setSpan(i*count_ch,3,count_ch,1);
    }

    list_old.clear();
    Standarts_Table->repaint();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void StandartsItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    int i,j;
    QRect rect;
    QString text;
    int row,col;
    int id;
    QColor color;
    QFont f = qApp->font();
    painter->setFont(f);
    int font_size = f.pointSize();
    bool ok;
    QFontMetrics fm = painter->fontMetrics();

    QPixmap pixmap(":/images/disable.png");

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();

    int count_ch = 0;
    for(i=0; i<COUNT_CH; i++)
    {
        if(*active_ch & (0x0f<<(i*4))) count_ch++;
    }

    //... Background ...
    id = div(div(row,count_ch).quot, 2).rem;
    if(id) color = QColor(235,235,235);
    else color = QColor(245,245,245);
    if((option.state & QStyle::State_Selected) && col == 4) color = QColor(215,215,255);
    painter->fillRect(option.rect, color);


    //... Data ...

    if(Standart_isDisable) painter->setPen(QColor(Qt::gray));

    switch(col)
    {
    case 0:
                id = div(row,count_ch).quot;
                text = QString::number(id+1);
                painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 1:     break;

    case 2:     id = div(row,count_ch).rem;
                j = -1;
                for(i=0; i<COUNT_CH; i++)
                {
                    if(*active_ch & (0x0f<<(i*4)))
                    {
                        j++;
                        if(j == id) break;
                    }
                }
                if(style == "fusion")
                {
                    switch(i)
                    {
                        case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
                        case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
                        case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
                        case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
                        case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
                        default: pixmap.load(":/images/flat/disable_flat.png");   break;
                    }
                }
                else
                {
                    switch(i)
                    {
                        case 0:  pixmap.load(":/images/fam.png");    break;
                        case 1:  pixmap.load(":/images/hex.png");    break;
                        case 2:  pixmap.load(":/images/rox.png");    break;
                        case 3:  pixmap.load(":/images/cy5.png");    break;
                        case 4:  pixmap.load(":/images/cy55.png");   break;
                        default: pixmap.load(":/images/disable.png");   break;
                    }
                }

                painter->drawPixmap(rect.x()+(rect.width()-16)/2, rect.y()+2, pixmap);
                break;

    case 3:     id = div(row,count_ch).quot;
                text = QString(tr("Standart_%1")).arg(id+1);
                painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 4:
                //f.setBold(true);
                painter->setFont(f);
                //painter->setPen(Qt::red);
                if(fm.width(text) > rect.width()) painter->drawText(rect, Qt::AlignLeft + Qt::AlignVCenter, text);
                else painter->drawText(rect, Qt::AlignCenter, text);
                painter->setPen(Qt::black);
                break;
    default:    break;
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget* StandartsItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int i;
    QWidget* obj;
    int col = index.column();
    QPalette palette;
    QFont f = qApp->font();
    int font_size = f.pointSize();


    if(col == 4)
    {
        /*DotCommaDoubleSpinBox* obj_dsb = new DotCommaDoubleSpinBox(parent);
        obj_dsb->setDecimals(3);
        obj_dsb->setRange(0,1.0e+20);
        obj_dsb->setStyleSheet("QDoubleSpinBox {selection-background-color: rgb(180,180,255); selection-color: black;}");
        obj_dsb->setButtonSymbols(QAbstractSpinBox::NoButtons);
        */
        QLineEdit *obj_edit = new QLineEdit(parent);
        //QDoubleValidator *doubleVal = new QDoubleValidator( 0, 1.00e+20, 3, parent);
        //doubleVal->setNotation(QDoubleValidator::ScientificNotation);
        //obj_edit->setValidator(doubleVal);
        //obj_edit->setValidator(new QDoubleValidator( 0, 1.00e+20, 3, obj_edit));

        obj_edit->setValidator(new QRegExpValidator(QRegExp("[+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?")));
        obj_edit->setStyleSheet("selection-background-color: rgb(215,215,255); selection-color: black; border: 1px white transparent;");


        return(obj_edit);
    }

    return nullptr;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void StandartsItemDelegate::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    bool ok;

    QLineEdit *obj_edit = static_cast<QLineEdit*>(editor);
    QString text = obj_edit->text();
    double value = text.toDouble(&ok);
    if(ok)
    {
        if(value < 0.1 || value > 1000) text = QString("%1").arg(value, 0, 'e', 3);
        else text = QString("%1").arg(value, 0, 'f', 2);
        if(value == 0.0) text = "0";
    }
    else text = "0";
    model->setData(index, text, Qt::EditRole);
    //qDebug() << "setModelData ok: " << ok;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void QualityItemDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    int i,j;
    QRect rect;
    QString text;
    int row,col;
    int id;
    QColor color;
    QFont f = qApp->font();
    painter->setFont(f);
    int font_size = f.pointSize();
    bool ok;
    QFontMetrics fm = painter->fontMetrics();

    QPixmap pixmap(":/images/disable.png");
    painter->setFont(f);

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();


    //... Background ...
    if(col == 2) painter->fillRect(option.rect, QColor(235,235,235));
    else painter->fillRect(option.rect, QColor(245,245,245));

    if((option.state & QStyle::State_Selected) && col == 2) painter->fillRect(option.rect, QColor(215,215,255));

    //... Data ...

    switch(col)
    {
    case 0:     i = text.toInt(&ok);
                if(!ok) break;

                switch(i)
                {
                case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
                case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
                case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
                case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
                case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
                default: pixmap.load(":/images/flat/disable_flat.png");   break;
                }

                painter->drawPixmap(rect.x()+rect.width()/2-8, rect.y()+rect.height()/2-8, pixmap);
                break;

    case 1:     painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 2:     if(text == "Specific") text = tr("Specific");
                if(text == "IC") text = tr("IC");

                painter->drawText(rect, Qt::AlignCenter, text);
                break;


    default:    break;
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget* QualityItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    int col = index.column();



    if(col == 2)
    {
        QComboBox *editor = new QComboBox(parent);
        editor->addItem(tr("Specific"));
        editor->addItem(tr("IC"));
        editor->setStyleSheet("selection-background-color: #d7d7ff; selection-color: black; background: white;");

        connect(editor,SIGNAL(currentIndexChanged(int)), this, SLOT(Close_Editor(int)));

        emit showComboPopup(editor);

        return(editor);
    }

    return nullptr;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void QualityItemDelegate::updateEditorGeometry(QWidget *editor,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void QualityItemDelegate::setEditorData(QWidget *editor,
                                        const QModelIndex &index) const
{
    bool ok;
    QString text = index.model()->data(index, Qt::EditRole).toString();

    QComboBox *cBox = static_cast<QComboBox*>(editor);
    cBox->blockSignals(true);
    cBox->setCurrentIndex(-1);
    if(text.startsWith("Specific")) cBox->setCurrentIndex(0);
    if(text.startsWith("IC")) cBox->setCurrentIndex(1);
    cBox->blockSignals(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void QualityItemDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model,
                                       const QModelIndex &index) const
{
    QComboBox *cBox = static_cast<QComboBox*>(editor);
    QString text = "";

    switch(cBox->currentIndex())
    {
    case 0: text = "Specific";  break;
    case 1: text = "IC";    break;
    default:    text = "";  break;
    }

    model->setData(index, text, Qt::EditRole);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void QualityItemDelegate::Close_Editor(int id)
{
    QComboBox *bx = qobject_cast<QComboBox *>(sender());
    if(bx)
    {
        emit commitData(bx);    // fix data: This signal must be emitted when the editor widget has completed editing the data,
                                // and wants to write it back into the model.

        emit closeEditor(bx);   // This signal is emitted when the user has finished editing an item using the specified editor.
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void QualityItemDelegate::openComboPopup(QComboBox *bx)
{
    bx->showPopup();
}
