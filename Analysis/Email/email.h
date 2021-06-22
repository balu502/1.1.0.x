#ifndef EMAIL_H
#define EMAIL_H

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
#include <QMovie>
#include <QTimer>
#include <QHash>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QPalette>
#include <QImage>
//#include <QSsl>
//#include <QtNetwork/QSslSocket>

#include <QDomDocument>

#include "protocol.h"
#include "utility.h"
#include "email_global.h"
#include "SmtpMime"



class EMAILSHARED_EXPORT Email: public QDialog
{
    Q_OBJECT

public:
    Email(QWidget *parent = 0);
    virtual ~Email();

    static EmailAddress * stringToEmail(const QString & str);

    QPushButton *send_button;
    QPushButton *cancel_button;

    QGroupBox   *main_Box;
    QLabel      *label_from;
    QLineEdit   *ledit_from;
    QString     from_Init;
    QLabel      *label_subject;
    QLineEdit   *ledit_subject;
    QLabel      *label_message;
    QTextEdit   *tedit_message;

    QLabel      *label_attachment;
    QPushButton *add_attachment;
    QPushButton *delete_attachment;
    QPushButton *add_protocol;
    QPushButton *add_screenshot;
    QListWidget *list_attachment;

    QTemporaryDir   *dir_temp;
    rt_Protocol     *Pro;
    QMessageBox     message;
    QString         Email_address;
    QString         Email_recipient;

    QAction *open_image;

public slots:
    void send_message();
    void add_item();
    void delete_item();

    void create_ScreenShot();
    void add_FileProtocol();

    void contextMenu_Attachment();
    void open_Image();

private:
    QTranslator translator;
    void readCommonSettings();

protected:

    void showEvent(QShowEvent *e);
};

#endif // EMAIL_H
