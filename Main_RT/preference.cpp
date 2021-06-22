#include "preference.h"

General::General(QWidget *parent) : QDialog(parent)
{
    //readCommonSettings();

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("General preference"));
    //setWindowIcon(QIcon(":/images/add_sample.png"));
    setFixedSize(600,320);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(2);
    layout->setSpacing(2);
    this->setLayout(layout);

    TabWidget = new QTabWidget(this);

    Interface = new QGroupBox(this);
    Interface->setObjectName("Transparent");
    QVBoxLayout *layout_interface = new QVBoxLayout();
    Interface->setLayout(layout_interface);
    QHBoxLayout *lang_layout = new QHBoxLayout();
    QHBoxLayout *font_layout = new QHBoxLayout();
    QHBoxLayout *style_layout = new QHBoxLayout();

    font_button = new QPushButton("font", this);
    font_label = new QLabel(tr("Application font"));


    lang_box = new QComboBox(this);
    lang_box->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    lang_box->insertItem(0,QIcon(":/images/ru_flag.png"),tr("russian"));
    lang_box->insertItem(1,QIcon(":/images/eng_flag.png"),tr("english"));

    lang_box->setIconSize(QSize(32,24));

    lang_label = new QLabel(tr("Application language"), this);
    style_box = new QComboBox(this);
    style_box->setStyleSheet("background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;");
    style_box->insertItem(0,tr("Win_XP,Vista"));
    style_box->insertItem(1,tr("Fusion"));
    style_box->setEnabled(false);
    style_label = new QLabel(tr("Application style"), this);

    lang_layout->addWidget(lang_box);
    lang_layout->addWidget(lang_label,0,Qt::AlignRight);
    font_layout->addWidget(font_button);
    font_layout->addWidget(font_label,0,Qt::AlignRight);
    style_layout->addWidget(style_box);
    style_layout->addWidget(style_label,0,Qt::AlignRight);

    layout_interface->addSpacing(5);
    layout_interface->addLayout(lang_layout);
    layout_interface->addLayout(font_layout);
    layout_interface->addLayout(style_layout);
    layout_interface->addStretch(1);

    //... Template ...
    Template_NameProtocol = new QGroupBox(this);
    Template_NameProtocol->setObjectName("Transparent");
    QVBoxLayout *layout_template = new QVBoxLayout();
    Template_NameProtocol->setLayout(layout_template);
    Template = new QGroupBox(tr("Users template"), this);
    Template->setObjectName("Transparent");
    Template->setCheckable(true);
    Template->setChecked(false);
    layout_template->addWidget(Template);
    QVBoxLayout *layout_Template = new QVBoxLayout();
    Template->setLayout(layout_Template);

    QHBoxLayout *layout_name = new QHBoxLayout();
    QHBoxLayout *layout_type = new QHBoxLayout();
    QHBoxLayout *layout_date = new QHBoxLayout();
    QHBoxLayout *layout_example = new QHBoxLayout();
    layout_name->setMargin(0);
    layout_type->setMargin(0);
    layout_date->setMargin(0);
    layout_example->setMargin(0);

    name_label = new QLabel(tr(" 1. Name:"), this);
    type_label = new QLabel(tr(" 2. Type plate:"), this);
    date_label = new QLabel(tr(" 3. Date/Time: (dd-MM-yy/hh:mm:ss)"), this);
    name_edit = new QLineEdit(this);
    name_edit->setFixedWidth(150);
    type_box = new QCheckBox(this);
    date_edit = new QLineEdit(this);
    date_edit->setFixedWidth(150);
    example = new QPushButton(tr("Example:"), this);
    example_label = new QLabel(this);
    //example_label->setFixedWidth(220);

    layout_name->addWidget(name_label,0,Qt::AlignLeft);
    layout_name->addWidget(name_edit,0,Qt::AlignRight);
    layout_type->addWidget(type_label,0,Qt::AlignLeft);
    layout_type->addWidget(type_box,0,Qt::AlignRight);
    layout_date->addWidget(date_label,0,Qt::AlignLeft);
    layout_date->addWidget(date_edit,0,Qt::AlignRight);

    layout_example->addWidget(example,0,Qt::AlignLeft);
    layout_example->addWidget(example_label,1,Qt::AlignRight);

    layout_Template->addSpacing(5);
    layout_Template->addLayout(layout_name);
    layout_Template->addLayout(layout_type);
    layout_Template->addLayout(layout_date);
    layout_Template->addStretch(1);
    layout_Template->addLayout(layout_example);

    //...

    TabWidget->addTab(Interface,tr("Interface Parameters"));
    TabWidget->addTab(Template_NameProtocol,tr("Protocol name template"));


    QHBoxLayout *ctrl_layout = new QHBoxLayout();
    apply_button = new QPushButton(tr("Apply"), this);
    apply_button->setDisabled(false);
    cancel_button = new QPushButton(tr("Cancel"), this);
    cancel_button->setFocus();
    ctrl_layout->addStretch(1);
    ctrl_layout->addWidget(apply_button,0,Qt::AlignRight);
    ctrl_layout->addWidget(cancel_button,0,Qt::AlignRight);
    ctrl_layout->setSpacing(2);


    layout->addWidget(TabWidget,1);
    layout->addLayout(ctrl_layout);

    connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(apply_button, SIGNAL(clicked(bool)), this, SLOT(slot_Apply()));
    connect(lang_box, SIGNAL(currentIndexChanged(int)), this, SLOT(change_Language(int)));
    connect(style_box, SIGNAL(currentIndexChanged(int)), this, SLOT(change_Style(int)));
    connect(font_button, SIGNAL(clicked(bool)), this, SLOT(load_Font()));
    connect(example, SIGNAL(clicked(bool)), this, SLOT(example_Template()));

    connect(name_edit, SIGNAL(textChanged(QString)), this, SLOT(change_NameEdit(QString)));
    connect(date_edit, SIGNAL(textChanged(QString)), this, SLOT(change_DateEdit(QString)));

    readCommonSettings();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
