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
    explicit AccountingLSMeasure(MathParser *p = NULL, UnitMeasure * ump = NULL);
    ~AccountingLSMeasure();

    AccountingLSMeasure &operator =(const AccountingLSMeasure &cp);

    QString comment() const;
    QString projFormula() const;
    double projQuantity() const;
    QString projQuantityStr() const;
    QString accFormula() const;
    double accQuantity() const;
    QString accQuantityStr() const;
    bool accFormulaFromProj() const;
    QDate accDate() const;
    QString accDateStr() const;

    void setComment(const QString &nc);
    void setUnitMeasure( UnitMeasure * ump );
    void setProjFormula(const QString &nf);
    void setAccFormula(const QString &nf);
    void setAccFormulaFromProj( bool newVal = true );
    void setAccDate( const QDate & newDate );
    void setAccDate( const QString & newDate );

    void writeXml( QXmlStreamWriter * writer );
    void loadFromXml(const QXmlStreamAttributes &attrs);

signals:
    void projQuantityChanged( double );
    void accQuantityChanged( double );
    void accFormulaFromProjChanged( bool );
    void accDateChanged( const QDate & );

private:
    AccountingLSMeasurePrivate * m_d;
    void updateProjQuantity();
    void updateAccQuantity();
};

#endif // ACCOUNTINGLSMEASURE_H
