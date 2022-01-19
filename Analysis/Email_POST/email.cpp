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
    //tedit_message->setHtml("<a href='mailto:baluev@mail.ru'>baluev@mail.ru</a>");
    //tedit_message->setHtml("<a href='http://www.w3schools.com/'>Link!</a>");
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

    zip_archive = false;

    label_gif = new QLabel(main_Box);
    label_gif->setFixedSize(48,48);
    label_gif->raise();
    obj_gif = new QMovie(":/images/wait_1.gif");
    label_gif->setMovie(obj_gif);
    label_gif->setVisible(false);

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

    delete obj_gif;
    delete label_gif;

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
    QString current_addres = ledit_from->text();
    if(current_addres.isEmpty() || !current_addres.contains("@")) ledit_from->setText(from);
    ledit_from->setFont(f);
    ledit_from->setPalette(palette);
    ledit_from->selectAll();
    ledit_from->setFocus();

    if(!Email_address.trimmed().isEmpty())
    {
        ledit_from->setText(Email_address);
        ledit_subject->setFocus();
    }    

    label_gif->move(width()/2 - label_gif->width()/2, height()/2 - label_gif->height()/2);


    //ledit_subject->setText("");
    //tedit_message->setText("");
    //list_attachment->clear();
}
//-------------------------------------------------------------------------------
//--- showEvent
//-------------------------------------------------------------------------------
void Email::resizeEvent(QResizeEvent *e)
{
    label_gif->move(width()/2 - label_gif->width()/2, height()/2 - label_gif->height()/2);
}
//-----------------------------------------------------------------------------
//--- send_message()
//-----------------------------------------------------------------------------
void Email::send_message()
{
    int i;
    QString text, fn_zip, fname;
    QFile file;
    QFileInfo fi;
    QListWidgetItem *item;

    QString subject = ledit_subject->text();
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

    QUrl url("http://178.57.127.142:8084/api/makeRequest");
    QNetworkRequest request(url);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart textPart_userToken;
    textPart_userToken.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"userToken\""));
    textPart_userToken.setBody("5816eb0c840053b911c9dd86ef6ad71cc41a028f192580b019b8eccb03562c08");
    QHttpPart textPart_serviceId;
    textPart_serviceId.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"serviceId\""));
    textPart_serviceId.setBody("2");
    QHttpPart textPart_senderEmail;
    textPart_senderEmail.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"senderEmail\""));
    textPart_senderEmail.setBody(ledit_from->text().trimmed().toUtf8());
    QHttpPart textPart_subject;
    textPart_subject.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"subject\""));
    textPart_subject.setBody(ledit_subject->text().trimmed().toUtf8());
    QHttpPart textPart_message;
    text = tedit_message->toPlainText().trimmed();
    if(text.isEmpty()) text = "Help...";
    textPart_message.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"message\""));
    textPart_message.setBody(text.toUtf8());

    multiPart->append(textPart_userToken);
    multiPart->append(textPart_serviceId);
    multiPart->append(textPart_senderEmail);
    multiPart->append(textPart_subject);
    multiPart->append(textPart_message);

    if(list_attachment->count() > 0)
    {
        if(zip_archive)
        {
            fn_zip = qApp->applicationDirPath() + "/user/POST.zip";
            QZipWriter writer(fn_zip);
            for(i=0; i<list_attachment->count(); i++)
            {
                text = list_attachment->item(i)->text();
                file.setFileName(text);
                if(file.exists())
                {
                    fi.setFile(text);
                    file.open(QIODevice::ReadOnly);
                    writer.addFile(fi.fileName(), file.readAll());
                    file.close();
                }
            }
            writer.close();

            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"files\"; filename=\"POST.zip\""));
            QFile *file_zip = new QFile(fn_zip);
            file_zip->open(QIODevice::ReadOnly);
            filePart.setBodyDevice(file_zip);
            file_zip->setParent(multiPart);

            multiPart->append(filePart);
        }
        else
        {

            for(i=0; i<list_attachment->count(); i++)
            {
                fname = list_attachment->item(i)->text();
                fi.setFile(fname);
                QHttpPart *files_Part = new QHttpPart();
                text = QString("form-data; name=\"files\"; filename=\"%1\"").arg(fi.fileName());
                files_Part->setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(text));
                QFile *file_attachment = new QFile(fname);
                file_attachment->open(QIODevice::ReadOnly);
                files_Part->setBodyDevice(file_attachment);
                file_attachment->setParent(multiPart);

                list_QHttpPart.append(files_Part);

                multiPart->append(*files_Part);
            }
        }
    }

    QNetworkAccessManager *m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &Email::Response_FromPost);
    connect(m_manager, &QNetworkAccessManager::finished, m_manager, &QNetworkAccessManager::deleteLater);
    connect(m_manager, &QNetworkAccessManager::finished, multiPart, &QHttpMultiPart::deleteLater);

    QNetworkReply* reply = m_manager->post(request, multiPart);

    int id = 0;
    while(!reply->isFinished())
    {
        if(id == 20)
        {
            label_gif->setVisible(true);
            obj_gif->start();
        }

        if(id > 200)
        {
            disconnect(m_manager, &QNetworkAccessManager::finished, nullptr, nullptr);
            qDeleteAll(list_QHttpPart);
            list_QHttpPart.clear();

            //if(multiPart) delete multiPart;
            //if(m_manager) delete m_manager;

            reply->abort();

            message.setStandardButtons(QMessageBox::Ok);
            message.setIcon(QMessageBox::Critical);
            message.setText(tr("Something's wrong!"));
            message.exec();

            break;
        }

        id++;
        //qDebug() << "id: " << id;
        Sleep(50);
        qApp->processEvents();
    }

    label_gif->setVisible(false);
    obj_gif->stop();


    hide();
}
//-----------------------------------------------------------------------------
//--- add_file()
//-----------------------------------------------------------------------------
void Email::Response_FromPost(QNetworkReply* reply)
{
    QByteArray ba = reply->readAll();
    qDebug() << "get from POST request..." << ba;

    qDeleteAll(list_QHttpPart);
    list_QHttpPart.clear();

    message.setStandardButtons(QMessageBox::Ok);

    if(ba.contains("Success"))
    {
        message.setIcon(QMessageBox::Information);
        message.button(QMessageBox::Ok)->animateClick(5000);
        message.setText(tr("The email was succesfully sent!"));
    }
    else
    {        
        message.setIcon(QMessageBox::Critical);
        message.setText(tr("Mail sending failed!"));
    }
    message.exec();

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
