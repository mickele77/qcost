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
#include "projectbillparentitem.h"

#include "bill.h"
#include "priceitem.h"

#include <QXmlStreamReader>
#include <QTextStream>
#include <QObject>
#include <QVariant>
#include <QList>

class ProjectBillParentItemPrivate{
public:
    ProjectBillParentItemPrivate( PriceFieldModel * pfm, MathParser * p = NULL ):
        priceFieldModel(pfm),
        parser(p),
        nextId(1){
    };
    QList<Bill *> billContainer;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;
    unsigned int nextId;
};

ProjectBillParentItem::ProjectBillParentItem(ProjectItem *parent, PriceFieldModel * pfm, MathParser * prs ):
    ProjectItem(parent),
    m_d( new ProjectBillParentItemPrivate( pfm, prs ) ){
}

int ProjectBillParentItem::billCount() {
    return m_d->billContainer.size();
}

Bill *ProjectBillParentItem::bill(int i) {
    if( i > -1 && i < m_d->billContainer.size() ){
        return m_d->billContainer[i];
    }
    return NULL;
}

ProjectItem *ProjectBillParentItem::child(int number) {
    if( number >= 0 && number < m_d->billContainer.size() ){
        return m_d->billContainer[number];
    }
    return NULL;
}

int ProjectBillParentItem::childCount() const {
    return m_d->billContainer.size();
}

int ProjectBillParentItem::childNumber(ProjectItem *item) {
    Bill * b = dynamic_cast<Bill *>(item);
    if( b ){
        return m_d->billContainer.indexOf( b );
    }
    return -1;
}

bool ProjectBillParentItem::canChildrenBeInserted() {
    return true;
}

bool ProjectBillParentItem::insertChildren(int position, int count) {
    if (position < 0 || position > m_d->billContainer.size())
        return false;

    emit beginInsertChildren( position, position+count-1);

    for (int row = 0; row < count; ++row) {
        QString purposedBillName = trUtf8("Computo %1").arg(m_d->nextId++);
        QList<Bill *>::iterator i = m_d->billContainer.begin();
        while( i != m_d->billContainer.end() ){
            if( (*i)->name().toUpper() == purposedBillName.toUpper() ){
                purposedBillName = trUtf8("Computo %1").arg(m_d->nextId++);
                i = m_d->billContainer.begin();
            } else {
                i++;
            }
        }
        Bill *item = new Bill( purposedBillName, this, m_d->priceFieldModel, m_d->parser );
        connect( item, &Bill::modelChanged, this, &ProjectBillParentItem::modelChanged );
        m_d->billContainer.insert(position, item);
    }

    emit endInsertChildren();

    return true;
}

bool ProjectBillParentItem::appendChild() {
    return insertChildren( m_d->billContainer.size(), 1 );
}

bool ProjectBillParentItem::removeChildren(int position, int count) {
    if (position < 0
            || (position + count) > m_d->billContainer.size()
            || count < 1 )
        return false;

    emit beginRemoveChildren( position, position+count-1);

    for (int row = 0; row < count; ++row){
        Bill * item = m_d->billContainer.at( position );
        disconnect( item, &Bill::modelChanged, this, &ProjectBillParentItem::modelChanged );
        delete item;
        m_d->billContainer.removeAt(  position );
    }

    emit endRemoveChildren();

    return true;
}

Qt::ItemFlags ProjectBillParentItem::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant ProjectBillParentItem::data() const {
    return QVariant( QObject::trUtf8("Computi") );
}

bool ProjectBillParentItem::setData(const QVariant &value) {
    Q_UNUSED(value);
    return false;
}

bool ProjectBillParentItem::isUsingPriceList(PriceList *pl) {
    for( QList<Bill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        if( (*i)->priceList() == pl ){
            return true;
        }
    }
    return false;
}

bool ProjectBillParentItem::isUsingPriceItem(PriceItem *p) {
    for( QList<Bill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        if( (*i)->isUsingPriceItem( p ) ){
            return true;
        }
        if( p->hasChildren() ){
            for( int i=0; i < p->childrenCount(); ++i ){
                bool ret = isUsingPriceItem( p->childItem(i) );
                if( ret ){
                    return true;
                }
            }
        }
    }
    return false;
}

void ProjectBillParentItem::writeXml(QXmlStreamWriter *writer, const QString & vers ) const {
    if( (vers == "1.0") || (vers == "0.3") ){
        writeXml10( writer );
    } else {
        writeXml10( writer );
    }
}

void ProjectBillParentItem::writeXml10(QXmlStreamWriter *writer) const {
    writer->writeStartElement( "Bills");
    for( QList<Bill *>::iterator i = m_d->billContainer.begin(); i != m_d->billContainer.end(); ++i ){
        (*i)->writeXml10( writer );
    }
    writer->writeEndElement();
}


void ProjectBillParentItem::readXml(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLS") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "BILL" && reader->isStartElement()) {
            if(appendChild()){
                m_d->billContainer.last()->readXml( reader, priceLists );
            }
        }
    }
}

bool ProjectBillParentItem::clear() {
    bool ret = removeChildren( 0, m_d->billContainer.size() );
    if( ret ){
        m_d->nextId = 1;
    }
    return ret;
}
