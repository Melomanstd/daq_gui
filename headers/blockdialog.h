#ifndef BLOCKDIALOG_H
#define BLOCKDIALOG_H

#include <QDialog>

namespace Ui {
class BlockDialog;
}

class BlockDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BlockDialog(QWidget *parent = 0);
    ~BlockDialog();

    void selectedPins(int &p1);
    int getSamplesCount();

private:
    Ui::BlockDialog *ui;
};

#endif // BLOCKDIALOG_H
