#include "headers/mainwindow.h"
//#include "headers/parametersdialog.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>

#include "headers/blockdialog.h"
#include "headers/singleshotdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _isWorking(false),
    _isLogging(false),
    _plotBufferZero(0),
    _plotBufferOne(0),
    _workingTime(0)
{
    ui->setupUi(this);
    setWindowTitle(tr("DAQ 2213 Signal visualizer"));

    //    singleShot();
//        blocks();

    ui->ch_0_voltage_range_slider->setVisible(false);
    ui->ch_0_zoom_in_btn->setVisible(false);
    ui->ch_0_zoom_out_btn->setVisible(false);
    ui->ch_1_voltage_range_slider->setVisible(false);
    ui->ch_1_zoom_in_btn->setVisible(false);
    ui->ch_1_zoom_out_btn->setVisible(false);

    _logFile.setFileName("log.csv");
    _workingTime = new QTime;

    int maxSamplesRange = 250000;
    int maxMeasureInterval = 99;

    ui->meas_block_count_spin->setMaximum(maxSamplesRange);
    ui->meas_per_second_spin->setMaximum(maxMeasureInterval);

    delayedSlider = new TimerSlider(Qt::Horizontal, 0);
    delayedSlider->setRange(1,maxMeasureInterval);
    delayedSlider->setSingleStep(1);
    delayedSlider->setPageStep(3);
    connect(delayedSlider, SIGNAL(NewValue(int)),
            this, SLOT(_delayedSliderNewValue(int)));
    ui->horizontalLayout->insertWidget(5,delayedSlider);

    delayedSlider_2 = new TimerSlider(Qt::Horizontal, 0);
    delayedSlider_2->setRange(2, maxSamplesRange);
    delayedSlider_2->setSingleStep(1000);
    delayedSlider_2->setPageStep(5000);
    connect(delayedSlider_2, SIGNAL(NewValue(int)),
            this, SLOT(_delayedSliderNewValue_2(int)));
    ui->horizontalLayout_3->insertWidget(3,delayedSlider_2);

    delayedSlider->setFixedSize(300, 32);
    delayedSlider_2->setFixedSize(300, 32);

    ui->stop_btn->setChecked(true);

    _updateTimer = new QTimer;
    connect(_updateTimer, SIGNAL(timeout()),
            this, SLOT(_updatePlot()));

    _initializePlot();
    _initializeDataOperator();
    _updateTimer->setInterval(1);
    _updateTimer->start();
    showMaximized();


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
}

MainWindow::~MainWindow()
{
    _stopLogging();

    delete _workingTime;
    _workingTime = 0;

    delete _updateTimer;
    _dataOperator->stopWorking();
    delete _dataOperator;

    _updateTimer = 0;
    _dataOperator = 0;

    delete ui;
}

