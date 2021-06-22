#include "editor.h"

extern "C" EDITORSHARED_EXPORT void __stdcall create_editor(std::string in, bool &ok, std::string &out)
{
    QString text = QString::fromStdString(in);

    Editor *p = new Editor();    
    if(!text.isEmpty())
    {        
        p->Text_To_Block(text);
        p->DrawBlock_inGraph();
        p->DrawBlock_inTable();

        p->Editor_Table->setCurrentCell(0,0);
        p->Editor_Table->setFocus();

        p->In_Source = p->Block_To_Text();
    }

    if(p->exec())
    {
        ok = true;
        out = p->Block_To_Text().toStdString();
    }
    else ok = false;

    delete p;
    return;
}

extern "C" EDITORSHARED_EXPORT void __stdcall cr_editor(char *p_in, bool &ok, char **p_out)
{
    int argc = 0;
    char **argv = 0;
    //QApplication a(argc, argv);
    //qDebug() << "Editor:";
    if(QApplication::instance() == 0)
    {
        new QApplication(argc, argv);
        qDebug() << "new app:" ;
    }

    QString text(p_in);
    QString text_out;//(p_out);
    int leng;
    char *out;

    //qDebug() << "IN: " << text;


    Editor *p = new Editor();

    if(!text.isEmpty())
    {
        p->Text_To_Block(text);
        p->DrawBlock_inGraph();
        p->DrawBlock_inTable();

        p->Editor_Table->setCurrentCell(0,0);
        p->Editor_Table->setFocus();
    }

    if(p->exec())
    {
        ok = true;        
        text_out = p->Block_To_Text();
        leng = text_out.length();
        //out = (char*)text_out.toStdString().c_str();
        out = new char[leng+1];
        *p_out = out;
        memcpy(out, text_out.toLatin1().data(), leng+1);
        //

        //qDebug() << "OUT: " << text_out;

    }
    else ok = false;

    delete p;
    return;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Editor::Editor(QWidget *parent): QDialog(parent)
{
    int i;
    QString text;
    QStringList list;
    QList<int> list_spl;
    QFont label_font("Times New Roman", 12, QFont::Bold);
    QPalette palette;


    readCommonSettings();
    setFont(qApp->font());
    if(StyleApp == "fusion")
    {
        setStyle(QStyleFactory::create("Fusion"));
        setStyleSheet(
            "QSplitter::handle:vertical   {height: 6px; image: url(:/images/v_splitter_pro.png);}"
            "QSplitter::handle:horizontal {width:  6px; image: url(:/images/h_splitter_pro.png);}"
            "QToolBar {border: 1px solid #ddd; background-color: #FAFAFA;}"
            "QToolTip {background: #FFFFFF; border: 1px solid #ddd;}"
            "QToolButton::hover {background: white;}"
            "QToolButton::disabled {background-color: #ccc;}"
            "QGroupBox {margin-top: 1ex; background: #FAFAFA; border: 1px solid #ddd;}"
            "QGroupBox#Transparent {border: 1px solid transparent;}"
            "QTextEdit {background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;}"
            "QDialog {background-color: #FAFAFA; border: 1px solid #ddd;}"
            "QPushButton::hover {background-color: #fff;}"
            "QPushButton::disabled {background-color: #ccc;}"
            "QPushButton::enabled {background-color: #FAFAFA;}"
            "QPushButton::pressed {background-color: #ccc;}"
            "QPushButton {text-align: center;}"
            "QPushButton {min-width: 4em; margin:0; padding:5;}"
            "QPushButton {border: 1px solid #aaa; border-radius: 0px;}"
            "QLineEdit {selection-background-color: #d7d7ff; selection-color: black;}"
            "QSpinBox {selection-background-color: #d7d7ff; selection-color: black;}"
            "QTreeWidget {selection-background-color: #d7d7ff; selection-color: black;}"
            "QComboBox {background-color: #ffffff; selection-background-color: #d7d7ff; selection-color: black;}"
            "QMenu::item:selected {background-color: #d7d7ff; color: black;}"
            "QMenuBar::item:selected {background-color: #d7d7ff; color: black;}"
            "QStatusBar {background: #FAFAFA; border: 1px solid #ddd;}"
            "QHeaderView::section {background-color: #FFFFFF; border: 1px solid gray; border-left: 0px; border-top: 0px; padding:3; margin:0;}"
            );

    }

    Gradient_Type = None;
    In_Source = "";

    box = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save);
    connect(box, SIGNAL(rejected()), this, SLOT(reject()));
    connect(box, SIGNAL(accepted()), this, SLOT(Accept_withValidation()));
    box->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    box->button(QDialogButtonBox::Save)->setText(tr("Save"));
    box->button(QDialogButtonBox::Cancel)->setFocusPolicy(Qt::NoFocus);
    box->button(QDialogButtonBox::Save)->setFocusPolicy(Qt::NoFocus);

    createActions();
    ToolBar = new QToolBar();

    ToolBar->addAction(open_program);
    ToolBar->addAction(new_program);
    ToolBar->addAction(save_program);
    //ToolBar->setIconSize(QSize(24,24));

    main_Box = new QGroupBox(this);
    main_Box->setObjectName("Transparent");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    QVBoxLayout *main_layout = new QVBoxLayout;
    setLayout(main_layout);

    main_layout->addWidget(ToolBar);
    main_layout->addWidget(main_Box);
    main_layout->addWidget(box);

    main_spl = new QSplitter(Qt::Vertical, this);
    main_spl->setHandleWidth(4);
    main_spl->setChildrenCollapsible(false);

    main_Box->setLayout(layout);
    layout->addWidget(main_spl);

    graph_Box = new QGroupBox(this);

    control_spl = new QSplitter(Qt::Horizontal, this);
    create_Box = new QGroupBox(this);
    create_Box->setObjectName("Transparent");
    control_Box = new QGroupBox(this);
    control_spl->addWidget(create_Box);
    control_spl->addWidget(control_Box);

    main_spl->addWidget(control_spl);
    main_spl->addWidget(graph_Box);

    // create Table
    Editor_Table = new QTableWidget(0,0,this);
    //Editor_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    Editor_Table->setSelectionBehavior(QAbstractItemView::SelectItems);
    Editor_Table->setSelectionMode(QAbstractItemView::SingleSelection);

    Editor_Delegate = new EditorItemDelegate();
    Editor_Delegate->Type.clear();
    Editor_Table->setItemDelegate(Editor_Delegate);

    Time_Delegate = new TimeEditorDelegate(this);
    Editor_Table->setItemDelegateForColumn(2,Time_Delegate);
    Time_Delegate->Type = &Editor_Delegate->Type;

    ComboBox_Delegate = new ComboBoxDelegate(this);
    Editor_Table->setItemDelegateForColumn(4,ComboBox_Delegate);
    ComboBox_Delegate->Type = &Editor_Delegate->Type;

    Button_Delegate = new DelegatPerson(this);
    Editor_Table->setItemDelegateForColumn(5,Button_Delegate);
    Editor_Table->setItemDelegateForColumn(6,Button_Delegate);
    Editor_Table->setItemDelegateForColumn(7,Button_Delegate);
    Button_Delegate->Type = &Editor_Delegate->Type;

    QVBoxLayout *create_layout = new QVBoxLayout;
    if(StyleApp == "fusion") create_layout->setMargin(0);
    create_Box->setLayout(create_layout);
    create_layout->addWidget(Editor_Table);
    connect(Editor_Table,SIGNAL(cellChanged(int,int)), this, SLOT(Value_Changed(int,int)));
    connect(Editor_Table,SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(Selection_Changed(int,int,int,int)));
    connect(this,SIGNAL(put_select(int,int)), this,SLOT(get_select(int,int)), Qt::QueuedConnection);
    connect(Button_Delegate, SIGNAL(signal_3D()),  this, SLOT(show_Gradient()));
    connect(Button_Delegate, SIGNAL(signal_IncrTemp()), this, SLOT(show_IncrTemperature()));


    // create Plot
    QVBoxLayout *graph_layout = new QVBoxLayout;
    graph_Box->setLayout(graph_layout);
    plot_Program = new Plot_ProgramAmpl(this);
    //plot_Program->gradient = &Gradient;
    plot_Program->curve->gradient = &Gradient_Type;
    graph_layout->addWidget(plot_Program);
    plot_Program->setMinimumHeight(150);

    // create Control
    QVBoxLayout *control_layout = new QVBoxLayout;
    control_layout->setSpacing(1);
    control_Box->setLayout(control_layout);
    add_Block = new QPushButton(tr("Add Block"), this);
    add_Level = new QPushButton(tr("Add Level"), this);
    delete_Block = new QPushButton(tr("Delete Block"), this);
    delete_Level = new QPushButton(tr("Delete Level"), this);

    type_Block = new QComboBox(this);
    //type_Block->setEditable(true);
    //type_Block->lineEdit()->setReadOnly(true);
    //type_Block->lineEdit()->setAlignment(Qt::AlignCenter);
    list << tr("Cycle") << tr("Melting Curve") << tr("Pause") << tr("Standby");
    type_Block->addItems(list);
    for(i=0; i < type_Block->count(); i++) type_Block->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole);
    type_Block->setCurrentIndex(-1);
    type_Block->setStyleSheet("selection-background-color: #d7d7ff; selection-color: black; background: white;");


    label_Name = new QLabel(tr("Name of Program: "), this);
    Name_Program = new QLineEdit("Example",this);
    Name_Program->setAlignment(Qt::AlignHCenter);
    Name_Program->setPalette(palette);
    QRegExp rx("[A-Za-z0-9_-]{1,14}");
    Name_Program->setValidator(new QRegExpValidator(rx, this));

    label_Volume = new QLabel(tr("Volume of tube (mkl): "), this);
    Volume_Tube = new QSpinBox(this);

    //...
    label_Volume->setVisible(false);
    Volume_Tube->setVisible(false);
    //...

    Volume_Tube->setMaximum(300);
    Volume_Tube->setValue(35);
    Volume_Tube->setPalette(palette);
    name_layout = new QHBoxLayout();
    volume_layout = new QHBoxLayout();
    name_layout->addWidget(label_Name);
    name_layout->addWidget(Name_Program);
    volume_layout->addWidget(label_Volume);
    volume_layout->addWidget(Volume_Tube);

    Name_Program->setFont(label_font);
    Volume_Tube->setFont(label_font);
    //type_Block->setFont(label_font);

    Name_Pr = Name_Program->text();
    Volume_Pr = Volume_Tube->value();

    connect(Name_Program,SIGNAL(textChanged(QString)), this,SLOT(Name_Changed(QString)));
    connect(Volume_Tube,SIGNAL(valueChanged(int)), this,SLOT(Volume_Changed(int)));

    control_layout->addLayout(name_layout);
    control_layout->addLayout(volume_layout);
    control_layout->addSpacing(20);

    control_layout->addWidget(add_Block);
    control_layout->addWidget(add_Level);
    control_layout->addWidget(delete_Block);
    control_layout->addWidget(delete_Level);
    control_layout->addWidget(type_Block, 1, Qt::AlignBottom);

    connect(add_Block,SIGNAL(clicked(bool)), this,SLOT(add_block()));
    connect(add_Level,SIGNAL(clicked(bool)), this,SLOT(add_level()));
    connect(delete_Block,SIGNAL(clicked(bool)), this,SLOT(delete_block()));
    connect(delete_Level,SIGNAL(clicked(bool)), this,SLOT(delete_level()));

    connect(type_Block,SIGNAL(currentIndexChanged(int)), this,SLOT(Change_TypeBlock(int)));

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);// | Qt::WindowStaysOnTopHint);
    //setWindowModality(Qt::WindowModal);    
    setWindowTitle(tr("Editor"));
    setWindowIcon(QIcon(":/images/DTm.ico"));
    //setWindowIcon(QIcon(":/images/RT.ico"));
    int w = QApplication::desktop()->availableGeometry().width();
    resize(w*0.7, w*0.7*0.45);
    box->button(QDialogButtonBox::Cancel)->setDefault(true);

    msgBox.setWindowIcon(QIcon(":/images/DTm.ico"));
    //msgBox.setWindowIcon(QIcon(":/images/RT.ico"));
    //setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);

    list_spl.append(w*0.5);
    list_spl.append(w*0.15);
    control_spl->setSizes(list_spl);

    Gradient_Widget = new Gradient(this);
    Gradient_Widget->gradient_Info = &Gradient_Info;
    dll_3D = NULL;
    dll_3D = ::LoadLibraryW(L"plugins\\bars_3D.dll");
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Editor::~Editor()
{
    box->clear();
    delete box;

    delete name_layout;
    delete volume_layout;

    Editor_Delegate->Type.clear();

    delete ToolBar;
    delete plot_Program;
    delete Editor_Table;
    delete graph_Box;
    delete create_Box;
    delete control_Box;
    delete main_Box;

    for(int i=0; i<Blocks.size(); i++) delete Blocks[i];
    Blocks.clear();

    if(dll_3D) ::FreeLibrary(dll_3D);
    delete Gradient_Widget;
}

