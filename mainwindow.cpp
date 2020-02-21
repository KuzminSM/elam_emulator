#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QScrollBar>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QXmlStreamWriter>

#include "addplcdialog.h"
#include "bitsmodel.h"
#include "registermodel.h"
#include "hexitemdelegate.h"
#include "settingsdialog.h"
#include "scripteditor.h"

MainWindow::MainWindow(QString dataFile, QString scriptFile, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _status(new QLabel(this)),
    _script_file(scriptFile)

{
    ui->setupUi(this);

    ui->console->setMaximumBlockCount(100);

    statusBar()->addWidget(_status);
    _status->setText("Выберите COM-порт");

    // Связывание моделей с виджетами
    ui->tableInputs->setModel(BitsModel::input());

    ui->tableHoldingRegs->setModel(RegisterModel::holding());
    ui->tableHoldingRegs->setItemDelegate(new HexItemDelegate(this));

    ui->tableInputRegs->setModel(RegisterModel::input());
    ui->tableInputRegs->setItemDelegate(new HexItemDelegate(this));

    ui->tableCoils->setModel(BitsModel::coil());

    // Минимизируем размеры столбцов и строк
    ui->tableInputs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableInputs->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableHoldingRegs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableHoldingRegs->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableInputRegs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableInputRegs->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableCoils->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableCoils->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // COM-порт
    port = new QSerialPort(this);
    connect(port, &QSerialPort::readyRead, this, &MainWindow::readData);

    // Таймер для межбайтовой паузы порта
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &MainWindow::commDataReady);

    _editor = new ScriptEditor(scriptFile, this);

    connect(ui->action_Script, &QAction::triggered, _editor, &ScriptEditor::show);
    connect(ui->action_Quit, &QAction::triggered, qApp, &QApplication::exit);

    if(!dataFile.isEmpty()) {
        openFile(dataFile);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::isCorrectAddress(ModbusLine *req) const
{
    // Проверяем корректное ли число команд в пакете
    if(req->size == 0 || req->size > ELAM_MAX_COMMANDS)
        return false;

    // Проверяем наличие адреса в модели
    if(!RegisterModel::holding()->contains(req->cmds[0].address))
        return false;

    if(req->cmds[0].address > 247) {
        // Если пакет ELAM, все команды должны быть к одному адресу
        for(int i = 1; i < req->size; ++i) {
            if(req->cmds[0].address != req->cmds[i].address) {
                return false;
            }
        }
        return true;

    } else {
        // Если Modbus, тогда только одна команда в пакете
        if(req->size == 1)
            return true;
    }

    return false;
}

void MainWindow::changePLC()
{
    if(ui->listPLC->currentRow() != -1) {

        uint16_t new_addr = ui->listPLC->currentIndex().data().toInt();

        BitsModel::input()->setAddress(new_addr);
        RegisterModel::holding()->setAddress(new_addr);
        RegisterModel::input()->setAddress(new_addr);
        BitsModel::coil()->setAddress(new_addr);
    } else {
        BitsModel::input()->setAddress(0);
        RegisterModel::holding()->setAddress(0);
        RegisterModel::input()->setAddress(0);
        BitsModel::coil()->setAddress(0);
    }
}

void MainWindow::readData()
{
    QByteArray new_data = port->readAll();

    printToConsole(new_data.toHex(' ').toUpper(), true);

    input += new_data;

    if(input.size() > 180) {
        input.clear();
    }

    timer->start(_interval); // Межбайтовый интервал
}

void MainWindow::commDataReady()
{
    // Межбайтовый интервал истек
    CircularBuffer buff;
    buff.data = (uint8_t*)(input.data());
    buff.len = input.count();

    CircularIterator iter;
    iter.buff = &buff;
    iter.pos = 0;

    ModbusLine req;

    if (0 == ParseRTURequest(&iter, input.count(), &req)) {
        if(isCorrectAddress(&req)) {
            QByteArray output;
            output.fill(0, 1024);
            uint16_t len = ProcessRTURequest(&req, (uint8_t*)output.data(), output.count());
            if (len > 0) {
                output.resize(len);
                port->write(output);

                printToConsole(output.toHex(' ').toUpper(), false);
            }
        }
    }

    // Очистка под новый пакет
    input.clear();
}

void MainWindow::printToConsole(QString text, bool in)
{
    QDateTime now = QDateTime::currentDateTime();

    if(in) {
        ui->console->insertPlainText(QString("Принял(%1) <= %2 \n").arg(now.toString("hh:mm:ss.zzz")).arg(text));
    } else {
        ui->console->insertPlainText(QString("Отправил(%1) => %2 \n").arg(now.toString("hh:mm:ss.zzz")).arg(text));
    }

    ui->console->verticalScrollBar()->setValue(ui->console->verticalScrollBar()->maximum());
}

void MainWindow::openPortShowStatus()
{
    if(port->open(QIODevice::ReadWrite)) {
        QString parity;
        switch(port->parity()) {
        case QSerialPort::NoParity:
            parity = QString("None");
            break;
        case QSerialPort::EvenParity:
            parity = QString("Even");
            break;
        case QSerialPort::OddParity:
            parity = QString("Odd");
            break;
        case QSerialPort::MarkParity:
            parity = QString("Mark");
            break;
        case QSerialPort::SpaceParity:
            parity = QString("Space");
            break;
        }

        QString stopBits;
        switch (port->stopBits()) {
        case QSerialPort::OneStop:
            stopBits = "1";
            break;
#ifdef Q_OS_WIN
        case QSerialPort::OneAndHalfStop:
            stopBits = "1.5";
            break;
#endif
        case QSerialPort::TwoStop:
            stopBits = "2";
            break;
        }

        QString flowControl;
        switch (port->flowControl()) {
        case QSerialPort::NoFlowControl:
            flowControl = "None";
            break;
        case QSerialPort::HardwareControl:
            flowControl = "RTS/CTS";
            break;
        case QSerialPort::SoftwareControl:
            flowControl = "XON/XOFF";
            break;
        }

        _status->setText(tr("Открыт порт %1 : %2, %3, %4, %5, %6")
                         .arg(port->portName())
                         .arg(QString::number(port->baudRate()))
                         .arg(QString::number(port->dataBits()))
                         .arg(parity)
                         .arg(stopBits)
                         .arg(flowControl));
    } else {
        _status->setText(QString("Ошибка: %1").arg(port->errorString()));
    }
}

void MainWindow::saveFile(QString filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Эмуляция контроллера ELAM",
                             tr("Не могу записать файл %1:\n%2.")
                             .arg(filename)
                             .arg(file.errorString()));
        return;
    }

    QXmlStreamWriter writer(&file);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("elam-emulator-data");

    if(port->isOpen()) {
        writer.writeAttribute("port-name", port->portName());
        writer.writeAttribute("baud-rate", QString::number(port->baudRate()));
        writer.writeAttribute("data-bits", QString::number(port->dataBits()));
        writer.writeAttribute("parity", QString::number(port->parity()));
        writer.writeAttribute("stop-bits", QString::number(port->stopBits()));
        writer.writeAttribute("flow-control", QString::number(port->flowControl()));
        writer.writeAttribute("interbyte", QString::number(_interval));
    }

    for(int i = 0; i < ui->listPLC->count(); ++i) {
        const QListWidgetItem* item = ui->listPLC->item(i);

        int plc_addr = item->data(Qt::DisplayRole).toInt();
        writer.writeStartElement("plc-data");
        writer.writeAttribute("address", QString::number(plc_addr));

        writer.writeStartElement("digital-inputs");
        writer.writeAttribute("count", QString::number(BitsModel::input()->count(plc_addr)));
        BitsModel::input()->toXml(plc_addr, &writer);
        writer.writeEndElement();

        writer.writeStartElement("input-regs");
        writer.writeAttribute("count", QString::number(RegisterModel::input()->count(plc_addr)));
        RegisterModel::input()->toXml(plc_addr, &writer);
        writer.writeEndElement();

        writer.writeStartElement("holding-regs");
        writer.writeAttribute("count", QString::number(RegisterModel::holding()->count(plc_addr)));
        RegisterModel::holding()->toXml(plc_addr, &writer);
        writer.writeEndElement();

        writer.writeStartElement("coils");
        writer.writeAttribute("count", QString::number(BitsModel::coil()->count(plc_addr)));
        BitsModel::coil()->toXml(plc_addr, &writer);
        writer.writeEndElement();

        writer.writeEndElement();
    }

    writer.writeEndDocument();

    file.close();
    _curr_file = filename;
}

