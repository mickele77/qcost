#ifndef PROJECTACCOUNTINGDATAMODEL_H
#define PROJECTACCOUNTINGDATAMODEL_H

#include "qcost_export.h"

class MathParser;
class AccountingBillItem;
class PaymentData;
class PaymentDataModelPrivate;

class QXmlStreamWriter;
class QXmlStreamReader;

#include <QAbstractItemModel>

class EXPORT_QCOST_LIB_OPT PaymentDataModel : public QAbstractItemModel {
    Q_OBJECT
public:
    PaymentDataModel(AccountingBillItem *rootItem, MathParser *prs );

    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;

    Qt::ItemFlags flags() const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    PaymentData *paymentData(int pos);

    void updateAmounts();

public slots:
    void insertPayment( int payNum, AccountingBillItem * pay );
    void removePayment( int payNum, AccountingBillItem * pay );

signals:
    void modelChanged();

private:
    PaymentDataModelPrivate * m_d;

    PaymentData *data(const QModelIndex &index) const;
};

#endif // PROJECTACCOUNTINGDATAMODEL_H
