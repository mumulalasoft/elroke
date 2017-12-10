/* -----------------------------------------------------------------
+  CopyLeft Muhammad Mukharom                       +
+ --------------------------------------------------------------*/

#include "mainwindow.h"
#include "about.h"
#include "addtodatabase.h"
#include "aerobutton.h"
#include "editdb.h"
#include "liststringfileparser.h"

#include <QAction>
#include <QDebug>
#include <QDirIterator>
#include <QFile>
#include <QFontDatabase>
#include <QVBoxLayout>
#include <QMenu>
#include <QHeaderView>
#include <QShortcut>
#include <QTime>
#include <QMessageBox>
#include <thread>
#include <chrono>
#include <QMutex>

using namespace std;

mainWindow::mainWindow(QWidget *parent)
    : QDialog(parent)
{
    fontSetup();

    createWidgets();
    keyBoardInstance();
    videoInstance();
    createShortcut();
    setBackground();

    getCategory();
    setWindowFlags(Qt::FramelessWindowHint);
  // setAttribute( Qt::AA_EnableHighDpiScaling);


}

void mainWindow::createWidgets(){

       resize(desktop_width, desktop_height);


    QVBoxLayout *lo_main = new QVBoxLayout;

    QHBoxLayout *lo_top = new QHBoxLayout;

    auto *pb_menu = new QPushButton(QIcon(":/usr/share/elroke/file/icon/logo.png"),"", this);
    pb_menu->setFlat(1);
    pb_menu->setFocusPolicy(Qt::NoFocus);
    connect(pb_menu,SIGNAL(pressed()),this,SLOT(checkAdmin()));

    AeroButton *pb_all = new AeroButton(tr("ALL"), this);
    AeroButton *button_cat_indonesia = new AeroButton("category1", this);
    AeroButton *button_cat_barat = new AeroButton("category2", this);
    AeroButton *button_cat_rock = new AeroButton("category3", this);
    AeroButton *button_cat_pop = new AeroButton("category4", this);
    AeroButton *button_cat_dangdut = new AeroButton("category5", this);

//    combo_cat = new QComboBox(this);
//    combo_cat->setSizeAdjustPolicy(QComboBox::AdjustToContents);


//show clock
    clock =new QLabel(this);
    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(showClock()));
    timer->start(60000);
    showClock();


    lo_top->addWidget(pb_menu);
        lo_top->addStretch();
    lo_top->addWidget(pb_all);
    lo_top->addWidget(button_cat_indonesia);
    lo_top->addWidget(button_cat_barat);
    lo_top->addWidget(button_cat_rock);
    lo_top->addWidget(button_cat_pop);
    lo_top->addWidget(button_cat_dangdut);
    lo_top->addStretch();
