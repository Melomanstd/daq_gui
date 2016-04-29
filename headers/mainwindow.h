#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "dataoperator.h"

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
    void _updatePlot();

private:
    Ui::MainWindow  *ui;

    DataOperator*   _dataOperator;
    QwtPlot*        _plot;
    QwtPlotCurve*   _curve;

    QTimer*         _updateTimer;

    QList <QPointF> _points;
    int count;
};

#endif // MAINWINDOW_H