void MainWindow::_initializePlot()
{
    _plot = new GraphicPlot(tr("Signal 1/2"), 2);
    ui->v_lay->insertWidget(1, _plot);

    _hfPlot = new GraphicPlot(tr("Signal 3"), 1);
    _hfPlot->rescaleAxis(QwtPlot::yLeft, -1.0, 1.0);
    _hfPlot->rescaleAxis(QwtPlot::yRight, -1.0, 1.0);
    _hfPlot->setChannels(true, false);
    QVBoxLayout *lay = dynamic_cast<QVBoxLayout*> (
                centralWidget()->layout());

    lay->insertWidget(3, _hfPlot);

    QColor channelZeroColor = Qt::red;
    QColor channelOneColor = Qt::yellow;

//    QPalette pal = ui->channelZero_check->palette();
//    pal.setColor(QPalette::WindowText, channelZeroColor);
//    ui->channelZero_check->setPalette(pal);
//    pal.setColor(QPalette::WindowText, channelOneColor);
//    ui->channelOne_check->setPalette(pal);

    _plot->setCurveProperties(0, QPen(channelZeroColor, 3, Qt::DashLine));
    _plot->setCurveProperties(1, QPen(channelOneColor, 3, Qt::DotLine));

    _hfPlot->setCurveProperties(0, QPen(channelZeroColor, 3, Qt::DashLine));
    _hfPlot->setCurveProperties(1, QPen(channelOneColor, 3, Qt::DotLine));
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

    double ch0 = 0.0, ch1 = 0.0;

    if (_parameters.mode == MODE_SINGLESHOT_MEASURING)
    {
        _dataOperator->getVoltage(ch0, ch1);
        _plot->setPoint(ch0, ch1);
    }
    else if (_parameters.mode == MODE_BLOCK_MEASURING)
    {
        _dataOperator->getSamplesBuffer(_plotBufferZero,
                                        _plotBufferOne);
        _hfPlot->displayBlock();

        if (_plotBufferZero != 0)
        {
            ch0 = _plotBufferZero[_parameters.blockSize-1];
        }

        if (_plotBufferOne != 0)
        {
            ch1 = _plotBufferOne[_parameters.blockSize-1];
        }
    }
    else if (_parameters.mode == MODE_HF_MEASURING)
    {
        _dataOperator->getHfVoltageBuffer(_plotBufferZero);
        _hfPlot->displayBlock();

        if (_plotBufferZero != 0)
        {
            ch0 = _plotBufferZero[_parameters.blockSize-1];
        }
    }

    if (_isLogging == true)
    {
        QString logString;
        logString.append(_workingTime->toString("mm:ss:zzz"));
        logString.append("|");
        logString.append(QString::number(ch0));
        logString.append("|");
        logString.append(QString::number(ch1));
        logString.append("|");
        logString.append(";\r\n");

        _logFile.write(logString.toAscii());
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

    QTime time;

    #define CardNumber    0
    #define ADChan	     0
    #define ScanCount    250
    #define ScanIntrv     160
    #define SampleIntrv   160
    int max = 1000;

    //AI config constants definition
    #define ConfigCtrl    DAQ2K_AI_ADCONVSRC_Int
    #define TrigCtrl       DAQ2K_AI_TRGSRC_SOFT|DAQ2K_AI_TRGMOD_POST
    #define ReTrgCnt  0
    #define BufAutoReset    0

    //variables definition
    I16 cardID = -1;
    I16 err=0;
    U32 AccessCnt = 0;
    U32 startPos = 0;
    U32 MemSize   = 0;
    U16 BufId   = 0;
//    I16 InBuf[1000]; //AI data buffer
    U16 *InBuf = new U16[ScanCount];
//    F64 *VoltBuf = new F64[ScanCount];
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


    qDebug() << "Measuring";
    time.start();
    for (int i = 0; i < max; i ++)
    {
        err=D2K_AI_ContReadChannel (cardID,
                                    ADChan,
                                    BufId,
                                    ScanCount,
                                    ScanIntrv,
                                    SampleIntrv,
                                    SYNCH_OP);
        if (err!=NoError) {
            qDebug() <<"D2K_AI_ContReadChannel";
               //Error occurs !!
               //ToDo : Handle error here
        }
    }
    qDebug() <<time.elapsed();
    qDebug() << max * ScanCount;

    /*err=::D2K_AI_ContVScale(cardID, AD_B_10_V, (void*) InBuf, VoltBuf, 1000);
    if (err!=NoError) {
        qDebug() <<"D2K_AI_ContVScale";
           //Error occurs !!
           //ToDo : Handle error here
    }*/

    D2K_Release_Card(cardID);

    delete [] InBuf;
//    delete [] VoltBuf;
}

void MainWindow::on_parameters_btn_clicked()
{
    SingleshotDialog d;

    if (d.exec() == false)
    {
        return;
    }

    _setupSingleshotParameters(d);
}

void MainWindow::on_start_btn_clicked()//singleshot
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

    SingleshotDialog d;

    if (d.exec() == false)
    {
        ui->start_btn->setChecked(false);
        ui->stop_btn->setChecked(true);
//        _modeValue->setText(tr("No mode"));
        return;
    }

    _parameters.mode = MODE_SINGLESHOT_MEASURING;
    _setupSingleshotParameters(d);

    _plot->setChannels(ui->channelZero_check->isChecked(),
                       ui->channelOne_check->isChecked());

    _isWorking = true;
    _dataOperator->singleshotMeasuring(true);
    _dataOperator->startWorking();
    _workingTime->restart();
    _updateTimer->start();
}

