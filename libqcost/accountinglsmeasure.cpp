#include "accountinglsmeasure.h"

#include "unitmeasure.h"
#include "mathparser.h"

#include <QTextStream>
#include <QXmlStreamAttributes>
#include <QVariant>
#include <QDate>
#include <QString>

class AccountingLSMeasurePrivate{
public:
    AccountingLSMeasurePrivate(MathParser * p, UnitMeasure * ump):
        parser( p ),
        unitMeasure( ump ),
        comment(""),
        projFormula(""),
        projQuantity(0.0),
        accDate( QDate::currentDate() ),
        accFormula(""),
        accQuantity(0.0),
        accFormulaFromProj(false){
    }
    QString	toString(double i, char f = 'g', int prec = 6) const{
        if( parser != NULL ){
            return parser->toString( i, f, prec );
        } else {
            return QString::number( i, f, prec );
        }
    }
    MathParser * parser;
    UnitMeasure * unitMeasure;
    QString comment;
    QString projFormula;
    double projQuantity;
    QDate accDate;
    QString accFormula;
    double accQuantity;
    bool accFormulaFromProj;
};

AccountingLSMeasure::AccountingLSMeasure(MathParser * p, UnitMeasure * ump) :
    QObject(0),
    m_d( new AccountingLSMeasurePrivate(p, ump)){
}

AccountingLSMeasure::~AccountingLSMeasure(){
    delete m_d;
}

AccountingLSMeasure &AccountingLSMeasure::operator=(const AccountingLSMeasure &cp) {
    if( &cp != this ){
        setUnitMeasure( cp.m_d->unitMeasure );
        setComment( cp.m_d->comment );
        setProjFormula( cp.m_d->projFormula );
        setAccFormula( cp.m_d->accFormula );
        setAccFormulaFromProj( cp.m_d->accFormulaFromProj );
        setAccDate( cp.m_d->accDate );
    }

    return *this;
}

QString AccountingLSMeasure::comment() const{
    return m_d->comment;
}

void AccountingLSMeasure::setComment( const QString & nc ){
    m_d->comment = nc;
}

void AccountingLSMeasure::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        m_d->unitMeasure = ump;
        updateProjQuantity();
        updateAccQuantity();
    }
}

QString AccountingLSMeasure::formula() const{
    return m_d->projFormula;
}

void AccountingLSMeasure::setProjFormula( const QString & nf ){
    if( nf != m_d->projFormula ){
        m_d->projFormula = nf;
        updateProjQuantity();
        if( m_d->accFormulaFromProj ){
            setAccFormula( m_d->projFormula );
        }
    }
}

void AccountingLSMeasure::updateProjQuantity(){
    double v = m_d->parser->evaluate( m_d->projFormula );
    if( m_d->unitMeasure ) {
        v = m_d->unitMeasure->applyPrecision( v );
    }
    if( v != m_d->projQuantity ){
        m_d->projQuantity = v;
        emit projQuantityChanged( v );
    }
}

double AccountingLSMeasure::projQuantity() const{
    double ret = 0.0;
    if( m_d->unitMeasure != NULL ){
        ret = m_d->unitMeasure->applyPrecision( m_d->projQuantity );
    } else {
        ret = m_d->projQuantity;
    }
    return ret;
}

QString AccountingLSMeasure::projQuantityStr() const{
    QString realFormula = m_d->projFormula;
    realFormula.remove(" ");
    if( realFormula.isEmpty() ){
        return QString();
    }
    if( m_d->unitMeasure != NULL ){
        return m_d->toString( projQuantity(), 'f', m_d->unitMeasure->precision() ) ;
    } else {
        return m_d->toString( projQuantity(), 'f', 6 ) ;
    }
}

QString AccountingLSMeasure::accFormula() const{
    return m_d->accFormula;
}

void AccountingLSMeasure::setAccFormula( const QString & nf ){
    if( nf != m_d->accFormula ){
        m_d->accFormula = nf;
        updateAccQuantity();
    }
}

