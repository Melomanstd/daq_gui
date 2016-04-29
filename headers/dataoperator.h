#ifndef DATAOPERATOR_H
#define DATAOPERATOR_H

#include <QThread>
#include <QMutex>

#include <D2kDask.h>
#include <DAQHeader.h>

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
    bool    _isUnitialize;

    QMutex  _mutex;
    I16     _errorCode;
    QString _lastError;
};

#endif // DATAOPERATOR_H
