#ifndef WEBDIALOG_H
#define WEBDIALOG_H

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
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTime>

#include <QDebug>

class WebDialog : public QDialog
{
    Q_OBJECT

public:
    WebDialog(QWidget *parent = 0);
    ~WebDialog();

    QPushButton *connect_button;
    QLineEdit   *edit_url;
    QTextEdit   *text_box;
    QPushButton *send_button;
    QLineEdit   *send_message;

public slots:
    void get_MessageFromServer(QString);
    void send_MessageToServer(QString);

};

#endif // WEBDIALOG_H
