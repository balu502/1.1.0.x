#include "test_hrm.h"

extern "C" TEST_HRMSHARED_EXPORT Test_Interface* __stdcall createTest_plugin()
{
    return(new Test_HRM());
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Test_HRM::Test_HRM()
{
    TabGrid = NULL;
    readCommonSettings();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Test_HRM::Create_Win(void *pobj, void *main)
{
    QWidget *parent = (QWidget*)pobj;
    parent_widget = parent;

    TabGrid = new QTabWidget(parent);

    create_HeaderTest(parent);
    create_Program(parent);
    create_Common(parent);
    create_Others(parent);

    TabGrid->addTab(header_Test, tr("Header"));
    TabGrid->addTab(program_group, tr("Program"));
    TabGrid->addTab(common_Tree, tr("Common"));
    TabGrid->addTab(group_Others, tr("Others"));

    connect(TabGrid, SIGNAL(currentChanged(int)), this, SLOT(Change_TabIndex(int)));

    return(TabGrid);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Test_HRM::Change_TabIndex(int index)
{
    QWidget *obj = TabGrid->currentWidget();
    if(obj) obj->setFocus();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Test_HRM::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Test_HRM::Destroy_Win()
{
    if(TabGrid != NULL)
    {
        delete program_group;

        header_Test->clear();
        delete header_Test;

        common_Tree->clear();
        delete common_Tree;

        delete label_quality;
        delete quality_percent;
        delete use_CpValidity;
        delete CpValidity;
        delete group_Others;

        delete TabGrid;
        TabGrid = NULL;
    }
}
//-----------------------------------------------------------------------------
//--- Show()
//-----------------------------------------------------------------------------
void Test_HRM::Show()
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
//--- Reserve(void*)
//-----------------------------------------------------------------------------
void Test_HRM::Reserve(void *p)
{
    QVector<QString> *info = (QVector<QString>*)p;

    User_Folder = info->at(0);
}

//-----------------------------------------------------------------------------
//--- Disable_Action()
//-----------------------------------------------------------------------------
void Test_HRM::Disable_Action()
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
void Test_HRM::GetInformation(QVector<QString> *info)
{
    info->append("0x0010");
    info->append(tr("Base\\HRM"));
}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Test_HRM::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/test_hrm_" + text + ".qm"))
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
//--- Set_Test
//-----------------------------------------------------------------------------
void Test_HRM::Set_Test(std::string xml, std::string name)
{
    int i,j,k,m,n;
    int value;
    int id_ch;
    int row;
    bool ok;
    bool change_ID = false;
    QByteArray b_array;

    QTreeWidgetItem *item;
    QList<QTreeWidgetItem*> list_item;
    QString text;
    rt_Protocol *prot_temp;
    QStringList list;

    QDomDocument doc;
    QDomElement root;
    QDomNode    child;
    QDomNode    child_node;
    QDomNode    child_property;
    QDomElement child_element;
    QDomElement child_ch;
    QDomElement property_element;

    //qDebug() << "set test:";
    //qDebug() << "xml_Simple:" << QString::fromStdString(xml);

    // Name of Test, IDTest, Type of Analysis
    header_Test->Load_NewTest(QString::fromStdString(name), GetRandomString(), 0x0010);

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
                Load_Properties(child);
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- Get_Test(string *xml, string *name)
//-----------------------------------------------------------------------------
void Test_HRM::Get_Test(string *xml, string *name)
{
    QDomDocument doc_temp;
    QDomElement  item;
    QString text;

    //qDebug() << "get test:";

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
//--- Load_Properties(QDomNode &node)
//-----------------------------------------------------------------------------
void Test_HRM::Load_Properties(QDomNode &node)
{
    int i;
    QDomNode        child;
    QDomElement     element;
    int value;
    double dvalue;
    bool ok;
    QString text,name;

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);
        element = child.firstChildElement("name");
        if(element.isNull()) continue;

        text = child.firstChildElement("value").text();
        name = element.text();

        if(name == "BORDER_QUALITY")
        {
            value = text.toInt(&ok);
            if(!ok) value = 75;
            if(value < 0 || value > 100) value = 75;
            quality_percent->setValue(value);
        }
        if(name == "CP_VALIDITY")
        {
            dvalue = text.toDouble(&ok);
            if(!ok) dvalue = 0;

            if(dvalue > 0) use_CpValidity->setChecked(true);
            CpValidity->setValue(dvalue);
        }
    }
}

//-----------------------------------------------------------------------------
//--- fill_Test(rt_Test *ptest)
//-----------------------------------------------------------------------------
void Test_HRM::fill_Test(rt_Test *ptest)
{    
    rt_Preference *preference;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    bool find;
    double value;

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

    // 5. Quality Border
    find = false;
    for(i=0; i<ptest->preference_Test.size(); i++)
    {
        preference = ptest->preference_Test.at(i);
        if(QString::fromStdString(preference->name) == "BORDER_QUALITY")
        {
            preference->value = QString::number(quality_percent->value()).toStdString();
            find = true;
            break;
        }
    }
    if(!find)
    {
        preference = new rt_Preference();
        preference->name = "BORDER_QUALITY";
        preference->value = QString::number(quality_percent->value()).toStdString();
        ptest->preference_Test.push_back(preference);
    }

    // 5. Cp Validity
    find = false;
    value = 0.;
    if(use_CpValidity->checkState() == Qt::Checked && CpValidity->value() > 0)
    {
        value = CpValidity->value();
    }
    for(i=0; i<ptest->preference_Test.size(); i++)
    {
        preference = ptest->preference_Test.at(i);
        if(QString::fromStdString(preference->name) == "CP_VALIDITY")
        {
            preference->value = QString("%1").arg(value,0,'f',1).toStdString();
            find = true;
            break;
        }
    }
    if(!find)
    {
        preference = new rt_Preference();
        preference->name = "CP_VALIDITY";
        preference->value = QString("%1").arg(value,0,'f',1).toStdString();
        ptest->preference_Test.push_back(preference);
    }



}//-----------------------------------------------------------------------------
//--- create_Others()
//-----------------------------------------------------------------------------
void Test_HRM::create_Others(QWidget *parent)
{
    group_Others = new QGroupBox(parent);
    QVBoxLayout *layout = new QVBoxLayout();
    group_Others->setLayout(layout);
    QHBoxLayout *layout_hor = new QHBoxLayout();
    QHBoxLayout *layout_hor_validity = new QHBoxLayout();

    quality_percent = new QSpinBox();
    quality_percent->setRange(0,100);
    quality_percent->setSingleStep(1);
    quality_percent->setValue(75);
    quality_percent->setSuffix(" %");
    quality_percent->setFont(QFont("Times New Roman", 12, QFont::Bold));

    label_quality = new QLabel(tr("an indicator of quality of clustering"), parent);

    QLabel *label_num_1 = new QLabel(tr("1.   "), parent);


    layout_hor->addWidget(label_num_1, 0, Qt::AlignLeft);
    layout_hor->addWidget(quality_percent, 0, Qt::AlignLeft);
    layout_hor->addWidget(label_quality, 1, Qt::AlignLeft);


    QLabel *label_num_2 = new QLabel(tr("2.   "), parent);
    label_num_2->setVisible(false);
    use_CpValidity = new QCheckBox(tr("Use Cp validity: "), parent);
    use_CpValidity->setVisible(false);
    CpValidity = new QDoubleSpinBox();
    CpValidity->setRange(0, 100);
    CpValidity->setDecimals(1);
    CpValidity->setSingleStep(0.1);
    CpValidity->setValue(0);
    CpValidity->setFont(QFont("Times New Roman", 12, QFont::Bold));
    CpValidity->setDisabled(true);
    CpValidity->setVisible(false);

    layout_hor_validity->addWidget(label_num_2, 0, Qt::AlignLeft);
    layout_hor_validity->addWidget(use_CpValidity, 0, Qt::AlignLeft);
    layout_hor_validity->addWidget(CpValidity, 1, Qt::AlignLeft);

    layout->addLayout(layout_hor);
    layout->addSpacing(20);
    layout->addLayout(layout_hor_validity);
    layout->addStretch(1);

    connect(use_CpValidity, SIGNAL(toggled(bool)), this, SLOT(Change_UseCpValidity(bool)));
}

//-----------------------------------------------------------------------------
//--- create_Common()
//-----------------------------------------------------------------------------
void Test_HRM::create_Common(QWidget *parent)
{
    common_Tree = new Common(parent);
    common_Tree->active_channel = active_ch;
    //common_Tree->topLevelItem(0)->setHidden(true);
}
//-----------------------------------------------------------------------------
//--- create_HeaderTest()
//-----------------------------------------------------------------------------
void Test_HRM::create_HeaderTest(QWidget *parent)
{
    header_Test = new Header_Test(parent);
    active_ch = header_Test->active_Channels;
    header_Test->method_analysis = "HRM";
}
//-----------------------------------------------------------------------------
//--- create_Program()
//-----------------------------------------------------------------------------
void Test_HRM::create_Program(QWidget *parent)
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
    scheme_Program = new Scheme_ProgramAmpl(/*Qt::transparent*/QColor(1,1,1,1), parent);

    program_spl->addWidget(plot_Program);
    program_spl->addWidget(scheme_Program);

}
//-----------------------------------------------------------------------------
//--- Open_Program(QString fn)
//-----------------------------------------------------------------------------
void Test_HRM::Open_Program(QString fn)
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
void Test_HRM::Edit_Program()
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
//--- Change_UseCpValidity(bool state)
//-----------------------------------------------------------------------------
void Test_HRM::Change_UseCpValidity(bool state)
{
    CpValidity->setEnabled(state);
}
