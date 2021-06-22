#include "info.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------

Info::Info(QWidget *parent): QDialog(parent)
{
    QList<int> list;
    list << 350 << 250;

    readCommonSettings();
    setFont(qApp->font());

    setWindowTitle(tr("Information about Protocol"));
    //setWindowIcon(QIcon(":/images/info.png"));
    //setFixedSize(600,250);
    setMinimumWidth(600);
    setMinimumHeight(250);
    resize(600,250);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    QVBoxLayout *main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setMargin(1);
    main_layout->setSpacing(1);

    TabInfo = new QTabWidget(this);
    info_pro = new Info_Protocol(this);
    program_spl = new QSplitter(Qt::Horizontal, this);
    program_spl->setHandleWidth(4);
    program_spl->setChildrenCollapsible(false);
    plot_Program = new Plot_ProgramAmpl(this);
    scheme_Program = new Scheme_ProgramAmpl(QColor(1,1,1,1), this);
    program_spl->addWidget(plot_Program);
    program_spl->addWidget(scheme_Program);
    program_spl->setSizes(list);
    comments = new QTextEdit(this);
    comments->copyAvailable(true);

    TabInfo->addTab(info_pro, tr("Protocol"));
    TabInfo->addTab(program_spl, tr("Program"));
    TabInfo->addTab(comments, tr("Comments"));

    main_layout->addWidget(TabInfo);


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Info::~Info()
{
    delete comments;
    delete program_spl;
    delete info_pro;
    delete TabInfo;
}
//-------------------------------------------------------------------------------
//--- showEvent
//-------------------------------------------------------------------------------
void Info::showEvent(QShowEvent *e)
{
    //qDebug() << "size:" << program_spl->sizes();
}
//-------------------------------------------------------------------------------
//--- refresh_Info(rt_Protocol *p)
//-------------------------------------------------------------------------------
void Info::refresh_Info(rt_Protocol *p)
{
    rt_Preference   *property;

    info_pro->clear_Info();
    info_pro->fill_Info(p);

    plot_Program->clear_Program();
    plot_Program->draw_Program(p);

    scheme_Program->draw_Program(p);

    comments->clear();
    foreach(property, p->preference_Pro)
    {
        if(property->name == "Comments")
        {
            comments->setPlainText(QString::fromStdString(property->value));
            break;
        }
    }

    TabInfo->setCurrentIndex(0);

    //qDebug() << "refresh_Info: " << p;

}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Info::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/info_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
