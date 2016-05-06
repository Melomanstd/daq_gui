#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QLabel>

#include "dataoperator.h"
#include "graphicplot.h"
#include "timerslider.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void singleShot();
    void blocks();

private:
    void            _initializePlot();
    void            _initializeDataOperator();
    bool            _setupParameters();
    ModeParameters  _lastParameters();
    void            _channelZeroState(bool state);
    void            _channelOneState(bool state);

private slots:
    void on_parameters_btn_clicked();
    void on_start_btn_clicked();
    void on_stop_btn_clicked();
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
    void _delayedSliderNewValue(int value);

    void _updatePlot();
    void _displayError();

private:
    Ui::MainWindow  *ui;

    DataOperator*   _dataOperator;
    GraphicPlot*    _plot;

    QTimer*         _updateTimer;

    ModeParameters  _parameters;

    bool            _isWorking;

    double* _plotBufferZero;
    double* _plotBufferOne;

    QLabel *modeLabel;
    QLabel *modeValue;
    QLabel *intervalLabel;
    QLabel *intervalValue;

    TimerSlider *delayedSlider;
};

#endif // MAINWINDOW_H
