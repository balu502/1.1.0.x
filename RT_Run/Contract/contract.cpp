#include "contract.h"
#include "../Run/request_dev.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Contract::Contract(QWidget *parent): QDialog(parent)
{

    readCommonSettings();
    setFont(qApp->font());

    QHBoxLayout *layout = new QHBoxLayout();
    this->setLayout(layout);
    layout->setMargin(1);

    Control_Box = new QGroupBox(this);
    Control_Box->setObjectName("Transparent");
    Container_Box = new QGroupBox(this);
    Container_Box->setObjectName("Transparent");

    layout->addWidget(Control_Box);
    layout->addWidget(Container_Box);


    label_CRYR = new QLabel(tr(" 1. Create request for contract extension:"), this);
    label_CRYS = new QLabel(tr(" 2. Contract extension (download to device):"), this);
    label_CRYG = new QLabel(tr(" 3. The current state of the contract:"), this);

    button_CRYR = new QPushButton("CRYR", this);
    button_CRYS = new QPushButton("CRYS", this);
    button_CRYG = new QPushButton("CRYG", this);

    button_CRYS->setDisabled(true);

    state_CRYR = new QLabel(this);
    state_CRYS = new QLabel(this);
    state_CRYG = new QLabel(this);
    state_CRYR->setFixedWidth(32);
    state_CRYS->setFixedWidth(32);
    state_CRYG->setFixedWidth(32);

    tool_CRYR = new QToolButton(this);
    tool_CRYS = new QToolButton(this);
    tool_CRYG = new QToolButton(this);
    tool_CRYR->setIcon(QIcon(":/images/save_flat.png"));
    tool_CRYS->setIcon(QIcon(":/images/open_flat.png"));
    tool_CRYG->setIcon(QIcon(":/images/save_flat.png"));

    remained_Days = new QLabel(this);
    remained_Days->setStyleSheet("QLabel {color : red;}");

    QVBoxLayout *layout_ver = new QVBoxLayout();
    Control_Box->setLayout(layout_ver);
    layout_ver->setMargin(1);
    QHBoxLayout *layout_hor1 = new QHBoxLayout();
    QHBoxLayout *layout_hor2 = new QHBoxLayout();
    QHBoxLayout *layout_hor3 = new QHBoxLayout();
    layout_ver->addLayout(layout_hor1);
    layout_ver->addLayout(layout_hor2);
    layout_ver->addLayout(layout_hor3);
    layout_ver->addStretch(1);
    layout_ver->addWidget(remained_Days, 1, Qt::AlignLeft | Qt::AlignBottom);

    layout_hor1->addWidget(label_CRYR, 1, Qt::AlignLeft);
    layout_hor1->addWidget(button_CRYR, 0, Qt::AlignRight);
    layout_hor1->addWidget(state_CRYR, 0, Qt::AlignRight);
    layout_hor1->addWidget(tool_CRYR, 0, Qt::AlignRight);

    layout_hor2->addWidget(label_CRYS, 1, Qt::AlignLeft);
    layout_hor2->addWidget(button_CRYS, 0, Qt::AlignRight);
    layout_hor2->addWidget(state_CRYS, 0, Qt::AlignRight);
    layout_hor2->addWidget(tool_CRYS, 0, Qt::AlignRight);

    layout_hor3->addWidget(label_CRYG, 1, Qt::AlignLeft);
    layout_hor3->addWidget(button_CRYG, 0, Qt::AlignRight);
    layout_hor3->addWidget(state_CRYG, 0, Qt::AlignRight);
    layout_hor3->addWidget(tool_CRYG, 0, Qt::AlignRight);


    state_CRYR->setPixmap(QPixmap(""));                         //(QPixmap(":/images/ok.png"));
    state_CRYS->setPixmap(QPixmap(""));                         //(QPixmap(":/images/warning.png"));
    state_CRYG->setPixmap(QPixmap(""));


    Text_Container = new QTextEdit(this);
    Text_Container->setReadOnly(true);
    QVBoxLayout *layout_ver_con = new QVBoxLayout();
    layout_ver_con->setMargin(1);
    Container_Box->setLayout(layout_ver_con);
    layout_ver_con->addWidget(Text_Container);

    connect(button_CRYR, SIGNAL(clicked(bool)), this, SLOT(slot_CRYR()));
    connect(tool_CRYR, SIGNAL(clicked(bool)), this, SLOT(slot_SaveCRYR()));
    connect(button_CRYS, SIGNAL(clicked(bool)), this, SLOT(slot_CRYS()));
    connect(tool_CRYS, SIGNAL(clicked(bool)), this, SLOT(slot_OpenCRYS()));
    connect(button_CRYG, SIGNAL(clicked(bool)), this, SLOT(slot_CRYG()));
    connect(tool_CRYG, SIGNAL(clicked(bool)), this, SLOT(slot_SaveCRYG()));


    setWindowTitle(tr("Contract"));
    //setWindowIcon(QIcon(":/images/adjustment.ico"));
    setFixedSize(700,170);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Contract::~Contract()
{


}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/contract_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::showEvent(QShowEvent *e)
{
    remained_Days->setText(tr("Remains (count of days): ") + QString::number(count_days));
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::slot_CRYG()
{
    Text_Container->clear();
    button_CRYS->setDisabled(true);
    map_CRYPTO->clear();
    map_CRYPTO->insert(CRYPTO_CTRL,"CRYG");
    state_CRYG->setPixmap(QPixmap(""));
    emit sSend_CRYPTO();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::slot_SaveCRYG()
{
    QString fileName;
    QString dirName = user_Dir->absolutePath() + "/cryg";
    QString selectedFilter;
    QString text = Text_Container->toPlainText();
    QByteArray ba = QByteArray::fromHex(text.toUtf8());

    fileName = QFileDialog::getSaveFileName(this, tr("Save as CRYG.BIN"),
                            dirName,
                            tr("cryg File (*.bin)"),
                            &selectedFilter);

    qDebug() << "fileName: " << fileName << ba.size();

    if(fileName.isEmpty()) return;

    QFile file(fileName);

    if(file.open(QFile::WriteOnly))
    {
        file.write(ba);
        file.close();
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::slot_CRYR()
{
    Text_Container->clear();
    button_CRYS->setDisabled(true);
    map_CRYPTO->clear();
    map_CRYPTO->insert(CRYPTO_CTRL,"CRYR");
    state_CRYR->setPixmap(QPixmap(""));
    emit sSend_CRYPTO();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::slot_SaveCRYR()
{
    QString fileName;
    QString dirName = user_Dir->absolutePath() + "/cryr";
    QString selectedFilter;
    QString text = Text_Container->toPlainText();
    QByteArray ba = QByteArray::fromHex(text.toUtf8());

    fileName = QFileDialog::getSaveFileName(this, tr("Save as CRYR.BIN"),
                            dirName,
                            tr("cryr File (*.bin)"),
                            &selectedFilter);

    qDebug() << "fileName: " << fileName << ba.size();

    if(fileName.isEmpty()) return;

    QFile file(fileName);

    if(file.open(QFile::WriteOnly))
    {
        file.write(ba);
        file.close();
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::slot_CRYS()
{
    QString text = Text_Container->toPlainText();
    QByteArray ba = QByteArray::fromHex(text.toUtf8());

    map_CRYPTO->clear();
    map_CRYPTO->insert(CRYPTO_CTRL,"CRYS");
    map_CRYPTO->insert(CRYPTO_DATA, ba);
    state_CRYS->setPixmap(QPixmap(""));

    emit sSend_CRYPTO();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::slot_OpenCRYS()
{
    QString fileName;
    QString dirName = user_Dir->absolutePath();
    QString selectedFilter;

    button_CRYS->setDisabled(true);
    Text_Container->clear();

    fileName = QFileDialog::getOpenFileName(this, tr("Open as crypto.CRYS"),
                                            dirName,
                                            tr("crys File (*.crys)"),
                                            &selectedFilter);

    if(fileName.isEmpty()) return;

    QFile file(fileName);
    QByteArray ba;

    if(file.open(QFile::ReadOnly))
    {
        ba = file.readAll();
        file.close();
    }

    QString text = ba.toHex();
    Text_Container->setPlainText(text);

    //qDebug() << "fileName: " << fileName << ba.size();

    if(ba.size() == 512) button_CRYS->setDisabled(false);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Contract::Read_CRYPTO(QMap<QString, QByteArray> *map)
{

    QString control = QString(map->value(CRYPTO_CTRL,""));
    QByteArray ba = map->value(CRYPTO_DATA, "");
    QByteArray status = map->value("status", "");
    QDataStream ds(status);
    short status_num;
    ds >> status_num;
    bool state = false;

    if(!status.isEmpty() && status_num == 0) state = true;

    /*QString fn = user_Dir->absolutePath() + "/crypto.bin";
    QFile file(fn);
    if(file.open(QFile::WriteOnly))
    {
        file.write(ba);
        file.close();
    }*/

    QString text = ba.toHex();
    Text_Container->setPlainText(text);

    if(control == "CRYR")
    {
        if(ba.size() == 512 && state) state_CRYR->setPixmap(QPixmap(":/images/ok.png"));
        else state_CRYR->setPixmap(QPixmap(":/images/warning.png"));
    }

    if(control == "CRYS")
    {
        state_CRYS->setPixmap(QPixmap(":/images/ok.png"));
        if(state) state_CRYS->setPixmap(QPixmap(":/images/ok.png"));
        else state_CRYS->setPixmap(QPixmap(":/images/warning.png"));
    }

    if(control == "CRYG")
    {
        if(ba.size() == 512 && state) state_CRYG->setPixmap(QPixmap(":/images/ok.png"));
        else state_CRYG->setPixmap(QPixmap(":/images/warning.png"));
    }
}
