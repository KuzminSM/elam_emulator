#ifndef BITSMODEL_H
#define BITSMODEL_H

#include <QAbstractTableModel>
#include <QHash>
#include <QBitArray>

class QXmlStreamReader;
class QXmlStreamWriter;

class BitsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    static BitsModel* coil();
    static BitsModel* input();

    void add(uint16_t plc_addr, int bit_count);
    void remove(uint16_t plc_addr);
    void clear() { _data.clear(); setAddress(0); }

    void fromXml(uint16_t plc_addr, QXmlStreamReader *reader);
    void toXml(uint16_t plc_addr, QXmlStreamWriter *writer) const;

    uint16_t address() const { return _curr_addr; }

    Q_INVOKABLE bool contains(unsigned plc_addr) const { return _data.contains(plc_addr); }
    Q_INVOKABLE int count(unsigned plc_addr) const { return _data.value(plc_addr).count(); }
    Q_INVOKABLE bool bitData(unsigned plc_addr, unsigned reg_addr) const { return _data.value(plc_addr).at(reg_addr); }
    Q_INVOKABLE void setBitData(unsigned plc_addr, unsigned reg_addr, bool value);

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
    explicit BitsModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
    BitsModel(BitsModel const&) = delete;
    BitsModel& operator= (BitsModel const&) = delete;

    uint16_t _curr_addr;
    QHash<uint16_t, QBitArray> _data;
};

#endif // BITSMODEL_H