void MainWindow::on_stop_btn_clicked()//singleshot
{
    ui->start_btn->setChecked(false);
    ui->stop_btn->setChecked(true);
    _isWorking = false;
    _updateTimer->stop();
    _dataOperator->singleshotMeasuring(false);
    _dataOperator->stopWorking();
    _plotBufferZero = 0;
    _plotBufferOne = 0;
//    _modeValue->setText(tr("No mode"));
    _plot->measuringStopped();
    ui->log_btn->setChecked(false);
    _stopLogging();
}

void MainWindow::on_parameters_btn_2_clicked()
{
    BlockDialog d;

    if (d.exec() == false)
    {
        return;
    }

    _setupBlockParameters(d);
}

void MainWindow::on_start_btn_2_clicked()//block
{
    if (_isWorking == true)
    {
        ui->stop_btn_2->setChecked(false);
        ui->start_btn_2->setChecked(true);
        return;
    }

    ui->stop_btn_2->setChecked(false);
    ui->start_btn_2->setChecked(true);

    BlockDialog d;

    if (d.exec() == false)
    {
        ui->start_btn_2->setChecked(false);
        ui->stop_btn_2->setChecked(true);
//        _modeValue->setText(tr("No mode"));
        return;
    }

    _parameters.mode = MODE_BLOCK_MEASURING;
    _setupBlockParameters(d);

    _hfPlot->setChannels(true, false);
    _isWorking = true;
    _dataOperator->blockMeasuring(true);
    _dataOperator->startWorking();
    _workingTime->restart();
    _updateTimer->start();
}

void MainWindow::on_stop_btn_2_clicked()//block
{
    ui->start_btn_2->setChecked(false);
    ui->stop_btn_2->setChecked(true);
    _isWorking = false;
    _dataOperator->blockMeasuring(false);
    _updateTimer->stop();
    _dataOperator->stopWorking();
    _plotBufferZero = 0;
    _plotBufferOne = 0;
//    _modeValue->setText(tr("No mode"));
    _hfPlot->measuringStopped();
    ui->log_btn->setChecked(false);
    _stopLogging();
}

void MainWindow::_setupSingleshotParameters(SingleshotDialog &p)
{
    int p1, p2;
    p.selectedPins(p1, p2);
//    _dataOperator->setChannelsPins(pins);
    _dataOperator->setPin(0, p1);
    _dataOperator->setPin(1, p2);

    delayedSlider->setValue(p.getMeasuresCount());

    _parameters.displayedInterval = 10;
    _parameters.measuringInterval = p.getMeasuresCount();

    //points per sec * displayed seconds
    _plot->setDisplayedPoints(_parameters.measuringInterval *
                              _parameters.displayedInterval,
                              _parameters.mode);

    _dataOperator->setParameters(_parameters, _isWorking);
}

void MainWindow::_setupBlockParameters(BlockDialog &p)
{
    delayedSlider_2->setValue(p.getSamplesCount());

    int p1;
    p.selectedPins(p1);
//    _dataOperator->setChannelsPins(pins);
    _dataOperator->setPin(2, p1);

    _parameters.measuringInterval = 1000;
    _parameters.blockSize = p.getSamplesCount();
    _parameters.scaningInterval = 160;
    _parameters.samplingInterval = 160;
    _hfPlot->setDisplayedPoints(_parameters.blockSize, _parameters.mode);

    _plotBufferZero = _hfPlot->initializeChannelZeroBuffer(
                _parameters.blockSize);

    _dataOperator->setParameters(_parameters, _isWorking);
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
        _plot->zoomAxis(QwtPlot::yLeft, 100 - value);
    }
}

