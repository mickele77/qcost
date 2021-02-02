#ifndef ACCOUNTINGLSMEASURE_H
#define ACCOUNTINGLSMEASURE_H

#include "qcost_export.h"

class UnitMeasure;
class MathParser;
class QTextStream;
class QXmlStreamWriter;
class QXmlStreamAttributes;

#include <QObject>

class AccountingLSMeasurePrivate;

class EXPORT_QCOST_LIB_OPT AccountingLSMeasure : public QObject
{
    Q_OBJECT
public:
    explicit AccountingLSMeasure(MathParser *p = nullptr, UnitMeasure * ump = nullptr);
    ~AccountingLSMeasure();

    AccountingLSMeasure &operator =(const AccountingLSMeasure &cp);

    QString comment() const;
    QString formula() const;

    void setComment(const QString &nc);
    void setUnitMeasure( UnitMeasure * ump );

    void writeXml( QXmlStreamWriter * writer );
    void loadFromXml(const QXmlStreamAttributes &attrs);

    double projQuantity() const;
    QString projQuantityStr() const;
    QString accFormula() const;
    void setProjFormula(const QString &nf);

    void setAccFormula(const QString &nf);
    double accQuantity() const;
    QString accQuantityStr() const;
    bool accFormulaFromProj() const;
    void setAccFormulaFromProj(bool newVal);
    QDate accDate() const;
    QString accDateStr() const;
    void setAccDate(const QDate &newDate);
    void setAccDate(const QString &newDate);

signals:
    void accQuantityChanged( double );
    void projQuantityChanged( double );
    void accFormulaFromProjChanged( bool );
    void accDateChanged( const QDate & newDate );

private:
    AccountingLSMeasurePrivate * m_d;

    void updateProjQuantity();
    void updateAccQuantity();
};

#endif // ACCOUNTINGLSMEASURE_H
