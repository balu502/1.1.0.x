#include "analyser_quantity.h"

extern "C" ANALYSER_QUANTITYSHARED_EXPORT Analysis_Interface* __stdcall createAnalyser_plugin()
{
    return(new Analyser_Quantity());
}

static const char* const TOAST_STYLESHEET =
    "QLabel {"
    "  font-weight: bold;"
    "  padding: 10px 15px;"
    "  border: 2px solid;"
    "  border-radius: 10px;"
    "  background-color: rgba(255, 153, 153, 200);"
    "  border-color: red;"
    "}";

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Analyser_Quantity::Analyser_Quantity()
{
    MainBox = NULL;
    ru_Lang = false;
    readCommonSettings();

    Calc_StValue = false;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* Analyser_Quantity::Create_Win(void *pobj, void *main)
{
    int i;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QFont f = qApp->font();

    QWidget *parent = (QWidget*)pobj;
    main_widget = (QWidget*)main;

    MainBox = new QGroupBox(parent);
    MainBox->setFont(f);
    MainBox->setObjectName("Transparent");
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(4);
    MainBox->setLayout(main_layout);
    //MainBox->setStyleSheet("border: 0px");
    FileName_AdditionalSt = "";
    current_Test = NULL;

    main_spl = new QSplitter(Qt::Horizontal, MainBox);
    main_spl->setHandleWidth(10);
    main_spl->setChildrenCollapsible(false);
    main_layout->addWidget(main_spl);

    Tab_Result = new QTabWidget(MainBox);
    Results_Table = new Quantity_TableWidget();
    Results_Table->setContextMenuPolicy(Qt::CustomContextMenu);
    Results_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    Results_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    results_Delegate = new ResultsItemDelegate();
    Results_Table->setItemDelegate(results_Delegate);
    results_Delegate->background = &Samples_Background;
    Tab_Result->addTab(Results_Table, tr("Results"));
    copy_to_clipboard = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard"), this);
    copy_to_excelfile = new QAction(QIcon(":/images/copy_excel.png"),tr("copy to Excel"), this);
    copy_to_clipboard_on_column = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard by column"), this);

    Control_Box = new QGroupBox(MainBox);
    //Control_Box->setStyleSheet("border: 0px");
    //Control_Box->setStyleSheet("border: 1px dashed transparent;");
    //Std_Curve = new QGroupBox(MainBox);
    //Std_Curve->setStyleSheet("border: 0px");
    StdCurve_plot = new StdCurvePlot(MainBox);
    StdCurve_plot->setMinimumHeight(40);
    StdCurve_plot->setMinimumWidth(40);
    StdCurve_plot->setContextMenuPolicy(Qt::CustomContextMenu);
    save_as_PNG = new QAction(QIcon(":/images/images.png"),tr("save as PNG_Image"), this);

    control_spl = new QSplitter(Qt::Vertical, MainBox);
    control_spl->setHandleWidth(6);
    control_spl->setChildrenCollapsible(false);
    control_spl->addWidget(Control_Box);
    control_spl->addWidget(StdCurve_plot);

    QVBoxLayout *control_layout = new QVBoxLayout;
    control_layout->setMargin(3);
    control_layout->setSpacing(4);
    Control_Box->setLayout(control_layout);

    Fluor = new Fluor_ComboBox();
    fluor_delegate = new Fluor_ItemDelegate();
    Fluor->setItemDelegate(fluor_delegate);
    fluor_delegate->style = StyleApp;
    for(i=0; i<COUNT_CH; i++) Fluor->addItem(QIcon(":/images/fam.png"), fluor_name[i]);
    Tests_Box = new QComboBox(MainBox);
    Tests_Box->setCurrentIndex(-1);
    Tests_Box->setMinimumWidth(150);
    //Tests_Box->setStyleSheet("selection-background-color: #b4b4ff; selection-color: black;");
    f.setBold(true);
    Tests_Box->setFont(f);
    Tests_Label = new QLabel(tr("Test: "), MainBox);
    calc_St = new QPushButton(QIcon(":/images/St.png"),"", MainBox);
    calc_St->setToolTip(tr("Calculated value for standards"));
    calc_St->setObjectName("Transparent");
    calc_St->setIconSize(QSize(24,24));
    calc_St->setFlat(true);
    calc_St->setCheckable(true);
    calc_St->setChecked(false);
    QHBoxLayout *horiz_layout = new QHBoxLayout;
    horiz_layout->addWidget(Tests_Label, 1, Qt::AlignRight);
    horiz_layout->addWidget(Tests_Box, 0, Qt::AlignRight);
    horiz_layout->addWidget(Fluor, 0, Qt::AlignRight);
    horiz_layout->addWidget(calc_St, 0, Qt::AlignRight);

    Tab_Control = new QTabWidget(MainBox);
    Standart_Table = new QTableWidget(MainBox);
    //Standart_Table->setStyleSheet("background-color: rgb(245,245,245);");
    Standart_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    Standart_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    standarts_Delegate = new StandartsItemDelegate();
    Standart_Table->setItemDelegate(standarts_Delegate);
    Group_AdditionalSt = new QGroupBox(MainBox);
    //Group_AdditionalSt->setCheckable(true);
    //Group_AdditionalSt->setChecked(false);
    //Group_AdditionalSt->setTitle(tr(" use standards from another protocol"));
    QVBoxLayout *add_layout = new QVBoxLayout;
    Group_AdditionalSt->setLayout(add_layout);
    QHBoxLayout *fpath_layout = new QHBoxLayout;
    FilePath_Label = new FileName_Label(Group_AdditionalSt);
    FilePath_Label->setFrameStyle(QFrame::Panel | QFrame::StyledPanel);
    //FilePath_Label->setStyleSheet("QLabel{background-color: rgb(255,255,225)}");
    f.setBold(false);
    FilePath_Label->setFont(f);
    //FilePath_Label->setFixedHeight(21);
    //FilePath_Label->setMinimumWidth(40);
    Open_addProtocol = new QPushButton(QIcon(":/images/open_flat.png"), "", Group_AdditionalSt);
    Open_addProtocol->setObjectName("Transparent");
    //Open_addProtocol->setStyleSheet("QPushButton{border:none;background-color:transparent;}");
    Open_addProtocol->setIconSize(QSize(24,24));
    Open_addProtocol->setFlat(true);
    fpath_layout->addWidget(FilePath_Label,1); //,Qt::AlignLeft);
    fpath_layout->addWidget(Open_addProtocol,0,Qt::AlignRight);
    AddStdCurve_plot = new AddStdCurvePlot(Group_AdditionalSt);
    AddStdCurve_plot->setMinimumHeight(40);
    AddStdCurve_plot->setMinimumWidth(40);
    AddStdCurve_plot->setContextMenuPolicy(Qt::CustomContextMenu);
    StNotDetected_Label = new QLabel(tr("Standards not detected!"), Group_AdditionalSt);
    StNotDetected_Label->setFrameStyle(QFrame::Panel | QFrame::Plain);
    StNotDetected_Label->setStyleSheet(TOAST_STYLESHEET);
    StNotDetected_Label->adjustSize();
    StNotDetected_Label->hide();
    add_layout->addLayout(fpath_layout);
    add_layout->addWidget(AddStdCurve_plot);
    Tab_Control->addTab(Standart_Table, tr("Standards"));
    Tab_Control->addTab(Group_AdditionalSt, tr("Additional standards"));
    //Tab_Control->setTabEnabled(1,false);    // !!!!

    control_layout->addLayout(horiz_layout);
    control_layout->addWidget(Tab_Control);

    main_spl->addWidget(control_spl);
    main_spl->addWidget(Tab_Result);


    connect(Tests_Box, SIGNAL(currentIndexChanged(int)), this, SLOT(Fill_StdCurveForTest(int)));
    connect(Fluor, SIGNAL(currentIndexChanged(int)), this, SLOT(Fill_StdCurveForFluor(int)));
    connect(Standart_Table, SIGNAL(cellChanged(int,int)), this, SLOT(Changed_StandartsParameters(int,int)));
    connect(StdCurve_plot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_Graph()));
    connect(save_as_PNG, SIGNAL(triggered(bool)), this, SLOT(to_Image()));
    connect(Results_Table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_ResultsTable()));
    connect(Results_Table, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(ResultsTable_ChangedCurrentCell(int,int,int,int)));
    connect(copy_to_clipboard, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard()));
    connect(copy_to_excelfile, SIGNAL(triggered(bool)), this, SLOT(to_Excel()));
    connect(Open_addProtocol, SIGNAL(clicked(bool)), this, SLOT(Open_AdditionalProtocol()));
    connect(FilePath_Label, SIGNAL(Resize()), this, SLOT(Resize_FilePath()));
    connect(copy_to_clipboard_on_column, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard_ByColumn()));
    connect(calc_St, SIGNAL(clicked(bool)), this, SLOT(Calculate_StandartValue()));

    return(MainBox);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::Destroy()
{    
    delete this;    
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::Destroy_Win()
{
    QVector<sample_Quantity*> *vector;
    QVector<StdCurve_Result*> *vector_result;
    StdCurve_Result *result;
    QVector<double> *vector_max;
    QVector<QColor> *vector_color;


    if(MainBox != NULL)
    {
        qDeleteAll(Samples_AddStandards);
        Samples_AddStandards.clear();

        qDeleteAll(Samples_Quantity);
        Samples_Quantity.clear();
        foreach(vector, Map_Samples)
        {
            vector->clear();
            delete vector;
        }
        Map_Samples.clear();
        Map_NamesTests.clear();

        foreach(vector_result, Map_TestsResults)
        {
            foreach(result, *vector_result)
            {
                delete result;
            }
            vector_result->clear();
            delete vector_result;
        }
        Map_TestsResults.clear();

        foreach(vector_max, Max_Concentration)
        {
            delete vector_max;
        }
        Max_Concentration.clear();
        foreach(vector_color, Color_Channels)
        {
            delete vector_color;
        }
        Color_Channels.clear();

        Samples_Background.clear();

        delete fluor_delegate;
        delete Fluor;
        delete calc_St;

        delete standarts_Delegate;
        Standart_Table->clear();
        delete Standart_Table;

        delete AddStdCurve_plot;
        delete Group_AdditionalSt;

        delete results_Delegate;
        Results_Table->clear();
        delete Results_Table;

        delete StdCurve_plot;

        delete MainBox;
        MainBox = NULL;
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::Show()
{
    MainBox->show();    
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::GetInformation(QVector<QString> *info)
{
    info->append("0x0001");
    info->append(tr("Quantity_PCR"));
}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Analyser_Quantity::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/analyser_quantity_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
            if(text == "ru") ru_Lang = true;
        }
        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::Enable_Tube(QVector<short> *e)
{
    Fill_ResultsTable();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::Select_Tube(int pos)
{
    int i;
    int col,row;

    Prot->Plate.PlateSize(Prot->count_Tubes,row,col);

    QString str;
    QString text = Convert_IndexToName(pos,col);


    //... Result ...
    for(i=0; i<Results_Table->rowCount(); i++)
    {
        str = Results_Table->item(i,0)->text();
        if(str.trimmed() == text.trimmed())
        {
            Results_Table->setCurrentCell(i,0);
            break;
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::Save_Results(char *fname)
{
    int i,j,k,m,n;
    int id_ch;
    QDomDocument    doc;
    QDomElement     root;
    QDomElement     data;
    QDomElement     item;
    QDomElement     result;
    QDomElement     ch_result;
    QDomElement     quantity_result;

    QDomNode        sourceSet;
    QDomNode        item_source;
    QDomNode        samples;
    QDomNode        sample;
    QDomNode        properties;
    QDomNode        item_properties;
    QDomNode        tubes;
    QDomNode        item_tubes;
    QDomNode        channels;
    QDomNode        item_channels;
    QDomNode        properties_ch;
    QDomNode        item_properties_ch;
    QDomElement     id_test;
    QDomElement     name;
    QDomElement     value;
    QDomElement     new_value;
    QDomElement     pos_element;
    bool find_St;
    int pos;
    bool ok;
    double dvalue;
    bool is_Exists;

    sample_Quantity *sample_St;
    QVector<sample_Quantity*> *vector_samples;
    QVector<StdCurve_Result*> *vector_result;
    StdCurve_Result *std_curve;

    QString text(fname);
    QString ID_Test;

    //qDebug() << "save to file: " << text;

    // 1. Read & Edit Info
    QFile file(text);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(doc.setContent(&file))
        {
            file.close();

            root = doc.documentElement();            

            //... ReWrite concentration of the Standards ...

            //qDebug() << "keys: idTests " << Map_Samples.keys();
            sourceSet = root.firstChildElement("sourceSet");
            if(!sourceSet.isNull())
            {
                for(i=0; i<sourceSet.childNodes().size(); i++)
                {
                    item_source = sourceSet.childNodes().at(i);
                    samples = item_source.firstChildElement("samples");
                    for(j=0; j<samples.childNodes().size(); j++)
                    {
                        sample = samples.childNodes().at(j);
                        find_St = false;
                        id_test = sample.firstChildElement("IDTest");
                        properties = sample.firstChildElement("properties");
                        if(properties.isNull()) continue;
                        for(k=0; k<properties.childNodes().size(); k++)
                        {
                            item_properties = properties.childNodes().at(k);
                            name = item_properties.firstChildElement("name");
                            value = item_properties.firstChildElement("value");
                            if(!name.isNull() && !value.isNull() && name.text() == "kind" && value.text() == "Standart")
                            {
                                find_St = true;
                                break;
                            }
                            else continue;
                        }
                        if(!find_St) continue;

                        // ... Standart ...
                        vector_samples = Map_Samples.value(id_test.text(), NULL);
                        if(vector_samples == NULL) continue;

                        tubes = sample.firstChildElement("tubes");
                        for(k=0; k<tubes.childNodes().size(); k++)
                        {
                            item_tubes = tubes.childNodes().at(k);
                            pos_element = item_tubes.firstChildElement("pos");
                            pos = pos_element.text().toInt(&ok);
                            if(!ok) continue;

                            foreach(sample_St, *vector_samples)
                            {
                                if(sample_St->type == mSample) continue;
                                if(sample_St->pos != pos) continue;

                                channels = item_tubes.firstChildElement("channels");
                                if(channels.isNull()) continue;
                                for(m=0; m<channels.childNodes().size(); m++)
                                {
                                    item_channels = channels.childNodes().at(m);
                                    properties_ch = item_channels.firstChildElement("properties");
                                    if(properties_ch.isNull()) break;
                                    for(n=0; n<properties_ch.childNodes().size(); n++)
                                    {
                                        item_properties_ch = properties_ch.childNodes().at(n);
                                        name = item_properties_ch.firstChildElement("name");
                                        value = item_properties_ch.firstChildElement("value");
                                        if(!name.isNull() && !value.isNull() && name.text() == "Standarts Value" &&
                                            m < sample_St->conc.size())
                                        {
                                            dvalue = sample_St->conc.at(m);
                                            if(!sample_St->active) dvalue = 0.;
                                            new_value = MakeElement(doc, "value", QString::number(dvalue,'g',6));
                                            item_properties_ch.replaceChild(new_value, value);
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }

            //... Analysis Data ...
            data = root.firstChildElement("Analysis_Data");
            if(!data.isNull())
            {
                is_Exists = false;
                for(i=0; i<data.childNodes().size(); i++)
                {
                    quantity_result = data.childNodes().at(i).toElement();
                    if(quantity_result.hasAttribute("name"))
                    {
                        if(quantity_result.attribute("name") == NAME_METHOD)
                        {
                            qDebug() << NAME_METHOD;
                            is_Exists = true;
                            break;
                        }
                    }
                }

                item = doc.createElement("item");
                item.setAttribute("name", NAME_METHOD);

                foreach(vector_result, Map_TestsResults)
                {
                    result = doc.createElement("item");
                    ID_Test = Map_TestsResults.key(vector_result);
                    result.setAttribute("IDTest", ID_Test);

                    id_ch = 0;
                    foreach(std_curve, *vector_result)
                    {
                        ch_result = doc.createElement("item");
                        ch_result.appendChild(MakeElement(doc, "IDChannel", QString::number(std_curve->id_channel)));   // id channel
                        ch_result.appendChild(MakeElement(doc, "C0", QString::number(std_curve->c0)));                  // C0
                        ch_result.appendChild(MakeElement(doc, "C1", QString::number(std_curve->c1)));                  // C1
                        ch_result.appendChild(MakeElement(doc, "Eff", QString::number(std_curve->Eff)));                // Eff
                        ch_result.appendChild(MakeElement(doc, "R2", QString::number(std_curve->R_2)));                 // R2
                        ch_result.appendChild(MakeElement(doc, "Sigma", QString::number(std_curve->Sigma)));            // Sigma

                        result.appendChild(ch_result);
                        id_ch++;
                    }
                    item.appendChild(result);
                }
                if(is_Exists) data.replaceChild(item, quantity_result);
                else data.appendChild(item);
            }
        }
        else file.close();

        // 2. Write info
        if(file.open(QIODevice::WriteOnly))
        {
            QTextStream(&file) << doc.toString();
            file.close();
        }
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::Analyser(rt_Protocol *prot)
{
    int i,j,k,m,n;
    int id;
    int num;
    bool ok;
    QString text;    
    QString ID_test, Name_test;
    QString name_res, value_res;
    double dvalue;
    QStringList list;
    int active_ch;
    int id_ch;
    bool use_Ct;
    QColor bg_color[COUNT_CH] = {0xD8E2F1,0xD8E9D8,0xFEF0CB,0xFFE6FF,0xFFE6FF};

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    rt_Channel      *channel;
    rt_Preference   *property;
    sample_Quantity *sample_Q;

    QVector<sample_Quantity*> *vector;
    QVector<StdCurve_Result*> *vector_result;
    StdCurve_Result *result;
    QVector<double> *vector_max;
    QVector<QColor> *vector_color;

    Prot = prot;
    StdCurve_plot->Prot = Prot;

    Tests_Box->blockSignals(true);

    // 1. Find samples with type_analysis = 0x0001
    // Clear ALL
    qDeleteAll(Samples_AddStandards);
    Samples_AddStandards.clear();

    qDeleteAll(Samples_Quantity);
    Samples_Quantity.clear();
    foreach (vector, Map_Samples)
    {
        vector->clear();
        delete vector;
    }
    Map_Samples.clear();
    Map_NamesTests.clear();
    foreach(vector_result, Map_TestsResults)
    {
        foreach(result, *vector_result)
        {
            delete result;
        }
        vector_result->clear();
        delete vector_result;
    }
    Map_TestsResults.clear();

    foreach(vector_max, Max_Concentration)
    {
        delete vector_max;
    }
    Max_Concentration.clear();
    foreach(vector_color, Color_Channels)
    {
        delete vector_color;
    }
    Color_Channels.clear();

    //...

    for(i=0; i<prot->Plate.groups.size(); i++)
    {
        group = prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            if(sample->p_Test->header.Type_analysis == 0x0001)
            {
                sample_Q = new sample_Quantity();
                Samples_Quantity.append(sample_Q);
                sample_Q->unique_name = QString::fromStdString(sample->Unique_NameSample);      // unique_name
                sample_Q->ptest = sample->p_Test;                                               // ptest
                sample_Q->sample_Plate = sample;                                                // sample_Plate
                tube = sample->tubes.at(0);
                sample_Q->pos = tube->pos;                                                      // pos

                // use Cp or Ct ...
                use_Ct = false;
                foreach(property, sample_Q->ptest->preference_Test)
                {
                    if(property->name == "use_Threshold")
                    {
                        if(property->value == "True") use_Ct = true;
                        break;
                    }
                }

                ID_test = QString::fromStdString(sample_Q->ptest->header.ID_Test);
                if(!Map_Samples.keys().contains(ID_test))
                {
                    vector = new QVector<sample_Quantity*>();
                    Map_Samples.insert(ID_test, vector);
                    Name_test = QString::fromStdString(sample_Q->ptest->header.Name_Test);
                    Map_NamesTests.insert(Name_test,ID_test);
                    Tests_Box->addItem(QIcon(":/images/null.png"), Name_test);
                }
                else vector = Map_Samples.value(ID_test);
                vector->append(sample_Q);

                for(k=0; k<sample->preference_Sample.size(); k++)                               // type
                {
                    property = sample->preference_Sample.at(k);
                    if(property->name == "kind" && property->value == "Standart")
                    {
                        sample_Q->type = mStandart;                        
                        break;
                    }
                }

                for(k=0; k<tube->channels.size(); k++)
                {
                    channel = tube->channels.at(k);
                    for(m=0; m<channel->result_Channel.size(); m++)
                    {
                        text = QString::fromStdString(channel->result_Channel.at(m));
                        list = text.split("=");
                        name_res = QString(list.at(0)).trimmed();
                        value_res = QString(list.at(1)).trimmed();

                        if((name_res == "Cp" && !use_Ct) || (name_res == "Ct" && use_Ct))                                                    // Cp
                        {
                            dvalue = value_res.toDouble(&ok);
                            if(!ok) dvalue = 0.;
                            sample_Q->Cp.append(dvalue);
                            continue;
                        }                        
                    }
                }

                for(k=0; k<tube->channels.size(); k++) {sample_Q->conc.append(0.); sample_Q->conc_calc.append(0.);}

                if(sample_Q->type == mStandart)                                                 // concentration
                {                    
                    for(k=0; k<tube->channels.size(); k++)
                    {
                        channel = tube->channels.at(k);                        
                        for(m=0; m<channel->preference_Channel.size(); m++)
                        {
                            property = channel->preference_Channel.at(m);                            
                            if(property->name == "Standarts Value")
                            {
                                dvalue = QString::fromStdString(property->value).toDouble(&ok);
                                if(!ok) dvalue = 0.;
                                sample_Q->conc.replace(k, dvalue);                                
                            }
                        }
                    }
                }

                if(!Max_Concentration.keys().contains(ID_test))
                {
                    num = tube->channels.size();
                    vector_max = new QVector<double>();
                    vector_max->fill(0.,num);
                    Max_Concentration.insert(ID_test, vector_max);
                }
                if(!Color_Channels.keys().contains(ID_test))
                {
                    num = tube->channels.size();
                    vector_color = new QVector<QColor>();
                    vector_color->fill(QColor(0,0,0), num);
                    Color_Channels.insert(ID_test, vector_color);
                }

                //qDebug() << "sample_Q: " << sample_Q->unique_name << sample_Q->pos << sample_Q->Cp << sample_Q->conc;
            }
        }
    }        
    //qDebug() << *Map_Samples.value(ID_test);


    //2. Analysis for all Tests and all Fluor:
    for(i=0; i<Map_Samples.keys().size(); i++)
    {
        ID_test = Map_Samples.keys().at(i);
        vector = Map_Samples.value(ID_test);
        vector_color = Color_Channels.value(ID_test);
        active_ch = vector->at(0)->ptest->header.Active_channel;
        id_ch = 0;
        vector_result = new QVector<StdCurve_Result*>();
        Map_TestsResults.insert(ID_test, vector_result);
        for(j=0; j<Fluor->count(); j++)
        {
            if(active_ch & (0x0f<<j*4))
            {
                result = new StdCurve_Result();
                vector_result->append(result);
                StdCurve_Calculate(ID_test, id_ch, result);

                vector_color->replace(id_ch, bg_color[j]);
                id_ch++;
            }
        }        
    }

    // ID_Test
    Tests_Box->setCurrentIndex(-1);
    Tests_Box->blockSignals(false);
    if(Tests_Box->count()) Tests_Box->setCurrentIndex(0);   // Fill StdCurve Analysis for Test...

}
//-----------------------------------------------------------------------------
//--- Fill_StandartsTable()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Fill_StandartsTable(QString id_test, int id_ch)
{
    int i,j;
    QString text;
    QStringList header;
    QVector<sample_Quantity*> *vector_samples = Map_Samples.value(id_test);
    QVector<sample_Quantity*> vector_standarts;
    sample_Quantity *sample;
    QTableWidgetItem *item;
    int row_plate,col_plate;
    Prot->Plate.PlateSize(Prot->count_Tubes,row_plate,col_plate);

    Standart_Table->blockSignals(true);

    foreach(sample, *vector_samples)
    {
        if(sample->type == mSample) continue;
        vector_standarts.append(sample);
        //qDebug() << "Standarts: " << sample->unique_name << sample->conc << sample->Cp << sample->type;
    }

    foreach(sample, Samples_AddStandards)
    {
        if(sample->type != mAddStandart) continue;
        vector_standarts.append(sample);
        //qDebug() << "Add Standarts " << sample->unique_name << sample->conc << sample->Cp << sample->type;
    }

    header << tr("") << tr("Pos") << tr("Name") << tr("Cp") << tr("Conc");
    Standart_Table->clear();
    //Standart_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //Standart_Table->setSelectionMode(QAbstractItemView::NoSelection);
    //Standart_Table->setFocusPolicy(Qt::NoFocus);
    Standart_Table->setColumnCount(5);
    Standart_Table->setHorizontalHeaderLabels(header);
    for(i=0; i<Standart_Table->columnCount(); i++) Standart_Table->setColumnWidth(i,30);
    Standart_Table->setColumnWidth(0,20);
    Standart_Table->setColumnWidth(4,150);
    for(i=0; i<Standart_Table->columnCount(); i++) Standart_Table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    Standart_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    Standart_Table->setRowCount(vector_standarts.size());

    for(i=0; i<Standart_Table->rowCount(); i++)
    {
        sample = vector_standarts.at(i);        
        for(j=0; j<Standart_Table->columnCount(); j++)
        {
            item = new QTableWidgetItem();
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            switch(j)
            {
            case 0:         // Active
                    if(sample->active) {item->setCheckState(Qt::Checked); item->setText("1");}
                    else {item->setCheckState(Qt::Unchecked); item->setText("0");}
                    break;
            case 1:         // Pos
                    if(sample->type == mAddStandart) break;
                    item->setText(Convert_IndexToName(sample->pos, col_plate));
                    break;
            case 2:         // Name
                    if(sample->type == mAddStandart) item->setText(sample->unique_name + tr(" (add)"));
                    else item->setText(sample->unique_name);
                    break;
            case 3:         // Cp
                    text = QString::number(sample->Cp.at(id_ch),'f',1);
                    item->setText(text);
                    break;
            case 4:         // Conc
                    text = QString::number(sample->conc.at(id_ch),'f',0);
                    item->setText(text);
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                    break;
            }
            Standart_Table->setItem(i,j,item);
        }
        Standart_Table->setRowHeight(i,20);
    }
    Standart_Table->blockSignals(false);
}

//-----------------------------------------------------------------------------
//--- Fill_ResultsTable()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Fill_ResultsTable()
{
    int i,j;
    int row_current = 0;
    QString text;
    QStringList header;    
    int count = 0;
    QTableWidgetItem *item;

    QVector<sample_Quantity*> *vector_samples;
    QVector<StdCurve_Result*> *vector_results;
    sample_Quantity *sample;
    int count_ActChannel;
    int count_Sample;

    bool status_background = false;

    int row_plate,col_plate;
    Prot->Plate.PlateSize(Prot->count_Tubes,row_plate,col_plate);

    header << tr("Pos") << tr("Name") << tr("Cp") << tr("Conc");
    Results_Table->clear();
    Results_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //Results_Table->setSelectionMode(QAbstractItemView::NoSelection);
    //Results_Table->setFocusPolicy(Qt::NoFocus);
    Results_Table->setColumnCount(4);
    Results_Table->setHorizontalHeaderLabels(header);
    Results_Table->verticalHeader()->hide();

    Results_Table->setColumnWidth(0,30);    // pos
    Results_Table->setColumnWidth(1,40);    // name
    Results_Table->setColumnWidth(2,55);    // Cp
    Results_Table->setColumnWidth(3,40);    // conc
    for(i=0; i<Results_Table->columnCount(); i++) Results_Table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
    Results_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    Results_Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    foreach(vector_samples, Map_Samples)
    {                
        foreach(sample, *vector_samples)
        {
            if(!Prot->enable_tube.at(sample->pos)) continue;
            count += sample->Cp.size();
        }
    }
    Results_Table->setRowCount(count);

    Samples_Background.clear();
    Samples_Background.reserve(count);

    // Load data
    foreach(vector_samples, Map_Samples)
    {        
        foreach(sample, *vector_samples)
        {
            if(!Prot->enable_tube.at(sample->pos)) continue;

            status_background = !status_background;

            count = sample->Cp.size();
            for(i=0; i<count; i++)
            {
                for(j=0; j<Results_Table->columnCount(); j++)
                {
                    item = new QTableWidgetItem();
                    switch(j)
                    {
                    case 0:         // Pos
                                item->setText(Convert_IndexToName(sample->pos, col_plate));
                                break;
                    case 1:         // Name
                                item->setText(sample->unique_name);
                                break;
                    case 2:         // Cp
                                text = QString::number(sample->Cp.at(i),'f',1);
                                text += QString("~%1").arg(i);
                                item->setText(text);
                                break;
                    case 3:         // Conc_calc
                                if(sample->type == mStandart && !Calc_StValue) text = QString::number(sample->conc.at(i),'f',0);
                                else text = QString::number(sample->conc_calc.at(i),'f',0);
                                text += QString("~%1").arg(i);
                                item->setText(text);
                                break;
                    default:        // default
                                text = "";
                                break;
                    }
                    Results_Table->setItem(row_current, j, item);
                }
               Samples_Background.append((short)status_background);
               Results_Table->setRowHeight(row_current,20);
               row_current++;
            }
            Results_Table->setSpan(row_current-count,0,count,1);
            Results_Table->setSpan(row_current-count,1,count,1);
        }
    }
}
//-----------------------------------------------------------------------------
//--- CpTable_ToDigits_ByColumn()
//-----------------------------------------------------------------------------
QString Analyser_Quantity::ResultsTable_ToDigits_ByColumn()
{
    int i,j,k;
    int num,id;
    QString text, str = "";
    QString pos, pos_prev = "";
    QString sample_str = "";
    QTableWidgetItem *item;
    QStringList list;
    QStringList Vec;
    bool new_tube;
    QMap<int,QString> Cp_map;
    QMap<int,QString> Conc_map;

    for(i=0; i<Results_Table->rowCount(); i++)
    {
        for(j=0; j<Results_Table->columnCount(); j++)
        {
            item = Results_Table->item(i,j);
            text = item->text();

            switch(j)
            {
            case 0:
                    pos = text;
                    if(pos != pos_prev)
                    {
                        new_tube = true;

                        if(sample_str.size())
                        {
                            foreach(text, Cp_map.values())
                            {
                                sample_str += QString("%1\t").arg(text);
                            }
                            foreach(text, Conc_map.values())
                            {
                                sample_str += QString("%1\t").arg(text);
                            }

                            str += sample_str + "\r\n";
                        }
                        for(k=0; k<COUNT_CH; k++)
                        {
                            Cp_map.insert(k,"-");
                            Conc_map.insert(k,"-");
                        }
                        sample_str.clear();
                        sample_str = QString("%1\t").arg(pos);
                    }
                    else new_tube = false;
                    pos_prev = pos;
                    break;

            case 1:
                    if(new_tube) sample_str += QString("%1\t").arg(text);
                    break;

            case 2:
                    text.replace(".",",");
                    list = text.split("~");
                    id = QString(list.at(1)).toInt();
                    Cp_map.insert(id, list.at(0));
                    break;

            case 3:
                    text.replace(".",",");
                    list = text.split("~");
                    id = QString(list.at(1)).toInt();
                    Conc_map.insert(id, list.at(0));
                    break;

            default:    break;
            }
        }
    }

    foreach(text, Cp_map.values())
    {
        sample_str += QString("%1\t").arg(text);
    }
    foreach(text, Conc_map.values())
    {
        sample_str += QString("%1\t").arg(text);
    }

    str += sample_str + "\r\n";

    return(str);
}

//-----------------------------------------------------------------------------
//--- CpTable_ToDigits()
//-----------------------------------------------------------------------------
QString Analyser_Quantity::ResultsTable_ToDigits()
{
    int i,j,k;
    int num;
    QString text, str = "";
    QTableWidgetItem *item;
    QStringList list;
    QStringList Vec;


    for(i=0; i<Results_Table->rowCount(); i++)
    {
        for(j=0; j<Results_Table->columnCount(); j++)
        {
            item = Results_Table->item(i,j);
            text = item->text();

            switch(j)
            {
            default:    Vec.append(text);
                        break;

            case 2:     list = text.split("~");
                        if(list.size() > 1) Vec.append(QString(list.at(1)));
                        else Vec.append("...");
                        text = list.at(0);
                        text.replace(".",",");
                        Vec.append(text);
                        break;

            case 3:     list = text.split("~");
                        text = list.at(0);
                        text.replace(".",",");
                        Vec.append(text);
                        break;
            }
        }

        if(Vec.size())
        {
            for(j=0; j<Vec.size(); j++)
            {
                if(j) str += "\t";
                str += Vec.at(j);
            }
            str += "\r\n";
        }
        else str += "\r\n";

        Vec.clear();
    }

    return(str);
}

//-----------------------------------------------------------------------------
//--- ResultsTable_ChangedCurrentCell(int, int, int, int)
//-----------------------------------------------------------------------------
void Analyser_Quantity::ResultsTable_ChangedCurrentCell(int cur_row, int cur_col, int prev_row, int prev_col)
{
    QTableWidgetItem *item;
    QString text;
    int pos = 0;
    int count_col,count_row;

    if(!Prot || cur_row < 0 || cur_col < 0) return;

    int count_tubes = Prot->count_Tubes;
    Prot->Plate.PlateSize(count_tubes, count_row, count_col);


    if(cur_col > 0)
    {
        Results_Table->setCurrentCell(cur_row,0);
    }

    item = Results_Table->item(cur_row,0);
    text = item->text();
    pos = Convert_NameToIndex(text,count_col);
    Select_Curve(pos);
}

//-----------------------------------------------------------------------------
//--- Open_AdditionalProtocol()
//-----------------------------------------------------------------------------
QString Analyser_Quantity::MinimizeFileName(QWidget *label, QString fn)
{
    int i,j;
    QString result = fn;
    QStringList list;
    int width_label = label->width();
    int width_text = label->fontMetrics().boundingRect(fn).width();
    if(width_text < width_label) return(result);

    list = fn.split("/");
    QString drive = list.at(0);
    QString fname = list.last();
    list.removeLast();
    list.removeFirst();
    QString text;

    for(i=0; i<list.size(); i++)
    {
        text = drive + "/../";
        for(j=0; j<list.size(); j++)
        {
            if(j <= i) continue;
            text += list.at(j) + "/";
        }
        text += fname;
        width_text = label->fontMetrics().boundingRect(text).width();
        if(width_text < width_label) break;
    }

    return(text);
}

//-----------------------------------------------------------------------------
//--- Open_AdditionalProtocol()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Resize_FilePath()
{
    FilePath_Label->setText(MinimizeFileName(FilePath_Label, FileName_AdditionalSt));

    int x = (Group_AdditionalSt->width() - StNotDetected_Label->width())/2;
    StNotDetected_Label->move(x,50);
}

//-----------------------------------------------------------------------------
//--- Open_AdditionalProtocol()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Open_AdditionalProtocol()
{
    QString fname;    
    QString selectedFilter;

    QString ID_test = Map_NamesTests.value(Tests_Box->currentText());
    int ch = Fluor->currentIndex();
    int i;
    int id_ch = -1;
    int active_ch = Map_Samples.value(ID_test)->at(0)->ptest->header.Active_channel;
    for(i=0; i<=ch; i++)
    {
        if(active_ch & (0x0f<<i*4)) id_ch++;
    }
    QVector<StdCurve_Result*> *vector_result = Map_TestsResults.value(ID_test);

    int x = (Group_AdditionalSt->width() - StNotDetected_Label->width())/2;

    StNotDetected_Label->hide();

    fname = QFileDialog::getOpenFileName(NULL,
                                         tr("Open Protocol"),
                                         "",
                                         tr("Protocols File (*.r96 *.r48 *.384 *.rt)"),
                                         &selectedFilter);
    if(fname.isEmpty()) return;

    QFileInfo fi(fname);
    if(!fi.isFile()) return;

    FileName_AdditionalSt = fi.absoluteFilePath();
    FilePath_Label->setText(MinimizeFileName(FilePath_Label, FileName_AdditionalSt));

    if(current_Test == NULL || !Load_AddStandards(FileName_AdditionalSt, current_Test))
    {
        StNotDetected_Label->move(x,50);
        StNotDetected_Label->raise();
        StNotDetected_Label->show();

        Clear_AddStdCurves();
    }
    else  Draw_AddStdCurves();

    foreach(sample_Quantity *sample, Samples_AddStandards)
    {
        //qDebug() << "add_standart: " << sample->unique_name << sample->Cp << sample->conc << sample->type;
    }

    if(Samples_AddStandards.size())
    {

        Fill_StandartsTable(ID_test, id_ch);
        StdCurve_Calculate(ID_test, id_ch, vector_result->at(id_ch));
        Draw_StdCurve(ID_test, id_ch);
        Fill_ResultsTable();
    }
}

//-----------------------------------------------------------------------------
//--- Clear_AddStdCurves()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Clear_AddStdCurves()
{
    qDeleteAll(AddStdCurve_plot->list_Curve);
    AddStdCurve_plot->list_Curve.clear();
    AddStdCurve_plot->replot();
}

//-----------------------------------------------------------------------------
//--- Draw_AddStdCurves()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Draw_AddStdCurves()
{
    int i;
    QString text;
    QStringList list_data;
    double value;
    sample_Quantity *sample_AddSt;
    QwtPlotCurve *curve = AddStdCurve_plot->curve;
    QVector<QwtPlotCurve*> *list_Curve = &AddStdCurve_plot->list_Curve;    
    int active_ch;
    int id_ch = -1;
    int ch = Fluor->currentIndex();
    bool ok;

    QVector<double> x,y;

    Clear_AddStdCurves();
    if(Samples_AddStandards.size() == 0) return;

    active_ch = current_Test->header.Active_channel;
    for(i=0; i<=ch; i++)
    {
        if(active_ch & (0x0f<<i*4)) id_ch++;
    }
    text = Samples_AddStandards.at(0)->Analysis_Curves.at(0).trimmed();
    list_data = text.split(" ");
    for(i=0; i<list_data.size(); i++) x.append(double(i+1));    // X array
    y.fill(1.,x.size());

    foreach(sample_AddSt, Samples_AddStandards)
    {
        curve = new QwtPlotCurve();
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setCurveAttribute(QwtPlotCurve::Fitted, true);

        curve->attach(AddStdCurve_plot);
        list_Curve->append(curve);

        text = sample_AddSt->Analysis_Curves.at(id_ch).trimmed();
        list_data = text.split(" ");
        for(i=0; i<list_data.size(); i++)
        {
            value = QString(list_data.at(i)).toDouble(&ok);
            if(!ok) value = 0.;
            y.replace(i, value);
        }
        curve->setSamples(x,y);
    }

    AddStdCurve_plot->replot();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Analyser_Quantity::Select_Curve(int pos)
{
    event_plot.pos_plot = pos;
    QApplication::sendEvent(main_widget, &event_plot);
}
//-----------------------------------------------------------------------------
//--- Load_AddStandards()
//-----------------------------------------------------------------------------
bool Analyser_Quantity::Load_AddStandards(QString fn, rt_Test *ptest)
{
    int i,j,k,m,n,l;
    bool res = false;

    QDomDocument    doc;
    QDomElement     root;
    QDomNode        child;
    QDomNode        node;
    QDomNode        item;
    QDomNode        group;
    QDomNode        samples;
    QDomNode        properties;
    QDomNode        properties_item;
    QDomNode        tubes;
    QDomNode        tubes_item;
    QDomNode        channels;
    QDomNode        channels_item;
    QDomNode        channel_results;
    QDomNode        channel_results_item;
    QDomElement     node_name;
    QDomElement     node_value;
    QDomElement     IDTest;
    QDomElement     data;
    bool ok;
    bool sample_St;
    double value;
    int count_meas, pos;

    sample_Quantity *sample_AddSt;

    QString id_test, name_test;
    int method_test, activeCh_test;

    QFile file(fn);

    qDeleteAll(Samples_AddStandards);
    Samples_AddStandards.clear();

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        if(!doc.setContent(&file))
        {
            file.close();
            return(false);
        }
        root = doc.documentElement();



        for(i=0; i<root.childNodes().size(); i++)
        {
            child = root.childNodes().at(i);

            // ... tests ...
            if(child.nodeName() == "tests")
            {
                for(j=0; j<child.childNodes().size(); j++)
                {
                    item = child.childNodes().at(j);
                    for(k=0; k<item.childNodes().size(); k++)
                    {
                        node = item.childNodes().at(k);
                        if(node.nodeName() == "IDTest") {id_test = node.toElement().text(); continue;}
                        if(node.nodeName() == "nameTest") {name_test = node.toElement().text(); continue;}
                        if(node.nodeName() == "method") {method_test = node.toElement().text().toInt(&ok); continue;}
                        if(node.nodeName() == "activeChannels") {activeCh_test = node.toElement().text().toInt(&ok,16); continue;}
                    }
                    if(/*name_test == QString::fromStdString(ptest->header.Name_Test) &&*/
                       method_test == ptest->header.Type_analysis &&
                       activeCh_test == ptest->header.Active_channel) break;
                    else id_test = "";
                }
                if(!id_test.isEmpty()) res = true;
            }

            // ... samples ...
            if(child.nodeName() == "sourceSet")
            {
                if(!res) break;

                for(j=0; j<child.childNodes().size(); j++)
                {
                    group = child.childNodes().at(j);
                    samples = group.firstChildElement("samples");
                    if(!samples.isNull())
                    {
                        for(k=0; k<samples.childNodes().size(); k++)
                        {
                            item = samples.childNodes().at(k);
                            sample_St = false;
                            IDTest = item.firstChildElement("IDTest");
                            if(!IDTest.isNull())
                            {
                                if(id_test != IDTest.text()) continue;

                                properties = item.firstChildElement("properties");
                                if(!properties.isNull())
                                {
                                    for(m=0; m<properties.childNodes().size(); m++)
                                    {
                                        node = properties.childNodes().at(m);
                                        node_name = node.firstChildElement("name");
                                        node_value = node.firstChildElement("value");
                                        if(!node_name.isNull() && !node_value.isNull())
                                        {
                                            if(node_name.text() == "kind" && node_value.text() == "Standart")
                                            {
                                                sample_St = true;
                                                break;
                                            }
                                        }
                                        else continue;
                                    }
                                }
                            }
                            if(!sample_St) continue;

                            tubes = item.firstChildElement("tubes");
                            if(tubes.isNull()) continue;
                            for(m=0; m<tubes.childNodes().size(); m++)
                            {
                                node = tubes.childNodes().at(m);
                                sample_AddSt = new sample_Quantity();
                                Samples_AddStandards.append(sample_AddSt);
                                sample_AddSt->type = mAddStandart;
                                sample_AddSt->pos = node.firstChildElement("pos").text().toInt(&ok);
                                sample_AddSt->unique_name = item.firstChildElement("name").text();

                                channels = node.firstChildElement("channels");
                                if(channels.isNull()) continue;
                                for(n=0; n<channels.childNodes().size(); n++)
                                {
                                    channels_item = channels.childNodes().at(n);
                                    properties = channels_item.firstChildElement("properties");
                                    if(properties.isNull()) continue;
                                    for(l=0; l<properties.childNodes().size(); l++)
                                    {
                                        properties_item = properties.childNodes().at(l);
                                        node_name = properties_item.firstChildElement("name");
                                        if(!node_name.isNull() && node_name.text() == "Standarts Value")
                                        {
                                            node_value = properties_item.firstChildElement("value");
                                            if(!node_value.isNull())
                                            {
                                                value = node_value.text().toDouble(&ok);
                                                if(!ok) value = 0.;
                                                sample_AddSt->conc.append(value);
                                                sample_AddSt->conc_calc.append(value);
                                                break;
                                            }
                                        }
                                    }
                                    channel_results = channels_item.firstChildElement("channel_results");
                                    if(channel_results.isNull()) continue;
                                    for(l=0; l<channel_results.childNodes().size(); l++)
                                    {
                                        channel_results_item = channel_results.childNodes().at(l);
                                        node_name = channel_results_item.firstChildElement("name");
                                        if(!node_name.isNull() && node_name.text() == "Cp")
                                        {
                                            node_value = channel_results_item.firstChildElement("value");
                                            if(!node_value.isNull())
                                            {
                                                value = node_value.text().toDouble(&ok);
                                                if(!ok) value = 0.;
                                                sample_AddSt->Cp.append(value);                                                
                                                continue;
                                            }
                                        }
                                        /*if(!node_name.isNull() && node_name.text() == "Ct")
                                        {
                                            node_value = channel_results_item.firstChildElement("value");
                                            if(!node_value.isNull())
                                            {
                                                value = node_value.text().toDouble(&ok);
                                                if(!ok) value = 0.;
                                                sample_AddSt->Ct.append(value);
                                                continue;
                                            }
                                        }*/
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ... Analysis Curves ...
            if(child.nodeName() == "Analysis_Curves")
            {
                res = Samples_AddStandards.size();
                if(!res) break;

                node = child.firstChildElement("PCR");
                if(node.isNull()) continue;

                count_meas = node.firstChildElement("count").text().toInt(&ok);
                if(!ok) count_meas = 0;
                if(count_meas <= 0) continue;

                item = node.firstChildElement("AnalysisData");
                if(item.isNull()) continue;

                channels = item.firstChildElement("channels");
                if(channels.isNull()) continue;

                for(j=0; j<channels.childNodes().size(); j++)
                {
                    channels_item = channels.childNodes().at(j);
                    tubes = channels_item.firstChildElement("tubes");
                    foreach(sample_AddSt, Samples_AddStandards)
                    {
                        pos = sample_AddSt->pos;
                        tubes_item = tubes.childNodes().at(pos);
                        sample_AddSt->Analysis_Curves.append(tubes_item.firstChildElement("data").text());
                    }
                }
            }
        }


    }
    else return(false);    

    return(res);
}

//-----------------------------------------------------------------------------
//--- to_Excel()
//-----------------------------------------------------------------------------
void Analyser_Quantity::to_Excel()
{
    QString filename = "";
    QByteArray ba;

    QString text = ResultsTable_ToDigits();
    text.replace(QString("\t"), QString(";"));

    filename = QFileDialog::getSaveFileName(NULL, tr("Save to Excel(*.csv)..."),
                                            "ToExcel.csv", tr("Excel (*.csv)"));

    QFile file(filename);

    if(!filename.isEmpty())
    {
        if(file.open(QIODevice::WriteOnly))
        {            
            if(ru_Lang)
            {
                QTextCodec *codec1 = QTextCodec::codecForName("CP1251");
                qDebug() << text;
                ba = codec1->fromUnicode(text);
                file.write(ba);
                file.close();
            }
            else
            {
                QTextStream(&file) << text;
                file.close();
            }
        }
    }
}
//-----------------------------------------------------------------------------
//--- to_ClipBoard_ByColumn()
//-----------------------------------------------------------------------------
void Analyser_Quantity::to_ClipBoard_ByColumn()
{
    QString text = ResultsTable_ToDigits_ByColumn();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}

//-----------------------------------------------------------------------------
//--- to_ClipBoard()
//-----------------------------------------------------------------------------
void Analyser_Quantity::to_ClipBoard()
{
    QString text = ResultsTable_ToDigits();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- contextMenu_CpTable
//-----------------------------------------------------------------------------
void Analyser_Quantity::contextMenu_ResultsTable()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");

    menu.addAction(copy_to_clipboard);
    menu.addAction(copy_to_excelfile);
    menu.addSeparator();
    menu.addAction(copy_to_clipboard_on_column);
    menu.exec(QCursor::pos());

    menu.clear();

}

//-----------------------------------------------------------------------------
//--- contextMenu_Graph
//-----------------------------------------------------------------------------
void Analyser_Quantity::contextMenu_Graph()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");

    menu.addAction(save_as_PNG);
    menu.addSeparator();

    //if(!prot) menu.setEnabled(false);

    menu.exec(QCursor::pos());

    menu.clear();
}
//-----------------------------------------------------------------------------
//--- to_Image
//-----------------------------------------------------------------------------
void Analyser_Quantity::to_Image()
{
    QString filename = "";

    filename = QFileDialog::getSaveFileName(NULL, tr("Save to Image..."),
                                            "image.png", tr("Images (*.png)"));
    if(!filename.isEmpty())
    {
        QPixmap pixmap(StdCurve_plot->grab());
        pixmap.save(filename, "PNG");
    }
}
//-----------------------------------------------------------------------------
//--- Draw_StdCurve()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Draw_StdCurve(QString id_test, int id_ch)
{
    QVector<QPointF> data_Std;
    QVector<QPointF> data_Smp;
    QVector<QPointF> data_Lin;
    QPointF P;
    QVector<StdCurve_Result*> *vector_results = Map_TestsResults.value(id_test);
    QVector<sample_Quantity*> *vector_samples = Map_Samples.value(id_test);
    QVector<sample_Quantity*> vector_standarts;
    sample_Quantity *sample;
    double x,y;
    double min_X, max_X;
    double C0,C1;
    QVector<double> MinMax;
    QwtText title;
    QString text;

    QFont font_title = qApp->font(); //font_plot;
    font_title.setBold(false);
    font_title.setPointSize(font_title.pointSize()+2);
    title.setFont(font_title);

    //qDebug() << "vector_samples" << vector_samples->size();

    // Clear Chart...
    StdCurve_plot->Standarts_curve->setData(NULL);
    StdCurve_plot->LinFit_curve->setData(NULL);
    StdCurve_plot->Samples_curve->setData(NULL);

    StdCurve_plot->vec_St.clear();
    StdCurve_plot->vec_Sample.clear();

    // Load Data...
    foreach(sample, *vector_samples)
    {        
        if(sample->type == mSample || !sample->active) continue;
        vector_standarts.append(sample);
        StdCurve_plot->vec_St.append(sample);
    }
    // Add Standarts...
    foreach(sample, Samples_AddStandards)
    {
        if(sample->type != mAddStandart || !sample->active) continue;
        vector_standarts.append(sample);
        StdCurve_plot->vec_St.append(sample);
    }

    // Standarts:    
    foreach(sample, vector_standarts)
    {
        x = sample->conc.at(id_ch);
        y = sample->Cp.at(id_ch);
        if(x <= 0. || y <= 0.) continue;

        P.setX(log10(x));
        P.setY(y);
        data_Std.append(P);
        MinMax.append(P.x());
    }    

    // Samples:    
    foreach(sample, *vector_samples)
    {
        if(sample->type == mSample)
        {
            x = sample->conc_calc.at(id_ch);
            y = sample->Cp.at(id_ch);
            //qDebug() << "sample: " << sample->unique_name << x << y;
            if(x <= 0. || y <= 0.) continue;

            P.setX(log10(x));
            P.setY(y);
            data_Smp.append(P);
            MinMax.append(P.x());
            StdCurve_plot->vec_Sample.append(sample);
        }        
    }

    // Linear Fit:
    C0 = vector_results->at(id_ch)->c0;
    C1 = vector_results->at(id_ch)->c1;
    min_X = *std::min_element(MinMax.begin(),MinMax.end());
    max_X = *std::max_element(MinMax.begin(),MinMax.end());
    x = (max_X - min_X)*0.1;
    P.setX(min_X - x);
    P.setY(C0+C1*P.x());
    data_Lin.append(P);
    P.setX(max_X + x);
    P.setY(C0+C1*P.x());
    data_Lin.append(P);

    // Draw curves
    if(data_Std.size() > 1 && C0 != 0. && C1 != 0.)
    {
        StdCurve_plot->LinFit_curve->setSamples(data_Lin);
        StdCurve_plot->Standarts_curve->setSamples(data_Std);
        StdCurve_plot->Samples_curve->setSamples(data_Smp);
    }


    // Titul
    text = tr("Standart Curve");
    if(C0 != 0. && C1 != 0.)
    {
        text += QString(": Y = %1").arg(C0,0,'f',1);
        if(C1 > 0) text += QString(" + %1*X").arg(C1,0,'f',1);
        else text += QString(" - %1*X").arg(fabs(C1),0,'f',1);
    }
    title.setText(text);
    StdCurve_plot->setTitle(title);

    // Eff, sigma, R2
    if(C0 == 0. && C1 == 0.)
    {
        StdCurve_plot->eff_Label->setText("");
        //StdCurve_plot->sigma_Label->setText("");
        StdCurve_plot->r2_Label->setText("");
    }
    else
    {
        //qDebug() << "Eff: " << vector_results->at(id_ch)->Eff << vector_results->at(id_ch)->c1;
        StdCurve_plot->eff_Label->setText(QString("Eff = %1%   ").arg(vector_results->at(id_ch)->Eff, 0, 'f', 0));
        StdCurve_plot->eff_Label->setStyleSheet("QLabel {color : black;}");
        if(vector_results->at(id_ch)->Eff > 105 || vector_results->at(id_ch)->Eff < 90)
        {
            StdCurve_plot->eff_Label->setStyleSheet("QLabel {color : red;}");
        }
        //StdCurve_plot->sigma_Label->setText(QString("Sigma = %1   ").arg(vector_results->at(id_ch)->Sigma, 0, 'f', 3));
        StdCurve_plot->r2_Label->setText(QString("R2 = %1   ").arg(vector_results->at(id_ch)->R_2, 0, 'f', 3));
        StdCurve_plot->r2_Label->setStyleSheet("QLabel {color : black;}");
        if(vector_results->at(id_ch)->R_2 < 0.98)
        {
            StdCurve_plot->r2_Label->setStyleSheet("QLabel {color : red;}");
        }
    }

    StdCurve_plot->updateAxes();
    StdCurve_plot->show();
    StdCurve_plot->replot();
}

//-----------------------------------------------------------------------------
//--- StdCurve_Calculate()
//-----------------------------------------------------------------------------
void Analyser_Quantity::StdCurve_Calculate(QString id_test, int id_ch, void *p)
{
    int i;
    QVector<sample_Quantity*> *vector_samples = Map_Samples.value(id_test);
    QVector<sample_Quantity*> vector_standarts;
    sample_Quantity *sample;
    QVector<double> *vector_max = Max_Concentration.value(id_test);
    double coef_0,coef_1, sigma;
    QVector<double> X;
    QVector<double> Y;
    double dvalue;
    QVector<double> Yi,Yst;
    double Ymean,SSreg,SSoct;
    bool validity = false;

    rt_Sample   *sample_Plate;
    rt_Tube     *tube_Plate;
    rt_Channel  *channel_Plate;
    QString     str_result;

    StdCurve_Result *result = (StdCurve_Result *)p;    

    // initial properties
    result->id_Test = id_test;
    result->id_channel = id_ch;
    result->c0 = 0.;
    result->c1 = 0.;
    foreach(sample, *vector_samples)
    {        
        sample->conc_calc.replace(id_ch,0.);

        // Fill result properties of samples in Protocol
        dvalue = 0.;
        sample_Plate = sample->sample_Plate;
        tube_Plate = sample_Plate->tubes.at(0);
        channel_Plate = tube_Plate->channels.at(id_ch);
        str_result = QString("Calculated_Concentration=%1").arg(dvalue);
        AddResult(channel_Plate->result_Channel, str_result, "Calculated_Concentration=");
    }    
    vector_max->replace(id_ch, 0.);


    foreach(sample, *vector_samples)
    {
        if(sample->type == mSample || !sample->active) continue;
        vector_standarts.append(sample);        
    }
    foreach(sample, Samples_AddStandards)       // add standarts
    {
        if(sample->type != mAddStandart || !sample->active) continue;
        vector_standarts.append(sample);
    }

    if(vector_standarts.size() < 2)
    {
        return;
    }

    foreach(sample, vector_standarts)
    {
        dvalue = sample->conc.at(id_ch);        // check concentration
        if(dvalue <= 0.) continue;
        dvalue = sample->Cp.at(id_ch);          // check Cp
        if(dvalue <= 0.) continue;

        X.append(sample->conc.at(id_ch));
        Y.append(sample->Cp.at(id_ch));
    }

    //... check for validity
    if(X.size() < 2) return;
    foreach(dvalue, X)
    {
        if(dvalue != X.at(0)) {validity = true; break;}
    }
    if(!validity) return;
    //...

    sigma = Linear_Fit(X,Y,coef_0,coef_1);

    // Fill results structure:    
    result->c0 = coef_0;
    result->c1 = coef_1;
    result->Sigma = sigma;
    //...
    X.clear();
    foreach(sample, *vector_samples)                            // Calculated Concentration
    {
        sample->conc_calc.replace(id_ch, 0.);
        dvalue = 0.;
        if(sample->Cp.at(id_ch) > 0. && coef_1 != 0.)
        {
            dvalue = (sample->Cp.at(id_ch) - coef_0)/coef_1;
            sample->conc_calc.replace(id_ch, pow(10.,dvalue));
        }
        X.append(sample->conc_calc.at(id_ch));
        dvalue = sample->conc_calc.at(id_ch);

        if(sample->type == mStandart && !Calc_StValue) dvalue = sample->conc.at(id_ch);

        // Fill result properties of samples in Protocol
        sample_Plate = sample->sample_Plate;
        tube_Plate = sample_Plate->tubes.at(0);
        channel_Plate = tube_Plate->channels.at(id_ch);
        str_result = QString("Calculated_Concentration=%1").arg(dvalue);
        AddResult(channel_Plate->result_Channel, str_result, "Calculated_Concentration=");

        // only for standarts for calculate Eff,R2,Sigma
        /*if(sample->type == mStandart && sample->active)
        {
            Yi.append(sample->conc_calc.at(id_ch));
            Yst.append(sample->conc.at(id_ch));
        }*/
    }

    foreach(sample, vector_standarts)                // Calculated Concentration for Add Standarts
    {
        if(sample->type != mAddStandart) continue;
        sample->conc_calc.replace(id_ch, 0.);
        dvalue = 0.;
        if(sample->Cp.at(id_ch) > 0. && coef_1 != 0.)
        {
            dvalue = (sample->Cp.at(id_ch) - coef_0)/coef_1;
            sample->conc_calc.replace(id_ch, pow(10.,dvalue));
        }
    }

    // All standarts: only for standarts for calculate Eff,R2,Sigma
    foreach(sample, vector_standarts)
    {
        if((sample->type == mAddStandart || sample->type == mStandart) && sample->active)
        {
            Yi.append(sample->conc_calc.at(id_ch));
            Yst.append(sample->conc.at(id_ch));
        }
    }

    // Max value
    vector_max->replace(id_ch, *std::max_element(X.begin(),X.end()));

    // Eff
    if(coef_1 != 0.)
    {
        result->Eff = (pow(10., (-1./coef_1)) - 1.) * 100.;
        //qDebug() << coef_0 << coef_1 << result->Eff;
    }
    else result->Eff = 0;

    // R2
    SSreg = 0;
    SSoct = 0;
    double s1=0;
    double s2=0;
    if(Yi.size())
    {
        Ymean = std::accumulate(Yi.begin(), Yi.end(), 0.0)/Yi.size();
        for(i=0; i<Yi.size(); i++)
        {
            SSreg += pow(Yi.at(i) - Ymean, 2.);
            SSoct += pow(Yi.at(i) - Yst.at(i), 2.);

            //s1 += pow(Yi.at(i) - Yst.at(i), 2.);
            //s2 += pow(Yi.at(i) - Ymean, 2.);
        }
        result->R_2 = SSreg/(SSreg+SSoct);      // Коэффициент детерминации
        //result->R_2 = 1. - s1/s2;
    }
    else result->R_2 = 0;

    //qDebug() << "std_Curve: " << sigma << coef_0 << coef_1 << result->Eff << Yi << Yst << Ymean << result->R_2;

    X.clear();
    Y.clear();

}
//-----------------------------------------------------------------------------
//--- AddResult(vector<string>&, QString, QString)
//-----------------------------------------------------------------------------
void Analyser_Quantity::AddResult(vector<string> &vec, QString result, QString ref)
{
    int i,id;
    QString text;
    bool replace = false;

    for(i=0; i<vec.size(); i++)
    {
        text = QString::fromStdString(vec.at(i));
        if(text.startsWith(ref))
        {
            replace = true;
            id = i;
            break;
        }
    }
    if(replace) vec.at(id) = result.toStdString();
    else vec.push_back(result.toStdString());

}
//-----------------------------------------------------------------------------
//--- Linear_Fit() Y = C0 + C1*X
//-----------------------------------------------------------------------------
double Analyser_Quantity::Linear_Fit(QVector<double> &X, QVector<double> &Y, double &coef_0, double &coef_1)
{
    int i;
    int count = X.count();
    QVector<double> fx, fy, fw;
    double c0,c1,cov00,cov01,cov11,chisq,val;
    int err;

    fy.resize(count);
    fx.resize(count);
    fw.resize(count);

    //qDebug() << "Linear Fif:" << X << Y;

    for(i=0; i<count; i++)
    {        
        fy[i] = Y.at(i);
        fx[i] = log10(X.at(i));
        fw[i] = 1.;
        //qDebug() << "x,y:" << fx[i] << fy[i];
    }
    err = gsl_fit_wlinear(fx.data(),1,fw.data(),1,fy.data(),1,count,&c0,&c1,&cov00,&cov01,&cov11,&chisq);

    //qDebug() << "err: " << err;

    coef_0 = c0;
    coef_1 = c1;
    val = sqrt(fabs(chisq/count));

    fx.clear();
    fy.clear();
    fw.clear();

    return(val);
}
//-----------------------------------------------------------------------------
//--- Fluor_ItemDelegate::paint
//-----------------------------------------------------------------------------
void Analyser_Quantity::Fill_StdCurveForTest(int id)
{
    int i,j;
    QIcon icon;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;    
    int first_ch = -1;
    QString ID_test = Map_NamesTests.value(Tests_Box->currentText());
    int active_ch = Map_Samples.value(ID_test)->at(0)->ptest->header.Active_channel;    

    current_Test = Map_Samples.value(ID_test)->at(0)->ptest;

    // 1. Fluor_select ...
    Fluor->blockSignals(true);
    Fluor->clear();

    for(i=0; i<COUNT_CH; i++)
    {
       Fluor->addItem(fluor_name[i]);
       if(StyleApp == "fusion")
       {
           switch(i)
           {
           case 0:  icon.addFile(":/images/flat/fam_flat.png");   break;
           case 1:  icon.addFile(":/images/flat/hex_flat.png");   break;
           case 2:  icon.addFile(":/images/flat/rox_flat.png");   break;
           case 3:  icon.addFile(":/images/flat/cy5_flat.png");   break;
           case 4:  icon.addFile(":/images/flat/cy55_flat.png");   break;
           default: icon.addFile(NULL);   break;
           }
       }
       else
       {
           switch(i)
           {
           case 0:  icon.addFile(":/images/fam.png");   break;
           case 1:  icon.addFile(":/images/hex.png");   break;
           case 2:  icon.addFile(":/images/rox.png");   break;
           case 3:  icon.addFile(":/images/cy5.png");   break;
           case 4:  icon.addFile(":/images/cy55.png");   break;
           default: icon.addFile(NULL);   break;
           }
       }

       Fluor->setItemIcon(i,icon);

       if(!(active_ch & (0x0f<<i*4)))
       {
          Fluor->setItemData(i,0,Qt::UserRole - 1);
       }
       else
       {
           if(first_ch < 0) first_ch = i;
       }
    }

    Fluor->setCurrentIndex(first_ch);
    Fluor->blockSignals(false);

    results_Delegate->max_Conc = Max_Concentration.value(ID_test);
    results_Delegate->color_Ch = Color_Channels.value(ID_test);

    // 2. Clear Additional Standards Info
    qDeleteAll(Samples_AddStandards);
    Samples_AddStandards.clear();
    FilePath_Label->clear();
    Clear_AddStdCurves();

    // 3.
    Fill_StandartsTable(ID_test, 0);    
    Draw_StdCurve(ID_test, 0);
    Fill_ResultsTable();

}
//-----------------------------------------------------------------------------
//--- Fill_StdCurveForFluor()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Fill_StdCurveForFluor(int ch)
{
    int i;
    int id_ch = -1;
    QString ID_test = Map_NamesTests.value(Tests_Box->currentText());
    int active_ch = Map_Samples.value(ID_test)->at(0)->ptest->header.Active_channel;
    for(i=0; i<=ch; i++)
    {
        if(active_ch & (0x0f<<i*4)) id_ch++;
    }

    Fill_StandartsTable(ID_test, id_ch);
    Draw_StdCurve(ID_test, id_ch);
    Draw_AddStdCurves();
}
//-----------------------------------------------------------------------------
//--- Changed_StandartsParameters()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Changed_StandartsParameters(int row, int col)
{    
    int i;
    double value = 0;
    bool ok;
    QTableWidgetItem *item = Standart_Table->item(row,col);    
    QString name_test = Tests_Box->currentText();
    QString ID_test = Map_NamesTests.value(name_test);
    QVector<sample_Quantity*> *vector_sample = Map_Samples.value(ID_test);
    QVector<StdCurve_Result*> *vector_result = Map_TestsResults.value(ID_test);
    sample_Quantity *sample;
    QVector<sample_Quantity*> vector_standarts;
    int id_ch = 0;
    int active_ch;

    foreach(sample, *vector_sample)
    {
        if(sample->type == mStandart) vector_standarts.append(sample);
    }
    foreach(sample, Samples_AddStandards)
    {
        if(sample->type == mAddStandart) vector_standarts.append(sample);
    }

    sample = vector_standarts.at(row);

    if(sample->type == mAddStandart) active_ch = Map_Samples.value(ID_test)->at(0)->ptest->header.Active_channel;
    else active_ch = sample->ptest->header.Active_channel;

    for(i=0; i<Fluor->currentIndex(); i++)
    {
        if(active_ch & (0x0f<<(i*4))) id_ch++;
    }

    switch(col)
    {
    case 0:         // active
            if(item->checkState() == Qt::Unchecked) {sample->active = false; item->setText("0");}
            else {sample->active = true; item->setText("1");}
            break;

    case 4:         // conc
            value = item->text().toDouble(&ok);
            sample->conc.replace(id_ch, value);
            break;
    }

    StdCurve_Calculate(ID_test, id_ch, vector_result->at(id_ch));
    Draw_StdCurve(ID_test, id_ch);
    Fill_ResultsTable();
}

//-----------------------------------------------------------------------------
//--- SaveResults_Quantity()
//-----------------------------------------------------------------------------
void Analyser_Quantity::SaveResults_Quantity()
{

}
//-----------------------------------------------------------------------------
//--- SaveResults_Quantity()
//-----------------------------------------------------------------------------
void Analyser_Quantity::Calculate_StandartValue()
{
    //qDebug() << "calculate St: " << calc_St->isChecked();

    bool state = calc_St->isChecked();

    if(state) calc_St->setIcon(QIcon(":/images/St_calc.png"));
    else calc_St->setIcon(QIcon(":/images/St.png"));

    Calc_StValue = state;

    if(Standart_Table->rowCount()) Changed_StandartsParameters(0,0);

}

//-----------------------------------------------------------------------------
//--- Fluor_ItemDelegate::paint
//-----------------------------------------------------------------------------
void Fluor_ItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QRect rect = option.rect;
    QString text = index.data().toString();
    QFont font = qApp->font();
    QPixmap pixmap(":/images/fam.png");
    int row = index.row();


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    //__1. Background

    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        painter->fillRect(rect, QColor(255,255,255)); //QColor(255,255,225)
    }

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    painter->setFont(font);

    if(option.state & QStyle::State_Enabled)
    {
        if(style == "fusion")
        {
            switch(row)
            {
            case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
            case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
            case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
            case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
            case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
            default: pixmap.load(NULL);   break;
            }
        }
        else
        {
            switch(row)
            {
            case 0:  pixmap.load(":/images/fam.png");    break;
            case 1:  pixmap.load(":/images/hex.png");    break;
            case 2:  pixmap.load(":/images/rox.png");    break;
            case 3:  pixmap.load(":/images/cy5.png");    break;
            case 4:  pixmap.load(":/images/cy55.png");   break;
            }
        }
    }
    else
    {
        painter->setPen(QPen(QColor(192,192,192) ,1,Qt::SolidLine));
        if(style == "fusion") pixmap.load(":/images/flat/disable_flat.png");
        else pixmap.load(":/images/disable.png");
    }

    painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter->drawPixmap(0, rect.y()+2, pixmap);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
AddStdCurvePlot::AddStdCurvePlot(QWidget *parent):
    QwtPlot(parent)
{
    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    //grid->enableX(false);
    grid->attach(this);
    setCursor(Qt::ArrowCursor);
    canvas()->setCursor(Qt::ArrowCursor);

    enableAxis(QwtPlot::xBottom, false);
    enableAxis(QwtPlot::yLeft, false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
AddStdCurvePlot::~AddStdCurvePlot()
{
    qDeleteAll(list_Curve.begin(), list_Curve.end());
    list_Curve.clear();

    delete grid;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
StdCurvePlot::StdCurvePlot(QWidget *parent):
    QwtPlot(parent)
{
    QString text;
    QwtText text_x, text_y;
    QwtText title;

    QFont font_title = qApp->font(); //font_plot;
    font_title.setBold(false);
    font_title.setPointSize(font_title.pointSize()+1);
    title.setFont(font_title);

    canvas()->setCursor(Qt::ArrowCursor);
    canvas()->installEventFilter(this);

    // Samples
    Samples_curve = new QwtPlotCurve(tr("Samples"));
    Samples_curve->setStyle(QwtPlotCurve::Dots);
    Samples_curve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse,
                                   QBrush(Qt::green),
                                   QPen(Qt::black, 1),
                                   QSize(7,7)));
    Samples_curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
    Samples_curve->attach(this);

    // Standarts
    Standarts_curve = new QwtPlotCurve(tr("Standarts"));
    Standarts_curve->setStyle(QwtPlotCurve::Dots);
    Standarts_curve->setSymbol(new QwtSymbol(QwtSymbol::Rect,
                                   QBrush(Qt::red),
                                   QPen(Qt::black, 1),
                                   QSize(8,8)));
    Standarts_curve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
    Standarts_curve->attach(this);

    // LinFit
    LinFit_curve = new QwtPlotCurve(tr("Linear Fit"));
    LinFit_curve->setStyle(QwtPlotCurve::Lines);
    LinFit_curve->setPen(Qt::black,1);
    LinFit_curve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
    LinFit_curve->attach(this);

    QFont f = qApp->font();
    f.setBold(false);
    f.setPointSize(f.pointSize()-1);
    setAxisFont(QwtPlot::yLeft, f);
    setAxisFont(QwtPlot::xBottom, f);

    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->attach(this);

    title.setText(tr("Standart Curve"));
    setTitle(title);
    text_x.setText(tr("Log10(Conc)"));
    text_x.setFont(font_title);
    setAxisTitle(QwtPlot::xBottom, text_x);
    text_y.setText(tr("Cp"));
    text_y.setFont(font_title);
    setAxisTitle(QwtPlot::yLeft, text_y);

    setAxisAutoScale(QwtPlot::yLeft, true);
    setAxisAutoScale(QwtPlot::xBottom, true);

    // Eff, Sigma, R2
    eff_Label = new QLabel(tr("eff = "));
    //sigma_Label = new QLabel(tr("s = "));
    r2_Label = new QLabel(tr("R = "));
    QChar MathSymbolSigma(0x03C3);
    text = QString(MathSymbolSigma) + " = ";
    r2_Label->setText(text);

    QHBoxLayout *graph_H_layout = new QHBoxLayout();
    QHBoxLayout *graph_H_layout_1 = new QHBoxLayout();
    QHBoxLayout *graph_H_layout_2 = new QHBoxLayout();
    QVBoxLayout *graph_V_layout = new QVBoxLayout();

    graph_H_layout->setMargin(0);
    graph_H_layout_1->setMargin(0);
    graph_H_layout_2->setMargin(0);
    graph_V_layout->setMargin(2);
    graph_V_layout->setSpacing(0);

    graph_H_layout->addWidget(eff_Label,0,Qt::AlignRight);
    //graph_H_layout_1->addWidget(sigma_Label,0,Qt::AlignRight);
    graph_H_layout_2->addWidget(r2_Label,0,Qt::AlignRight);
    canvas()->setLayout(graph_V_layout);
    graph_V_layout->addLayout(graph_H_layout,0);
    graph_V_layout->addLayout(graph_H_layout_1,0);
    graph_V_layout->addLayout(graph_H_layout_2,0);
    graph_V_layout->addStretch();

    //font_title.setBold(true);
    //eff_Label->setFont(font_title);
    //sigma_Label->setFont(font_title);
    //r2_Label->setFont(font_title);

    //... Legend ...
    legend = new QwtLegend();
    insertLegend(legend, QwtPlot::BottomLegend);

    //... Sample_Info
    Sample_Info = new QLabel("", canvas());
    Sample_Info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    Sample_Info->setAutoFillBackground(true);
    Sample_Info->setAlignment(Qt::AlignCenter);
    Sample_Info->setVisible(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
StdCurvePlot::~StdCurvePlot()
{
    delete grid;
    delete legend;

    delete LinFit_curve;
    delete Standarts_curve;
    delete Samples_curve;
    delete eff_Label;
    //delete sigma_Label;
    delete r2_Label;

    delete Sample_Info;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool StdCurvePlot::eventFilter(QObject *object, QEvent *e)
{
    if(object != (QObject *)canvas()) return(false);

    switch(e->type())
    {
    default:    break;

    case QEvent::MouseButtonPress:
        if(((QMouseEvent *)e)->buttons() & Qt::LeftButton)
        {
            select_GraphMarker(((QMouseEvent *)e)->pos());
        }
        break;    
    }

    return(false);
}
//-----------------------------------------------------------------------------
//--- select_GraphMarker
//-----------------------------------------------------------------------------
void StdCurvePlot::select_GraphMarker(const QPoint &pos)
{
    double d_St, d_Sample;
    double d;
    int id_St, id_Sample;
    int id;
    int w;
    int row_plate,col_plate;
    QString text;
    QVector<sample_Quantity*> *vec;
    QPoint P;

    if(Standarts_curve->dataSize() == 0 || Samples_curve->dataSize() == 0) return;

    id_St = Standarts_curve->closestPoint(pos, &d_St);
    id_Sample = Samples_curve->closestPoint(pos, &d_Sample);

    id = id_St;
    d = d_St;
    vec = &vec_St;
    if(d_St > d_Sample) {id = id_Sample; d = d_Sample; vec = &vec_Sample;}

    if(d < 10)
    {
        P = pos;
        P.setY(P.y()-Sample_Info->height());
        Prot->Plate.PlateSize(Prot->count_Tubes,row_plate,col_plate);
        text = QString("%1, %2").arg(Convert_IndexToName(vec->at(id)->pos,col_plate))
                                .arg(vec->at(id)->unique_name);
        Sample_Info->setText(text);
        w = Sample_Info->fontMetrics().width(Sample_Info->text());
        w *= 1.2;
        Sample_Info->setFixedWidth(w);
        if(canvas()->width() < (P.x()+w))
        {
            P.setX(canvas()->width() - w - 5);
        }
        Sample_Info->move(P);
        Sample_Info->setVisible(true);
        QTimer::singleShot(2000, this, SLOT(Hide_LabelInfo()));
    }    
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ResultsItemDelegate::paint(QPainter *painter,
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
    double dvalue;
    QStringList list;
    double coef_conc = 1.;
    double dmax, width_conc;
    QPixmap pixmap(":/images/fam_flat.png");

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();

    //... Background ...
    if(background->at(row)) color = QColor(255,255,255);
    else color = QColor(235,235,235);
    //color = QColor(245,245,245);
    if(option.state & QStyle::State_Selected) color = QColor(215,215,255);
    /*if(col < 2)*/ painter->fillRect(option.rect, color);

    //... Data ...
    switch(col)
    {
    case 0:     painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 1:     painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 2:
                list.clear();
                list = text.split("~");
                text = list.at(0);
                dvalue = text.toDouble(&ok);
                if(!ok) dvalue = 0.;
                if(dvalue == 0.) text = "-";                

                id = QString(list.at(1)).toInt(&ok);
                if(!ok || id < 0 || id >= max_Conc->size()) break;

                switch(id)
                {
                case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
                case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
                case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
                case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
                case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
                default: pixmap.load(":/images/flat/disable_flat.png");   break;
                }

                //color = color_Ch->at(id);
                //painter->fillRect(rect.left(), rect.top(), 10, rect.height(), color);
                painter->drawPixmap( rect.left(), rect.top() + (rect.height()-8)/2 - 2, pixmap);

                rect.setLeft(rect.left() + 14);
                painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 3:
                list.clear();
                list = text.split("~");
                text = list.at(0);
                dvalue = text.toDouble(&ok);
                if(ok)
                {
                    if(dvalue < 0.01 || dvalue > 99999) text = QString("%1").arg(dvalue, 0, 'e', 3);
                    else
                    {
                        if(dvalue >= 100.) text = QString("%1").arg(dvalue, 0, 'f', 0);
                        if(dvalue >= 10. && dvalue < 100.) text = QString("%1").arg(dvalue, 0, 'f', 1);
                        if(dvalue < 10.) text = QString("%1").arg(dvalue, 0, 'f', 2);
                    }
                }
                else dvalue = 0.;
                if(dvalue == 0.) text = "";
                id = QString(list.at(1)).toInt(&ok);
                if(!ok || id < 0 || id >= max_Conc->size()) break;
                color = color_Ch->at(id);
                //painter->fillRect(rect, color);

                if(dvalue > 0.)
                {
                    dmax = max_Conc->at(id);
                    if(dmax <= 0.) break;

                    coef_conc = log10(dvalue)/log10(dmax);
                    width_conc = rect.width()*coef_conc - 10;
                    if(width_conc < 0.) width_conc = 1;

                    painter->fillRect(rect.left() + 5, rect.top() + 2, width_conc, rect.height() - 4, QColor(255, 0, 0,100));
                    painter->drawRect(rect.left() + 5, rect.top() + 2, width_conc, rect.height() - 4);

                    //f.setBold(true);
                    painter->setFont(f);
                    painter->drawText(rect, Qt::AlignCenter, text);
                }

                break;

    default:    break;
    }

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

    QStyleOptionButton cbOpt;
    bool isChecked;


    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();

    //... Background ...
    id = div(row, 2).rem;
    if(id) color = QColor(235,235,235);
    else color = QColor(255,255,255);

    if(option.state & QStyle::State_Selected) {color = QColor(215,215,255); painter->fillRect(option.rect, color);}
    else {/*if(col)*/ painter->fillRect(option.rect, color);}


    //... Data ...

    switch(col)
    {
    case 0:     //QStyledItemDelegate::paint(painter, viewOption, index);
                //qDebug() << "check: " << index.data().toBool();
                cbOpt.rect = option.rect;
                cbOpt.rect.moveLeft(3);
                isChecked = index.data().toBool();
                if(isChecked) cbOpt.state |= QStyle::State_On;
                else cbOpt.state |= QStyle::State_Off;
                cbOpt.state |= QStyle::State_Active | QStyle::State_Enabled;

                QApplication::style()->drawControl(QStyle::CE_CheckBox, &cbOpt, painter);
                break;

    case 1:     painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 2:     painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 3:     painter->drawText(rect, Qt::AlignCenter, text);
                break;

    case 4:     painter->drawText(rect, Qt::AlignCenter, text);
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

        obj_edit->setValidator(new QRegExpValidator(QRegExp("[+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?")));
        obj_edit->setStyleSheet("selection-background-color: rgb(215,215,255); selection-color: black; background-color: rgb(255,255,255);");

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
        if(value < 0.01 || value > 99999) text = QString("%1").arg(value, 0, 'e', 3);
        else text = QString("%1").arg(value, 0, 'f', 2);
        if(value == 0.0) text = "0";
    }
    else text = "0";
    model->setData(index, text, Qt::EditRole);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void FileName_Label::resizeEvent(QResizeEvent *e)
{    
    emit Resize();
}


