#include "var.h"

#include "mathparser.h"

#include <QTextStream>
#include <QXmlStreamAttributes>
#include <QVariant>
#include <QString>

class VarPrivate{
public:
    VarPrivate(MathParser * prs):
        parser(prs){
    }
    QString name;
    QString comment;
    QString value;
    MathParser * parser;
};

Var::Var( MathParser * prs ) :
    QObject(0),
    m_d( new VarPrivate(prs)){
}

Var::~Var(){
    delete m_d;
}

Var &Var::operator=(const Var &cp) {
    if( &cp != this ){
        setComment( cp.m_d->comment );
        setName( cp.m_d->name );
        setValue( cp.m_d->value );
    }

    return *this;
}

QString Var::comment() const{
    return m_d->comment;
}

void Var::setComment( const QString & nc ){
    if( m_d->comment != nc ){
        m_d->comment = nc;
    }
}

QString Var::name() const {
    return m_d->name;
}

void Var::setName( const QString & nf ){
    if( nf != m_d->name ){
        m_d->name = nf;
    }
}

QString Var::value() const{
    return m_d->value;
}

void Var::setValue( const QString & nv ){
    if( nv != m_d->value ){
        m_d->value = nv;
    }
}

void Var::writeXml( QXmlStreamWriter * writer ){
    writer->writeStartElement( "Var" );
    writer->writeAttribute( "comment", comment() );
    writer->writeAttribute( "name", name() );
    QString v = m_d->value;
    v.replace( m_d->parser->decimalSeparator(), "." );
    writer->writeAttribute( "value", v );
    writer->writeEndElement();
}

void Var::loadXml(const QXmlStreamAttributes &attrs) {
    for( QXmlStreamAttributes::const_iterator it = attrs.begin(); it != attrs.end(); ++it ){
        QString nUp = (*it).name().toString().toUpper();
        if( nUp == "COMMENT" ){
            setComment(  (*it).value().toString() );
        }
        if( nUp == "NAME" ){
            setName(  (*it).value().toString() );
        }
        if( nUp == "VALUE" ){
            QString v = (*it).value().toString();
            setValue( v.replace(".", m_d->parser->decimalSeparator()) );
        }

    }
}

void Var::replaceVar( QString * expr ) {
    if( !(m_d->name.isEmpty()) ){
        expr->replace(m_d->name, m_d->value);
    }
}
