#include "scandialog.h"


//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ScanDialog::ScanDialog(QWidget *parent): QDialog(parent)
{
    int i;
    QString text;

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->setMargin(2);
    layout->setSpacing(2);

    Tab = new QTabWidget(this);
    TextEdit = new QTextEdit(this);
    TextEdit->setStyleSheet("background-image: url(:/images/qr_code.png)");
    ClearTextEdit = new QPushButton(QIcon(":/images/flat/clear_flat_16.png"), NULL, this);
    ClearTextEdit->setObjectName("Transparent");
    ClearTextEdit->setIconSize(QSize(16,16));
    ClearTextEdit->setFlat(true);
    ClearTextEdit->setToolTip(tr("Clear scanned text"));
    ClearTextEdit->setStyleSheet("QPushButton::hover {background-color: white; border: 1px solid #ddd;}");
    box_parameters = new QGroupBox(this);
    box_parameters->setObjectName("Transparent");
    QVBoxLayout *param_layout = new QVBoxLayout;
    box_parameters->setLayout(param_layout);
    TPeaks_values = new QLineEdit(this);
    TPeaks_values->setReadOnly(false);
    TPeaks_values->setFixedWidth(250);
    TPeaks_label = new QLabel(tr(" - TPeaks"), this);
    QHBoxLayout *tPeaks_layout = new QHBoxLayout;
    tPeaks_layout->addWidget(TPeaks_values, 0, Qt::AlignLeft);
    tPeaks_layout->addWidget(TPeaks_label, 1, Qt::AlignLeft);
    Amplitude_values = new QLineEdit(this);
    Amplitude_values->setReadOnly(false);
    Amplitude_values->setFixedWidth(250);
    Amplitude_label = new QLabel(tr(" - Amplitude"), this);
    QHBoxLayout *amplitude_layout = new QHBoxLayout;
    amplitude_layout->addWidget(Amplitude_values, 0, Qt::AlignLeft);
    amplitude_layout->addWidget(Amplitude_label, 1, Qt::AlignLeft);
    ID_plate = new QLineEdit(this);
    ID_plate->setReadOnly(false);
    ID_plate->setFixedWidth(250);
    ID_label = new QLabel(tr(" - ID plate"), this);
    QHBoxLayout *ID_layout = new QHBoxLayout;
    ID_layout->addWidget(ID_plate, 0, Qt::AlignLeft);
    ID_layout->addWidget(ID_label, 1, Qt::AlignLeft);


    param_layout->addLayout(tPeaks_layout);
    param_layout->addLayout(amplitude_layout);
    param_layout->addSpacing(50);
    param_layout->addLayout(ID_layout);
    param_layout->addStretch(1);


    Tab->addTab(TextEdit ,tr("Scanned text"));
    Tab->addTab(box_parameters ,tr("Coefficients"));


    QHBoxLayout *control_layout = new QHBoxLayout;
    control_layout->setMargin(2);
    control_layout->setSpacing(1);
    Cancel_Button = new QPushButton(tr("Cancel"), this);
    Apply_Button = new QPushButton(tr("Apply"), this);
    Apply_Button->setDisabled(true);
    Default_Button = new QPushButton(tr("Default"), this);
    control_layout->addWidget(ClearTextEdit, 0, Qt::AlignLeft);
    control_layout->addWidget(Apply_Button, 1, Qt::AlignRight);
    control_layout->addWidget(Default_Button, 0, Qt::AlignRight);
    control_layout->addWidget(Cancel_Button, 0, Qt::AlignRight);

    Cancel_Button->setFocusPolicy(Qt::NoFocus);
    Apply_Button->setFocusPolicy(Qt::NoFocus);
    Default_Button->setFocusPolicy(Qt::NoFocus);

    layout->addWidget(Tab);
    layout->addLayout(control_layout);

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerCheck()));

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Scanning the coefficients"));
    resize(400,500);

    TextEdit->setFocus();
    timer->start(1000);
    Result = -1;

    GroupButtons = new QButtonGroup(this);
    GroupButtons->addButton(Apply_Button);
    GroupButtons->addButton(Default_Button);
    GroupButtons->addButton(Cancel_Button);
    connect(GroupButtons, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(close_Exit(QAbstractButton*)));

    connect(ID_plate, SIGNAL(textChanged(QString)), this, SLOT(Check_ApplyChanges()));
    connect(Amplitude_values, SIGNAL(textChanged(QString)), this, SLOT(Check_ApplyChanges()));
    connect(TPeaks_values, SIGNAL(textChanged(QString)), this, SLOT(Check_ApplyChanges()));

    connect(ClearTextEdit, SIGNAL(clicked(bool)), this, SLOT(clear_TextEdit()));

    //... Create File_Registered ...
    QDomDocument    doc;
    QDomElement     root;
    QDomElement     child;
    QDomElement     item;
    QString id_Plate;
    QMap<QString, QString> *map;
    bool ok;
    int limit = 10;  // define Limit_startup
    Limit_startup = limit;

    QFile file(qApp->applicationDirPath() + "/calibration/registrated.xml");
    if(!file.exists())
    {
        QDomNode xmlNode = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"");
        doc.insertBefore(xmlNode, doc.firstChild());
        QDomElement  root = doc.createElement("PLATEs");
        root.setAttribute("Limit", QString("%1").arg(limit));
        doc.appendChild(root);
        //root.setAttribute("created", "now");

        if(file.open(QIODevice::WriteOnly))
        {
            QTextStream(&file) << doc.toString();
            file.close();
        }
        doc.clear();
    }
    else    //
    {
        if(file.open(QIODevice::ReadOnly))
        {
            if(doc.setContent(&file))
            {
                root = doc.documentElement();                

                for(i=0; i<root.childNodes().size(); i++)
                {
                    child = root.childNodes().at(i).toElement();
                    id_Plate = child.attribute("barcode");
                    map = new QMap<QString,QString>;
                    item = child.firstChildElement("default_TPeaks");
                    if(!item.isNull()) map->insert("TPeaks", item.text());
                    item = child.firstChildElement("default_Amplitude");
                    if(!item.isNull()) map->insert("Amplitude", item.text());
                    map_Plate.insert(id_Plate,map);
                    //qDebug() << "map: " << *map;
                }
                qDebug() << "map_Plate: " << map_Plate;

                doc.clear();
            }
            file.close();
        }
    }

    // English locale
    PostMessage(GetForegroundWindow(), WM_INPUTLANGCHANGEREQUEST, 1, 0x00000409);
    qDebug() << "Lang to Eng";

    // message
    message.setFont(qApp->font());
    message.setWindowIcon(QIcon(":/images/DTm.ico"));
    //message.setWindowIcon(QIcon(":/images/RT.ico"));
    //if(CALIBRATION_status) message.setWindowIcon(QIcon(":/images/check.ico"));

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ScanDialog::~ScanDialog()
{
    timer->stop();
    delete timer;

    delete Apply_Button;
    delete Cancel_Button;
    delete Default_Button;
    delete TextEdit;
    delete TPeaks_values;
    delete Amplitude_values;
    delete ID_plate;
    delete ID_label;
    delete TPeaks_label;
    delete Amplitude_label;
    delete box_parameters;
    delete Tab;

    for(int i=0; i<map_Plate.size(); i++)
    {
        QMap<QString, QString> *map = map_Plate.values().at(i);
        delete map;
    }
    map_Plate.clear();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ScanDialog::clear_TextEdit()
{
    TextEdit->clear();
    TextEdit->setReadOnly(false);
    TextEdit->setFocus();

    timer->stop();
    Sleep(100);

    timer->start(1000);

    qDebug() << "clear: ";
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ScanDialog::slotTimerCheck()
{
    int i;
    bool ok;
    int value;
    double dvalue;
    QString str;
    QStringList list_ch, list_json;
    list_ch << "FAM" << "HEX" << "ROX" << "CY5" << "CY55";
    list_json  << "F" << "H" << "R" << "C5" << "C55";

    QString temp_result, photo_result;
    QString name_plate;
    QStringList list;

    QChar ch;
    bool sts;

    QString text = TextEdit->toPlainText();

    //qDebug() << "slotTimerCheck: " << text;

    // 1. ...QR-code...
    /*if(text.contains("</DTcheckKit>"))
    {

        timer->stop();
        TextEdit->setReadOnly(true);
        //TextEdit->setFocusPolicy(Qt::NoFocus);
        //Apply_Button->setDisabled(false);

        //text = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\r\n" + text;

        //qDebug() << text;

        QDomDocument    doc;
        QDomElement     root;
        QDomElement     params;
        QDomElement     temperature;
        QDomElement     photo;
        QDomElement     child;
        QDomAttr        attr_plate;

        if(doc.setContent(text))
        {

            root = doc.documentElement();

            if(root.hasAttribute("Plate"))
            {
                attr_plate = root.attributeNode("Plate");
                name_plate = attr_plate.value();
                ID_plate->setText(name_plate.trimmed());

                //qDebug() << "name_plate: " << name_plate;
            }



            params = root.firstChildElement("Params");
            if(!params.isNull())
            {
                temperature = params.firstChildElement("temp");
                if(!temperature.isNull())
                {
                    foreach(str, list_ch)
                    {
                        if(!temp_result.isEmpty()) temp_result += " ";
                        child = temperature.firstChildElement(str);
                        if(!child.isNull()) temp_result += child.text();
                        else temp_result += "50.0";
                    }
                    temp_result = temp_result.replace(",",".");
                    TPeaks_values->setText(temp_result);
                }

                photo = params.firstChildElement("photo");
                if(!photo.isNull())
                {
                    foreach(str, list_ch)
                    {
                        if(!photo_result.isEmpty()) photo_result += " ";
                        child = photo.firstChildElement(str);
                        if(!child.isNull())
                        {
                            dvalue = child.text().toDouble(&ok);
                            value = dvalue;
                            photo_result += QString("%1").arg(value);
                        }
                        else photo_result += "1000";
                    }
                    photo_result = photo_result.replace(",",".");
                    Amplitude_values->setText(photo_result);
                }

                Tab->setCurrentWidget(box_parameters);
                ID_plate->setFocus();
            }

            doc.clear();
        }
        //qDebug() << "qr_code" << name_plate;

        return;
    }*/

    // 1.1 QR kod (JSON)
    //Sleep(200);
    if(text.contains("{\"DTcheckKit\"") && text.contains("}"))
    {
        timer->stop();
        TextEdit->setReadOnly(true);


        QJsonDocument jdoc = QJsonDocument::fromJson(text.toUtf8());
        QJsonObject json = jdoc.object();

        name_plate = json["DTcheckKit"].toString();
        sn_LOT = name_plate;
        list = name_plate.split(":");
        if(list.size() > 1)
        {
            ID_plate->setText(list.at(1));
            sn_LOT = list.at(0);
        }
        else ID_plate->setText(name_plate.trimmed());

        date_plate = "";
        if(json.contains("Date"))
        {
            date_plate = json["Date"].toString();
        }
        //qDebug() << "date_plate: " << date_plate;


        foreach(str, list_json)
        {
            temp_result += json[QString("%1t").arg(str)].toString() + " ";
            photo_result += json[QString("%1p").arg(str)].toString() + " ";
        }
        temp_result = temp_result.replace(",",".");
        photo_result = photo_result.replace(",",".");

        Amplitude_values->setText(photo_result.trimmed());
        TPeaks_values->setText(temp_result.trimmed());
        Tab->setCurrentWidget(box_parameters);
        ID_plate->setFocus();

        return;
    }



    // 2. ...Barcode...
    Sleep(200);
    if(!text.isEmpty() && text == TextEdit->toPlainText() && !text.contains("DTcheckKit"))
    {
        timer->stop();
        TextEdit->setReadOnly(true);

        //qDebug() << "barcode: " << text.size() << text;
        text.remove("\n");
        //qDebug() << "barcode: (new) " << text.size() << text;

        sts = true;
        for(i=0; i<text.size(); i++)
        {
            ch = text.at(i);
            if(!(ch.isLetterOrNumber() || ch == '_')) {sts = false; break;}
        }

        if(sts)
        {
            ID_plate->setText(text);
            Tab->setCurrentWidget(box_parameters);
            ID_plate->setFocus();

            qDebug() << "barcode, map: " << text << map_Plate;
            if(map_Plate.contains(text))
            {
                QMap<QString,QString> *map = map_Plate.value(text);
                TPeaks_values->setText(map->value("TPeaks",""));
                Amplitude_values->setText(map->value("Amplitude",""));
            }
        }
    }

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ScanDialog::close_Exit(QAbstractButton *button)
{

    if(button == Cancel_Button) Result = -1;
    if(button == Apply_Button)
    {
        if(!Check_ValidCoefficients())
        {     
            QMessageBox::critical(this, tr("Error"), tr("Invalid format data!"), QMessageBox::Ok);
            return;
        }
        Result = 0;
        Write_RegisteredItem();
    }
    if(button == Default_Button)
    {
        //Result = 1;
        Default_Values();
        return;
    }

    close();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ScanDialog::Write_RegisteredItem()
{
    int i;
    QString barcode;
    bool find = false;
    QString text;
    int count_runs;

    QDomDocument doc;
    QDomElement  root;
    QDomElement  item;
    QDomElement  run;
    QDomElement  child_run;


    QFile file(qApp->applicationDirPath() + "/calibration/registrated.xml");

    if(file.exists() && file.open(QIODevice::ReadWrite))
    {
        if(!doc.setContent(&file))
        {
            QMessageBox::warning(this, tr("Attention!"), tr("Invalid format file!"));
        }
        else
        {
            root = doc.documentElement();
            if(!root.hasAttribute("Limit")) root.setAttribute("Limit", QString("%1").arg(Limit_startup));

            for(i=0; i<root.childNodes().size(); i++)
            {
                item = root.childNodes().at(i).toElement();
                barcode = item.attribute("barcode","");
                if(barcode == ID_plate->text())
                {
                    find = true;
                    break;
                }
            }

            if(!find)
            {
                item = doc.createElement("item");
                root.appendChild(item);
                item.setAttribute("barcode", ID_plate->text());
                item.setAttribute("created", QDateTime::currentDateTime().toString("d MMMM yyyy, H:mm:ss"));
                item.appendChild(MakeElement(doc, "default_TPeaks", TPeaks_values->text().trimmed()));
                item.appendChild(MakeElement(doc, "default_Amplitude", Amplitude_values->text().trimmed()));
                item.appendChild(MakeElement(doc,"run",""));
            }

            run = item.firstChildElement("run");
            if(!run.isNull())
            {
                count_runs = run.childNodes().count();
                if(count_runs >= Limit_startup)
                {
                    text = tr("Attention!");
                    text += QString(" %1 - %2(%3) %4 (%5)!").
                            arg(tr("You have exceeded the limit on the number of launches")).
                            arg(count_runs).
                            arg(Limit_startup).
                            arg(tr("for the plate")).
                            arg(ID_plate->text());
                    //QMessageBox::warning(this, tr("Attention!"), text);
                    message.setStandardButtons(QMessageBox::Ok);
                    message.setIcon(QMessageBox::Critical);
                    message.setText(text);
                    message.exec();

                    Result = -1;
                }
                else
                {
                    child_run = doc.createElement("item");
                    run.appendChild(child_run);
                    child_run.setAttribute("start", QDateTime::currentDateTime().toString("d MMMM yyyy, H:mm:ss"));
                    child_run.setAttribute("Device", name_Dev);
                    child_run.appendChild(MakeElement(doc,"TPeaks", TPeaks_values->text()));
                    child_run.appendChild(MakeElement(doc,"Amplitude", Amplitude_values->text()));

                    //file.seek(0);
                    file.resize(0);
                    QTextStream(&file) << doc.toString();
                }
            }
        }
        file.close();

        doc.clear();
    }

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
bool ScanDialog::Check_ValidCoefficients()
{
    QVector<double> T, A;
    QString str_T, str_A;
    QString str;
    bool ok;
    double value;
    QStringList list;

    str_T = TPeaks_values->text().trimmed();
    str_A = Amplitude_values->text().trimmed();

    list = str_T.split(QRegExp("\\s+"));
    //qDebug() << "list_T: " << list << list.size() << count_ActiveCh;
    //if(!list.size() || list.size() > 4) return(false);
    if(list.size() < count_ActiveCh) return(false);
    foreach(str, list)
    {
        value = str.toDouble(&ok);
        if(!ok) return(false);
        if(value < 10 || value > 100) return(false);
        T.append(value);
    }

    list = str_A.split(QRegExp("\\s+"));
    //if(!list.size() || list.size() > 4) return(false);
    //qDebug() << "list_A: " << list << list.size() << count_ActiveCh;
    if(list.size() < count_ActiveCh) return(false);
    foreach(str, list)
    {
        value = str.toDouble(&ok);
        if(!ok) return(false);
        if(value < 10 || value > 20000) return(false);
        A.append(value);
    }

    return(true);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ScanDialog::Default_Values()
{
    Tab->setCurrentWidget(box_parameters);
    //Apply_Button->setDisabled(false);
    TextEdit->setReadOnly(false);
    TextEdit->clear();
    TextEdit->setFocus();
    timer->start(1000);

    QMap<QString,QString> map_settings;
    emit sReadSettings(&map_settings);
    //qDebug() << "map_Settings: " << map_settings;

    if(map_settings.keys().contains("Amplitude")) Amplitude_values->setText(map_settings.value("Amplitude"));
    if(map_settings.keys().contains("TPeaks")) TPeaks_values->setText(map_settings.value("TPeaks"));

    ID_plate->setText("1111");

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ScanDialog::Check_ApplyChanges()
{
    bool state = false;    

    if(ID_plate->text().trimmed().isEmpty() ||
       Amplitude_values->text().trimmed().isEmpty() ||
       TPeaks_values->text().trimmed().isEmpty()) state = true;

    Apply_Button->setDisabled(state);
}