General::~General()
{
    lang_box->clear();
    style_box->clear();

    delete TabWidget;

    delete apply_button;
    delete cancel_button;

}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void General::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);
    id_lang = 1;
    id_style = 0;
    id_type = 0;    // base,limited,oem
    bool ok;

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        if(text.trimmed() == "ru") id_lang = 0;

        //... Font ...
        text = CommonSettings->value("font","").toString();
        id_font = text;

        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        if(text.trimmed() == "fusion") id_style = 1;

        //... Type ...
        text = CommonSettings->value("type","0").toString();
        id_type = text.toInt(&ok);
        if(!ok) id_type = 0;


    CommonSettings->endGroup();

    CommonSettings->beginGroup("Template_NameProtocol");

        //... use template ...
        text = CommonSettings->value("use_template","yes").toString();
        if(text == "yes") Template->setChecked(true);
        else Template->setChecked(false);

        //... name ...
        text = CommonSettings->value("name_template","Protocol").toString();
        name_edit->setText(text);

        //... type ...
        text = CommonSettings->value("use_type","yes").toString();
        if(text == "yes") type_box->setChecked(true);
        else type_box->setChecked(false);

        //... date ...
        text = CommonSettings->value("date_template","").toString();
        date_edit->setText(text);

        //... example ...
        example_label->setText("");


    CommonSettings->endGroup();

    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//--- showEvent()
