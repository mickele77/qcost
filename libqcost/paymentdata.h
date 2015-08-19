#ifndef PAYMENTDATA_H
#define PAYMENTDATA_H

#include "library_common.h"

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

class EXPORT_LIB_OPT PaymentData: public QObject {
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
                                    MathParser * prs );

    ~PaymentData();

    void writeXml( QXmlStreamWriter * writer );
    void readXml( QXmlStreamReader *reader );
    void loadFromXml( const QXmlStreamAttributes &attrs );

    PaymentData * parentData();

    int childrenCount() const;
    PaymentData *child(int number);
    int childNumber() const;
    bool hasChildren() const;
    bool insertPayments( int position, int count=1 );
    bool appendPayments(int count=1);
    bool removePayments(int position, int purpCount=1);

    PaymentData::DataType dataType();
    QString name();
    QString totalAmount();
    void updateAmounts();

    QDate dateBegin() const;
    void setDateBegin(const QDate &newDate);
    void setDateBegin(const QString &newDate);
    QDate dateEnd() const;
    void setDateEnd(const QDate &newDate);
    void setDateEnd(const QString &newDate);

    void addBillItem(AccountingBillItem *billItem);

    void removeBillItem(AccountingBillItem *billItem);

signals:
    void dataChanged();

private slots:
    void removeBillItem();
private:
    PaymentDataPrivate * m_d;
};

#endif // PAYMENTDATA_H