//-----------------------------------------------------------------------------
//--- keyPressEvent
//-----------------------------------------------------------------------------
void Editor::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return)
    {
        event->ignore();
    }
    else event->accept();
}

//-----------------------------------------------------------------------------
//--- closeEvent
//-----------------------------------------------------------------------------
void Editor::closeEvent(QCloseEvent *event)
{
    bool sts_exit = true;
    QString text;
    int res;

    if(result() == 0)
    {
        Out_Source = Block_To_Text();
        if(Out_Source != In_Source)
        {
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.setWindowTitle(tr("Attention"));
            text = tr("You don't save changes of the programm! Will you want to save this changes?");
            msgBox.setText(text);
            msgBox.setIcon(QMessageBox::Information);            
            res = msgBox.exec();
            switch(res)
            {
            case QMessageBox::Yes:       setResult(QDialog::Accepted);  break;
            case QMessageBox::No:        setResult(QDialog::Rejected);  break;
            case QMessageBox::Cancel:    sts_exit = false;  break;
            default: break;
            }
        }

    }

    if(sts_exit)
    {
        if(result() == QDialog::Rejected) event->accept();      // QDialog::Rejected
        else                                                    // QDialog::Accepted
        {
            if(!Check_Validation())
            {
                setResult(0);
                event->ignore();
            }
            else event->accept();
        }
    }
    else event->ignore();
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Editor::readCommonSettings()
{
    QString text;
    QString dir_SysTranslate;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");        

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/editor_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }
        // ... System Language ...
        dir_SysTranslate = dir_path + "/tools/translations";
        //if(translator_sys.load(":/translations/qt_" + text + ".qm", dir_SysTranslate))
        if(translator_sys.load("qt_" + text + ".qm", dir_SysTranslate))
        {
            qApp->installTranslator(&translator_sys);
        }        
        //... Style ...
        text = CommonSettings->value("style","xp").toString();
        StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//--- createActions()
//-----------------------------------------------------------------------------
void Editor::createActions()
{
    open_program = new QAction(QIcon(":/images/open_new.png"), tr("Open Program"), this);
    if(StyleApp == "fusion") open_program->setIcon(QIcon(":/images/flat/open_flat.png"));
    connect(open_program, SIGNAL(triggered()), this, SLOT(Open_Program()));

    new_program = new QAction(QIcon(":/images/new_new.png"), tr("New Program"), this);
    if(StyleApp == "fusion") new_program->setIcon(QIcon(":/images/flat/new_flat.png"));
    connect(new_program, SIGNAL(triggered()), this, SLOT(New_Program()));

    save_program = new QAction(QIcon(":/images/save_new.png"), tr("Save Program in File"), this);
    if(StyleApp == "fusion") save_program->setIcon(QIcon(":/images/flat/save_flat.png"));
    connect(save_program, SIGNAL(triggered()), this, SLOT(Save_Program()));
}

//-----------------------------------------------------------------------------
//--- New_Program()
//-----------------------------------------------------------------------------
void Editor::New_Program()
{
    for(int i=0; i<Blocks.size(); i++) delete Blocks[i];
    Blocks.clear();

    DrawBlock_inGraph();
    DrawBlock_inTable();

    Name_Program->setText("Example");
    Volume_Tube->setValue(35);
    type_Block->setCurrentIndex(-1);
}
//-----------------------------------------------------------------------------
//--- Save_Program()
//-----------------------------------------------------------------------------
void Editor::Save_Program()
{
    QString fileName = "";
    QString text;
    //Dir ApplDir(qApp->applicationDirPath());
    //QString dirName = ApplSettings->value("SaveDir",ApplDir.dirName()).toString();

    QString selectedFilter;

    if(!Check_Validation())
    {
        return;
    }

    fileName = QFileDialog::getSaveFileName(this, tr("Save Program in File"),
                            "",
                            tr("File with Program (*.rta)"),
                            &selectedFilter);

    if(fileName.isEmpty()) return;

    text = Block_To_Text();

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly))
    {
        QTextStream(&file) << text;
        file.close();
    }
    else
    {
        qDebug() << "error in open file...";
    }

}
//-----------------------------------------------------------------------------
//--- Accept_withValidation()
//-----------------------------------------------------------------------------
void Editor::Accept_withValidation()
{
    if(!Check_Validation())
    {
        return;
    }
    accept();
}

