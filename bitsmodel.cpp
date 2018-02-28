#include "bitsmodel.h"
#include <cmath>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

BitsModel* BitsModel::coil()
{
    static BitsModel* _inst = new BitsModel();
    return _inst;
}

BitsModel* BitsModel::input()
{
    static BitsModel* _inst = new BitsModel();
    return _inst;
}

void BitsModel::add(uint16_t plc_addr, int bit_count)
{
    _data.insert(plc_addr, QBitArray(bit_count));
}

void BitsModel::remove(uint16_t plc_addr)
{
    _data.remove(plc_addr);
}

void BitsModel::setAddress(uint16_t new_addr)
{
    if(new_addr != _curr_addr) {
        emit beginResetModel();

        _curr_addr = new_addr;

        emit endResetModel();
    }
}

QVariant BitsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString::number(section, 16).toUpper();

    if (orientation == Qt::Vertical)
        return QString("%1").arg(section * 16, 4, 16, QChar('0')).toUpper();

    return QVariant();
}

bool BitsModel::setHeaderData(int, Qt::Orientation, const QVariant &, int)
{
    return false;
}


int BitsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return int(std::ceil(_data.value(_curr_addr).count() / 16.0));
}

int BitsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 16;
}

QVariant BitsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int i = index.row() * 16 + index.column();

    if (i >= _data.value(_curr_addr).count() || i < 0)
        return QVariant();

    if (role == Qt::CheckStateRole) {
        if (_data.value(_curr_addr).at(i))
            return Qt::Checked;
        else
            return Qt::Unchecked;
    }

    if (role == Qt::DisplayRole) {
        if (_data.value(_curr_addr).at(i))
          return QString("1");
        else
          return QString("0");
    }

    return QVariant();
}

bool BitsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value && role == Qt::CheckStateRole ) {
        _data[_curr_addr][index.row() * 16 + index.column()] = value.toBool();

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

void BitsModel::setBitData(unsigned plc_addr, unsigned reg_addr, bool value)
{
    if(_data.value(plc_addr).at(reg_addr) != value) {

        _data[plc_addr][reg_addr] = value;

        if(plc_addr == _curr_addr) {
            QModelIndex i = index(reg_addr / 16, reg_addr % 16);
            emit dataChanged(i, i);
        }
    }
}

Qt::ItemFlags BitsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
         return Qt::ItemIsEnabled;

    int i = index.row() * 16 + index.column();
    if (i >= _data.value(_curr_addr).count() || i < 0)
         return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void BitsModel::fromXml(uint16_t plc_addr, QXmlStreamReader *reader)
{
    const uint8_t set_bit[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

    reader->readNext();
    if (reader->tokenType() == QXmlStreamReader::Characters) {
        QStringRef string = reader->text();
        QByteArray data = QByteArray::fromBase64(string.toUtf8());

        for(int i = 0; i < data.count(); ++i) {
            for(int b=0; b<8; ++b) {
                _data[plc_addr][i*8+b] = data.at(i) & set_bit[b];
            }
        }
    }
}

void BitsModel::toXml(uint16_t plc_addr, QXmlStreamWriter *writer) const
{
    const uint8_t set_bit[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

    QByteArray bytes;
    bytes.fill(0, std::ceil(_data.value(plc_addr).count()/8.0));

    // Convert from QBitArray to QByteArray
    for(int b = 0; b < _data.value(plc_addr).count(); ++b) {
        int i = b >> 3;
        int j = b & 0x07;
        bytes[i] = bytes.at(i) | (_data.value(plc_addr).at(b) ? set_bit[j] : 0);
    }

    writer->writeCharacters(bytes.toBase64());
}