void MainWindow::openFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, "Эмуляция контроллера ELAM",
                             tr("Не могу открыть файл %1:\n%2.")
                             .arg(filename)
                             .arg(file.errorString()));
        return;
    }

    QXmlStreamReader reader(&file);

    uint16_t plc_addr;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            if (reader.name() == "elam-emulator-data") {
                QStringRef com_port = reader.attributes().value("port-name");
                if(!com_port.isEmpty()) {
                    port->close();

                    port->setPortName(com_port.toString());
                    port->setBaudRate(reader.attributes().value("baud-rate").toInt());
                    port->setDataBits(static_cast<QSerialPort::DataBits>(reader.attributes().value("data-bits").toInt()));
                    port->setParity(static_cast<QSerialPort::Parity>(reader.attributes().value("parity").toInt()));
                    port->setStopBits(static_cast<QSerialPort::StopBits>(reader.attributes().value("stop-bits").toInt()));
                    port->setFlowControl(static_cast<QSerialPort::FlowControl>(reader.attributes().value("flow-control").toInt()));

                    _interval = reader.attributes().value("interbyte").toInt();
                    openPortShowStatus();
                }

            } else if (reader.name() == "plc-data") {
                plc_addr = reader.attributes().value("address").toInt();

                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setData(Qt::DisplayRole, plc_addr);
                ui->listPLC->addItem(newItem);

            } else if (reader.name() == "digital-inputs") {
                int count = reader.attributes().value("count").toInt();
                BitsModel::input()->add(plc_addr, count);
                BitsModel::input()->fromXml(plc_addr, &reader);

            } else if (reader.name() == "input-regs") {
                int count = reader.attributes().value("count").toInt();
                RegisterModel::input()->add(plc_addr, count);
                RegisterModel::input()->fromXml(plc_addr, &reader);

            } else if (reader.name() == "holding-regs") {
                int count = reader.attributes().value("count").toInt();
                RegisterModel::holding()->add(plc_addr, count);
                RegisterModel::holding()->fromXml(plc_addr, &reader);

            } else if (reader.name() == "coils") {
                int count = reader.attributes().value("count").toInt();
                BitsModel::coil()->add(plc_addr, count);
                BitsModel::coil()->fromXml(plc_addr, &reader);

            }
        }
    }

    if(ui->listPLC->count() > 0) {
        ui->listPLC->setCurrentRow(0);
    }

    file.close();
    _curr_file = filename;
}

