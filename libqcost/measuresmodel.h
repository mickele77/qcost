#ifndef MEASURESMODEL_H
#define MEASURESMODEL_H

#include "qcost_export.h"

class QXmlStreamWriter;
class QXmlStreamReader;
class MathParser;
class BillItem;
class AccountingBillItem;
class Measure;
class UnitMeasure;

#include <QAbstractTableModel>

class MeasuresModelPrivate;

class EXPORT_QCOST_LIB_OPT MeasuresModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MeasuresModel(BillItem *bItem, MathParser *p = NULL, UnitMeasure *ump = NULL);
    explicit MeasuresModel(AccountingBillItem *accBItem, MathParser *p = NULL, UnitMeasure *ump = NULL);

    ~MeasuresModel();

    MeasuresModel &operator =(const MeasuresModel &cp);

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

    void writeXml10( QXmlStreamWriter * writer ) const;
    void writeXml20( QXmlStreamWriter * writer ) const;
    void readFromXmlTmp();
    void readXmlTmp(QXmlStreamReader *reader);

    int measuresCount();
    Measure * measure( int i );

    QList<BillItem *> connectedBillItems();
    QList<AccountingBillItem *> connectedAccBillItems();

signals:
    void quantityChanged( double );
    void modelChanged();

private slots:
    void updateQuantity();
    void updateAllQuantities();
private:
    MeasuresModelPrivate * m_d;
};

#endif // MEASURESMODEL_H
