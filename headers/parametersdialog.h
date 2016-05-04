#ifndef PARAMETERSDIALOG_H
#define PARAMETERSDIALOG_H

#include <QDialog>

namespace Ui {
class ParametersDialog;
}

class ParametersDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParametersDialog(QWidget *parent = 0);
    ~ParametersDialog();

    qint32 getMeasTime();
    qint32 getSamplesCount();

private:
    void _singleshotMode();
    void _blockMode();

private slots:
    void on_singleshot_btn_clicked();
    void on_block_btn_clicked();

private:
    Ui::ParametersDialog *ui;
};

#endif // PARAMETERSDIALOG_H