void MainWindow::on_action_New_triggered()
{    
    ui->listPLC->clear();
    BitsModel::input()->clear();
    BitsModel::coil()->clear();
    RegisterModel::input()->clear();
    RegisterModel::holding()->clear();
    _curr_file =  QString();
}

void MainWindow::on_action_Open_triggered()
{
    on_action_New_triggered();

    QString filename = QFileDialog::getOpenFileName(this,
                            "Открыть данные", ".",
                            "Xml файлы (*.xml)");
    if(filename.isEmpty())
        return;

    openFile(filename);
}

void MainWindow::on_action_Save_triggered()
{
    if(_curr_file.isEmpty()) {
        on_action_SaveAs_triggered();
    } else {
        saveFile(_curr_file);
    }
}

void MainWindow::on_action_SaveAs_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                            "Сохранить данные", ".",
                            "Xml файлы (*.xml)");
    if(!filename.isEmpty()) {
        saveFile(filename);
    }
}

void MainWindow::on_action_Add_triggered()
{
    AddPLCDialog dialog;

    if(ui->listPLC->count() > 0) {
        int row = ui->listPLC->count() - 1;
        int max_addr = ui->listPLC->item(row)->data(Qt::DisplayRole).toInt();
        dialog.setAddress(max_addr + 1);
    }

    if(dialog.exec()) {
        uint16_t new_addr = dialog.address();

        if(RegisterModel::holding()->contains(new_addr)) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(this, "Ошибка", "Контроллер с таким адресом уже есть в списке");

        } else {
            BitsModel::input()->add(new_addr, dialog.inputs());
            RegisterModel::holding()->add(new_addr, dialog.holdingRegs());
            RegisterModel::input()->add(new_addr, dialog.inputRegs());
            BitsModel::coil()->add(new_addr, dialog.coils());

            QListWidgetItem *newItem = new QListWidgetItem;
            newItem->setData(Qt::DisplayRole, new_addr);
            ui->listPLC->addItem(newItem);

            if(ui->listPLC->count() == 1) {
                ui->listPLC->setCurrentRow(0);
            }
        }
    }
}

void MainWindow::on_action_Remove_triggered()
{
    int row = ui->listPLC->currentRow();

    if(row != -1) {

        uint16_t plc_addr = ui->listPLC->currentIndex().data().toInt();

        BitsModel::input()->remove(plc_addr);
        RegisterModel::holding()->remove(plc_addr);
        RegisterModel::input()->remove(plc_addr);
        BitsModel::coil()->remove(plc_addr);

        delete ui->listPLC->currentItem();

        if(row > 0) {
            ui->listPLC->setCurrentRow(row - 1);
        } else {
            if(ui->listPLC->count() > 0) {
                ui->listPLC->setCurrentRow(0);
            }
        }
    }
}

void MainWindow::on_action_Settings_triggered()
{
    if(_dialog == nullptr) {
        _dialog = new SettingsDialog(this);
    }

    if(_dialog->exec()) {
        port->close();

        auto comm_setting = _dialog->settings();

        port->setPortName(comm_setting.name);
        port->setBaudRate(comm_setting.baudRate);
        port->setDataBits(comm_setting.dataBits);
        port->setParity(comm_setting.parity);
        port->setStopBits(comm_setting.stopBits);
        port->setFlowControl(comm_setting.flowControl);

        _interval = comm_setting.interByteInterval;

        openPortShowStatus();
    }
}
