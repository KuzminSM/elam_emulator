#ifndef EDITORSETTINGDIALOG_H
#define EDITORSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class EditorSettingDialog;
}

class EditorSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditorSettingDialog(QWidget *parent = 0);
    ~EditorSettingDialog();

    bool isPeriod();
    int period();

private:
    Ui::EditorSettingDialog *ui;
};

#endif // EDITORSETTINGDIALOG_H
