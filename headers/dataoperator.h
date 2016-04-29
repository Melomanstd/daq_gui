#ifndef DATAOPERATOR_H
#define DATAOPERATOR_H

#include <QThread>
#include <QMutex>

class DataOperator : public QThread
{
public:
    explicit    DataOperator(QObject *parent = 0);
    virtual     ~DataOperator();

    void        startWorking();
    void        stopWorking();

protected:
    virtual void run();

private:
    bool    _isWorking;
    QMutex  _mutex;
};

#endif // DATAOPERATOR_H
