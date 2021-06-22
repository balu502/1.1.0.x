#include "standarts.h"

//-----------------------------------------------------------------------------
extern "C" STANDARTSSHARED_EXPORT HWND __stdcall cr_Standarts(HWND handle, HWND parent_widget, char* Pro, int act_ch)
{
    QString xml_text(Pro);

    RECT rect;
    GetClientRect(handle, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    QWidget *pw = QWidget::find((WId)parent_widget);

    Standarts* p = new Standarts(pw);
    p->setWindowFlags(Qt::ToolTip);
    HWND handle_standarts = (HWND)p->winId();

    p->Units_Box->setVisible(false);
    p->label_unit->setVisible(false);

    p->Set_ActiveCh(act_ch);
    //qDebug() << "Set_ActiveCh(act_ch): " << act_ch;

    if(!xml_text.isEmpty()) p->Read_XML(xml_text);
    ((Standarts*)p)->show();

    ::SetParent(handle_standarts, handle);
    ::SetWindowPos(handle_standarts, HWND_TOP, 0,0, width, height, SWP_SHOWWINDOW);

    return(handle_standarts);
}
//-----------------------------------------------------------------------------
extern "C" STANDARTSSHARED_EXPORT void __stdcall delete_Standarts(HWND handle)
{
    QWidget *p = QWidget::find((WId)handle);
    delete (Standarts*)p;
    return;
}
//-----------------------------------------------------------------------------
extern "C" STANDARTSSHARED_EXPORT void __stdcall resize_Standarts(HWND handle)
{
    HWND handle_parent;
    RECT rect;
    int width, height;

    QWidget *p = QWidget::find((WId)handle);
    if(p)
    {
        handle_parent = ::GetAncestor(handle, GA_PARENT);

        //qDebug() << "handle_parent: " << handle_parent;

        if(handle_parent)
        {
            GetClientRect(handle_parent, &rect);
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
            ::SetWindowPos(handle, HWND_TOP, 0,0, width, height, SWP_SHOWWINDOW);

            //qDebug() << "rect: " << width << height;
        }
    }
}
//-----------------------------------------------------------------------------
extern "C" STANDARTSSHARED_EXPORT void __stdcall write_Standarts(HWND handle, char** p_out)
{
    char *out;
    int leng;
    QString text_out;

    Standarts* p = (Standarts*)QWidget::find((WId)handle);

    p->Write_XML(text_out);

    //qDebug() << "text_out: " << text_out;

    leng = text_out.length();
    out = new char[leng+1];
    *p_out = out;

    memcpy(out, text_out.toUtf8().data(), leng+1);

}
//-----------------------------------------------------------------------------
extern "C" STANDARTSSHARED_EXPORT void __stdcall Standarts_ActiveCh(HWND handle, int act_ch)
{

    Standarts* p = (Standarts*)QWidget::find((WId)handle);

    //qDebug() << "Standarts_ActiveCh: " << act_ch;
    p->Set_ActiveCh(act_ch);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Standarts::Standarts(QWidget *parent):
    QGroupBox(parent)
{
    QStringList header;
    QPalette palette;

    readCommonSettings();

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);
    QHBoxLayout *layout_count = new QHBoxLayout();

    active_ch = 0xf;

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

    Standarts_Table = new QTableWidget(0,5);
    Standarts_Table->setSelectionBehavior(QAbstractItemView::SelectItems);
    Standarts_Table->setSelectionMode(QAbstractItemView::SingleSelection);
    Standarts_Table->verticalHeader()->setVisible(false);
    Standarts_Table->setColumnWidth(0,30);
    Standarts_Table->setColumnWidth(1,15);
    Standarts_Table->setColumnWidth(2,70);
    Standarts_Table->setColumnWidth(4,300);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    Standarts_Table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    standarts_Delegate = new Standarts1ItemDelegate();
    Standarts_Table->setItemDelegate(standarts_Delegate);
    standarts_Delegate->active_ch = &active_ch;
    //standarts_Delegate->style = StyleApp;
    header << "N°" << "" << tr("ch") << tr("Name") << tr("Values");
    Standarts_Table->setHorizontalHeaderLabels(header);
    Standarts_Table->horizontalHeader()->setFont(qApp->font());

    connect(count_Standarts, SIGNAL(valueChanged(int)), this, SLOT(Change_CountStandarts(int)));

    label_unit = new QLabel(tr("Units of value:  "));
    Units_Box = new QComboBox();
    header.clear();
    header << tr("copies") << tr("pg") << tr("IU");
    Units_Box->addItems(header);

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
//---
//-----------------------------------------------------------------------------
Standarts::~Standarts()
{
    delete count_Standarts;
    delete count_Doubles;
    delete Units_Box;
    delete label_count;
    delete label_Doubles;
    delete label_unit;
    delete standarts_Delegate;
    Standarts_Table->clear();
    delete Standarts_Table;

}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Standarts::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/Standarts_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Standarts::Load_XML(QDomNode &node)
{
    int j,k,m;

    QTableWidgetItem *item_value;
    QString text;
    QStringList list,list_ch;

    int count_St, count_D, type_unit;
    int count_ch = 0;

    for(j=0; j<COUNT_CH; j++)
    {
        if(active_ch & (0x0f<<j*4)) count_ch++;
    }

    QDomNode    child;
    QDomNode    child_node;
    QDomElement child_element;    

    for(j=0; j<node.childNodes().size(); j++)
    {
        child_node = node.childNodes().at(j);
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
                //qDebug() << "list)ch: " << list_ch;
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
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Standarts::Read_XML(QString text)
{
    QDomDocument doc;
    QDomElement root;

    //qDebug() << "Read_XML:" << text;


    if(text.length() && doc.setContent(text))
    {
        root = doc.documentElement();
        Load_XML(root);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Standarts::Write_XML(QString &str)
{
    int i,j,k;
    QString text, str_temp, value_str;
    QDomDocument doc;
    QTableWidgetItem *item_table;

    if(Standarts_Table->rowCount() <= 0) return;


    int count_ch = 0;
    for(j=0; j<COUNT_CH; j++)
    {
        if(active_ch & (0x0f<<j*4)) count_ch++;
    }

    QStringList list;
    list << STANDARTS_INFORMATION;

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    QDomElement  root = doc.createElement("properties");
    doc.appendChild(root);

    for(i=0; i<list.size(); i++)
    {
        value_str = list.at(i);



        switch(i)
        {
        case 0:                                         // STANDARTS_INFORMATION
                text = QString("%1\t%2\t%3\t").arg(count_Standarts->value())
                                              .arg(count_Doubles->value())
                                              .arg(Units_Box->currentIndex()); // Number standarts, number doubles, type units
                for(j=0; j<count_Standarts->value(); j++)
                {
                    if(j) text += "\t";
                    for(k=0; k<count_ch; k++)
                    {
                        item_table = Standarts_Table->item(j*count_ch + k, 4);
                        str_temp = item_table->text().trimmed();
                        if(str_temp.isEmpty()) str_temp = "0";
                        if(k) text += ";";
                        text += str_temp;
                    }
                }

                break;

        default:   text = ""; break;
        }

        QDomElement item_property = doc.createElement("item");
        item_property.appendChild(MakeElement(doc,"name", value_str));
        item_property.appendChild(MakeElement(doc,"value", text));
        item_property.appendChild(MakeElement(doc,"unit", ""));
        root.appendChild(item_property);
    }

    str = doc.toString();
}
//-----------------------------------------------------------------------------
//--- Set_ActiveCh
//-----------------------------------------------------------------------------
void Standarts::Set_ActiveCh(int ch)
{
    active_ch = ch;

    int count = count_Standarts->value();
    Change_CountStandarts(count);
}

//-----------------------------------------------------------------------------
//--- Change_CountStandarts
//-----------------------------------------------------------------------------
void Standarts::Change_CountStandarts(int count_tubes)
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
void Standarts1ItemDelegate::paint(QPainter *painter,
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

                    switch(i)
                    {
                        case 0:  pixmap.load(":/images/flat/fam_flat.png");    break;
                        case 1:  pixmap.load(":/images/flat/hex_flat.png");    break;
                        case 2:  pixmap.load(":/images/flat/rox_flat.png");    break;
                        case 3:  pixmap.load(":/images/flat/cy5_flat.png");    break;
                        case 4:  pixmap.load(":/images/flat/cy55_flat.png");   break;
                        default: pixmap.load(":/images/flat/disable_flat.png");   break;
                    }


                painter->drawPixmap(rect.x()+20, rect.y()+2, pixmap);
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
QWidget* Standarts1ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
void Standarts1ItemDelegate::setModelData(QWidget *editor,
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
