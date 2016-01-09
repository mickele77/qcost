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
#include "billattribute.h"

#include "bill.h"
#include "billitem.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QXmlStreamAttributes>
#include <QTextStream>
#include <QString>
#include <QList>

class BillAttributePrivate{
public:
    BillAttributePrivate(MathParser * prs, PriceFieldModel * pfm):
        parser(prs),
        priceFieldModel(pfm),
        name(QString()),
        id(0){
    };

    MathParser * parser;
    PriceFieldModel * priceFieldModel;
    QString name;
    unsigned int id;
};

BillAttribute::BillAttribute(MathParser * prs, PriceFieldModel *pfm):
    m_d(new BillAttributePrivate(prs, pfm) ){
}

BillAttribute::~BillAttribute(){
    delete m_d;
}

QString BillAttribute::name() {
    return m_d->name;
}

void BillAttribute::setName(const QString &n) {
    m_d->name = n;
}

unsigned int BillAttribute::id() {
    return m_d->id;
}

void BillAttribute::writeXml10(QXmlStreamWriter *writer) {
    writer->writeStartElement( "BillAttribute" );
    writer->writeAttribute( "id", QString::number( m_d->id ) );
    writer->writeAttribute( "name", m_d->name );
    writer->writeEndElement();
}

void BillAttribute::loadFromXml(const QXmlStreamAttributes &attrs) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "name" ) ){
        setName( attrs.value( "name").toString() );
    }
}

void BillAttribute::nextId() {
    m_d->id++;
}
