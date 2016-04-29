#include "headers/dataoperator.h"

#define CardNumber 0

#include <QDebug>

DataOperator::DataOperator(QObject *parent)
    :   QThread(parent),
        _isWorking(false),
        _isUnitialize(false),
        _isNewParameters(false),
        _newDataReady(false),
        _isDoubleBuffer(0),
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
        _measuringInterval(160),
        _resultBufferId(0),
        _cardID(-1)
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
    _initializeCard();

    while (_isWorking == true)
    {
        _updateParameters();
        if (_isWorking == false)
        {
            break;
        }

        if (_workingMode == MODE_SINGLESHOT_MEASURING)
        {
            ::D2K_AI_VReadChannel(_cardID, 0, &_voltageSingleshotValue);
            msleep(_measuringInterval);
            qDebug() << _voltageSingleshotValue;
            _newDataReady = true;
        }

//        _isWorking = false;
    }

    ::D2K_Release_Card(0);
    _isUnitialize = true;
}

void DataOperator::startWorking()
{
    start();
}

void DataOperator::stopWorking()
{
    _mutex.tryLock();
    _isWorking = false;
    _mutex.unlock();
    wait(3000);
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
    if ((interval < MINIMUM_MEASURING_INTERVAL) ||
            (interval > MAXIMUM_OPTION_VALUE))
    {
        return;
    }

    _mutex.tryLock();
    _measuringInterval = static_cast<U32> (interval);
    _mutex.unlock();
}

void DataOperator::setMeasureSampleInterval(quint32 interval)
{
    if ((interval < MINIMUM_SAMPLES_INTERVAL) ||
            (interval > MAXIMUM_SAMPLES_INTERVAL))
    {
        return;
    }

    _mutex.tryLock();
    _measureSampleInterval = static_cast<U32> (interval);
    _mutex.unlock();
}

void DataOperator::setSampleCount(quint32 count)
{
    if ((count < MINIMUM_SAMPLES_PER_BLOCK) ||
            (count > MAXIMUM_OPTION_VALUE))
    {
        return;
    }

    _mutex.tryLock();
    _measureSampleCount = static_cast<U32> (count);
    _mutex.unlock();
}

bool DataOperator::_initializeBlockMode()
{
    //AI config constants definition
    U16     configCtrl = DAQ2K_AI_ADCONVSRC_Int;
    U32     trigCtrl = DAQ2K_AI_TRGSRC_SOFT |
                        DAQ2K_AI_TRGMOD_POST;
    U16     reTrgCnt = 0;
    BOOLEAN bufAutoReset = 1;
    U32     memSize = 0;

    _errorCode = ::D2K_AI_AsyncDblBufferMode(_cardID, _isDoubleBuffer);
    if (_errorCode != NoError)
    {
        qDebug() << "Cannot set double buufer mode";
        _lastError = tr("Cannot set double buufer mode");
        emit someError();
    }

    _errorCode = ::D2K_AI_InitialMemoryAllocated(_cardID, &memSize);
    if (_errorCode != NoError)
    {
        qDebug() <<"Cannot get current memory size";
        _lastError = tr("Cannot get current memory size");
        return false;
    }
    if (memSize*1024 < _measureSampleCount * sizeof(I16) )
    {
        qDebug() << "Not enough memory to initialize current mode";
        _lastError = tr("Not enough memory to initialize current mode");
        return false;
           //available memory size for analog input in the device driver
           //is smaller than the data size specified!!
           //ToDo : do something here
    }

    _errorCode= ::D2K_AI_Config(_cardID,
                                configCtrl,
                                trigCtrl,
                                0,
                                0,
                                reTrgCnt,
                                bufAutoReset);
    if (_errorCode != NoError) {
        qDebug() <<"Cant config block mode";
        _lastError = tr("Cant config block mode");
        return false;
    }

    _errorCode=D2K_AI_ContBufferSetup (_cardID,
                                       _samplesBlockBuffer,
                                       _measureSampleCount,
                                       &_resultBufferId);
    if (_errorCode != NoError)
    {
        qDebug() <<"Cannot initialize buffer";
        _lastError = tr("Cannot initialize buffer");
        return false;
    }

    return true;
}

void DataOperator::_initializeCard()
{
    _cardID = ::D2K_Register_Card(DAQ_2213, 0);
    if (_cardID < 0)
    {
        _isWorking = false;
        _lastError = tr("Cant initialize device");
        qDebug() << "Cant initialize device";
        emit someError();
    }
    else
    {
        ::D2K_AI_CH_Config(_cardID, 0, AD_B_10_V|AI_RSE);
//        ::D2K_AI_CH_Config(cardID, 1, AD_B_10_V|AI_RSE);
        _isWorking = true;
    }
}

void DataOperator::_updateParameters()
{
    if (_isNewParameters == true)
    {
        _isNewParameters = false;
        if (_workingMode == MODE_BLOCK_MEASURING)
        {
            if (_initializeBlockMode() == false)
            {
                _isWorking = false;
                emit someError();
            }
        }
    }
}

U16 DataOperator::getSamples()
{
    _mutex.tryLock();
    U16 temp = _sampleSingleshotValue;
    _newDataReady = false;
    _mutex.unlock();
    return temp;
}

U16* DataOperator::getSamplesBuffer()
{
    _mutex.tryLock();
    U16 *temp = new U16[_measureSampleCount];
    for (unsigned int i = 0; i < _measureSampleCount; i++)
    {
        temp[i] = _samplesBlockBuffer[i];
    }
    _newDataReady = false;
    _mutex.unlock();
    return temp;
}

F64 DataOperator::getVoltage()
{
    _mutex.tryLock();
    F64 temp = _voltageSingleshotValue;
    _newDataReady = false;
    _mutex.unlock();
    return temp;
}

F64* DataOperator::getVoltageBuffer()
{
    _mutex.tryLock();
    F64 *temp = new F64[_measureSampleCount];
    for (unsigned int i = 0; i < _measureSampleCount; i++)
    {
        temp[i] = _voltageBlockBuffer[i];
    }
    _newDataReady = false;
    _mutex.unlock();
    return temp;
}

bool DataOperator::isDataReady()
{
//    _mutex.tryLock();
    return _newDataReady;
//    _mutex.unlock();
}
