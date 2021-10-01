#include "sp.h"



//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
SplashScreen::SplashScreen(QApplication *aApp, QWidget *parent, int param) :
    QSplashScreen(parent), app(aApp), m_progress(0)

{
    WCHAR buffer[1024];
    DWORD buf_size = 1024;
    QString name_PC;
    QString version_info;
    QString ver, text;
    QString app_name;

    readCommonSettings();
    version_info = GetVersion();

    qDebug() << "version_info: " << version_info;
    QCoreApplication::setApplicationVersion(version_info.split("\t").at(0));

    QFont f("Times New Roman", 32, QFont::Bold);
    //QPixmap pix(":/images/dna_2.ico");
    //QPixmap pix_logo(":/images/dna_logo_eng_3.png");
    //QPixmap pix_logo_rus(":/images/dna_logo_rus_3.png");
    QPixmap pix_logo(":/images/logo_en.png");
    QPixmap pix_logo_rus(":/images/logo_ru.png");

    setStyleSheet(
                "QProgressBar:horizontal {border: 1px solid transparent; background: #DD3A3A61; padding: 1px;}"
                "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #A4A4C8, stop: 1 #3A3A61);}"
                //"QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #CCCCFF, stop: 1 #6464FF);}"
                "QGroupBox {border: 1px solid black; background: #DD3A3A61; border-radius: 3px; margin-top: 1ex;}"
                "QLabel#Gray {color: #EEE;}"
                 );


#ifdef CALIBRATION
    /*setStyleSheet(
                "QProgressBar:horizontal {border: 1px solid gray; background: #99FAFAFA; padding: 1px;}"
                "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #E4C5C5, stop: 1 #7A3636);}"
                //"QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #CCCCFF, stop: 1 #6464FF);}"
                "QGroupBox {border: 1px solid gray; background: #88FAFAFA; border-radius: 3px; margin-top: 1ex;}"
                "QLabel#Gray {color: #404040;}"
                 );*/
    this->setPixmap(QPixmap(":/images/Splash BG.png"));
    //this->setPixmap(QPixmap(":/images/dna_start_red_w.png"));
    //pix.load(":/images/dna_2_red.ico");
    //pix_logo.load(NULL);
    //pix_logo_rus.load(NULL);
#else
    //this->setPixmap(QPixmap(":/images/dna_start_w.png"));
    this->setPixmap(QPixmap(":/images/Splash BG.png"));
    //this->setPixmap(QPixmap(":/images/Splash BG_temp.png"));
#endif


    if(param)
    {
        setWindowFlags(Qt::Tool);
        setWindowTitle(tr("About"));
    }
    else
    {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Dialog);
        setCursor(Qt::BusyCursor);
    }

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->setMargin(4);

    QHBoxLayout *main_layout = new QHBoxLayout;
    main_layout->setMargin(2);

    box = new QGroupBox(this);
    box->setFixedWidth(this->width() * 0.98);
    box->setFixedHeight(100);
    box->move((this->width()-box->width())/2., (this->height()-box->height())/2.5 + 20);
    box->setLayout(main_layout);
    int dx = (this->width()-box->width())/2.;

    //layout->addWidget(box);

    if(!param)
    {
        progress = new QProgressBar(box);
        progress->setMinimum(0);
        progress->setMaximum(100);
        progress->setFixedWidth(box->width() * 0.98);
        progress->setFixedHeight(12);
        progress->setTextVisible(false);
        progress->move((box->width()-progress->width())/2., box->height()*0.85);
    }

    //icon = new QLabel(box);
    //icon->setPixmap(pix);

    ver = version_info.split("\t").at(0);
    app_name = "  " + QString(APP_NAME) + "  " + ver.split(".").at(0) + "." + ver.split(".").at(1);
    name = new QLabel(app_name, box);   //"DTmaster 9.1"
    name->setObjectName("Gray");
    name->setFont(f);
    //name->move(70,25);

    ::GetComputerNameW(buffer, &buf_size);
    name_PC = QString::fromStdWString(buffer);

    //main_layout->addWidget(icon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    main_layout->addWidget(name, 1, Qt::AlignLeft | Qt::AlignVCenter);

    QVBoxLayout *v_layout = new QVBoxLayout;
    v_layout->setSpacing(1);
    web_address = new QLabel(name_PC, box);
    web_address->setObjectName("Gray");
    //version = new QLabel(tr("version:") + " " + version_info.split("\t").at(0) + " (32bit)", box);
    text = tr("version:") + " " + ver.split(".").at(2) + "." + ver.split(".").at(3) + " (32bit)";
    version = new QLabel(text, box);
    version->setObjectName("Gray");
    last_mod = new QLabel(tr("last mod.:") + " " + version_info.split("\t").at(1), box);
    last_mod->setObjectName("Gray");
    //limited = new QLabel("", box);
    //limited->setObjectName("Gray");
    //if(type_reagent) limited->setText("+ limited");


    v_layout->addWidget(web_address, 0, Qt::AlignRight);
    v_layout->addStretch(2);
    //v_layout->addWidget(limited, 0, Qt::AlignLeft);
    v_layout->addWidget(version, 0, Qt::AlignLeft);
    v_layout->addWidget(last_mod, 0, Qt::AlignLeft);
    if(!param)
    {
        QLabel *label = new QLabel("", box);
        v_layout->addWidget(label, 1, Qt::AlignLeft);
    }

    main_layout->addLayout(v_layout);   

    box_web = new QGroupBox(this);
    box_web->setFixedHeight(50);
    box_web->setFixedWidth(150);
    box_web->setStyleSheet("border: 1px solid transparent; background: transparent;");
    QHBoxLayout *h_layout = new QHBoxLayout;
    box_web->setLayout(h_layout);
    h_layout->setMargin(0);

    logo = new QLabel(box_web);
    //logo->setScaledContents(true);
    //logo->setFixedHeight(40);
    //logo->setFixedWidth(40*4.11);
    if(lang == "ru") logo->setPixmap(pix_logo_rus);
    else logo->setPixmap(pix_logo);
    h_layout->addWidget(logo,0, Qt::AlignCenter);

    load_Tests = new QLabel("", this);
    load_Tests->setObjectName("Gray");
    tests_progress = new QProgressBar(this);
    tests_progress->setMinimum(0);
    tests_progress->setMaximum(100);
    //tests_progress->setFixedWidth(this->width() * 0.70);
    tests_progress->setFixedHeight(12);
    tests_progress->setTextVisible(false);
    tests_progress->setVisible(false);
    tests_progress->setStyleSheet(
        "QProgressBar:horizontal {border: 1px solid transparent; background: #DD3A3A61; padding: 1px;}"
        "QProgressBar::chunk:horizontal {background: qlineargradient(x1: 0, y1: 0.1, x2: 1, y2: 0.1, stop: 0 #3A3A61, stop: 1 #A4A4C8);}");
    QHBoxLayout *tests_layout = new QHBoxLayout;
    tests_layout->addWidget(load_Tests, 0, Qt::AlignLeft);
    tests_layout->addWidget(tests_progress, 1);


    //box_web->move(this->width()-box_web->width()-dx, dx);
    web_info = new QLabel("www.dna-technology.ru", this);
    web_info->setObjectName("Gray");
    factory = new QLabel(tr("ООО \"НПО ДНК-Технология\" 2019"), this);
    factory->setObjectName("Gray");
    QHBoxLayout *web_layout = new QHBoxLayout;
    web_layout->addWidget(factory, 0, Qt::AlignLeft);
    web_layout->addWidget(web_info, 1, Qt::AlignRight);

    layout->addWidget(box_web, 0, Qt::AlignLeft);
    //layout->addSpacing(220);
    layout->addStretch(1);
    //layout->addWidget(load_Tests, 0, Qt::AlignLeft);
    layout->addLayout(tests_layout);
    layout->addLayout(web_layout);

    restart = false;

    if(!param)
    {
        timer = new QTimer();
        connect(timer,SIGNAL(timeout()),this,SLOT(setProgress()));
        timer->start(100);
    }

    int width = this->width();
    int height = this->height();
    setFixedSize(width, height);
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void SplashScreen::readCommonSettings()
{
    QString text;
    bool ok;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        lang = text;

        //... Type ...
        text = CommonSettings->value("type","0").toString();
        type_reagent = text.toInt(&ok);
        if(!ok) type_reagent = 0;




    CommonSettings->endGroup();
    delete CommonSettings;
}

