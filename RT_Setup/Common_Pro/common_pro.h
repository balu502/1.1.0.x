#ifndef COMMON_PRO_H
#define COMMON_PRO_H

#include <QtCore/QObject>
#include <QApplication>
#include <QWidget>
#include <QObject>
#include <QDialog>
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
#include <QSlider>
#include <QMessageBox>
#include <QDebug>

#include "protocol.h"
#include "define_PCR.h"

#include "common_pro_global.h"



class COMMON_PROSHARED_EXPORT Common_Pro: public QDialog
{    
    Q_OBJECT

public:
    Common_Pro(QWidget *parent = NULL, rt_Protocol *p = NULL);
    virtual ~Common_Pro();

    QTreeWidget *property_list;
    rt_Protocol *prot;

    void readCommonSettings();
    QTranslator translator;
    QMessageBox message;

private:
    QPushButton *cancel_button;
    QPushButton *apply_button;
    QStringList List;
    QSlider     *param_Spline;
    QPushButton *back_Default;

private slots:
    void Load_Properties();
    void Save_Properties();
    void Enable_Changes();
    void Set_DefaultValues();
    void Change_ParamSpline();
};

#endif // COMMON_PRO_H
