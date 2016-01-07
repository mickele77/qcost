/*
   QCost is a cost estimating software.
   Copyright (C) 2013-2014 Mocciola Michele
°ùà
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
#include "projectpricelistparentitem.h"

#include "pricelist.h"

#include <QXmlStreamReader>
#include <QTextStream>
#include <QVariant>

class ProjectPriceListParentItemPrivate{
public:
    ProjectPriceListParentItemPrivate( PriceFieldModel * priceFields, MathParser * p ):
        priceFieldsModel( priceFields ),
        parser(p),
        nextId(1){
    };
    QList<PriceList *> priceListContainer;
    PriceFieldModel * priceFieldsModel;
    MathParser * parser;
    unsigned int nextId;
};

ProjectPriceListParentItem::ProjectPriceListParentItem(ProjectItem *parent, PriceFieldModel * priceFields, MathParser *p):
    ProjectItem(parent),
    m_d( new ProjectPriceListParentItemPrivate(priceFields, p)){
}

ProjectPriceListParentItem::~ProjectPriceListParentItem(){
    delete m_d;
}

ProjectItem *ProjectPriceListParentItem::child(int number) {
    if( number >= 0 && number < m_d->priceListContainer.size() ){
        return m_d->priceListContainer[number];
    }
    return NULL;
}

int ProjectPriceListParentItem::childCount() const {
    return m_d->priceListContainer.size();
}

int ProjectPriceListParentItem::childNumber(ProjectItem *item) {
    PriceList * pl = dynamic_cast<PriceList *>(item);
    if( pl ){
        return m_d->priceListContainer.indexOf( pl );
    }
    return -1;
}

bool ProjectPriceListParentItem::canChildrenBeInserted() {
    return true;
}

bool ProjectPriceListParentItem::insertChildren(int position, int count) {
    if (position < 0 || position > m_d->priceListContainer.size())
        return false;

    emit beginInsertChildren( position, position+count-1);

    for (int row = 0; row < count; ++row) {
        QString purposedPLName = trUtf8("Prezzario %1").arg(m_d->nextId++);
        QList<PriceList *>::iterator i = m_d->priceListContainer.begin();
        while( i != m_d->priceListContainer.end() ){
            if( (*i)->name().toUpper() == purposedPLName.toUpper() ){
                purposedPLName = trUtf8("Prezzario %1").arg(m_d->nextId++);
                i = m_d->priceListContainer.begin();
            } else {
                i++;
            }
        }
        PriceList *item = new PriceList( purposedPLName, m_d->priceFieldsModel, this, m_d->parser );
        while( priceListId(item->id()) != NULL ){
            item->nextId();
        }
        connect( item, &PriceList::removePriceItemSignal, this, &ProjectPriceListParentItem::emitRemovePriceItemSignal );
        connect( item, &PriceList::modelChanged, this, &ProjectPriceListParentItem::modelChanged );
        m_d->priceListContainer.insert(position, item);
    }

    emit endInsertChildren();

    return true;
}

bool ProjectPriceListParentItem::appendChildren( int count ) {
    return insertChildren( m_d->priceListContainer.size(), count );
}

bool ProjectPriceListParentItem::removeChildren(int position, int count) {
    if (position < 0
            || (position + count) > m_d->priceListContainer.size()
            || count < 1 )
        return false;

    emit removePriceListSignal( position, count );

    return true;
}

bool ProjectPriceListParentItem::removeChildrenPrivate(int position, int count) {
    if (position < 0 || position + count > m_d->priceListContainer.size())
        return false;
    emit beginRemoveChildren(position, position+count-1);
    for (int row = 0; row < count; ++row){
        PriceList * item = m_d->priceListContainer.at( position );
        disconnect( item, &PriceList::removePriceItemSignal, this, &ProjectPriceListParentItem::emitRemovePriceItemSignal );
        disconnect( item, &PriceList::modelChanged, this, &ProjectPriceListParentItem::modelChanged );
        delete item;
        m_d->priceListContainer.removeAt(position);
    }
    emit endRemoveChildren();
    return true;
}

Qt::ItemFlags ProjectPriceListParentItem::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant ProjectPriceListParentItem::data() const {
    return QVariant( QObject::trUtf8("Prezzari") );
}

bool ProjectPriceListParentItem::setData(const QVariant &value) {
    Q_UNUSED(value);
    return false;
}

int ProjectPriceListParentItem::priceListCount() {
    return m_d->priceListContainer.size();
}

PriceList *ProjectPriceListParentItem::priceList(int i) {
    if( i > -1 && i < m_d->priceListContainer.size() ){
        return m_d->priceListContainer[i];
    }
    return NULL;
}

PriceList *ProjectPriceListParentItem::priceListId(unsigned int dd) {
    for( QList<PriceList *>::iterator i = m_d->priceListContainer.begin(); i != m_d->priceListContainer.end(); ++i){
        if( (*i)->id() == dd ){
            return (*i);
        }
    }
    return NULL;
}

void ProjectPriceListParentItem::writeXml(QXmlStreamWriter *writer, const QString & vers ) const {
    if( (vers == "1.0") || (vers == "0.3") ){
        writeXml10(writer);
    } else {
        writeXml10(writer);
    }
}

void ProjectPriceListParentItem::writeXml10(QXmlStreamWriter *writer) const {
    writer->writeStartElement("PriceLists");
    for( QList<PriceList*>::iterator i = m_d->priceListContainer.begin(); i != m_d->priceListContainer.end(); ++i ){
        (*i)->writeXml10( writer );
    }
    writer->writeEndElement();
}

void ProjectPriceListParentItem::readXml(QXmlStreamReader *reader, UnitMeasureModel * uml ) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "PRICELISTS") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "PRICELIST" && reader->isStartElement()) {
            if(appendChildren()){
                m_d->priceListContainer.last()->readXml( reader, uml );
            }
        }
    }
    for( QList<PriceList*>::iterator i = m_d->priceListContainer.begin(); i != m_d->priceListContainer.end(); ++i ){
        (*i)->loadTmpData( this );
    }
}

bool ProjectPriceListParentItem::clear() {
    bool ret = removeChildren( 0, m_d->priceListContainer.size() );
    if( ret ){
        m_d->nextId = 1;
    }
    return ret;
}

void ProjectPriceListParentItem::emitRemovePriceItemSignal(int position, int count, const QModelIndex &parent) {
    PriceList * pl = dynamic_cast<PriceList *>(QObject::sender());
    emit removePriceItemSignal( pl, position, count, parent );
}