//-----------------------------------------------------------------------------
//--- GetVersion()
//-----------------------------------------------------------------------------
QString SplashScreen::GetVersion()
{
    QString res = "...\t...";
    QString str,cmd;
    QString exe_name = QString(APP_NAME) + ".exe";

//#ifndef CALIBRATION
    exe_name = "about_moduls.dll";
//#endif

    UINT Size,Len;
    UINT h;
    int lang_page;
    BYTE* buf;
    char *ValueBuf;

    QFileInfo fi;
    QDir modules_Dir(qApp->applicationDirPath());

    fi.setFile(modules_Dir, exe_name); //"RT_PCR.exe");
    QString fileName = fi.absoluteFilePath();
    //qDebug() << "fn: " << fileName;

    Size = GetFileVersionInfoSizeA(fileName.toStdString().c_str(), NULL);
    if(Size)
    {
        buf = new BYTE[Size];
        if(GetFileVersionInfoA(fileName.toStdString().c_str(), h, Size, buf) != 0)
        {
            VerQueryValueA(buf,"\\VarFileInfo\\Translation", (LPVOID*)&ValueBuf, &Len);
            if(Len >= 4)
            {
                lang_page = (int)MAKELONG(*(int *)(ValueBuf+2), *(int *)ValueBuf);
                str = QString("%1").arg(lang_page,8,16,QLatin1Char('0'));
                cmd = "\\StringFileInfo\\" + str + "\\FileVersion";
                if(VerQueryValueA(buf, cmd.toStdString().c_str(), (LPVOID*)&ValueBuf, &Len) != 0)
                {
                    res = QString::fromStdString(ValueBuf);
                    res += "\t" + fi.lastModified().toString("dd.MM.yyyy");
                }
            }
        }
    }
    delete buf;

    return(res);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void SplashScreen::Get_SplashPercent(QString str)
{
    QStringList list = str.split(",");
    int count = list.at(1).toInt();
    int current = list.at(0).toInt();

    //qDebug() << "i:" << str << current << count;
    load_Tests->setText(QString("%1: %2(%3)   ").arg(tr("load tests")).arg(current).arg(count));

    if(!tests_progress->isVisible())
    {
        tests_progress->setVisible(true);
        if(progress && progress->isVisible()) progress->setVisible(false);
    }

    if(count)
    {
        tests_progress->setValue((current*100)/count);
        tests_progress->update();
        tests_progress->repaint();
    }

    //load_Tests->update();
    //load_Tests->repaint();
}
