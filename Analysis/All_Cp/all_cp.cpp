#include "all_cp.h"

extern "C" ALL_CPSHARED_EXPORT Alg_Interface* __stdcall create_plugin()
{
    return(new All_Cp());
}


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
All_Cp::All_Cp()
{

}


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* All_Cp::ReferenceForMap()
{
    qDebug() << "map_All_Cp: " << &Map_TestTranslate;
    return(&Map_TestTranslate);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void* All_Cp::Create_Win(void *pobj, void *main, void *user)
{
    ru_Lang = false;
    readCommonSettings();

    QWidget *parent = (QWidget*)pobj;
    main_widget = (QWidget*)main;
    ax_user = (QAxObject*)user;

    TabGrid = new QTabWidget(parent);    

    //... Cp_Table ...
    Cp_Table = new QTableWidget(0,6);
    Cp_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    Cp_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    Cp_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Cp_Table->setContextMenuPolicy(Qt::CustomContextMenu);
    Delegate = new Cp_ItemDelegate;
    Cp_Table->setItemDelegate(Delegate);
    Delegate->style = StyleApp;
    connect(Cp_Table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_CpTable()));
    connect(Cp_Table, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(SelelectCell_CpTable(int,int)));
    //connect(Result_Table,SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(select_Curve(int,int,int,int)));


    //... MC_Table ...
    MC_Table = new QTableWidget(0,3);
    MC_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    MC_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    MC_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    MC_Table->setContextMenuPolicy(Qt::CustomContextMenu);
    MC_Delegate = new MC_ItemDelegate;
    MC_Table->setItemDelegate(MC_Delegate);
    MC_Delegate->style = StyleApp;
    connect(MC_Table, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_MCTable()));
    connect(MC_Table, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(SelelectCell_MCTable(int,int)));

    //... PCR Analysis ...
    PCR_Analysis = new PCRAnalysis(parent);

    //... Cross Table ...
    Cross_Box = new CrossBox(parent);
    Cross_Box->ru_Lang = ru_Lang;
    Cross_Box->map_TestTranslate = &Map_TestTranslate;
    connect(Cross_Box, SIGNAL(sSelect_Curve(int)), this, SLOT(Select_Curve(int)));

    TabGrid->addTab(Cp_Table,tr("Cp_Results"));
    TabGrid->addTab(PCR_Analysis,tr("PCR_Analysis"));
    TabGrid->addTab(MC_Table,tr("MC_Results"));
    TabGrid->addTab(Cross_Box,tr("CrossTable"));


    //... Actions ...
    copy_to_clipboard = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard"), this);
    copy_to_excelfile = new QAction(QIcon(":/images/copy_excel.png"),tr("copy to Excel"), this);
    copy_to_clipboard_MC = new QAction(QIcon(":/images/copy_clipboard.png"),tr("copy to ClipBoard"), this);
    copy_to_excelfile_MC = new QAction(QIcon(":/images/copy_excel.png"),tr("copy to Excel"), this);
    copy_Cp = new QAction(QIcon(":/images/copy_Cp.png"), tr("copy Cp"), this);

    connect(copy_to_clipboard, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard()));
    connect(copy_to_excelfile, SIGNAL(triggered(bool)), this, SLOT(to_Excel()));
    connect(copy_to_clipboard_MC, SIGNAL(triggered(bool)), this, SLOT(to_ClipBoard_MC()));
    connect(copy_to_excelfile_MC, SIGNAL(triggered(bool)), this, SLOT(to_Excel_MC()));
    connect(copy_Cp, SIGNAL(triggered(bool)), this, SLOT(to_Cp()));

    //... Flags ...
    flag_AddAnalysis = false;
    flag_CrossTable = false;
    if(ax_user)
    {
        flag_AddAnalysis = ax_user->dynamicCall("getPriv(QString)", "ENABLE_ADD_ANALYSIS").toBool();
        flag_CrossTable = ax_user->dynamicCall("getPriv(QString)", "ENABLE_CROSSTABLE").toBool();
    }



    return(TabGrid);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Destroy_Win()
{
    if(TabGrid != NULL)
    {
        Map_Validity.clear();
        Map_TestTranslate.clear();

        delete Cp_Table;
        delete MC_Table;
        delete PCR_Analysis;
        delete Cross_Box;

        delete TabGrid;
        TabGrid = NULL;
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Show()
{    
    if(!p_prot->count_PCR)
    {
        TabGrid->setTabEnabled(0,false);
        TabGrid->setTabEnabled(1,false);
    }
    if(!p_prot->count_MC) TabGrid->setTabEnabled(2,false);
    TabGrid->show();

    if(!flag_AddAnalysis) TabGrid->setTabEnabled(1,false);
    if(!flag_CrossTable) TabGrid->setTabEnabled(3,false);
    //Cp_Table->show();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Analyser(rt_Protocol *p, int param)
{

    p_prot = p;
    rt_Test *ptest;
    QStringList list_tr;

    //... ... Validate PCR Research    
    Validate_PCR();
    Validity_MC();
    //qDebug() << "Validate_PCR() - ok";    

    //... Translate Tests and Research
    if(!ru_Lang)
    {
        foreach(ptest, p_prot->tests)
        {
            if(ptest->header.Type_analysis > 0x20000)
            {
                Create_TranslateList(ptest, list_tr);
            }
        }
        if(list_tr.size()) Translate_Tests(list_tr);
    }

    //... 1. Cp_Table
    if(param == 0x0f) Fill_Cp_Table_Only_Ct();
    else Fill_Cp_Table();
    //qDebug() << "Cp_Table - ok";


    //... 2. PCR_Analysis    
    if(p_prot->count_PCR) PCR_Analysis->Fill_PCRAnalysis(p);
    //qDebug() << "PCR_Analysis  - ok";

    //... 3. MC_Table    
    Fill_MC_Table();
    //qDebug() << "Fill_MC_Table()  - ok";

    //... 4. Cross Tables    
    Cross_Box->Fill_CrossInfo(p);
    //qDebug() << "Cross Tables  - ok";    
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Destroy()
{
    delete this;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void All_Cp::readCommonSettings()
{
    QString text;
    QString dir_SysTranslate;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    ID_LANG = ID_LANG_RU;

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/allcp_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
            if(text == "ru") ru_Lang = true;
        }
        if(text == "en") ID_LANG = ID_LANG_EN;

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
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Enable_Tube(QVector<short> *buf)
{
    Fill_Cp_Table();
    Fill_MC_Table();
    Cross_Box->Fill_CrossInfo(p_prot);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Select_Tube(int pos)
{
    int i;
    int col,row;
    p_prot->Plate.PlateSize(p_prot->count_Tubes,row,col);

    QString str;
    QString text = Convert_IndexToName(pos,col);


    //... Cp ...
    for(i=0; i<Cp_Table->rowCount(); i++)
    {
        str = Cp_Table->verticalHeaderItem(i)->text();
        if(str.trimmed() == text.trimmed())
        {            
            Cp_Table->setCurrentCell(i,0);
            break;
        }
    }

    //... MC ...
    for(i=0; i<MC_Table->rowCount(); i++)
    {
        str = MC_Table->verticalHeaderItem(i)->text();
        if(str.trimmed() == text.trimmed())
        {
            MC_Table->setCurrentCell(i,0);
            break;
        }
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Select_Curve(int pos)
{
    event_plot.pos_plot = pos;

    QApplication::sendEvent(main_widget, &event_plot);

}

//-----------------------------------------------------------------------------
//--- Draw Cp Table
//-----------------------------------------------------------------------------
void Cp_ItemDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text;
    QStringList list, list_ch, list_tmp;
    int pos;
    int font_size;
    int left;
    QFont font;
    int dy;
    double K;
    bool ok;
    double value;

    int Dy,Dx;    

    QPixmap pixmap(":/images/fam.png");


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();

    //--- Header sample ---
    pos = text.indexOf("Header_Sample:");
    if(pos >= 0)
    {
        if(style == "fusion") painter->fillRect(rect, QColor(255,255,255));
        else painter->fillRect(rect, QColor(255,255,225));
        painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
        font = painter->font();
        font.setBold(true);
        painter->setFont(font);
        text = text.mid(14).trimmed();
        if(index.column() == 0) painter->drawText(rect, Qt::AlignCenter, text);        

        return;
    }
    //----------------------

    //__1. Background
    int row = index.row();
    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        if(div(row,2).rem)  painter->fillRect(option.rect, QColor(240,240,240));
        else painter->fillRect(option.rect, QColor(245,245,245));
    }
    //-----------------------

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    font = painter->font();
    font.setBold(false);
    painter->setFont(font);

    QFontMetrics fm(font);

    int col = index.column();
    text = index.data().toString();
    switch(col)
    {
    case 0:
            list = text.split("\r\n");
            for(i=0; i<list.size(); i++)
            {
                list_ch.append(list.at(i).mid(0,1));
                text = list.at(i);
                text.remove(0,2);
                list_tmp.append(text);
            }
            text = list_tmp.join("\r\n");
            rect.setX(rect.x()+20);
            painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);

            //K = 0.9;
            //dy = 0;
            //if(list_ch.size() > 1) {K = 0.7; dy = 2*(list_ch.size()-1);}
            //dy = (option.rect.height() - 16*list_ch.size())/(list_ch.size() - 1 + 4);
            dy = option.rect.height()/list_ch.size();
            for(i=0; i<list_ch.size(); i++)
            {
               id = list_ch.at(i).toInt();
               if(style == "fusion")
               {
                   switch(id)
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
               //painter->drawPixmap(0, option.rect.y() + 22*K*i + 2 + dy,pixmap);
               //painter->drawPixmap(0, option.rect.y() + (16 + dy)*i + 2*dy,pixmap);
               painter->drawPixmap(0, option.rect.y() + dy*i + (dy-16)/2., pixmap);
            }            
            break;    
    case 1:             
            font = painter->font();
            font.setBold(true);
            //font.setItalic(true);
            painter->setFont(font);            

            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    case 6:
            font = painter->font();
            font.setBold(false);
            painter->setFont(font);
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    case 2:
            font = painter->font();
            font.setBold(false);
            //font.setItalic(true);
            painter->setFont(font);
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    case 3:
            list = text.split("\r\n");

            font = painter->font();
            font.setBold(false);
            font.setItalic(false);
            painter->setFont(font);
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);            

            Dy = (option.rect.height() - fm.lineSpacing()*list.size())/2.;
            pixmap.load(":/images/BadImage.png");

            for(i=0; i<list.size(); i++)
            {
                text = list.at(i);
                if(text.contains("?"))
                {
                    painter->drawPixmap(option.rect.x() + option.rect.width() - 8,
                                        option.rect.y()+Dy+fm.lineSpacing()*i,
                                        pixmap);
                }
            }

            break;

    case 4:                             //aFF
    case 5:                             // Relative validity

            list = text.split("\r\n");
            Dy = (option.rect.height() - fm.lineSpacing()*list.size())/2.;
            Dx = (option.rect.width() - 16)/2.;

            text.replace("true", " ");
            text.replace("false", " ");
            text.replace("warning", " ");
            text.replace("none", "-");

            font = painter->font();
            font.setBold(false);
            font.setItalic(false);
            painter->setFont(font);
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);

            for(i=0; i<list.size(); i++)
            {
                text = list.at(i);
                if(text.contains("warning")) pixmap.load(":/images/warning.png");
                if(text.contains("true")) pixmap.load(":/images/ok.png");
                //if(text.contains("false")) pixmap.load(":/images/attention.png");
                if(text.contains("false")) pixmap.load(":/images/erase_new.png");

                if(text.contains("none")) ;//painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, "-");
                else
                {
                    painter->drawPixmap(option.rect.x() + Dx,
                                        option.rect.y()+Dy+fm.lineSpacing()*i,
                                        pixmap);
                }
            }

            break;


    default:
            break;

    }
}
//-----------------------------------------------------------------------------
//--- Draw MC Table
//-----------------------------------------------------------------------------
void MC_ItemDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text;
    QStringList list, list_ch, list_tmp;
    int pos;
    int left;
    QFont font;
    int dy;
    double K;
    bool attention;

    QPixmap pixmap(":/images/fam.png");


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();

    //--- Header sample ---
    pos = text.indexOf("Header_Sample:");
    if(pos >= 0)
    {
        if(style == "fusion") painter->fillRect(rect, QColor(255,255,255));
        else painter->fillRect(rect, QColor(255,255,225));
        painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
        font = painter->font();
        font.setBold(true);
        font.setItalic(false);
        painter->setFont(font);
        text = text.mid(14).trimmed();
        if(index.column() == 0) painter->drawText(rect, Qt::AlignCenter, text);

        return;
    }
    //----------------------

    //__1. Background
    int row = index.row();
    if(option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, QColor(215,215,255));
    }
    else
    {
        if(div(row,2).rem)  painter->fillRect(option.rect, QColor(240,240,240));
        else painter->fillRect(option.rect, QColor(245,245,245));
    }
    //-----------------------

    //__2. Data
    painter->setPen(QPen(Qt::black,1,Qt::SolidLine));
    font = painter->font();
    font.setBold(false);
    font.setItalic(false);
    painter->setFont(font);

    int col = index.column();
    text = index.data().toString();
    switch(col)
    {
    case 0:
            list = text.split("\r\n");
            for(i=0; i<list.size(); i++)
            {
                list_ch.append(list.at(i).mid(0,1));
                text = list.at(i);
                text.remove(0,2);
                list_tmp.append(text);
            }
            text = list_tmp.join("\r\n");
            rect.setX(rect.x()+20);
            painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);

            //K = 0.9;
            //dy = 0;
            //if(list_ch.size() > 1) {K = 0.7; dy = 2*(list_ch.size()-1);}
            //dy = (option.rect.height() - 16*list_ch.size())/(list_ch.size() - 1 + 4);
            dy = option.rect.height()/list_ch.size();
            for(i=0; i<list_ch.size(); i++)
            {
               id = list_ch.at(i).toInt();
               if(style == "fusion")
               {
                   switch(id)
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
               //painter->drawPixmap(0, option.rect.y() + 22*K*i + 2 + dy,pixmap);
               //painter->drawPixmap(0, option.rect.y() + (16 + dy)*i + 2*dy,pixmap);
               painter->drawPixmap(0, option.rect.y() + dy*i + (dy-16)/2., pixmap);
            }
            break;
    case 1:
    case 2:
            //font = painter->font();
            //font.setBold(true);
            //font.setItalic(true);
            //painter->setFont(font);
            painter->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
            break;

    default:
            break;

    }
}
//-----------------------------------------------------------------------------
//--- contextMenu_CpTable
//-----------------------------------------------------------------------------
void All_Cp::contextMenu_CpTable()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(copy_to_clipboard);
    menu.addAction(copy_to_excelfile);
    menu.addAction(copy_Cp);

    menu.exec(QCursor::pos());

    menu.clear();

}
//-----------------------------------------------------------------------------
//--- contextMenu_MCTable
//-----------------------------------------------------------------------------
void All_Cp::contextMenu_MCTable()
{
    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");
    menu.setFont(qApp->font());

    menu.addAction(copy_to_clipboard_MC);
    menu.addAction(copy_to_excelfile_MC);

    menu.exec(QCursor::pos());

    menu.clear();

}
//-----------------------------------------------------------------------------
//--- CpTable_Cp()
//-----------------------------------------------------------------------------
QString All_Cp::CpTable_Cp()
{
    int i,j,k;
    int num;
    QString text, str = "";
    QString pos, name;
    QTableWidgetItem *item;
    QStringList list;    
    QVector<int> ID_vec;
    QVector<QString> Cp_vec;
    QStringList Vec;
    QMap<int,QString> map_Results;

    for(i=0; i<Cp_Table->rowCount(); i++)
    {
        pos = Cp_Table->verticalHeaderItem(i)->text();
        ID_vec.clear();
        Cp_vec.clear();
        for(j=0; j<COUNT_CH; j++)
        {
            map_Results.insert(j,"-");
        }

        for(j=0; j<Cp_Table->columnCount(); j++)
        {
            if(j > 1) continue;

            item = Cp_Table->item(i,j);
            text = item->text();            

            if(text.startsWith("Header_Sample:"))
            {
                if(j == 0)
                {
                    text.remove("Header_Sample:");
                    name = text;
                    break;
                }
                else break;
            }
            else
            {
                if(j >0) text.replace(".",",");
                list = text.split("\r\n");
                switch(j)
                {
                case 0:
                            foreach(str, list)
                            {
                                num = str.mid(0,1).toInt();
                                ID_vec.append(num);
                            }
                            break;

                case 1:
                            foreach(str, list)
                            {
                                Cp_vec.append(str);
                            }
                            break;

                default:    break;
                }
            }
        }

        if(ID_vec.size())
        {

            //qDebug() << "i: " << i << pos << name << ID_vec << Cp_vec;

            text = QString("%1\t%2").arg(pos).arg(name);

            foreach(num, ID_vec)
            {
                map_Results.insert(num, Cp_vec.at(num));
            }

            foreach(str, map_Results.values())
            {
                text += QString("\t%1").arg(str);
            }

            Vec.append(text);
        }        
    }

    return(Vec.join("\r\n"));
}

//-----------------------------------------------------------------------------
//--- CpTable_ToDigits()
//-----------------------------------------------------------------------------
QString All_Cp::CpTable_ToDigits()
{
    int i,j,k;
    int num;
    QString text, str = "";
    QString pos;
    QString temp;
    QTableWidgetItem *item;
    QStringList list;
    QVector<QStringList*> Vec;
    QStringList *temp_list;
    QString header_str = " ";

    for(j=0; j<Cp_Table->columnCount(); j++)
    {
        temp = Cp_Table->horizontalHeaderItem(j)->text();
        header_str += "\t" + temp;
    }

    for(i=0; i<Cp_Table->rowCount(); i++)
    {
        pos = Cp_Table->verticalHeaderItem(i)->text();
        for(j=0; j<Cp_Table->columnCount(); j++)
        {
            item = Cp_Table->item(i,j);
            text = item->text();
            //qDebug() << "i,j,text: " << i << j << text;

            if(text.startsWith("Header_Sample:"))
            {
                if(j == 0)
                {
                    text.remove("Header_Sample:");
                    str += "\r\n" + text;
                    break;
                }
                else break;
            }
            else
            {
                if(j >0) text.replace(".",",");
                list = text.split("\r\n");

                if(j == 0)
                {
                    k = 0;
                    foreach(temp, list)
                    {
                        temp.remove(0,2);
                        list.replace(k,temp);
                        k++;
                    }
                }

                temp_list = new QStringList();
                for(k=0; k<list.size(); k++) temp_list->append(list.at(k));
                Vec.append(temp_list);
            }
        }

        if(Vec.size())
        {
            num = Vec.at(0)->size();
            for(j=0; j<num; j++)
            {
                for(k=0; k<Vec.size(); k++)
                {
                    if(k == 0) str += pos + "\t";
                    str += Vec.at(k)->at(j) + "\t";
                }
                str += "\r\n";
            }
        }
        else str += "\r\n";

        qDeleteAll(Vec);
        Vec.clear();
    }

    str = header_str + "\r\n" + str;
    //qDebug() << "str: " << str;
    return(str);
}

//-----------------------------------------------------------------------------
//--- MCTable_ToDigits()
//-----------------------------------------------------------------------------
QString All_Cp::MCTable_ToDigits()
{
    int i,j,k;
    int num;
    QString text, str = "";
    QString pos;
    QString temp;
    QTableWidgetItem *item;
    QStringList list;
    QVector<QStringList*> Vec;
    QStringList *temp_list;
    QString header_str = " ";

    for(j=0; j<MC_Table->columnCount(); j++)
    {
        temp = MC_Table->horizontalHeaderItem(j)->text();
        header_str += "\t" + temp;
    }

    for(i=0; i<MC_Table->rowCount(); i++)
    {
        pos = MC_Table->verticalHeaderItem(i)->text();
        for(j=0; j<MC_Table->columnCount(); j++)
        {
            item = MC_Table->item(i,j);
            text = item->text();

            if(text.startsWith("Header_Sample:"))
            {
                if(j == 0)
                {
                    text.remove("Header_Sample:");
                    str += "\r\n" + text;
                    break;
                }
                else break;
            }
            else
            {
                if(j >0) text.replace(".",",");
                list = text.split("\r\n");

                if(j == 0)
                {
                    k = 0;
                    foreach(temp, list)
                    {
                        temp.remove(0,2);
                        list.replace(k,temp);
                        k++;
                    }
                }

                temp_list = new QStringList();
                for(k=0; k<list.size(); k++) temp_list->append(list.at(k));
                Vec.append(temp_list);
            }
        }

        if(Vec.size())
        {
            num = Vec.at(0)->size();
            for(j=0; j<num; j++)
            {
                for(k=0; k<Vec.size(); k++)
                {
                    if(k == 0) str += pos + "\t";
                    str += Vec.at(k)->at(j) + "\t";
                }
                str += "\r\n";
            }
        }
        else str += "\r\n";

        qDeleteAll(Vec);
        Vec.clear();
    }

    str = header_str + "\r\n" + str;
    return(str);
}
//-----------------------------------------------------------------------------
//--- to_Excel()
//-----------------------------------------------------------------------------
void All_Cp::to_Excel()
{
    QString filename = "";
    QByteArray ba;

    QString text = CpTable_ToDigits();
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
//--- to_Excel_MC()
//-----------------------------------------------------------------------------
void All_Cp::to_Excel_MC()
{
    QString filename = "";
    QByteArray ba;

    QString text = MCTable_ToDigits();
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
//--- to_Cp()
//-----------------------------------------------------------------------------
void All_Cp::to_Cp()
{
    QString text = CpTable_Cp();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- to_ClipBoard()
//-----------------------------------------------------------------------------
void All_Cp::to_ClipBoard()
{    
    QString text = CpTable_ToDigits();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- to_ClipBoard_MC()
//-----------------------------------------------------------------------------
void All_Cp::to_ClipBoard_MC()
{
    QString text = MCTable_ToDigits();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(text);
}
//-----------------------------------------------------------------------------
//--- SelelectCell_CpTable
//-----------------------------------------------------------------------------
void All_Cp::SelelectCell_CpTable(int row, int col)
{    
    if(Cp_Table->item(row,col)->text().contains("Header_Sample:")) return;

    int pos = 0;
    QString text = Cp_Table->verticalHeaderItem(row)->text();
    int count_col,count_row;
    int count_tubes = p_prot->count_Tubes;

    p_prot->Plate.PlateSize(count_tubes, count_row, count_col);
    pos = Convert_NameToIndex(text,count_col);

    //qDebug() << row << col << text << Convert_NameToIndex(text,count_col);

    Select_Curve(pos);
}
//-----------------------------------------------------------------------------
//--- SelelectCell_MCTable
//-----------------------------------------------------------------------------
void All_Cp::SelelectCell_MCTable(int row, int col)
{
    if(MC_Table->item(row,col)->text().contains("Header_Sample:")) return;

    int pos = 0;
    QString text = MC_Table->verticalHeaderItem(row)->text();
    int count_col,count_row;
    int count_tubes = p_prot->count_Tubes;

    p_prot->Plate.PlateSize(count_tubes, count_row, count_col);
    pos = Convert_NameToIndex(text,count_col);

    //qDebug() << row << col << text << Convert_NameToIndex(text,count_col);

    Select_Curve(pos);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Load_RelativeValidityStructure(QMap<QString, double> &RV, rt_Protocol *p)
{
    int i,j,k,m,l;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_Tube         *tube;
    rt_Channel      *channel;

    POINT_TAKEOFF *sigmoid;
    int id_tube, id_channel;
    int pos, id;
    int active_ch = p->active_Channels;    

    QString name_test;
    QString name;
    double value;

    RV.clear();

    if(p->count_PCR == 0) return;

    for(i=0; i<p->Plate.groups.size(); i++)
    {       
        group = p->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {            
            sample = group->samples.at(j);
            test = sample->p_Test;
            name_test = QString::fromStdString(test->header.Name_Test);            

            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                id_tube = tube->ID_Tube;
                pos = tube->pos;
                for(m=0; m<tube->channels.size(); m++)
                {                    
                    channel = tube->channels.at(m);
                    id_channel = channel->ID_Channel;

                    id = 0;
                    for(l=0; l<=id_channel; l++)
                    {
                       if(active_ch & (0x0f<<4*l)) id++;
                    }

                    id = (id-1)*p->count_Tubes + pos;
                    sigmoid = p->list_PointsOff.at(id);

                    if(!sigmoid->valid) continue;

                    //name = name_test + ":" + FindNameByTest(test, id_tube, id_channel);
                    name = QString("%1:%2:%3:%4").arg(name_test).
                                                  arg(FindNameByTest(test, id_tube, id_channel)).
                                                  arg(id_tube).
                                                  arg(id_channel);
                    value = sigmoid->Fluor_Cp;

                    if(RV.contains(name))
                    {
                        if(value > RV.value(name)) RV.insert(name,value);
                    }
                    else RV.insert(name,value);
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Validity_MC()
{
    int i,j,k,m,l,n;
    int id, pos;
    int id_channel, id_tube;
    double value;
    bool ok;
    QString text;
    QStringList list;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_Tube         *tube;
    rt_Channel      *channel;
    rt_Preference   *preference;

    int fwhm_border, peaks_border;
    QPointF p_mc;

    Criterion_Test  *criterion;
    QMap<rt_Test*, Criterion_Test*> map_criterion;
    int col,row;
    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);
    int active_ch = p_prot->active_Channels;
    int count_tubes = p_prot->count_Tubes;

    if(p_prot->count_MC == 0) return;

    for(i=0; i<p_prot->Criterion.size(); i++)
    {
        criterion = p_prot->Criterion.at(i);
        map_criterion.insert(criterion->ptest, criterion);
        //if(criterion->ptest) qDebug() << "borders: " << criterion->fwhm_border << criterion->peaks_border;
    }

    for(i=0; i<p_prot->Plate.groups.size(); i++)        // Groups
    {
        group = p_prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)          // Samples
        {
            sample = group->samples.at(j);
            test = sample->p_Test;

            criterion = map_criterion.value(test,NULL);
            if(criterion == NULL) continue;

            for(k=0; k<sample->tubes.size(); k++)       // Tubes
            {
                tube = sample->tubes.at(k);
                pos = tube->pos;
                id = 0;
                for(m=0; m<COUNT_CH; m++)
                {
                    if(active_ch & (0x0f<<4*m))
                    {
                        n = (pos + id*count_tubes)*2;

                        // only for first peak
                        p_mc.setX(p_prot->MC_TPeaks.at(n));
                        p_mc.setY(p_prot->MC_TPeaks_Y.at(n));
                        if(p_mc.x() > 0. && p_mc.y() > 0.)
                        {
                            if(!Validate_Peaks(pos + id*count_tubes, p_mc, criterion))        // Validate
                            {
                                p_prot->MC_TPeaks.at(n) = 0.;
                                p_prot->MC_TPeaks_Y.at(n) = 0.;
                            }
                        }

                        /*for(l=0; l<2; l++)
                        {
                            p_mc.setX(p_prot->MC_TPeaks.at(n+l));
                            p_mc.setY(p_prot->MC_TPeaks_Y.at(n+l));

                            if(p_mc.x() > 0. && p_mc.y() > 0.)
                            {
                                if(!Validate_Peaks(pos + id*count_tubes, p_mc, criterion))        // Validate
                                {
                                    p_prot->MC_TPeaks.at(n+l) = 0.;
                                    p_prot->MC_TPeaks_Y.at(n+l) = 0.;
                                }
                            }
                        }*/
                        id++;
                    }
                }
            }
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool All_Cp::Validate_Peaks(int pos, QPointF param, Criterion_Test *criterion)
{
    int id;
    int left_cycle;
    double left_border, right_border;
    double fwhm;
    double tga;
    bool res = true;
    int count_mc = p_prot->count_MC;
    QVector<double> vec_mc = QVector<double>::fromStdVector(p_prot->MC_dF_dT).mid(pos*count_mc,count_mc);

    //qDebug() << "pos: " << pos << param;
    //qDebug() << "vec_mc: " << vec_mc;

    // 1. filter:  peaks_border
    if(param.y() < criterion->peaks_border) return(false);

    // 2. filter: fwhm_border
    left_cycle = (int)(param.x() - p_prot->T_initial)/p_prot->dT_mc;

    if(left_cycle < 4 || left_cycle > count_mc-4) return(true);

    //qDebug() << "vec_mc: " << vec_mc.mid(left_cycle+1, 4) << vec_mc.mid(left_cycle-3,4);
    // left:
    id = left_cycle - 1;
    left_border = id;
    while(id > 0)
    {
        if((vec_mc.at(id) > vec_mc.at(id-1)) && vec_mc.at(id) > param.y()/2.) id--;
        else
        {
            left_border = id;
            tga = (vec_mc.at(id+1) - vec_mc.at(id))/1.;
            left_border += (param.y()/2. - vec_mc.at(id))/tga;
            break;
        }
    }

    // right:
    id = left_cycle + 2;
    right_border = id;
    while(id < count_mc)
    {
        if((vec_mc.at(id) > vec_mc.at(id+1)) && vec_mc.at(id) > param.y()/2.) id++;
        else
        {
            right_border = id;
            tga = (vec_mc.at(id) - vec_mc.at(id+1))/1.;
            right_border -= (param.y()/2. - vec_mc.at(id))/tga;
            break;
        }
    }
    // result:
    fwhm = (right_border - left_border) * p_prot->dT_mc;

    //qDebug() << "fwhm: " << fwhm << criterion->fwhm_border << left_border << right_border << left_cycle;

    if(fwhm/(param.y()/2.) * 100. > criterion->fwhm_border) res = false;


    return(res);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Validate_PCR()
{
    int i,j,k,m,l,n;
    int id, pos;
    int id_channel, id_tube;    
    QString key_Validity;
    QString value_Validity;
    double value;
    double coef_exp;
    QVector<double> Coef_Exp;
    bool ok;
    double val, val_delta;
    QString text;
    QStringList list;
    QString str_result, str_temp;
    QString result_S, result_aFF, result_Kplus;
    double delta = 5;           // +/- %

    bool aFF = true;
    bool K_plus = true;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_Tube         *tube;
    rt_Channel      *channel;
    rt_Preference   *preference;

    Criterion_Test  *criterion;
    POINT_TAKEOFF   *sigmoid;
    QMap<QString,double> RelativeValidity;    

    int active_ch = p_prot->active_Channels;
    int count_pcr = p_prot->count_PCR;    

    int col,row;

    p_prot->Plate.PlateSize(p_prot->count_Tubes, row, col);

    Map_Validity.clear();

    if(p_prot->count_PCR == 0) return;

    //sigmoid = p_prot->list_PointsOff.at(95 + 12);
    //qDebug() << "sigmoid: B1 " << sigmoid->cp_Sigmoid;

    Load_RelativeValidityStructure(RelativeValidity, p_prot);

    for(i=0; i<p_prot->Plate.groups.size(); i++)        // Groups
    {
        group = p_prot->Plate.groups.at(i);        
        for(j=0; j<group->samples.size(); j++)          // Samples
        {
            sample = group->samples.at(j);

            test = sample->p_Test;
            Coef_Exp.clear();
            Coef_Exp.fill(1,COUNT_CH);
            foreach(preference, test->preference_Test)
            {
                if(preference->name == "exposure_Measurements")
                {
                    text = QString::fromStdString(preference->value);
                    list = text.split(QRegExp("\\s+"));
                    id = 0;
                    foreach(text, list)
                    {
                        value = text.toDouble(&ok);
                        if(!ok || value > 10. || value < 0.1) value = 1.;
                        if(id >= Coef_Exp.size()) break;
                        Coef_Exp.replace(id, value);
                        id++;
                    }
                    break;
                }
            }
            //qDebug() << "Coef_Exp: " << Coef_Exp;

            criterion = p_prot->Criterion.at(0);
            for(k=0; k<p_prot->Criterion.size(); k++)
            {
                criterion = p_prot->Criterion.at(k);
                if(criterion->ptest == test) break;
            }

            for(k=0; k<sample->tubes.size(); k++)       // Tubes
            {
                tube = sample->tubes.at(k);
                id_tube = tube->ID_Tube;
                pos = tube->pos;
                //qDebug() << "tubes: " << Convert_IndexToName(pos, col);
                for(m=0; m<tube->channels.size(); m++)  // Channels
                {
                    channel = tube->channels.at(m);
                    id_channel = channel->ID_Channel;
                    //coef_exp = (double)(p_prot->Exposure.at(id_channel))/1000.;     // relative 1000msec = 1sec
                    if(id_channel < Coef_Exp.size()) coef_exp = Coef_Exp.at(id_channel);
                    else coef_exp = 0;

                    id = 0;
                    for(l=0; l<=id_channel; l++)
                    {
                       if(active_ch & (0x0f<<4*l)) id++;
                    }

                    id = (id-1)*p_prot->count_Tubes + pos;
                    sigmoid = p_prot->list_PointsOff.at(id);

                    key_Validity = QString("%1:%2").arg(pos).arg(id_channel);
                    value_Validity = "";
                    aFF = true;
                    K_plus = true;

                    //... 1. Sigmoid validate ...
                    //qDebug() << "channel: " << m;
                    //qDebug() << "Sigmoid validate: " << sigmoid->Criteria << criterion->sigmoid_ValidityMax << criterion->sigmoid_ValidityMin;




                    if(criterion->sigmoid_ValidityMax > 0 && sigmoid->valid)
                    {
                        value = 100. - sigmoid->Criteria * 100./criterion->sigmoid_ValidityMax;
                        value = roundTo(value,0);
                        if(sigmoid->Criteria <= criterion->sigmoid_ValidityMin) str_result = "+";
                        else str_result = "?";
                        str_result +=  " (" + QString::number(value,'f',0) + "%)";
                    }
                    else str_result = "-";
                    result_S = str_result;
                    value_Validity = result_S;
                    str_result = "Sigmoid_validity=" + str_result;                    
                    AddResult(channel->result_Channel, str_result, "Sigmoid_validity=");


                    //... 2. Analysis of Finish Flash (aFF)

                    value = p_prot->PCR_FinishFlash.at(id);
                    //qDebug() << "pos: " << pos << m << value << coef_exp;
                    //qDebug() << "aFF: " << criterion->criterion_AFF << criterion->criterion_AFF_MIN << criterion->criterion_AFF_MAX;

                    if(criterion->use_AFF && criterion->criterion_AFF > 0)  // && value > 0)
                    {
                        str_result = "true";
                        if(sigmoid->valid)
                        {
                            if(criterion->criterion_AFF_MIN <= 0 || coef_exp == 0) str_result = "none";
                            else
                            {
                                val = coef_exp*(double)(criterion->criterion_AFF*criterion->criterion_AFF_MIN)/100.;
                                val_delta = coef_exp*(double)(criterion->criterion_AFF*(criterion->criterion_AFF_MIN + delta)/100.);
                                if(value < (val+val_delta) && value >= val) str_result = "warning";
                                if(value < val) str_result = "false";

                                /*if(pos == 1 && !m)
                                {
                                    qDebug() << "aff: " << criterion->criterion_AFF << criterion->criterion_AFF_MIN << coef_exp << delta;
                                    qDebug() << "result: " << val << val_delta << value << str_result;

                                }*/
                            }
                        }
                        else
                        {
                            if(criterion->criterion_AFF_MAX <= 0 || coef_exp == 0) str_result = "none";
                            else
                            {
                                val = coef_exp*(double)(criterion->criterion_AFF*criterion->criterion_AFF_MAX)/100.;
                                if(value >= val) str_result = "warning";
                            }
                        }
                    }
                    else str_result = "none";
                    result_aFF = str_result;
                    value_Validity += ";" + result_aFF;

                    if(str_result == "false") aFF = false;

                    str_result = "aFF=" + str_result;
                    AddResult(channel->result_Channel, str_result, "aFF=");


                    //... 3. Relative(K+) validity
                    //qDebug() << "K+ " << criterion->criterion_Validity << delta;

                    if(sigmoid->valid && criterion->criterion_Validity > 0)
                    {
                        str_result = "true";
                        //str_temp = QString::fromStdString(test->header.Name_Test) + ":" + FindNameByTest(test, id_tube, id_channel);
                        str_temp = QString("%1:%2:%3:%4").arg(QString::fromStdString(test->header.Name_Test)).
                                                          arg(FindNameByTest(test, id_tube, id_channel)).
                                                          arg(id_tube).
                                                          arg(id_channel);
                        if(RelativeValidity.contains(str_temp))
                        {
                            value = RelativeValidity.value(str_temp);
                            if(value > 0)
                            {

                                val = sigmoid->Fluor_Cp/value * 100.;
                                if(val < (criterion->criterion_Validity+delta) && val >= criterion->criterion_Validity) str_result = "warning";
                                if(val < criterion->criterion_Validity) str_result = "false";
                                //qDebug() << "K+, value: " << value << val;
                            }

                        }
                    }
                    else  str_result = "none";
                    result_Kplus = str_result;
                    value_Validity += ";" + result_Kplus;

                    if(str_result == "false") K_plus = false;

                    str_result = "Relative_validity=" + str_result;
                    AddResult(channel->result_Channel, str_result, "Relative_validity=");


                    Map_Validity.insert(key_Validity, value_Validity);

                    if((!K_plus || !aFF) && sigmoid->valid)     // make PCR negative
                    {
                        // 1. valid
                        sigmoid->valid = false;
                        sigmoid->cp_Sigmoid = 0.;
                        sigmoid->real_ct = 0.;

                        // 2. PCR_Bace
                        id *= count_pcr;    // ID = ch*tubes*meas + pos*meas
                        std::fill(p_prot->PCR_Bace.begin()+id, p_prot->PCR_Bace.begin()+id+count_pcr, 1.);
                        std::fill(p_prot->NormCp_Value.begin()+id, p_prot->NormCp_Value.begin()+id+count_pcr, 1.);
                        std::fill(p_prot->NormCt_Value.begin()+id, p_prot->NormCt_Value.begin()+id+count_pcr, 1.);
                    }
                }
            }
        }
    }
    RelativeValidity.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Fill_Cp_Table_Only_Ct()
{
    int i,j,k,n,l;
    QString text, str_result;
    int num_row = 0;
    int count_ch;
    bool header_sample;
    int id, id_channel;
    int active_ch = p_prot->active_Channels;
    double value;
    QTableWidgetItem *item;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_Tube         *tube;
    rt_Channel      *channel;

    POINT_TAKEOFF *sigmoid;

    Cp_Table->blockSignals(true);

    //... cycle of groups ...
    for(i=0; i<p_prot->Plate.groups.size(); i++)
    {
        group = p_prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            header_sample = false;
            foreach(rt_Tube *t, sample->tubes)
            {
                if(p_prot->enable_tube.at(t->pos))
                {
                    num_row++;
                    header_sample = true;
                    break;
                }
            }
            if(!header_sample) continue;


            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                if(!p_prot->enable_tube.at(tube->pos)) continue;

                count_ch = tube->channels.size();

                text = "";
                for(n=0; n<count_ch; n++)
                {
                    if(n) text += "\r\n";

                    channel = tube->channels.at(n);
                    id_channel = channel->ID_Channel;

                    id = 0;
                    for(l=0; l<=id_channel; l++)
                    {
                       if(active_ch & (0x0f<<4*l)) id++;
                    }

                    id = (id-1)*p_prot->count_Tubes + tube->pos;

                    //qDebug() << "channel: " << id << id_channel << channel->result_Channel.size();


                    sigmoid = p_prot->list_PointsOff.at(id);

                    value = sigmoid->real_ct;
                    if(value > 0) str_result = QString::number(value,'f',1);
                    else str_result = "-";
                    if(value < -1) str_result = QString::number(value,'f',1);
                    text += str_result;
                    str_result = "Ct=" + QString::number(value,'f',3);
                    AddResult(channel->result_Channel, str_result, "Ct=");

                    item = Cp_Table->item(num_row,2);
                    item->setText(text);

                }
                num_row++;
            }
        }
    }



    Cp_Table->blockSignals(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Fill_Cp_Table()
{
    int i,j,k,m,n,l;
    int count;
    QString text, str_result, str_temp;
    QStringList header;
    QTableWidgetItem *newItem;
    int num_row=0;
    int count_ch;
    int heigth;
    int pos;
    int col,row;
    int id_channel, id_tube;
    QString id_Group, id_Sample;
    QString key_Validity;
    QString value_Validity;
    QStringList list_Validity;
    int id;
    POINT_TAKEOFF *sigmoid;
    SPLINE_CP *spline_Cp;
    double value;
    short value_short;
    int active_ch = p_prot->active_Channels;
    bool header_visible;
    double coef_exp;
    QString name_str;

    if(p_prot->count_PCR == 0) return;    

    p_prot->Plate.PlateSize(p_prot->count_Tubes,row,col);

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_Tube         *tube;
    rt_Channel      *channel;

    Criterion_Test *criterion;
    QMap<QString,double> RelativeValidity;
    Load_RelativeValidityStructure(RelativeValidity, p_prot);

    //...

    Cp_Table->blockSignals(true);

    for(i = Cp_Table->rowCount(); i>0; i--) Cp_Table->removeRow(i-1);
    header << tr("Identificator") << "Cp" << "Ct" << "S(%)" << "aFF" << tr("K+(%)"); // << "Cp_s";

    Cp_Table->clear();
    //Cp_Table->setColumnCount(7);
    Cp_Table->setColumnCount(6);
    Cp_Table->setHorizontalHeaderLabels(header);
    Cp_Table->setColumnWidth(1,60);
    Cp_Table->setColumnWidth(2,50);
    Cp_Table->setColumnWidth(3,70);
    Cp_Table->setColumnWidth(4,50);
    Cp_Table->setColumnWidth(5,50);
    //Cp_Table->setColumnWidth(6,50);
    Cp_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    Cp_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    Cp_Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    Cp_Table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    Cp_Table->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);
    //Cp_Table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Fixed);
    Cp_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    if(!flag_AddAnalysis)
    {
        for(i=3; i<Cp_Table->columnCount(); i++) Cp_Table->setColumnWidth(i,0);
    }

    //... count rows ...

    //qDebug() << "count groups = " << p_prot->Plate.groups.size();

    count = p_prot->count_Tubes;
    for(i=0; i<p_prot->Plate.groups.size(); i++)
    {
        group = p_prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            count += sample->tubes.size();
        }
    }
    Cp_Table->setRowCount(count);
    header.clear();

    //... cycle of groups ...
    for(i=0; i<p_prot->Plate.groups.size(); i++)
    {
        group = p_prot->Plate.groups.at(i);
        id_Group = QString::fromStdString(group->ID_Group);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            id_Sample = QString::fromStdString(sample->ID_Sample);            
            // check for visible header
                header_visible = false;
                for(k=0; k<sample->tubes.size(); k++)
                {
                    tube = sample->tubes.at(k);
                    pos = tube->pos;
                    if(p_prot->enable_tube.at(pos)) {header_visible = true; break;}
                }
                if(!header_visible) continue;
            //...


            test = sample->p_Test;
            criterion = p_prot->Criterion.at(0);
            for(k=0; k<p_prot->Criterion.size(); k++)
            {
                criterion = p_prot->Criterion.at(k);
                if(criterion->ptest == test) break;
            }
            //criterion->criterion_AFF = 20;
            //criterion->criterion_Validity = 10;

            //... Header ...
            Cp_Table->setRowHeight(num_row, 30);
            for(k=0; k<Cp_Table->columnCount(); k++)
            {
                newItem = new QTableWidgetItem();
                text = "Header_Sample:";
                if(!k)
                {
                    text += QString::fromStdString(sample->Unique_NameSample);
                    name_str = QString::fromStdString(test->header.Name_Test);
                    if(Map_TestTranslate.size()) name_str = Map_TestTranslate.value(name_str, name_str);
                    text += "  (" + name_str + ")";
                }
                newItem->setText(text);
                newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                Cp_Table->setItem(num_row,k,newItem);
            }

            num_row++;

            header.append(" ");

            //... Tubes ...
            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                id_tube = tube->ID_Tube;

                //... 1. position of tube ...
                pos = tube->pos;
                if(!p_prot->enable_tube.at(pos)) continue;
                str_temp = Convert_IndexToName(pos,col);
                header.append(QString("%1  ").arg(str_temp));

                //qDebug() << "header:" << header;

                //... 2. count active channel in tube ...
                count_ch = tube->channels.size();
                heigth = 22;
                if(count_ch > 1) Cp_Table->setRowHeight(num_row, heigth*0.9*count_ch);
                else Cp_Table->setRowHeight(num_row, heigth);

                //... 3. load data ...
                for(m=0; m<Cp_Table->columnCount(); m++)
                {
                    text = "";
                    for(n=0; n<count_ch; n++)
                    {
                        if(n) text += "\r\n";

                        channel = tube->channels.at(n);
                        id_channel = channel->ID_Channel;

                        id = 0;
                        for(l=0; l<=id_channel; l++)
                        {
                           if(active_ch & (0x0f<<4*l)) id++;
                        }

                        id = (id-1)*p_prot->count_Tubes + pos;

                        //qDebug() << "channel: " << id << id_channel << channel->result_Channel.size();


                        sigmoid = p_prot->list_PointsOff.at(id);

                        spline_Cp = p_prot->list_SplineCp.at(id);

                        //coef_exp = (double)(p_prot->Exposure.at(id_channel))/1000.;

                        key_Validity = QString("%1:%2").arg(pos).arg(id_channel);
                        value_Validity = Map_Validity.value(key_Validity, " ; ; ");
                        list_Validity.clear();
                        list_Validity = value_Validity.split(";");

                        switch(m)
                        {
                        case 0: // Identificator
                                    str_result = FindNameByTest(test,id_tube,id_channel);
                                    name_str = str_result;
                                    if(Map_TestTranslate.size()) name_str = Map_TestTranslate.value(name_str, name_str);
                                    text += QString::number(id_channel) + " " + name_str;
                                            //QString::fromStdString(channel->Unique_NameChannel);
                                            //FindNameByTest(test,id_tube,id_channel);
                                    channel->Unique_NameChannel = str_result.toStdString();
                                    break;

                        case 1: // Cp                                    
                                    value = sigmoid->cp_Sigmoid;                                    
                                    if(value > 0) str_result = QString::number(value,'f',1);
                                    else str_result = "-";
                                    if(value < -1) str_result = QString::number(value,'f',1);
                                    text += str_result;
                                    str_result = "Cp=" + QString::number(value,'f',3);
                                    AddResult(channel->result_Channel, str_result, "Cp=");
                                    //channel->result_Channel.push_back(str_result.toStdString());
                                    //qDebug() << channel->result_Channel.size();
                                    break;

                        case 2: // Ct
                                    value = sigmoid->real_ct;                                    
                                    if(value > 0) str_result = QString::number(value,'f',1);
                                    else str_result = "-";
                                    if(value < -1) str_result = QString::number(value,'f',1);
                                    text += str_result;
                                    str_result = "Ct=" + QString::number(value,'f',3);
                                    AddResult(channel->result_Channel, str_result, "Ct=");                                    
                                    break;

                        case 3: // sigmoid validity
                                    /*if(criterion->sigmoid_ValidityMax > 0 && sigmoid->valid)
                                    {
                                        value = 100. - sigmoid->Criteria * 100./criterion->sigmoid_ValidityMax;
                                        value = roundTo(value,0);

                                        if(sigmoid->Criteria <= criterion->sigmoid_ValidityMin) str_result = "+";
                                        else str_result = "?";
                                        str_result +=  " (" + QString::number(value,'f',0) + "%)";
                                    }
                                    else str_result = "-";*/

                                    str_result = list_Validity.at(0);
                                    text += str_result;

                                    //qDebug() << "sigmoid validity: " << str_result;
                                    //str_result = "Sigmoid_validity=" + str_result;
                                    //AddResult(channel->result_Channel, str_result, "Sigmoid_validity=");
                                    break;

                        case 4: // analysis of Finish Flash (aFF)
                                    /*value_short = p_prot->PCR_FinishFlash.at(id);
                                    if(criterion->use_AFF && criterion->criterion_AFF > 0 && value_short > 0)
                                    {
                                        str_result = "ok";                                        
                                        if(sigmoid->valid)
                                        {
                                            if(criterion->criterion_AFF_MIN <= 0 || coef_exp == 0) str_result = "none";
                                            else
                                            {
                                                if(value_short < coef_exp*criterion->criterion_AFF*criterion->criterion_AFF_MIN/100) str_result = "warning";
                                            }
                                        }
                                        else
                                        {
                                            if(criterion->criterion_AFF_MAX <= 0 || coef_exp == 0) str_result = "none";
                                            else
                                            {
                                                if(value_short >= coef_exp*criterion->criterion_AFF*criterion->criterion_AFF_MAX/100) str_result = "warning";
                                            }
                                        }
                                    }
                                    else
                                    {
                                        str_result = "none";

                                    }*/

                                    str_result = list_Validity.at(1);
                                    text += str_result;
                                    //qDebug() << "aFF: " << str_result;

                                    //str_result = "aFF=" + str_result;
                                    //AddResult(channel->result_Channel, str_result, "aFF=");
                                    break;

                        case 5:     // Relative(K+) validity

                                    /*if(sigmoid->valid && criterion->criterion_Validity > 0)
                                    {
                                        str_result = "ok";
                                        str_temp = QString::fromStdString(test->header.Name_Test) + ":" + FindNameByTest(test, id_tube, id_channel);
                                        if(RelativeValidity.contains(str_temp))
                                        {
                                            value = RelativeValidity.value(str_temp);
                                            if(value > 0)
                                            {
                                                if(sigmoid->Fluor_Cp/value * 100. < criterion->criterion_Validity) str_result = "warning";
                                            }
                                        }
                                    }
                                    else  str_result = "none";*/

                                    str_result = list_Validity.at(2);
                                    text += str_result;
                                    //qDebug() << "K+: " << str_result;

                                    //str_result = "Relative_validity=" + str_result;
                                    //AddResult(channel->result_Channel, str_result, "Relative_validity=");
                                    break;

                        case 6:     // Cp (Spline)
                                    break;

                                    value = spline_Cp->Cp;
                                    if(value > 0) str_result = QString::number(value,'f',1);
                                    else str_result = "-";
                                    text += str_result;
                                    str_result = "Cp_Spline=" + QString::number(value,'f',1);
                                    AddResult(channel->result_Channel, str_result, "Cp_Spline=");

                                    break;

                        default:    text = "";  break;

                        }

                    }
                    newItem = new QTableWidgetItem();
                    newItem->setText(text);
                    newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                    Cp_Table->setItem(num_row,m,newItem);
                }
                num_row++;
            }
        }
    }

    RelativeValidity.clear();

    Cp_Table->setRowCount(num_row);
    Cp_Table->setVerticalHeaderLabels(header);
    Cp_Table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);


    //Cp_Table->resizeColumnToContents(1);
    //Cp_Table->resizeColumnToContents(2);

    Cp_Table->blockSignals(false);

    Cp_Table->resizeRowsToContents();


}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void All_Cp::Fill_MC_Table()
{
    int i,j,k,m,n,l;
    int count;
    QString text, str_result, str_result_add;
    QStringList header;
    QTableWidgetItem *newItem;
    int num_row=0;
    int count_ch;
    int heigth;
    int pos;
    int col,row;
    int id_channel, id_tube;
    int id;    
    double value;
    bool header_visible;
    int active_ch = p_prot->active_Channels;
    QString name_str;

    if(p_prot->count_MC == 0) return;

    p_prot->Plate.PlateSize(p_prot->count_Tubes,row,col);

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Test         *test;
    rt_Tube         *tube;
    rt_Channel      *channel;

    MC_Table->blockSignals(true);

    for(i = MC_Table->rowCount(); i>0; i--) MC_Table->removeRow(i-1);
    header << tr("Identificator") << tr("Temperature Peaks") << tr("Height of peaks");

    MC_Table->clear();
    MC_Table->setColumnCount(3);
    MC_Table->setHorizontalHeaderLabels(header);
    MC_Table->setColumnWidth(1,170);
    MC_Table->setColumnWidth(2,170);
    MC_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    MC_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    MC_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);


    //... count rows ...

    //qDebug() << "count groups = " << p_prot->Plate.groups.size();

    count = p_prot->count_Tubes;
    for(i=0; i<p_prot->Plate.groups.size(); i++)
    {
        group = p_prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            count += sample->tubes.size();
        }
    }
    MC_Table->setRowCount(count);
    header.clear();

    //... cycle of groups ...
    for(i=0; i<p_prot->Plate.groups.size(); i++)
    {
        group = p_prot->Plate.groups.at(i);
        for(j=0; j<group->samples.size(); j++)
        {
            sample = group->samples.at(j);
            test = sample->p_Test;

            // check for visible header
                header_visible = false;
                for(k=0; k<sample->tubes.size(); k++)
                {
                    tube = sample->tubes.at(k);
                    pos = tube->pos;
                    if(p_prot->enable_tube.at(pos)) {header_visible = true; break;}
                }
                if(!header_visible) continue;
            //...

            //... Header ...
            MC_Table->setRowHeight(num_row, 30);
            for(k=0; k<MC_Table->columnCount(); k++)
            {
                newItem = new QTableWidgetItem();
                text = "Header_Sample:";
                if(!k)
                {
                    text += QString::fromStdString(sample->Unique_NameSample);
                    name_str = QString::fromStdString(test->header.Name_Test);
                    if(Map_TestTranslate.size()) name_str = Map_TestTranslate.value(name_str, name_str);
                    text += "  (" + name_str + ")";
                }
                newItem->setText(text);
                newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                MC_Table->setItem(num_row,k,newItem);
            }

            num_row++;

            header.append(" ");

            //... Tubes ...
            for(k=0; k<sample->tubes.size(); k++)
            {
                tube = sample->tubes.at(k);
                id_tube = tube->ID_Tube;

                //... 1. position of tube ...
                pos = tube->pos;
                if(!p_prot->enable_tube.at(pos)) continue;
                header.append(QString("%1  ").arg(Convert_IndexToName(pos,col)));

                //... 2. count active channel in tube ...
                count_ch = tube->channels.size();
                heigth = 22;
                if(count_ch > 1) MC_Table->setRowHeight(num_row, heigth*0.9*count_ch);
                else MC_Table->setRowHeight(num_row, heigth);

                //... 3. load data ...
                for(m=0; m<MC_Table->columnCount(); m++)
                {
                    text = "";
                    for(n=0; n<count_ch; n++)
                    {
                        if(n) text += "\r\n";

                        channel = tube->channels.at(n);
                        id_channel = channel->ID_Channel;

                        id = 0;
                        for(l=0; l<=id_channel; l++)
                        {
                           if(active_ch & (0x0f<<4*l)) id++;
                        }

                        id = (id-1)*p_prot->count_Tubes + pos;

                        switch(m)
                        {
                        case 0: // Identificator
                                    text += QString::number(id_channel) + " ";
                                    name_str = FindNameByTest(test,id_tube,id_channel);
                                    if(Map_TestTranslate.size()) name_str = Map_TestTranslate.value(name_str, name_str);
                                    text += name_str;

                                    break;

                        case 1: // Peaks

                                    //qDebug() << "MC_TPeaks" << id << p_prot->MC_TPeaks.at(id*2) << p_prot->MC_TPeaks.at(id*2+1);

                                    str_result = "";
                                    str_result_add = "";
                                    value = p_prot->MC_TPeaks.at(id*2);
                                    if(value > 0)
                                    {
                                        str_result = QString("%1").arg(value,5,'f',2);
                                        text += str_result;
                                    }
                                    value = p_prot->MC_TPeaks.at(id*2+1);
                                    if(value > 0)
                                    {
                                        str_result_add = QString(" (%1)").arg(value,5,'f',2);
                                        text += str_result_add;
                                    }
                                    if(str_result.isEmpty()) text += "-";


                                    str_result = "mc_peaks=" + str_result; // + str_result_add;  // only first peak
                                    AddResult(channel->result_Channel, str_result, "mc_peaks=");

                                    //channel->result_Channel.push_back(str_result.toStdString());
                                    break;

                        case 2: // Peaks: Y-value

                                    str_result = "";
                                    str_result_add = "";
                                    value = p_prot->MC_TPeaks_Y.at(id*2);
                                    if(value > 0)
                                    {
                                        str_result = QString("%1").arg(value,2,'f',0);
                                        text += str_result;
                                    }
                                    value = p_prot->MC_TPeaks_Y.at(id*2+1);
                                    if(value > 0)
                                    {
                                        str_result_add = QString(" (%1)").arg(value,2,'f',0);
                                        text += str_result_add;
                                    }
                                    if(str_result.isEmpty()) text += "-";

                                    str_result = "mc_amplitude=" + str_result;
                                    AddResult(channel->result_Channel, str_result, "mc_amplitude=");

                                    break;
                        }

                    }
                    newItem = new QTableWidgetItem();
                    newItem->setText(text);
                    newItem->setFlags(newItem->flags() & ~Qt::ItemIsEditable);
                    MC_Table->setItem(num_row,m,newItem);
                }
                num_row++;
            }
        }
    }

    MC_Table->setRowCount(num_row);
    MC_Table->setVerticalHeaderLabels(header);
    MC_Table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    MC_Table->blockSignals(false);

    MC_Table->resizeRowsToContents();
}

//-----------------------------------------------------------------------------
//--- AddResult(vector<string>&, QString, QString)
//-----------------------------------------------------------------------------
void All_Cp::AddResult(vector<string> &vec, QString result, QString ref)
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
//--- Create_TranslateList(rt_Test *, QStringList &)
//-----------------------------------------------------------------------------
void All_Cp::Create_TranslateList(rt_Test *p_test, QStringList &list)
{
    QString text;
    rt_TubeTest     *tube_test;
    rt_ChannelTest  *channel_test;

    // 1. name test
    text = QString::fromStdString(p_test->header.Name_Test);
    if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text)) list.append(text);

    // 2. name research
    foreach(tube_test, p_test->tubes)
    {
        foreach(channel_test, tube_test->channels)
        {
            text = QString::fromStdString(channel_test->name);
            if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text))
            {
                list.append(text);
            }
        }
    }
    // 3. name Catalog
    text = QString::fromStdString(p_test->header.Catalog);
    if(text.contains(QRegularExpression("[А-Яа-я]+")) && !list.contains(text)) list.append(text);
}
//-----------------------------------------------------------------------------
//--- Translate_Tests
//-----------------------------------------------------------------------------
void All_Cp::Translate_Tests(QStringList &list)
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

