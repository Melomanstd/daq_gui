#include "headers/measurethread.h"

#define CardNumber 0

#include <QDebug>

MeasureThread::MeasureThread(QObject *parent)
    :   QThread(parent),
        _isWorking(false),
        _isUnitialize(true),
        _blockDataReady(false),
        _singleshotDataReady(false),
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
        _cardID(-1),
        _measuringBlock(false),
        _measuringSingleshot(false)
{
    for (int i = 0; i < MAXIMUM_CHANNELS; i++)
    {
        _channelsPins[i] = -1;
    }

    _hfBuffer = new U16[MAXIMUM_PLOT_SAMPLES];
}

MeasureThread::~MeasureThread()
{
    if (_isUnitialize == false)
    {
        ::D2K_Release_Card(_cardID);
        ::D2K_AI_ContBufferReset(_cardID);
        _cardID = -1;
    }

    if (_samplesBlockBuffer_0 != 0)
    {
        delete [] _samplesBlockBuffer_0;
    }
    if (_samplesBlockBuffer_1 != 0)
    {
        delete [] _samplesBlockBuffer_1;
    }

    delete [] _hfBuffer;
}

void MeasureThread::run()
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
        if (_isWorking == false)
        {
            break;
        }

        if (_measuringSingleshot == true)
        {
            _singleshotMeasure();
        }

        if (_measuringBlock == true)
        {
            _blockMeasure();
        }
    }

    ::D2K_AI_ContBufferReset(_cardID);
    ::D2K_Release_Card(_cardID);
    _cardID = -1;
    _isUnitialize = true;
}

void MeasureThread::startWorking()
{
    if (isRunning() == false)
    {
        start();
    }
}

void MeasureThread::stopWorking()
{
    if ((_measuringSingleshot == true) || (_measuringBlock == true))
    {
        return;
    }
    _mutex.tryLock();
    _isWorking = false;
    _mutex.unlock();
    wait(3000);
}

void MeasureThread::setWorkingMode(qint8 mode)
{
    _mutex.tryLock();
    _workingMode = mode;
    _mutex.unlock();
}

void MeasureThread::setChannelStatus(qint8 channel, qint8 state)
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

void MeasureThread::setBlockMeasuringInterval(quint32 interval)
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

void MeasureThread::setMeasureSampleInterval(quint32 interval)
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

void MeasureThread::setSampleCount(quint32 count)
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

bool MeasureThread::_initializeBlockMode()
{
    //AI config constants definition
    U16     configCtrl = DAQ2K_AI_ADCONVSRC_Int;
    U32     trigCtrl = DAQ2K_AI_TRGSRC_SOFT |
                        DAQ2K_AI_TRGMOD_POST;
    U16     reTrgCnt = 0;
    BOOLEAN bufAutoReset = 0;
    U32     memSize = 0;

    if (_samplesBlockBuffer_0 != 0)
    {
        delete [] _samplesBlockBuffer_0;
    }
    if (_samplesBlockBuffer_1 != 0)
    {
        delete [] _samplesBlockBuffer_1;
    }

    _samplesBlockBuffer_0 = new U16[_measureSampleCount];
    _samplesBlockBuffer_1 = new U16[_measureSampleCount];

    _errorCode = ::D2K_AI_AsyncDblBufferMode(_cardID, _isDoubleBuffer);
    if (_errorCode != NoError)
    {
        _lastError = tr("Cannot set double buufer mode: ") +
                QString::number(_errorCode);;
        emit someError();
    }

    _errorCode = ::D2K_AI_InitialMemoryAllocated(_cardID, &memSize);
    if (_errorCode != NoError)
    {
        _lastError = tr("Cannot get current memory size: ") +
                QString::number(_errorCode);;
        return false;
    }
    if (memSize*1024 < _measureSampleCount * 2 * sizeof(I16) )
    {
        _lastError = tr("Not enough memory to initialize current mode: ") +
                QString::number(_errorCode);;
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
        _lastError = tr("Cant config block mode: ") +
                QString::number(_errorCode);;
        return false;
    }

    _errorCode=D2K_AI_ContBufferSetup (_cardID,
                                       _samplesBlockBuffer_0,
                                       _measureSampleCount,
                                       &_resultBufferIdZero);
    if (_errorCode != NoError)
    {
        _lastError = tr("Cannot initialize buffer for channel 0: ") +
                QString::number(_errorCode);;
        return false;
    }

    _errorCode=D2K_AI_ContBufferSetup (_cardID,
                                       _samplesBlockBuffer_1,
                                       _measureSampleCount,
                                       &_resultBufferIdOne);
    if (_errorCode != NoError)
    {
        _lastError = tr("Cannot initialize buffer for channel 1: ") +
                QString::number(_errorCode);;
        return false;
    }

    return true;
}

