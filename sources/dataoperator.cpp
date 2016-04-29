#include "headers/dataoperator.h"

#include <QDebug>

DataOperator::DataOperator(QObject *parent)
    :   QThread(parent),
        _isWorking(false),
        _isUnitialize(false),
        _errorCode(0),
        _lastError(tr("No error")),
        _samplesBlockBuffer(0),
        _sampleSingleshotValue(0),
        _voltageBlockBuffer(0),
        _voltageSingleshotValue(0)
{

}

DataOperator::~DataOperator()
{
    if (_isUnitialize == false)
    {
        ::D2K_Release_Card(0);
    }
}

void DataOperator::run()
{
    _errorCode = ::D2K_Register_Card(DAQ_2213, 0);
    if (_errorCode != NoError)
    {
        _isWorking = false;
        _lastError = tr("Cant initialize device");
        qDebug() << "Cant initialize device";
    }

    while (_isWorking)
    {
        //
    }

    ::D2K_Release_Card(0);
    _isUnitialize = true;
}

void DataOperator::stopWorking()
{
    _mutex.tryLock();
    _isWorking = false;
    _mutex.unlock();
}
