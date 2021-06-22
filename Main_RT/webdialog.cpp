#include "webdialog.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
WebDialog::WebDialog(QWidget *parent)
        :QDialog(parent)
{
    QVBoxLayout *main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setMargin(2);

    QHBoxLayout *layout_control = new QHBoxLayout();
    layout_control->setMargin(3);
    edit_url = new QLineEdit(this);
    edit_url->setReadOnly(true);
    edit_url->setFocusPolicy(Qt::NoFocus);
    //edit_url->setText("ws://localhost:9003");
    connect_button = new QPushButton(tr("Connect..."), this);
    //connect_button->setCheckable(true);
    layout_control->addWidget(edit_url, 1);
    layout_control->addWidget(connect_button);

    text_box = new QTextEdit(this);
    text_box->setReadOnly(true);

    /*
    send_message = new QLineEdit(this);
    send_button = new QPushButton("Send...", this);
    QHBoxLayout *layout_send = new QHBoxLayout();
    layout_send->addWidget(send_message, 1);
    layout_send->addWidget(send_button);
    */

    main_layout->addLayout(layout_control);
    main_layout->addWidget(text_box,1);
    //main_layout->addLayout(layout_send);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Web Client"));
    setWindowIcon(QIcon(":/images/DTm.ico"));
    //setWindowIcon(QIcon(":/images/RT.ico"));
    resize(800,250);
    hide();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
WebDialog::~WebDialog()
{
    delete edit_url;
    delete connect_button;
    text_box->clear();
    delete text_box;
    //delete send_message;
    //delete send_button;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void WebDialog::get_MessageFromServer(QString message)
{    
    //QString tim = QDateTime::currentDateTime().toString("h:mm:ss ");
    //text_box->append(tim + "-> " + message);
    //qDebug() << "get_MessageFromServer: " << message;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void WebDialog::send_MessageToServer(QString message)
{
    //QString tim = QDateTime::currentDateTime().toString("h:mm:ss ");
    //text_box->append(tim + "<- " + message);
    //qDebug() << "send_MessageToServer: " << message;
}
