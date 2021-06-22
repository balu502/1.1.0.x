#include "aboutmoduls.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
AboutModuls::AboutModuls(QWidget *parent): QDialog(parent)
{
    readCommonSettings();
    setFont(qApp->font());

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("About Moduls"));
    resize(500,550);

    QVBoxLayout *main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setMargin(2);

    info = new QTreeWidget(this);
    info->setFont(qApp->font());
    close_button = new QPushButton(tr("Close"), this);
    connect(close_button, SIGNAL(clicked(bool)), this, SLOT(close()));

    main_layout->addWidget(info,1);
    main_layout->addWidget(close_button,0,Qt::AlignRight);
    close_button->setFocusPolicy(Qt::NoFocus);

    Fill_Info();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
AboutModuls::~AboutModuls()
{
    info->clear();
    delete info;
    delete close_button;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void AboutModuls::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/aboutmoduls_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void AboutModuls::Fill_Info()
{
    QStringList pp;
    QFileInfo fi;
    QString fn, text;
    QTreeWidgetItem *item;

    UINT Size,Len;
    UINT h;
    int lang_page;
    BYTE* buf;
    char *ValueBuf;
    QString fileName,str,cmd;

    pp << tr("Name") << tr("Version") << tr("Size") << tr("Last modification");
    info->setColumnCount(4);
    info->setHeaderLabels(pp);

    Files_Version("");
    Files_Version("analyser_plugins");
    Files_Version("test_plugins");
    Files_Version("report_plugins");
    Files_DTR("forms");

    //... DTReportTools.dll
    char path[MAX_PATH] = "";
    SHGetSpecialFolderPathA(NULL, path, CSIDL_SYSTEMX86, false); //CSIDL_SYSTEMX86 == 0x0029
    QString Path = QString::fromUtf8(path);
    Path += "\\DTReportTools.dll";
    fi.setFile(Path);
    //qDebug() << "DTReportTools.dll: " << fi.exists();

    if(fi.isFile())
    {
        item = new QTreeWidgetItem(info);
        fn = fi.fileName();
        item->setText(0, fn);

        item->setText(1, "...");        
        fileName = fi.absoluteFilePath();
        Size = GetFileVersionInfoSizeA(fi.absoluteFilePath().toStdString().c_str(), NULL);
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
                        cmd = QString::fromStdString(ValueBuf);
                        item->setText(1, cmd);
                    }
                }
            }
            delete buf;
        }

        text = QString::number(fi.size());
        item->setText(2, text);
        text = fi.lastModified().toString("dd.MM.yyyy");
        item->setText(3, text);
    }
    //...

    info->resizeColumnToContents(0);
    info->setColumnWidth(0,info->columnWidth(0)*1.3);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void AboutModuls::Files_DTR(QString dir_name)
{
    QString text;
    QString fn;
    QTreeWidgetItem *item_current;
    QTreeWidgetItem *item_ch;
    QTreeWidgetItem *item;

    char *DisplayName;
    char *Path;
    int release, build;

    QFileInfo fi;
    QDir modules_Dir(qApp->applicationDirPath());

    if(!dir_name.isEmpty())
    {
        modules_Dir.setPath(modules_Dir.absolutePath() + "\\" + dir_name);
        item = new QTreeWidgetItem(info);
        item->setText(0,dir_name);
        item->setExpanded(false);
    }

    get_Info = NULL;
    ext_dll_handle = ::LoadLibraryW(L"DTReport2.dll");
    if(ext_dll_handle)
    {
        get_Info = (GetDTRDisplayInfo)(::GetProcAddress(ext_dll_handle,"GetDTRDisplayInfo"));
        if(get_Info)
        {
        foreach (QString fileName, modules_Dir.entryList(QDir::Files))
        {
            fi.setFile(modules_Dir, fileName);
            if(fi.suffix() == "dtr")
            {
                fn = fi.fileName();
                fileName = fi.absoluteFilePath();
                text = fi.baseName();

                if(dir_name.isEmpty())
                {
                    item = new QTreeWidgetItem(info);
                    item_current = item;
                }
                else
                {
                    item_ch = new QTreeWidgetItem(item);
                    item_current = item_ch;
                }

                // 1. Name
                item_current->setText(0, fn);

                // 2. Version
                get_Info(text.toLatin1().data(), &DisplayName, &Path, &release, &build);
                item_current->setText(1, QString("%1.%2").arg(release).arg(build));

                text = QString(" %1:    %2").arg(QString(DisplayName)).arg(QString(Path));
                //qDebug() << text;

                // 3. Size
                text = QString::number(fi.size());
                item_current->setText(2, text);

                // 4. Last modify
                text = fi.lastModified().toString("dd.MM.yyyy");
                item_current->setText(3, text);
            }
        }
        }
        ::FreeLibrary(ext_dll_handle);
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void AboutModuls::Files_Version(QString dir_name)
{
    QString text;
    QString fn,str,cmd;
    QTreeWidgetItem *item_current;
    QTreeWidgetItem *item_ch;
    QTreeWidgetItem *item;

    UINT Size,Len;
    UINT h;
    int lang_page;
    BYTE* buf;
    char *ValueBuf;

    QFileInfo fi;
    QDir modules_Dir(qApp->applicationDirPath());

    if(!dir_name.isEmpty())
    {
        modules_Dir.setPath(modules_Dir.absolutePath() + "\\" + dir_name);
        item = new QTreeWidgetItem(info);
        item->setText(0,dir_name);
        item->setExpanded(false);
    }

    foreach (QString fileName, modules_Dir.entryList(QDir::Files))
    {
        fi.setFile(modules_Dir, fileName);
        if(fi.suffix() == "exe" || fi.suffix() == "dll")
        {
            fn = fi.fileName();
            fileName = fi.absoluteFilePath();

            //if(dir_name.isEmpty()) qDebug() << fn;

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
                        cmd = "\\StringFileInfo\\" + str + "\\LegalCopyright";
                        if(VerQueryValueA(buf, cmd.toStdString().c_str(), (LPVOID*)&ValueBuf, &Len) != 0)
                        {
                            cmd = QString::fromStdString(ValueBuf);
                            if(cmd != "dna-technology" && fn != "DTReport2.dll") continue;

                            if(dir_name.isEmpty())
                            {
                                item = new QTreeWidgetItem(info);
                                item_current = item;
                            }
                            else
                            {
                                item_ch = new QTreeWidgetItem(item);
                                item_current = item_ch;
                            }

                            // 1. Name

                            item_current->setText(0, fn);

                            // 2. Version
                            cmd = "\\StringFileInfo\\" + str + "\\FileVersion";
                            if(VerQueryValueA(buf, cmd.toStdString().c_str(), (LPVOID*)&ValueBuf, &Len) != 0)
                            {
                                cmd = QString::fromStdString(ValueBuf);
                                item_current->setText(1, cmd);
                            }

                            // 3. Size
                            text = QString::number(fi.size());
                            item_current->setText(2, text);

                            // 4. Last modify
                            text = fi.lastModified().toString("dd.MM.yyyy");
                            item_current->setText(3, text);
                        }
                    }
                }
                delete buf;
            }
        }
    }
}
