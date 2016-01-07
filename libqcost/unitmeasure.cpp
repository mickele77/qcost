/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2014 Mocciola Michele

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include "unitmeasure.h"

#include <QTextStream>
#include <QXmlStreamAttributes>
#include <cmath>
#include <cfloat>

class UnitMeasurePrivate{
public:
    UnitMeasurePrivate():
        id(0),
        precision(3){
    };
    unsigned int id;
    QString tag;
    QString description;
    int precision;
};

UnitMeasure::UnitMeasure(QObject *parent) :
    QObject(parent),
    m_d( new UnitMeasurePrivate() ){
}

UnitMeasure::~UnitMeasure(){
    delete m_d;
}

unsigned int UnitMeasure::id() {
    return m_d->id;
}

void UnitMeasure::nextId() {
    m_d->id++;
}

QString UnitMeasure::tag(){
    return m_d->tag;
}

void UnitMeasure::setTag( const QString & t ){
    if( t != m_d->tag ){
        m_d->tag = t;
        emit tagChanged( t );
    }
}

int UnitMeasure::precision() {
    return m_d->precision;
}

void UnitMeasure::setPrecision(int p ) {
    if( m_d->precision != p ){
        m_d->precision = p;
        emit precisionChanged( p );
    }
}

double UnitMeasure::applyPrecision(double value ) {
    return applyPrecision( value, m_d->precision );
}

double UnitMeasure::applyPrecision(double value, int prec ) {
    if( prec > 0 && value != 0.0 ){
        // e' necessario farla un po' piu' complicata a causa di
        // problemi di conversione decimale->binario e
        // successivo arrotondamento:
        // round( 2.55 * 0.50 ) = round( 1.2749999999999 ) = 1.27
        // invece di 1.28
        double absValue = fabs(value);
        int exp = ceil( log10( absValue ));
        if( (exp+prec) > (DBL_DIG-1) ){
            prec = (DBL_DIG-1) - exp;
        }
        double sgnValue = value / absValue;
        double ord = pow(10.0, prec);
        double ret = round( absValue * ord );
        double D = 10.0 * (absValue * ord - ret);

        double inf = 4.0;
        for( int i=0; i < ((DBL_DIG-1) - (exp+prec)); ++i ){
            inf += 9.0 * pow( 10.0, -(i+1)  );
        }
        if( D < 5.0 && D > inf ){
            ret += 1.0;
        }
        ret = sgnValue * ret / ord;
        return ret;
    }
    return value;
}

QString UnitMeasure::description() {
    return m_d->description;
}

void UnitMeasure::setDescription(const QString &d) {
    if( m_d->description != d ){
        m_d->description = d;
        emit descriptionChanged( d );
    }
}

void UnitMeasure::writeXml10(QXmlStreamWriter *writer ) const{
    writer->writeStartElement( "UnitMeasure" );
    writer->writeAttribute( "id", QString::number( m_d->id ) );
    writer->writeAttribute( "tag", m_d->tag );
    writer->writeAttribute( "description", m_d->description );
    writer->writeAttribute( "precision", QString::number( m_d->precision ) );
    writer->writeEndElement();
}

void UnitMeasure::loadFromXml(const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "tag" ) ){
        setTag( attrs.value( "tag").toString() );
    }
    if( attrs.hasAttribute( "description" ) ){
        setDescription( attrs.value( "description").toString() );
    }
    if( attrs.hasAttribute( "precision" ) ){
        setPrecision( attrs.value( "precision").toInt() );
    }
}
