#ifndef PAYMENTDATA_H
#define PAYMENTDATA_H

#include "qcost_export.h"

class AccountingBillItem;
class MathParser;

class QXmlStreamAttributes;
class QXmlStreamReader;
class QXmlStreamWriter;
class QString;

#include <QObject>

class PaymentDataPrivate;

/**
* @class PaymentData
*
* @brief Classe usata per memorizzare le informazioni sui S.A.L.
*
* Questa classe viene impiegata per memorizzare le informazioni relative ai S.A.L.
* (Stato Avanzamento lavori).
* Contiene i riferimenti alle relative voci del libretto delle misure.
*
* @author Michele Mocciola
*
*/

class EXPORT_QCOST_LIB_OPT PaymentData: public QObject {
    Q_OBJECT
public:
    enum DataType {
        Root,
        Payment,
        PPU,
        LumpSum,
        TimeAndMaterials
    };

    explicit PaymentData( PaymentData * parent,
                          PaymentData::DataType dType,
                          AccountingBillItem *pay,
                          MathParser * prs );

    ~PaymentData();

    PaymentData * parentData();

    int childrenCount() const;
    PaymentData *child(int number);
    int childNumber() const;
    bool hasChildren() const;
    bool insertPayment( int position, AccountingBillItem * pay );
    bool removePayment(int position, AccountingBillItem * pay );

    PaymentData::DataType dataType();
    QString name();
    QString totalAmount();
    void updateAmounts();

    QDate dateBegin() const;
    QDate dateEnd() const;

    AccountingBillItem * associatedPayment();
    void setAssociatedPayment(AccountingBillItem *newPayment);

signals:
    void dataChanged();

private slots:
    void removeBillItem();

private:
    PaymentDataPrivate * m_d;
};

#endif // PAYMENTDATA_H
