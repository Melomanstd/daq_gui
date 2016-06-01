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
    _workingTime(0),
    _isBlockRunning(false),
    _isSingleshotRunning(false),
    _value0(0.0),
    _value1(0.0),
    _value2(0.0),
    _lastInterval(1)
{
    ui->setupUi(this);
    setWindowTitle(tr("DAQ 2213 Signal visualizer"));

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
    ui->horizontalLayout->insertWidget(6,delayedSlider);

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
    ui->stop_btn_2->setChecked(true);

    _initializePlot();
    _initializeDataOperator();

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

    delayedSlider->setVisible(false);
    delayedSlider_2->setVisible(false);
    ui->forward_btn->setVisible(false);
    ui->forward_btn_2->setVisible(false);
    ui->backward_btn->setVisible(false);
    ui->backward_btn_2->setVisible(false);
    ui->meas_block_count_spin->setVisible(false);
    ui->meas_per_second_spin->setVisible(false);
}

MainWindow::~MainWindow()
{
    _stopLogging();

    delete _workingTime;
    _workingTime = 0;

    _measureThread->stopWorking();
    delete _measureThread;

    _measureThread = 0;

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
    QColor channelOneColor = Qt::green;

    QPalette pal = ui->signal_1_color_lbl->palette();
    pal.setColor(QPalette::Window, channelZeroColor);
    ui->signal_1_color_lbl->setPalette(pal);

    pal = ui->signal_2_color_lbl->palette();
    pal.setColor(QPalette::Window, channelOneColor);
    ui->signal_2_color_lbl->setPalette(pal);

    _plot->setCurveProperties(0, QPen(channelZeroColor, 3, Qt::DashLine));
    _plot->setCurveProperties(1, QPen(channelOneColor, 3, Qt::DotLine));

    _hfPlot->setCurveProperties(0, QPen(channelZeroColor, 3, Qt::DashLine));
    _hfPlot->setCurveProperties(1, QPen(channelOneColor, 3, Qt::DotLine));
}

void MainWindow::_initializeDataOperator()
{
    _measureThread = new MeasureThread();

    connect(_measureThread, SIGNAL(someError()),
            this, SLOT(_displayError()));

    connect(_measureThread, SIGNAL(measured()),
            this, SLOT(_updatePlot()));
}

void MainWindow::_updatePlot()
{
    _readSingleshotData();
    _readBlockData();
//    _measureThread->resumeThread();
}

void MainWindow::_readBlockData()
{
    if (_measureThread->isBlockDataReady() == true)
    {
        _measureThread->getSamplesBuffer(_plotBufferZero,
                                        _plotBufferOne);
        _hfPlot->displayBlock();

        if (_plotBufferZero != 0)
        {
            _value2 = _plotBufferZero[MAXIMUM_PLOT_SAMPLES-1];
        }

        if (_isLogging == true)
        {
            _writeLog();
        }
    }
}

void MainWindow::_readSingleshotData()
{
    if (_measureThread->isSingleshotDataReady() == true)
    {
        _measureThread->getVoltage(_value0, _value1);
        _plot->setPoint(_value0, _value1);

        if ((_isLogging == true) && (_isBlockRunning == false))
        {
            _writeLog();
        }
    }
}

void MainWindow::_writeLog()
{
    _logString.clear();
    _logString.append(QString::number(_workingTime->elapsed()));//_workingTime->toString("mm:ss:zzz")
    _logString.append("|");
    _logString.append(QString::number(_value0));
    _logString.append("|");
    _logString.append(QString::number(_value1));
    _logString.append("|");
    _logString.append(QString::number(_value2));
    _logString.append("|");
    _logString.append(";\r\n");

    _logFile.write(_logString.toAscii());
}

void MainWindow::on_parameters_btn_clicked()
{
    int parameters[3];
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    parameters[0] = settings.value("measuring_interval", 1).toInt();
    parameters[1] = settings.value("signal_1_pin", 0).toInt();
    parameters[2] = settings.value("signal_2_pin", 1).toInt();

    SingleshotDialog d(parameters);

    if (d.exec() == false)
    {
        return;
    }

    _setupSingleshotParameters(d);
}

void MainWindow::on_start_btn_clicked()//singleshot
{
    if (_isSingleshotRunning == true)
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

    int parameters[3];
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    parameters[0] = settings.value("measuring_interval", 1).toInt();
    parameters[1] = settings.value("signal_1_pin", 0).toInt();
    parameters[2] = settings.value("signal_2_pin", 1).toInt();
    SingleshotDialog d(parameters);

    if (d.exec() == false)
    {
        ui->start_btn->setChecked(false);
        ui->stop_btn->setChecked(true);
        return;
    }

    _parameters.mode = MODE_SINGLESHOT_MEASURING;
    _setupSingleshotParameters(d);

    _plot->setChannels(ui->channelZero_check->isChecked(),
                       ui->channelOne_check->isChecked());
    _measureThread->singleshotMeasuring(true);
    _isSingleshotRunning = true;

    _tryToStart();
}

