#include "httpprocess.h"
#include <w32api.h>


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
HttpProcess::HttpProcess(QWidget *parent): QDialog(parent)
{
    qDebug() << "HttpProcess start: ";
    readCommonSettings();

    QFont font = qApp->font();
    setFont(font);

    ax_user = NULL;

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->setMargin(2);
    layout->setSpacing(2);

    box = new QGroupBox(this);
    //box->setObjectName("Transparent");
    QHBoxLayout *layout_box = new QHBoxLayout;
    box->setLayout(layout_box);
    layout_box->setMargin(2);
    layout_box->setSpacing(2);

    http_input = new QRadioButton(tr("INPUT"), this);
    http_output = new QRadioButton(tr("OUTPUT"), this);    
    http_input->setChecked(true);
    http_input->setDisabled(true);
    http_output->setDisabled(true);

    reload_IP = new QPushButton(QIcon(":/images/reload_24.png"), "", this);
    reload_IP->setFocusPolicy(Qt::NoFocus);
    IP_edit = new QLineEdit("", this);
    IP_edit->setText(ip_address);
    //IP_edit->setFocusPolicy(Qt::NoFocus);
    IP_edit->setAlignment(Qt::AlignHCenter);
    //IP_edit->setReadOnly(false);
    IP_label = new QLabel(tr(""), this);

    layout_box->addWidget(reload_IP, 0, Qt::AlignLeft);
    layout_box->addWidget(IP_edit, 0, Qt::AlignLeft);
    layout_box->addWidget(IP_label, 0, Qt::AlignLeft);
    layout_box->addWidget(http_input, 1, Qt::AlignRight);
    layout_box->addWidget(http_output, 0, Qt::AlignRight);

    ListFiles = new QListWidget(this);
    //ListFiles->setViewMode(QListView::IconMode);

    QHBoxLayout *control_layout = new QHBoxLayout;
    control_layout->setMargin(2);
    control_layout->setSpacing(1);

    http_cancel = new QPushButton(tr("Cancel"), this);    
    http_delete = new QPushButton(tr("Delete"), this);
    http_delete->setDisabled(true);
    http_get = new QPushButton(tr("Get file"), this);
    http_get->setDisabled(true);
    http_put = new QPushButton(tr("Put file"), this);
    http_put->setDisabled(true);

    control_layout->addWidget(http_put, 1, Qt::AlignRight);
    control_layout->addWidget(http_get, 0, Qt::AlignRight);
    control_layout->addWidget(http_delete, 0, Qt::AlignRight);
    control_layout->addWidget(http_cancel, 0, Qt::AlignRight);

    http_cancel->setFocusPolicy(Qt::NoFocus);
    http_delete->setFocusPolicy(Qt::NoFocus);
    http_get->setFocusPolicy(Qt::NoFocus);
    http_put->setFocusPolicy(Qt::NoFocus);

    layout->addWidget(box);
    layout->addWidget(ListFiles);
    layout->addLayout(control_layout);

    http_Obj = new http(this);

    //connect(this, SLOT(show()), this, SLOT(getListFiles()));
    connect(http_cancel, SIGNAL(clicked(bool)), this, SLOT(hide()));
    connect(reload_IP, SIGNAL(clicked(bool)), this, SLOT(getListFiles()));
    connect(http_input, SIGNAL(clicked(bool)), this, SLOT(getListFiles()));
    connect(http_output, SIGNAL(clicked(bool)), this, SLOT(getListFiles()));
    connect(this, SIGNAL(sGetListFiles(QString,bool)), http_Obj, SLOT(getListFiles(QString,bool)));
    connect(this, SIGNAL(sPutFile(QString)), http_Obj, SLOT(putFile(QString)));
    connect(this, SIGNAL(sGetFile(QString,QString)), http_Obj, SLOT(getFile(QString,QString)));
    connect(this, SIGNAL(sRemoveFile(QString,bool)), http_Obj, SLOT(remove_File(QString,bool)));
    connect(http_Obj, SIGNAL(sSetListFiles(QStringList&)), this, SLOT(setListFiles(QStringList&)));
    connect(http_Obj, SIGNAL(sSetError(int)), this, SLOT(getError(int)));
    connect(ListFiles, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(setState()));
    connect(http_put, SIGNAL(clicked(bool)), this, SLOT(putFile()));
    connect(http_get, SIGNAL(clicked(bool)), this, SLOT(getFile()));
    connect(http_Obj, SIGNAL(sUpdateListFiles()), this, SLOT(getListFiles()));
    connect(IP_edit, SIGNAL(returnPressed()), this, SLOT(getListFiles()));
    connect(http_delete, SIGNAL(clicked(bool)), this, SLOT(removeFile()));



    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("HttpProcess"));
    //setWindowIcon(QIcon(":/images/RT.ico"));
    setWindowIcon(QIcon(":/images/DTm.ico"));
    resize(450,250);
    hide();

    user_Dir.setPath(qApp->applicationDirPath() + "/user");

    qDebug() << "HttpProcess create: ";
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
HttpProcess::~HttpProcess()
{
    delete http_Obj;

    ListFiles->clear();
    delete ListFiles;

    delete http_cancel;
    delete http_delete;
    delete http_get;
    delete http_put;
    delete IP_edit;
    delete IP_label;
    delete reload_IP;
}
//-----------------------------------------------------------------------------
//--- showEvent
//-----------------------------------------------------------------------------
void HttpProcess::showEvent(QShowEvent *e)
{
    QString temp;

    e->accept();
    //getListFiles();

    if(ax_user)
    {
        temp = ax_user->dynamicCall("getAttr(QString)","dir:PR_HOME").toString();
        if(QDir(temp).exists()) user_Dir.setPath(temp);
    }

    QTimer::singleShot(100, this, SLOT(getListFiles()));
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void HttpProcess::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/httpprocess_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();

    CommonSettings->beginGroup("IP_Device");
    ip_address = CommonSettings->value("ip","0.0.0.0").toString();
    CommonSettings->endGroup();

    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HttpProcess::getListFiles()
{
    QString text = IP_edit->text().trimmed();
    ListFiles->clear();
    emit sGetListFiles(text, http_input->isChecked());

    if(http_input->isChecked()) http_input->setFocus();
    else http_output->setFocus();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HttpProcess::setListFiles(QStringList &lf)
{
    int i;
    QListWidgetItem *item;

    ListFiles->addItems(lf);

    for(i=0; i<ListFiles->count(); i++)
    {
        item = ListFiles->item(i);
        item->setIcon(QIcon(":/images/icon_NULL.ico"));
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HttpProcess::getError(int err)
{
    bool sts = true;

    if(err) sts = false;

    http_input->setEnabled(sts);
    http_output->setEnabled(sts);

    setState(err);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HttpProcess::setState(int err)
{
    http_delete->setDisabled(true);
    http_get->setDisabled(true);
    http_put->setDisabled(true);

    if(err) return;

    if(http_input->isChecked()) http_put->setDisabled(false);

    QListWidgetItem *item = ListFiles->currentItem();
    if(item /*&& item->isSelected()*/)
    {
        http_delete->setDisabled(false);
        if(http_output->isChecked()) http_get->setDisabled(false);
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HttpProcess::putFile()
{
    QString dirName = user_Dir.absolutePath();
    QString selectedFilter;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Protocol"),
                                    dirName,
                                    tr("Protocols File (*.rt)"),
                                    &selectedFilter);

    if(fileName.isEmpty()) return;

    QFile file(fileName);

    if(file.exists()) emit sPutFile(fileName);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HttpProcess::getFile()
{
    QString fn = "";
    QListWidgetItem *item = ListFiles->currentItem();
    if(item) fn = item->text();
    else return;

    QString dirName = user_Dir.absolutePath() + "/" + fn;
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"),
                                                    dirName,
                                                    tr("Protocols File (*.rt)"),
                                                    &selectedFilter);
    qDebug() << "getFile: " << fileName;
    if(fileName.isEmpty()) return;

    emit sGetFile(fn, fileName);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void HttpProcess::removeFile()
{
    QString text;

    QListWidgetItem *item = ListFiles->currentItem();
    if(item)
    {
        text = item->text();
        emit sRemoveFile(text, http_input->isChecked());
    }
}
