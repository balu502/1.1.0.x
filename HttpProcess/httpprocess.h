#ifndef HTTPPROCESS_H
#define HTTPPROCESS_H

#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QBoxLayout>
#include <QSplitter>
#include <QFont>
#include <QFontMetrics>
#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QProgressBar>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSize>
#include <QList>
#include <QString>
#include <QFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDir>
#include <QWindow>
#include <QTabWidget>
#include <QEvent>
#include <QResizeEvent>
#include <QVariant>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QPainterPath>
#include <QHeaderView>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QPalette>
#include <QShowEvent>
#include <QTranslator>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QListWidget>
#include <QRadioButton>
#include <QAxObject>

#include "http.h"
#include "httpprocess_global.h"

class HTTPPROCESSSHARED_EXPORT HttpProcess: public QDialog
{
    Q_OBJECT

public:
    HttpProcess(QWidget *parent = 0);
    virtual ~HttpProcess();

    http        *http_Obj;
    QAxObject*  ax_user;
    QDir        user_Dir;

    QGroupBox   *box;    
    QListWidget *ListFiles;
    QRadioButton *http_input;
    QRadioButton *http_output;

    QPushButton *http_cancel;
    QPushButton *http_delete;
    QPushButton *http_get;
    QPushButton *http_put;
    QPushButton *reload_IP;
    QLabel      *IP_label;
    QLineEdit   *IP_edit;

    void readCommonSettings();
    QTranslator translator;
    QString ip_address;

protected:
    //void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;

public slots:
    void setListFiles(QStringList&);
    void getError(int);
    void setState(int err = 0);

private slots:
    void getListFiles(void);
    void putFile();
    void getFile();
    void removeFile();

signals:
    void sGetListFiles(QString, bool);
    void sPutFile(QString);
    void sGetFile(QString, QString);
    void sRemoveFile(QString, bool);

};

#endif // HTTPPROCESS_H
