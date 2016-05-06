#ifndef TIMERSLIDER_H
#define TIMERSLIDER_H

#include <QSlider>
#include <QTimer>

class TimerSlider : public QSlider
{
    Q_OBJECT
public:
            TimerSlider(Qt::Orientation orientation, QWidget * parent = 0 );
    virtual ~TimerSlider();
    void    SetTimeout(quint32 timeout);
    void    SetBuddy(TimerSlider *slider);

private slots:
    void    TimerTimeout();
    void    ResetTimerTimeout();

signals:
    void    NewValue(int value);

private:
    QTimer      *_timer;
    TimerSlider *_buddy;
};

#endif // TIMERSLIDER_H
