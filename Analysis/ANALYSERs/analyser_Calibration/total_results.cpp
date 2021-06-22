#include "total_results.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Total_Results::Total_Results(QWidget *parent) : QGroupBox(parent)
{
    setFont(qApp->font());

    setObjectName("Transparent");
    QVBoxLayout *main_layout = new QVBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(4);
    setLayout(main_layout);

    Date_label = new QLabel(this);
    Device_label = new QLabel(this);
    Operator_label = new QLabel(this);
    Operator_label->setVisible(false);

    Plate_label = new QLabel(this);

    Open_Dir = new QPushButton(tr("Open video Catalogue..."), this);
    Open_Dir->setVisible(false);

    QHBoxLayout *layout_web = new QHBoxLayout;
    Email_button = new QToolButton(this);
    Email_button->setIcon(QIcon(":/images/email.png"));
    Email_button->setVisible(false);
    //Email_button->setStyleSheet("border: 0px solid #aaa");
    Web_button = new QToolButton(this);
    Web_button->setIcon(QIcon(":/images/web.png"));
    Web_button->setVisible(false);
    layout_web->setMargin(0);
    layout_web->addWidget(Email_button);
    layout_web->addWidget(Web_button);
    QVBoxLayout *layout_webplus = new QVBoxLayout;
    layout_webplus->addLayout(layout_web);
    layout_webplus->addStretch(1);

    Total_Table = new QTableWidget(0, 3, this);
    Total_Table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    Total_Table->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum);
    Total_Table->setSelectionMode(QAbstractItemView::NoSelection);
    Total_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Delegate = new Total_ItemDelegate;
    Total_Table->setItemDelegate(Delegate);
    Total_Table->setFont(qApp->font());
    //qApp->font()

    Conclusion = new QTextEdit(this);
    Conclusion->setReadOnly(true);

    QVBoxLayout *layout_label = new QVBoxLayout;
    layout_label->setMargin(0);
    layout_label->addWidget(Date_label, 0, Qt::AlignLeft);
    layout_label->addWidget(Device_label, 0, Qt::AlignLeft);
    layout_label->addWidget(Plate_label, 0, Qt::AlignLeft);
    layout_label->addWidget(Operator_label, 0, Qt::AlignLeft);

    QHBoxLayout *layout_info = new QHBoxLayout;
    layout_info->setMargin(0);
    layout_info->addLayout(layout_label,1);
    layout_info->addLayout(layout_webplus);

    main_layout->addSpacing(10);
    main_layout->addLayout(layout_info);
    main_layout->addSpacing(30);
    main_layout->addWidget(Total_Table, 0);
    main_layout->addSpacing(10);
    main_layout->addWidget(Conclusion);
    main_layout->addStretch(1);
    main_layout->addWidget(Open_Dir, 0, Qt::AlignLeft);
    main_layout->addSpacing(3);

    total_result = -1;

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Total_Results::~Total_Results()
{
    delete Date_label;
    delete Device_label;
    delete Operator_label;
    delete Plate_label;

    delete Email_button;
    delete Web_button;

    Total_Table->clear();
    delete Delegate;
    delete Total_Table;
    delete Open_Dir;
    delete Conclusion;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Total_Results::fill_TotalResults(rt_Protocol *prot, QVector<int> *error_Buf)
{
    int i,j,k;
    QString text,key;
    QString id_name = "";
    QTableWidgetItem *item;
    QStringList header;
    rt_Preference   *property;
    bool verification;
    QDateTime DT;
    header << tr("Name research") << tr("status") << tr("notice");

    int error_state = 0;
    int overflow_T = false;
    int overflow_A = false;

    k = 0;
    foreach(int state, *error_Buf)
    {
        switch(k)
        {
        case 0: error_state = state;    break;
        case 1: overflow_T = (bool)state;   break;
        case 2: overflow_A = (bool)state;   break;
        default:    break;
        }

        k++;
    }

    //...
    foreach(property, prot->preference_Pro)
    {
        if(property->name == "ID_plate")
        {
            id_name = QString::fromStdString(property->value);
            break;
        }
    }

    //...
    Date_label->setText(QString("   %1  %2").arg(tr("Date:")).arg(QString::fromStdString(prot->time_created)));
    Device_label->setText(QString("   %1  %2 (%3)").arg(tr("Device:")).
                                                   arg(QString::fromStdString(prot->SerialName)).
                                                   arg(QString::fromStdString(prot->Type_ThermoBlock)));
    Plate_label->setText(QString("   %1  %2").arg(tr("Plate:")).arg(id_name));
    Operator_label->setText(QString("   %1  %2").arg(tr("Operator:")).arg(QString::fromStdString(prot->owned_by)));

    Total_Table->clear();
    Total_Table->setColumnCount(3);
    Total_Table->setHorizontalHeaderLabels(header);
    Total_Table->setColumnWidth(0, 250);
    Total_Table->setColumnWidth(1, 80);
    Total_Table->setColumnWidth(2, 100);
    Total_Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    Total_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    Total_Table->setRowCount(4);

    for(i=0; i<Total_Table->rowCount(); i++)
    {
        for(j=0; j<Total_Table->columnCount(); j++)
        {
            item = new QTableWidgetItem();

            switch(j)
            {
            case 0:             // research
                    switch(i)
                    {
                    case 0:     text = tr("Optical mask");  break;
                    case 1:     text = tr("Temperature calibration");  break;
                    case 2:     text = tr("Amplitude calibration");  break;
                    case 3:     text = tr("Crosstalk");  break;
                    default:    break;
                    }

                    item->setText(text);
                    item->setTextAlignment(Qt::AlignCenter);

                    switch(i)
                    {
                    case 0:     item->setIcon(QIcon(":/images/mask.png"));  break;
                    case 1:     item->setIcon(QIcon(":/images/temperature.png"));  break;
                    case 2:     item->setIcon(QIcon(":/images/spectrum.png"));  break;
                    case 3:     item->setIcon(QIcon(":/images/Unequal.png"));  break;
                    default:    break;
                    }

                    break;

            case 1:             // status
                    switch(i)
                    {
                    case 0:     text = QString("%1").arg(mask_status);  break;
                    case 1:     text = QString("%1").arg(temperature_status);
                                if(error_state < 0)
                                {
                                    text = "...";
                                    if(overflow_T) text = "0";
                                }
                                break;
                    case 2:     text = QString("%1").arg(optic_status);
                                if(error_state < 0)
                                {
                                    text = "...";
                                    if(overflow_A) text = "0";
                                }

                                break;
                    default:
                                text = "..."; break;
                    }

                    item->setText(text);
                    break;

            case 2:             // notice
                    text = "";
                    switch(i)
                    {
                    case 0:     if(error_state < 0 /*&& mask_status*/) {text = ""; break;}

                                foreach(key, mask_ATTENTION->keys())
                                {
                                    if(!text.isEmpty()) text += "\r\n";
                                    text += "     " + key + "   " + mask_ATTENTION->value(key);
                                }

                                if(mask_ATTENTION->size() > 1) Total_Table->setRowHeight(i,20*mask_ATTENTION->size());
                                break;

                    case 1:     foreach(key, temperature_ATTENTION->keys())
                                {
                                    if(!text.isEmpty()) text += "\r\n";
                                    text += "     " + key + "   " + temperature_ATTENTION->value(key);
                                }

                                if(temperature_ATTENTION->size() > 1) Total_Table->setRowHeight(i,20*temperature_ATTENTION->size());
                                break;

                    case 2:     foreach(key, optic_ATTENTION->keys())
                                {
                                    if(!text.isEmpty()) text += "\r\n";
                                    text += "     " + key + "   " + optic_ATTENTION->value(key);
                                }

                                if(optic_ATTENTION->size() > 1) Total_Table->setRowHeight(i,20*optic_ATTENTION->size());
                                break;


                    default:
                                text = ""; break;
                    }

                    item->setText(text);
                    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    break;

            default:    item->setText(""); break;
            }

            Total_Table->setItem(i, j, item);
        }
    }

    // 2. Conclusion
    QFont f = qApp->font();
    //f.setPointSize(f.pointSize() + 1);
    f.setBold(true);
    Conclusion->setFont(f);
    if(mask_status && temperature_status && optic_status) verification = true;
    else verification = false;
    Conclusion->insertPlainText(tr("Conclusion:") + "\r\n" + "\r\n");

    if(error_state == 0)
    {
        text = tr("Device");
        //Conclusion->insertPlainText(QString("%1 (%2)").arg(text).arg(QString::fromStdString(prot->SerialName)));
        if(verification)
        {
            //text = tr(" meets all technical requirements and is ready for operation!");
            DT = QDateTime::currentDateTime();
            DT = DT.addDays(365/2);
            text = tr("conclusion_0") + "\r\n" + tr("conclusion_1") + DT.toString(" dd/MM/yyyy.");
            Conclusion->insertPlainText(text);
            total_result = 0;
        }
        else
        {
            //Conclusion->setTextColor(Qt::red);
            text = tr("Error_0");
            Conclusion->insertPlainText(text);
            //Conclusion->setTextColor(Qt::black);
            text = tr("Error_01");
            //text += tr(" You must contact customer support.");
            Conclusion->insertPlainText(text);
            total_result = 1;
        }
    }
    else    // error_state < 0
    {        
        //Conclusion->setTextColor(Qt::red);
        switch(error_state)
        {
        default:    text = tr("Defective plate is detected...");
                    break;
        case -1:    text = "";  break;
        case -2:    text = tr("Error_2.");    break;
        case -3:    text = tr("Error_3.");     break;
        case -4:    text = tr("Error_4.");    break;
        }
        total_result = error_state;


        Conclusion->insertPlainText(text);
        /*if(error_state != -4)
        {
            Conclusion->setTextColor(Qt::black);
            text = tr(" You must contact customer support.");
            Conclusion->insertPlainText(text);
        }*/
    }


}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Total_ItemDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text,str;
    int flag;
    bool ok;
    QStringList list;
    painter->setFont(qApp->font());

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;
    text = index.data().toString();
    int row = index.row();
    int col = index.column();

    QPixmap pixmap(":/images/error_16.png");
    QColor bg_color = Qt::white;

    //__1. Background
    if(col == 1)
    {

        painter->fillRect(rect, bg_color);
        id = text.toInt(&ok);

        if(ok && id > 0) pixmap.load(":/images/check_16.png");
        if(ok)painter->drawPixmap(option.rect.x() + (option.rect.width()-16)/2,
                            option.rect.y() + (option.rect.height()-16)/2,
                            pixmap);
        else
        {
            painter->setPen(Qt::black);
            painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
        }
    }

    if(col == 2)
    {
        painter->fillRect(rect, bg_color);

        if(row) painter->setPen(Qt::red);
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
}
