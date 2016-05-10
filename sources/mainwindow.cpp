#include "headers/mainwindow.h"
#include "headers/parametersdialog.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _isWorking(false),
    _plotBufferZero(0),
    _plotBufferOne(0)
{
    ui->setupUi(this);

    setWindowTitle(tr("DAQ 2213 Signal visualizer"));

    delayedSlider = new TimerSlider(Qt::Horizontal, 0);
    connect(delayedSlider, SIGNAL(NewValue(int)),
            this, SLOT(_delayedSliderNewValue(int)));
    ui->horizontalLayout->insertWidget(4,delayedSlider);

    _modeLabel = new QLabel(tr("Current mode:"));
    _modeValue = new QLabel(tr("No mode"));
    _intervalLabel = new QLabel(tr("Measuring interval(msec):"));
    _intervalValue = new QLabel(QString::number(
                               (delayedSlider->value())));

    QFont font = _modeLabel->font();
    font.setBold(true);
    font.setPointSize(12);

    _modeLabel->setFont(font);
    _modeValue->setFont(font);
    _intervalLabel->setFont(font);
    _intervalValue->setFont(font);

    statusBar()->addWidget(_modeLabel);
    statusBar()->addWidget(_modeValue);
    statusBar()->addWidget(_intervalLabel);
    statusBar()->addWidget(_intervalValue);

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

    QSettings settings("settings.ini", QSettings::IniFormat, this);
    if (settings.value("channel_zero", STATE_ON) == STATE_ON)
    {
        ui->channelZero_check->setChecked(true);
        _channelZeroState(true);
    }
    else
    {
        ui->channelZero_check->setChecked(false);
        _channelZeroState(false);
    }

    if (settings.value("channel_one", STATE_OFF) == STATE_ON)
    {
        ui->channelOne_check->setChecked(true);
        _channelOneState(true);
    }
    else
    {
        ui->channelOne_check->setChecked(false);
        _channelOneState(false);
    }

    ui->ch_0_voltage_range_slider->setValue(80);
    ui->ch_1_voltage_range_slider->setValue(80);

    QColor channelZeroColor = Qt::blue;
    QColor channelOneColor = Qt::darkCyan;

    QPalette pal = ui->channelZero_check->palette();
    pal.setColor(QPalette::WindowText, channelZeroColor);
    ui->channelZero_check->setPalette(pal);
    pal.setColor(QPalette::WindowText, channelOneColor);
    ui->channelOne_check->setPalette(pal);

    _plot->setCurveProperties(0, QPen(channelZeroColor, 3));
    _plot->setCurveProperties(1, QPen(channelOneColor, 3));
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
//    _plot->enableAxis(QwtPlot::yRight);
}

