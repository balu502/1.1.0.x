#include "email.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Email::Email(QWidget *parent): QDialog(parent)
{
    readCommonSettings();
    setFont(qApp->font());

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->setMargin(2);
    layout->setSpacing(2);

    //---
    main_Box = new QGroupBox(this);
    main_Box->setObjectName("Transparent");
    QVBoxLayout *layout_box = new QVBoxLayout;
    main_Box->setLayout(layout_box);

    QHBoxLayout *layout_from = new QHBoxLayout();
    layout_from->setMargin(0);
    label_from = new QLabel(tr("1. From: "), this);
    label_from->setFixedWidth(120);
    ledit_from = new QLineEdit(this);
    layout_from->addWidget(label_from);
    layout_from->addWidget(ledit_from, 1);


    QHBoxLayout *layout_subject = new QHBoxLayout();
    layout_subject->setMargin(0);
    label_subject = new QLabel(tr("2. Subject: "), this);
    label_subject->setFixedWidth(120);
    ledit_subject = new QLineEdit(this);
    layout_subject->addWidget(label_subject);
    layout_subject->addWidget(ledit_subject, 1);

    QHBoxLayout *layout_message = new QHBoxLayout();
    layout_message->setMargin(0);
    label_message = new QLabel(tr("3. Message: "), this);
    label_message->setFixedWidth(120);
    tedit_message = new QTextEdit(this);
    layout_message->addWidget(label_message, 0, Qt::AlignTop);
    layout_message->addWidget(tedit_message, 1);


    QHBoxLayout *layout_attachment = new QHBoxLayout();
    layout_attachment->setMargin(0);
    QVBoxLayout *layout_attach = new QVBoxLayout();
    layout_attach->setMargin(0);
    layout_attach->setSpacing(2);

    label_attachment = new QLabel(tr("4. Attachment: "), this);
    label_attachment->setFixedWidth(120);
    add_attachment = new QPushButton("+", this);
    add_attachment->setMaximumWidth(30);
    delete_attachment = new QPushButton("-", this);
    delete_attachment->setFixedSize(30,30);
    add_protocol = new QPushButton(tr("protocol"), this);
    add_screenshot = new QPushButton(tr("screenshot"), this);
    layout_attach->addWidget(label_attachment);
    layout_attach->addSpacing(10);
    layout_attach->addWidget(add_attachment, 0, Qt::AlignRight);
    layout_attach->addWidget(delete_attachment, 0, Qt::AlignRight);
    layout_attach->addStretch(1);
    layout_attach->addWidget(add_protocol, 2, Qt::AlignBottom);
    layout_attach->addWidget(add_screenshot, 0, Qt::AlignBottom);
    list_attachment = new QListWidget(this);
    list_attachment->setContextMenuPolicy(Qt::CustomContextMenu);
    //list_attachment->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    layout_attachment->addLayout(layout_attach);
    layout_attachment->addWidget(list_attachment);
    connect(add_screenshot, SIGNAL(clicked(bool)), this, SLOT(create_ScreenShot()));
    connect(add_protocol, SIGNAL(clicked(bool)), this, SLOT(add_FileProtocol()));
    connect(list_attachment, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu_Attachment()));

    open_image = new QAction(QIcon(":/images/images.png"),tr("open as PNG_Image"), this);
    connect(open_image, SIGNAL(triggered(bool)), this, SLOT(open_Image()));

    layout_box->addLayout(layout_from);
    layout_box->addLayout(layout_subject);
    layout_box->addLayout(layout_message);
    layout_box->addLayout(layout_attachment);

    connect(add_attachment, SIGNAL(clicked()), this, SLOT(add_item()));
    connect(delete_attachment, SIGNAL(clicked()), this, SLOT(delete_item()));

    //---

    QHBoxLayout *layout_control = new QHBoxLayout();
    layout_control->setMargin(2);
    send_button = new QPushButton(tr("Send"), this);
    cancel_button = new QPushButton(tr("Cancel"), this);
    layout_control->addWidget(send_button, 1, Qt::AlignRight);
    layout_control->addWidget(cancel_button, 0, Qt::AlignRight);
    connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(hide()));
    connect(send_button, SIGNAL(clicked(bool)), this, SLOT(send_message()));

    layout->addWidget(main_Box, 0, Qt::AlignTop);
    layout->addLayout(layout_control);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Send message via Email"));
    //setWindowIcon(QIcon(":/images/dna_2.ico"));
    resize(700,500);
    //setFixedSize(550,450);
    hide();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Email::~Email()
{


    delete label_from;
    delete ledit_from;
    delete label_subject;
    delete ledit_subject;
    delete label_message;
    delete tedit_message;
    delete label_attachment;
    delete add_attachment;
    delete delete_attachment;
    delete add_protocol;
    delete add_screenshot;

    list_attachment->clear();
    delete list_attachment;

    delete main_Box;

    delete cancel_button;
    delete send_button;
}
//-------------------------------------------------------------------------------
//--- showEvent
//-------------------------------------------------------------------------------
void Email::showEvent(QShowEvent *e)
{
    QFont f = qApp->font();
    f.setItalic(true);
    QPalette palette;
    palette.setColor(ledit_from->foregroundRole(), Qt::red);

    QString from = tr("here you must place your email address for answering...");
    from_Init = from;
    ledit_from->setText(from);
    ledit_from->setFont(f);
    ledit_from->setPalette(palette);
    ledit_from->selectAll();
    ledit_from->setFocus();

    if(!Email_address.trimmed().isEmpty())
    {
        ledit_from->setText(Email_address);
        ledit_subject->setFocus();
    }


    //ledit_subject->setText("");
    //tedit_message->setText("");
    //list_attachment->clear();
}