//    lo_top->addWidget(new QLabel("OTHER :", this));
//    lo_top->addWidget(combo_cat);
//    lo_top->addStretch();
    lo_top->addWidget(clock);
    lo_top->setSpacing(0);
    lo_top->setMargin(0);

    QWidget *widget_top = new QWidget(this);
    QPixmap bg_trans(":/usr/share/elroke/file/background/backgroundTrans.png");
    QPalette palette_bg;

    bg_trans =   bg_trans.scaled(widget_top->size(),Qt::IgnoreAspectRatio,Qt::FastTransformation);
    palette_bg.setBrush(QPalette::Background, bg_trans);


    widget_top->setAutoFillBackground(1);
    widget_top->setPalette(palette_bg);
    widget_top->setLayout(lo_top);


    QHBoxLayout *lo_table = new QHBoxLayout;

    QVBoxLayout *lo_left = new QVBoxLayout;

    QPalette table_palette;
    table_palette.setColor(QPalette::Base, Qt::transparent);
    table_palette.setColor(QPalette::Text, Qt::white);
    table_palette.setColor(QPalette::Highlight, Qt::transparent);
    table_palette.setColor(QPalette::HighlightedText, Qt::white);

    db = new dbmanager(dbmanager::show, this);
    db->connectToDB();

    sql_model = new QSqlTableModel(this, db->database());
    sql_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    proxy_model = new ProxyModel(ProxyModel::smart, this);
    proxy_model->setSourceModel(sql_model);
    proxy_model->setAlignment(1, Qt::AlignLeft);
    proxy_model->setAlignment(2, Qt::AlignRight);

    table = new QTableView(this);
    table->setModel(proxy_model);
    tableRule();
    QPalette header_palette = table->horizontalHeader()->palette();
    header_palette.setBrush(QPalette::Button, Qt::blue);
    header_palette.setColor(QPalette::Background, Qt::transparent);
    header_palette.setColor(QPalette::ButtonText, Qt::red);
    table->horizontalHeader()->setPalette(header_palette);
    table->setPalette(table_palette);



   // header_palette.setColor(QPalette::Button, Qt::transparent);

    connect(table,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(addToPlaylist()));
    auto *lo_search = new QHBoxLayout;

    auto *label_search = new QLabel(tr("SEARCH "), this);
    le_search = new CLineEdit(this);

    connect(le_search,SIGNAL(focussed(bool)),this,SLOT(showKeyboard(bool)));
    connect(le_search,SIGNAL(textChanged(QString)),proxy_model,SLOT(search(QString)));

    auto *button_backspace = new AeroButton(QIcon(":/usr/share/elroke/file/icon/backspace.png"), this);
    button_backspace->setFocusPolicy(Qt::NoFocus);
    connect(button_backspace,&QPushButton::pressed,le_search,&QLineEdit::backspace);

    auto *button_clear = new AeroButton(QIcon(":/usr/share/elroke/file/icon/clear.png"), this);
    button_clear->setFocusPolicy(Qt::NoFocus);
    connect(button_clear,&QPushButton::pressed,le_search,&QLineEdit::clear);

    lo_search->addStretch();
    lo_search->setSpacing(0);
    lo_search->addWidget(label_search);
    lo_search->addWidget(le_search);
    lo_search->addWidget(button_backspace);
    lo_search->addWidget(button_clear);
    lo_search->addStretch();


    lo_left->addWidget(table);
    lo_left->addLayout(lo_search);
    lo_left->setMargin(0);

    auto *w_table_left = new QWidget(this);//THIS WIDGET ENCAPSULATE TABLE,LINEEDIT, AND BUTTOn to apply sizepolicy
    w_table_left->setLayout(lo_left);

    QSizePolicy spLeft(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spLeft.setHorizontalStretch(3);
    w_table_left->setSizePolicy(spLeft);

    auto *lo_playlist = new QVBoxLayout;
    model_playlist =new QStandardItemModel(this);
    model_playlist->setColumnCount(5);
    model_playlist->setHeaderData(2,Qt::Horizontal, tr("PLAYLIST"));


    table_playlist = new QTableView(this);
    table_playlist->setModel(model_playlist);
    table_playlist->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
    table_playlist->hideColumn(0);
    table_playlist->hideColumn(1);
    table_playlist->hideColumn(3);
    table_playlist->hideColumn(4);
    table_playlist->verticalHeader()->hide();
    table_playlist->setShowGrid(0);
    table_playlist->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_playlist->setSelectionMode(QAbstractItemView::SingleSelection);
    table_playlist->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_playlist->horizontalHeader()->setHighlightSections(0);
    table_playlist->horizontalHeader()->setPalette(header_palette);
    table_playlist->setItemDelegate(new NoFocusDelegate());
    table_playlist->setPalette(table_palette);
    table_playlist->installEventFilter(this);
    connect(table_playlist,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(playPlayer()));

    auto *lo_button_playlist = new QHBoxLayout;

    auto *button_menu = new AeroButton(QIcon(":/usr/share/elroke/file/icon/menu.png"),this);
//    button_menu->setIcon();

    auto *menu_playlist = new QMenu(this);
    autosave_playlist = new QAction(tr("Auto save playlist"), this);
    autosave_playlist->setFont(font());
    autosave_playlist->setCheckable(1);
    autosave_playlist->setChecked(1);
    //load playlist
    if(autosave_playlist->isChecked())
        loadPlaylist();

    auto*save_as = new QAction(tr("Save as"),  this);
    save_as->setFont(font());
    connect(save_as, SIGNAL(triggered(bool)), this, SLOT(dialogSavePlaylist()));
    auto *load_playlist = new QAction(tr("Load playlist") , this);
    load_playlist->setFont(font());
    connect(load_playlist, SIGNAL(triggered(bool)),this,SLOT(dialogLoadPlaylist()));

    menu_playlist->addAction(autosave_playlist);
    menu_playlist->addAction(save_as);
    menu_playlist->addAction(load_playlist);

    button_menu->setMenu(menu_playlist);

    auto *button_delete = new AeroButton(QIcon(":/usr/share/elroke/file/icon/delete.png"), this);
    button_delete->setFocusPolicy(Qt::NoFocus);
    connect(button_delete,SIGNAL(pressed()),this,SLOT(deleteItemPlaylist()));

    auto *button_clear_playlist = new AeroButton(QIcon(":/usr/share/elroke/file/icon/clear.png"), this);
    button_clear_playlist->setFocusPolicy(Qt::NoFocus);
    connect(button_clear_playlist,SIGNAL(pressed()),this,SLOT(clearPlaylist()));

    auto *button_up = new AeroButton(QIcon(":/usr/share/elroke/file/icon/up.png"), this);
    button_up->setFocusPolicy(Qt::NoFocus);
    connect(button_up,SIGNAL(pressed()),this,SLOT(moveItemUp()));

    auto *button_down = new AeroButton(QIcon(":/usr/share/elroke/file/icon/down.png"), this);
    button_down->setFocusPolicy(Qt::NoFocus);
    connect(button_down,SIGNAL(pressed()),this,SLOT(moveItemDown()));

    button_lock_playlist = new AeroButton(QIcon(":/usr/share/elroke/file/icon/lock.png"), this);
    button_lock_playlist->setFocusPolicy(Qt::NoFocus);
    button_lock_playlist->setCheckable(true);
    connect(button_lock_playlist,&QPushButton::clicked,this,&mainWindow::updateLockButton);
//    button_lock_playlist->setCentang(1);

    lo_button_playlist->addWidget(button_menu);
    lo_button_playlist->addWidget(button_delete);
    lo_button_playlist->addWidget(button_clear_playlist);
    lo_button_playlist->addWidget(button_up);
    lo_button_playlist->addWidget(button_down);
    lo_button_playlist->addWidget(button_lock_playlist);
    lo_button_playlist->setSpacing(0);
    lo_playlist->addWidget(table_playlist);
    lo_playlist->addLayout(lo_button_playlist);
    lo_playlist->setMargin(0);

    //CREATE WIDGET TO APPLY SIZEPOLICY
    auto *w_table_right = new QWidget(this);
    w_table_right->setLayout(lo_playlist);

    QSizePolicy spRight(QSizePolicy::Preferred,QSizePolicy::Preferred);
    spRight.setHorizontalStretch(1);
    w_table_right->setSizePolicy(spRight);

    lo_table->addWidget(w_table_left);
    lo_table->addWidget(w_table_right);
    lo_table->setMargin(0);
    lo_table->setSpacing(0);


    //CREATE WIDGET TO APPLY SIZEPOLICY
    auto *all_table = new QWidget(this);
    all_table->setLayout(lo_table);

    QSizePolicy spTable (QSizePolicy::Expanding,QSizePolicy::Expanding);
    spTable.setVerticalStretch(2);
    all_table->setSizePolicy(spTable);




     auto *lo_player_control = new QHBoxLayout;

     slider_pos = new QSlider(Qt::Horizontal, this);
     slider_pos->setRange(0,10000);
     slider_pos->setFixedWidth(200);
     slider_pos->setFocusPolicy(Qt::NoFocus);
     slider_pos->setFixedHeight(40);

     auto *btn_next = new QPushButton(this);
     btn_next->setIcon(QIcon(":/usr/share/elroke/file/icon/next.png"));
     btn_next->setFlat(1);
     btn_next->setIconSize(QSize(64,64));
     btn_next->setFocusPolicy(Qt::NoFocus);
     connect(btn_next,SIGNAL(pressed()),this,SLOT(playPlayer()));

//     QPushButton *button_stop = new QPushButton(QIcon::fromTheme("stop"),"", this);
//     button_stop->setFlat(1);
//     button_stop->setFocusPolicy(Qt::NoFocus);
//     button_stop->setPalette(btn_pal);
//     connect(button_stop, SIGNAL(pressed()),video,SLOT(stop()));

     auto *button_play_pause = new QPushButton(QIcon(":/usr/share/elroke/file/icon/play.png"), "", this);
     button_play_pause->setFlat(1);
     button_play_pause->setIconSize(QSize(64,64));
     button_play_pause->setFocusPolicy(Qt::NoFocus);
     //connect(button_play_pause, SIGNAL(pressed()),this,SLOT(playPausePlayer()));

    button_audio_channel = new QPushButton(this);
    button_audio_channel->setIcon(QIcon(":/usr/share/elroke/file/icon/stereo.png"));
    button_audio_channel->setIconSize(QSize(64,64));
    button_audio_channel->setFlat(1);
    button_audio_channel->setFocusPolicy(Qt::NoFocus);
    connect(button_audio_channel,SIGNAL(pressed()),this,SLOT(setAudioChannelManual()));

     QPushButton *button_vol_down = new QPushButton(QIcon(":/usr/share/elroke/file/icon/minus.png"),"", this);
     button_vol_down->setFocusPolicy(Qt::NoFocus);
     button_vol_down->setFlat(1);
     button_vol_down->setIconSize(QSize(24,24));
     connect(button_vol_down, SIGNAL(pressed()),this,SLOT(setVolStepDown()));

     slider_vol = new QSlider(Qt::Horizontal, this);
     slider_vol->setRange(0, 100);
     slider_vol->setFixedWidth(200);
     slider_vol->setFocusPolicy(Qt::NoFocus);


     QPushButton *button_vol_up = new QPushButton(QIcon(":/usr/share/elroke/file/icon/plus.png"),"", this);
     button_vol_up->setFocusPolicy(Qt::NoFocus);
     button_vol_up->setIconSize(QSize(24,24));
     button_vol_up->setFlat(1);
     connect(button_vol_up,SIGNAL(pressed()), this, SLOT(setVolStepUp()));

     button_audio_mute = new QPushButton(QIcon(":/usr/share/elroke/file/icon/mute.png"),"", this);
     button_audio_mute->setFocusPolicy(Qt::NoFocus);
     button_audio_mute->setIconSize(QSize(64,64));
     button_audio_mute->setFlat(1);
     connect(button_audio_mute,SIGNAL(pressed()),this,SLOT(setMute()));

     lo_player_control->addWidget( slider_pos);
     lo_player_control->addStretch();
     lo_player_control->addWidget(btn_next);
     lo_player_control->addWidget(button_play_pause);
     lo_player_control->addStretch();
     lo_player_control->addWidget(button_audio_channel);
     lo_player_control->addStretch();
     lo_player_control->addWidget(button_vol_down);
     lo_player_control->addWidget(slider_vol);
     lo_player_control->addWidget(button_vol_up);
     lo_player_control->addStretch();
     lo_player_control->addWidget(button_audio_mute);
     lo_player_control->setMargin(0);
     QWidget *widget_bottom = new QWidget(this);

//     QPixmap bg_trans(":/usr/share/elroke/file/background/backgroundTrans.png");
//     QPalette palette_bg;

//     bg_trans =   bg.scaled(widget_bottom->size(),Qt::IgnoreAspectRatio,Qt::FastTransformation);
//     palette_bg.setBrush(QPalette::Background, bg_trans);


     widget_bottom->setAutoFillBackground(1);
     widget_bottom->setPalette(palette_bg);
     widget_bottom->setLayout(lo_player_control);




    lo_main->addWidget(widget_top);
    lo_main->addWidget(all_table);

    //spacer willbe mask when video playing, keep lyric vicible
    spacer = new QWidget(this);
    spacer->adjustSize();
    QSizePolicy spSpacer (QSizePolicy::Expanding,QSizePolicy::Expanding);
    spSpacer.setVerticalStretch(1);


    spacer->setSizePolicy(spSpacer);
    lo_main->addWidget(spacer);

//    lo_main->addLayout(lo_player_control);
    lo_main->addWidget(widget_bottom);
    lo_main->setMargin(0);
    setLayout(lo_main);


}

