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

#include "attributemodel.h"

#include "attribute.h"
#include "accountinglsbill.h"
#include "accountingtambill.h"
#include "accountingbill.h"
#include "bill.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include <QList>

class AttributeModelPrivate{
public:
    AttributeModelPrivate(Bill * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBill(NULL),
        accountingTAMBill(NULL),
        accountingBill(NULL),
        bill(b),
        parser(prs),
        priceFieldModel(pfm) {
    }
    AttributeModelPrivate(AccountingBill * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBill(NULL),
        accountingTAMBill(NULL),
        accountingBill(b),
        bill(NULL),
        parser(prs),
        priceFieldModel(pfm) {
    }
    AttributeModelPrivate(AccountingTAMBill * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBill(NULL),
        accountingTAMBill(b),
        accountingBill(NULL),
        bill(NULL),
        parser(prs),
        priceFieldModel(pfm) {
    }
    AttributeModelPrivate(AccountingLSBill * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBill(b),
        accountingTAMBill(NULL),
        accountingBill(NULL),
        bill(NULL),
        parser(prs),
        priceFieldModel(pfm) {
    }
    ~AttributeModelPrivate(){
        for( int i=0; i < attributeContainer.size(); ++i ){
            delete attributeContainer.takeAt(i);
        }
    }
    void insert( int row, Attribute * attr ){
        attributeContainer.insert( row, attr );
    }
    void removeAndDel( int row ){
        delete attributeContainer.at( row );
        attributeContainer.removeAt( row );
    }
    Attribute * attribute( unsigned int id ){
        for( QList<Attribute *>::iterator i = attributeContainer.begin(); i != attributeContainer.end(); ++i ){
            if( (*i)->id() == id ){
                return *i;
            }
        }
        return NULL;
    }
    QList<Attribute *> attributeContainer;
    AccountingLSBill * accountingLSBill;
    AccountingTAMBill * accountingTAMBill;
    AccountingBill * accountingBill;
    Bill * bill;
    MathParser * parser;
    PriceFieldModel * priceFieldModel;
};

AttributeModel::AttributeModel(Bill * myBill, MathParser *prs, PriceFieldModel * pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributeModelPrivate(myBill, prs, pfm) ){
}

AttributeModel::AttributeModel(AccountingBill * myBill, MathParser *prs, PriceFieldModel * pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributeModelPrivate(myBill, prs, pfm) ){
}

AttributeModel::AttributeModel(AccountingTAMBill * myBill, MathParser *prs, PriceFieldModel * pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributeModelPrivate(myBill, prs, pfm) ){
}

AttributeModel::AttributeModel(AccountingLSBill *myBill, MathParser *prs, PriceFieldModel *pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributeModelPrivate(myBill, prs, pfm) ){
}

AttributeModel::~AttributeModel() {
    emit aboutToBeDeleted();
    delete m_d;
}

void AttributeModel::insertStandardAttributes() {
    if( m_d->bill != NULL ){
        if( insertRows( 0, 2) ){
            setData( createIndex(0,0), QVariant( trUtf8("Sogg.ribasso")) );
            setData( createIndex(1,0), QVariant( trUtf8("Non sogg.ribasso")) );
        }
    }
}


int AttributeModel::size() {
    return m_d->attributeContainer.size();
}

int AttributeModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return m_d->attributeContainer.size();
}

int AttributeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    if( m_d->priceFieldModel != NULL ){
        return m_d->priceFieldModel->fieldCount()+1;
    } else {
        return 2;
    }
}

Qt::ItemFlags AttributeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if( index.column() == 0 ){
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    } else {
        return QAbstractTableModel::flags(index);
    }
}

QVariant AttributeModel::data(const QModelIndex &index, int role) const {
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
            if( m_d->bill != NULL ){
                return QVariant( m_d->bill->amountAttributeStr( m_d->attributeContainer.at(index.row()), index.column() - 1) );
            }
            if( m_d->accountingBill != NULL ){
                return QVariant( m_d->accountingBill->totalAmountToDiscountStr() );
            }
            if( m_d->accountingTAMBill != NULL ){
                return QVariant( m_d->accountingTAMBill->totalAmountToDiscountStr() );
            }
            if( m_d->accountingLSBill != NULL ){
                return QVariant( m_d->accountingLSBill->projAmountStr() );
            }
        }
    }
    return QVariant();
}

bool AttributeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole && index.row() < m_d->attributeContainer.size() ) {
        if( index.column() == 0 ){
            m_d->attributeContainer.at(index.row())->setName( value.toString() );
            emit(dataChanged(index, index));
            return true;
        }
    }
    return false;
}

QVariant AttributeModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

bool AttributeModel::insertRows(int row, int count){
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
        Attribute * attr = new Attribute( m_d->parser, m_d->priceFieldModel );
        while( attributeId(attr->id()) != NULL ){
            attr->nextId();
        }
        m_d->insert( row, attr );
    }
    endInsertRows();
    return true;
}

bool AttributeModel::append() {
    return insertRows( m_d->attributeContainer.size() );
}

bool AttributeModel::removeRows(int row, int count) {
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

bool AttributeModel::clear() {
    return removeRows( 0, m_d->attributeContainer.size() );
}

Attribute *AttributeModel::attribute(int i) {
    if( i >= 0 && i < m_d->attributeContainer.size()){
        return m_d->attributeContainer.value(i);
    }
    return NULL;
}

Attribute *AttributeModel::attributeId(unsigned int id) {
    for( QList<Attribute *>::iterator i = m_d->attributeContainer.begin(); i != m_d->attributeContainer.end(); ++i ){
        if( (*i)->id() == id ){
            return (*i);
        }
    }
    return NULL;
}

void AttributeModel::writeXml(QXmlStreamWriter *writer) {
    writer->writeStartElement( "AttributeModel" );
    for( QList<Attribute *>::iterator i = m_d->attributeContainer.begin(); i != m_d->attributeContainer.end(); ++i ){
        (*i)->writeXml( writer );
    }
    writer->writeEndElement();
}

void AttributeModel::readXml(QXmlStreamReader *reader) {
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ATTRIBUTEMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "ATTRIBUTE" && reader->isStartElement()) {
            if(append()){
                m_d->attributeContainer.last()->loadFromXml( reader->attributes() );
            }
        }
    }
}
