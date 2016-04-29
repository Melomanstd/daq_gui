#include "headers/dataoperator.h"

DataOperator::DataOperator(QObject *parent)
    :   QThread(parent),
        _isWorking(false)
{

}

DataOperator::~DataOperator()
{

}

void DataOperator::run()
{
    while (_isWorking)
    {
        //
    }
}

void DataOperator::stopWorking()
{
    _mutex.tryLock();
    _isWorking = false;
    _mutex.unlock();
}