void mainWindow::createShortcut(){

    QShortcut *sc_quit =new QShortcut(QKeySequence("Esc"),this);
    connect(sc_quit,SIGNAL(activated()),qApp,SLOT(quit()));

}
void mainWindow::getCategory(){

//QList <QString> s = listStringFileParser::parse(QDir::homePath()+"/.elroke/meta/category");


}


void mainWindow::setVolStepDown(){

    if(video->volume()<5)
         video->setVolume(0);
    else
    video->setVolume(video->volume()-5);

}

void mainWindow::setVolStepUp(){

    if(video->volume()>95)
        video->setVolume(100);
    else
     video->setVolume(video->volume()+5);
}

void mainWindow::setMute(){

    if(!video->isPlaying())
        return;

    if(video->isMute()){
        video->setMute(0);
        button_audio_mute->setIcon(QIcon(":/usr/share/elroke/file/icon/mute.png"));
    }

    else{
        video->setMute(1);
         button_audio_mute->setIcon(QIcon(":/usr/share/elroke/file/icon/unmute.png"));
    }
}
void mainWindow::showClock(){

     clock->setText(QTime::currentTime().toString("hh:mm"));
}

void mainWindow::addToPlaylist(){

  //  int rowCount = model_playlist->rowCount();
    model_playlist->setRowCount(model_playlist->rowCount()+1);
    int row =  proxy_model->mapToSource( table->selectionModel()->currentIndex()).row();
    int id = sql_model->data(sql_model->index(row,0),Qt::DisplayRole).toInt();
    QString path =  sql_model->data(sql_model->index(row,7),Qt::DisplayRole).toString();
    QString title = sql_model->data(sql_model->index(row,1),Qt::DisplayRole).toString();
    QString singer = sql_model->data(sql_model->index(row,2),Qt::DisplayRole).toString();
    QString channel = sql_model->data(sql_model->index(row,5),Qt::DisplayRole).toString();

    QStandardItem *item_id = new QStandardItem;
    item_id->setText(QString::number(id));
    model_playlist->setItem(model_playlist->rowCount()-1,0,item_id);

    QStandardItem *item_path = new QStandardItem;
    item_path ->setText( path);
    model_playlist->setItem(model_playlist->rowCount()-1,1,item_path);

    QStandardItem *item_title = new QStandardItem;
    item_title->setText(title);
    item_title->setTextAlignment(Qt::AlignCenter);
    model_playlist->setItem(model_playlist->rowCount()-1,2,item_title);

    QStandardItem *item_singer = new QStandardItem;
    item_singer->setText(singer);
    model_playlist->setItem(model_playlist->rowCount()-1,3,item_singer);



    QStandardItem *item_channel = new QStandardItem;
    item_channel->setText(channel);
    model_playlist->setItem(model_playlist->rowCount()-1,4,item_channel);


    //set curretn item at 0
    table_playlist->selectRow(0);

}