void MainWindow::on_ch_1_voltage_range_slider_valueChanged(int value)
{
    if (_plot != 0)
    {
        _plot->zoomAxis(QwtPlot::yRight, 100 - value);
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
        _plotBufferZero = _plot->initializeChannelZeroBuffer(
                    _parameters.blockSize);
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
        _plotBufferOne = _plot->initializeChannelOneBuffer(
                    _parameters.blockSize);
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
//    ui->ch_0_voltage_range_slider->setVisible(state);
//    ui->ch_0_zoom_in_btn->setVisible(state);
//    ui->ch_0_zoom_out_btn->setVisible(state);
}

void MainWindow::_channelOneState(bool state)
{
    _plot->setChannels(ui->channelZero_check->isChecked(),
                       ui->channelOne_check->isChecked());
//    _plot->enableAxis(QwtPlot::yRight, false);
//    ui->ch_1_voltage_range_slider->setVisible(state);
//    ui->ch_1_zoom_in_btn->setVisible(state);
//    ui->ch_1_zoom_out_btn->setVisible(state);
}

void MainWindow::on_forward_btn_clicked()
{
    delayedSlider->setValue(delayedSlider->value() + 1);
}

void MainWindow::on_backward_btn_clicked()
{
    delayedSlider->setValue(delayedSlider->value() - 1);
}

void MainWindow::on_forward_btn_2_clicked()
{
    delayedSlider_2->setValue(delayedSlider_2->value() + 100);
}

void MainWindow::on_backward_btn_2_clicked()
{
    delayedSlider_2->setValue(delayedSlider_2->value() - 100);
}

void MainWindow::_delayedSliderNewValue(int value)
{
    ui->meas_per_second_spin->setValue(value);
//    return;
    _parameters.mode = MODE_SINGLESHOT_MEASURING;

    QSettings settings("settings.ini", QSettings::IniFormat, this);
    _parameters.measuringInterval = value;
    settings.setValue("measuring_interval", value);

    //points per sec * displayed seconds
    _plot->setDisplayedPoints(value * 10, _parameters.mode);

    if (_dataOperator != 0)
    {
        _dataOperator->setMeasuringInterval(value);
    }
}

void MainWindow::_delayedSliderNewValue_2(int value)
{
    ui->meas_block_count_spin->setValue(value);

    _parameters.mode = MODE_BLOCK_MEASURING;
    _parameters.measuringInterval = 1000;
    _parameters.blockSize = value;
    _parameters.scaningInterval = 160;
    _parameters.samplingInterval = 160;
    _hfPlot->setDisplayedPoints(value, _parameters.mode);

    _plotBufferZero = _hfPlot->initializeChannelZeroBuffer(
                _parameters.blockSize);

    if (_dataOperator != 0)
    {
        _dataOperator->setParameters(_parameters, _isWorking);
    }

    QSettings settings("settings.ini", QSettings::IniFormat, this);
    settings.setValue("measuring_samples_count", value);

//    if (_dataOperator != 0)
//    {
//        _dataOperator->setSampleCount(value);
//    }
}

void MainWindow::on_screenshot_btn_clicked()
{
    QPixmap screenshot = QPixmap::grabWidget(this);
    QString format = "png";
    QString defaultPath = QDir::currentPath();
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save as"),
                                                    defaultPath,
                                                    tr("%1 Files(*.%2);;All Files (*)")
                                                    .arg(format.toUpper())
                                                    .arg(format));
    if (filename.isEmpty() == true)
    {
        return;
    }

    screenshot.save(filename, format.toAscii());
}

void MainWindow::on_log_btn_clicked()
{
    _stopLogging();
    _startLogging();
}

void MainWindow::_startLogging()
{
    if (ui->log_btn->isChecked() == true)
    {
        bool result = _logFile.open(QFile::WriteOnly);
        if (result == false)
        {
            QMessageBox::critical(this,
                                  tr("Attention"),
                                  _logFile.errorString(),
                                  QMessageBox::Ok);
            return;
        }
        _isLogging = true;

        QString header;
        header.append(tr("TIME|"));
        header.append(tr("SIGNAL_1|"));
        header.append(tr("SIGNAL_2|"));
        header.append(tr("SIGNAL_3|;"));
        header.append("\r\n");
        _logFile.write(header.toAscii());
    }
}

void MainWindow::_stopLogging()
{
    if (_logFile.isOpen() == true)
    {
        _logFile.close();
        _isLogging = false;
    }
}

void MainWindow::on_meas_per_second_spin_valueChanged(int value)
{
    if (delayedSlider == 0)
    {
        return;
    }
    delayedSlider->setValue(value);
}

void MainWindow::on_meas_block_count_spin_valueChanged(int value)
{
    if (delayedSlider_2 == 0)
    {
        return;
    }
    delayedSlider_2->setValue(value);
}
