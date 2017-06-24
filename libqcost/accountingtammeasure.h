#ifndef ACCOUNTINGTAMMEASURE_H
#define ACCOUNTINGTAMMEASURE_H

#include "qcost_export.h"

class UnitMeasure;
class MathParser;
class QTextStream;
class QXmlStreamWriter;
class QXmlStreamAttributes;

#include <QObject>

class AccountingTAMMeasurePrivate;

class EXPORT_QCOST_LIB_OPT AccountingTAMMeasure : public QObject
{
    Q_OBJECT
public:
    explicit AccountingTAMMeasure( MathParser *p = NULL, UnitMeasure * ump = NULL);
    ~AccountingTAMMeasure();

    AccountingTAMMeasure &operator =(const AccountingTAMMeasure &cp);

    QString comment() const;
    QString formula(int i) const;
    double quantity() const;
    QString quantityStr() const;

    void setComment(const QString &nc);
    void setUnitMeasure( UnitMeasure * ump );
    void setFormula(int i, const QString &nf);

    void writeXml( QXmlStreamWriter * writer );
    void loadXmlTmp20(const QXmlStreamAttributes &attrs);
    void loadFromXmlTmp20();

    void resizeDays(int newSize);

signals:
    void quantityChanged( double );

private:
    AccountingTAMMeasurePrivate * m_d;
    void updateQuantity();
};

#endif // ACCOUNTINGTAMMEASURE_H
