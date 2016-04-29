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
        _voltageSingleshotValue(0),
        _workingMode(0),
        _channelZeroMeasuring(0),
        _channelOneMeasuring(0),
        _measureSampleInterval(8),
        _measureSampleCount(2),
        _measuringInterval(160)
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

void DataOperator::setWorkingMode(qint8 mode)
{
    _mutex.tryLock();
    _workingMode = mode;
    _mutex.unlock();
}

void DataOperator::setChannelStatus(qint8 channel, qint8 state)
{
    _mutex.tryLock();
    if (channel == CHANNEL_0)
    {
        _channelZeroMeasuring = state;
    }
    else if (channel == CHANNEL_1)
    {
        _channelOneMeasuring = state;
    }
    _mutex.unlock();
}

void DataOperator::setMeasuringInterval(quint32 interval)
{
    _mutex.tryLock();
    _measuringInterval = static_cast<U32> (interval);
    _mutex.unlock();
}

void DataOperator::setMeasureSampleInterval(quint32 interval)
{
    _mutex.tryLock();
    _measureSampleInterval = static_cast<U32> (interval);
    _mutex.unlock();
}

void DataOperator::setSampleCount(quint32 count)
{
    _mutex.tryLock();
    _measureSampleCount = static_cast<U32> (count);
    _mutex.unlock();
}
