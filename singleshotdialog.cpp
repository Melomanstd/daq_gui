#include "singleshotdialog.h"
#include "ui_singleshotdialog.h"

SingleshotDialog::SingleshotDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SingleshotDialog)
{
    ui->setupUi(this);

    QStringList items;
    for (int i = 0; i < 16; i++)
    {
        items.append(QString::number(i));
    }
    ui->ch1_combo->addItems(items);
    ui->ch2_combo->addItems(items);

    ui->ch1_combo->setCurrentIndex(0);
    ui->ch2_combo->setCurrentIndex(1);
}

SingleshotDialog::~SingleshotDialog()
{
    delete ui;
}

void SingleshotDialog::selectedPins(int &p1, int &p2)
{
    p1 = ui->ch1_combo->currentIndex();
    p2 = ui->ch2_combo->currentIndex();
}

int SingleshotDialog::getMeasuresCount()
{
   return ui->measurings_count_spin->value();
}
