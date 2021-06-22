#include "sp_rt.h"

SplashScreen_RT::SplashScreen_RT(QApplication *aApp, QWidget *parent, int param) :    
    QSplashScreen(parent), app(aApp), m_progress(0)
{
    WCHAR buffer[1024];
    DWORD buf_size = 1024;
    QString name_PC, name_User;
    QString version_info;
    QString ver, text;
    QString app_name;

    QFont font = app->font();
    qDebug() << "sp_font: " << font;
    int size = font.pointSize();
    size--;
    font.setPointSize(size);
    setFont(font);

    QFont f("Times New Roman", 20, QFont::Bold);

    readCommonSettings();
    version_info = GetVersion();

    QCoreApplication::setApplicationVersion(version_info.split("\t").at(0));

    QPixmap pix_logo(":/images/logo_en.png");
    QPixmap pix_logo_rus(":/images/logo_ru.png");

    setPixmap(QPixmap(":/images/Splash_BG_RT.png"));

    if(param)
    {
        setWindowFlags(windowFlags() | Qt::Tool);
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

    ver = version_info.split("\t").at(0);
    app_name = "  " + QString(APP_NAME) + "  " + ver.split(".").at(0) + "." + ver.split(".").at(1);
    name = new QLabel(app_name, this);
    name->setFont(f);
    layout->addWidget(name, 0, Qt::AlignTop | Qt::AlignCenter);

    animation = new QLabel(this);
    gif = new QMovie(":/images/rt710.gif");
    animation->setMovie(gif);
    animation->setVisible(true);
    gif->start();
    animation->setFixedSize(100,160);
    QVBoxLayout *layout_gif = new QVBoxLayout;
    layout_gif->addWidget(animation, 0, Qt::AlignCenter);

    ::GetComputerNameW(buffer, &buf_size);
    name_PC = QString::fromStdWString(buffer);
    PC = new QLabel(this);
    PC->setText(tr("Computer: ") + name_PC);
    PC->setFont(font);
    ::GetUserNameW(buffer, &buf_size);
    name_User = QString::fromStdWString(buffer);
    User = new QLabel(this);
    User->setText(tr("User: ") + name_User);
    User->setFont(font);
    QVBoxLayout *layout_names = new QVBoxLayout;
    layout_names->setSpacing(2);
    layout_names->addWidget(PC, 0, Qt::AlignRight);
    layout_names->addWidget(User, 0, Qt::AlignRight);

    factory = new QLabel(tr("ООО \"НПО ДНК-Технология\" 2021"), this);
    email = new QLabel("Email:  hotline@dna-technology.ru", this);
    //email->setTextFormat(Qt::RichText);
    //email->setText("Email:href='mailto:hotline@dna-technology.ru'>hotline@dna-technology.ru</a>");
    //email->setOpenExternalLinks(true);

    www = new QLabel("URL:  www.dna-technology.ru", this);
    QVBoxLayout *layout_info = new QVBoxLayout;
    layout_info->setSpacing(2);
    layout_info->addWidget(factory, 0, Qt::AlignLeft);
    layout_info->addSpacing(20);
    layout_info->addWidget(email, 0, Qt::AlignLeft);
    layout_info->addWidget(www, 0, Qt::AlignLeft);

    QVBoxLayout *layout_ver_0 = new QVBoxLayout;
    layout_ver_0->addLayout(layout_names);
    layout_ver_0->addStretch(1);
    layout_ver_0->addLayout(layout_info);

    QHBoxLayout *layout_hor_0 = new QHBoxLayout;
    layout_hor_0->addLayout(layout_gif);
    layout_hor_0->addStretch(1);
    layout_hor_0->addLayout(layout_ver_0);
    layout_hor_0->addSpacing(5);

    version = new QLabel(this);
    text = tr("version:") + " " + ver.split(".").at(2) + "." + ver.split(".").at(3) + "  ";
    text += version_info.split("\t").at(1);
    version->setText(text);
    logo = new QLabel(this);
    if(lang == "ru") logo->setPixmap(pix_logo_rus);
    else logo->setPixmap(pix_logo);
    QHBoxLayout *layout_hor_1 = new QHBoxLayout;
    layout_hor_1->addSpacing(5);
    layout_hor_1->addWidget(version, 1, Qt::AlignLeft);
    layout_hor_1->addWidget(logo, 1, Qt::AlignRight);
    layout_hor_1->addSpacing(5);

    layout->addStretch(1);
    layout->addLayout(layout_hor_0);
    layout->addStretch(1);
    layout->addLayout(layout_hor_1);


    setFixedSize(400, 300);
    //setWindowIcon(QIcon(":/images/RT.ico"));
    setWindowIcon(QIcon(":/images/DTm.ico"));
}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void SplashScreen_RT::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        lang = text;

        //... Type ...
        text = CommonSettings->value("type","0").toString();



    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//--- GetVersion()
//-----------------------------------------------------------------------------
QString SplashScreen_RT::GetVersion()
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
