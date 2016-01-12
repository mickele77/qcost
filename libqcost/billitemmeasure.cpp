#include "billitemmeasure.h"

#include "unitmeasure.h"
#include "mathparser.h"

#include <QTextStream>
#include <QXmlStreamAttributes>
#include <QVariant>
#include <QString>

class BillItemMeasurePrivate{
public:
    BillItemMeasurePrivate(MathParser * p, UnitMeasure * ump):
        parser( p ),
        unitMeasure( ump ),
        comment(""),
        formula(""),
        quantity(0.0){
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
    QString formula;
    double quantity;
};

BillItemMeasure::BillItemMeasure(MathParser * p, UnitMeasure * ump) :
    QObject(0),
    m_d( new BillItemMeasurePrivate(p, ump)){
}

BillItemMeasure::~BillItemMeasure(){
    delete m_d;
}

BillItemMeasure &BillItemMeasure::operator=(const BillItemMeasure &cp) {
    if( &cp != this ){
        setUnitMeasure( cp.m_d->unitMeasure );
        setComment( cp.m_d->comment );
        setFormula( cp.m_d->formula );
    }

    return *this;
}

QString BillItemMeasure::comment(){
    return m_d->comment;
}

void BillItemMeasure::setComment( const QString & nc ){
    m_d->comment = nc;
}

void BillItemMeasure::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->unitMeasure != ump ){
        m_d->unitMeasure = ump;
        updateQuantity();
    }
}

QString BillItemMeasure::formula(){
    return m_d->formula;
}

void BillItemMeasure::setFormula( const QString & nf ){
    if( nf != m_d->formula ){
        m_d->formula = nf;
        updateQuantity();
    }
}

void BillItemMeasure::updateQuantity(){
    double v = m_d->parser->evaluate( m_d->formula );
    if( m_d->unitMeasure ) {
        v = m_d->unitMeasure->applyPrecision( v );
    }
    if( v != m_d->quantity ){
        m_d->quantity = v;
        emit quantityChanged( v );
    }
}

double BillItemMeasure::quantity(){
    double ret = 0.0;
    if( m_d->unitMeasure != NULL ){
        ret = m_d->unitMeasure->applyPrecision( m_d->quantity );
    } else {
        ret = m_d->quantity;
    }
    return ret;
}

QString BillItemMeasure::quantityStr(){
    QString realFormula = m_d->formula;
    realFormula.remove(" ");
    if( realFormula.isEmpty() ){
        return QString();
    }
    if( m_d->unitMeasure != NULL ){
        return m_d->toString( quantity(), 'f', m_d->unitMeasure->precision() ) ;
    } else {
        return m_d->toString( quantity(), 'f', 6 ) ;
    }
}

void BillItemMeasure::writeXml10( QXmlStreamWriter * writer ){
    writer->writeStartElement( "BillItemMeasure" );
    writer->writeAttribute( "comment", m_d->comment );
    QString f = m_d->formula;
    if( m_d->parser->decimalSeparator() != "." ){
        f.replace( m_d->parser->decimalSeparator(), ".");
    }
    writer->writeAttribute( "formula", f );
    writer->writeEndElement();
}

void BillItemMeasure::loadFromXml10(const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "comment" ) ){
        setComment(  attrs.value( "comment").toString() );
    }
    if( attrs.hasAttribute( "formula" ) ){
        QString f = attrs.value( "formula").toString();
        if( m_d->parser->decimalSeparator() != "." ){
            f.replace( ".", m_d->parser->decimalSeparator());
        }
        setFormula( f );
    }
}
