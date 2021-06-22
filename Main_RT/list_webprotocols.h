#ifndef LIST_WEBPROTOCOLS_H
#define LIST_WEBPROTOCOLS_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QDialog>
#include <QTreeWidget>
#include <QSettings>
#include <QTranslator>
#include <QPushButton>
#include <QBoxLayout>
#include <QTreeWidgetItem>

enum QmlType{rtapp,consumer};

class List_WebProtocols : public QDialog
{
    Q_OBJECT

public:
    List_WebProtocols(QWidget *parent = 0);
    ~List_WebProtocols();

    void readCommonSettings();
    QTranslator translator;

    QmlType type;

    QTreeWidget *tree_Protocols;
    QPushButton *apply_button;
    QPushButton *cancel_button;
    QPushButton *reload_list;

    QMap<QString,QString> *web_List;

    void Load_Protocols(QMap<QString,QString>*);
    QString Key_result;

public slots:
    void change_CurrentItem();
    void Reload_List();

signals:
    void reload_ListProtocols();

};

#endif // LIST_WEBPROTOCOLS_H