//-----------------------------------------------------------------------------
//--- send_message()
//-----------------------------------------------------------------------------
void Email::send_message()
{
    int i;

    QString host = "mail.dna-technology.ru"; //"smtp.mail.ru";
    int port = 465;
    bool ssl = true;
    bool auth = true;
    QString user = "realtimepcr_support@dna-technology.ru";
    QString password = "4G2iG742Wmm";

    QString recipient_address = "hotline@dna-technology.ru";
    //QString recipient_address = "a.baluev@dna-technology.ru";
    if(!Email_recipient.isEmpty()) recipient_address = Email_recipient;

    EmailAddress *sender = stringToEmail(user);
    EmailAddress *recipient = stringToEmail(recipient_address);


    QString subject = ledit_subject->text();
    QString html = tedit_message->toHtml();

    if(!ledit_from->text().contains("@"))
    {
        message.setText(tr("You must create the sender's email address"));
        message.setIcon(QMessageBox::Critical);
        message.exec();

        ledit_from->setText("");
        ledit_from->selectAll();
        ledit_from->setFocus();

        return;
    }


    if(ledit_from->text() != from_Init)
    {
        subject += QString("  from: %1").arg(ledit_from->text());
    }

    SmtpClient smtp(host, port, ssl ? SmtpClient::SslConnection : SmtpClient::TcpConnection);

    MimeMessage E_message;

    E_message.setSender(sender);
    E_message.setSubject(subject);
    E_message.addRecipient(recipient);

    MimeHtml content;
    content.setHtml(html);

    E_message.addPart(&content);

    for(i=0; i<list_attachment->count(); i++)
    {
        E_message.addPart(new MimeAttachment(new QFile(list_attachment->item(i)->text())));
    }

    message.setStandardButtons(QMessageBox::Ok);
    message.setIcon(QMessageBox::Critical);

    //... Error ...
    if(!smtp.connectToHost())
    {
        message.setText(tr("Connection Failed"));
        message.exec();
        hide();
        return;
    }
    if(auth)
    {
      if(!smtp.login(user, password))
      {
          message.setText(tr("Authentification Failed"));
          message.exec();
          hide();
          return;
      }
    }
    //...
    if(!smtp.sendMail(E_message))
    {
        message.setText(tr("Mail sending failed"));
        message.exec();
        hide();
        return;
    }
    else
    {
        message.setIcon(QMessageBox::Information);
        message.button(QMessageBox::Ok)->animateClick(5000);
        message.setText(tr("The email was succesfully sent!"));
        message.exec();
    }

    smtp.quit();

    hide();
}
//-----------------------------------------------------------------------------
//--- add_file()
//-----------------------------------------------------------------------------
void Email::add_item()
{
    QString fn = "";
    QListWidgetItem *item;
    QString dirName = qApp->applicationDirPath();

    fn = QFileDialog::getOpenFileName(this, tr("Select file"));
    if(fn.isEmpty()) return;

    QFile f(fn);
    if(f.exists())
    {
        item = new QListWidgetItem();
        item->setText(fn);
        list_attachment->addItem(item);
        list_attachment->setCurrentItem(item);


    }
}
//-----------------------------------------------------------------------------
//--- add_file()
//-----------------------------------------------------------------------------
void Email::delete_item()
{
    QListWidgetItem *item;

    if(!list_attachment->count()) return;

    int row = list_attachment->currentRow();
    item = list_attachment->takeItem(row);
    delete item;

    list_attachment->repaint();

}
//-----------------------------------------------------------------------------
//--- stringToEmail
//-----------------------------------------------------------------------------
EmailAddress* Email::stringToEmail(const QString &str)
{
    int p1 = str.indexOf("<");
    int p2 = str.indexOf(">");

    if (p1 == -1)
    {
        // no name, only email address
        return new EmailAddress(str);
    }
    else
    {
        return new EmailAddress(str.mid(p1 + 1, p2 - p1 - 1), str.left(p1));
    }

}

