#include "headers/dataoperator.h"

#define CardNumber 0

#include <QDebug>

DataOperator::DataOperator(QObject *parent)
    :   QThread(parent),
        _isWorking(false),
        _isUnitialize(true),
        _isNewParameters(false),
        _newDataReady(false),
        _isDoubleBuffer(0),
        _errorCode(0),
        _lastError(tr("No error")),
        _samplesBlockBuffer_0(0),
        _sampleSingleshotValue_0(0),
        _voltageBlockBuffer_0(0),
        _voltageSingleshotValue_0(0),
        _samplesBlockBuffer_1(0),
        _sampleSingleshotValue_1(0),
        _voltageBlockBuffer_1(0),
        _voltageSingleshotValue_1(0),
        _workingMode(0),
        _channelZeroMeasuring(0),
        _channelOneMeasuring(0),
        _measuringInterval(0),
        _measureSampleInterval(8),
        _measureSampleCount(2),
        _measuringBlockInterval(160),
        _resultBufferIdZero(0),
        _cardID(-1)
{
    //
}

DataOperator::~DataOperator()
{
    if (_isUnitialize == false)
    {
        ::D2K_Release_Card(_cardID);
        ::D2K_AI_ContBufferReset(_cardID);
    }

    if (_samplesBlockBuffer_0 != 0)
    {
        delete [] _samplesBlockBuffer_0;
    }
    if (_samplesBlockBuffer_1 != 0)
    {
        delete [] _samplesBlockBuffer_1;
    }
}

void DataOperator::run()
{
    _initializeCard();

    if (_workingMode == MODE_BLOCK_MEASURING)
    {
        if (_initializeBlockMode() == false)
        {
            _isWorking = false;
            emit someError();
        }
    }

    while (_isWorking == true)
    {
        _updateParameters();
        if (_isWorking == false)
        {
            break;
        }

        if (_workingMode == MODE_SINGLESHOT_MEASURING)
        {
            _singleshotMeasure();
        }
        else if (_workingMode == MODE_BLOCK_MEASURING)
        {
            _blockMeasure();
        }
//        _isWorking = false;
    }

    ::D2K_AI_ContBufferReset(_cardID);
    ::D2K_Release_Card(_cardID);
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

void DataOperator::setBlockMeasuringInterval(quint32 interval)
{
    if ((interval < MINIMUM_MEASURING_INTERVAL) ||
            (interval > MAXIMUM_OPTION_VALUE))
    {
        return;
    }

    _mutex.tryLock();
    _measuringBlockInterval = static_cast<U32> (interval);
    _measureSampleInterval = static_cast<U32> (interval);
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
    _measuringBlockInterval = static_cast<U32> (interval);
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
    BOOLEAN bufAutoReset = 0;
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
    if (memSize*1024 < _measureSampleCount * 2 * sizeof(I16) )
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
                                       _samplesBlockBuffer_0,
                                       _measureSampleCount,
                                       &_resultBufferIdZero);
    if (_errorCode != NoError)
    {
        qDebug() <<"Cannot initialize buffer for channel 0";
        _lastError = tr("Cannot initialize buffer for channel 0");
        return false;
    }

    _errorCode=D2K_AI_ContBufferSetup (_cardID,
                                       _samplesBlockBuffer_1,
                                       _measureSampleCount,
                                       &_resultBufferIdOne);
    if (_errorCode != NoError)
    {
        qDebug() <<"Cannot initialize buffer for channel 1";
        _lastError = tr("Cannot initialize buffer for channel 1");
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
        ::D2K_AI_CH_Config(_cardID, 1, AD_B_10_V|AI_RSE);
        _isWorking = true;
        _isUnitialize = false;
    }
}

void DataOperator::_updateParameters()
{
    if (_isNewParameters == true)
    {
        _isNewParameters = false;
    }
}

U16 DataOperator::getSamples()
{
    _mutex.tryLock();
    U16 temp = _sampleSingleshotValue_0;
    _newDataReady = false;
    _mutex.unlock();
    return temp;
}

