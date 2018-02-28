#include "hexitemdelegate.h"

#include <cstdint>
#include "hexspinbox.h"

QWidget *HexItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                      const QModelIndex &index) const
{
    HexSpinBox *editor = new HexSpinBox(parent);

    connect(editor, &HexSpinBox::editingFinished,
            this, &HexItemDelegate::commitAndCloseEditor);

    return editor;
}

void HexItemDelegate::commitAndCloseEditor()
{
    HexSpinBox *editor = qobject_cast<HexSpinBox *>(sender());

    emit commitData(editor);
    emit closeEditor(editor);
}

QString HexItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
   return QString("%1").arg(value.toInt(), 4, 16, QChar('0')).toUpper();
}

void HexItemDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    HexSpinBox *hexEditor = qobject_cast<HexSpinBox *>(editor);
    hexEditor->setValue(index.data().toInt());
}

void HexItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    HexSpinBox *hexEditor = qobject_cast<HexSpinBox *>(editor);
    model->setData(index, QVariant::fromValue(hexEditor->value()));
}
