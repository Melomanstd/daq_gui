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
    void _initializePlot();
    void _initializeDataOperator();

private slots:
    void on_parameters_btn_clicked();

    void _updatePlot();

private:
    Ui::MainWindow  *ui;

    DataOperator*   _dataOperator;
    GraphicPlot*    _plot;

    QTimer*         _updateTimer;

    ModeParameter   _parameters;
};

#endif // MAINWINDOW_H
