#include "editorsettingdialog.h"
#include "ui_editorsettingdialog.h"

EditorSettingDialog::EditorSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditorSettingDialog)
{
    ui->setupUi(this);
}

EditorSettingDialog::~EditorSettingDialog()
{
    delete ui;
}

bool EditorSettingDialog::isPeriod()
{
    return ui->checkPeriodFlag->isChecked();
}

int EditorSettingDialog::period()
{
    return ui->spinPeriod->value();
}
