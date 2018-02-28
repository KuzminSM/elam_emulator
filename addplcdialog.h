#ifndef ADDPLCDIALOG_H
#define ADDPLCDIALOG_H

#include <QDialog>

namespace Ui {
class AddPLCDialog;
}

class AddPLCDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddPLCDialog(QWidget *parent = 0);
    ~AddPLCDialog();

    int address() const;
    void setAddress(int value);
    int inputs() const;
    int holdingRegs() const;
    int inputRegs() const;
    int coils() const;

private:
    Ui::AddPLCDialog *ui;
};

#endif // ADDPLCDIALOG_H