void MainWindow::on_stop_btn_clicked()//singleshot
{
    if (_isSingleshotRunning == false)
    {
        ui->start_btn->setChecked(false);
        ui->stop_btn->setChecked(true);
        return;
    }

    ui->start_btn->setChecked(false);
    ui->stop_btn->setChecked(true);
    _measureThread->singleshotMeasuring(false);
    _plot->measuringStopped();
    _isSingleshotRunning = false;

    _tryToStop();
}

void MainWindow::on_parameters_btn_2_clicked()
{
    int parameters[2];
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    parameters[0] = settings.value("samples_count", 1).toInt();
    parameters[1] = settings.value("signal_3_pin", 2).toInt();
    BlockDialog d(parameters);

    if (d.exec() == false)
    {
        return;
    }

    _setupBlockParameters(d);
}

void MainWindow::on_start_btn_2_clicked()//block
{
    if (_isBlockRunning == true)
    {
        ui->stop_btn_2->setChecked(false);
        ui->start_btn_2->setChecked(true);
        return;
    }

    ui->stop_btn_2->setChecked(false);
    ui->start_btn_2->setChecked(true);

    int parameters[2];
    QSettings settings("settings.ini", QSettings::IniFormat, this);
    parameters[0] = settings.value("samples_count", 1).toInt();
    parameters[1] = settings.value("signal_3_pin", 2).toInt();
    BlockDialog d(parameters);

    if (d.exec() == false)
    {
        ui->start_btn_2->setChecked(false);
        ui->stop_btn_2->setChecked(true);
        return;
    }

    _plot->setDisplayedPoints(200, MODE_SINGLESHOT_MEASURING, 200);
    _measureThread->setMeasuringInterval(1);

    _parameters.mode = MODE_BLOCK_MEASURING;
    _setupBlockParameters(d);
    _hfPlot->setChannels(true, false);
    _measureThread->blockMeasuring(true);
    _isBlockRunning = true;

    _tryToStart();
}

void MainWindow::on_stop_btn_2_clicked()//block
{
    if (_isBlockRunning == false)
    {
        ui->start_btn_2->setChecked(false);
        ui->stop_btn_2->setChecked(true);
        return;
    }

    _plot->setDisplayedPoints(10 * _lastInterval,
                              MODE_SINGLESHOT_MEASURING,
                              10 * _lastInterval);
    _measureThread->setMeasuringInterval(_lastInterval);

    ui->start_btn_2->setChecked(false);
    ui->stop_btn_2->setChecked(true);
    _measureThread->blockMeasuring(false);
    _plotBufferZero = 0;
    _plotBufferOne = 0;
    _hfPlot->measuringStopped();
    _isBlockRunning = false;

    _tryToStop();
}

void MainWindow::_setupSingleshotParameters(SingleshotDialog &p)
{
    int p1, p2;
    int temp = 0;
    p.selectedPins(p1, p2);
    _measureThread->setPin(0, p1);
    _measureThread->setPin(1, p2);

    delayedSlider->setValue(p.getMeasuresCount());

    _parameters.displayedInterval = 10;
    _parameters.measuringInterval = p.getMeasuresCount();
    _lastInterval = _parameters.measuringInterval;

    temp = _parameters.measuringInterval *
                _parameters.displayedInterval;

    QSettings settings("settings.ini", QSettings::IniFormat, this);
    settings.setValue("measuring_interval", _parameters.measuringInterval);
    settings.setValue("signal_1_pin", p1);
    settings.setValue("signal_2_pin", p2);

    //points per sec * displayed seconds
    _plot->setDisplayedPoints(temp, _parameters.mode, temp);

    _measureThread->setParameters(_parameters, _isWorking);
}

