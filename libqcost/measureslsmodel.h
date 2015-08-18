#ifndef MEASURESLSMODEL_H
#define MEASURESLSMODEL_H

#include "library_common.h"

class QXmlStreamWriter;
class QXmlStreamReader;
class MathParser;
class AccountingLSItemMeasure;
class UnitMeasure;

#include <QAbstractTableModel>

class MeasuresLSModelPrivate;

class EXPORT_LIB_OPT MeasuresLSModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    static int accDateCol();

    explicit MeasuresLSModel(MathParser *p = NULL, UnitMeasure *ump = NULL, QObject *parent = 0);

    ~MeasuresLSModel();

    MeasuresLSModel &operator =(const MeasuresLSModel &cp);

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
    AccountingLSItemMeasure * measure( int i );

signals:
    void projQuantityChanged( double );
    void accQuantityChanged( double );
    void modelChanged();

private slots:
    void updateProjQuantity();
    void updateAllProjQuantities();
    void updateAccQuantity();
    void updateAllAccQuantities();
private:
    MeasuresLSModelPrivate * m_d;
};

#endif // MEASURESLSMODEL_H
