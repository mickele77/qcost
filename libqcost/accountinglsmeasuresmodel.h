#ifndef ACCOUNTINGLSMEASURESMODEL_H
#define ACCOUNTINGLSMEASURESMODEL_H

#include "qcost_export.h"

class QXmlStreamWriter;
class QXmlStreamReader;
class MathParser;
class AccountingLSMeasure;
class UnitMeasure;

#include <QAbstractTableModel>

class AccountingLSMeasuresModelPrivate;

class EXPORT_QCOST_LIB_OPT AccountingLSMeasuresModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static int accDateCol();

    explicit AccountingLSMeasuresModel(MathParser *p = NULL, UnitMeasure *ump = NULL, QObject *parent = 0);

    ~AccountingLSMeasuresModel();

    AccountingLSMeasuresModel &operator =(const AccountingLSMeasuresModel &cp);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex())  const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool append( int count = 1 );
    bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex() );

    double projQuantity();
    double accQuantity();
    double accQuantity( const QDate &dBegin, const QDate &dEnd);

    void setUnitMeasure( UnitMeasure * ump );

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader);

    int measuresCount();
    AccountingLSMeasure * measure( int i );

signals:
    void projQuantityChanged( double );
    void accQuantityChanged( double );
    void modelChanged();

private slots:
    void updateAllProjQuantities();
    void updateProjQuantity();
    void updateAccQuantity();
    void updateAllAccQuantities();
private:
    AccountingLSMeasuresModelPrivate * m_d;
};

#endif // ACCOUNTINGLSMEASURESMODEL_H