//-----------------------------------------------------------------------------
//--- Check_Validation
//-----------------------------------------------------------------------------
bool Editor::Check_Validation()
{
    int i,j;
    int num;
    short leng;
    QString text = "";
    bool res = true;
    Block_Pr *block;
    int count = Blocks.size();


    // 0. Empty name of program
    if(Name_Program->text().trimmed().isEmpty())
    {
        text = tr("Empty the name of the program!");
        QMessageBox::warning(this, tr("Error in the program!"), text);
        return(false);
    }

    // 1. hold only in end program
    for(i=0; i<count; i++)
    {
        block = Blocks.at(i);
        if(i < count-1 && block->Type_Block == 4)
        {
            text = tr("The Hold block can be located only at the end of the program!");            
            QMessageBox::warning(this, tr("Error in the program!"), text);
            return(false);
        }
    }

    // 2. count of blocks
    if(Blocks.size() > TR_PRG_BLK)
    {
        text = QString("%1 (%2)").arg(tr("There is a limit on the number of blocks: ")).arg(TR_PRG_BLK);
        QMessageBox::warning(this, tr("Error in the program!"), text);
        return(false);
    }

    // 3. count of levels
    num = 0;
    for(i=0; i<count; i++)
    {
        block = Blocks.at(i);
        num += block->Temperature.size();
    }
    if(num > TR_PRG_LEV)
    {
        text = QString("%1 (%2)").arg(tr("There is a limit on the number of levels in programm: ")).arg(TR_PRG_LEV);
        QMessageBox::warning(this, tr("Error in the program!"), text);
        return(false);
    }

    // 4. count of levels in block
    for(i=0; i<count; i++)
    {
        block = Blocks.at(i);
        num = block->Temperature.size();
        if(num > TR_BLK_LEV)
        {
            text = QString("%1 (%2)").arg(tr("There is a limit on the number of levels in block: ")).arg(TR_BLK_LEV);
            QMessageBox::warning(this, tr("Error in the program!"), text);
            return(false);
        }
    }

    // 5. leng of levels
    for(i=0; i<count; i++)
    {
        block = Blocks.at(i);
        for(j=0; j<block->Time.size(); j++)
        {
            leng = block->Time.at(j);
            if(leng > MAX_LEV_TIME)
            {
                text = QString("%1 (%2sec)").arg(tr("There is a limit on the leng of level: ")).arg(MAX_LEV_TIME);
                QMessageBox::warning(this, tr("Error in the program!"), text);
                return(false);
            }
        }
    }

    // 6. count of number of cycles
    for(i=0; i<count; i++)
    {
        block = Blocks.at(i);
        if(block->Num_Cycle > MAX_BLK_REP)
        {
            text = QString("%1 (%2)").arg(tr("There is a limit on the number of cycles in block: ")).arg(MAX_BLK_REP);
            QMessageBox::warning(this, tr("Error in the program!"), text);
            return(false);
        }
    }



    return(res);
}

//-----------------------------------------------------------------------------
//--- Open_Program()
//-----------------------------------------------------------------------------
void Editor::Open_Program()
{
    int i;
    rt_Protocol *prot_temp;
    QVector<string> q;
    QString fileName = "";
    QString ext;
    //QDir ApplDir(qApp->applicationDirPath());
    //QString dirName = ApplSettings->value("OpenDir",ApplDir.dirName()).toString();
    QString selectedFilter;
    QString text = "";

    fileName = QFileDialog::getOpenFileName(this, tr("Open Protocol"),
                            "",
                            tr("Protocols&Pragram Files (*.rt *.trt *.rta)"),
                            &selectedFilter);

    if(fileName.isEmpty()) return;

    QFile file(fileName);
    QFileInfo file_info(fileName);
    ext = file_info.suffix();

    New_Program();  // clear program

    if(ext == "rt")
    {        
        prot_temp = Create_Protocol();

        Read_XML(this,NULL,prot_temp,fileName,"",true);

        q = QVector<string>::fromStdVector(prot_temp->program);
        for(i=0; i<q.size(); i++)
        {
            if(i) text += "\r\n";
            text += QString::fromStdString(q.at(i));
        }
        prot_temp->Clear_Protocol();
        delete prot_temp;
    }
    if(ext == "rta")
    {
        if(file.open(QIODevice::ReadOnly))
        {
            QTextStream t_str(&file);
            text = t_str.readAll();
            file.close();
        }
    }

    if(text.isEmpty()) return;

    Text_To_Block(text);
    DrawBlock_inGraph();
    DrawBlock_inTable();

    Editor_Table->setCurrentCell(0,0);
    Editor_Table->setFocus();    
}

