#ifndef SCORING_H
#define SCORING_H

#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <player.h>

class Scoring : public QDialog
{
    Q_OBJECT
public:
    explicit Scoring(QWidget *parent = nullptr);

signals:
    void finished();
public slots:
    void start();
    void finish();

private :
    QLabel *label;
    QTimer *timer_random;
    QTimer *timer;
    Player *player;
public slots:
};

#endif // SCORING_H
