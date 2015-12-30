#ifndef ATTRIBUTESELECTMODEL_H
#define ATTRIBUTESELECTMODEL_H

class Attribute;
class AttributesModel;
class AttributeSelectModelPrivate;

#include <QAbstractTableModel>

class AttributeSelectModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit AttributeSelectModel(AttributesModel * m, QObject *parent = 0);
    ~AttributeSelectModel();

    QList<Attribute *> selectedAttributes();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex())  const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    AttributeSelectModelPrivate * m_d;
};

#endif // ATTRIBUTESELECTMODEL_H
