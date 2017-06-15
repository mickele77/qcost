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
    explicit AccountingTAMMeasure(MathParser *p = NULL, UnitMeasure * ump = NULL);
    ~AccountingTAMMeasure();

    AccountingTAMMeasure &operator =(const AccountingTAMMeasure &cp);

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
    AccountingTAMMeasurePrivate * m_d;
    void updateProjQuantity();
    void updateAccQuantity();
};

#endif // ACCOUNTINGTAMMEASURE_H
