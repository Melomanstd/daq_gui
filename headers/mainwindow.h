#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSettings>

#include "dataoperator.h"
#include "graphicplot.h"

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

    void _updatePlot();

private:
    Ui::MainWindow  *ui;

    DataOperator*   _dataOperator;
    GraphicPlot*    _plot;

    QTimer*         _updateTimer;

    ModeParameters  _parameters;

    bool            _isWorking;

    double* _plotBufferZero;
    double* _plotBufferOne;
};

#endif // MAINWINDOW_H
