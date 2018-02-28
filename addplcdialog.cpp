#include "addplcdialog.h"
#include "ui_addplcdialog.h"

AddPLCDialog::AddPLCDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddPLCDialog)
{
    ui->setupUi(this);
}

AddPLCDialog::~AddPLCDialog()
{
    delete ui;
}

int AddPLCDialog::address() const
{
    return ui->spinAddress->value();
}

void AddPLCDialog::setAddress(int value)
{
    ui->spinAddress->setValue(value);
}

int AddPLCDialog::inputs() const
{
    return ui->spinInputs->value();
}

int AddPLCDialog::holdingRegs() const
{
    return ui->spinHoldingRegs->value();
}

int AddPLCDialog::inputRegs() const
{
    return ui->spinInputRegs->value();
}

int AddPLCDialog::coils() const
{
    return ui->spinCoils->value();
}