void MeasureThread::_initializeCard()
{
    _cardID = ::D2K_Register_Card(DAQ_2213, 0);
    if (_cardID < 0)
    {
        _isWorking = false;
        _lastError = tr("Cant initialize device");
        emit someError();
    }
    else
    {
        _initializeChannels();
        _isWorking = true;
        _isUnitialize = false;
    }
}

U16 MeasureThread::getSamples()
{
    _mutex.tryLock();
    U16 temp = _sampleSingleshotValue_0;
    _singleshotDataReady = false;
    _mutex.unlock();
    return temp;
}

void MeasureThread::getSamplesBuffer(double* bufferZero,
                                    double* bufferOne)
{
    _mutex.tryLock();
    if (_measureSampleCount < MAXIMUM_PLOT_SAMPLES)
    {
        ::D2K_AI_ContVScale(_cardID,
                            AD_B_10_V,
                            _samplesBlockBuffer_0,
                            bufferZero,
                            _measureSampleCount);
    }
    else
    {
        int sampleStep = qRound((static_cast<double> (_measureSampleCount) /
                static_cast<double> (MAXIMUM_PLOT_SAMPLES)) - 0.5);

        for (int i = 0; i < MAXIMUM_PLOT_SAMPLES; i++)
        {
            _hfBuffer[i] = _samplesBlockBuffer_0[i*sampleStep];
        }

        ::D2K_AI_ContVScale(_cardID,
                            AD_B_10_V,
                            _hfBuffer,
                            bufferZero,
                            MAXIMUM_PLOT_SAMPLES);
    }
    /*if (bufferZero != 0)
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
    }*/
    _blockDataReady = false;
    _mutex.unlock();
}

void MeasureThread::getVoltage(double &ch0, double &ch1)
{
    _mutex.tryLock();
    ch0 = _voltageSingleshotValue_0;
    ch1 = _voltageSingleshotValue_1;
    _singleshotDataReady = false;
    _mutex.unlock();
}

F64* MeasureThread::getVoltageBuffer()
{
    _mutex.tryLock();
    F64 *temp = new F64[_measureSampleCount];
    for (unsigned int i = 0; i < _measureSampleCount; i++)
    {
        temp[i] = _voltageBlockBuffer_0[i];
    }
    _blockDataReady = false;
    _mutex.unlock();
    return temp;
}

bool MeasureThread::isSingleshotDataReady()
{
    return _singleshotDataReady;
}

bool MeasureThread::isBlockDataReady()
{
    return _blockDataReady;
}

void MeasureThread::setMeasuringInterval(quint32 msec)
{
    _mutex.tryLock();
    _measuringInterval = 1000 / msec;
    _mutex.unlock();
}

