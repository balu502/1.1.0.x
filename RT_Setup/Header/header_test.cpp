#include "header_test.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Header_Test::Header_Test(QWidget *parent):
    QTreeWidget(parent)
{
    int i;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QString fluor_name[COUNT_CH] = FLUOR_NAME;
    Qt::CheckState state;

    readCommonSettings();
    setFont(qApp->font());
    this->header()->setFont(qApp->font());

    setStyleSheet(
                "QPushButton {min-width: 2em; margin:1; padding:0;}"
                );

    // edit buttons    
    edit_VolumeTube = new QPushButton("...");
    edit_VolumeTube->setFixedSize(30,18);
    edit_VolumeTube->setToolTip(tr("edit volume"));
    edit_Comments = new QPushButton("...");
    edit_Comments->setFixedSize(30,18);
    edit_Comments->setToolTip(tr("edit comments"));

    connect(edit_VolumeTube, SIGNAL(clicked()), this, SLOT(Edit_Volume()));
    connect(edit_Comments, SIGNAL(clicked()), this, SLOT(Edit_Comments()));

    header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1,QHeaderView::ResizeToContents);
    header_Delegate = new HeaderTestDelegate(this);
    setItemDelegate(header_Delegate);
    header_Delegate->style = StyleApp;
    header_Delegate->method_analysis = &method_analysis;

    setColumnCount(3);
    QStringList pp;
    pp << tr("Name") << tr("Value") << tr("Edit");
    setHeaderLabels(pp);

    // 1. Name of Test
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Name of Test"));
    item->setText(1, "");

    // 2. ID Test
    //item = new QTreeWidgetItem(this);
    //item->setText(0, tr("ID Test"));
    //item->setText(1, "");


    // 3. Name of Program
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Name of Program"));
    item->setText(1, "");

    // 4. Type of Analysis
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Type of Analysis"));
    item->setText(1, "0x00");

    // 5. Active channelc
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Active Channels"));
    item->setText(1, "0x00011");
    active_Channels = 0x00011;
    //item->setFlags(item->flags() | Qt::ItemIsEditable);

    state = Qt::Checked; // 2
    for(i=0; i<COUNT_CH; i++)
    {
        if(i > 1) state = Qt::Unchecked; // 0
        item_child = new QTreeWidgetItem(item);
        item_child->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item_child->setCheckState(1, state);
        item_child->setText(1, fluor_name[i]);
    }
    item->setExpanded(true);


    // 6. Volume tube

    item = new QTreeWidgetItem(this);
    //item->setText(0, tr("Volume tube, (mcl)"));
    item->setText(0, tr("Volume tube, ") + "(" + QString::fromUtf8("\u03BC") + "l)");
    item->setText(1, "35");
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    setItemWidget(item, 2, edit_VolumeTube);


    // 7. Comments
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Comments"));
    item->setText(1, "");
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    setItemWidget(item, 2, edit_Comments);


    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(Edit_ActiveChannels(QTreeWidgetItem*,int)));
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Header_Test::~Header_Test()
{
    clear();    
    delete edit_VolumeTube;
    delete edit_Comments;
    delete header_Delegate;
}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Header_Test::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/header_test_" + text + ".qm"))
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
//--- SetDisable_ActiveChannels()
//-----------------------------------------------------------------------------
void Header_Test::SetDisable_ActiveChannels()
{
    int i;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;

    item = topLevelItem(3);
    for(i=0; i<COUNT_CH; i++)
    {
        item_child = item->child(i);
        item_child->setDisabled(true);
    }
    item->setDisabled(true);
}
//-----------------------------------------------------------------------------
//--- Set_ActiveChannels(int act_ch)
//-----------------------------------------------------------------------------
void Header_Test::Set_ActiveChannels(int act_ch)
{
    int i;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    Qt::CheckState state;

    item = topLevelItem(3);
    for(i=0; i<COUNT_CH; i++)
    {
        item_child = item->child(i);
        if(act_ch & (0x0f << 4*i)) state = Qt::Checked;
        else state = Qt::Unchecked;
        item_child->setCheckState(1, state);
    }
    emit Change_ActiveChannels(act_ch);
    repaint();
}

