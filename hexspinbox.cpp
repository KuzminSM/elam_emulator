#include "hexspinbox.h"

#include <QRegExpValidator>

HexSpinBox::HexSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    setRange(0, 0xFFFF);
    validator = new QRegExpValidator(QRegExp("[0-9A-Fa-f]{1,4}"), this);
}

QValidator::State HexSpinBox::validate(QString &input, int &pos) const
{
    return validator->validate(input, pos);
}

int HexSpinBox::valueFromText(const QString &text) const
{
    bool ok;
    return text.toInt(&ok, 16);
}

QString HexSpinBox::textFromValue(int val) const
{
    return QString("%1").arg(val, 4, 16, QChar('0')).toUpper();
}