void MeasureThread::setParameters(ModeParameters parameters, bool update)
{
    _mutex.tryLock();

    _workingMode = parameters.mode;

    if ((_workingMode == MODE_BLOCK_MEASURING) &&
            (update == true))
    {
        ::D2K_AI_ContBufferReset(_cardID);
    }

    _channelZeroMeasuring = parameters.channelZeroState;
    _channelOneMeasuring = parameters.channelOneState;

    _measuringInterval      = 1000 / parameters.measuringInterval;

    _measureSampleCount     =
            static_cast<U32> (parameters.blockSize);
    _measureSampleInterval  =
            static_cast<U32> (parameters.samplingInterval);
    _measuringBlockInterval =
            static_cast<U32> (parameters.scaningInterval);

    if ((_workingMode == MODE_BLOCK_MEASURING) &&
            (update == true))
    {
        _initializeBlockMode();
    }

    _initializeChannels();

    _mutex.unlock();
}

void MeasureThread::_singleshotMeasure()
{
    _mutex.tryLock();
    if (_channelZeroMeasuring == STATE_ON)
    {
        _lastError = ::D2K_AI_VReadChannel(_cardID,
                              _channelsPins[0],
                              &_voltageSingleshotValue_0);
        if (_errorCode != NoError)
        {
            _lastError = tr("Error while reading: ") +
                    QString::number(_errorCode);
            _isWorking = false;

        }
    }
    if (_channelOneMeasuring == STATE_ON)
    {
        _lastError= ::D2K_AI_VReadChannel(_cardID,
                              _channelsPins[1],
                              &_voltageSingleshotValue_1);
        if (_errorCode != NoError)
        {
            _lastError = tr("Error while reading: ") +
                    QString::number(_errorCode);
            _isWorking = false;
        }
    }
    msleep(_measuringInterval);
    _singleshotDataReady = true;
    _mutex.unlock();
}

void MeasureThread::_blockMeasure()
{
    _mutex.tryLock();
    if (_channelZeroMeasuring == true)
    {
        _errorCode = ::D2K_AI_ContReadChannel (_cardID,
                                  _channelsPins[2],
                                  _resultBufferIdZero,
                                  _measureSampleCount,
                                  _measuringBlockInterval,
                                  _measureSampleInterval,
                                  SYNCH_OP);

        if (_errorCode != NoError)
        {
            _lastError = tr("Error while continue reading: ") +
                    QString::number(_errorCode);
            _isWorking = false;
        }
    }
    if (_channelOneMeasuring == true)
    {
        _errorCode = ::D2K_AI_ContReadChannel (_cardID,
                                  _channelsPins[1],
                                  _resultBufferIdOne,
                                  _measureSampleCount,
                                  _measuringBlockInterval,
                                  _measureSampleInterval,
                                  SYNCH_OP);

        if (_errorCode != NoError)
        {
            _lastError = tr("Error while continue reading: ") +
                    QString::number(_errorCode);
            _isWorking = false;
        }
    }
    _blockDataReady = true;
    _mutex.unlock();
}

QString MeasureThread::getLastError()
{
    return _lastError;
}

void MeasureThread::setChannelsPins(char pins[])
{
    for (int i = 0; i < MAXIMUM_CHANNELS; i++)
    {
        _channelsPins[i] = static_cast<I16> (pins[i]);
    }
}

void MeasureThread::setPin(int id, char value)
{
    _channelsPins[id] = static_cast<I16> (value);
}

void MeasureThread::singleshotMeasuring(bool state)
{
    _mutex.tryLock();
    _measuringSingleshot = state;
    _mutex.unlock();
}

void MeasureThread::blockMeasuring(bool state)
{
    _mutex.tryLock();
    _measuringBlock = state;
    _mutex.unlock();
}

void MeasureThread::_initializeChannels()
{
    if (_cardID == -1)
    {
        return;
    }

    _errorCode = NoError;

    for (int i = 0; i < MAXIMUM_CHANNELS; i++)
    {
        if (_channelsPins[i] != -1)
        {
            _errorCode = ::D2K_AI_CH_Config(
                        _cardID, _channelsPins[i], AD_B_10_V|AI_RSE);

            if (_errorCode != NoError)
            {
                _lastError = tr("Cannot initialize ") +
                        QString::number(_channelsPins[i]) +
                        tr(" channel: ") + QString::number(_errorCode);
            }
        }
    }
}
