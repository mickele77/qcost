#ifndef BILLATTRIBUTESELECTMODEL_H
#define BILLATTRIBUTESELECTMODEL_H

class BillAttribute;
class BillAttributeModel;
class BillAttributeSelectModelPrivate;

#include <QAbstractTableModel>

class BillAttributeSelectModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit BillAttributeSelectModel(BillAttributeModel * m, QObject *parent = 0);
    ~BillAttributeSelectModel();

    QList<BillAttribute *> selectedAttributes();
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex())  const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    BillAttributeSelectModelPrivate * m_d;
};

#endif // BILLATTRIBUTESELECTMODEL_H
