#ifndef PARAMETERSDIALOG_H
#define PARAMETERSDIALOG_H

#include <QDialog>
#include "defines.h"

namespace Ui {
class ParametersDialog;
}

class ParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParametersDialog(QWidget *parent = 0);
    ~ParametersDialog();

    qint32 getMeasuringTime();
    qint32 getSamplesCount();
    qint32 getMeasuringMode();

    bool   channelZeroState();
    bool   channelOneState();

    void   setDefaultParameters(ModeParameters parameters);

private:
    void _singleshotMode();
    void _blockMode();

private slots:
    void on_singleshot_btn_clicked();
    void on_block_btn_clicked();

private:
    Ui::ParametersDialog*   ui;

    qint32                  _mode;
};

#endif // PARAMETERSDIALOG_H
