#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QStackedLayout>
#include <QStandardPaths>
#include <QCheckBox>
#include <QSpinBox>
#include <QThread>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QLabel>
#include <themewidget.h>
#include <QPair>
#include <QPainter>
#include <QListWidget>

class previewer : public QWidget
{
    Q_OBJECT
public :
    previewer (QWidget *parent=nullptr) : QWidget(parent){
        backgroundColor =Qt::white;
    };
public slots:
    void setColor(QColor color){
        backgroundColor =color;
        update();
    };
private :
    QColor backgroundColor;
protected:
    void paintEvent(QPaintEvent *){
        QPainter painter(this);
        QPixmap pix(":/usr/share/elroke/icon/transparentbg.png");
        painter.drawPixmap(rect(),pix);
        painter.setPen(Qt::gray);
        painter.setBrush(backgroundColor);
        painter.drawRect(rect());

    }
};

class resizer : public QObject
{
    Q_OBJECT
public:
 explicit   resizer(QObject *parent=nullptr) :
    QObject(parent){}
private :
     QStringList _filename;
public slots:
     void setInputs(const QStringList &filenames){
         _filename = filenames;
     }

     void start(){
         foreach (const QString &x, _filename) {
             QImage output(x);
             output = output.scaled(QSize(160,90), Qt::KeepAspectRatio, Qt::SmoothTransformation );
             emit finished(output);
         }
     }

signals :
     void error();
     void finished(QImage output);
};

class preferences : public QDialog
{
    Q_OBJECT
public:
    explicit preferences(QWidget *parent = nullptr);
    ~preferences();
private :
    QString selected_font,selected_background;
    unsigned short  int fontSize;
    QCheckBox *check_startapp;
    QString app_dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QStringList bg_list ;
    QStringList favGroup;
    QThread *thread_resizer;
    QStandardItemModel *model ;
    resizer *img_resizer;
    bool startup=false;
//    QString language;
    unsigned int short language_index;
    QListWidget *list_menu_selected;
    QListWidget *list_theme;
    uint newEntriesLimit;
    QSpinBox *monthRange ;
    Theme theme;



private slots:
    QStringList getLanguageGenre();
    void handleImage(QImage);
    void apply();
    void ok();
    void readSetting();

};

#endif // PREFERENCES_H