//-----------------------------------------------------------------------------
//--- create_ScreenShot()
//-----------------------------------------------------------------------------
void Email::create_ScreenShot()
{
    QListWidgetItem *item;
    QScreen *screen = QGuiApplication::primaryScreen();

    //setWindowState(Qt::WindowMinimized);
    showMinimized();

    QString filename = QString("%1/screenshot_%2.png").arg(dir_temp->path()).arg(GetRandomString(6));    

    foreach (QWidget *widget, qApp->topLevelWidgets())
    {
        if(widget->accessibleDescription() == "Main_RT")
        {
            //QPixmap pixmap(widget->grab());

            widget->repaint();
            Sleep(100);

            //QPixmap pixmap = QPixmap::grabWindow(widget->winId());
            QPixmap pixmap = screen->grabWindow(widget->winId());
            pixmap.save(filename, "PNG");

            QFile f(filename);
            if(f.exists())
            {
                item = new QListWidgetItem(list_attachment);
                item->setText(filename);
                list_attachment->addItem(item);
                list_attachment->setCurrentItem(item);
            }

            break;
        }
    }

    showNormal();



}
//-----------------------------------------------------------------------------
//--- add_FileProtocol()
//-----------------------------------------------------------------------------
void Email::add_FileProtocol()
{
    QListWidgetItem *item;

    if(!Pro) return;

    QString file_pro = QString::fromStdString(Pro->xml_filename);
    QFile f(file_pro);
    if(f.exists() && list_attachment->findItems(file_pro, Qt::MatchContains).size() == 0)
    {
        item = new QListWidgetItem();
        item->setText(file_pro);
        list_attachment->addItem(item);
        list_attachment->setCurrentItem(item);
    }
}
//-----------------------------------------------------------------------------
//--- contextMenu_Attachment()
//-----------------------------------------------------------------------------
void Email::contextMenu_Attachment()
{
    QListWidgetItem *item = list_attachment->currentItem();
    if(!item || !item->isSelected()) return;

    QString fn = item->text();
    QFileInfo f(fn);
    if(!f.suffix().startsWith("png",Qt::CaseInsensitive)) return;

    QMenu menu;
    menu.setStyleSheet("QMenu::item:selected {background-color: #d7d7ff; color: black;}");

    menu.addAction(open_image);

    menu.exec(QCursor::pos());

    menu.clear();
}
//-----------------------------------------------------------------------------
//--- open_Image()
//-----------------------------------------------------------------------------
void Email::open_Image()
{
    QListWidgetItem *item = list_attachment->currentItem();
    if(!item || !item->isSelected()) return;

    QString fn = item->text();

    QDialog *IMAGE = new QDialog();
    IMAGE->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    IMAGE->setWindowTitle(tr("Screenshot"));
    IMAGE->resize(800,500);
    QVBoxLayout *layout = new QVBoxLayout(IMAGE);

    foreach (QWidget *widget, qApp->topLevelWidgets())
    {
        if(widget->accessibleDescription() == "Main_RT")
        {
            IMAGE->resize(widget->width()*0.3,widget->height()*0.3);
            break;
        }
    }

    QLabel *imageLabel = new QLabel();
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
    layout->addWidget(scrollArea);

    QImage image(fn);
    imageLabel->setPixmap(QPixmap::fromImage(image));
    imageLabel->adjustSize();

    IMAGE->exec();

    delete imageLabel;
    delete scrollArea;
    delete IMAGE;
}

//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Email::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/email_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;

    //...
    QSettings *EmailSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    EmailSettings->beginGroup("Email");
    Email_recipient = EmailSettings->value("address", "").toString();
    EmailSettings->endGroup();

    qDebug() << "Email: " << Email_recipient;

    delete EmailSettings;
}
