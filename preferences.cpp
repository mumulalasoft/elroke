#include "preferences.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QFontComboBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QDebug>
#include <QFileDialog>
#include <QListWidget>
#include <QSettings>
#include <QDirIterator>
#include <QFileInfo>
#include <QTextStream>
#include "liststringfileparser.h"

preferences::preferences(QWidget *parent) : QDialog(parent)
{
    readSetting();
//    setCursor(Qt::WaitCursor);
    QHBoxLayout *main_layout = new QHBoxLayout;

    QVBoxLayout *layout_button = new QVBoxLayout;

    auto *button_system = new QPushButton(tr("System"), this);
    auto *button_font = new QPushButton(tr("Font"), this);
    auto *button_theme = new QPushButton(tr("Theme"), this);
    auto *button_menu = new QPushButton(tr("Menu"), this);
    auto *button_background = new QPushButton(tr("Background"), this);

    layout_button->addWidget(button_system);
    layout_button->addWidget(button_font);
    layout_button->addWidget(button_theme);
    layout_button->addWidget(button_menu);
    layout_button->addWidget(button_background);

    layout_button->setSpacing(0);
    layout_button->setMargin(0);
    layout_button->addStretch();

    auto *button_ok = new QPushButton(tr("Ok"), this);
    connect(button_ok,&QPushButton::pressed,this,&preferences::ok);
    auto *button_apply = new QPushButton(tr("Apply"), this);
    connect(button_apply,&QPushButton::pressed,this,&preferences::apply);
    auto *button_cancel = new QPushButton(tr("Cancel"), this);
    connect(button_cancel,&QPushButton::pressed,this,&preferences::close);

    layout_button->addWidget(button_ok);
    layout_button->addWidget(button_apply);
    layout_button->addWidget(button_cancel);

    stack = new QStackedLayout;

    QVBoxLayout *layout_background = new QVBoxLayout;

    QDirIterator it("/usr/share/elroke//background",QStringList()<<"*.jpg"<<"*.jpeg"<<"*.png",QDir::Files, QDirIterator::NoIteratorFlags);

    QFileInfo info;

    while(it.hasNext())
    {
        info.setFile(it.next());
        bg_list<<info.absoluteFilePath();
    }

    img_resizer = new resizer();
    img_resizer->setInputs(bg_list);

    thread_resizer = new QThread(this);

    img_resizer->moveToThread(thread_resizer);

    qRegisterMetaType< QList<QImage>>( "QList<QImage>" );

    connect(thread_resizer,SIGNAL(started()),img_resizer,SLOT(start()));
    connect(img_resizer,SIGNAL(finished(QList<QImage>)),this,SLOT(handleImage(QList<QImage>)));
    thread_resizer->start();

    QListView *background_view  = new QListView(this);
    background_view->setViewMode(QListView::IconMode);
    background_view->setIconSize(QSize(320,180));

    model = new QStandardItemModel;
    background_view->setModel(model);

   QList<QStandardItem*>l = model->findItems(selected_background);
    if(!l.isEmpty()){
        background_view->setCurrentIndex(l.at(0)->index());
    }

    connect(background_view,&QListView::clicked,[this,background_view](const QModelIndex &index)
   {
       selected_background = background_view->model()->data(index,Qt::UserRole).toString();
    });

    auto *button_add = new QPushButton(QIcon::fromTheme("add"),"", this);

    layout_background->addWidget(background_view);
    layout_background->addWidget(button_add,0,Qt::AlignLeft);
    connect(button_add,&QPushButton::pressed,[this]()
    {
         QStringList f_list =  QFileDialog::getOpenFileNames(nullptr,tr("Choose image"), QDir::homePath(),tr("Image Files (*.png *.jpg *.jpeg *.PNG *.JPG *.JPEG)"));
    });

    auto *group_system = new QGroupBox(tr("System"),this);
    QVBoxLayout *layout_system = new QVBoxLayout;

    check_startapp  = new QCheckBox(tr("Run at startup"), this);
    check_startapp->setChecked(startup);
    layout_system->addWidget(check_startapp);

    connect(check_startapp,&QCheckBox::toggled,[this](bool d)
    {
        startup = d;
    });


    QGroupBox *group_font = new QGroupBox(tr("Font"), this);

    QVBoxLayout *layout_font = new QVBoxLayout;

    QFontComboBox *combo_font = new QFontComboBox(this);

    if(!selected_font.isEmpty())
        combo_font->setCurrentText(selected_font);
    connect(combo_font,static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),[this](const QString &s_font)
    {
        selected_font = s_font;
    });

    QHBoxLayout *layout_sub_font = new QHBoxLayout;
    QSpinBox *spin_font_size = new QSpinBox(this);
    spin_font_size->setRange(14,48);//not tested
    spin_font_size->setValue(font_size);

    layout_sub_font->addWidget(combo_font);
    layout_sub_font->addWidget(spin_font_size);
    layout_sub_font->addStretch();
    layout_font->addLayout(layout_sub_font);
    layout_font->addStretch();
    group_font->setLayout(layout_font);

    connect(spin_font_size,static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),[this](int val)
    {
        font_size = val;
    });

    auto *group_theme = new QGroupBox(tr("Themes"), this);
    auto *group_menu = new QGroupBox(tr("Menu"), this);

    QHBoxLayout *layout_menu = new QHBoxLayout;

    QListWidget* list_menu_all = new QListWidget(this);
    QVBoxLayout *layout_move_button = new QVBoxLayout;

    auto *button_right = new QPushButton(QIcon::fromTheme("go-next"),"",this);

    auto *button_left = new QPushButton(QIcon::fromTheme("go-previous"),"", this);

    layout_move_button->addStretch();
    layout_move_button->addWidget(button_right);
    layout_move_button->addWidget(button_left);
    layout_move_button->addStretch();

    QListWidget *list_menu_selected = new QListWidget(this);
    layout_menu->addWidget(list_menu_all,0,Qt::AlignVCenter);
    layout_menu->addLayout(layout_move_button);
    layout_menu->addWidget(list_menu_selected,0,Qt::AlignVCenter);
    group_menu->setLayout(layout_menu);

    list_menu_all->addItems(getLanguageGenre());

    connect(button_right,&QPushButton::clicked,[list_menu_all,list_menu_selected]()
    {
       list_menu_selected->addItem(list_menu_all->takeItem(list_menu_all->currentRow()));
    });

    connect(button_left,&QPushButton::clicked,[list_menu_all,list_menu_selected]()
    {
       list_menu_all->addItem(list_menu_selected->takeItem(list_menu_selected->currentRow()));
    });

    QGroupBox *group_background = new QGroupBox(tr("Background"), this);
    group_background->setLayout(layout_background);

    QComboBox *combo_language = new QComboBox(this);
    combo_language->addItem("English");
    combo_language->addItem("Bahasa Indonesia");
    connect(combo_language,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),[this](int index)
    {
        switch (index) {
        case 0:
            language = "english";
            break;
        case 1:
            language = "indonesian";
        default:
            break;
        }
    });

    QHBoxLayout *layout_language = new QHBoxLayout;
    layout_language->addWidget(new QLabel(tr("Language"),this));
    layout_language->addWidget(combo_language);
    layout_system->addLayout(layout_language);

    layout_system->addStretch();
    group_system->setLayout(layout_system);

    stack->addWidget(group_system);
    stack->addWidget(group_font);
    stack->addWidget(group_theme);
    stack->addWidget(group_menu);
    stack->addWidget(group_background);


    connect(button_background,&QPushButton::pressed,[this]()
    {
        stack->setCurrentIndex(4);
    });

    connect(button_font,&QPushButton::pressed,[this]()
    {
       stack->setCurrentIndex(1);
    });

    connect(button_theme,&QPushButton::pressed,[this]()
    {
        stack->setCurrentIndex(2);
    });
    connect(button_menu,&QPushButton::pressed,[this]()
    {
        stack->setCurrentIndex(3);
    });
    connect(button_system,&QPushButton::pressed,[this]()
    {
        stack->setCurrentIndex(0);
    });

    QPalette palet;
    palet.setColor(QPalette::Base, Qt::white);
    palet.setColor(QPalette::Window, Qt::white);
    palet.setColor(QPalette::Text, QColor(0,0,0,128));
    palet.setColor(QPalette::WindowText, QColor(0,0,0,128));
    palet.setColor(QPalette::Button, palette().dark().color());
    palet.setColor(QPalette::ButtonText, Qt::white);

    setPalette(palet);

    main_layout->addLayout(layout_button);
    main_layout->addLayout(stack);
    setLayout(main_layout);
    setWindowFlags(Qt::Popup);
}

