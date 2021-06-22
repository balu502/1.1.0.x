#include "zoomplate.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ZoomPlate::ZoomPlate(rt_Protocol *P, QString fn, QMap<QString,QString> *map_Translate, QWidget *parent) : QDialog(parent)
{
    int i,j;
    int row,col;
    int num_tube = 1;
    QStringList header;
    QFont f = qApp->font();

    file_name = fn;

    rt_GroupSamples *group;
    rt_Sample       *sample;
    rt_Tube         *tube;
    QString name_sample;
    QString name_test;
    QString name;
    int pos, num, count;
    QTableWidgetItem* item;

    int count_tube = P->count_Tubes;
    P->Plate.PlateSize(count_tube, row, col);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QHBoxLayout *layout_control = new QHBoxLayout;
    print_Button = new QPushButton(QIcon(":/images/flat/print_32.png"),"", this);
    png_Button = new QPushButton(QIcon(":/images/flat/toPNG.png"),"", this);
    print_Button->setFocusPolicy(Qt::NoFocus);
    png_Button->setFocusPolicy(Qt::NoFocus);
    connect(png_Button, SIGNAL(clicked(bool)), this, SLOT(zoom_SaveAsPNG()));
    print_Button->setIconSize(QSize(24,24));
    png_Button->setIconSize(QSize(24,24));
    layout_control->addSpacing(10);
    layout_control->addWidget(print_Button);
    layout_control->addWidget(png_Button);
    layout_control->addStretch(1);

    print_Button->setVisible(false);

    zoom_Table = new QTableWidget(row,col, this);
    zoom_Table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zoom_Table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    zoom_Table->horizontalHeader()->setFont(f);
    zoom_Table->verticalHeader()->setFont(f);
    zoom_Table->setFont(f);
    zoom_Table->setSelectionMode(QAbstractItemView::NoSelection);
    zoom_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    header.clear();
    for(i=0; i<row; i++) header.append(QChar(0x41 + i));
    zoom_Table->setVerticalHeaderLabels(header);
    ZoomPlateItemDelegate *delegate = new ZoomPlateItemDelegate();
    zoom_Table->setItemDelegate(delegate);
    delegate->p_prot = P;

    foreach(group, P->Plate.groups)
    {
        foreach(sample, group->samples)
        {
            num = 1;
            count = sample->tubes.size();
            foreach(tube, sample->tubes)
            {
                pos = tube->pos;
                if(pos >= 0 && pos < count_tube)
                {
                    item = new QTableWidgetItem();
                    i = div(pos,col).quot;
                    j = div(pos,col).rem;
                    zoom_Table->setItem(i, j, item);
                    name_sample = QString::fromStdString(sample->Unique_NameSample);
                    name_test = QString::fromStdString(sample->p_Test->header.Name_Test);
                    if(map_Translate->size()) name_test = map_Translate->value(name_test, name_test);
                    name = QString("%1\r\n%2").arg(name_sample).arg(name_test);
                    if(count > 1) name += QString("\r\n %1(%2)").arg(num).arg(count);
                    name += QString(";%1").arg(num_tube);
                    item->setText(name);
                }
                num++;
                num_tube++;
            }
        }
    }

    layout->addLayout(layout_control);
    layout->addWidget(zoom_Table);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowState(Qt::WindowMaximized);
    setWindowTitle(tr("Location Samples/Tests"));
    //qApp->setAttribute(Qt::AA_DisableWindowContextHelpButton);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
ZoomPlate::~ZoomPlate()
{
    delete print_Button;
    delete png_Button;
    delete delegate;
    delete zoom_Table;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ZoomPlate::zoom_SaveAsPNG()
{
    QString filename = "";

    filename = QFileDialog::getSaveFileName(this, tr("Save to Image..."),
                                                file_name, tr("Images (*.png)"));
    if(!filename.isEmpty())
    {
        QPixmap pixmap(zoom_Table->grab());
        pixmap.save(filename, "PNG");
    }
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void ZoomPlateItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor color,rgb;
    int pos;
    QString str = "";
    QRect rect = option.rect;
    QFont f = qApp->font();

    QPalette pal = option.palette;
    QStyleOptionViewItem  viewOption(option);
    viewOption.state &= ~QStyle::State_HasFocus;    // disable state HasFocus


    QStyledItemDelegate::paint(painter, viewOption, index);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setFont(f);
    pos = index.column() + index.row() * index.model()->columnCount();
    QString text = index.data().toString();
    QStringList list = text.split(";");
    text = list.at(0);
    if(list.size() > 1) str = list.at(1) + " ";

    //__1. Background
    painter->fillRect(option.rect, QColor(255,255,255));   // clear
    if(text.isEmpty())
    {
       return;
    }

    color = p_prot->color_tube.at(pos);
    color.setAlpha(80);
    painter->fillRect(option.rect, color);

    //__2. Data
    painter->drawText(option.rect,Qt::AlignLeft | Qt::AlignVCenter,text);
    painter->drawText(option.rect,Qt::AlignRight | Qt::AlignBottom, str);
}
