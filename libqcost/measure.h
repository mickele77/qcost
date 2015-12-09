#ifndef BILLITEMMEASURE_H
#define BILLITEMMEASURE_H

#include "qcost_export.h"

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
    ~Measure();

    Measure &operator =(const Measure &cp);

    QString comment();
    QString formula();
    double quantity();
    QString quantityStr();

    /** Imposta il valore della formula
     * @param connItemFromId se vero i valori tra parentesi [] sono gli id degli item connessi,
              altrimenti sono i progressiveCode */
    void setFormula(const QString &nf, bool connItemFromId = false );
    void setComment(const QString &nc);
    void setUnitMeasure( UnitMeasure * ump );

    void writeXml( QXmlStreamWriter * writer );
    void loadFromXmlTmp();
    void loadXmlTmp(const QXmlStreamAttributes &attrs);

    QList<BillItem *> connectedBillItems();

signals:
    void quantityChanged( double );

private:
    MeasurePrivate * m_d;
    void updateQuantity();
};

#endif // BILLITEMLINE_H
