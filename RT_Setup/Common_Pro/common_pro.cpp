#include "common_pro.h"



//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Common_Pro::Common_Pro(QWidget *parent, rt_Protocol *P):
    QDialog(parent)
{
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItem *item_ch_ch;

    setStyleSheet("QSlider::sub-page:qlineargradient {background: black;}");
    setFont(qApp->font());

    readCommonSettings();

    QStringList pp;
    pp << tr("Property") << tr("Value") << tr("Comments");

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Protocol preferences"));
    resize(630,300);


    QVBoxLayout *main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setMargin(2);

    property_list = new QTreeWidget(this);
    property_list->clear();
    property_list->setColumnCount(3);
    property_list->setHeaderLabels(pp);

    property_list->setStyleSheet("QPushButton {min-width: 2em; margin:1; padding:0;}"
                                 //"QComboBox {min-width:4em; margin:1; padding:1;"
                                 );
    property_list->setFont(qApp->font());

    QHBoxLayout *layout_control = new QHBoxLayout();
    layout_control->setMargin(2);
    cancel_button = new QPushButton(tr("Cancel"), this);
    apply_button = new QPushButton(tr("Apply"), this);
    layout_control->addWidget(apply_button,1,Qt::AlignRight);
    layout_control->addWidget(cancel_button,0,Qt::AlignRight);
    apply_button->setFocusPolicy(Qt::NoFocus);
    cancel_button->setFocusPolicy(Qt::NoFocus);
    apply_button->setDisabled(true);

    main_layout->addWidget(property_list,1);
    main_layout->addLayout(layout_control);

    //1. Software properties
    item = new QTreeWidgetItem(property_list);
    item->setText(0, tr("Analysis Properties:"));

    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Use CrossTalk compensation"));
    item_child->setCheckState(1,Qt::Checked);

    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Use Optical correction"));
    item_child->setCheckState(1,Qt::Checked);

    item_child = new QTreeWidgetItem(item);
    item_child->setText(0,tr("Digital Filter"));
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("use median digital filter"));
    item_ch_ch->setCheckState(1,Qt::Checked);
    item_ch_ch = new QTreeWidgetItem(item_child);
    item_ch_ch->setText(0,tr("spline parameter"));
    param_Spline = new QSlider(Qt::Horizontal);
    param_Spline->setMinimum(0);
    param_Spline->setMaximum(100);
    param_Spline->setSingleStep(1);
    param_Spline->setValue(30);
    //param_Spline->setStyleSheet("QSlider::sub-page:qlineargradient {background: black;}");

    property_list->setItemWidget(item_ch_ch, 1, (QSlider*)param_Spline);
    item_child->setExpanded(true);

    item->setExpanded(true);    

    //3. Default values
    back_Default = new QPushButton(tr("Set Default values..."));
    item = new QTreeWidgetItem(property_list);
    item = new QTreeWidgetItem(property_list);
    property_list->setItemWidget(item, 0, back_Default);

    property_list->resizeColumnToContents(0);

    connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(apply_button, SIGNAL(clicked(bool)), this, SLOT(Save_Properties()));
    connect(back_Default, SIGNAL(clicked()), this, SLOT(Set_DefaultValues()));

    prot = P;
    List << USE_CROSSTALK << USE_OPTICALCORRECTION << USE_DIGITFILTER << PARAM_SPLINECUBE;
    Load_Properties();
    Change_ParamSpline();

    connect(property_list, SIGNAL(itemChanged(QTreeWidgetItem*,int)), this, SLOT(Enable_Changes()));
    connect(param_Spline, SIGNAL(valueChanged(int)), this, SLOT(Enable_Changes()));
    connect(param_Spline, SIGNAL(valueChanged(int)), this, SLOT(Change_ParamSpline()));
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Common_Pro::~Common_Pro()
{

    delete cancel_button;
    delete apply_button;
    delete param_Spline;
    delete back_Default;

    property_list->clear();
    delete property_list;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Common_Pro::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();

        if(translator.load(":/translations/Common_Pro_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common_Pro::Load_Properties()
{
    int i,j;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QString text;
    rt_Preference *preference;
    double dvalue;
    int value;
    bool ok;

    if(prot == NULL) return;

    item = property_list->topLevelItem(0);

    for(i=0; i<prot->preference_Pro.size(); i++)
    {
        preference = prot->preference_Pro.at(i);

        j = -1;
        foreach(text, List)
        {
            j++;
            if(preference->name == text.toStdString())
            {
                if(text == PARAM_SPLINECUBE)
                {
                    dvalue = QString::fromStdString(preference->value).toDouble(&ok);
                    if(!ok) dvalue = 30;
                    value = dvalue * 1;
                    param_Spline->setValue(value);
                    break;
                }
                if(text == USE_DIGITFILTER) item_child = item->child(j)->child(0);
                else item_child = item->child(j);
                if(preference->value != "yes") item_child->setCheckState(1,Qt::Unchecked);
                break;
            }
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common_Pro::Save_Properties()
{
    int i,j;
    rt_Preference *preference;
    QString text;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    bool find;
    double dvalue;

    if(prot == NULL) {reject(); return;}

    item = property_list->topLevelItem(0);

    for(j=0; j<List.size(); j++)
    {        
        text = List.at(j);
        if(text == PARAM_SPLINECUBE) item_child = item->child(2)->child(1);
        else
        {
            if(text == USE_DIGITFILTER) item_child = item->child(2)->child(0);
            else item_child = item->child(j);
        }
        find = false;

        for(i=0; i<prot->preference_Pro.size(); i++)
        {

            preference = prot->preference_Pro.at(i);
            if(preference->name == text.toStdString())
            {
                find = true;
                if(text == PARAM_SPLINECUBE)
                {
                    dvalue = (double)(param_Spline->value())/1.;
                    preference->value = QString("%1").arg(dvalue,0,'f',0).toStdString();
                }
                else
                {
                    if(item_child->checkState(1) == Qt::Checked) preference->value = "yes";
                    else preference->value = "no";
                }
                break;
            }
        }

        if(!find)
        {
            preference = new rt_Preference;
            preference->name = text.toStdString();
            if(text == PARAM_SPLINECUBE)
            {
                dvalue = (double)(param_Spline->value())/1.;
                preference->value = QString("%1").arg(dvalue,0,'f',0).toStdString();
            }
            else
            {
                if(item_child->checkState(1) == Qt::Checked) preference->value = "yes";
                else preference->value = "no";
            }
            prot->preference_Pro.push_back(preference);
        }
    }

    accept();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common_Pro::Enable_Changes()
{
    apply_button->setDisabled(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Common_Pro::Set_DefaultValues()
{
    int i,j;
    QString text,name;
    QTreeWidgetItem *item;
    QTreeWidgetItem *item_child;
    QTreeWidgetItem *item_ch_ch;
    QWidget *obj;


    item = property_list->topLevelItem(0);
    item->child(0)->setCheckState(1, Qt::Checked);
    item->child(1)->setCheckState(1, Qt::Checked);
    item_child = item->child(2);
    item_child->child(0)->setCheckState(1, Qt::Checked);
    param_Spline->setValue(30);

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
void Common_Pro::Change_ParamSpline()
{
    QString text;
    QTreeWidgetItem *item;
    int value;
    int val_100 = param_Spline->maximum() - param_Spline->minimum();


    value = param_Spline->value() - param_Spline->minimum();
    value = value*100./val_100;

    item = property_list->topLevelItem(0)->child(2)->child(1);
    text = QString("%1%").arg(value);
    item->setText(2, text);

}
