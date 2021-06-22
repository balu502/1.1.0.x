#include "saver.h"

extern "C" SAVERSHARED_EXPORT Saver* __stdcall create_page()
{
    return(new Saver());
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Saver::Saver(QWidget *parent): QMainWindow(parent)
{    
    readCommonSettings();

    MainGroupBox = new QGroupBox();
    MainGroupBox->setObjectName("Transparent");
    setCentralWidget(MainGroupBox);
    QVBoxLayout *main_layout = new QVBoxLayout();
    main_layout->setMargin(2);
    main_layout->setSpacing(2);
    MainGroupBox->setLayout(main_layout);

    TabHelp = new QTabWidget(MainGroupBox);
    main_layout->addWidget(TabHelp);

    QPixmap pix(":/images/general_idea.png");
    main_Idea = new QLabel(this);
    main_Idea->setScaledContents(true);
    main_Idea->setPixmap(pix);

    Browser = new QTextBrowser(this);    

    TabHelp->insertTab(0, main_Idea, tr("General Idea"));
    TabHelp->insertTab(1, Browser, tr("Introduction ..."));

    Load_HTML();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Saver::~Saver()
{       
    delete Browser;
    delete main_Idea;
    delete TabHelp;
    delete MainGroupBox;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Saver::Load_HTML()
{
    QString path = qApp->applicationDirPath();
    QString page = path + "/RT_9.htm";
    Browser->setSearchPaths(QStringList() << path);
    Browser->setSource(QUrl::fromLocalFile(page));    
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Saver::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/saver_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-------------------------------------------------------------------------------
//--- User events
//-------------------------------------------------------------------------------
bool Saver::event(QEvent *e)
{
    if(e->type() == 3010)       // open Help tab
    {
        TabHelp->setCurrentIndex(1);
        return(true);
    }

    return QWidget::event(e);
}


