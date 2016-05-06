#include "headers/parametersdialog.h"
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
    ui->spin_2->setEnabled(false);
    ui->block_btn->setChecked(false);
    ui->singleshot_btn->setChecked(true);
    ui->spin_0->setMinimum(1);  //measuring per second
    ui->spin_0->setMaximum(100);//measuring per second
    ui->spin_0->setValue(1);
    ui->spin_1->setMinimum(1);  //displayed measuring interval(sec)
    ui->spin_1->setMaximum(100);//displayed measuring interval(sec)
    ui->spin_1->setValue(10);
    _mode = MODE_SINGLESHOT_MEASURING;
    ui->label_3->setText(tr("Displayed interval"));
    ui->label_4->setText(tr("Measuring per second"));
    ui->label_5->setText(tr("Disabled"));
}

void ParametersDialog::_blockMode()
{
    ui->spin_2->setEnabled(true);
    ui->singleshot_btn->setChecked(false);
    ui->block_btn->setChecked(true);
    ui->spin_0->setMinimum(2);          //Samples per measure
    ui->spin_0->setMaximum(16777215);   //Samples per measure
    ui->spin_0->setSingleStep(2);
    ui->spin_0->setValue(10000);

    ui->spin_1->setMinimum(160);        //Scaning interval
    ui->spin_1->setMaximum(16777215);   //Scaning interval
    ui->spin_1->setValue(160);

    ui->spin_2->setMinimum(160);        //Samples interval
    ui->spin_2->setMaximum(65535);      //Samples interval
    ui->spin_2->setValue(160);

    _mode = MODE_BLOCK_MEASURING;
    ui->label_3->setText(tr("Samples per measure"));
    ui->label_4->setText(tr("Scaning interval"));
    ui->label_5->setText(tr("Samples interval"));
}

qint32 ParametersDialog::getMeasuringMode()
{
    return _mode;
}

void ParametersDialog::setDefaultParameters(ModeParameters parameters)
{
    if (parameters.mode == MODE_SINGLESHOT_MEASURING)
    {
        _singleshotMode();
    }
    else if (parameters.mode == MODE_BLOCK_MEASURING)
    {
        _blockMode();
    }
}

qint32 ParametersDialog::getMeasuringsPerSecond()
{
    return ui->spin_0->value();
}

qint32 ParametersDialog::getDisplayedInterval()
{
    return ui->spin_1->value();
}

qint32 ParametersDialog::getSamplesPerMeasuring()
{
    if ((ui->spin_0->value() % 2) != 0)
    {
        return ui->spin_0->value() - 1;
    }
    else
    {
        return ui->spin_0->value();
    }
}

qint32 ParametersDialog::getScaningInterval()
{
    return ui->spin_1->value();
}

qint32 ParametersDialog::getSamplesInterval()
{
    return ui->spin_2->value();
}
