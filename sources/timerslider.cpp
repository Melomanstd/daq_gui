#include "headers/timerslider.h"

TimerSlider::TimerSlider(Qt::Orientation orientation, QWidget *parent)
    :   QSlider(orientation, parent)
{
    _timer = new QTimer(this);
    _timer->setInterval(500);
    connect(_timer, SIGNAL(timeout()), this,
            SLOT(TimerTimeout()));
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(ResetTimerTimeout()));

    _buddy = 0;
}

TimerSlider::~TimerSlider()
{
    disconnect(_timer, SIGNAL(timeout()), this,
            SLOT(TimerTimeout()));
    _timer->stop();
    delete _timer;
}

void TimerSlider::SetTimeout(quint32 timeout)
{
    _timer->start(timeout);
}

void TimerSlider::TimerTimeout()
{
    _timer->stop();
    emit NewValue(this->value());
}

void TimerSlider::ResetTimerTimeout()
{
    if (_buddy != 0)
    {
        if ((_buddy->value() + value()) > 1024)
        {
            _buddy->setValue(maximum()-value());
        }
    }
    _timer->start();
}

void TimerSlider::SetBuddy(TimerSlider *slider)
{
    _buddy = slider;
}
