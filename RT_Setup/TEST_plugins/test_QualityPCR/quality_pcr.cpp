#include "quality_pcr.h"

extern "C" QUALITY_PCRSHARED_EXPORT Test_Interface* __stdcall createTest_plugin()
{
    return(new Quality_PCR());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Quality_PCR::Quality_PCR()
{
    TabGrid = NULL;
    readCommonSettings();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Quality_PCR::Create_Win(void *pobj, void *main)
{
    QWidget *parent = (QWidget*)pobj;
    parent_widget = parent;    

    TabGrid = new QTabWidget(parent);

    create_HeaderTest(parent);
    create_Program(parent);
    create_Common(parent);
    create_TableQuality(parent);

    TabGrid->addTab(header_Test, tr("Header"));
    TabGrid->addTab(program_group, tr("Program"));
    TabGrid->addTab(common_Tree, tr("Common"));
    TabGrid->addTab(Table_Quality, tr("Secific && IC"));

    connect(header_Test, SIGNAL(Change_ActiveChannels(int)), this, SLOT(Get_ActiveChannels(int)));
    connect(TabGrid, SIGNAL(currentChanged(int)), this, SLOT(Change_TabIndex(int)));

    Get_ActiveChannels(0x11);

    return(TabGrid);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Quality_PCR::Change_TabIndex(int index)
{
    QWidget *obj = TabGrid->currentWidget();
    if(obj) obj->setFocus();
}
//-----------------------------------------------------------------------------
//--- Get_ActiveChannels()
//-----------------------------------------------------------------------------
void Quality_PCR::Get_ActiveChannels(int act_ch, QMap<int,QString> *map)
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
//---
//-----------------------------------------------------------------------------
void Quality_PCR::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Quality_PCR::Destroy_Win()
{
    if(TabGrid != NULL)
    {
        delete program_group;

        header_Test->clear();
        delete header_Test;

        common_Tree->clear();
        delete common_Tree;

        delete quality_Delegate;
        Table_Quality->clear();
        delete Table_Quality;

        delete TabGrid;
        TabGrid = NULL;
    }
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Quality_PCR::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/test_quality_" + text + ".qm"))
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
void Quality_PCR::Show()
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
}
//-----------------------------------------------------------------------------
//--- Reserve(void *p)
//-----------------------------------------------------------------------------
void Quality_PCR::Reserve(void *p)
{
    QVector<QString> *info = (QVector<QString>*)p;

    User_Folder = info->at(0);
}

//-----------------------------------------------------------------------------
//--- Disable_Action()
//-----------------------------------------------------------------------------
void Quality_PCR::Disable_Action()
{
    // Header
    header_Test->setDisabled(true);

    // Program
    open_program->setDisabled(true);
    edit_program->setDisabled(true);

    // Common
    QTreeWidgetItem *item = common_Tree->topLevelItem(1);
    item->setDisabled(true);

}
//-----------------------------------------------------------------------------
//--- GetInformation
//-----------------------------------------------------------------------------
void Quality_PCR::GetInformation(QVector<QString> *info)
{    
    info->append("0x0002");
    info->append(tr("Base\\Quality"));
}
//-----------------------------------------------------------------------------
//--- Set_Test
//-----------------------------------------------------------------------------
void Quality_PCR::Set_Test(std::string xml, std::string name)
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
    QDomNode    child_node;
    QDomNode    channel_child;
    QDomNode    child_property;
    QDomElement child_element;
    QDomElement child_ch;
    QDomElement property_element;

    QMap<int,QString> Specific_IC;
    int key;


    // Name of Test, IDTest, Type of Analysis
    header_Test->Load_NewTest(QString::fromStdString(name), GetRandomString(), 0x0002);

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

            //
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

        // Quality values
        Get_ActiveChannels(active_ch, &Specific_IC);        
    }
}
//-----------------------------------------------------------------------------
//--- Get_Test(string *xml, string *name)
//-----------------------------------------------------------------------------
void Quality_PCR::Get_Test(string *xml, string *name)
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
void Quality_PCR::fill_Test(rt_Test *ptest)
{
    QString fluor_name[COUNT_CH] = FLUOR_NAME;

    if(ptest == NULL) return;

    int i,j;
    int id;
    int value;
    bool ok;
    QString text, str;    
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
        ch = new rt_ChannelTest();
        tube->channels.push_back(ch);
        ch->ID_Channel = i;
        ch->name = fluor_name[i].toStdString();
        ch->number = i;
        count_ch++;
    }

    // 5. About Specific & IC
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
//--- create_TableQuality()
//-----------------------------------------------------------------------------
void Quality_PCR::create_TableQuality(QWidget *parent)
{
    QStringList header;

    Table_Quality = new QTableWidget(0, 3, parent);
    Table_Quality->setFont(qApp->font());
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
//--- create_HeaderTest()
//-----------------------------------------------------------------------------
void Quality_PCR::create_HeaderTest(QWidget *parent)
{
    header_Test = new Header_Test(parent);
    active_ch = header_Test->active_Channels;
    header_Test->method_analysis = tr("Quality");
}
//-----------------------------------------------------------------------------
//--- create_Common()
//-----------------------------------------------------------------------------
void Quality_PCR::create_Common(QWidget *parent)
{
    common_Tree = new Common(parent);
    common_Tree->active_channel = active_ch;
}
//-----------------------------------------------------------------------------
//--- create_Program()
//-----------------------------------------------------------------------------
void Quality_PCR::create_Program(QWidget *parent)
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
void Quality_PCR::Open_Program(QString fn)
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
void Quality_PCR::Edit_Program()
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