//-----------------------------------------------------------------------------
//--- Block_To_Text()
//-----------------------------------------------------------------------------
QString Editor::Block_To_Text()
{
    int i,j;
    QString text = "";
    Block_Pr *block;
    int value = Gradient_Type;

    for(i=0; i<Blocks.size(); i++)
    {
        if(!i)
        {
            text = QString("XPRG 0 %1 %2").arg(Volume_Pr).arg(value);
        }
        block = Blocks.at(i);
        for(j=0; j<block->Temperature.size(); j++)
        {
            // XTCH ...
            if(block->Temperature_Gradient.at(j).length() &&
               block->Gradient.at(j) != 0)
            {
                text += "\r\n";
                text += "XTCH " + block->Temperature_Gradient.at(j);
            }

            text += "\r\n";
            text += QString("XLEV %1 %2 %3 %4 %5 %6").
                    arg(block->Temperature.at(j)).
                    arg(block->Time.at(j)).
                    arg(block->Incr_Temp.at(j)).
                    arg(block->Incr_Time.at(j)).
                    arg(block->Gradient.at(j)).
                    arg(block->Measure.at(j));
            //qDebug() << i << j << block->Gradient.at(j);
        }
        text += "\r\n";

        switch(block->Type_Block)
        {
        case bCycle:                // Cycle
        case bMelt:                 // Melting
                        text += QString("XCYC %1").arg(block->Num_Cycle);
                        break;
        case bPause:                // Pause
                        text += "XPAU";
                        break;
        case bHold:                 // Hold
                        text += "XHLD";
                        break;
        }

    }    
    if(text.length()) text += QString("\r\nXSAV %1").arg(Name_Pr);

    //qDebug() << "Block_To_Text(): " << text;

    return(text);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Editor::Text_To_Block(QString str)
{
    int i;
    short value;
    QString text, str_tmp;
    QStringList list = str.split("\r\n");
    QStringList s;
    bool new_block = true;
    int current_level = 0;
    bool ok;
    int val;

    Block_Pr *block;
    Gradient_Type = None;    // 0 - not gradient
    bool gradient_exist = false;

    //qDebug() << list;

    for(i=0; i<list.size(); i++)
    {
        text = list.at(i);

        // XPRG
        if(text.indexOf("XPRG") == 0)
        {
            new_block = true;
            s = text.split(QRegExp("\\s+"));
            text = s.at(2);
            Volume_Pr = text.toShort();

            if(s.size() >= 4)
            {
                val = QString(s.at(3)).toInt(&ok);
                if(ok) Gradient_Type = (gradient_Type)val;
            }

            continue;
        }

        // XTCH
        if(text.indexOf("XTCH ") == 0)
        {
            gradient_exist = true;
            str_tmp = text.mid(5);
            continue;
        }

        // XLEV
        if(text.indexOf("XLEV") == 0)
        {
            if(new_block) {block = new Block_Pr(); current_level = 0; Blocks.push_back(block);}
            new_block = false;

            s = text.split(QRegExp("\\s+"));

            text = s.at(1);
            value = text.toShort();
            block->Temperature.push_back(value);        // Temperature
            text = s.at(2);
            value = text.toShort();
            block->Time.push_back(value);               // Time
            text = s.at(3);
            value = text.toShort();
            block->Incr_Temp.push_back(value);          // Incr_Temp
            text = s.at(4);
            value = text.toShort();
            block->Incr_Time.push_back(value);          // Incr_Time

            text = s.at(5);
            value = text.toShort();
            if(!gradient_exist) {value = 0; str_tmp = "";}
            block->Temperature_Gradient.push_back(str_tmp);
            block->Gradient.push_back(value);           // Grad
            gradient_exist = false;

            text = s.at(6);
            value = text.toShort();
            block->Measure.push_back(value);            // Measure


            current_level++;
            continue;
        }

        // XCYC
        if(text.indexOf("XCYC") == 0)
        {
            s = text.split(QRegExp("\\s+"));
            text = s.at(1);
            value = text.toShort();
            block->Num_Cycle = value;

            if(block->Measure.contains(2)) block->Type_Block = bMelt;
            else block->Type_Block = bCycle;

            new_block = true;
            continue;
        }

        // XPAU
        if(text.indexOf("XPAU") == 0)
        {
            block->Type_Block = bPause;

            new_block = true;
            continue;
        }

        // XHLD
        if(text.indexOf("XHLD") == 0)
        {
            block->Type_Block = bHold;

            new_block = true;
            continue;
        }

        // XSAV
        if(text.indexOf("XSAV") == 0)
        {
            s = text.split(QRegExp("\\s+"));
            Name_Pr = s.at(1);

            new_block = true;
            continue;
        }
    }
    Name_Program->setText(Name_Pr);
    Volume_Tube->setValue(Volume_Pr);    
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Editor::DrawBlock_inTable()
{
    int i,j,k;
    int count = 0;
    QString text, str_result;
    QStringList header;
    QTableWidgetItem *newItem;
    Block_Pr* block;
    double value;

    Editor_Table->blockSignals(true);

    QString label_IncrTau = QChar(0x94, 0x03) + tr(" time, sec");
    QString label_IncrTemp = QChar(0x94, 0x03) + tr(" temperature,°C");

    header << "N°" << tr("Temperature,°C") << tr("Time, hour:min:sec") << tr("Number of cycles") << tr("Measure") << tr("Gradient,°C") << label_IncrTau << label_IncrTemp;

    for(i = Editor_Table->rowCount(); i>0; i--) Editor_Table->removeRow(i-1);

    Editor_Table->clear();
    Editor_Delegate->Type.clear();

    Editor_Table->setColumnCount(8);
    Editor_Table->setHorizontalHeaderLabels(header);
    //Editor_Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    Editor_Table->setColumnWidth(0,40);
    Editor_Table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
    for(i=1; i<Editor_Table->columnCount(); i++) Editor_Table->horizontalHeader()->setSectionResizeMode(i,QHeaderView::Stretch);
    Editor_Table->verticalHeader()->setVisible(false);
    Editor_Table->verticalHeader()->setMinimumSectionSize(1);
    // count rows

    for(i=0; i<Blocks.size(); i++)
    {
        if(i) count++;
        block = Blocks.at(i);
        for(j=0; j<block->Temperature.size(); j++) count++;
    }
    Editor_Table->setRowCount(count);
    header.clear();

    count = 0;
    for(i=0; i<Blocks.size(); i++)
    {
        if(i)
        {
            Editor_Table->setRowHeight(count,2);
            for(k=0; k<Editor_Table->columnCount(); k++)
            {
                newItem = new QTableWidgetItem();
                newItem->setText("...");
                Editor_Table->setItem(count,k,newItem);
                newItem->setFlags(Editor_Table->item(count,k)->flags() & ~Qt::ItemIsEditable);
            }
            Editor_Delegate->Type.push_back(0);
            count++;
        }
        block = Blocks.at(i);
        for(j=0; j<block->Temperature.size(); j++)
        {
            Editor_Table->setRowHeight(count,24);
            for(k=0; k<Editor_Table->columnCount(); k++)
            {

                newItem = new QTableWidgetItem();
                Editor_Table->setItem(count,k,newItem);

                switch(k)
                {
                case 0:             // N
                        text = QString("%1").arg(i+1);
                        newItem->setFlags(Editor_Table->item(count,k)->flags() & ~Qt::ItemIsEditable);
                        break;
                case 1:             // Temperature
                        value = (double)block->Temperature.at(j)/100.;
                        text = QString::number(value,'f',1);
                        break;
                case 2:             // Time
                        text = QString::number(block->Time.at(j));
                        break;
                case 3:             // Cycles
                        text = QString::number(block->Num_Cycle);
                        break;
                case 4:             // Measure
                        text = QString::number(block->Measure.at(j));
                        break;
                case 5:             // Gradient
                        text = QString::number((double)block->Gradient.at(j)/10.,'f',1);
                        break;
                case 6:             // Incr_Time
                        text = QString::number((double)block->Incr_Time.at(j)/10.,'f',0);
                        break;
                case 7:             // Incr_Temp
                        text = QString::number((double)block->Incr_Temp.at(j)/100.,'f',1);
                        break;
                default:
                        text = "";
                        break;
                }
                newItem->setText(text);

                if((block->Type_Block == bPause || block->Type_Block == bHold) && k != 1)   // Disable edit options in Hold&Pause
                {
                    newItem->setFlags(Editor_Table->item(count,k)->flags() & ~Qt::ItemIsEditable);
                }
            }
            Editor_Delegate->Type.push_back(block->Type_Block);
            count++;
        }
        if(block->Temperature.size() > 1)
        {
            Editor_Table->setSpan(count - block->Temperature.size(), 0, block->Temperature.size(), 1);  // Number
            Editor_Table->setSpan(count - block->Temperature.size(), 3, block->Temperature.size(), 1);  // Num Cycles
        }

    }


    Editor_Table->blockSignals(false);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Editor::DrawBlock_inGraph()
{
    int i,j;
    double value_x = 0;
    double value_y;
    double val;
    QVector<double> x,y;
    QVector<double> x_border,y_border;
    QColor color;
    int count = Blocks.size();
    QString text;
    Block_Pr *block;
    QwtPlotCurve *curve_border;

    plot_Program->curve->setData(NULL);
    plot_Program->curve->color_lev.clear();
    plot_Program->Delete_Curves_Border();

    y_border << 0. << 100.;

    for(i=0; i<count; i++)
    {
        block = Blocks.at(i);

        block->Coordinates_Level.clear();

        if(!i)
        {
            value_y = (double)(block->Temperature.at(0))/100.;
            x.push_back(value_x);
            y.push_back(value_y);
        }

        // create curve_border
        if(i)
        {
            curve_border = new QwtPlotCurve();
            curve_border->setPen(Qt::black, 1, Qt::DotLine);
            plot_Program->Curves_Border.push_back(curve_border);
            x_border.clear();
            x_border << value_x << value_x;
            curve_border->setSamples(x_border,y_border);
            curve_border->attach(plot_Program);

            //qDebug() << "i:" << i << value_x;

        }

        switch(block->Type_Block)
        {
        case bMelt:

            value_y = (double)(block->Temperature.at(0))/100.;
            val = (double)(block->Incr_Temp.at(0))/100.;
            block->Coordinates_Level.push_back(value_x);
            for(j=0; j<block->Num_Cycle; j++)
            {
                if(j) value_x += 1.;
                x.push_back(value_x);
                y.push_back(value_y);
                plot_Program->curve->color_lev.push_back(Qt::darkGreen);

                //value_x += 1./(atan(1.*0.01)/M_PI_2);
                value_x += 4.;
                x.push_back(value_x);
                y.push_back(value_y);
                plot_Program->curve->color_lev.push_back(Qt::red);

                value_y += val;
            }
            block->Coordinates_Level.push_back(value_x);

            break;

        default:

            color = Qt::red;
            if(block->Type_Block == bHold) color = Qt::blue;
            if(block->Type_Block == bPause) color = Qt::yellow;

            for(j=0; j<block->Temperature.size(); j++)
            {
                if(j) value_x += 20.;
                x.push_back(value_x);
                value_y = (double)(block->Temperature.at(j))/100.;
                y.push_back(value_y);
                plot_Program->curve->color_lev.push_back(Qt::darkGreen);
                block->Coordinates_Level.push_back(value_x);


                val = block->Time.at(j);
                if(block->Type_Block == bHold) val = 50.;
                value_x += val/(atan(val*0.1)/M_PI_2);
                x.push_back(value_x);
                y.push_back(value_y);
                plot_Program->curve->color_lev.push_back(color);
                block->Coordinates_Level.push_back(value_x);
            }
            break;
        }

    }
    plot_Program->curve->curves_border = &plot_Program->Curves_Border;
    plot_Program->curve->Blocks = &Blocks;
    plot_Program->curve->setSamples(x,y);
    plot_Program->replot();    
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Editor::Check_MeltingBlock()
{
    int i;
    double T, T_finish;
    int num_cycle;
    double incr;
    QString text;
    Block_Pr *block;
    bool sts = false;

    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);
        if(block->Type_Block != bMelt) continue;

        T = (double)(block->Temperature.at(0))/100.;
        incr = (double)(block->Incr_Temp.at(0))/100.;
        num_cycle = block->Num_Cycle;
        T_finish = T + incr*num_cycle;

        if(T_finish > 100 || T_finish < 0)
        {
            sts = true;
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setWindowTitle(tr("Attention"));
            text = tr("The wrong value of final temperature in the Melting block!");
            msgBox.setText(text);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();

            block->Num_Cycle = 1;
            block->Incr_Temp.replace(0,10);
        }

    }

    if(sts)
    {
        DrawBlock_inTable();
        DrawBlock_inGraph();
    }

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Editor::Value_Changed(int row, int col)
{
    int i,j,k;
    int i_level = 0;
    QString text, str_old, xtch_old;
    bool ok;
    int value;
    double dvalue;
    QTableWidgetItem *item = Editor_Table->item(row,col);

    Block_Pr *block;

    //--------------------------------------
    // In which block and where data changed
    //--------------------------------------
    int num = 0;
    bool sts = false;
    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);
        if(i) num++;

        for(j=0; j<block->Temperature.size(); j++)
        {
            if(num == row) {sts = true; i_level = j; break;}
            num++;
        }
        if(sts) break;
    }
    if(sts)
    {
        switch(col)
        {
        case 1:     str_old = QString::number((double)block->Temperature.at(i_level)/100.,'f',1);    break;
        case 2:     str_old = QString::number(block->Time.at(i_level)); break;
        case 3:     str_old = QString::number(block->Num_Cycle);    break;
        case 4:     str_old = QString::number(block->Measure.at(i_level)); break;
        case 5:     str_old = QString::number((double)block->Gradient.at(i_level)/10.,'f',1);break;
        case 6:     str_old = QString::number((double)block->Incr_Time.at(i_level)/10,'f',0); break;
        case 7:     str_old = QString::number((double)block->Incr_Temp.at(i_level)/100,'f',2); break;
        default:    str_old = "";   break;
        }
    }
    else qDebug()<< "Block is absent!";

    //---------------------------------------

    text = item->text();

    Editor_Table->blockSignals(true);

    sts = false;
    switch(col)
    {
    default:    break;

    case 1:                                         // Temperature
                dvalue = text.toDouble(&ok);
                if(ok)
                {
                   if(dvalue > 98.9) dvalue = 98.9;
                   if(dvalue < 1.0)  dvalue = 1.;
                   text = QString::number(dvalue,'f', 1);
                   sts = true;
                }
                else text = str_old;
                break;
    case 2:                                         // Time
                value = text.toInt(&ok);
                if(ok)
                {
                    if(value < 0) value = 1;
                    if(value > MAX_LEV_TIME) value = MAX_LEV_TIME;
                    text = QString::number(value);
                    sts = true;
                }
                else text = str_old;
                break;
    case 3:                                         // Num cycle
                value = text.toInt(&ok);
                if(ok)
                {
                    if(value > MAX_BLK_REP) value = MAX_BLK_REP;
                    if(value < 1)   value = 1;
                    text = QString::number(value);
                    sts = true;
                }
                else text = str_old;
                break;
    case 4:                                         // Measure
                value = text.toInt(&ok);
                if(ok)
                {
                    if(value) value = 1;
                    else value = 0;
                    text = QString::number(value);
                    sts = true;
                }
                else text = str_old;
                break;
    case 5:                                         // Gradient
                dvalue = text.toDouble(&ok);                
                if(ok)
                {
                   if(dvalue > 8.0) dvalue = 8.0;
                   if(dvalue < -8.0)  dvalue = -8.0;
                   text = QString::number(dvalue,'f', 1);
                   sts = true;
                }
                else text = str_old;                
                break;
    case 6:                                         // Incr_Time
                dvalue = text.toDouble(&ok);
                if(ok)
                {
                    if(dvalue > 100.0) dvalue = 100.0;
                    if(dvalue < -100.0)  dvalue = -100.0;
                    text = QString::number(dvalue,'f', 0);
                    sts = true;
                }
                else text = str_old;
                break;
    case 7:                                         // Incr_Temp
                dvalue = text.toDouble(&ok);
                if(ok)
                {
                   if(dvalue > 100.0) dvalue = 100.0;
                   if(dvalue < -100.0)  dvalue = -100.0;
                   text = QString::number(dvalue,'f', 2);
                   sts = true;
                }
                else text = str_old;
                break;
    }

    item->setText(text);

    if(sts)
    {
        switch(col)
        {
        default:    break;
        case 1:                 // Temperature
                    dvalue = text.toDouble(&ok);
                    if(ok)
                    {
                        block->Temperature.replace(i_level, dvalue * 100);
                    }
                    break;
        case 2:                 // Time
                    block->Time.replace(i_level, value);
                    break;
        case 3:                 // Num_cycle
                    block->Num_Cycle = value;
                    break;
        case 4:                 // Measure
                    block->Measure.replace(i_level, value);
                    break;
        case 5:                 // Gradient
                    block->Gradient.replace(i_level, roundTo(dvalue,1) * 10);
                    break;
        case 6:                 // Incr_Time
                    block->Incr_Time.replace(i_level, roundTo(dvalue,1) * 10);
                    break;
        case 7:                 // Incr_Temp
                    block->Incr_Temp.replace(i_level, roundTo(dvalue,1) * 100);
                    break;
        }

        DrawBlock_inGraph();    // -> Graph...
    }

    Editor_Table->blockSignals(false);

    Check_MeltingBlock();
}

//-----------------------------------------------------------------------------
//--- Change_TypeBlock
//-----------------------------------------------------------------------------
void Editor::Change_TypeBlock(int type)
{
    int i,j;
    int num = -1;
    int current_block = -1;
    Block_Pr *block;
    int size;
    int first_row;
    int row =  Editor_Table->currentRow();

    if(row < 0) return;

    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);

        if(i) num++;
        first_row = num + 1;
        num += block->Temperature.size();
        if(row <= num) {current_block = i; break;}
    }
    if(current_block < 0) return;

    size = block->Temperature.size();
    if(size > 1)
    {
        block->Temperature.remove(1,size-1);
        block->Time.remove(1,size-1);
        block->Measure.remove(1,size-1);
        block->Gradient.remove(1,size-1);
        block->Temperature_Gradient.remove(1,size-1);
        block->Incr_Temp.remove(1,size-1);
        block->Incr_Time.remove(1,size-1);
    }
    type++;
    block->Type_Block = (block_Type)type;

    switch(type)
    {
    case bPause:
    case bHold:
                    block->Num_Cycle = 1;
                    block->Measure.replace(0,0);
                    block->Gradient.replace(0,0);
                    block->Temperature_Gradient.replace(0,"");
                    block->Incr_Temp.replace(0,0);
                    block->Incr_Time.replace(0,0);

                    block->Temperature.clear();
                    block->Temperature.append(1000);    // 10 degrees
                    break;

    case bCycle:
                    block->Time.replace(0,15);
                    break;

    case bMelt:
                    block->Time.replace(0,15);
                    block->Incr_Temp.replace(0,100);
                    block->Measure.replace(0,2);
                    if(block->Num_Cycle == 0) block->Num_Cycle = 1;
                    break;

    default:
                    break;
    }

    if(type == bPause || type == bHold) block->Num_Cycle = 1;

    DrawBlock_inGraph();
    DrawBlock_inTable();

    Editor_Table->setCurrentCell(first_row, 1);
    Editor_Table->setFocus();
}

