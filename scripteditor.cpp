#include "scripteditor.h"
#include "ui_scripteditor.h"

#include <QJSEngine>
#include <QJSValue>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QTimer>
#include <QScrollBar>
#include <QMessageBox>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qsciapis.h>

#include "bitsmodel.h"
#include "registermodel.h"
#include "editorsettingdialog.h"

ScriptEditor::ScriptEditor(QString scriptFile, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ScriptEditor)
{
    ui->setupUi(this);

    ui->console->setMaximumBlockCount(100);
    ui->action_Stop->setEnabled(false);

    auto lexer =  new QsciLexerJavaScript( ui->editor );

    auto api = new QsciAPIs( lexer );
    api->add("digital_inputs");
    api->add("coils");
    api->add("input_regs");
    api->add("holding_regs");
    api->add("count");
    api->add("contains");
    api->add("count");
    api->add("contains");
    api->add("bitData");
    api->add("setBitData");
    api->add("regData");
    api->add("setRegData");
    api->prepare();
    lexer->setAPIs(api);

    ui->editor->setLexer( lexer );

    ui->editor->setFont(QFont("Consolas", 12));

    //! Текущая строка кода и ее подсветка
    ui->editor->setCaretLineVisible(true);
    ui->editor->setCaretLineBackgroundColor(QColor("gainsboro"));

    //! Выравнивание
    ui->editor->setAutoIndent(true);
    ui->editor->setIndentationGuides(false);
    ui->editor->setIndentationsUseTabs(true);
    ui->editor->setIndentationWidth(4);

    //! margin это полоска слева, на которой обычно распологаются breakpoints
    ui->editor->setMarginsBackgroundColor(QColor("gainsboro"));
    ui->editor->setMarginLineNumbers(1, true);
    ui->editor->setMarginWidth(1, 25);

    //! Авто-дополнение кода в зависимости от источника
    ui->editor->setAutoCompletionSource(QsciScintilla::AcsAll);
    ui->editor->setAutoCompletionCaseSensitivity(true);
//    ui->editor->setAutoCompletionReplaceWord(true);
//    ui->editor->setAutoCompletionUseSingle(QsciScintilla::AcusAlways);
    ui->editor->setAutoCompletionThreshold(2);

    //! Подсветка соответствий скобок
    ui->editor->setBraceMatching(QsciScintilla::SloppyBraceMatch);
    ui->editor->setMatchedBraceBackgroundColor(Qt::yellow);
    ui->editor->setUnmatchedBraceForegroundColor(Qt::blue);

    _engine = new QJSEngine(this);

    // Подвязываем модели к JavaScript движку
    QJSValue digitalInputsObj = _engine->newQObject( BitsModel::input() );
    _engine->globalObject().setProperty( "digital_inputs", digitalInputsObj );

    QJSValue coilsObj = _engine->newQObject( BitsModel::coil() );
    _engine->globalObject().setProperty( "coils", coilsObj );

    QJSValue inputRegsObj = _engine->newQObject( RegisterModel::input() );
    _engine->globalObject().setProperty( "input_regs", inputRegsObj );

    QJSValue holdingRegsObj = _engine->newQObject( RegisterModel::holding() );
    _engine->globalObject().setProperty( "holding_regs", holdingRegsObj );

    _timer = new QTimer(this);
    _timer->setInterval(1000);
    connect(_timer, &QTimer::timeout, this, &ScriptEditor::run);

    connect(ui->action_Stop, &QAction::triggered, _timer, &QTimer::stop);
    connect(ui->action_Close, &QAction::triggered, this, &ScriptEditor::close);


}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}

bool ScriptEditor::isModifiedSaved()
{
    if (ui->editor->isModified()) {
        int res = QMessageBox::warning(this, "Редактор скрипта",
                     "Скрипт был изменен.\n"
                     "Хотите его сохранить?",
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::No,
                     QMessageBox::Cancel | QMessageBox::Escape);
        if (res == QMessageBox::Yes) {
            return on_action_Save_triggered();
        } else if (res == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

bool ScriptEditor::saveFile(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        QMessageBox::warning(this, tr("Редактор скрипта"),
                             tr("Не могу записать файл %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << ui->editor->text();
    QApplication::restoreOverrideCursor();

    _curr_file = fileName;
    return true;
}

void ScriptEditor::openFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, "Редактор скрипта",
                             tr("Не могу открыть файл %1:\n%2.")
                             .arg(filename)
                             .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    ui->editor->setText(in.readAll());
    QApplication::restoreOverrideCursor();

    _curr_file = filename;
}

void ScriptEditor::on_action_New_triggered()
{
    if(!isModifiedSaved()) {
        return;
    }
    ui->editor->clear();
    _curr_file = QString();
}

void ScriptEditor::on_action_Open_triggered()
{
    if(!isModifiedSaved()) {
        return;
    }

    QString file_name = QFileDialog::getOpenFileName(this,
                            "Открыть скрипт", ".",
                            "JavaScript файлы (*.js)");
    if(file_name.isEmpty())
        return;

    openFile(file_name);
}

bool ScriptEditor::on_action_Save_triggered()
{
    if(_curr_file.isEmpty()) {
        return on_action_SaveAs_triggered();
    }

    return saveFile(_curr_file);
}

bool ScriptEditor::on_action_SaveAs_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this,
                            "Сохранить скрипт", ".",
                            "JavaScript файлы (*.js)");
    if(file_name.isEmpty())
        return false;

    return saveFile(file_name);
}

void ScriptEditor::on_action_Run_triggered()
{
    run();

    if(ui->action_Stop->isEnabled()) {
        _timer->start();
    }
}

void ScriptEditor::on_action_Setting_triggered()
{
    if(_dialog == nullptr) {
        _dialog = new EditorSettingDialog(this);
    }

    if(_dialog->exec()) {
        if(_timer->isActive() && !_dialog->isPeriod()) {
            _timer->stop();
        }

        ui->action_Stop->setEnabled(_dialog->isPeriod());
        _timer->setInterval(_dialog->period() * 1000);
    }
}

void ScriptEditor::run()
{
    QJSValue result = _engine->evaluate(ui->editor->text());

    ui->console->insertPlainText(QString("(%1): %2 \n")
                                 .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                                 .arg(result.toString()));

    ui->console->verticalScrollBar()->setValue(ui->console->verticalScrollBar()->maximum());
}