//-----------------------------------------------------------------------------
void General::showEvent(QShowEvent *event)
{
    lang_box->blockSignals(true);
    lang_box->setCurrentIndex(id_lang);
    lang_box->blockSignals(false);

    style_box->blockSignals(true);
    style_box->setCurrentIndex(id_style);
    style_box->blockSignals(false);

    drawFont_OnButton(id_font);
}
//-----------------------------------------------------------------------------
//--- slot_Apply()
//-----------------------------------------------------------------------------
void General::slot_Apply()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

    switch(id_lang)
    {
    default:    text = "en";    break;
    case 0:     text = "ru";    break;
    case 1:     text = "en";    break;
    }
    CommonSettings->setValue("language",text);    
    CommonSettings->setValue("font", id_font);

    switch(id_style)
    {
    default:
    case 0:     text = "xp";    break;
    case 1:     text = "fusion";    break;
    }
    CommonSettings->setValue("style",text);

    CommonSettings->setValue("type", QString::number(id_type));

    CommonSettings->endGroup();

    CommonSettings->beginGroup("Template_NameProtocol");

    if(Template->isChecked()) text = "yes";
    else                      text = "no";
    CommonSettings->setValue("use_template", text);

    CommonSettings->setValue("name_template", name_edit->text());
    CommonSettings->setValue("date_template", date_edit->text());
    if(type_box->isChecked()) text = "yes";
    else text = "no";
    CommonSettings->setValue("use_type", text);

    CommonSettings->endGroup();

    delete CommonSettings;

    close();
}
//-----------------------------------------------------------------------------
//--- change_Language(int lang)
//-----------------------------------------------------------------------------
void General::change_Language(int lang)
{
    apply_button->setDisabled(false);
    id_lang = lang;
}
//-----------------------------------------------------------------------------
//--- change_Style(int style)
//-----------------------------------------------------------------------------
void General::change_Style(int style)
{
    apply_button->setDisabled(false);
    id_style = style;
}
//-----------------------------------------------------------------------------
//--- load_Font()
//-----------------------------------------------------------------------------
void General::load_Font()
{
    bool ok;
    QString text;
    int w;

    QFont f = QFontDialog::getFont(&ok,this);
    if(ok)
    {
        apply_button->setDisabled(false);

        text = QString("%1\t%2\t%3").arg(f.family()).arg(f.pointSize()).arg(f.weight());
        id_font = text;
        drawFont_OnButton(id_font);
    }
}
//-----------------------------------------------------------------------------
//--- drawFont_OnButton
//-----------------------------------------------------------------------------
void General::drawFont_OnButton(QString str)
{
    QString text;
    int s,w;
    QStringList list = str.split("\t");
    if(list.size() < 3) return;

    text = list.at(0);
    str = text.trimmed();
    text = list.at(1);
    s = text.toInt();
    str += ", " + text;
    text = list.at(2);
    w = text.toInt();
    text = "Normal";
    if(w > 50) text = "Bold";
    str += ", " + text;
    font_button->setText(str);

    QFont f(list.at(0), s, w);
    font_button->setFont(f);
}
//-----------------------------------------------------------------------------
//--- example_Template()
//-----------------------------------------------------------------------------
void General::example_Template()
{
    QString name_str = name_edit->text() + "_";
    QString type_str = "(384)_";
    QString date_str = QDateTime::currentDateTime().toString(date_edit->text());

    if(!type_box->isChecked()) type_str = "";
    QString res = name_str + type_str + date_str;

    example_label->setText(res);
}
//-----------------------------------------------------------------------------
//--- change_NameEdit(QString text)
//-----------------------------------------------------------------------------
void General::change_NameEdit(QString text)
{
    int i;
    QChar ch;
    QVector<QChar> vec_char;

    vec_char << '_' << '-' << '(' << ')' << ':';

    for(i=text.length()-1; i>=0; i--)
    {
        ch = text.at(i);
        if(ch.isLetterOrNumber()) continue;
        if(vec_char.contains(ch)) continue;

        text.remove(i,1);
    }
    if(text.length() > 30) text = text.mid(0,30);

    name_edit->blockSignals(true);
    name_edit->setText(text);
    name_edit->blockSignals(false);
}
//-----------------------------------------------------------------------------
//--- change_DateEdit(QString text)
//-----------------------------------------------------------------------------
void General::change_DateEdit(QString text)
{
    int i;
    QChar ch;
    QVector<QChar> vec_char;

    vec_char << '_' << '-' << '(' << ')' << ':';

    for(i=text.length()-1; i>=0; i--)
    {
        ch = text.at(i);
        if(ch.isLetterOrNumber()) continue;
        if(vec_char.contains(ch)) continue;

        text.remove(i,1);
    }
    if(text.length() > 30) text = text.mid(0,30);

    date_edit->blockSignals(true);
    date_edit->setText(text);
    date_edit->blockSignals(false);
}
