#ifndef REGISTERMODEL_H
#define REGISTERMODEL_H

#include <cstdint>
#include <QAbstractTableModel>
#include <QHash>
#include <QVector>

class QXmlStreamReader;
class QXmlStreamWriter;

class RegisterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static RegisterModel* input();
    static RegisterModel* holding();

    void add(uint16_t plc_addr, int reg_count);
    void remove(uint16_t plc_addr);
    void clear() { _data.clear(); setAddress(0); }

    void fromXml(uint16_t plc_addr, QXmlStreamReader *reader);
    void toXml(uint16_t plc_addr, QXmlStreamWriter *writer) const;

    uint16_t address() const { return _curr_addr; }

    Q_INVOKABLE bool contains(unsigned plc_addr) const { return _data.contains(plc_addr); }
    Q_INVOKABLE int count(unsigned plc_addr) const { return _data.value(plc_addr).count(); }
    Q_INVOKABLE unsigned regData(unsigned plc_addr, unsigned reg_addr) const { return _data.value(plc_addr).at(reg_addr); }
    Q_INVOKABLE void setRegData(unsigned plc_addr, unsigned reg_addr, unsigned value);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

public slots:
    void setAddress(uint16_t new_addr);

private:
    explicit RegisterModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    RegisterModel(RegisterModel const&) = delete;
    RegisterModel& operator= (RegisterModel const&) = delete;

    uint16_t _curr_addr;
    QHash<uint16_t, QVector<uint16_t> > _data;
};

#endif // REGISTERMODEL_H
