#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("ELAM emulator");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption f_opt({"f","datafile"}, "The file with PLC registers data.", "datafile");
    QCommandLineOption s_opt({"s","script"}, "Script for PLC register automation.", "script");

    parser.addOption(f_opt);
    parser.addOption(s_opt);

    parser.process(a);

    QString dataFile = parser.value("datafile");
    QString scriptFile = parser.value("script");

    MainWindow w(dataFile, scriptFile);
    w.show();

    return a.exec();
}
