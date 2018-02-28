#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QMainWindow>

namespace Ui {
class ScriptEditor;
}

class QJSEngine;
class QTimer;
class EditorSettingDialog;

class ScriptEditor : public QMainWindow
{
    Q_OBJECT

public:
    ScriptEditor(QString scriptFile, QWidget *parent = nullptr);
    ~ScriptEditor();

private slots:
    bool isModifiedSaved();
    bool saveFile(QString fileName);
    void openFile(QString filename);

    void on_action_New_triggered();
    void on_action_Open_triggered();
    bool on_action_Save_triggered();
    bool on_action_SaveAs_triggered();

    void on_action_Run_triggered();
    void on_action_Setting_triggered();

    void run();

private:
    Ui::ScriptEditor *ui;
    QString _curr_file;
    EditorSettingDialog *_dialog = nullptr;
    QJSEngine *_engine;
    QTimer *_timer;
};

#endif // SCRIPTEDITOR_H
