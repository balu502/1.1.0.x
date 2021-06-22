#ifndef HEADER_TEST_H
#define HEADER_TEST_H

#include <QtCore/QObject>
#include <QApplication>
#include <QWidget>
#include <QObject>
#include <QTabWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QVector>
#include <QTableWidget>
#include <QHeaderView>
#include <QFont>
#include <QStyledItemDelegate>
#include <QPalette>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QLabel>
#include <QBoxLayout>
#include <QFont>
#include <QTreeWidget>
#include <QScrollArea>
#include <QDateTime>
#include <QPolygon>
#include <QSettings>
#include <QTranslator>
#include <QDomDocument>
#include <QPushButton>

#include <QDebug>

#include "protocol.h"
#include "utility.h"
#include "define_PCR.h"

#include "header_global.h"

#define  min_Vol 5
#define  max_Vol 200
#define  default_Vol 35

//-----------------------------------------------------------------------------
class HeaderTestDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    HeaderTestDelegate(QObject *parent=0) : QStyledItemDelegate(parent)
    {
    }

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint (QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QString style;
    QString *method_analysis;

};
//-----------------------------------------------------------------------------
class HEADERSHARED_EXPORT Header_Test: public QTreeWidget
{
    Q_OBJECT

public:
    Header_Test(QWidget *parent = NULL);
    virtual ~Header_Test();

    HeaderTestDelegate *header_Delegate;

    void Load_XML(QDomNode &node);
    void Fill_Properties(rt_Test *ptest);
    void Change_NameProgramm(QString);
    void Load_NewTest(QString, QString, int);
    void SetDisable_ActiveChannels();
    void Set_ActiveChannels(int);

    int active_Channels;
    QString method_analysis;
    QString test_ID;

    void readCommonSettings();
    QTranslator translator;
    QString StyleApp;

signals:
    void Change_ActiveChannels(int);

private:

    QPushButton *edit_VolumeTube;
    QPushButton *edit_Comments;

private slots:

    void Edit_Volume();
    void Edit_Comments();

    void Edit_ActiveChannels(QTreeWidgetItem*,int);
};

#endif // HEADER_TEST_H
