#include "headers/parametersdialog.h"
#include "headers/defines.h"
#include "ui_parametersdialog.h"

ParametersDialog::ParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParametersDialog),
    _mode(MODE_NO_MEASURING)
{
    ui->setupUi(this);

    ui->singleshot_btn->setChecked(true);
    _singleshotMode();
}

ParametersDialog::~ParametersDialog()
{
    delete ui;
}

void ParametersDialog::on_block_btn_clicked()
{
    _blockMode();
}

void ParametersDialog::on_singleshot_btn_clicked()
{
    _singleshotMode();
}

void ParametersDialog::_singleshotMode()
{
    ui->block_btn->setChecked(false);
    ui->singleshot_btn->setChecked(true);
    ui->meas_samples_count_spin->setMinimum(1);
    ui->meas_time_spin->setMinimum(1);
    ui->meas_time_spin->setValue(1);
    _mode = MODE_SINGLESHOT_MEASURING;
    ui->meas_samples_count_lbl->setText("Displayed interval");
    ui->meas_time_lbl->setText(tr("Measuring per second"));
}

void ParametersDialog::_blockMode()
{
    ui->singleshot_btn->setChecked(false);
    ui->block_btn->setChecked(true);
    ui->meas_samples_count_spin->setMinimum(2);
    ui->meas_time_spin->setMinimum(160);
    ui->meas_time_spin->setValue(160);
    _mode = MODE_BLOCK_MEASURING;
    ui->meas_samples_count_lbl->setText(tr("Samples per block"));
    ui->meas_time_lbl->setText(tr("Measuring interval"));
}

qint32 ParametersDialog::getMeasuringTime()
{
    return ui->meas_time_spin->value();
}

qint32 ParametersDialog::getSamplesCount()
{
    return ui->meas_samples_count_spin->value();
}

qint32 ParametersDialog::getMeasuringMode()
{
    return _mode;
}

bool ParametersDialog::channelZeroState()
{
    return ui->channel_zero_check->isChecked();
}

bool ParametersDialog::channelOneState()
{
    return ui->channel_one_check->isChecked();
}