//-----------------------------------------------------------------------------
//--- Edit_ActiveChannels()
//-----------------------------------------------------------------------------
void Header_Test::Load_XML(QDomNode &node)
{
    int i,j;
    int value;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    rt_Protocol *prot_temp;
    QDomNode child;
    QString text;
    QStringList list;
    bool ok;
    Qt::CheckState state;

    blockSignals(true);

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);

        if(child.nodeName() == "nameTest")
        {item = topLevelItem(0); item->setText(1, child.toElement().text()); continue;}
        if(child.nodeName() == "IDTest")
        //{item = topLevelItem(1); item->setText(1, child.toElement().text()); continue;}
        {test_ID = child.toElement().text(); continue;}
        if(child.nodeName() == "method")
        {
            item = topLevelItem(2);
            value = child.toElement().text().toInt(&ok,10);
            item->setText(1, QString("0x%1").arg(value,1,16));
            //item->setText(1, method_analysis);
            continue;
        }

        if(child.nodeName() == "activeChannels")
        {
            item = topLevelItem(3);
            value = child.toElement().text().toInt(&ok,16);            
            item->setText(1, QString("0x%1").arg(value,1,16));

            active_Channels = value;    // Active Channels

            for(j=0; j<COUNT_CH; j++)
            {
                item_child = item->child(j);
                if(value & (0x0f<<j*4)) state = Qt::Checked;
                else state = Qt::Unchecked;
                item_child->setCheckState(1, state);
            }
            continue;
        }
        if(child.nodeName() == "volumeTube")
        {
            item = topLevelItem(4);
            value = child.toElement().text().toInt(&ok);
            if(value > max_Vol || value < min_Vol) value = default_Vol;
            item->setText(1, QString("%1").arg(value));            
            continue;
        }
        if(child.nodeName() == "comment") {item = topLevelItem(5); item->setText(1, child.toElement().text()); continue;}

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
            item = topLevelItem(1);
            item->setText(1, QString::fromStdString(prot_temp->PrAmpl_name));
            prot_temp->Clear_Protocol();
            delete prot_temp;

            continue;
        }
    }

    blockSignals(false);
}
//-----------------------------------------------------------------------------
//--- Fill_Properties()
//-----------------------------------------------------------------------------
void Header_Test::Fill_Properties(rt_Test *ptest)
{
    int i;
    QTreeWidgetItem *item;
    QString text;
    bool ok;

    for(i=0; i<topLevelItemCount(); i++)
    {
        item = topLevelItem(i);
        text = item->text(1);
        //qDebug() << "item: " << i << text;
        switch(i)
        {
        case 0: ptest->header.Name_Test = text.toStdString();   break;
        //case 1: ptest->header.ID_Test = text.toStdString();     break;
        case 1:                                                 break;
        case 2: ptest->header.Type_analysis = text.toInt(&ok,16);   break;
        case 3: ptest->header.Active_channel = text.toInt(&ok,16);  break;
        case 4: ptest->header.Volume_Tube = text.toInt(&ok,10); break;
        case 5: ptest->header.comment = text.toStdString();     break;
        default:    break;
        }
    }
    ptest->header.ID_Test = test_ID.toStdString();    
}
//-----------------------------------------------------------------------------
//--- Change_NameProgramm()
//-----------------------------------------------------------------------------
void Header_Test::Change_NameProgramm(QString name)
{
    blockSignals(true);
    QTreeWidgetItem *item = topLevelItem(1);
    item->setText(1, name);
    blockSignals(false);
}
//-----------------------------------------------------------------------------
//--- Load_NewTest()
//-----------------------------------------------------------------------------
void Header_Test::Load_NewTest(QString name, QString ID, int method)
{
    QTreeWidgetItem *item;

    blockSignals(true);
    item = topLevelItem(0); item->setText(1, name);
    //item = topLevelItem(1); item->setText(1, ID);
    test_ID = ID;

    item = topLevelItem(2); item->setText(1, QString("0x%1").arg(method,0,16));
    blockSignals(false);
}