void mainWindow::keyPressEvent(QKeyEvent *event){

  //IF USER TYPE USER TYPE NUMBER OR ALPHABET DIRECT TO LINEEDIT SEARCH
    if(isKeyValid(event->key())){
        le_search->setFocus();
        le_search->setText(event->text());
    }

        if(event->key()==Qt::Key_Enter || event->key()==Qt::Key_Return){
            //send item to playlist with enter key
            if(focusWidget()==table){
                addToPlaylist();
            }
            //play item with enter key
            else if(focusWidget()==table_playlist){
                playPlayer();
            }
        }



        //move focus widget from table to playlist with right key
        else if(event->key()==Qt::Key_Right && qApp->focusWidget()==table){

            table_playlist->setFocus();
        }

        //move focus widget from playlist to table with left key
        else if(event->key()==Qt::Key_Left && qApp->focusWidget()==table_playlist){
            table->setFocus();

        }




        if(event->key()==Qt::Key_Backspace){
            if(le_search->text().isEmpty())
                return;
            else{
                le_search->setFocus();
                le_search->backspace();
            }
        }
}

void mainWindow::dialogAbout(){

    about About;
    About.exec();

}

void mainWindow::updateInterface(){

    slider_pos->setSliderPosition(video->position());
    slider_vol->setSliderPosition(video->volume());

}


void mainWindow::moveItemUp(){
//move up item in playlist
    QModelIndex selection = table_playlist->selectionModel()->currentIndex();

    //ignore if empty
  if(model_playlist->rowCount()==0)
      return;

    int row = selection.row();
//ignore first item
    if(row==0)
        return;

    QList<QStandardItem *> item_list = model_playlist->takeRow(row);
    model_playlist->insertRow(row-1, item_list);

    //set new current row
    QModelIndex new_index = table_playlist->model()->index(row-1,0);
    table_playlist->setCurrentIndex(new_index);
}

void mainWindow::moveItemDown(){
//move down item in playlist
    QModelIndex selection = table_playlist->selectionModel()->currentIndex();

    //ignore if empty
    if(model_playlist->rowCount()==0)
        return;

    int  row = selection.row();
//ignore last item
    if(row==model_playlist->rowCount()-1)
        return;

    QList<QStandardItem *> item_list = model_playlist->takeRow(row);//take row

    model_playlist->insertRow(row+1, item_list);//insert row

    //set new current row
     QModelIndex new_index = table_playlist->model()->index(row+1,0);
     table_playlist->setCurrentIndex(new_index);

}

