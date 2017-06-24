#include "accountingtammeasure.h"

#include "unitmeasure.h"
#include "mathparser.h"

#include <QTextStream>
#include <QXmlStreamAttributes>
#include <QVariant>
#include <QDate>
#include <QString>

class AccountingTAMMeasurePrivate{
public:
    AccountingTAMMeasurePrivate(MathParser * p, UnitMeasure * ump):
        parser( p ),
        unitMeasure( ump ),
        comment(""),
        quantity(0.0) {
        formula << QString();
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
    QVector<QString> formula;
    double quantity;
    QXmlStreamAttributes tmpAttrs;
};

AccountingTAMMeasure::AccountingTAMMeasure( MathParser * p, UnitMeasure * ump) :
    QObject(0),
    m_d( new AccountingTAMMeasurePrivate(p, ump)){
}

AccountingTAMMeasure::~AccountingTAMMeasure(){
    delete m_d;
}

AccountingTAMMeasure &AccountingTAMMeasure::operator=(const AccountingTAMMeasure &cp) {
    if( &cp != this ){
        setUnitMeasure( cp.m_d->unitMeasure );
        setComment( cp.m_d->comment );
        resizeDays( cp.m_d->formula.size() );
        for( int i = 0; i < cp.m_d->formula.size(); ++i ){
            setFormula( i, cp.m_d->formula.at(i) );
        }
    }

    return *this;
}

QString AccountingTAMMeasure::comment() const{
    return m_d->comment;
}

void AccountingTAMMeasure::setComment( const QString & nc ){
    m_d->comment = nc;
}

void AccountingTAMMeasure::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        m_d->unitMeasure = ump;
        updateQuantity();
    }
}

QString AccountingTAMMeasure::formula( int i ) const{
    if( i < m_d->formula.size() ){
        return m_d->formula.at(i);
    }
    return QString();
}

void AccountingTAMMeasure::setFormula( int i, const QString & nf ){
    if( i < m_d->formula.size() ){
        if( nf != m_d->formula.at(i) ){
            m_d->formula[i] = nf;
            updateQuantity();
        }
    }
}

void AccountingTAMMeasure::updateQuantity(){
    double v = 0.0;
    for( QVector<QString>::iterator i=m_d->formula.begin(); i != m_d->formula.end(); ++i ){
        double valTmp = m_d->parser->evaluate( *i );
        if( m_d->unitMeasure ) {
            valTmp += m_d->unitMeasure->applyPrecision( valTmp );
        }
        v += valTmp;
    }
    if( v != m_d->quantity ){
        m_d->quantity = v;
        emit quantityChanged( v );
    }
}

double AccountingTAMMeasure::quantity() const{
    double ret = 0.0;
    if( m_d->unitMeasure != NULL ){
        ret = m_d->unitMeasure->applyPrecision( m_d->quantity );
    } else {
        ret = m_d->quantity;
    }
    return ret;
}

QString AccountingTAMMeasure::quantityStr() const{
    if( m_d->unitMeasure != NULL ){
        return m_d->toString( quantity(), 'f', m_d->unitMeasure->precision() ) ;
    } else {
        return m_d->toString( quantity(), 'f', 6 ) ;
    }
}

void AccountingTAMMeasure::writeXml( QXmlStreamWriter * writer ){
    writer->writeStartElement( "AccountingTAMMeasure" );

    writer->writeAttribute( "comment", m_d->comment );

    for( int i=0; i < m_d->formula.size(); ++i ){
        QString f = m_d->formula.at(i);
        if( m_d->parser->decimalSeparator() != "." ){
            f.replace( m_d->parser->decimalSeparator(), ".");
        }
        writer->writeAttribute( "formula"+QString(i), f );
    }

    writer->writeEndElement();
}

void AccountingTAMMeasure::loadXmlTmp20(const QXmlStreamAttributes &attrs) {
    m_d->tmpAttrs.clear();
    m_d->tmpAttrs = attrs;
}

void AccountingTAMMeasure::loadFromXmlTmp20() {
    for( QXmlStreamAttributes::const_iterator i = m_d->tmpAttrs.begin(); i != m_d->tmpAttrs.end(); ++i ){
        if( i->name() == "comment" ){
            setComment(  i->value().toString() );
        }
        QString attrForm( "formula" );
        if( i->name().startsWith(attrForm) ){
            QString numStr = i->name().toString();
            numStr.remove( 0, attrForm.size() );
            bool ok = false;
            int num = numStr.toInt( & ok );
            if( ok ){
                QString f = i->value().toString();
                if( m_d->parser->decimalSeparator() != "." ){
                    f.replace( ".", m_d->parser->decimalSeparator());
                }
                setFormula( num, f );
            }
        }
    }
}

void AccountingTAMMeasure::resizeDays(int newSize) {
    if( newSize > 0 ){
        m_d->formula.resize( newSize );
        updateQuantity();
    }
}