//-----------------------------------------------------------------------------
//--- Edit_Volume()
//-----------------------------------------------------------------------------
void Header_Test::Edit_Volume()
{
    setFocus();
    edit(currentIndex().model()->index(4,1));
    edit_VolumeTube->clearFocus();
}
//-----------------------------------------------------------------------------
//--- Edit_Comments()
//-----------------------------------------------------------------------------
void Header_Test::Edit_Comments()
{    
    setFocus();
    edit(currentIndex().model()->index(5,1));
    edit_Comments->clearFocus();    
}
//-----------------------------------------------------------------------------
//--- Edit_Comments()
//-----------------------------------------------------------------------------
void Header_Test::Edit_ActiveChannels(QTreeWidgetItem *item, int col)
{
    int index;
    QTreeWidgetItem *item_ActiveCH;
    Qt::CheckState state;
    bool sts = false;
    bool ok;
    int value;

    blockSignals(true);
    if(item->parent())
    {
        index = item->parent()->indexOfChild(item);
        state = item->checkState(col);
        setCurrentItem(item);

        if(state) active_Channels |= (1 << index*4);
        else      active_Channels ^= (1 << index*4);

        item_ActiveCH = topLevelItem(3);
        item_ActiveCH->setText(1, QString("0x%1").arg(active_Channels,1,16));
        sts = true;
    }
    else
    {        
        if(item == topLevelItem(4)) // Volume tube
        {
            value = item->text(1).toInt(&ok);
            if(value < min_Vol || value > max_Vol)
            {
                value = default_Vol;    // 25
                item->setText(1, QString::number(value));
            }            
        }
    }
    blockSignals(false);

    if(sts) emit Change_ActiveChannels(active_Channels);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HeaderTestDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString text;
    int active_ch;
    bool ok;
    int i;
    int y;

    painter->setFont(qApp->font());
    QPixmap pixmap(":/images/fam.png");
    QColor color_bg;
    QRect rect = option.rect;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus

    QStyledItemDelegate::paint(painter, viewOption, index);


    if(index.row() < 3 && index.column() == 1 && !index.parent().isValid())
    {
        text = index.data().toString();
        painter->setPen(QColor(Qt::gray));
        color_bg = painter->background().color();
        painter->fillRect(rect, color_bg);
        if(index.row() == 2) text = *method_analysis;
        painter->drawText(rect, Qt::AlignLeft, text);
    }

    if(index.row() == 3 && index.column() == 1 && !index.parent().isValid())
    {
        text = index.data().toString();
        text = text.replace(" ","").trimmed();
        active_ch = text.toInt(&ok, 16);

        painter->fillRect(option.rect, painter->background().color());
        if(viewOption.state & QStyle::State_Selected)
        {
           // painter->fillRect(option.rect, Qt::red);
        }

        for(i=0; i<COUNT_CH; i++)
        {
            if(active_ch & (0xf<<4*i))
            {
                if(style == "fusion")
                {
                    switch(i)
                    {
                        case 0: pixmap.load(":/images/flat/fam_pro.png");    break;
                        case 1: pixmap.load(":/images/flat/hex_pro.png");    break;
                        case 2: pixmap.load(":/images/flat/rox_pro.png");    break;
                        case 3: pixmap.load(":/images/flat/cy5_pro.png");    break;
                        case 4: pixmap.load(":/images/flat/cy55_pro.png");   break;
                        default: pixmap.load(":/images/flat/disable_pro.png");   break;
                    }
                }
                else
                {
                    switch(i)
                    {
                        case 0: pixmap.load(":/images/fam.png");    break;
                        case 1: pixmap.load(":/images/hex.png");    break;
                        case 2: pixmap.load(":/images/rox.png");    break;
                        case 3: pixmap.load(":/images/cy5.png");    break;
                        case 4: pixmap.load(":/images/cy55.png");   break;
                        default: pixmap.load(":/images/disable.png");   break;
                    }
                }
            }
            else
            {
                if(style == "fusion") pixmap.load(":/images/flat/disable_pro.png");
                else pixmap.load(":/images/disable.png");
            }

            y = option.rect.y() + 2;
            if(style == "fusion") y = option.rect.y();
            painter->drawPixmap(option.rect.x() + 13*i /*+ 50*/, y, pixmap);
        }
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget* HeaderTestDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget* obj;
    //return QStyledItemDelegate::createEditor(parent, option, index);


    if(index.column() == 1) // && (index.row() == 5 || index.row() ==6))
    {
        obj = QStyledItemDelegate::createEditor(parent, option, index);
        obj->setStyleSheet("QLineEdit {selection-background-color: rgb(215,215,255); selection-color: black;}");
        return(obj);
        //return(new QLineEdit(parent));
        //return(new QTextEdit(parent));
    }
    return nullptr;

}
