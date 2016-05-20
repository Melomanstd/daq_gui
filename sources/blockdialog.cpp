#include "headers/blockdialog.h"
#include "ui_blockdialog.h"

BlockDialog::BlockDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BlockDialog)
{
    ui->setupUi(this);

    QStringList items;
    for (int i = 0; i < 16; i++)
    {
        items.append(QString::number(i));
    }

    ui->ch3_combo->addItems(items);
    ui->ch3_combo->setCurrentIndex(2);
}

BlockDialog::~BlockDialog()
{
    delete ui;
}

void BlockDialog::selectedPins(int &p1)
{
    p1 = ui->ch3_combo->currentIndex();
}

int BlockDialog::getSamplesCount()
{
    if (ui->samples_count_spin->value() % 2 != 0)
    {
        return ui->samples_count_spin->value() - 1;
    }
    else
    {
        return ui->samples_count_spin->value();
    }
}

