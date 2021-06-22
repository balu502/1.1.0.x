#ifndef TEST_EDITOR_H
#define TEST_EDITOR_H

#include "test_editor_global.h"

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
#include <QMultiMap>
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
#include <QMessageBox>
#include <QEvent>
#include <QDropEvent>
#include <QMovie>

#include <QAxObject>

#include <protocol.h>
#include "test_interface.h"
#include "utility.h"
#include "define_PCR.h"

typedef Test_Interface*  (__stdcall *InterfaceTest_factory)();
typedef void  (__stdcall *LoadTestForContainer)(void*, QVector<rt_Test*>*);

typedef void (__stdcall *ShowEditor)(HWND handle, char* xml, bool from_protocol);
typedef void (__stdcall *CloseEditor)();
typedef void (__stdcall *ResizeEditor)(HWND handle);
typedef void (__stdcall *SaveEditor)(char** xml, bool *modified);
typedef void (__stdcall *GetDTRDisplayName)(char* FormID, char** DisplayName);
typedef void (__stdcall *GetDTRDisplayInfo)(char* FormID, char** DisplayName, char** Path, int* Release, int* Build);

class Test_editor;
//-----------------------------------------------------------------------------
class MyTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
        MyTreeWidget(QWidget* parent = 0)
                : QTreeWidget(parent) {widget_parent = (Test_editor*)parent;}

        QMessageBox *message;
        Test_editor *widget_parent;

protected:

        virtual void dropEvent(QDropEvent *);

};
//-----------------------------------------------------------------------------
class groupBox : public QGroupBox
{
    Q_OBJECT

public:
    groupBox(QWidget *parent = 0)
        : QGroupBox(parent) {}

signals:
    void Resize(void);

protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void showEvent(QShowEvent *e);
};
//-----------------------------------------------------------------------------
class TEST_EDITORSHARED_EXPORT Test_editor: public QDialog
{
    Q_OBJECT

public:
    Test_editor(QWidget *parent = 0, bool disable = false);
    virtual ~Test_editor();

    QVector<rt_Test*> *TESTs;
    QAxObject*  ax_user;
    QAxObject*  axgp;
    bool apply_change;
    bool disable_action;
    bool From_Protocol;
    rt_Protocol *prot;
    QString User_Folder;

    QMap<QString,QString> *map_TestTranslate;
    QList<rt_Test*> List_Deleted;
    QMovie  *obj_gif;
    QLabel  *label_gif;

    virtual void showEvent(QShowEvent * event);
    virtual void closeEvent(QCloseEvent * event);
    //virtual bool eventFilter(QObject* o, QEvent* e);

    QToolBar    *tool_editor;
    MyTreeWidget *tests_list;
    //QPushButton *edit_test;
    //QGroupBox   *box_TestList;

    HINSTANCE ext_dll_handle;   // handle external lib
    bool LoadTest_ExtPlugins();

    QString StyleApp;

    QString IN_buffer;
    QString OUT_buffer;
    QString Current_Catalog;
    QDomElement  XML_Signature;

    QIcon logo;
    QIcon logonull;

//private:
    void readCommonSettings();
    QTranslator translator;
    QTranslator qt_translator;
    QString Lang_editor;

    QPushButton *close_button;
    QPushButton *apply_button;
    QSplitter   *main_spl;
    QGroupBox   *box_editor;

    QGroupBox   *group_editor;
    QGroupBox   *widget_editor;
    QVBoxLayout *layout_plugin;

    QLineEdit   *editor_Name;
    QWidget     *spacer;
    QWidget     *obj;

    rt_Test         *ptest_action;
    QTreeWidgetItem *item_action;
    QMessageBox     message;

    QPushButton *edit_TestButton;

    // Action:
    QAction     *new_Container;
    QAction     *new_Test;
    QAction     *copy_Test;
    QAction     *save_Test;
    QAction     *clear_Test;
    QAction     *catalog_Test;
    QAction     *temporary_Test;
    QAction     *menu_deleteTest;
    QAction     *menu_copyTest;
    QAction     *menu_editTest;

    QAction     *menu_AddItem;
    QAction     *menu_DeleteItem;

    QMenu       *menu_Move;
    void        Create_MenuMove(QMenu*);
    void        Delete_MenuMove();
    QList<QAction*> List_ActionsMove;
    void        create_Actions(QTreeWidgetItem*, QMenu*);
    QString     Catalog_ForItem(QTreeWidgetItem*);
    void        Save_Catalog(QAction*, rt_Test*);
    void        Save_PropertyCatalog(rt_Test*);
    bool        Check_StateEditor(rt_Test*);
    void        Add_CatalogXML(QString&);
    bool        Whats_Type(QTreeWidgetItem*);

    bool LoadTest_Plugins();        // Load Plugins
    HINSTANCE dll_handle;           // hahdle plugins dll
    Test_Interface  *alg_editor;    // current Test editor
    QMap<int, Test_Interface*> Map_editor;
    QMap<int, QString> Map_editorNAME;
    QVector<HINSTANCE> List_handle;

    // External Plugins    
    QMap<int, QString> Map_editorEXT;
    QMap<int, QString> Map_editorEXT_formID;
    bool Ext_Editor;
    ShowEditor show_Editor;
    CloseEditor close_Editor;
    ResizeEditor resize_Editor;
    SaveEditor save_Editor;    
    GetDTRDisplayInfo get_Info;

    // methods    
    void Load_editor(QString xml, QString name = "");
    void Fill_ListTests(QString name = "");
    bool Save_DBaseTests(rt_Test*, bool);
    bool Save_SimpleTest(rt_Test*);

    void Create_DefaultCommonProperties(QDomDocument &doc, QDomElement &root);
    void Temporary_Insert(QString &xml, QDomDocument &doc, QDomElement &root, int method);
    
    void Clear_and_Open_Editor(rt_Test*);
    void Load_ListDeleted(QTreeWidgetItem*);

    //void Translate_Catalog();

private slots:
    void Close_Editor();
    void Apply_Editor();
    bool Clear_editor(bool check_Modify = true);
    void create_newTest();
    void create_newContaner();
    void Save_Test(bool confirm = true);
    void Copy_Test();
    void Catalog_editor();
    void DeleteTest_menu();
    void CopyTest_menu();
    void EditTest_menu();
    void auto_resize(QString);
    void edit_Test(QTreeWidgetItem*);
    void edit_Test(rt_Test*);
    void contextMenu_ListTests(QPoint);
    void contextMenu_Catalog(QPoint);
    void AddItem_menu();
    void DeleteItem_menu();
    void resize_extTests();
    void Check_EditStatus(QTreeWidgetItem*,QTreeWidgetItem*);
    void Click_edit_test();
    void Item_changed(QTreeWidgetItem*, int);
    void Item_Move();

    void InputDialog_Select(QString);

    bool Check_Programm_inContainer(QDomElement&);   // check on equal programm in container

};

#endif // TEST_EDITOR_H
