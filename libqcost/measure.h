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
    explicit Measure(BillItem * bItem, MathParser *p = nullptr, UnitMeasure * ump = nullptr );
    explicit Measure( AccountingBillItem * accBItem, MathParser *p = nullptr, UnitMeasure * ump = nullptr );
    ~Measure();

    Measure &operator =(const Measure &cp);

    QString comment() const;
    QString formula() const;
    /** Nel caso di quantità o importi collegati, ne vengono sostituiti i relativi valori */
    QString effectiveFormula() const;
    double quantity() const;
    QString quantityStr() const;

    /** Imposta il valore della formula
     * @param connItemFromId se vero i valori tra parentesi [] sono gli id degli item connessi,
              altrimenti sono i progressiveCode */
    void setFormula(const QString &newFormulaInput, bool connItemFromId = false );
    void setComment(const QString &nc);
    void setUnitMeasure( UnitMeasure * ump );

    void writeXml10(QXmlStreamWriter *writer) const;
    void loadFromXml10(const QXmlStreamAttributes &attrs);
    void writeXml20( QXmlStreamWriter * writer ) const;
    void loadFromXmlTmp20();
    void loadXmlTmp20(const QXmlStreamAttributes &attrs);

    QList<BillItem *> connectedBillItems();
    QList<AccountingBillItem *> connectedAccBillItems();
signals:
    void quantityChanged( double );

private:
    MeasurePrivate * m_d;
    void updateQuantity();
};

#endif // MEASURE_H
