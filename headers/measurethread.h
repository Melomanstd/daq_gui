#ifndef DATAOPERATOR_H
#define DATAOPERATOR_H

#define MAXIMUM_OPTION_VALUE        16777215
#define MAXIMUM_SAMPLES_INTERVAL    65535
#define MINIMUM_SAMPLES_PER_BLOCK   2
#define MINIMUM_SAMPLES_INTERVAL    160
#define MINIMUM_MEASURING_INTERVAL  160 //miliseconds
#define MAXIMUM_CHANNELS            3

#include <QThread>
#include <QMutex>
#include <QTime>

#include <D2kDask.h>
#include <DAQHeader.h>

#include "defines.h"

class MeasureThread : public QThread
{
    Q_OBJECT
public:
    explicit    MeasureThread(QObject *parent = 0);
    virtual     ~MeasureThread();

    void        startWorking();
    void        stopWorking();
    void        setWorkingMode(qint8 mode);
    void        setMeasuringInterval(quint32 msec);
    void        setChannelStatus(qint8 channel, qint8 state);
    void        setBlockMeasuringInterval(quint32 interval);
    void        setMeasureSampleInterval(quint32 interval);
    void        setSampleCount(quint32 count); //samples per block
    void        setParameters(ModeParameters parameters, bool update);
    void        setPin(int id, char value);

    void        getVoltage(double &ch0, double &ch1);
    F64*        getVoltageBuffer();

    U16         getSamples();
    void        getSamplesBuffer(double *bufferZero,
                                 double *bufferOne);

    bool        isDataReady();
    bool        isBlockDataReady();
    bool        isSingleshotDataReady();
    QString     getLastError();
    void        setChannelsPins(char pins[]);

    void        singleshotMeasuring(bool state);
    void        blockMeasuring(bool state);

protected:
    virtual void run();

private:
    bool        _initializeBlockMode();
    void        _initializeCard();
    void        _updateParameters();
    inline void _singleshotMeasure();
    inline void _blockMeasure();
    void        _initializeChannels();

signals:
    void someError();

private:
    bool            _isWorking;
    bool            _isUnitialize;
    mutable bool    _blockDataReady;
    mutable bool    _singleshotDataReady;
    BOOLEAN         _isDoubleBuffer;

    QMutex          _mutex;
    I16             _errorCode;
    QString         _lastError;

    U16*            _samplesBlockBuffer_0;
    U16             _sampleSingleshotValue_0;

    F64*            _voltageBlockBuffer_0;
    F64             _voltageSingleshotValue_0;

    U16*            _samplesBlockBuffer_1;
    U16             _sampleSingleshotValue_1;

    F64*            _voltageBlockBuffer_1;
    F64             _voltageSingleshotValue_1;

    qint8           _workingMode;
    qint8           _channelZeroMeasuring;
    qint8           _channelOneMeasuring;

    quint32         _measuringInterval;

    U32             _measureSampleInterval;     //block measuring time
                                                //[8:16777215]
    U32             _measureSampleCount;        //samples per block
                                                //[2:16777215]
    U32             _measuringBlockInterval;    //delay between measurings
                                                //[160:16777215]
    U16             _resultBufferIdZero;
    U16             _resultBufferIdOne;

    I16             _cardID;

    I16             _channelsPins[MAXIMUM_CHANNELS];

    U16*            _hfBuffer;

    bool            _measuringSingleshot;
    bool            _measuringBlock;
    QTime*          _blockModeTime;
    int             _tempValue;
};

#endif // DATAOPERATOR_H
