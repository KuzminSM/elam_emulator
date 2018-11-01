#include "registermodel.h"
#include <cmath>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

RegisterModel* RegisterModel::input()
{
    static RegisterModel* _inst = new RegisterModel();
    return _inst;
}

RegisterModel* RegisterModel::holding()
{
    static RegisterModel* _inst = new RegisterModel();
    return _inst;
}

void RegisterModel::add(uint16_t plc_addr, int reg_count)
{
    _data.insert(plc_addr, QVector<uint16_t>(reg_count));
}

void RegisterModel::remove(uint16_t plc_addr)
{
    _data.remove(plc_addr);
}

void RegisterModel::setAddress(uint16_t new_addr)
{
    if(new_addr != _curr_addr) {
        emit beginResetModel();

        _curr_addr = new_addr;

        emit endResetModel();
    }
}

QVariant RegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString::number(section, 16).toUpper();

    if (orientation == Qt::Vertical)
        return QString("%1").arg(section * 16, 4, 16, QChar('0')).toUpper();

    return QVariant();
}

bool RegisterModel::setHeaderData(int, Qt::Orientation, const QVariant &, int)
{
    return false;
}

int RegisterModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return int(std::ceil(_data.value(_curr_addr).count() / 16.0));
}

int RegisterModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 16;
}

QVariant RegisterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int i = index.row() * 16 + index.column();

    if (i >= _data.value(_curr_addr).count() || i < 0)
        return QVariant();

    if (role == Qt::DisplayRole)
        return _data.value(_curr_addr).at(i);

    return QVariant();
}

bool RegisterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{    
    if (data(index, role) != value && role == Qt::EditRole) {
        _data[_curr_addr][index.row() * 16 + index.column()] = value.toInt();

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

void RegisterModel::setRegData(unsigned plc_addr, unsigned reg_addr, unsigned value)
{
    if(_data.value(plc_addr).at(reg_addr) != value) {

        _data[plc_addr][reg_addr] = value;

        if(plc_addr == _curr_addr) {
            QModelIndex i = index(reg_addr / 16, reg_addr % 16);
            emit dataChanged(i, i);
        }
    }
}

Qt::ItemFlags RegisterModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
         return Qt::ItemIsEnabled;

    int i = index.row() * 16 + index.column();
    if (i >= _data.value(_curr_addr).count() || i < 0)
         return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

void RegisterModel::fromXml(uint16_t plc_addr, QXmlStreamReader *reader)
{
    reader->readNext();
    if (reader->tokenType() == QXmlStreamReader::Characters) {
        QByteArray data = QByteArray::fromBase64(reader->text().string()->toLatin1());

        _data[plc_addr].clear();
        for(int i = 0; i < data.count(); i+=2) {
            _data[plc_addr].push_back( ((uint16_t)data.at(i) << 8) | (data.at(i+1) & 0x00ffu) );
        }
    }
}

void RegisterModel::toXml(uint16_t plc_addr, QXmlStreamWriter *writer) const
{
    QByteArray bytes;

    // Convert from QBitArray to QByteArray
    for(auto i = 0; i < _data.value(plc_addr).count(); ++i) {
        bytes.push_back(_data.value(plc_addr).at(i) >> 8);
        bytes.push_back(_data.value(plc_addr).at(i) & 0xff);
    }

    writer->writeCharacters(QString::fromLatin1(bytes.toBase64()));
}
