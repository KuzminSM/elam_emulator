#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include "modbus.h"

namespace Ui {
class MainWindow;
}

class BitsModel;
class RegisterModel;
class QSerialPort;
class QTimer;
class QLabel;
class ScriptEditor;
class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString dataFile, QString scriptFile, QWidget *parent = 0);
    ~MainWindow();

    bool isCorrectAddress(ModbusLine *req) const;

private slots:
    void changePLC();
    void readData();
    void commDataReady();
    void printToConsole(QString text, bool in);
    void openPortShowStatus();

    void saveFile(QString filename);
    void openFile(QString filename);

    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_SaveAs_triggered();

    void on_action_Add_triggered();
    void on_action_Remove_triggered();
    void on_action_Settings_triggered();

private:
    Ui::MainWindow *ui;

    SettingsDialog *_dialog = nullptr;
    QLabel *_status;
    QSerialPort *port;
    QTimer *timer;
    QByteArray input;
    qint32 _interval;
    ScriptEditor *_editor;
    QString _script_file;
    QString _curr_file;
};

#endif // MAINWINDOW_H
