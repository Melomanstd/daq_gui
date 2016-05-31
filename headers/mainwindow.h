#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QLabel>
#include <QDir>
#include <QFileDialog>
#include <QTime>

#include "measurethread.h"
#include "graphicplot.h"
#include "timerslider.h"

class BlockDialog;
class SingleshotDialog;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void            _initializePlot();
    void            _initializeDataOperator();
    void            _channelZeroState(bool state);
    void            _channelOneState(bool state);
    void            _stopLogging();
    void            _startLogging();
    void            _setupSingleshotParameters(SingleshotDialog &p);
    void            _setupBlockParameters(BlockDialog &p);
    void            _tryToStop();
    void            _tryToStart();
    void            _writeLog();
    void            _readBlockData();
    void            _readSingleshotData();

private slots:
    void on_parameters_btn_clicked();
    void on_start_btn_clicked();
    void on_stop_btn_clicked();
    void on_start_btn_2_clicked();
    void on_stop_btn_2_clicked();
    void on_parameters_btn_2_clicked();
    void on_ch_0_voltage_range_slider_valueChanged(int value);
    void on_ch_1_voltage_range_slider_valueChanged(int value);
    void on_ch_0_zoom_in_btn_clicked();
    void on_ch_1_zoom_in_btn_clicked();
    void on_ch_0_zoom_out_btn_clicked();
    void on_ch_1_zoom_out_btn_clicked();
    void on_channelZero_check_toggled(bool state);
    void on_channelOne_check_toggled(bool state);
    void on_forward_btn_clicked();
    void on_backward_btn_clicked();
    void on_forward_btn_2_clicked();
    void on_backward_btn_2_clicked();
    void on_screenshot_btn_clicked();
    void on_log_btn_clicked();
    void _delayedSliderNewValue(int value);
    void _delayedSliderNewValue_2(int value);

    void on_meas_per_second_spin_valueChanged(int value);
    void on_meas_block_count_spin_valueChanged(int value);

    void _updatePlot();
    void _displayError();

private:
    Ui::MainWindow  *ui;

    MeasureThread*   _measureThread;
    GraphicPlot*    _plot;
    GraphicPlot*    _hfPlot;

    ModeParameters  _parameters;

    bool            _isWorking;
    bool            _isLogging;

    double*         _plotBufferZero;
    double*         _plotBufferOne;

    TimerSlider*    delayedSlider;
    TimerSlider*    delayedSlider_2;

    QFile           _logFile;
    QTime*          _workingTime;

    bool            _isBlockRunning;
    bool            _isSingleshotRunning;

    double          _value0, _value1, _value2;
    QString         _logString;
    quint32         _lastInterval;
};

#endif // MAINWINDOW_H
