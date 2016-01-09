#ifndef MEASURE_H
#define MEASURE_H

#include "qcost_export.h"

class AccountingBillItem;
class BillItem;
class UnitMeasure;
class MathParser;
class QTextStream;
class QXmlStreamWriter;
class QXmlStreamAttributes;

#include <QObject>

class MeasurePrivate;

class EXPORT_QCOST_LIB_OPT Measure : public QObject {
    Q_OBJECT
public:
    explicit Measure( BillItem * bItem = NULL, MathParser *p = NULL, UnitMeasure * ump = NULL );
    explicit Measure( AccountingBillItem * accBItem = NULL, MathParser *p = NULL, UnitMeasure * ump = NULL );
    ~Measure();

    Measure &operator =(const Measure &cp);

    QString comment() const;
    QString formula() const;
    QString effectiveFormula() const;
    double quantity() const;
    QString quantityStr() const;

    /** Imposta il valore della formula
     * @param connItemFromId se vero i valori tra parentesi [] sono gli id degli item connessi,
              altrimenti sono i progressiveCode */
    void setFormula(const QString &nf, bool connItemFromId = false );
    void setComment(const QString &nc);
    void setUnitMeasure( UnitMeasure * ump );

    void writeXml10(QXmlStreamWriter *writer) const;
    void writeXml20( QXmlStreamWriter * writer ) const;
    void loadFromXmlTmp();
    void loadXmlTmp(const QXmlStreamAttributes &attrs);

    QList<BillItem *> connectedBillItems();
    QList<AccountingBillItem *> connectedAccBillItems();
signals:
    void quantityChanged( double );

private:
    MeasurePrivate * m_d;
    void updateQuantity();
};

#endif // MEASURE_H
