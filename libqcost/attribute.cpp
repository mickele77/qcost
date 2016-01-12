/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2016 Mocciola Michele

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
#include "attribute.h"

#include "bill.h"
#include "billitem.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QXmlStreamAttributes>
#include <QTextStream>
#include <QString>
#include <QList>

class AttributePrivate{
public:
    AttributePrivate(MathParser * prs, PriceFieldModel * pfm):
        parser(prs),
        priceFieldModel(pfm),
        name(QString()),
        id(0){
    }

    MathParser * parser;
    PriceFieldModel * priceFieldModel;
    QString name;
    unsigned int id;
};

Attribute::Attribute(MathParser * prs, PriceFieldModel *pfm):
    m_d(new AttributePrivate(prs, pfm) ){
}

Attribute::~Attribute(){
    delete m_d;
}

Attribute &Attribute::operator=(const Attribute &cp) {
    if( &cp != this ){
        setName( cp.m_d->name );
    }
    return *this;
}

QString Attribute::name() {
    return m_d->name;
}

void Attribute::setName(const QString &n) {
    m_d->name = n;
}

unsigned int Attribute::id() {
    return m_d->id;
}

void Attribute::writeXml10(QXmlStreamWriter *writer) {
    writer->writeStartElement( "BillAttribute" );
    writer->writeAttribute( "id", QString::number( m_d->id ) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeEndElement();
}

void Attribute::writeXml20(QXmlStreamWriter *writer) {
    writer->writeStartElement( "Attribute" );
    writer->writeAttribute( "id", QString::number( m_d->id ) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeEndElement();
}

void Attribute::loadFromXml10(const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "name" ) ){
        setName( attrs.value( "name").toString() );
    }
}

void Attribute::loadFromXml20(const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "name" ) ){
        setName( attrs.value( "name").toString() );
    }
}

void Attribute::nextId() {
    m_d->id++;
}
