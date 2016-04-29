#ifndef DATAOPERATOR_H
#define DATAOPERATOR_H

#include <QThread>
#include <QMutex>

#include <D2kDask.h>
#include <DAQHeader.h>

class DataOperator : public QThread
{
public:
    enum
    {
        MODE_NO_MEASURING,
        MODE_SINGLESHOT_MEASURING,
        MODE_BLOCK_MEASURING
    };

    enum
    {
        OFF,
        ON
    };

    enum
    {
        CHANNEL_0,
        CHANNEL_1
    };

    explicit    DataOperator(QObject *parent = 0);
    virtual     ~DataOperator();

    void        startWorking();
    void        stopWorking();
    void        setWorkingMode(qint8 mode);
    void        setChannelStatus(qint8 channel, qint8 state);
    void        setMeasuringInterval(quint32 interval);
    void        setMeasureSampleInterval(quint32 interval);
    void        setSampleCount(quint32 count); //samples per block

protected:
    virtual void run();

private:
    bool    _isWorking;
    bool    _isUnitialize;

    QMutex  _mutex;
    I16     _errorCode;
    QString _lastError;

    U16*    _samplesBlockBuffer;
    U16     _sampleSingleshotValue;

    F64*    _voltageBlockBuffer;
    F64     _voltageSingleshotValue;

    qint8   _workingMode;
    qint8   _channelZeroMeasuring;
    qint8   _channelOneMeasuring;

    U32     _measureSampleInterval;     //block measuring time
                                        //[8:16777215]
    U32     _measureSampleCount;        //samples per block
                                        //[2:16777215]
    U32     _measuringInterval;         //delay between measurings
                                        //[160:16777215]
};

#endif // DATAOPERATOR_H
