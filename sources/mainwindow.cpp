#include "headers/mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#include <D2kDask.h>
#include <DAQHeader.h>

#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _initializePlot();
    _initializeDataOperator();
    _updateTimer = new QTimer;
    _updateTimer->setInterval(500);
    _updateTimer->start();
//    singleShot();
//    blocks();

}

MainWindow::~MainWindow()
{
    delete _updateTimer;
    delete _dataOperator;

    _updateTimer = 0;
    _dataOperator = 0;

    delete ui;
}

void MainWindow::_initializePlot()
{
    _plot = new QwtPlot();
    _plot->setAxisTitle(QwtPlot::yLeft, "Y");
    _plot->setAxisTitle(QwtPlot::xBottom, "X");
    _plot->setCanvasBackground(Qt::white);

    _plot->insertLegend(new QwtLegend);

    QwtPlotGrid *g = new QwtPlotGrid();
    g->setMajPen(QPen(Qt::gray, 2));
    g->attach(_plot);

    _curve = new QwtPlotCurve();
    _curve->setTitle(tr("Channel 0"));
    _curve->setPen(QPen(Qt::blue, 6));
    _curve->setRenderHint(QwtPlotItem::RenderAntialiased, true);

    QwtSymbol *simba = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::yellow), QPen(Qt::red), QSize(8,8));
    _curve->setSymbol(simba);

    _curve->attach(_plot);

    QwtPlotZoomer *zoomer = new QwtPlotZoomer(_plot->canvas());
    zoomer->setTrackerMode(QwtPlotZoomer::AlwaysOff);

//    QwtPlotMagnifier *magna = new QwtPlotMagnifier(p->canvas());
//    magna->setMouseButton(Qt::RightButton);

//    QwtPlotPanner *pana = new QwtPlotPanner(p->canvas());
//    pana->setMouseButton(Qt::LeftButt11on);

    setCentralWidget(_plot);
}

void MainWindow::_initializeDataOperator()
{
    _dataOperator = new DataOperator();
    _dataOperator->setWorkingMode(DataOperator::MODE_SINGLESHOT_MEASURING);
    _dataOperator->setChannelStatus(DataOperator::CHANNEL_0,
                                    DataOperator::ON);
    _dataOperator->setChannelStatus(DataOperator::CHANNEL_1,
                                    DataOperator::OFF);
    _dataOperator->setMeasuringInterval(1000);
    _dataOperator->setMeasureSampleInterval(160);
    _dataOperator->setSampleCount(1000);
    _dataOperator->startWorking();
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
    #define ScanIntrv     160
    #define SampleIntrv   160

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
