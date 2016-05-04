#include "headers/mainwindow.h"
#include "headers/parametersdialog.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stop_btn->setChecked(true);

    _updateTimer = new QTimer;
    connect(_updateTimer, SIGNAL(timeout()),
            this, SLOT(_updatePlot()));

    _initializePlot();
    _initializeDataOperator();
    _updateTimer->setInterval(1);
    _updateTimer->start();
//    singleShot();
//    blocks();

}

MainWindow::~MainWindow()
{
    delete _updateTimer;
    _dataOperator->stopWorking();
    delete _dataOperator;

    _updateTimer = 0;
    _dataOperator = 0;

    delete ui;
}

void MainWindow::_initializePlot()
{
    _plot = new GraphicPlot();
    _plot->setDisplayedPoints(10);
    ui->v_lay->addWidget(_plot);
    _plot->setAxisTitle(QwtPlot::yLeft, tr("Voltage"));
}

void MainWindow::_initializeDataOperator()
{
    _dataOperator = new DataOperator();
}

void MainWindow::_updatePlot()
{
    if (_dataOperator->isDataReady() == false)
    {
        return;
    }

    if (_parameters.mode == MODE_SINGLESHOT_MEASURING)
    {
        _plot->setPoint(_dataOperator->getVoltage());
    }
    else if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        //TODO
    }
}

void MainWindow::singleShot()
{
    //constants definition
    #define CardNumber	0
    #define ADChan	0

    //variables definition
    I16 cardID = -1;
    I16 err=0;
    F64 AdVoltage = 0.0; // returned AI voltage
    U16 AdValue = 0;

        cardID = D2K_Register_Card(DAQ_2213, CardNumber);
        if (cardID<0) {
               //Error occurs !!
               //ToDo : Handle error here
        }

        err = D2K_AI_CH_Config(cardID, ADChan, AD_B_10_V|AI_RSE);
        if (err<0) {
               //Error occurs !!
               //ToDo : Handle error here
        }
        err = D2K_AI_VReadChannel(cardID, ADChan, &AdVoltage);
        if (err!=NoError) {
               //Error occurs !!
               //ToDo : Handle error here
        }

        err = ::D2K_AI_ReadChannel(cardID, ADChan, &AdValue);
        if (err!=NoError) {
               //Error occurs !!
               //ToDo : Handle error here
        }

        D2K_Release_Card(cardID);
}

void MainWindow::blocks()
{
    //constants definition
    #define CardNumber    0
    #define ADChan	     0
    #define ScanCount    1000
    #define ScanIntrv     30000
    #define SampleIntrv   30000

    //AI config constants definition
    #define ConfigCtrl    DAQ2K_AI_ADCONVSRC_Int
    #define TrigCtrl       DAQ2K_AI_TRGSRC_SOFT|DAQ2K_AI_TRGMOD_POST
    #define ReTrgCnt  0
    #define BufAutoReset    1

    //variables definition
    I16 cardID = -1;
    I16 err=0;
    U32 AccessCnt = 0;
    U32 startPos = 0;
    U32 MemSize   = 0;
    U16 BufId   = 0;
//    I16 InBuf[1000]; //AI data buffer
    U16 *InBuf = new U16[1000];
    F64 *VoltBuf = new F64[1000];
    U16 vi = 0;

    cardID = D2K_Register_Card(DAQ_2213, CardNumber);
    if (cardID<0) {
        qDebug() <<"D2K_Register_Card";
           //Error occurs !!
           //ToDo : Handle error here
    }
    err = D2K_AI_InitialMemoryAllocated(cardID, &MemSize);
    if (err!=NoError) {
        qDebug() <<"D2K_AI_InitialMemoryAllocated";
           //Error occurs !!
           //ToDo : Handle error here
    }
    if (MemSize*1024 < ScanCount*sizeof(I16) ) {
        qDebug() << "MemSize";
           //available memory size for analog input in the device driver
           //is smaller than the data size specified!!
           //ToDo : do something here
    }

    err = D2K_AI_CH_Config(cardID, ADChan, AD_B_10_V|AI_RSE);
    if (err<0) {
        qDebug() <<"D2K_AI_CH_Config";
           //Error occurs !!
           //ToDo : Handle error here
    }
    err = D2K_AI_AsyncDblBufferMode(cardID, 0);
    if (err!=NoError) {
        qDebug() <<"D2K_AI_AsyncDblBufferMode";
           //Error occurs !!
           //ToDo : Handle error here
    }
    err=D2K_AI_Config(cardID, ConfigCtrl, TrigCtrl, 0, 0, ReTrgCnt, BufAutoReset);
    if (err!=NoError) {
        qDebug() <<"D2K_AI_Config";
           //Error occurs !!
           //ToDo : Handle error here
    }
    err=D2K_AI_ContBufferSetup (cardID, InBuf, ScanCount, &BufId);
    if (err!=NoError) {
        qDebug() <<"D2K_AI_ContBufferSetup";
           //Error occurs !!
           //ToDo : Handle error here
    }
    err=D2K_AI_ContReadChannel (cardID, ADChan, BufId, ScanCount, ScanIntrv, SampleIntrv, SYNCH_OP);
    if (err!=NoError) {
        qDebug() <<"D2K_AI_ContReadChannel";
           //Error occurs !!
           //ToDo : Handle error here
    }

    err=::D2K_AI_ContVScale(cardID, AD_B_10_V, (void*) InBuf, VoltBuf, 1000);
    if (err!=NoError) {
        qDebug() <<"D2K_AI_ContVScale";
           //Error occurs !!
           //ToDo : Handle error here
    }

    D2K_Release_Card(cardID);
}

