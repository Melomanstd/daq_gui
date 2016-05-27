#ifndef SINGLESHOTDIALOG_H
#define SINGLESHOTDIALOG_H

#include <QDialog>

namespace Ui {
class SingleshotDialog;
}

class SingleshotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SingleshotDialog(int defaultParameters[] ,
                              QWidget *parent = 0);
    ~SingleshotDialog();

    void selectedPins(int &p1, int &p2);
    int getMeasuresCount();

private:
    Ui::SingleshotDialog *ui;
};

#endif // SINGLESHOTDIALOG_H