//-----------------------------------------------------------------------------
//--- Selection_Changed
//-----------------------------------------------------------------------------
void Editor::Selection_Changed(int row, int col, int prev_row, int prev_col)
{
    int i,j;
    int num = -1;
    int new_row, new_col;
    bool sts = false;
    int type = Editor_Delegate->Type.at(row);    
    Block_Pr *block;

    Button_Delegate->current_Rejime = col;

    if(type < bCycle)
    {
        sts = true;
        //... KeyBoard ...
        if(col == prev_col)
        {
            if(row > prev_row) new_row = row + 1;
            else
            {
                if(col == 0 || col == 3)    // 1) from bottom to up  2) block has two or more levels
                {
                    row--;
                    for(i=0; i<Blocks.size(); i++)
                    {
                        if(i) num++;
                        block = Blocks.at(i);
                        num += block->Temperature.size();
                        if(num == row)
                        {
                            new_row = num - block->Temperature.size() + 1;
                            break;
                        }
                    }
                }
                else  new_row = row - 1;
            }
            new_col = col;
        }
        else
        {
            new_row = row + 1;
            new_col = col;
        }
    }

    if(!type) type_Block->setEnabled(false);
    else type_Block->setEnabled(true);

    type_Block->blockSignals(true);
    type_Block->setCurrentIndex(type-1);
    type_Block->blockSignals(false);

    if(sts)
    {
        put_select(new_row,new_col);
    }
}

//-----------------------------------------------------------------------------
//--- get_select
//-----------------------------------------------------------------------------
void Editor::get_select(int row, int col)
{
    Editor_Table->setCurrentCell(row,col);
    Editor_Table->repaint();
}

