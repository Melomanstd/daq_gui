#include "headers/parametersdialog.h"
#include "ui_parametersdialog.h"

ParametersDialog::ParametersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParametersDialog),
    _mode(MODE_NO_MEASURING)
{
    ui->setupUi(this);

    for (int i = 0; i < 3; i++)
    {
        _channelsPin[i] = -1;
    }

    QStringList items;
    for (int i = 0; i < 16; i++)
    {
        items.append(QString::number(i));
    }
    ui->ch1_combo->addItems(items);
    ui->ch2_combo->addItems(items);
    ui->ch3_combo->addItems(items);

    ui->ch1_combo->setCurrentIndex(0);
    ui->ch2_combo->setCurrentIndex(1);
    ui->ch3_combo->setCurrentIndex(2);

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

void ParametersDialog::on_hf_btn_clicked()
{
    _hfMode();
}

void ParametersDialog::_singleshotMode()
{
    ui->spin_0->setEnabled(true);
    ui->spin_1->setEnabled(true);
    ui->spin_2->setEnabled(false);

    ui->hf_btn->setChecked(false);
    ui->block_btn->setChecked(false);
    ui->singleshot_btn->setChecked(true);

    ui->spin_0->setMinimum(1);  //measuring per second
    ui->spin_0->setMaximum(100);//measuring per second
    ui->spin_0->setValue(1);

    ui->spin_1->setMinimum(1);  //displayed measuring interval(sec)
    ui->spin_1->setMaximum(100);//displayed measuring interval(sec)
    ui->spin_1->setValue(10);

    ui->label_3->setText(tr("Measuring per second"));
    ui->label_4->setText(tr("Displayed interval"));
    ui->label_5->setText(tr("Disabled"));

    _mode = MODE_SINGLESHOT_MEASURING;
}

void ParametersDialog::_blockMode()
{
    ui->spin_0->setEnabled(true);
    ui->spin_1->setEnabled(true);
    ui->spin_2->setEnabled(true);

    ui->hf_btn->setChecked(false);
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

    ui->label_3->setText(tr("Samples per measure"));
    ui->label_4->setText(tr("Scaning interval"));
    ui->label_5->setText(tr("Samples interval"));

    _mode = MODE_BLOCK_MEASURING;
}

void ParametersDialog::_hfMode()
{
    ui->spin_0->setEnabled(true);
    ui->spin_1->setEnabled(false);
    ui->spin_2->setEnabled(false);

    ui->singleshot_btn->setChecked(false);
    ui->hf_btn->setChecked(true);
    ui->block_btn->setChecked(false);

    ui->spin_0->setMinimum(2);          //Samples per measure
    ui->spin_0->setMaximum(16777215);   //Samples per measure
    ui->spin_0->setSingleStep(2);
    ui->spin_0->setValue(1000);

    ui->spin_1->setMinimum(160);        //Scaning interval
    ui->spin_1->setMaximum(16777215);   //Scaning interval
    ui->spin_1->setValue(160);

    ui->spin_2->setMinimum(160);        //Samples interval
    ui->spin_2->setMaximum(65535);      //Samples interval
    ui->spin_2->setValue(160);

    ui->label_3->setText(tr("Samples per measure"));
    ui->label_4->setText(tr("Scaning interval"));
    ui->label_5->setText(tr("Samples interval"));

    _mode = MODE_HF_MEASURING;
//    _mode = MODE_BLOCK_MEASURING;
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

void ParametersDialog::on_ch1_combo_currentIndexChanged(int index)
{
    _checkChannelsCommutation();
}

void ParametersDialog::on_ch2_combo_currentIndexChanged(int index)
{
    _checkChannelsCommutation();
}

void ParametersDialog::on_ch3_combo_currentIndexChanged(int index)
{
    _checkChannelsCommutation();
}

void ParametersDialog::_checkChannelsCommutation()
{
    QPalette palette = QLabel().palette();

    if ((ui->ch1_combo->currentIndex() == ui->ch2_combo->currentIndex()) ||
            (ui->ch1_combo->currentIndex() == ui->ch3_combo->currentIndex()))
    {
        palette.setColor(QPalette::WindowText, Qt::red);
        ui->attention_lbl_1->setPalette(palette);
        ui->attention_lbl_1->setText(tr("PIN bussy"));
        _channelsPin[0] = -1;
    }
    else
    {
        palette.setColor(QPalette::WindowText, Qt::darkGreen);
        ui->attention_lbl_1->setPalette(palette);
        ui->attention_lbl_1->setText(tr("Ok"));
        _channelsPin[0] = ui->ch1_combo->currentIndex();
    }

    if ((ui->ch2_combo->currentIndex() == ui->ch1_combo->currentIndex()) ||
            (ui->ch2_combo->currentIndex() == ui->ch3_combo->currentIndex()))
    {
        palette.setColor(QPalette::WindowText, Qt::red);
        ui->attention_lbl_2->setPalette(palette);
        ui->attention_lbl_2->setText(tr("PIN bussy"));
        _channelsPin[1] = -1;
    }
    else
    {
        palette.setColor(QPalette::WindowText, Qt::darkGreen);
        ui->attention_lbl_2->setPalette(palette);
        ui->attention_lbl_2->setText(tr("Ok"));
        _channelsPin[1] = ui->ch1_combo->currentIndex();
    }

    if ((ui->ch3_combo->currentIndex() == ui->ch2_combo->currentIndex()) ||
            (ui->ch3_combo->currentIndex() == ui->ch1_combo->currentIndex()))
    {
        palette.setColor(QPalette::WindowText, Qt::red);
        ui->attention_lbl_3->setPalette(palette);
        ui->attention_lbl_3->setText(tr("PIN bussy"));
        _channelsPin[2] = -1;
    }
    else
    {
        palette.setColor(QPalette::WindowText, Qt::darkGreen);
        ui->attention_lbl_3->setPalette(palette);
        ui->attention_lbl_3->setText(tr("Ok"));
        _channelsPin[2] = ui->ch1_combo->currentIndex();
    }
}

void ParametersDialog::getChannelsPin(char *ch)
{
    for (int i = 0; i < 3; i++)
    {
        ch[i] = _channelsPin[i];
    }
}
