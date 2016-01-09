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

#include "billattributemodel.h"

#include "billattribute.h"
#include "bill.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include <QList>

class BillAttributeModelPrivate{
public:
    BillAttributeModelPrivate(Bill * b, MathParser *prs, PriceFieldModel * pfm):
        bill(b),
        parser(prs),
        priceFieldModel(pfm) {
    }
    ~BillAttributeModelPrivate(){
        for( int i=0; i < attributeContainer.size(); ++i ){
            delete attributeContainer.takeAt(i);
        }
    }
    void insert( int row, BillAttribute * attr ){
        attributeContainer.insert( row, attr );
    }
    void removeAndDel( int row ){
        delete attributeContainer.at( row );
        attributeContainer.removeAt( row );
    }
    BillAttribute * attribute( unsigned int id ){
        for( QList<BillAttribute *>::iterator i = attributeContainer.begin(); i != attributeContainer.end(); ++i ){
            if( (*i)->id() == id ){
                return *i;
            }
        }
        return NULL;
    }
    QList<BillAttribute *> attributeContainer;
    Bill * bill;
    MathParser * parser;
    PriceFieldModel * priceFieldModel;
};

BillAttributeModel::BillAttributeModel(Bill * myBill, MathParser *prs, PriceFieldModel * pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new BillAttributeModelPrivate(myBill, prs, pfm) ){
}

BillAttributeModel::~BillAttributeModel() {
    emit aboutToBeDeleted();
    delete m_d;
}

void BillAttributeModel::insertStandardAttributes() {
    if( insertRows( 0, 2) ){
        setData( createIndex(0,0), QVariant( trUtf8("Sogg.ribasso")) );
        setData( createIndex(1,0), QVariant( trUtf8("Non sogg.ribasso")) );
    }
}


int BillAttributeModel::size() {
    return m_d->attributeContainer.size();
}

int BillAttributeModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return m_d->attributeContainer.size();
}

int BillAttributeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return m_d->priceFieldModel->fieldCount()+1;
}

Qt::ItemFlags BillAttributeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if( index.column() == 0 ){
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    } else {
        return QAbstractItemModel::flags(index);
    }
}

QVariant BillAttributeModel::data(const QModelIndex &index, int role) const {
    if( !index.isValid() || !(index.row() < m_d->attributeContainer.size()) ){
        return QVariant();
    }
    if( role == Qt::TextAlignmentRole ){
        if( index.column() > 0 ){
            return Qt::AlignRight + Qt::AlignVCenter;
        } else {
            return Qt::AlignLeft + Qt::AlignVCenter;
        }
    }
    if( role == Qt::EditRole ){
        if( index.column() == 0 ){
            return QVariant( m_d->attributeContainer.at(index.row())->name() );
        }
    }
    if( role == Qt::DisplayRole ){
        if( index.column() == 0 ){
            return QVariant( m_d->attributeContainer.at(index.row())->name() );
        }
        if( index.column() < m_d->priceFieldModel->fieldCount() + 1 ){
            return QVariant( m_d->bill->amountAttributeStr( m_d->attributeContainer.at(index.row()), index.column() - 1) );
        }
    }
    return QVariant();
}

bool BillAttributeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole && index.row() < m_d->attributeContainer.size() ) {
        if( index.column() == 0 ){
            m_d->attributeContainer.at(index.row())->setName( value.toString() );
            emit(dataChanged(index, index));
            return true;
        }
    }
    return false;
}

QVariant BillAttributeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if( section == 0 ) {
            return trUtf8("Nome");
        }
        if( section < m_d->priceFieldModel->fieldCount() + 1 ){
            return QVariant( m_d->priceFieldModel->amountName(section - 1) );
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool BillAttributeModel::insertRows(int row, int count){
    if( count < 1 ){
        return false;
    }
    if( row < 0 ){
        row = 0;
    }
    if( row > m_d->attributeContainer.size() ){
        row = m_d->attributeContainer.size();
    }
    beginInsertRows(QModelIndex(), row, row+count-1 );
    for(int i=0; i < count; ++i){
        BillAttribute * attr = new BillAttribute( m_d->parser, m_d->priceFieldModel );
        while( attributeId(attr->id()) != NULL ){
            attr->nextId();
        }
        m_d->insert( row, attr );
    }
    endInsertRows();
    return true;
}

bool BillAttributeModel::append() {
    return insertRows( m_d->attributeContainer.size() );
}

bool BillAttributeModel::removeRows(int row, int count) {
    if( count < 1 || row < 0 || row > m_d->attributeContainer.size() ){
        return false;
    }

    if( (row+count) > m_d->attributeContainer.size() ){
        count = m_d->attributeContainer.size() - row;
    }

    beginRemoveRows(QModelIndex(), row, row+count-1);
    for(int i=0; i < count; ++i){
        m_d->removeAndDel( row );
    }
    endRemoveRows();
    return true;
}

bool BillAttributeModel::clear() {
    return removeRows( 0, m_d->attributeContainer.size() );
}

BillAttribute *BillAttributeModel::attribute(int i) {
    if( i >= 0 && i < m_d->attributeContainer.size()){
        return m_d->attributeContainer.value(i);
    }
    return NULL;
}

BillAttribute *BillAttributeModel::attributeId(unsigned int id) {
    for( QList<BillAttribute *>::iterator i = m_d->attributeContainer.begin(); i != m_d->attributeContainer.end(); ++i ){
        if( (*i)->id() == id ){
            return (*i);
        }
    }
    return NULL;
}

void BillAttributeModel::writeXml10(QXmlStreamWriter *writer) const {
    writer->writeStartElement( "BillAttributeModel" );
    for( QList<BillAttribute *>::iterator i = m_d->attributeContainer.begin(); i != m_d->attributeContainer.end(); ++i ){
        (*i)->writeXml10( writer );
    }
    writer->writeEndElement();
}

void BillAttributeModel::readXml10(QXmlStreamReader *reader) {
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLATTRIBUTEMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "BILLATTRIBUTE" && reader->isStartElement()) {
            if(append()){
                m_d->attributeContainer.last()->loadFromXml( reader->attributes() );
            }
        }
    }
}