void AccountingLSMeasure::updateAccQuantity(){
    double v = m_d->parser->evaluate( m_d->accFormula );
    if( m_d->unitMeasure ) {
        v = m_d->unitMeasure->applyPrecision( v );
    }
    if( v != m_d->accQuantity ){
        m_d->accQuantity = v;
        emit accQuantityChanged( v );
    }
}

double AccountingLSMeasure::accQuantity() const{
    double ret = 0.0;
    if( m_d->unitMeasure != NULL ){
        ret = m_d->unitMeasure->applyPrecision( m_d->accQuantity );
    } else {
        ret = m_d->accQuantity;
    }
    return ret;
}

QString AccountingLSMeasure::accQuantityStr() const{
    QString realFormula = m_d->accFormula;
    realFormula.remove(" ");
    if( realFormula.isEmpty() ){
        return QString();
    }
    if( m_d->unitMeasure != NULL ){
        return m_d->toString( accQuantity(), 'f', m_d->unitMeasure->precision() ) ;
    } else {
        return m_d->toString( accQuantity(), 'f', 6 ) ;
    }
}

bool AccountingLSMeasure::accFormulaFromProj() const {
    return m_d->accFormulaFromProj;
}

void AccountingLSMeasure::setAccFormulaFromProj(bool newVal) {
    if( newVal != m_d->accFormulaFromProj ){
        m_d->accFormulaFromProj = newVal;
        if( m_d->accFormulaFromProj ){
            setAccFormula( m_d->projFormula );
        }
        emit accFormulaFromProjChanged( m_d->accFormulaFromProj );
    }
}

QDate AccountingLSMeasure::accDate() const {
    return m_d->accDate;
}

QString AccountingLSMeasure::accDateStr() const {
    if( m_d->parser != NULL ){
        return m_d->parser->toString( m_d->accDate, QLocale::NarrowFormat );
    }
    return m_d->accDate.toString();
}

void AccountingLSMeasure::setAccDate(const QDate &newDate) {
    if( m_d->accDate != newDate ){
        m_d->accDate = newDate;
        emit accDateChanged( m_d->accDate );
    }
}

void AccountingLSMeasure::setAccDate(const QString &newDate) {
    setAccDate( m_d->parser->evaluateDate(newDate) );
}

void AccountingLSMeasure::writeXml( QXmlStreamWriter * writer ){
    writer->writeStartElement( "AccountingLSMeasure" );

    writer->writeAttribute( "comment", m_d->comment );

    QString f = m_d->projFormula;
    if( m_d->parser->decimalSeparator() != "." ){
        f.replace( m_d->parser->decimalSeparator(), ".");
    }
    writer->writeAttribute( "projFormula", f );

    writer->writeAttribute( "accDate", QString::number( m_d->accDate.toJulianDay() ) );
    if( m_d->accFormulaFromProj ){
        writer->writeAttribute( "accFormulaFromProj",  "true" );
    } else {
        f = m_d->accFormula;
        if( m_d->parser->decimalSeparator() != "." ){
            f.replace( m_d->parser->decimalSeparator(), ".");
        }
        writer->writeAttribute( "accFormula", f );
    }

    writer->writeEndElement();
}

void AccountingLSMeasure::loadFromXml(const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "comment" ) ){
        setComment(  attrs.value( "comment").toString() );
    }
    if( attrs.hasAttribute( "projFormula" ) ){
        QString f = attrs.value( "projFormula").toString();
        if( m_d->parser->decimalSeparator() != "." ){
            f.replace( ".", m_d->parser->decimalSeparator());
        }
        setProjFormula( f );
    }
    if( attrs.hasAttribute( "accFormula" ) ){
        QString f = attrs.value( "accFormula").toString();
        if( m_d->parser->decimalSeparator() != "." ){
            f.replace( ".", m_d->parser->decimalSeparator());
        }
        setAccFormula( f );
    }
    if( attrs.hasAttribute( "accFormulaFromProj" ) ){
        QString f = attrs.value( "accFormulaFromProj").toString();
        if( f.toUpper() == "TRUE" ){
            setAccFormulaFromProj( true );
        }
    }
    if( attrs.hasAttribute( "accDate" ) ){
        setAccDate( QDate::fromJulianDay( attrs.value( "accDate").toLongLong() ) );
    }
}