void MainWindow::on_parameters_btn_clicked()
{
    _setupParameters();
}

void MainWindow::on_start_btn_clicked()
{
    ui->stop_btn->setChecked(false);
    ui->start_btn->setChecked(true);
    if (_setupParameters() == false)
    {
        ui->start_btn->setChecked(false);
        ui->stop_btn->setChecked(true);
        return;
    }
    _dataOperator->startWorking();
}

void MainWindow::on_stop_btn_clicked()
{
    ui->start_btn->setChecked(false);
    ui->stop_btn->setChecked(true);

    _dataOperator->stopWorking();
}

bool MainWindow::_setupParameters()
{
    ParametersDialog p(this);
    p.setDefaultParameters(_lastParameters());

    if (p.exec() != QDialog::Accepted)
    {
        return false;
    }
    QSettings settings("settings.ini", QSettings::IniFormat, this);

    _parameters.mode = p.getMeasuringMode();
    _parameters.measuringInterval = p.getMeasuringTime();
    int value = p.getSamplesCount();

    if (p.channelZeroState() == true)
    {
        _parameters.channelZeroState = STATE_ON;
        settings.setValue("channel_zero", STATE_ON);
    }
    else
    {
        _parameters.channelZeroState = STATE_OFF;
        settings.setValue("channel_zero", STATE_OFF);
    }

    if (p.channelOneState() == true)
    {
        _parameters.channelOneState = STATE_ON;
        settings.setValue("channel_one", STATE_ON);
    }
    else
    {
        _parameters.channelOneState = STATE_OFF;
        settings.setValue("channel_one", STATE_OFF);
    }

    settings.setValue("measuring_mode", _parameters.mode);
    settings.setValue("measuring_interval", _parameters.measuringInterval);
    if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        _parameters.blockSize = value;
        settings.setValue("samples_count", value);
    }
    else if (_parameters.mode == MODE_SINGLESHOT_MEASURING)
    {
        _parameters.displayedInterval = value;
//        _plot->setAxisTitle(QwtPlot::xBottom, tr("Seconds"));

        //points per sec * displayed seconds
        _plot->setDisplayStep(_parameters.measuringInterval);
        _plot->setDisplayedPoints(_parameters.measuringInterval *
                                  _parameters.displayedInterval);
        settings.setValue("displayed_interval", value);
    }

//    int updateInterval = 1000 / _parameters.measuringInterval;

    if (_dataOperator != 0)
    {
        _dataOperator->setParameters(_parameters);
    }
    return true;
}

ModeParameters MainWindow::_lastParameters()
{
    ModeParameters parameters;
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    parameters.mode = settings.value(
                "measuring_mode", MODE_SINGLESHOT_MEASURING).toInt();
    parameters.measuringInterval = settings.value(
                "measuring_interval", 1).toInt();
    parameters.blockSize = settings.value(
                "samples_count", 10).toInt();
    parameters.displayedInterval = settings.value(
                "displayed_interval", 10).toInt();
    parameters.channelZeroState = settings.value(
                "channel_zero", STATE_ON).toInt();
    parameters.channelOneState = settings.value(
                "channel_one", STATE_OFF).toInt();

    return parameters;
}
