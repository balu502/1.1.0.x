#include "video_archive.h"

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Video_Archive::Video_Archive()
{
    readCommonSettings();
    setFont(qApp->font());

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->setMargin(4);
    QHBoxLayout *layout_open = new QHBoxLayout;
    layout_open->setMargin(0);
    file_open = new QLabel();
    file_open->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    file_open->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    file_open->setStyleSheet("QLabel{background-color: rgb(255, 255, 255)}");
    open = new QPushButton(QIcon(":/images/open_flat.png"), NULL, this);    
    open->setObjectName("Transparent");
    open->setIconSize(QSize(20,20));
    open->setFlat(true);
    open->setToolTip(tr("Open Image file"));
    layout_open->addWidget(file_open, 1);
    layout_open->addWidget(open);

    Image = new QImage(W_IMAGE_COEF, H_IMAGE, QImage::Format_RGB32);
    Image->fill(Qt::white);
    image_Widget = new Image_Widget(Image, this);
    image_Widget->setFixedSize(W_IMAGE_COEF, H_IMAGE);

    QHBoxLayout *layout_control = new QHBoxLayout;
    layout_control->setMargin(0);
    next_video = new QPushButton(QIcon(":/images/next.png"), "", this);
    next_video->setToolTip(tr("next image"));
    next_video->setEnabled(false);
    previous_video = new QPushButton(QIcon(":/images/previous.png"), "", this);
    previous_video->setToolTip(tr("previous image"));
    previous_video->setEnabled(false);
    layout_control->addWidget(previous_video);
    layout_control->addWidget(next_video);

    layout->addLayout(layout_open);
    layout->addWidget(image_Widget);
    layout->addLayout(layout_control);

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowFlags(this->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle(tr("Preview Video Archive"));
    setWindowIcon(QIcon(":/images/DTm.ico"));

    connect(open, SIGNAL(clicked(bool)), this, SLOT(Open_File()));
    connect(next_video, SIGNAL(clicked(bool)), this, SLOT(Next_Video()));
    connect(previous_video, SIGNAL(clicked(bool)), this, SLOT(Previous_Video()));

    Video_BUF = new QVector<int>(W_IMAGE*H_IMAGE);
    List_FN = new QStringList();
    current_index = -1;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Video_Archive::~Video_Archive()
{

    delete open;
    delete file_open;
    delete Image;
    delete image_Widget;
    delete next_video;
    delete previous_video;

    Video_BUF->clear();
    delete Video_BUF;
    List_FN->clear();
    delete List_FN;
}
//-----------------------------------------------------------------------------
//--- readCommonSettings()
//-----------------------------------------------------------------------------
void Video_Archive::readCommonSettings()
{
    QString text;
    QString dir_path = qApp->applicationDirPath();
    QSettings *CommonSettings = new QSettings(dir_path + "/tools/ini/preference.ini", QSettings::IniFormat);

    CommonSettings->beginGroup("Common");

        //... Language ...
        text = CommonSettings->value("language","ru").toString();
        if(translator.load(":/translations/archive_" + text + ".qm"))
        {
            qApp->installTranslator(&translator);
        }

        //... Style ...
        //text = CommonSettings->value("style","xp").toString();
        //StyleApp = text;

    CommonSettings->endGroup();
    delete CommonSettings;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Video_Archive::Clear_Image()
{
    Image->fill(Qt::white);
    image_Widget->update();
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Video_Archive::Draw_Image(QString fname)
{
    int i,j;
    int num = 0;
    int pixel;
    QRgb pb = QColor(255,0,0).rgba();
    QByteArray data;
    QVector<ushort> buffer;
    QString text, str;
    int index;

    QFile fn(fname);
    QFileInfo fi(fn);
    if(!fi.exists()) return;
    QDir dir(fi.absoluteDir());
    QString name = dir.rootPath() + "../" + fi.fileName();
    file_open->setText(name);

    List_FN->clear();
    str = fi.fileName().mid(0,10);
    foreach(text, dir.entryList(QDir::Files))
    {
        if(text.contains(str) && text.startsWith("video_"))
        {
            List_FN->append(dir.absolutePath() + "/" + text);
        }
    }
    if(List_FN->size()) qSort(List_FN->begin(), List_FN->end());

    next_video->setEnabled(true);
    previous_video->setEnabled(true);
    index = List_FN->indexOf(fname);
    current_index = index;
    if(current_index < 0)
    {
        next_video->setEnabled(false);
        previous_video->setEnabled(false);
        List_FN->clear();
        return;
    }
    if(index == 0) previous_video->setEnabled(false);
    if(index == List_FN->size()-1) next_video->setEnabled(false);


    Video_BUF->fill(0);
    QImage image(W_IMAGE, H_IMAGE, QImage::Format_RGB32);

    if(fn.open(QFile::ReadOnly))
    {
        data = fn.readAll();

        buffer.resize(data.size()/2);
        memcpy(buffer.data(), data.data(), data.size());

        for(i=0; i<buffer.size(); i++)
        {
            Video_BUF->replace(i, (int)((buffer.at(i) >> 4) & 0xfff));
        }
        fn.close();
    }

    for(i=0; i<H_IMAGE; i++)
    {
        QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(i));
        for(j=0; j<W_IMAGE; j++)
        {
            pixel = Video_BUF->at(num);
            pixel = (pixel >> 4) & 0xff;
            pb = qRgb(pixel,pixel,pixel);
            line[j] = pb;
            num++;
        }
    }

    // Scaled image: ...
    *Image = image.scaled(W_IMAGE_COEF, H_IMAGE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    image_Widget->update();

    data.clear();
    buffer.clear();
}

//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Video_Archive::Open_File()
{
    QString fileName = QFileDialog::getOpenFileName(this,
          tr("Open Image"), "", tr("Image Files (*.dat)"));

    if(fileName.isEmpty()) return;

    Draw_Image(fileName);

}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Video_Archive::Next_Video()
{
    if(current_index < (List_FN->size() - 1))
    {
        current_index++;
        Draw_Image(List_FN->at(current_index));
    }
    else next_video->setEnabled(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Video_Archive::Previous_Video()
{
    if(current_index > 0)
    {
        current_index--;
        Draw_Image(List_FN->at(current_index));
    }
    else previous_video->setEnabled(false);
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
Image_Widget::Image_Widget(QImage *picture, QWidget *parent): QWidget(parent)
{
    p_archive = (Video_Archive*)parent;
    image = picture;
}
//-----------------------------------------------------------------------------
//---
//-----------------------------------------------------------------------------
void Image_Widget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    painter.setPen(Qt::black);

    //... image ...
    painter.drawImage(0,0,*image);


    //... border ...
    painter.drawRect(0,0, width()-1, height()-1);
}
