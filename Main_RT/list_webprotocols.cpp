#include "list_webprotocols.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
List_WebProtocols::List_WebProtocols(QWidget *parent)
    :QDialog(parent)
{
    QStringList pp;
    pp << tr("Name") << tr("ID") << tr("BarCode");
    Key_result = "";

    readCommonSettings();

    QVBoxLayout *main_layout = new QVBoxLayout();
    setLayout(main_layout);
    main_layout->setMargin(2);

    tree_Protocols = new QTreeWidget(this);
    tree_Protocols->clear();
    tree_Protocols->setColumnCount(3);
    tree_Protocols->setHeaderLabels(pp);
    tree_Protocols->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *layout_control = new QHBoxLayout();
    layout_control->setMargin(2);
    cancel_button = new QPushButton(tr("Cancel"), this);
    apply_button = new QPushButton(tr("Select"), this);
    reload_list = new QPushButton(QIcon(":/images/reload_24.png"), "", this);
    reload_list->setFixedWidth(32);
    layout_control->addWidget(reload_list, 0, Qt::AlignLeft);
    layout_control->addWidget(apply_button,1,Qt::AlignRight);
    layout_control->addWidget(cancel_button,0,Qt::AlignRight);
    apply_button->setFocusPolicy(Qt::NoFocus);
    cancel_button->setFocusPolicy(Qt::NoFocus);
    reload_list->setFocusPolicy(Qt::NoFocus);
    apply_button->setDisabled(true);


    main_layout->addWidget(tree_Protocols,1);
    main_layout->addLayout(layout_control);

    connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(apply_button, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(reload_list, SIGNAL(clicked(bool)), this, SLOT(Reload_List()));
    connect(tree_Protocols, SIGNAL(itemSelectionChanged()), this, SLOT(change_CurrentItem()));
    connect(tree_Protocols, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(accept()));

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("List of Web Protocols"));
    resize(650,300);
    move(parent->x()+25, parent->y()+25);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
List_WebProtocols::~List_WebProtocols()
{
    delete cancel_button;
    delete apply_button;
    delete reload_list;

    tree_Protocols->clear();
    delete tree_Protocols;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void List_WebProtocols::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);


    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void List_WebProtocols::Load_Protocols(QMap<QString, QString> *list)
{
    int i;
    QString str, text;
    QTreeWidgetItem *item;
    QStringList list_item;

    tree_Protocols->clear();

    for(i=0; i<list->size(); i++)
    {
        str = list->keys().at(i);
        text = list->value(str);
        list_item = text.split(";");
        item = new QTreeWidgetItem(tree_Protocols);
        item->setText(0, list_item.at(0));
        item->setText(1, str);
        item->setText(2, list_item.at(1));

    }
    for(i=0; i<tree_Protocols->columnCount(); i++) tree_Protocols->resizeColumnToContents(i);
    tree_Protocols->setColumnWidth(0, tree_Protocols->columnWidth(0)*1.1);
    //tree_Protocols->setCurrentItem(NULL);
}
//-----------------------------------------------------------------------------
//--- change_CurrentItem
//-----------------------------------------------------------------------------
void List_WebProtocols::change_CurrentItem()
{
    QTreeWidgetItem *item = tree_Protocols->currentItem();
    if(!item)
    {
        apply_button->setEnabled(false);
        Key_result = "";
        return;
    }

    apply_button->setEnabled(true);
    Key_result = item->text(1);
}
//-----------------------------------------------------------------------------
//--- Reload_List()
//-----------------------------------------------------------------------------
void List_WebProtocols::Reload_List()
{
    tree_Protocols->clear();
    apply_button->setDisabled(true);
    emit reload_ListProtocols();
}