QStringList preferences::getLanguageGenre()
{
    QStringList list ;
    list<< listStringFileParser::parse(app_dir+"/meta/language");
    list<<listStringFileParser::parse(app_dir+"/meta/category");
    return list;
}

void preferences::apply()
{
    QDir dir(QDir::homePath()+"/.config/autostart");
    QFile file(dir.path()+"/elroke.desktop");
   if(startup)
   {
       if(file.exists())
       {
           return;
       }
       if(!dir.exists())
       {
          QDir().mkdir(dir.path());
       }

       QTextStream stream(&file);
       if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
           qDebug()<<"unable open"+dir.path()+"/.config/autostart/elroke.desktop";
       stream<<
               "[Desktop Entry]\n"
               "Version=1.0\n"
               "Terminal=false\n"
               "Icon=elroke\n"
               "Type=Application\n"
                "Name=Elroke\n"
                "GenericName=Karaoke Entertainment\n"
               "Categories=Video\n"
               "Exec=elroke";
   file.close();
   }
   else
   {
       file.remove();
   }

    QSettings setting("elroke","elroke");
    setting.beginGroup("Preferences");
    setting.setValue("font",selected_font);
    setting.setValue("background", selected_background);
    setting.setValue("font_size", font_size);
    setting.setValue("startup", startup);
    setting.setValue("language", language);
    setting.endGroup();
}
void preferences::ok()
{
    apply();
    accept();
}

void preferences::readSetting()
{
    QSettings setting("elroke","elroke");
    setting.beginGroup("Preferences");
    selected_font = setting.value("font").toString();
    selected_background = setting.value("background").toString();
    font_size = setting.value("font_size").toInt();
    startup = setting.value("startup").toBool();
    language = setting.value("language").toString();
    setting.endGroup();
}

void preferences::handleImage(QList<QImage> imglist)
{
    for(int i =0;i<imglist.count();i++)
    {
        QStandardItem *item = new QStandardItem;
        item->setIcon(QPixmap::fromImage(imglist.at(i)));
        item->setData(bg_list.at(i),Qt::UserRole);
        model->setItem(i,0,item);
    }
}

preferences::~preferences()
{
    thread_resizer->quit();
    thread_resizer->wait();
    delete img_resizer;
}
