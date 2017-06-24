#ifndef ACCOUNTINGTAMMEASURESMODEL_H
#define ACCOUNTINGTAMMEASURESMODEL_H

#include "qcost_export.h"

class QXmlStreamWriter;
class QXmlStreamReader;
class MathParser;
class AccountingTAMMeasure;
class UnitMeasure;

#include <QAbstractTableModel>

class AccountingTAMBillItem;
class AccountingTAMMeasuresModelPrivate;

class EXPORT_QCOST_LIB_OPT AccountingTAMMeasuresModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AccountingTAMMeasuresModel( AccountingTAMBillItem * tamBItem, MathParser *p = NULL, UnitMeasure *ump = NULL, QObject *parent = 0);

    ~AccountingTAMMeasuresModel();

    AccountingTAMMeasuresModel &operator =(const AccountingTAMMeasuresModel &cp);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex())  const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool append( int count = 1 );
    bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex() );

    void setUnitMeasure( UnitMeasure * ump );

    void writeXml20( QXmlStreamWriter * writer );
    void readXmlTmp20(QXmlStreamReader *reader);
    void readFromXmlTmp20();

    int measuresCount();
    AccountingTAMMeasure * measure( int i );

    double quantity();

signals:
    void quantityChanged( double );
    void modelChanged();

private slots:
    void updateQuantity();

private:
    AccountingTAMMeasuresModelPrivate * m_d;
};

#endif // ACCOUNTINGTAMMEASURESMODEL_H