//-----------------------------------------------------------------------------
//--- add_block()
//-----------------------------------------------------------------------------
void Editor::add_block()
{
    int i,j;
    int num = -1;
    int current_block;
    Block_Pr *block;
    QString text;

    int row = Editor_Table->currentRow();


    if(Blocks.size() >= TR_PRG_BLK)
    {
        text = QString("%1 (%2)").arg(tr("There is a limit on the number of blocks: ")).arg(TR_PRG_BLK);
        QMessageBox::warning(this, tr("Attention"), text);
        return;
    }

    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);

        if(i) num++;
        num += block->Temperature.size();
        if(row <= num) {current_block = i; break;}
    }


    //... create new block - cycle block
    block = new Block_Pr();
    block->Temperature.push_back(2000);
    block->Time.push_back(15);
    block->Num_Cycle = 1;
    block->Incr_Time.push_back(0);
    block->Incr_Temp.push_back(0);
    block->Gradient.push_back(0);
    block->Temperature_Gradient.push_back("");
    block->Measure.push_back(0);
    //...

    if(num < 0) Blocks.push_back(block);
    else Blocks.insert(current_block + 1, block);

    DrawBlock_inGraph();
    DrawBlock_inTable();


    if(num < 0) num = 0;
    else num += 2;
    Editor_Table->setCurrentCell(num, 1);
    Editor_Table->setFocus();

    //Editor_Table->setColumnWidth(0,40);
    //Editor_Table->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Fixed);
}
//-----------------------------------------------------------------------------
//--- add_level()
//-----------------------------------------------------------------------------
void Editor::add_level()
{
    int i,j;
    int num = -1;
    int current_level = -1;
    QString text;
    Block_Pr *block = NULL;

    int row = Editor_Table->currentRow();
    if(row < 0) return;

    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);

        if(i) num++;
        for(j=0; j<block->Temperature.size(); j++)
        {
            num++;
            if(row <= num) {current_level = j; break;}
        }
        if(current_level >= 0) break;
    }

    if(block && block->Type_Block == bCycle)
    {
        if(block->Temperature.size() >= TR_BLK_LEV)
        {
            text = QString("%1 (%2)").arg(tr("There is a limit on the number of levels: ")).arg(TR_BLK_LEV);
            QMessageBox::warning(this, tr("Attention"), text);
            return;
        }

        block->Temperature.insert(current_level + 1, 2000);
        block->Time.insert(current_level + 1, 15);
        block->Measure.insert(current_level + 1, 0);
        block->Gradient.insert(current_level + 1, 0);
        block->Temperature_Gradient.insert(current_level + 1, "");
        block->Incr_Temp.insert(current_level + 1, 0);
        block->Incr_Time.insert(current_level + 1, 0);

        DrawBlock_inGraph();
        DrawBlock_inTable();

        Editor_Table->setCurrentCell(num+1, 1);
        Editor_Table->setFocus();
    }
}
//-----------------------------------------------------------------------------
//--- delete_level()
//-----------------------------------------------------------------------------
void Editor::delete_level()
{
    int i,j;
    int num = -1;
    int current_level = -1;
    int current_block;
    Block_Pr *block = NULL;

    int row = Editor_Table->currentRow();
    if(row < 0) return;

    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);

        if(i) num++;
        for(j=0; j<block->Temperature.size(); j++)
        {
            num++;
            if(row <= num) {current_level = j; current_block = i; break;}
        }
        if(current_level >= 0) break;
    }

    if(block)
    {
        block->Temperature.remove(current_level);
        block->Time.remove(current_level);
        block->Measure.remove(current_level);
        block->Gradient.remove(current_level);
        block->Temperature_Gradient.remove(current_level);
        block->Incr_Temp.remove(current_level);
        block->Incr_Time.remove(current_level);

        if(block->Temperature.size() == 0)
        {
            Blocks.remove(current_block);
            delete block;
        }

        DrawBlock_inGraph();
        DrawBlock_inTable();

        num--;
        if(num < 0) num = 0;
        Editor_Table->setCurrentCell(num, 1);
        Editor_Table->setFocus();
    }

}

//-----------------------------------------------------------------------------
//--- delete_block()
//-----------------------------------------------------------------------------
void Editor::delete_block()
{
    int i,j;
    int num = -1;
    int prev_row = -1;
    int current_block;
    Block_Pr *block;

    int row = Editor_Table->currentRow();

    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);

        if(i) num++;
        num += block->Temperature.size();
        if(row <= num)
        {
            current_block = i;
            Blocks.remove(current_block);
            delete block;
            break;
        }
        else prev_row = num;
    }

    DrawBlock_inGraph();
    DrawBlock_inTable();

    if(prev_row >= 0)
    {
        if(Editor_Table->rowCount() > prev_row + 1) prev_row += 2;
        Editor_Table->setCurrentCell(prev_row, 1);
        Editor_Table->setFocus();
    }

}