void mainWindow::setBackground(){

        QPixmap bg(background);
        bg=bg.scaled(desktop->size(),Qt::IgnoreAspectRatio,Qt::FastTransformation);

        QPalette pal;
        pal.setColor(QPalette::Window,Qt::black);
        pal.setColor(QPalette::WindowText,Qt::white);
        pal.setBrush(QPalette::Background, bg);
//        pal.setColor(QPalette::Button, Qt::transparent);

        setPalette(pal);
}

void mainWindow::clearPlaylist(){

    model_playlist->setRowCount(0);

}

void mainWindow::deleteItemPlaylist(){

         QModelIndex index = table_playlist->selectionModel()->currentIndex();
        model_playlist->removeRow(index.row());

}

void mainWindow::playPlayer(){


    //IF TABLE EMPTY RETURN
            if(model_playlist->rowCount()==0){

                video->close();
                clearMask();

                 return;
            }
//STOP IF PLAYING
            if(video->isPlaying()){
                video->stop();
            }

            int cur = table_playlist->selectionModel()->currentIndex().row();
int id = model_playlist->item(cur,0)->text().toInt();
            QString file= model_playlist->item(cur,1)->text();
            QString title = model_playlist->item(cur,2)->text();
            QString singer = model_playlist->item(cur,3)->text();
            channel = model_playlist->item(cur,4)->text();


            //CHECK IF FILE EXIST
            if (!QFile(file).exists()){

               QMessageBox message;
               message.setIcon(QMessageBox::Information);
               message.setInformativeText(tr("File is not found, maybe drive is not mounted"));
               message.setWindowFlags(Qt::FramelessWindowHint);
               message.setStandardButtons(QMessageBox::Close);
               message.exec();


              return;
           }

           video->setFile(file);
           video->setMeta(title, singer);

           video->play();
           db->updatePlayedTime(id);
           sql_model->select();
//tableRule();

            if(button_lock_playlist->isChecked()){
                //move to last
                QList<QStandardItem *> item_list = model_playlist->takeRow(cur);
                model_playlist->insertRow(model_playlist->rowCount(), item_list);

            }else{
                       model_playlist->removeRow(cur);
            }
           table_playlist->selectRow(0);
}

void mainWindow::setaudiochannelAuto(){

        //wait while video get fully loaded but this break gui
        this_thread::sleep_for(chrono::milliseconds(100));

        if(channel.contains("LEFT", Qt::CaseInsensitive)){
            video->setAudioChannelLeft();
            button_audio_channel->setIcon(QIcon(":/usr/share/elroke/file/icon/left.png"));

        }
        else if(channel.contains("RIGHT", Qt::CaseInsensitive)){
            video->setAudioChannelRight();
            button_audio_channel->setIcon(QIcon(":/usr/share/elroke/file/icon/right.png"));
        }

}

void mainWindow::errorHandling(){

    QMessageBox *error_message = new QMessageBox(this);
    error_message->setInformativeText("Error playing "+video->getFile());
    error_message->show();

    QTimer::singleShot(3000, error_message,SLOT(deleteLater()));

}

void mainWindow::almostEnd(){
// this will notify next item in playlist before song end
    QDialog *dia = new QDialog;

    QPalette let;
    let.setColor(QPalette::Background,Qt::white);
    let.setColor(QPalette::Foreground,Qt::gray);

    QFont f;
    f.setPointSize(32);

    QLabel *notif = new QLabel(dia);

            if(model_playlist->rowCount()==0)
                 notif->setText("Playlist Is Empty");
          else{

                int  index = table_playlist->selectionModel()->currentIndex().row();

                QString next_song = model_playlist->item(index,2)->text();
                  notif->setText("Next song : "+next_song);
            }


    QHBoxLayout *lo_main = new QHBoxLayout;
    dia->setWindowFlags(Qt::FramelessWindowHint);
//    dia->setAttribute(Qt::WA_NoSystemBackground);
//    dia->setAttribute(Qt::WA_TransparentForMouseEvents);
//    dia->setAttribute(Qt::WA_TranslucentBackground);
    dia->setPalette(let);
    dia->setFont(f);
    lo_main->addWidget(notif);
    dia->setLayout(lo_main);
    dia->adjustSize();
    dia->move((desktop_width/2) - dia->rect().center().x(),0);
    QTimer::singleShot(5000, dia, SLOT(close()));
    dia->exec();

}


bool mainWindow::eventFilter(QObject *target, QEvent *event){

    if(target==video){

    if(event->type()==QEvent::KeyPress ){

            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

            if(isKeyValid(keyEvent->key())){

                this->activateWindow();
                this->setMask(getMaska());
                }

                le_search->setFocus();
                le_search->setText(keyEvent->text().toUpper());


             }

    //if video is playing, user click, mainwindow will be shown
 if(event->type()==QEvent::MouseButtonPress){

     if(!tampak){
         this->activateWindow();
         this->setMask(getMaska());
         tampak=true;
     }
     else{
         video->activateWindow();
         tampak=false;}
      }
}

        return QObject::eventFilter(target, event);
}

void mainWindow::delayHideWindow(){

    qDebug()<<"tred started";

}

void mainWindow::delayEvent(){

    on_delay=false;
    qDebug()<<"delay event"<< on_delay;
    if(on_delay)
        return;
    this->hide();

}

void mainWindow::dialogAddToDatabase(){

    addtodatabase atd;// = new addtodatabase(NULL);

    connect(&atd,SIGNAL(accepted()),this,SLOT(tableRule()));
    connect(&atd,SIGNAL(accepted()),this,SLOT(getCategory()));
    atd.exec();

}