void DataOperator::getSamplesBuffer(double* bufferZero,
                                    double* bufferOne)
{
    _mutex.tryLock();
    if (bufferZero != 0)
    {
        ::D2K_AI_ContVScale(_cardID,
                            AD_B_10_V,
                            _samplesBlockBuffer_0,
                            bufferZero,
                            _measureSampleCount);
    }

    if (bufferOne != 0)
    {
        ::D2K_AI_ContVScale(_cardID,
                            AD_B_10_V,
                            _samplesBlockBuffer_1,
                            bufferOne,
                            _measureSampleCount);
    }
    _newDataReady = false;
    _mutex.unlock();
}

void DataOperator::getVoltage(double &ch0, double &ch1)
{
    _mutex.tryLock();
    ch0 = _voltageSingleshotValue_0;
    ch1 = _voltageSingleshotValue_1;
    _newDataReady = false;
    _mutex.unlock();
//    return temp;
}

F64* DataOperator::getVoltageBuffer()
{
    _mutex.tryLock();
    F64 *temp = new F64[_measureSampleCount];
    for (unsigned int i = 0; i < _measureSampleCount; i++)
    {
        temp[i] = _voltageBlockBuffer_0[i];
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

void DataOperator::setMeasuringInterval(quint32 msec)
{
    _mutex.tryLock();
    _measuringInterval = msec;
    _mutex.unlock();
}

void DataOperator::setParameters(ModeParameters parameters)
{
    _mutex.tryLock();
    if (_samplesBlockBuffer_0 != 0)
    {
        delete [] _samplesBlockBuffer_0;
    }
    if (_samplesBlockBuffer_1 != 0)
    {
        delete [] _samplesBlockBuffer_1;
    }

    _workingMode = parameters.mode;

    _channelZeroMeasuring = parameters.channelZeroState;
    _channelOneMeasuring = parameters.channelOneState;

    _measuringInterval      = parameters.measuringInterval;
//            static_cast<quint32> (1000 / );

    //block mode
    _measureSampleCount     =
            static_cast<U32> (parameters.blockSize);
    _measureSampleInterval  =
            static_cast<U32> (parameters.samplingInterval);
    _measuringBlockInterval =
            static_cast<U32> (parameters.scaningInterval);

    _samplesBlockBuffer_0 = new U16[_measureSampleCount];
    _samplesBlockBuffer_1 = new U16[_measureSampleCount];

    _mutex.unlock();
}

void DataOperator::_singleshotMeasure()
{
    _mutex.tryLock();
    if (_channelZeroMeasuring == true)
    {
        ::D2K_AI_VReadChannel(_cardID, 0, &_voltageSingleshotValue_0);
    }
    if (_channelOneMeasuring == true)
    {
        ::D2K_AI_VReadChannel(_cardID, 1, &_voltageSingleshotValue_1);
    }
    msleep(1000 / _measuringInterval);
    _newDataReady = true;
    _mutex.unlock();
}

void DataOperator::_blockMeasure()
{
    _mutex.tryLock();
    _errorCode = NoError;
    if (_channelZeroMeasuring == true)
    {
        _errorCode = ::D2K_AI_ContReadChannel (_cardID,
                                  0,
                                  _resultBufferIdZero,
                                  _measureSampleCount,
                                  _measuringBlockInterval,
                                  _measureSampleInterval,
                                  SYNCH_OP);

        if (_errorCode != NoError)
        {
            qDebug() << "error";
        }
    }
    if (_channelOneMeasuring == true)
    {
        _errorCode = ::D2K_AI_ContReadChannel (_cardID,
                                  1,
                                  _resultBufferIdOne,
                                  _measureSampleCount,
                                  _measuringBlockInterval,
                                  _measureSampleInterval,
                                  SYNCH_OP);

        if (_errorCode != NoError)
        {
            qDebug() << "error";
        }
    }
    msleep(_measuringInterval);
    _newDataReady = true;
    _mutex.unlock();
}

QString DataOperator::getLastError()
{
    return _lastError;
}
