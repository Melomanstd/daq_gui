#include "headers/parametersdialog.h"
#include "ui_parametersdialog.h"

ParametersDialog::ParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParametersDialog)
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
    ui->meas_samples_count_spin->setDisabled(true);
    ui->meas_time_spin->setMinimum(1);
    ui->meas_time_spin->setValue(1);
}

void ParametersDialog::_blockMode()
{
    ui->singleshot_btn->setChecked(false);
    ui->meas_samples_count_spin->setDisabled(false);
    ui->meas_time_spin->setMinimum(160);
    ui->meas_time_spin->setValue(160);
}

qint32 ParametersDialog::getMeasTime()
{
    return ui->meas_time_spin->value();
}

qint32 ParametersDialog::getSamplesCount()
{
    return ui->meas_samples_count_spin->value();
}
