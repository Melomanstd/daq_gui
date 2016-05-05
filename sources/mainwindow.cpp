#include "headers/mainwindow.h"
#include "headers/parametersdialog.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _isWorking(false),
    _plotBufferZero(0),
    _plotBufferOne(0)
{
    ui->setupUi(this);

    QPalette pal = ui->channelZero_check->palette();
    pal.setColor(QPalette::WindowText, Qt::blue);
    ui->channelZero_check->setPalette(pal);
    pal.setColor(QPalette::WindowText, Qt::darkCyan);
    ui->channelOne_check->setPalette(pal);

    ui->stop_btn->setChecked(true);

    _updateTimer = new QTimer;
    connect(_updateTimer, SIGNAL(timeout()),
            this, SLOT(_updatePlot()));

    _initializePlot();
    _initializeDataOperator();
    _updateTimer->setInterval(1);
    _updateTimer->start();
    showMaximized();
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
//    _plot->setDisplayedPoints(10, true, );

//    ui->v_lay->addWidget(_plot);
    ui->v_lay->insertWidget(1, _plot);

    _plot->setAxisTitle(QwtPlot::yLeft, tr("Channel 0 Voltage"));
    _plot->setAxisTitle(QwtPlot::yRight, tr("Channel 1 Voltage"));
    _plot->enableAxis(QwtPlot::yRight);
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

    double ch0, ch1;

    if (_parameters.mode == MODE_SINGLESHOT_MEASURING)
    {
        _dataOperator->getVoltage(ch0, ch1);
        _plot->setPoint(ch0, ch1);
    }
    else if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        _dataOperator->getSamplesBuffer(_plotBufferZero,
                                        _plotBufferOne);
        _plot->displayBlock();
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
    if (_isWorking == true)
    {
        ui->stop_btn->setChecked(false);
        ui->start_btn->setChecked(true);
        return;
    }
    ui->stop_btn->setChecked(false);
    ui->start_btn->setChecked(true);
    if (_setupParameters() == false)
    {
        ui->start_btn->setChecked(false);
        ui->stop_btn->setChecked(true);
        return;
    }
    _isWorking = true;
    _dataOperator->startWorking();
}

void MainWindow::on_stop_btn_clicked()
{
    ui->start_btn->setChecked(false);
    ui->stop_btn->setChecked(true);
    _isWorking = false;
    _dataOperator->stopWorking();
    _plotBufferZero = 0;
    _plotBufferOne = 0;
}

bool MainWindow::_setupParameters()
{
    ParametersDialog p(this);
    p.setDefaultParameters(_lastParameters());

    if (p.exec() != QDialog::Accepted)
    {
        return false;
    }
    _updateTimer->stop();
    QSettings settings("settings.ini", QSettings::IniFormat, this);

    _parameters.mode = p.getMeasuringMode();
    _parameters.measuringInterval = p.getMeasuringTime();
    int value = p.getSamplesCount();

    if ((p.channelOneState() == false) &&
            (p.channelZeroState() == false))
    {
        _updateTimer->start();
        return false;
    }

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

    _plot->enableAxis(QwtPlot::yLeft, p.channelZeroState());
    _plot->enableAxis(QwtPlot::yRight, p.channelOneState());
    _plot->setChannels(p.channelZeroState(),
                       p.channelOneState());

    settings.setValue("measuring_mode", _parameters.mode);
    settings.setValue("measuring_interval", _parameters.measuringInterval);
    if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        _parameters.blockSize = value;
        _plot->setDisplayStep(value / 10);
        _plot->setDisplayedPoints(value, !_isWorking, _parameters.mode);
        settings.setValue("samples_count", value);
        if (p.channelZeroState() == true)
        {
            _plotBufferZero = _plot->initializeChannelZeroBuffer(value);
        }
        if (p.channelOneState() == true)
        {
            _plotBufferOne = _plot->initializeChannelOneBuffer(value);
        }
    }
    else if (_parameters.mode == MODE_SINGLESHOT_MEASURING)
    {
        _parameters.displayedInterval = value;
//        _plot->setAxisTitle(QwtPlot::xBottom, tr("Seconds"));

        //points per sec * displayed seconds
        _plot->setDisplayStep(_parameters.measuringInterval);
        _plot->setDisplayedPoints(_parameters.measuringInterval *
                                  _parameters.displayedInterval,
                                  !_isWorking,
                                  _parameters.mode);
        settings.setValue("displayed_interval", value);
    }

//    int updateInterval = 1000 / _parameters.measuringInterval;

    if (_dataOperator != 0)
    {
        _dataOperator->setParameters(_parameters);
    }
    _updateTimer->start();
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

void MainWindow::on_ch_0_voltage_range_slider_valueChanged(int value)
{
    if (_plot != 0)
    {
        _plot->rescaleAxis(QwtPlot::yLeft, value);
    }
}

void MainWindow::on_ch_1_voltage_range_slider_valueChanged(int value)
{
    if (_plot != 0)
    {
        _plot->rescaleAxis(QwtPlot::yRight, value);
    }
}