void mainWindow::dialogSavePlaylist(){

    QDialog *dialog_save_playlist = new QDialog(NULL);

    QVBoxLayout *lo_main = new QVBoxLayout;

    QLineEdit *le_playlist_name = new QLineEdit(dialog_save_playlist);
    QAction *action_delete = new QAction(QIcon(":/usr/share/elroke/file/icon/backspace.png"), "", this);
    le_playlist_name->addAction(action_delete, QLineEdit::TrailingPosition);
    connect(action_delete,&QAction::triggered,le_playlist_name,&QLineEdit::backspace);

    lo_main->addWidget(new QLabel("Save Playlist As :", dialog_save_playlist));
    lo_main->addWidget(le_playlist_name);

    QHBoxLayout *lo_btn = new QHBoxLayout;

    QPushButton *btn_close = new QPushButton("Cancel", dialog_save_playlist);
    connect(btn_close,SIGNAL(clicked(bool)),dialog_save_playlist,SLOT(close()));

    QPushButton *btn_save = new QPushButton("Save", dialog_save_playlist);
    connect(btn_save,SIGNAL(clicked(bool)),dialog_save_playlist,SLOT(accept()));
    Keyboard *key = new Keyboard(dialog_save_playlist);
    lo_main->addWidget(key);

    lo_btn->addWidget(btn_close);
    lo_btn->addStretch();
    lo_btn->addWidget(btn_save);

    lo_main->addLayout(lo_btn);
    dialog_save_playlist->setLayout(lo_main);

   dialog_save_playlist->setWindowFlags( Qt::FramelessWindowHint);
   dialog_save_playlist->setAttribute(Qt::WA_DeleteOnClose);

   le_playlist_name->setFocus();

   if(dialog_save_playlist->exec()==QDialog::Accepted){

       writePlaylist(le_playlist_name->text());

   }

   }

void mainWindow::dialogLoadPlaylist(){

    auto *dialog_load_playlist = new QDialog(this);

    QDirIterator it(QDir::homePath()+"/.elroke/playlist", QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);

    QList<QString> list_play;

    QFileInfo info;

    while(it.hasNext()){

        info.setFile(it.next());
        if(info.suffix()=="elp" && info.baseName()!="playlist" )//exception for "playlist", this is for default /auto save
        list_play << info.baseName();

    }

    auto *lo_main = new QVBoxLayout;

    auto *_list = new QListWidget(dialog_load_playlist);

    _list->addItems(list_play);

    lo_main->addWidget(new QLabel("Load Playlist :", dialog_load_playlist));

    auto *lo_btn = new QHBoxLayout;

    auto *btn_cancel = new QPushButton("Cancel", dialog_load_playlist);
    connect(btn_cancel,SIGNAL(clicked(bool)),dialog_load_playlist,SLOT(close()));

    auto *btn_load = new QPushButton("Load", dialog_load_playlist);
    connect(btn_load,SIGNAL(clicked(bool)),dialog_load_playlist,SLOT(accept()));

    lo_btn->addWidget(btn_cancel);
    lo_btn->addStretch();
    lo_btn->addWidget(btn_load);

    lo_main->addWidget(_list);
    lo_main->addLayout(lo_btn);

    dialog_load_playlist->setLayout(lo_main);

    dialog_load_playlist->setWindowFlags(Qt::FramelessWindowHint);
    dialog_load_playlist->setAttribute(Qt::WA_TintedBackground);

    dialog_load_playlist->setMinimumSize(300,200);


    if(dialog_load_playlist->exec()==QDialog::Accepted){

        loadPlaylist(_list->currentItem()->text());
    }
}
void mainWindow::writePlaylist(){

    writePlaylist("playlist");

}

void mainWindow::writePlaylist(const QString &playlistname){

    if(playlistname==NULL)
        return;

    QString dir_playlist = QDir::homePath()+"/.elroke/playlist";

    if(!QDir(dir_playlist).exists())
        QDir().mkpath(dir_playlist);

    QFile file(dir_playlist+"/"+playlistname+".elp");

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        qDebug()<<"cant open playlist file";
    }
     QTextStream stream(&file);

     stream << "[elroke playlist]\n";

     for(int i=0; i<model_playlist->rowCount(); i++){
            for (int x=0; x<5; x++){
                stream<<model_playlist->item(i,x)->text();
                if(x<=3)
                    stream<<'\t';

            }
            stream<<'\n';
     }

     file.close();

}

void mainWindow::loadPlaylist(){
//default saved playlist
    loadPlaylist("playlist");

}

void mainWindow::loadPlaylist(const QString &s){

    model_playlist->setRowCount(0);


    QFile file(QDir::homePath()+"/.elroke/playlist/"+s+".elp");

    if(!file.exists())
        return;

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug() << "cant read playlist" << s;

    QTextStream stream(&file);

    QString stuff= stream.readLine();
    //validate
    if(stuff!="[elroke playlist]")
        return;


    stuff= stream.readLine();//move to next line
    int line_number=0;

    QStandardItem *item_id, *item_path, *item_title, *item_singer, *item_channel;

    while(stuff!=NULL){
        QString id=stuff.split('\t').at(0);
        QString path=stuff.split('\t').at(1);
        QString title=stuff.split('\t').at(2);
        QString singer=stuff.split('\t').at(3);
        QString channel=stuff.split('\t').at(4);
        model_playlist->setRowCount(line_number+1);

        item_id =  new QStandardItem;
        item_id->setText(path);
        model_playlist->setItem(line_number,0, item_id);

        item_path =  new QStandardItem;
        item_path->setText(path);
        model_playlist->setItem(line_number,1, item_path);

        item_title = new QStandardItem;
        item_title->setText(title);
        item_title->setTextAlignment(Qt::AlignCenter);
        model_playlist->setItem(line_number,2,item_title);

        item_singer = new QStandardItem;
        item_singer->setText(singer);
        model_playlist->setItem(line_number,3,item_singer);

        item_channel = new QStandardItem;
        item_channel->setText(channel);
        model_playlist->setItem(line_number,4, item_channel);

         line_number++;
         stuff= stream.readLine();
    }

    table_playlist->selectRow(0);

}