//-----------------------------------------------------------------------------
//--- Name_Changed
//-----------------------------------------------------------------------------
void Editor::Name_Changed(QString text)
{
    Name_Pr = text;
}
//-----------------------------------------------------------------------------
//--- Volume_Changed
//-----------------------------------------------------------------------------
void Editor::Volume_Changed(int value)
{
    Volume_Pr = value;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Plot_ProgramAmpl::Plot_ProgramAmpl(QWidget *parent):
    QwtPlot(parent)
{
    curve = new ColorCurve();
    curve->setStyle(QwtPlotCurve::Lines);
    curve->attach(this);

    QColor color_bg = 0x99D9EA;
    /*if(StyleApp == "fusion")*/ color_bg = 0xEAF8FB;
    setCanvasBackground(color_bg);

    setAxisScale(QwtPlot::yLeft,0.,110.);
    QwtText title_Y;
    title_Y.setText(tr("Temperature,°C"));
    setAxisTitle(QwtPlot::yLeft, title_Y);
    title_Y.setFont(qApp->font());
    setAxisFont(QwtPlot::yLeft, qApp->font());
    //setAxisTitle(QwtPlot::yLeft, "Temperature, C");
    enableAxis(QwtPlot::xBottom,false);
    setCursor(Qt::ArrowCursor);
    canvas()->setCursor(Qt::ArrowCursor);


    grid = new QwtPlotGrid;
    grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
    grid->enableX(false);
    grid->attach(this);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Plot_ProgramAmpl::~Plot_ProgramAmpl()
{
    delete curve;
    Delete_Curves_Border();

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Plot_ProgramAmpl::clear_Program()
{

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Plot_ProgramAmpl::Delete_Curves_Border()
{
    qDeleteAll(Curves_Border.begin(), Curves_Border.end());
    Curves_Border.clear();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Plot_ProgramAmpl::draw_Program(QString s)
{
    //qDebug() << "pro:" << s;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ColorCurve::drawLines(QPainter *painter,
                           const QwtScaleMap &xMap,
                           const QwtScaleMap &yMap,
                           const QRectF &canvasRect,
                           int from,
                           int to) const
{
    int i,j,k;
    QPen pen;
    QRect rect;
    QPoint p1,p2,p;
    QColor color;
    QString text,str;
    double value;
    int dx;

    Block_Pr *block;

    QFontMetrics fm(painter->font());
    int w,dw;
    QImage image(":/images/foto.png");
    QImage image_temp(":/images/temp.png");
    QImage image_tau(":/images/tau.png");
    QImage image_grad(":/images/grad.png");

    if(*gradient == step_Gradient) image_grad.load(":/images/grad_step.png");

    for(i=from; i<to; i++)
    {
        if(color_lev.at(i) != Qt::darkGreen) pen.setWidth(2);
        else pen.setWidth(1);
        pen.setColor(color_lev.at(i));

        painter->setPen(pen);

        QwtPlotCurve::drawLines(painter, xMap, yMap, canvasRect, i, i+1);
    }

    pen.setWidth(1);
    pen.setColor(Qt::black);
    painter->setPen(pen);

    int h = yMap.transform(105.);

    for(i=0; i<Blocks->size(); i++)         // cycle for all blocks
    {
        block = Blocks->at(i);

        p1.setY(0);
        p2.setY(h);

        // LeftTop
        if(!i) p1.setX(canvasRect.bottomLeft().x()-2);
        else p1.setX(xMap.transform(curves_border->at(i-1)->data()->sample(0).x()));

        //RightBottom
        if(i == Blocks->size()-1) p2.setX(canvasRect.topRight().x());
        else p2.setX(xMap.transform(curves_border->at(i)->data()->sample(0).x()));

        rect.setTopLeft(p1);
        rect.setBottomRight(p2);

        k = Blocks->at(i)->Num_Cycle;
        text = QString("x%1").arg(k);

        switch(Blocks->at(i)->Type_Block)
        {
        case bCycle:    color = Qt::lightGray;
                        break;

        case bMelt:     color = (QColor)0x95ffca; //Qt::green;
                        str = QObject::tr("melting (");
                        text = str + text + ")";
                        break;

        case bPause:    color = (QColor)0xffffb4; //Qt::yellow;
                        text = QObject::tr("pause");
                        break;

        case bHold:     color = (QColor)0x95caff; //QColor(0,128,255);
                        text = QObject::tr("standby");
                        break;

        }

        painter->fillRect(rect, color);
        painter->drawRect(rect);
        painter->drawText(rect,Qt::AlignCenter,text);

        for(j=0; j<block->Temperature.size(); j++)          // cycle for all levels
        {
            dx = 0;

            //... temperature ...
            p1.setX(xMap.transform(block->Coordinates_Level.at(j*2)));
            p2.setX(xMap.transform(block->Coordinates_Level.at(j*2+1)));
            //p1.setY(yMap.transform(block->Temperature.at(j)/100.+8.));
            p1.setY(yMap.transform(block->Temperature.at(j)/100.) - 16);
            p2.setY(yMap.transform(block->Temperature.at(j)/100.));
            rect.setTopLeft(p1);
            rect.setBottomRight(p2);

            text = QString("%1°C").arg(block->Temperature.at(j)/100.,0,'f',1);
            if(block->Type_Block == bMelt)
            {
                value = block->Temperature.at(j)/100. + block->Num_Cycle*block->Incr_Temp.at(j)/100.;
                text += QString(" -> %1°C").arg(value,0,'f',1);
            }
            w = fm.width(text);
            if(rect.width() < w)
            {
                dw = w - rect.width() + 2;
                rect.setLeft(p1.x()-dw/2);
                rect.setRight(p2.x()+dw/2);
            }

            painter->drawText(rect,Qt::AlignCenter,text);

            //... measure ...
            if((block->Type_Block == bCycle || block->Type_Block == bMelt) && block->Measure.at(j))
            {
                p.setX(rect.left() + rect.width()/2 - image.width()/2);
                p.setY(rect.top() - image.height());
                if(block->Temperature.at(j)/100. > 90.) p.setY(rect.bottom() + rect.height());
                painter->drawImage(p,image);
            }

            //... time ...
            if(block->Type_Block == bCycle ||
               block->Type_Block == bMelt)
            {
                rect.setTop(yMap.transform(block->Temperature.at(j)/100.));
                //rect.setBottom(yMap.transform(block->Temperature.at(j)/100.-8.));
                rect.setBottom(yMap.transform(block->Temperature.at(j)/100.) + 16);

                text = QDateTime::fromTime_t(block->Time.at(j)).toUTC().toString("h:mm:ss");
                painter->drawText(rect,Qt::AlignCenter,text);
            }

            //... d_temp ...
            if(block->Incr_Temp.at(j))
            {
                p.setY(yMap.transform(0.) - image_temp.height());
                p.setX(rect.left()+rect.width()/2-image_temp.width()/2 + dx);
                painter->drawImage(p,image_temp);
                dx += image_temp.width();
            }
            //... d_tau ...
            if(block->Incr_Time.at(j))
            {
                p.setY(yMap.transform(0.) - image_tau.height());
                p.setX(rect.left()+rect.width()/2-image_tau.width()/2 + dx);
                painter->drawImage(p,image_tau);
                dx += image_tau.width();
            }
            //... grad ...
            if(block->Gradient.at(j))
            {
                p.setY(yMap.transform(0.) - image_grad.height());
                p.setX(rect.left()+rect.width()/2-image_grad.width()/2 + dx);
                painter->drawImage(p,image_grad);
                dx += image_grad.width();
            }
        }
    }
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget* EditorItemDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    QWidget* obj;

    if(index.column() == 1 || index.column() == 3 || index.column() == 6 || index.column() == 7)
    {
        obj = QStyledItemDelegate::createEditor(parent, option, index);
        obj->setStyleSheet("QLineEdit {selection-background-color: rgb(215,215,255); selection-color: black;}");
        return(obj);
    }
    return(nullptr);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void EditorItemDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text;
    int row,col;
    QColor color;
    int flag;
    int value;
    double dvalue;
    bool ok;
    int type;
    QPoint p;

    QImage image_meas(":/images/ok_green.png");

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    QStyledItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();
    type = Type.at(row);

    //... Background ...
    if(text.trimmed() == "...")                             // ... Separator ...
    {
        text = "";
        painter->fillRect(rect, QColor(220,220,220));
    }
    else                                                    // ... blocks ...
    {
        switch(type)
        {
        default:        color = QColor(0,0,0);
                        break;

        case bCycle:    color = QColor(250,250,250); //QColor(255,255,225);
                        break;
        case bMelt:     color = (QColor)0x95ffca;   //QColor(0,255,128);
                        break;
        case bPause:    color = (QColor)0xffffb4;   //QColor(255,255,128);
                        break;
        case bHold:     color = (QColor)0x95caff;   //QColor(0,128,255);
                        break;
        }
        if(option.state & QStyle::State_Selected) color = QColor(215,215,255);
        painter->fillRect(rect, color);
    }

    //... Data ...

    switch(col)
    {
    default:    text = "";
                break;

    case 0:                             // N block
                break;
    case 1:                             // Temperature
                break;
    case 2:                             // Time
                value = text.toInt(&ok);
                if(ok) text = QDateTime::fromTime_t(value).toUTC().toString("h:mm:ss");
                if(type == bHold || type == bPause) text = "-";
                break;
    case 3:                             // Num cycles                
                if(type == bHold) text = tr("standby");
                if(type == bPause) text = tr("pause");
                break;
    case 4:                             // Measure
                if(type == bHold || type == bPause) {text = "-"; break;}
                value = text.toInt(&ok);
                if(ok)
                {
                   if(value > 0)
                   {
                       p.setX(rect.left() + rect.width()/2 - image_meas.width()/2);
                       p.setY(rect.top() + rect.height()/2 - image_meas.height()/2);
                       painter->drawImage(p,image_meas);
                   }
                }
                text = "";
                break;
    case 5:                             // Gradient
                if(type == bHold || type == bPause) {text = "-"; break;}
                dvalue = text.toDouble(&ok);
                if(ok)
                {
                    if(dvalue == 0.) text = "";
                }
                else text = "";
                break;
    case 6:                             // Incr Time
                if(type == bHold || type == bPause) {text = "-"; break;}
                dvalue = text.toDouble(&ok);
                if(ok)
                {
                    if(dvalue == 0.) text = "";
                }
                else text = "";
                break;
    case 7:                             // Incr Temperature
                if(type == bHold || type == bPause) {text = "-"; break;}
                dvalue = text.toDouble(&ok);
                if(ok)
                {
                    if(dvalue == 0.) text = "";
                }
                else text = "";
                break;

    }

    painter->drawText(rect, Qt::AlignCenter, text);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ComboBoxDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    int i,id;
    QRect rect;
    QString text;
    int row,col;
    QColor color;
    int flag;
    int value;
    double dvalue;
    bool ok;
    int type;
    QPoint p;

    QImage image_meas(":/images/ok_green.png");

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    //QStyledItemDelegate::paint(painter, viewOption, index);
    QItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();
    type = Type->at(row);


    //... Background ...
    if(text.trimmed() == "...")                             // ... Separator ...
    {
        text = "";
        painter->fillRect(rect, QColor(220,220,220));
    }
    else                                                    // ... blocks ...
    {
        switch(type)
        {
        default:        color = QColor(0,0,0);
                        break;

        case bCycle:    color = QColor(250,250,250); //QColor(255,255,225);
                        break;
        case bMelt:     color = (QColor)0x95ffca;   //QColor(0,255,128);
                        break;
        case bPause:    color = (QColor)0xffffb4;   //QColor(255,255,128);
                        break;
        case bHold:     color = (QColor)0x95caff;   //QColor(0,128,255);
                        break;
        }
        if(option.state & QStyle::State_Selected) color = QColor(215,215,255);
        painter->fillRect(rect, color);
    }

    //... Data ...

    if(type == bHold || type == bPause) text = "-";
    else
    {
        value = text.toInt(&ok);
        if(ok)
        {
            if(value > 0)
            {
                p.setX(rect.left() + rect.width()/2 - image_meas.width()/2);
                p.setY(rect.top() + rect.height()/2 - image_meas.height()/2);
                painter->drawImage(p,image_meas);
            }
        }
        text = "";
    }
    painter->drawText(rect, Qt::AlignCenter, text);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ComboBoxDelegate::Close_Editor(int id)
{    
    QComboBox *bx = qobject_cast<QComboBox *>(sender());
    if(bx)
    {
        emit commitData(bx);    // fix data: This signal must be emitted when the editor widget has completed editing the data,
                                // and wants to write it back into the model.

        emit closeEditor(bx);   // This signal is emitted when the user has finished editing an item using the specified editor.
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ComboBoxDelegate::openComboPopup(QComboBox *bx)
{
    bx->showPopup();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
CEditLineButton::CEditLineButton(QWidget *parent) :
    QFrame(parent)
{
    setFrameShape(QFrame::NoFrame);
    lineEdit = new QLineEdit(this);
    button = new QToolButton(this);

    layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(lineEdit);
    layout->addWidget(button);
    button->setSizePolicy (QSizePolicy::Fixed,QSizePolicy::Minimum);
    lineEdit->setSizePolicy( QSizePolicy::Preferred,QSizePolicy::Preferred);
    connect (button, SIGNAL (clicked()), this, SLOT (clicked()));
    button->setText("...");
    //lineEdit->setFocus();
    lineEdit->setReadOnly(false);

   // lineEdit->setFocusProxy(button);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
CEditLineButton::~CEditLineButton()
{
    delete lineEdit;
    delete button;
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void CEditLineButton::clicked()
{    
    emit clickButton();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void CEditLineButton::setTextToEditLine(QString text)
{
    lineEdit->setText(text);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget *DelegatPerson::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{

    CEditLineButton *editor = new CEditLineButton (parent);
    editor->setStyleSheet("selection-background-color: #d7d7ff; selection-color: black;");
    editor->setTextToEditLine(index.data(Qt::EditRole).toString());   
    connect(editor, SIGNAL(clickButton()), this, SLOT(show_Dialog()));

    return(editor);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DelegatPerson::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    CEditLineButton *myEdit = static_cast<CEditLineButton*>(editor);
    myEdit->setTextToEditLine(index.data(Qt::EditRole).toString());

    QString text = index.data(Qt::EditRole).toString();

    myEdit->Param = text;

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DelegatPerson::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const
{    
    bool ok;
    CEditLineButton *myEdit = static_cast<CEditLineButton*>(editor);
    QString text = "0.0";

    double dvalue = myEdit->lineEdit->text().toDouble(&ok);
    if(ok) text = QString::number(dvalue,'f',1);

    model->setData(index, text, Qt::EditRole);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DelegatPerson::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DelegatPerson::show_Dialog()
{
    CEditLineButton *editor = qobject_cast<CEditLineButton *>(sender());

    //qDebug() << "load editor:" << editor->Param;

    //QDialog *my = new QDialog();
    //my->setWindowTitle(editor->Param);
    //my->exec();
    //delete my;

    //Gradient_Widget = new Gradient();
    //Gradient_Widget->exec();
    //delete Gradient_Widget;
    switch(current_Rejime)
    {
    case 5: emit signal_3D();
            break;

    case 6: break;

    case 7: emit signal_IncrTemp();
            break;

    default:    break;
    }

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Editor::show_Gradient()
{
    int i,j;
    bool ok;
    int num = 0;
    int i_level;
    bool sts = false;
    Block_Pr *block;
    int res;
    QTableWidgetItem *item;

    if(dll_3D)
    {
        int row = Editor_Table->currentRow();
        QString text = Editor_Table->item(row,1)->text();
        double value = text.toDouble(&ok);
        if(!ok) value = 0.;

        for(i=0; i<Blocks.size(); i++)
        {
            block = Blocks.at(i);
            if(i) num++;

            for(j=0; j<block->Temperature.size(); j++)
            {
                if(num == row) {sts = true; i_level = j; break;}
                num++;
            }
            if(sts) break;
        }

        if(sts)
        {
            Gradient_Info.gradient = block->Gradient.at(i_level);
            Gradient_Info.type = Gradient_Type;
            Gradient_Info.gradient_block = block->Temperature_Gradient.at(i_level);

            //qDebug() << "GRAD: " << Gradient_Info.gradient_block << Gradient_Info.type << Gradient_Info.gradient;

            Gradient_Widget->base = value;
            Gradient_Widget->BLOCKs.fill(value,6);
            Gradient_Widget->Base_Temperature->setText(QString("%1 °C").arg(value,5,'f',1));
            Gradient_Widget->Load_lib(dll_3D);
            res = Gradient_Widget->exec();
            if(res)
            {
                item = Editor_Table->item(row,5);
                item->setText(QString::number(Gradient_Info.gradient/10.,'f',1));
                block->Gradient.replace(i_level, Gradient_Info.gradient);
                Gradient_Type = Gradient_Info.type;
                block->Temperature_Gradient.replace(i_level, Gradient_Info.gradient_block);
            }
        }
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Editor::show_IncrTemperature()
{
    int res;
    int i,j;
    bool ok;
    int num = 0;
    int i_level;
    bool sts = false;
    Block_Pr *block;
    int row = Editor_Table->currentRow();
    QString text = Editor_Table->item(row,1)->text();
    double value = text.toDouble(&ok);
    QTableWidgetItem *item;
    if(!ok) value = 0.;

    for(i=0; i<Blocks.size(); i++)
    {
        block = Blocks.at(i);
        if(i) num++;

        for(j=0; j<block->Temperature.size(); j++)
        {
            if(num == row) {sts = true; i_level = j; break;}
            num++;
        }
        if(sts) break;
    }

    if(sts)
    {
        Incr_Temp = new Incr_Temperature(this);
        IncrTemperature_Info = &Incr_Temp->incrTemp_Info;

        IncrTemperature_Info->dt = block->Incr_Temp.at(i_level)/100.;
        IncrTemperature_Info->T_s = value;
        IncrTemperature_Info->T_f = value;
        IncrTemperature_Info->N = block->Num_Cycle;

        res = Incr_Temp->exec();
        //qDebug() << "res: " << res;

        if(res)
        {
            qDebug() << "Incr: " << IncrTemperature_Info->T_s << IncrTemperature_Info->dt << IncrTemperature_Info->N;

            //item = Editor_Table->item(row,7);
            //item->setText(QString::number(IncrTemperature_Info->dt,'f',1));

            block->Incr_Temp.replace(i_level, IncrTemperature_Info->dt*100);
            block->Num_Cycle = IncrTemperature_Info->N;
            block->Temperature.replace(i_level, IncrTemperature_Info->T_s*100);

            DrawBlock_inGraph();
            DrawBlock_inTable();

        }

        delete Incr_Temp;


    }


}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void DelegatPerson::paint (
            QPainter *painter,
            const QStyleOptionViewItem& option,
            const QModelIndex& index ) const
{
    int i,id;
    QRect rect;
    QString text;
    int row,col;
    QColor color;
    int flag;
    int value;
    double dvalue;
    bool ok;
    int type;
    QPoint p;


    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus
    //QStyledItemDelegate::paint(painter, viewOption, index);
    QItemDelegate::paint(painter, viewOption, index);

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();
    type = Type->at(row);


    //... Background ...
    if(text.trimmed() == "...")                             // ... Separator ...
    {
        text = "";
        painter->fillRect(rect, QColor(220,220,220));
    }
    else                                                    // ... blocks ...
    {
        switch(type)
        {
        default:        color = QColor(0,0,0);
                        break;

        case bCycle:    color = QColor(250,250,250); //QColor(255,255,225);
                        break;
        case bMelt:     color = (QColor)0x95ffca;   //QColor(0,255,128);
                        break;
        case bPause:    color = (QColor)0xffffb4;   //QColor(255,255,128);
                        break;
        case bHold:     color = (QColor)0x95caff;   //QColor(0,128,255);
                        break;
        }
        if(option.state & QStyle::State_Selected) color = QColor(215,215,255);
        painter->fillRect(rect, color);
    }

    //... Data ...

    if(type == bHold || type == bPause) text = "-";
    else
    {
        dvalue = text.toDouble(&ok);
        if(ok)
        {
            if(dvalue == 0.) text = "";
            else text =QString::number(dvalue,'f',1);
        }
        else text = "";
    }
    painter->drawText(rect, Qt::AlignCenter, text);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
QWidget *TimeEditorDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{

    QTimeEdit *editor = new QTimeEdit(parent);
    editor->setStyleSheet("selection-background-color: #d7d7ff; selection-color: black;");
    editor->setDisplayFormat("h:mm:ss");
    //editor->setTextToEditLine(index.data(Qt::EditRole).toString());
    //connect(editor, SIGNAL( clickButton()), this, SLOT(show_Dialog()));


    return(editor);

}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TimeEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{     
     int secs = index.model()->data(index, Qt::DisplayRole).toInt();
     if(secs <= 0) secs = 1;
     QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
     int h = secs/3600;
     int m = (secs - h*3600)/60;
     int s = secs - h*3600 - m*60;
     timeEdit->setTime(QTime(h, m, s));     
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TimeEditorDelegate::setModelData(QWidget *editor,
                                 QAbstractItemModel *model,
                                 const QModelIndex &index) const
{    
    QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
    QTime time = timeEdit->time();
    int secs = (time.hour() * 3600) + (time.minute() * 60) + time.second();
    if(secs <= 0) secs = 1;
    model->setData(index, secs, Qt::EditRole);    
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TimeEditorDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{    
    QRect r = option.rect;
    r.setTop(r.top()-1);
    r.setBottom(r.bottom()+1);
    editor->setGeometry(r);    
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void TimeEditorDelegate::paint (QPainter *painter,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index ) const
{
    QRect rect;
    QString text;
    int row,col;
    QColor color;
    int value;
    bool ok;
    int type;

    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus

    //QStyledItemDelegate::paint(painter, viewOption, index);
    QItemDelegate::paint(painter, viewOption, index);

    //qDebug() << "paint start:";

    rect = option.rect;

    text = index.data().toString();
    row = index.row();
    col = index.column();
    type = Type->at(row);

    //... Background ...
    if(text.trimmed() == "...")                             // ... Separator ...
    {
        text = "";
        painter->fillRect(rect, QColor(220,220,220));
    }
    else                                                    // ... blocks ...
    {
        switch(type)
        {
        default:        color = QColor(0,0,0);
                        break;

        case bCycle:    color = QColor(250,250,250);
                        break;
        case bMelt:     color = (QColor)0x95ffca;   //QColor(0,255,128);
                        break;
        case bPause:    color = (QColor)0xffffb4;   //QColor(255,255,128);
                        break;
        case bHold:     color = (QColor)0x95caff;   //QColor(0,128,255);
                        break;
        }
        if(option.state & QStyle::State_Selected) color = QColor(215,215,255);
        painter->fillRect(rect, color);
    }

    //... Data ...

    if(type == bHold || type == bPause) text = "-";
    else
    {

        value = text.toInt(&ok);
        //if(ok) text = QDateTime::fromTime_t(value).toString("h:mm:ss");
        if(ok)
        {
            QTime tim(0,0,0);
            QTime tau;
            tau = tim.addSecs(value);
            text = tau.toString("h:mm:ss");
        }
        else text = "...";
    }
    painter->drawText(rect, Qt::AlignCenter, text);

    //qDebug() << "paint stop:";

}


