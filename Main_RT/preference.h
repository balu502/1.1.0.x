#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QTranslator>
#include <QPushButton>
#include <QBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QIcon>
#include <QShowEvent>
#include <QDebug>
#include <QFontDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QDateTime>

class General : public QDialog
{
    Q_OBJECT
public:
    explicit General(QWidget *parent = 0);
    virtual ~General();

    QPushButton     *apply_button;
    QPushButton     *cancel_button;
    QTabWidget      *TabWidget;
    QGroupBox       *Interface;
    QGroupBox       *Template_NameProtocol;

    QPushButton     *font_button;
    QLabel          *font_label;
    QComboBox       *lang_box;
    QLabel          *lang_label;
    QComboBox       *style_box;
    QLabel          *style_label;

    QGroupBox       *Template;
    QLabel          *name_label;
    QLabel          *type_label;
    QLabel          *date_label;
    QLineEdit       *name_edit;
    QCheckBox       *type_box;
    QLineEdit       *date_edit;
    QPushButton     *example;
    QLabel          *example_label;


    int id_lang;
    int id_style;
    QString id_font;
    int id_type;

    void drawFont_OnButton(QString);
    void readCommonSettings();
    QTranslator translator;

protected:
    virtual void showEvent(QShowEvent * event);

signals:

public slots:
    void slot_Apply();
    void change_Language(int);
    void change_Style(int);
    void load_Font();
    void example_Template();

    void change_NameEdit(QString);
    void change_DateEdit(QString);
};

#endif // PREFERENCE_H