QRegion mainWindow::getMaska(){

    int   free_top_point = spacer->mapToGlobal(spacer->rect().topLeft()).y();
    int   free_bottom_point = spacer->mapToGlobal( spacer->rect().bottomLeft()).y();
    QRegion r(0,free_top_point,desktop_width,free_bottom_point-free_top_point, QRegion::Rectangle);
    QRegion desk (0,0, desktop_width, desktop_height, QRegion::Rectangle);
    return desk.subtracted(r);

}


void mainWindow::dialogEditDatabase(){

    //sql_model must be inActive before editing
    sql_model->clear();

    managedb md;
    connect(&md, SIGNAL(finished(int)),this,SLOT(tableRule()));
    md.exec();

}


void mainWindow::setAudioChannelManual(){

    switch(video->getAudioChannel()){
    case 1://stereo
        video->setAudioChannelRight();
        button_audio_channel->setIcon(QIcon(":/usr/share/elroke/file/icon/right.png"));
        break;
    case 3://left
        video->setAudioChannelStereo();
        button_audio_channel->setIcon(QIcon(":/usr/share/elroke/file/icon/stereo.png"));
        break;
    case 4://right
        video->setAudioChannelLeft();
        button_audio_channel->setIcon(QIcon(":/usr/share/elroke/file/icon/left.png"));
        break;

    }

}


void mainWindow::tableRule(){

    //rebuild rule after sql_model->clear();
    //is there better way to update table?
    sql_model->setTable("ELROKE123");
    sql_model->setSort(1,Qt::AscendingOrder);
    sql_model->select();
    table->verticalHeader()->hide();
    table->setShowGrid(0);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->hideColumn(0);
    table->hideColumn(3);
    table->hideColumn(4);
    table->hideColumn(5);
    table->hideColumn(6);
    table->hideColumn(7);
    table->hideColumn(8);
    table->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
    table->model()->setHeaderData(1, Qt::Horizontal,Qt::AlignLeft, Qt::TextAlignmentRole);
    table->model()->setHeaderData(2, Qt::Horizontal,Qt::AlignRight, Qt::TextAlignmentRole);
    table->horizontalHeader()->setHighlightSections(0);
    table->setItemDelegate(new NoFocusDelegate());

}

bool mainWindow::isKeyValid(int key){

    QList <int> validKey;
    //this enum key 0-9a-zspace
    validKey<<0x30<<0x31<<0x32<<0x33<<0x34<<0x35<<0x36<<0x37<<0x38<<0x39<<0x41<<0x42<<0x43
                   <<0x44<<0x45<<0x46<<0x47<<0x48<<0x49<<0x4a<<0x4b<<0x4c<<0x4d<<0x4e<<0x4f<<0x50
                   <<0x51<<0x52<<0x53<<0x54<<0x55<<0x56<<0x57<<0x58<<0x59<<0x5a<<0x20;

    if(validKey.contains(key))
        return true;

    return false;


}

void mainWindow::showKeyboard(bool x){


    if(x){
        int x =(table->width()-keyboard->width())/2;
        int y = table->mapToGlobal(table->rect().bottomLeft()).y()-keyboard->height();
        //key show on the midle of table
         keyboard->showKeyboard(QPoint(x,y));
    }
      else
            keyboard->hide();

}

void mainWindow::videoInstance(){
    //video player
    video = new Player;
    video->setMinimumWidth(500);
    video->setMinimumHeight(250);
    video->setCursor(Qt::ArrowCursor);
    video->setMouseTracking(1);
    video->installEventFilter(this);

    connect(video, SIGNAL(positionChanged()),this,SLOT(updateInterface()));
    connect(video,SIGNAL(reachEnded()),this,SLOT(playPlayer()));
    connect(video,SIGNAL(almostEnded()),this,SLOT(almostEnd()));
    connect(video,SIGNAL(playing()),this,SLOT(setaudiochannelAuto()));
    connect(video,SIGNAL(error()),this,SLOT(errorHandling()));
    connect(slider_pos,SIGNAL(sliderMoved(int)),video,SLOT(changePosition(int)));
    connect(slider_vol,&QSlider::sliderMoved,video,&Player::setVolume);
}

void mainWindow::keyBoardInstance(){
    keyboard = new Keyboard(this);
    keyboard->hide();
}

void mainWindow::fontSetup(){

    //set font
    QFontDatabase basefont;
    int fontindex = basefont.addApplicationFont(":/usr/share/elroke/file/font/Xolonium.ttf");

    if(fontindex==-1)
        qDebug()<<"Failed to load custom font";

    QFont font= basefont.font("Xolonium", "normal", 16);
    setFont(font);


}

void mainWindow::updateLockButton(bool lock){

    if(lock)
        button_lock_playlist->setIcon(QIcon(":/usr/share/elroke/file/icon/unlock.png"));
    else
         button_lock_playlist->setIcon(QIcon(":/usr/share/elroke/file/icon/lock.png"));
}

