#ifndef COPY_BLOCKTESTS_H
#define COPY_BLOCKTESTS_H

#include "copy_blocktests_global.h"

#include <QObject>
#include <QtWidgets>
#include <QWidget>
#include <QMainWindow>
#include <QDialog>
#include <QGroupBox>
#include <QTranslator>
#include <QSettings>
#include <QSplitter>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QBoxLayout>
#include <QTableWidget>
#include <QVector>
#include <QShowEvent>
#include <QDir>
#include <QDebug>
#include <QMap>
#include <QVector>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QInputDialog>
#include <QPalette>
#include <QFont>
#include <QFontMetrics>
#include <QDomDocument>
#include <QMenu>
#include <QCursor>
#include <QMouseEvent>
#include <QMessageBox>
#include <QMovie>
#include <QStandardPaths>


#include <QAxObject>

#include <protocol.h>
#include <utility.h>

#define ID_LANG_RU	0x0419
#define ID_LANG_EN	0x0409

typedef void (__stdcall *Translate_Service) (int Lang, char *source, char **target);


class Tests_ComboBox: public QComboBox
{
    Q_OBJECT

protected:

    //virtual void hidePopup();
    virtual void showPopup();

signals:
    //signalPopupHidden();
};

class COPY_BLOCKTESTSSHARED_EXPORT Copy_BlockTests: public QDialog
{
    Q_OBJECT

public:
    Copy_BlockTests(QWidget *parent = 0);
    virtual ~Copy_BlockTests();

    virtual void showEvent(QShowEvent * event);
    virtual void resizeEvent(QResizeEvent *e);

    QVector<rt_Test*> *TESTs;
    QAxObject*  ax_user;
    QAxObject*  ax_OtherUser;
    QAxObject*  ax_rita;
    QAxObject*  axgp;
    QVector<rt_Test*> *source_Tests;
    QVector<rt_Test*> *target_Tests;
    QVector<rt_Test*> TESTs_Source_temporary;
    QVector<rt_Test*> TESTs_Target_temporary;
    QMap<int,QString> map_AllUsers;
    QMap<QString,QString> *map_TestTranslate;

    QMessageBox     message;
    QString FileName_To;
    QDir    *user_dir;

    QMovie  *obj_gif;
    QLabel  *label_gif;

    QIcon logo;
    QIcon logonull;

    int ID_LANG;
    Translate_Service translate_service;

private:
    void readCommonSettings();
    QTranslator translator;
    QTranslator qt_translator;

    QGroupBox   *box_control;
    QGroupBox   *box_tests;
    QPushButton *close_button;
    QPushButton *copy_button;
    QPushButton *delete_button;
    QSplitter   *main_spl;
    QGroupBox   *box_source;
    QGroupBox   *box_target;

    Tests_ComboBox   *items_source;
    Tests_ComboBox   *items_target;

    QTreeWidget *tree_source;
    QTreeWidget *tree_target;
    QLabel      *label_source;
    QLabel      *label_target;

    QString     ax_NameTest;
    rt_Test         *ptest_action;
    QTreeWidgetItem *item_action;
    QAction     *menu_MoveToCommon;
    QAction     *menu_MoveToPrivate;

    //methods
    QVector<rt_Test*>* Open_fromProtocol();
    QVector<rt_Test*>* Open_fromFile();
    QVector<rt_Test*>* Open_fromCommon();
    QVector<rt_Test*>* Open_toFile();
    QVector<rt_Test*>* Open_toOtherUser();
    void CreateCopy_Test(rt_Test*, rt_Test*);   // create copy test
    bool Validate_Test(rt_Test*);               // validate test

    void Update_MapTranslate(QVector<rt_Test*>*);   // update map_TestsTranslate

private slots:
    void Fill_SourseTests();
    void contextMenu_ListTests(QPoint);
    void Fill_TargetTests(QVector<rt_Test*> *target = NULL);
    void Change_CheckedSource();
    void Copy_Tests();

    void MoveToCommon();
    void MoveToPrivate();
};



#endif // COPY_BLOCKTESTS_H
