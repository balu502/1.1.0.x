#include "common.h"
#define FLUOR_AVALABLENAME {"Fam","Hex,R6G","Rox","Cy5","Cy5.5"}    // name


//-----------------------------------------------------------------------------
extern "C" COMMONSHARED_EXPORT HWND __stdcall cr_Common(HWND handle, HWND parent_widget, char* Pro)
{
    //int argc = 0;
    //char **argv = 0;
    /*if(QApplication::instance() == 0)
    {
        new QApplication(argc, argv);
        qDebug() << "new app: ";
    }*/    

    //qDebug() << "parent_widget: " << parent_widget;

    QString xml_text(Pro);
    //qDebug() << "TEXT: " << xml_text;


    RECT rect;
    GetClientRect(handle, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    QWidget *pw = QWidget::find((WId)parent_widget);

    Common* p = new Common(pw);
    p->setWindowFlags(Qt::ToolTip);
    HWND handle_common = (HWND)p->winId();

    if(!xml_text.isEmpty()) p->Read_XML(xml_text);
    ((Common*)p)->show();

    ::SetParent(handle_common, handle);
    ::SetWindowPos(handle_common, HWND_TOP, 0,0, width, height, SWP_SHOWWINDOW);

    return(handle_common);
}
//-----------------------------------------------------------------------------
extern "C" COMMONSHARED_EXPORT void __stdcall delete_Common(HWND handle)
{
    QWidget *p = QWidget::find((WId)handle);
    delete (Common*)p;
    return;
}
//-----------------------------------------------------------------------------
extern "C" COMMONSHARED_EXPORT void __stdcall resize_Common(HWND handle)
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
extern "C" COMMONSHARED_EXPORT void __stdcall write_Common(HWND handle, char** p_out)
{
    char *out;
    int leng;
    QString text_out;

    Common* p = (Common*)QWidget::find((WId)handle);

    p->Write_XML(text_out);

    //qDebug() << "text_out: " << text_out;

    leng = text_out.length();
    out = new char[leng+1];
    *p_out = out;

    memcpy(out, text_out.toLatin1().data(), leng+1);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Common::Common(QWidget *parent):
    QTreeWidget(parent)
{
    int i;
    //QString fluor_name[COUNT_CH] = FLUOR_NAME;
    QString text;    

    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItem *item_ch_ch;
    Edit_Button *w_obj;

    readCommonSettings();
    setFont(qApp->font());
    this->header()->setFont(qApp->font());

    //message.setWindowIcon(QIcon(":/images/DTm.ico"));
    //message.setWindowIcon(QIcon(":/images/RT.ico"));

    setStyleSheet(
                "QPushButton {min-width: 2em; margin:1; padding:0;}"
                //"QComboBox {min-width:4em; margin:1; padding:1;"
                );

    QStringList pp;
    pp << tr("Property") << tr("Value") << tr("Edit") << tr("Comments");

    clear();
    setColumnCount(4);
    setHeaderLabels(pp);
    common_Delegate = new CommonDelegate(this);
    setItemDelegate(common_Delegate);
    //setEditTriggers(QAbstractItemView::NoEditTriggers);

    // edit buttons
    edit_CriterionPosResult = new Edit_Button("...");
    edit_CriterionValidity = new Edit_Button("...");
    edit_CriterionAFF = new Edit_Button("...");
    edit_MinBorderCriterionAFF = new Edit_Button("...");
    edit_MaxBorderCriterionAFF = new Edit_Button("...");
    edit_MinBorderSigmoidValidity = new Edit_Button("...");
    edit_MaxBorderSigmoidValidity = new Edit_Button("...");
    edit_ThresholdValue = new Edit_Button("...");
    edit_0_expo = new Edit_Button("...");
    edit_1_expo = new Edit_Button("...");
    edit_2_expo = new Edit_Button("...");
    edit_3_expo = new Edit_Button("...");
    edit_4_expo = new Edit_Button("...");

    back_Default = new QPushButton(tr("Set Default values..."));

    connect((QPushButton*)edit_CriterionPosResult,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_CriterionValidity,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_CriterionAFF,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_MinBorderCriterionAFF,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_MaxBorderCriterionAFF,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_MinBorderSigmoidValidity,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_MaxBorderSigmoidValidity,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_ThresholdValue,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_0_expo,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_1_expo,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_2_expo,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_3_expo,SIGNAL(clicked()), this, SLOT(Edit_Show()));
    connect((QPushButton*)edit_4_expo,SIGNAL(clicked()), this, SLOT(Edit_Show()));

    connect(back_Default, SIGNAL(clicked()), this, SLOT(Set_DefaultValues()));

    //1. Software properties
    item = new QTreeWidgetItem(this);    
    item->setText(0, tr("Software Properties"));

    item_child = new QTreeWidgetItem(item);    
    item_child->setText(0,tr("Criterion of the Positive Result"));
    item_child->setText(1,tr("80"));
    item_child->setText(3,tr("(50-100%)"));
    item_child->setFlags(item->flags() | Qt::ItemIsEditable);
    setItemWidget(item_child, 2, (QPushButton*)edit_CriterionPosResult);

    item_child = new QTreeWidgetItem(item);    
    item_child->setText(0,tr("Criterion to Validity(K+)"));
    item_child->setText(1,tr("5"));
    item_child->setText(3,tr("(0-100%)"));
    item_child->setFlags(item->flags() | Qt::ItemIsEditable);
    setItemWidget(item_child, 2, (QPushButton*)edit_CriterionValidity);

    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Criterion of AFF"));
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Use this method in Analysis"));
    item_ch_ch->setCheckState(1,Qt::Checked);
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Absolute Value of AFF"));
    item_ch_ch->setText(1,tr("1000"));
    item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
    setItemWidget(item_ch_ch, 2, (QPushButton*)edit_CriterionAFF);
    item_ch_ch->setText(3,tr("(100-20000) absolute value of the fluorescence"));
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Min border"));
    item_ch_ch->setText(1,tr("10"));
    item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
    setItemWidget(item_ch_ch, 2, (QPushButton*)edit_MinBorderCriterionAFF);
    item_ch_ch->setText(3,tr("(0-100%)"));
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Max border"));
    item_ch_ch->setText(1,tr("90"));
    item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
    setItemWidget(item_ch_ch, 2, (QPushButton*)edit_MaxBorderCriterionAFF);
    item_ch_ch->setText(3,tr("(0-100%)"));

    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Borders to Sigmoid Validity"));
    item_child->setText(3,tr("(3-50)"));
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Min"));
    item_ch_ch->setText(1,tr("7"));
    setItemWidget(item_ch_ch, 2, (QPushButton*)edit_MinBorderSigmoidValidity);
    item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Max"));
    item_ch_ch->setText(1,tr("20"));
    setItemWidget(item_ch_ch, 2, (QPushButton*)edit_MaxBorderSigmoidValidity);
    item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
    //item_child->setExpanded(true);

    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Threshold method:(Ct)"));
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Use this method in Analysis"));
    item_ch_ch->setCheckState(1,Qt::Unchecked);
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Value of threshold"));
    item_ch_ch->setText(1,tr("10"));
    item_ch_ch->setText(3,tr("(1-100) StD at linear fitting range"));
    item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
    setItemWidget(item_ch_ch, 2, (QPushButton*)edit_ThresholdValue);
    //item_child->setExpanded(true);

    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Melting Curve"));
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Value of FWHM"));
    item_ch_ch->setText(1,tr("30"));
    item_ch_ch->setText(3,tr("(1-100%) FWHM"));
    item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("Value of Peaks border"));
    item_ch_ch->setText(1,tr("20"));
    item_ch_ch->setText(3,tr("(1-1000) Peaks border"));
    item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);

    item->setExpanded(true);

    //2. Hardware properties
    item = new QTreeWidgetItem(this);
    item->setText(0, tr("Hardware Properties"));

    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Exposure of the optical measurements"));
    item_child->setText(3,tr("(0.1-10.0 c.u.e.)"));
    //item_child->setExpanded(true);
    QString name_channel(tr("channel"));
    for(i=0; i<COUNT_CH; i++)
    {
        item_ch_ch = new QTreeWidgetItem(item_child);
        text = QString("%1 %2").arg(i+1).arg(name_channel);
        item_ch_ch->setText(0,text);
        item_ch_ch->setText(1,tr("1.00"));
        item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
        switch(i)
        {
        case 0: w_obj = edit_0_expo;    break;
        case 1: w_obj = edit_1_expo;    break;
        case 2: w_obj = edit_2_expo;    break;
        case 3: w_obj = edit_3_expo;    break;
        case 4: w_obj = edit_4_expo;    break;
        }

        setItemWidget(item_ch_ch, 2, (QPushButton*)w_obj);
    }


    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Fluorofors Name of the optical channels"));
    for(i=0; i<COUNT_CH; i++)
    {
        item_ch_ch = new QTreeWidgetItem(item_child);
        text = QString("%1 %2").arg(i+1).arg(name_channel);
        item_ch_ch->setText(0,text);
        item_ch_ch->setFlags(item->flags() | Qt::ItemIsEditable);
        Fluor_Channel *fluor_ch = new Fluor_Channel(i, this);
        setItemWidget(item_ch_ch, 1, (QComboBox*)fluor_ch);
    }

    item->setExpanded(true);

    //3. Default values
    item = new QTreeWidgetItem(this);
    item = new QTreeWidgetItem(this);    
    setItemWidget(item, 0, back_Default);

    resizeColumnToContents(0);

    //this->header()->setMinimumWidth(0);
    //setColumnWidth(2,0);
    hideColumn(2);

    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(CheckValid_Value(QTreeWidgetItem*,int)));

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Common::~Common()
{
    delete edit_CriterionPosResult;
    delete edit_CriterionValidity;
    delete edit_CriterionAFF;
    delete edit_MinBorderCriterionAFF;
    delete edit_MaxBorderCriterionAFF;
    delete edit_MinBorderSigmoidValidity;
    delete edit_MaxBorderSigmoidValidity;
    delete edit_ThresholdValue;
    delete edit_0_expo;
    delete edit_1_expo;
    delete edit_2_expo;
    delete edit_3_expo;
    delete edit_4_expo;    
    delete back_Default;

    delete common_Delegate;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Fluor_Channel::Fluor_Channel(int index, QWidget *parent):
    QComboBox(parent)
{
    QString names[COUNT_CH] = FLUOR_AVALABLENAME;
    QStringList list = names[index].split(",");

    addItems(list);
    setStyleSheet("min-width:50px; max-width:150px; margin:0; padding:0;");

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Fluor_Channel::~Fluor_Channel()
{

}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Common::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/Common_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common::Edit_Show()
{
    QPushButton *edit_button = qobject_cast<QPushButton *>(sender());

    if(currentItem()->isDisabled()) return;
    setFocus();
    editItem(currentItem(),1);
    edit_button->clearFocus();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common::Set_DefaultValues()
{
    int i,j;
    QString text,name;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItem *item_ch_ch;
    QWidget *obj;

    topLevelItem(0)->child(0)->setText(1, "80");
    topLevelItem(0)->child(1)->setText(1, "5");

    topLevelItem(0)->child(2)->child(0)->setCheckState(1,Qt::Checked);
    topLevelItem(0)->child(2)->child(1)->setText(1, "1000");
    topLevelItem(0)->child(2)->child(2)->setText(1, "10");
    topLevelItem(0)->child(2)->child(3)->setText(1, "90");

    topLevelItem(0)->child(3)->child(0)->setText(1, "7");
    topLevelItem(0)->child(3)->child(1)->setText(1, "20");
    topLevelItem(0)->child(4)->child(0)->setCheckState(1,Qt::Unchecked);
    topLevelItem(0)->child(4)->child(1)->setText(1, "10");
    topLevelItem(0)->child(5)->child(0)->setText(1, "30");
    topLevelItem(0)->child(5)->child(1)->setText(1, "20");

    if(!topLevelItem(1)->isDisabled())
    {
        item_child = topLevelItem(1)->child(0);
        for(i=0; i<item_child->childCount(); i++)
        {
            item_ch_ch = item_child->child(i);
            item_ch_ch->setText(1, "1.00");
        }

        item_child = topLevelItem(1)->child(1);
        for(i=0; i<item_child->childCount(); i++)
        {
            item_ch_ch = item_child->child(i);
            obj = itemWidget(item_ch_ch, 1);
            ((QComboBox*)obj)->setCurrentIndex(0);
        }
    }

    text = tr("Default values are set!");
    message.setText(text);
    message.setIcon(QMessageBox::Information);
    message.setStandardButtons(QMessageBox::Ok);
    message.button(QMessageBox::Ok)->animateClick(3000);
    message.exec();

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common::CheckValid_Value(QTreeWidgetItem *item, int col)
{
    if(col != 1) return;

    QTreeWidgetItem *item_parent;
    QTreeWidgetItem *item_tmp;
    QTreeWidgetItem *item_ch;
    int id, value, id_ch;
    double dvalue;
    bool ok;
    bool checked;
    bool exposure_state = false;
    QString text = item->text(1);

    item_parent = item->parent();
    item_tmp = item_parent->parent();
    if(item_tmp == NULL)
    {
        if(item_parent == topLevelItem(0))
        {
            id = item_parent->indexOfChild(item);
            switch(id)
            {
            case 0:                                 // PosResult
                    value = text.toInt(&ok);
                    if(!ok) value = 80;
                    else
                    {
                        if(value > 100) value = 100;
                        if(value < 50) value = 50;
                    }
                    break;

            case 1:                                 // Validity(K+)
                    value = text.toInt(&ok);
                    if(!ok) value = 5;
                    else
                    {
                        if(value > 100) value = 100;
                        if(value < 0) value = 0;
                    }
                    break;

            /*case 2:                                 // AFF
                    value = text.toInt(&ok);
                    if(!ok) value = 20;
                    else
                    {
                        if(value > 100) value = 100;
                        if(value < 0) value = 0;
                    }
                    break;*/

            default:
                    value = 0;  break;
            }
        }

    }
    else
    {
        if(item_tmp == topLevelItem(0))
        {
            id = item_tmp->indexOfChild(item_parent);

            if(id == 2)                                         // AFF
            {
                id_ch = item_parent->indexOfChild(item);
                switch(id_ch)
                {
                default:
                case 0:     if(item->checkState(1) == Qt::Checked) checked = true;
                            else checked = false;
                            item_ch = item_parent->child(1);
                            item_ch->setDisabled(!checked);
                            item_ch = item_parent->child(2);
                            item_ch->setDisabled(!checked);
                            item_ch = item_parent->child(3);
                            item_ch->setDisabled(!checked);
                            return;

                case 1:     value = text.toInt(&ok);
                            if(!ok) value = 1000;
                            else
                            {
                                if(value > 20000) value = 20000;
                                if(value < 100) value = 100;
                            }
                            break;

                case 2:     value = text.toInt(&ok);
                            if(!ok) value = 10;
                            else
                            {
                                if(value > 100) value = 100;
                                if(value < 0) value = 0;
                            }
                            break;

                case 3:     value = text.toInt(&ok);
                            if(!ok) value = 90;
                            else
                            {
                                if(value > 100) value = 100;
                                if(value < 0) value = 0;
                            }
                            break;
                }
            }

            if(id == 3)                                         // Sigmoid validation: borders
            {
                id_ch = item_parent->indexOfChild(item);
                switch (id_ch)
                {
                case 0:                             // Min
                        value = text.toInt(&ok);
                        if(!ok) value = 7;
                        else
                        {
                            if(value > 50) value = 50;
                            if(value < 3) value = 3;
                        }
                        break;

                case 1:                             // Max
                        value = text.toInt(&ok);
                        if(!ok) value = 20;
                        else
                        {
                            if(value > 50) value = 50;
                            if(value < 3) value = 3;
                        }
                        break;

                default:
                        value = 0;  break;
                }
            }

            if(id == 4)                                         // Threshold method
            {
                id_ch = item_parent->indexOfChild(item);

                if(id_ch == 0)  if(item->checkState(1) == Qt::Checked) checked = true;
                                else checked = false;
                                item_ch = item_parent->child(1);
                                item_ch->setDisabled(!checked);
                                return;
                if(id_ch == 1)
                {                           // Value of Threshold
                    value = text.toInt(&ok);
                    if(!ok) value = 10;
                    else
                    {
                        if(value > 100) value = 100;
                        if(value < 1) value = 1;
                    }
                }
            }

            if(id == 5)                                         // MC parameters
            {
                id_ch = item_parent->indexOfChild(item);

                if(id_ch == 0)
                {                               // MC_FWHM_BORDER
                    value = text.toInt(&ok);
                    if(!ok) value = 30;
                    else
                    {
                        if(value > 100) value = 100;
                        if(value < 1) value = 1;
                    }
                }

                if(id_ch == 1)
                {                               // MC_PEAKS_BORDER
                    value = text.toInt(&ok);
                    if(!ok) value = 20;
                    else
                    {
                        if(value > 1000) value = 1000;
                        if(value < 1) value = 1;
                    }
                }
            }

        }

        if(item_tmp == topLevelItem(1))
        {
            id = item_tmp->indexOfChild(item_parent);
            if(id == 0)
            {                               // Exposure
                exposure_state = true;
                dvalue = text.toDouble(&ok);
                if(!ok) dvalue = 1.0;
                else
                {
                    if(dvalue > 10.0) dvalue = 1.0;
                    if(dvalue < 0.1) dvalue = 1.0;
                }
            }
            if(id == 1)
            {
                return;
            }

        }
    }

    if(exposure_state) text = QString("%1").arg(dvalue,0,'f',2);
    else text = QString::number(value);

    blockSignals(true);
    item->setText(1,text);
    blockSignals(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common::Fill_Properties(rt_Test *ptest)
{
    int i,j;
    QString text,value_str;
    QStringList list;
    list << CRITERION_POSRES << CRITERION_VALIDITY << USE_AFF << CRITERION_AFF << CRITERION_AFF_MIN << CRITERION_AFF_MAX << \
            SIGMOIDVAL_MIN << SIGMOIDVAL_MAX << USE_THRESHOLD << VALUE_THRESHOLD << EXPOSURE << FLUOROFORS << MC_FWHM_BORDER << MC_PEAKS_BORDER;
    rt_Preference *preference;
    bool find;
    QWidget *obj;

    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItem *item_ch_ch;


    for(i=0; i<list.size(); i++)
    {
        text = list.at(i);
        find = false;
        item = topLevelItem(0);

        switch(i)
        {
        case 0:                                         // CRITERION_POSRES
        case 1:                                         // CRITERION_VALIDITY
                item_child = item->child(i);
                value_str = item_child->text(1);
                break;

        case 2:                                         // USE_AFF
                item_child = item->child(2);
                item_ch_ch = item_child->child(0);
                if(item_ch_ch->checkState(1) == Qt::Checked) value_str = "True";
                else value_str = "False";
                break;

        case 3:
        case 4:
        case 5:
                item_child = item->child(2);            // CRITERION_AFF
                item_ch_ch = item_child->child(i-2);    // CRITERION_AFF_MIN
                value_str = item_ch_ch->text(1);        // CRITERION_AFF_MAX
                break;

        case 6:                                         // SIGMOIDVAL_MIN
        case 7:                                         // SIGMOIDVAL_MAX
                item_child = item->child(3);
                item_ch_ch = item_child->child(i-6);
                value_str = item_ch_ch->text(1);                
                break;

        case 8:                                         // USE_THRESHOLD
                item_child = item->child(4);
                item_ch_ch = item_child->child(0);
                if(item_ch_ch->checkState(1) == Qt::Checked) value_str = "True";
                else value_str = "False";
                break;

        case 9:                                         // VALUE_THRESHOLD
                item_child = item->child(4);
                item_ch_ch = item_child->child(1);
                value_str = item_ch_ch->text(1);
                break;

        case 10:                                         // EXPOSURE
                item = topLevelItem(1);
                item_child = item->child(0);
                value_str = "";
                for(j=0; j<COUNT_CH; j++)
                {
                    item_ch_ch = item_child->child(j);
                    if(!value_str.isEmpty()) value_str += " ";
                    value_str += item_ch_ch->text(1);
                }
                break;

        case 11:                                         // FLOUROFORS
                item = topLevelItem(1);
                item_child = item->child(1);
                value_str = "";
                for(j=0; j<COUNT_CH; j++)
                {
                    item_ch_ch = item_child->child(j);
                    if(!value_str.isEmpty()) value_str += " ";
                    obj = itemWidget(item_ch_ch, 1);
                    Fluor_Channel *ch = (Fluor_Channel*)obj;
                    value_str += ((QComboBox*)ch)->currentText();
                }
                break;

        case 12:                                        // MC_FWHM_BORDER
                item = topLevelItem(0);
                item_child = item->child(5);
                item_ch_ch = item_child->child(0);
                value_str = item_ch_ch->text(1);
                break;

        case 13:                                        // MC_PEAKS_BORDER
                item = topLevelItem(0);
                item_child = item->child(5);
                item_ch_ch = item_child->child(1);
                value_str = item_ch_ch->text(1);
                break;

        default:   value_str = ""; break;
        }


        for(j=0; j<ptest->preference_Test.size(); j++)
        {
            preference = ptest->preference_Test.at(j);
            if(QString::fromStdString(preference->name) == text)
            {
                preference->value = value_str.toStdString();
                find = true;
            }
        }

        if(!find)
        {
            preference = new rt_Preference;
            preference->name = text.toStdString();
            preference->value = value_str.toStdString();
            ptest->preference_Test.push_back(preference);
        }

        //qDebug() << "pro:" << find << QString::fromStdString(preference->name) << QString::fromStdString(preference->value);
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common::Read_XML(QString text)
{
    QDomDocument doc;
    QDomElement root;


    if(text.length() && doc.setContent(text))
    {
        root = doc.documentElement();
        Load_XML(root);
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common::Write_XML(QString &str)
{
    int i,j;
    QString text, value_str;
    QDomDocument doc;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItem *item_ch_ch;
    QWidget *obj;

    QStringList list;
    list << CRITERION_POSRES << CRITERION_VALIDITY << USE_AFF << CRITERION_AFF << CRITERION_AFF_MIN << CRITERION_AFF_MAX << \
            SIGMOIDVAL_MIN << SIGMOIDVAL_MAX << USE_THRESHOLD << VALUE_THRESHOLD << EXPOSURE << FLUOROFORS << MC_FWHM_BORDER << MC_PEAKS_BORDER;

    QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
    doc.insertBefore(xmlNode, doc.firstChild());

    QDomElement  root = doc.createElement("properties");
    doc.appendChild(root);

    for(i=0; i<list.size(); i++)
    {
        text = list.at(i);

        item = topLevelItem(0);

        switch(i)
        {
        case 0:                                         // CRITERION_POSRES
        case 1:                                         // CRITERION_VALIDITY
                item_child = item->child(i);
                value_str = item_child->text(1);
                break;

        case 2:                                         // USE_AFF
                item_child = item->child(2);
                item_ch_ch = item_child->child(0);
                if(item_ch_ch->checkState(1) == Qt::Checked) value_str = "True";
                else value_str = "False";
                break;

        case 3:
        case 4:
        case 5:
                item_child = item->child(2);            // CRITERION_AFF
                item_ch_ch = item_child->child(i-2);    // CRITERION_AFF_MIN
                value_str = item_ch_ch->text(1);        // CRITERION_AFF_MAX
                break;

        case 6:                                         // SIGMOIDVAL_MIN
        case 7:                                         // SIGMOIDVAL_MAX
                item_child = item->child(3);
                item_ch_ch = item_child->child(i-6);
                value_str = item_ch_ch->text(1);
                break;

        case 8:                                         // USE_THRESHOLD
                item_child = item->child(4);
                item_ch_ch = item_child->child(0);
                if(item_ch_ch->checkState(1) == Qt::Checked) value_str = "True";
                else value_str = "False";
                break;

        case 9:                                         // VALUE_THRESHOLD
                item_child = item->child(4);
                item_ch_ch = item_child->child(1);
                value_str = item_ch_ch->text(1);
                break;

        case 10:                                         // EXPOSURE
                item = topLevelItem(1);
                item_child = item->child(0);
                value_str = "";
                for(j=0; j<COUNT_CH; j++)
                {
                    item_ch_ch = item_child->child(j);
                    if(!value_str.isEmpty()) value_str += " ";
                    value_str += item_ch_ch->text(1);
                }
                break;

        case 11:                                         // FLOUROFORS
                item = topLevelItem(1);
                item_child = item->child(1);
                value_str = "";
                for(j=0; j<COUNT_CH; j++)
                {
                    item_ch_ch = item_child->child(j);
                    if(!value_str.isEmpty()) value_str += " ";
                    obj = itemWidget(item_ch_ch, 1);
                    Fluor_Channel *ch = (Fluor_Channel*)obj;
                    value_str += ((QComboBox*)ch)->currentText();
                }
                break;

        case 12:                                        // MC_FWHM_BORDER
                item = topLevelItem(0);
                item_child = item->child(5);
                item_ch_ch = item_child->child(0);
                value_str = item_ch_ch->text(1);
                break;

        case 13:                                        // MC_PEAKS_BORDER
                item = topLevelItem(0);
                item_child = item->child(5);
                item_ch_ch = item_child->child(1);
                value_str = item_ch_ch->text(1);
                break;

        default:   value_str = ""; break;
        }

        QDomElement item_property = doc.createElement("item");
        item_property.appendChild(MakeElement(doc,"name", text));
        item_property.appendChild(MakeElement(doc,"value", value_str));
        item_property.appendChild(MakeElement(doc,"unit", ""));
        root.appendChild(item_property);
    }

    str = doc.toString();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common::Load_XML(QDomNode &node)
{
    int i,j;
    QString text,name;
    QDomNode        child;
    QDomElement     element;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItem *item_ch_ch;
    QStringList list;
    QWidget *obj;
    int id;
    bool checked;

    for(i=0; i<node.childNodes().size(); i++)
    {
        child = node.childNodes().at(i);
        element = child.firstChildElement("name");
        if(element.isNull()) continue;

        text = child.firstChildElement("value").text();
        name = element.text();

        item = topLevelItem(0);

        if(name == CRITERION_POSRES)
        {
            item_child = item->child(0);
            item_child->setText(1, text);
            continue;
        }
        if(name == CRITERION_VALIDITY)
        {
            item_child = item->child(1);
            item_child->setText(1, text);
            continue;
        }
        if(name == USE_AFF)
        {
            item_child = item->child(2);
            item_ch_ch = item_child->child(0);
            if(text == "no" || text == "False") item_ch_ch->setCheckState(1,Qt::Unchecked);
            else item_ch_ch->setCheckState(1,Qt::Checked);
            if(item_ch_ch->checkState(1) == Qt::Checked) checked = true;
            else checked = false;

            item_ch_ch = item_child->child(1);
            item_ch_ch->setDisabled(!checked);
            item_ch_ch = item_child->child(2);
            item_ch_ch->setDisabled(!checked);
            item_ch_ch = item_child->child(3);
            item_ch_ch->setDisabled(!checked);

            continue;
        }
        if(name == CRITERION_AFF)
        {
            item_child = item->child(2);
            item_ch_ch = item_child->child(1);
            item_ch_ch->setText(1, text);
            continue;
        }
        if(name == CRITERION_AFF_MIN)
        {
            item_child = item->child(2);
            item_ch_ch = item_child->child(2);
            item_ch_ch->setText(1, text);
            continue;
        }
        if(name == CRITERION_AFF_MAX)
        {
            item_child = item->child(2);
            item_ch_ch = item_child->child(3);
            item_ch_ch->setText(1, text);
            continue;
        }
        if(name == SIGMOIDVAL_MIN)
        {
            item_child = item->child(3);
            item_ch_ch = item_child->child(0);
            item_ch_ch->setText(1, text);
            continue;
        }
        if(name == SIGMOIDVAL_MAX)
        {
            item_child = item->child(3);
            item_ch_ch = item_child->child(1);
            item_ch_ch->setText(1, text);
            continue;
        }
        if(name == EXPOSURE)
        {
            item = topLevelItem(1);
            item_child = item->child(0);
            list = text.split(QRegExp("\\s+"));
            //qDebug() << "list_expo: " << list;
            for(j=0; j<list.size(); j++)
            {
                if(j >= COUNT_CH) break;
                text = list.at(j);

                item_ch_ch = item_child->child(j);
                item_ch_ch->setText(1, text);
            }
            continue;
        }
        if(name == FLUOROFORS)
        {
            item = topLevelItem(1);
            item_child = item->child(1);
            list = text.split(QRegExp("\\s+"));
            for(j=0; j<list.size(); j++)
            {
                if(j >= COUNT_CH) break;
                text = list.at(j);
                item_ch_ch = item_child->child(j);
                obj = itemWidget(item_ch_ch, 1);
                if(obj)
                {
                    Fluor_Channel *ch = (Fluor_Channel*)obj;
                    id = ((QComboBox*)ch)->findText(text);
                    if(id < 0) id = 0;
                    ((QComboBox*)ch)->setCurrentIndex(id);

                    if(topLevelItem(1)->isDisabled()) obj->setDisabled(true);
                }
            }
            continue;
        }
        if(name == USE_THRESHOLD)
        {
            item_child = item->child(4);
            item_ch_ch = item_child->child(0);
            if(text == "no" || text == "False") item_ch_ch->setCheckState(1,Qt::Unchecked);
            else item_ch_ch->setCheckState(1,Qt::Checked);
            if(item_ch_ch->checkState(1) == Qt::Checked) checked = true;
            else checked = false;

            item_ch_ch = item_child->child(1);
            item_ch_ch->setDisabled(!checked);
            continue;
        }
        if(name == VALUE_THRESHOLD)
        {
            item_child = item->child(4);
            item_ch_ch = item_child->child(1);
            item_ch_ch->setText(1, text);
            continue;
        }
        if(name == MC_FWHM_BORDER)
        {
            item = topLevelItem(0);
            item_child = item->child(5);
            item_ch_ch = item_child->child(0);
            item_ch_ch->setText(1, text);
            continue;
        }
        if(name == MC_PEAKS_BORDER)
        {
            item = topLevelItem(0);
            item_child = item->child(5);
            item_ch_ch = item_child->child(1);
            item_ch_ch->setText(1, text);
            continue;
        }
    }

    if(topLevelItem(1)->isDisabled()) common_Delegate->Item1_isDisable = true;
    //qDebug() << "item: " << topLevelItem(1)->isDisabled();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void CommonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{      
    QString text;
    double val;
    bool ok;

    QColor color_bg;
    QRect rect = option.rect;
    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus

    painter->setFont(qApp->font());

    QStyledItemDelegate::paint(painter, viewOption, index);

    text = index.data().toString();
    if(index.row() == 0 && index.parent().row() == 4)
    {
        text = "        ";
        painter->drawText(rect, Qt::AlignCenter, text);
    }

    // for values of exposure

    if(index.parent().parent().row() == 1 && index.parent().row() == 0 && index.column() == 1)
    {
        val = index.data().toString().trimmed().toDouble(&ok);
        if(!ok) val = 1.0;

        //if(index.data().toString().trimmed() == "0")
        if(val == 1.0)
        {
            color_bg = painter->background().color();
            text = tr("default");
            painter->fillRect(rect, color_bg); //QColor(255,255,255));
            if(Item1_isDisable) painter->setPen(QColor(Qt::gray));
            painter->drawText(rect, Qt::AlignLeft, text);
            //qDebug() << "index: " << index.row() << index.column() << index.data().toString();
        }
    }




}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget* CommonDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget* obj;
    QPalette palette;

    if(index.column() == 1)
    {
        //if(/*index.row() == 0 &&*/ index.parent().row() == 4)
        //{
            /*QComboBox *editor = new QComboBox(parent);
            editor->addItem(tr("no"));
            editor->addItem(tr("yes"));
            palette.setColor(QPalette::Text,Qt::black);//Qt::darkBlue);
            palette.setColor(QPalette::Base, QColor(255,255,255)); //QColor(255,255,225));
            palette.setColor(QPalette::Highlight,QColor(180,180,255));
            palette.setColor(QPalette::HighlightedText,Qt::black);
            editor->setPalette(palette);
            obj = editor;

            connect(editor,SIGNAL(currentIndexChanged(int)), this, SLOT(Close_Editor(int)));

            emit showComboPopup(editor);*/

            //return nullptr;
        //}
        //else
        //{
            //obj = QStyledItemDelegate::createEditor(parent, option, index);
            QLineEdit *editor = new QLineEdit(parent);
            QRegExp rx("[0-9.]{1,6}");
            editor->setValidator(new QRegExpValidator(rx, parent));
            obj = editor;
            obj->setStyleSheet("QLineEdit {selection-background-color: rgb(215,215,255); selection-color: black;}");


        //}

        //return(new QLineEdit(parent));
        //return(new QTextEdit(parent));
        //qDebug() << "row:" << index.row() << index.parent().row();
        return(obj);
    }
    return nullptr;

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void CommonDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString text;
    int id = 0;

    if(index.row() == 0 && index.parent().row() == 4)
    {
        text = index.model()->data(index, Qt::EditRole).toString();
        if(text == tr("yes")) id = 1;

        QComboBox *cBox = static_cast<QComboBox*>(editor);
        cBox->blockSignals(true);
        cBox->setCurrentIndex(id);
        cBox->blockSignals(false);
    }
    else QStyledItemDelegate::setEditorData(editor, index);

}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void CommonDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.row() == 0 && index.parent().row() == 4)
    {
        QComboBox *cBox = static_cast<QComboBox*>(editor);
        QString text = tr("no");

        if(cBox->currentIndex()) text = tr("yes");
        model->setData(index, text, Qt::EditRole);

        qDebug() << text;
    }
    else QStyledItemDelegate::setModelData(editor,model,index);
}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void CommonDelegate::Close_Editor(int id)
{
    QComboBox *bx = qobject_cast<QComboBox *>(sender());
    if(bx)
    {
        emit commitData(bx);    // fix data: This signal must be emitted when the editor widget has completed editing the data,
                                // and wants to write it back into the model.

        emit closeEditor(bx);   // This signal is emitted when the user has finished editing an item using the specified editor.
    }
}*/
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
/*void CommonDelegate::openComboPopup(QComboBox *bx)
{
    bx->showPopup();
}*/

