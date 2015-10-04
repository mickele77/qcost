#ifndef PROJECTACCOUNTINGDATAMODEL_H
#define PROJECTACCOUNTINGDATAMODEL_H

#include "qcost_export.h"

class MathParser;
class PaymentData;
class PaymentDataModelPrivate;

class QXmlStreamWriter;
class QXmlStreamReader;

#include <QAbstractItemModel>

class EXPORT_QCOST_LIB_OPT PaymentDataModel : public QAbstractItemModel {
    Q_OBJECT
public:
    PaymentDataModel( MathParser *prs );

    void writeXml( QXmlStreamWriter * writer );
    void readXml( QXmlStreamReader * reader );

    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;

    Qt::ItemFlags flags() const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    int paymentsCount() const;
    PaymentData *paymentData(int pos);

    bool insertPayments( int position, int count=1 );
    bool removePayments(int position, int purpCount=1 );
    bool clear();

    void insertPaymentsRequest(int position, int count=1);
    void removePaymentsRequest(int position, int count=1);

    void changePaymentDateEnd( const QDate & newDate, int position);
    void changePaymentDateBegin( const QDate & newDate, int position);

    void updateAmounts();
signals:
    void insertPaymentsSignal( int position, int count );
    void removePaymentsSignal( int position, int count );
    void modelChanged();

private:
    PaymentDataModelPrivate * m_d;

    PaymentData *data(const QModelIndex &index) const;
};

#endif // PROJECTACCOUNTINGDATAMODEL_H
