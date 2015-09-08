#ifndef BILLITEMMEASURE_H
#define BILLITEMMEASURE_H

#include "qcost_export.h"

class UnitMeasure;
class MathParser;
class QTextStream;
class QXmlStreamWriter;
class QXmlStreamAttributes;

#include <QObject>

class BillItemMeasurePrivate;

class EXPORT_QCOST_LIB_OPT BillItemMeasure : public QObject
{
    Q_OBJECT
public:
    explicit BillItemMeasure(MathParser *p = NULL, UnitMeasure * ump = NULL);
    ~BillItemMeasure();

    BillItemMeasure &operator =(const BillItemMeasure &cp);

    QString comment();
    QString formula();
    double quantity();
    QString quantityStr();

    void setFormula(const QString &nf);
    void setComment(const QString &nc);
    void setUnitMeasure( UnitMeasure * ump );

    void writeXml( QXmlStreamWriter * writer );
    void loadFromXml(const QXmlStreamAttributes &attrs);

signals:
    void quantityChanged( double );

private:
    BillItemMeasurePrivate * m_d;
    void updateQuantity();
};

#endif // BILLITEMLINE_H