void MainWindow::_initializeDataOperator()
{
    _dataOperator = new DataOperator();

    connect(_dataOperator, SIGNAL(someError()),
            this, SLOT(_displayError()));
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
{//test function
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
{//test function
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

    if ((ui->channelZero_check->isChecked() == false) &&
        (ui->channelOne_check->isChecked() == false))
    {
        QMessageBox::critical(this,
                              tr("Error"),
                              tr("Select atleast one channel for measuring"),
                              QMessageBox::Ok);
        return;
    }

    ui->stop_btn->setChecked(false);
    ui->start_btn->setChecked(true);
    if (_setupParameters() == false)
    {
        ui->start_btn->setChecked(false);
        ui->stop_btn->setChecked(true);
        _modeValue->setText(tr("No mode"));
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
    _updateTimer->stop();
    _dataOperator->stopWorking();
    _plotBufferZero = 0;
    _plotBufferOne = 0;
    _modeValue->setText(tr("No mode"));
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

    settings.setValue("measuring_mode", _parameters.mode);

    if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        delayedSlider->setMaximum(990);
        delayedSlider->setMinimum(10);
        delayedSlider->setValue(10);

        _parameters.measuringInterval = 990;
        _parameters.blockSize = p.getSamplesPerMeasuring();
        _parameters.scaningInterval = p.getScaningInterval();
        _parameters.samplingInterval = p.getSamplesInterval();
        _plot->setDisplayStep(_parameters.blockSize / 10);
        _plot->setDisplayedPoints(_parameters.blockSize,
                                  !_isWorking,
                                  _parameters.mode);
        settings.setValue("samples_count",
                          _parameters.blockSize);

        if (ui->channelZero_check->isChecked() == true)
        {
            _plotBufferZero = _plot->initializeChannelZeroBuffer(
                        _parameters.blockSize);
        }
        if (ui->channelOne_check->isChecked() == true)
        {
            _plotBufferOne = _plot->initializeChannelOneBuffer(
                        _parameters.blockSize);
        }

        _modeValue->setText(tr("Block measuring mode"));
    }
    else if (_parameters.mode == MODE_SINGLESHOT_MEASURING)
    {
        delayedSlider->setMaximum(100);
        delayedSlider->setMinimum(1);
        delayedSlider->setValue(1);
        delayedSlider->setPageStep(1);
        delayedSlider->setSingleStep(1);

        _parameters.displayedInterval = p.getDisplayedInterval();
        _parameters.measuringInterval = p.getMeasuringsPerSecond();
//        _plot->setAxisTitle(QwtPlot::xBottom, tr("Seconds"));

        _plot->setDisplayStep(_parameters.measuringInterval);
        //points per sec * displayed seconds
        _plot->setDisplayedPoints(_parameters.measuringInterval *
                                  _parameters.displayedInterval,
                                  !_isWorking,
                                  _parameters.mode);
        settings.setValue("displayed_interval",
                          _parameters.displayedInterval);

        _plotBufferZero = 0;
        _plotBufferOne = 0;

        _modeValue->setText(tr("Singleshot measuring mode"));
    }

    _plot->setChannels(ui->channelZero_check->isChecked(),
                       ui->channelOne_check->isChecked());

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
        _plot->rescaleAxis(QwtPlot::yLeft, 100 - value);
    }
}

void MainWindow::on_ch_1_voltage_range_slider_valueChanged(int value)
{
    if (_plot != 0)
    {
        _plot->rescaleAxis(QwtPlot::yRight, 100 - value);
    }
}

void MainWindow::on_ch_0_zoom_in_btn_clicked()
{
    ui->ch_0_voltage_range_slider->setValue(
                ui->ch_0_voltage_range_slider->value() + 5);
}

void MainWindow::on_ch_0_zoom_out_btn_clicked()
{
    ui->ch_0_voltage_range_slider->setValue(
                ui->ch_0_voltage_range_slider->value() - 5);
}

void MainWindow::on_ch_1_zoom_in_btn_clicked()
{
    ui->ch_1_voltage_range_slider->setValue(
                ui->ch_1_voltage_range_slider->value() + 5);
}

void MainWindow::on_ch_1_zoom_out_btn_clicked()
{
    ui->ch_1_voltage_range_slider->setValue(
                ui->ch_1_voltage_range_slider->value() - 5);
}

void MainWindow::on_channelZero_check_toggled(bool state)
{
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    if (state == true)
    {
        _parameters.channelZeroState = STATE_ON;
        settings.setValue("channel_zero", STATE_ON);
    }
    else
    {
        _parameters.channelZeroState = STATE_OFF;
        settings.setValue("channel_zero", STATE_OFF);
        _plotBufferZero = 0;
    }
    _channelZeroState(state);
    if (_dataOperator != 0)
    {
        _dataOperator->setChannelStatus(CHANNEL_0,
                                        _parameters.channelZeroState);
    }
}

void MainWindow::on_channelOne_check_toggled(bool state)
{
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    if (state == true)
    {
        _parameters.channelOneState = STATE_ON;
        settings.setValue("channel_one", STATE_ON);
    }
    else
    {
        _parameters.channelOneState = STATE_OFF;
        settings.setValue("channel_one", STATE_OFF);
        _plotBufferOne = 0;
    }
    _channelOneState(state);
    if (_dataOperator != 0)
    {
        _dataOperator->setChannelStatus(CHANNEL_1,
                                        _parameters.channelOneState);
    }
}

void MainWindow::_displayError()
{
    QMessageBox::critical(this,
                          tr("Error"),
                          _dataOperator->getLastError(),
                          QMessageBox::Ok);
}

void MainWindow::_channelZeroState(bool state)
{
    _plot->setChannels(ui->channelZero_check->isChecked(),
                       ui->channelOne_check->isChecked());
//    _plot->enableAxis(QwtPlot::yLeft, false);
    ui->ch_0_voltage_range_slider->setVisible(state);
    ui->ch_0_zoom_in_btn->setVisible(state);
    ui->ch_0_zoom_out_btn->setVisible(state);
}

void MainWindow::_channelOneState(bool state)
{
    _plot->setChannels(ui->channelZero_check->isChecked(),
                       ui->channelOne_check->isChecked());
//    _plot->enableAxis(QwtPlot::yRight, false);
    ui->ch_1_voltage_range_slider->setVisible(state);
    ui->ch_1_zoom_in_btn->setVisible(state);
    ui->ch_1_zoom_out_btn->setVisible(state);
}

void MainWindow::on_forward_btn_clicked()
{
    int temp = 0;
    if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        temp = 5;
    }
    else
    {
        temp = 1;
    }
    delayedSlider->setValue(delayedSlider->value() + temp);
}

void MainWindow::on_backward_btn_clicked()
{
    int temp = 0;
    if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        temp = 5;
    }
    else
    {
        temp = 1;
    }
    delayedSlider->setValue(delayedSlider->value() - temp);
}

void MainWindow::_delayedSliderNewValue(int value)
{
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    _parameters.measuringInterval = value;
    settings.setValue("measuring_interval", value);

    if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        value = 1000 - value;
    }
    else
    {
        _plot->setDisplayStep(_parameters.measuringInterval);
        //points per sec * displayed seconds
        _plot->setDisplayedPoints(_parameters.measuringInterval *
                                  _parameters.displayedInterval,
                                  true,
                                  _parameters.mode);
    }

    if (_dataOperator != 0)
    {
        _dataOperator->setMeasuringInterval(value);
    }
    _intervalValue->setText(QString::number(value));
}
