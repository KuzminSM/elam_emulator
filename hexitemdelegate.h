#ifndef HEXITEMDELEGATE_H
#define HEXITEMDELEGATE_H

#include <QStyledItemDelegate>

class HexItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    HexItemDelegate(QWidget *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    QString displayText(const QVariant &value, const QLocale &locale) const override;

    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

private slots:
    void commitAndCloseEditor();
};

#endif // HEXITEMDELEGATE_H