void MainWindow::_setupBlockParameters(BlockDialog &p)
{
    delayedSlider_2->setValue(p.getSamplesCount());

    int p1;
    p.selectedPins(p1);
    _measureThread->setPin(2, p1);

    _parameters.measuringInterval = delayedSlider->value();
    _parameters.blockSize = p.getSamplesCount();
    _parameters.scaningInterval = 160;
    _parameters.samplingInterval = 160;

    QSettings settings("settings.ini", QSettings::IniFormat, this);
    settings.setValue("samples_count", _parameters.blockSize);
    settings.setValue("signal_3_pin", p1);

    if (_parameters.blockSize > MAXIMUM_PLOT_SAMPLES)
    {
        _plotBufferZero = _hfPlot->initializeChannelZeroBuffer(
                    MAXIMUM_PLOT_SAMPLES);
        _hfPlot->setDisplayedPoints(MAXIMUM_PLOT_SAMPLES,
                                    _parameters.mode,
                                    _parameters.blockSize);
    }
    else
    {
        _hfPlot->setDisplayedPoints(_parameters.blockSize,
                                    _parameters.mode,
                                    _parameters.blockSize);
        _plotBufferZero = _hfPlot->initializeChannelZeroBuffer(
                    _parameters.blockSize);
    }

    _measureThread->setParameters(_parameters, _isWorking);
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
    if ((state == false) &&
            (ui->channelOne_check->isChecked() == false))
    {
        ui->channelZero_check->setChecked(true);
        return;
    }
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
    }
    _channelZeroState(state);
    if (_measureThread != 0)
    {
        _measureThread->setChannelStatus(CHANNEL_0,
                                        _parameters.channelZeroState);
    }
}

void MainWindow::on_channelOne_check_toggled(bool state)
{
    if ((state == false) &&
            (ui->channelZero_check->isChecked() == false))
    {
        ui->channelOne_check->setChecked(true);
        return;
    }
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
    }
    _channelOneState(state);
    if (_measureThread != 0)
    {
        _measureThread->setChannelStatus(CHANNEL_1,
                                        _parameters.channelOneState);
    }
}

void MainWindow::_displayError()
{
    QMessageBox::critical(this,
                          tr("Error"),
                          _measureThread->getLastError(),
                          QMessageBox::Ok);
}

void MainWindow::_channelZeroState(bool state)
{
    _plot->setChannels(ui->channelZero_check->isChecked(),
                       ui->channelOne_check->isChecked());
}

void MainWindow::_channelOneState(bool state)
{
    _plot->setChannels(ui->channelZero_check->isChecked(),
                       ui->channelOne_check->isChecked());;
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
    if (ui->meas_per_second_spin->value() != value)
    {
        ui->meas_per_second_spin->setValue(value);
    }

    if (value == delayedSlider->value())
    {
        return;
    }

    _parameters.mode = MODE_SINGLESHOT_MEASURING;

    QSettings settings("settings.ini", QSettings::IniFormat, this);
    _parameters.measuringInterval = value;
    settings.setValue("measuring_interval", value);

    //points per sec * displayed seconds
    _plot->setDisplayedPoints(value * 10, _parameters.mode, value * 10);

    if (_measureThread != 0)
    {
        _measureThread->setMeasuringInterval(value);
    }
}

void MainWindow::_delayedSliderNewValue_2(int value)
{
    if (ui->meas_block_count_spin->value())
    {
        ui->meas_block_count_spin->setValue(value);
    }

    if (value == delayedSlider_2->value())
    {
        return;
    }

    _parameters.mode = MODE_BLOCK_MEASURING;
    _parameters.measuringInterval = delayedSlider->value();;
    _parameters.blockSize = value;
    _parameters.scaningInterval = 160;
    _parameters.samplingInterval = 160;

    if (value > MAXIMUM_PLOT_SAMPLES)
    {
        _hfPlot->setDisplayedPoints(MAXIMUM_PLOT_SAMPLES,
                                    _parameters.mode,
                                    _parameters.blockSize);
        _plotBufferZero = _hfPlot->initializeChannelZeroBuffer(
                    MAXIMUM_PLOT_SAMPLES);
    }
    else
    {
        _hfPlot->setDisplayedPoints(_parameters.blockSize,
                                    _parameters.mode,
                                    _parameters.blockSize);
        _plotBufferZero = _hfPlot->initializeChannelZeroBuffer(value);
    }

    if (_measureThread != 0)
    {
        _measureThread->setParameters(_parameters, _isWorking);
    }

    QSettings settings("settings.ini", QSettings::IniFormat, this);
    settings.setValue("measuring_samples_count", value);
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

void MainWindow::_tryToStop()
{
    if ((_isSingleshotRunning == true) || (_isBlockRunning == true))
    {
        return;
    }

    _measureThread->stopWorking();
    _isWorking = false;
    _stopLogging();
    ui->log_btn->setChecked(false);
}

void MainWindow::_tryToStart()
{
    if (_isWorking == true)
    {
        return;
    }

        _isWorking = true;
        _measureThread->startWorking();
        _workingTime->restart();
}
