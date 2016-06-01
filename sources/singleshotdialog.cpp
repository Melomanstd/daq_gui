#include "headers/singleshotdialog.h"
#include "ui_singleshotdialog.h"

SingleshotDialog::SingleshotDialog(int defaultParameters[],
                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SingleshotDialog)
{
    ui->setupUi(this);

    setWindowTitle(tr("Single measure parameters"));

    QStringList items;
    for (int i = 0; i < 16; i++)
    {
        items.append(QString::number(i));
    }
    ui->ch1_combo->addItems(items);
    ui->ch2_combo->addItems(items);

    ui->measurings_count_spin->setValue(defaultParameters[0]);
    ui->ch1_combo->setCurrentIndex(defaultParameters[1]);
    ui->ch2_combo->setCurrentIndex(defaultParameters[2]);
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
