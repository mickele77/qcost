#ifndef BILLITEMMEASURESMODEL_H
#define BILLITEMMEASURESMODEL_H

#include "qcost_export.h"

class QXmlStreamWriter;
class QXmlStreamReader;
class MathParser;
class BillItemMeasure;
class UnitMeasure;

#include <QAbstractTableModel>

class BillItemMeasuresModelPrivate;

class EXPORT_QCOST_LIB_OPT BillItemMeasuresModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit BillItemMeasuresModel(MathParser *p = NULL, UnitMeasure *ump = NULL, QObject *parent = 0);

    ~BillItemMeasuresModel();

    BillItemMeasuresModel &operator =(const BillItemMeasuresModel &cp);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex())  const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool append( int count = 1 );
    bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex() );

    double quantity();

    void setUnitMeasure( UnitMeasure * ump );

    void writeXml10( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader);

    int billItemMeasureCount();
    BillItemMeasure * measure( int i );

signals:
    void quantityChanged( double );
    void modelChanged();

private slots:
    void updateQuantity();
    void updateAllQuantities();
private:
    BillItemMeasuresModelPrivate * m_d;
};

#endif // BILLITEMMEASURESMODEL_H