void mainWindow::dialogAdmin(){

    QDialog dialog_admin;

    QVBoxLayout *lo_main = new QVBoxLayout;

    auto *button_add_to_database = new QPushButton(tr("ADD TO DATABASE"), &dialog_admin);
    connect(button_add_to_database,SIGNAL(pressed()),this,SLOT(dialogAddToDatabase()));

    auto *button_manage_database = new QPushButton(tr("MANAGE DATABASE"), &dialog_admin);
    connect(button_manage_database,SIGNAL(pressed()),this,SLOT(dialogEditDatabase()));

    auto *button_preferences = new QPushButton(tr("PREFERENCES"), &dialog_admin);

    auto *button_about = new QPushButton(tr("ABOUT"), &dialog_admin);
    connect(button_about,SIGNAL(pressed()),this,SLOT(dialogAbout()));

    auto *button_close = new QPushButton(tr("CLOSE"), &dialog_admin);
    connect(button_close,SIGNAL(pressed()),&dialog_admin,SLOT(close()));

    auto *button_exit = new QPushButton(tr("QUIT APP"), &dialog_admin);
    connect(button_exit,SIGNAL(pressed()),&dialog_admin,SLOT(close()));
    connect(button_exit,SIGNAL(pressed()),this,SLOT(close()));

    lo_main->addWidget(button_add_to_database);
    lo_main->addWidget(button_manage_database);
    lo_main->addWidget(button_preferences);
    lo_main->addWidget(button_about);
    lo_main->addWidget(button_close);
    lo_main->addWidget(button_exit);

    dialog_admin.setLayout(lo_main);
    dialog_admin.setWindowFlags(Qt::FramelessWindowHint);
    dialog_admin.exec();

}

void mainWindow::checkAdmin(){

//    dialogCreateAdmin();
    QSettings settings("elroke","elroke");
    QString user;
    settings.beginGroup(group_auth);
    user=settings.value("name").toString();
    settings.endGroup();

    qDebug()<<"yser"<<user;

    if(user==NULL)
        dialogCreateAdmin();
    else
        dialogLogin();

}

void mainWindow::dialogCreateAdmin(){

    QDialog dialog;
    QVBoxLayout *lo_main = new QVBoxLayout;
    QHBoxLayout *lo_username = new QHBoxLayout;
    QHBoxLayout *lo_password = new QHBoxLayout;

    le_userName = new QLineEdit(&dialog);


    lo_username->addWidget(new QLabel(tr("Username"),&dialog));
    lo_username->addWidget(le_userName);

    le_password = new QLineEdit(&dialog);
    le_password->setEchoMode(QLineEdit::Password);
    lo_password->addWidget(new QLabel(tr("Password"),&dialog));
    lo_password->addWidget(le_password);

    auto *button_create_admin = new QPushButton(tr("Create Administrator"), &dialog);
    connect(button_create_admin,SIGNAL(pressed()),this,SLOT(createAdminAccount()));

    lo_main->addWidget(new QLabel(tr("First setup,  create admin account"), &dialog));
    lo_main->addLayout(lo_username);
    lo_main->addLayout(lo_password);
    lo_main->addWidget(button_create_admin);

     dialog.setLayout(lo_main);
     connect(this,SIGNAL(usernameCreated()),&dialog,SLOT(close()));
     dialog.setWindowFlags(Qt::FramelessWindowHint);
     dialog.exec();



}

void mainWindow::dialogLogin(){

    QDialog dialog;
    QVBoxLayout *lo_main = new QVBoxLayout;
    QHBoxLayout *lo_username = new QHBoxLayout;
    QHBoxLayout *lo_password = new QHBoxLayout;

    le_userName = new QLineEdit(&dialog);


    lo_username->addWidget(new QLabel(tr("Username"),&dialog));
    lo_username->addWidget(le_userName);

    le_password = new QLineEdit(&dialog);
    le_password->setEchoMode(QLineEdit::Password);
    lo_password->addWidget(new QLabel(tr("Password"),&dialog));
    lo_password->addWidget(le_password);

    auto *button_close = new QPushButton(tr("Close"), &dialog);
    connect(button_close,SIGNAL(pressed()),&dialog,SLOT(close()));
    auto *button_login = new QPushButton(tr("Login"), &dialog);
    button_login->setFocus();
    connect(button_login,SIGNAL(pressed()),this,SLOT(login()));

    QHBoxLayout *lo_button = new QHBoxLayout;
    lo_button->addWidget(button_close);
    lo_button->addWidget(button_login);
    lo_main->addLayout(lo_username);
    lo_main->addLayout(lo_password);
    lo_main->addLayout(lo_button);

     dialog.setLayout(lo_main);
     dialog.setWindowFlags(Qt::FramelessWindowHint);
     connect(this,SIGNAL(loginAccepted()),&dialog,SLOT(close()));
     dialog.exec();



}

void mainWindow::createAdminAccount(){

QSettings setting("elroke","elroke");

QString user = le_userName->text();
QString pass = le_password->text();

if(user.isEmpty() && pass.isEmpty())
    return;

setting.beginGroup(group_auth);
setting.setValue("name", user);
setting.setValue("sandi", pass);
setting.endGroup();

emit usernameCreated();


}

void mainWindow::login(){

    QSettings settings("elroke","elroke");
    QString user;
    QString password;
    settings.beginGroup(group_auth);
    user=settings.value("name").toString();
    password=settings.value("sandi").toString();
    settings.endGroup();

    if(le_userName->text()==user && le_password->text()==password){
     emit loginAccepted();
    dialogAdmin();
    }
    else return;

}
mainWindow::~mainWindow()
{
    delete video;
    if(autosave_playlist->isChecked())
        writePlaylist();

}
