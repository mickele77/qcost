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

#include "attributesmodel.h"

#include "attribute.h"
#include "accountinglsbills.h"
#include "accountinglsbill.h"
#include "accountingtambill.h"
#include "accountingbill.h"
#include "bill.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include <QList>

class AttributesModelPrivate{
public:
    AttributesModelPrivate(Bill * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBills(nullptr),
        accountingLSBill(nullptr),
        accountingTAMBill(nullptr),
        accountingBill(nullptr),
        bill(b),
        parser(prs),
        priceFieldModel(pfm) {
    }
    AttributesModelPrivate(AccountingBill * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBills(nullptr),
        accountingLSBill(nullptr),
        accountingTAMBill(nullptr),
        accountingBill(b),
        bill(nullptr),
        parser(prs),
        priceFieldModel(pfm) {
    }
    AttributesModelPrivate(AccountingTAMBill * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBills(nullptr),
        accountingLSBill(nullptr),
        accountingTAMBill(b),
        accountingBill(nullptr),
        bill(nullptr),
        parser(prs),
        priceFieldModel(pfm) {
    }
    AttributesModelPrivate(AccountingLSBill * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBills(nullptr),
        accountingLSBill(b),
        accountingTAMBill(nullptr),
        accountingBill(nullptr),
        bill(nullptr),
        parser(prs),
        priceFieldModel(pfm) {
    }
    AttributesModelPrivate(AccountingLSBills * b, MathParser *prs, PriceFieldModel * pfm):
        accountingLSBills(b),
        accountingLSBill(nullptr),
        accountingTAMBill(nullptr),
        accountingBill(nullptr),
        bill(nullptr),
        parser(prs),
        priceFieldModel(pfm) {
    }
    ~AttributesModelPrivate(){
        for( int i=0; i < attributesContainer.size(); ++i ){
            delete attributesContainer.takeAt(i);
        }
    }
    void insert( int row, Attribute * attr ){
        attributesContainer.insert( row, attr );
    }
    void removeAndDel( int row ){
        delete attributesContainer.at( row );
        attributesContainer.removeAt( row );
    }
    Attribute * attribute( unsigned int id ){
        for( QList<Attribute *>::iterator i = attributesContainer.begin(); i != attributesContainer.end(); ++i ){
            if( (*i)->id() == id ){
                return *i;
            }
        }
        return nullptr;
    }
    QList<Attribute *> attributesContainer;
    AccountingLSBills * accountingLSBills;
    AccountingLSBill * accountingLSBill;
    AccountingTAMBill * accountingTAMBill;
    AccountingBill * accountingBill;
    Bill * bill;
    MathParser * parser;
    PriceFieldModel * priceFieldModel;
};

AttributesModel::AttributesModel(Bill * myBill, MathParser *prs, PriceFieldModel * pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributesModelPrivate(myBill, prs, pfm) ){
}

AttributesModel::AttributesModel(AccountingBill * myBill, MathParser *prs, PriceFieldModel * pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributesModelPrivate(myBill, prs, pfm) ){
}

AttributesModel::AttributesModel(AccountingTAMBill * myBill, MathParser *prs, PriceFieldModel * pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributesModelPrivate(myBill, prs, pfm) ){
}

AttributesModel::AttributesModel(AccountingLSBill *myBill, MathParser *prs, PriceFieldModel *pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributesModelPrivate(myBill, prs, pfm) ){
}

AttributesModel::AttributesModel(AccountingLSBills *myBills, MathParser *prs, PriceFieldModel *pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AttributesModelPrivate(myBills, prs, pfm) ){

}

AttributesModel::~AttributesModel() {
    emit aboutToBeDeleted();
    delete m_d;
}

AttributesModel &AttributesModel::operator=(const AttributesModel &cp) {
    if( &cp != this ){
        if( m_d->attributesContainer.size() > cp.m_d->attributesContainer.size() ){
            insertRows( m_d->attributesContainer.size(), cp.m_d->attributesContainer.size()-m_d->attributesContainer.size() );
        } else if( m_d->attributesContainer.size() < cp.m_d->attributesContainer.size() ){
            removeRows( m_d->attributesContainer.size(), m_d->attributesContainer.size()-cp.m_d->attributesContainer.size() );
        }

        for( int i=0; i < m_d->attributesContainer.size(); ++i ){
            *(m_d->attributesContainer[i]) = *(cp.m_d->attributesContainer.at(i));
        }
    }
    return *this;
}

void AttributesModel::insertStandardAttributes() {
    if( m_d->bill != nullptr ){
        if( insertRows( 0, 2) ){
            setData( createIndex(0,0), QVariant( tr("Sogg.ribasso")) );
            setData( createIndex(1,0), QVariant( tr("Non sogg.ribasso")) );
        }
    }
}

void AttributesModel::setBill( Bill * b ) {
    if( m_d->bill != nullptr ){
        beginResetModel();
        m_d->bill = b;
        m_d->accountingBill = nullptr;
        m_d->accountingTAMBill = nullptr;
        m_d->accountingLSBill = nullptr;
        m_d->accountingLSBills = nullptr;
        endResetModel();
    }
}

void AttributesModel::setBill( AccountingBill * b ) {
    if( m_d->accountingBill != nullptr ){
        beginResetModel();
        m_d->bill = nullptr;
        m_d->accountingBill = b;
        m_d->accountingTAMBill = nullptr;
        m_d->accountingLSBill = nullptr;
        m_d->accountingLSBills = nullptr;
        endResetModel();
    }
}

void AttributesModel::setBill( AccountingTAMBill * b ) {
    if( m_d->accountingLSBill != nullptr ){
        beginResetModel();
        m_d->bill = nullptr;
        m_d->accountingBill = nullptr;
        m_d->accountingTAMBill = b;
        m_d->accountingLSBill = nullptr;
        m_d->accountingLSBills = nullptr;
        endResetModel();
    }
}

void AttributesModel::setBill( AccountingLSBill * b ) {
    if( m_d->accountingLSBill != nullptr ){
        beginResetModel();
        m_d->bill = nullptr;
        m_d->accountingBill = nullptr;
        m_d->accountingTAMBill = nullptr;
        m_d->accountingLSBill = b;
        m_d->accountingLSBills = nullptr;
        endResetModel();
    }
}

void AttributesModel::setBill( AccountingLSBills * b ) {
    if( m_d->accountingLSBill != nullptr ){
        beginResetModel();
        m_d->bill = nullptr;
        m_d->accountingBill = nullptr;
        m_d->accountingTAMBill = nullptr;
        m_d->accountingLSBill = nullptr;
        m_d->accountingLSBills = b;
        endResetModel();
    }
}

int AttributesModel::size() {
    return m_d->attributesContainer.size();
}

int AttributesModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return m_d->attributesContainer.size();
}

int AttributesModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    if( m_d->bill != nullptr ){
        if( m_d->priceFieldModel != nullptr ){
            return m_d->priceFieldModel->fieldCount()+1;
        } else {
            return 2;
        }
    } else if( m_d->accountingBill != nullptr ){
        return 4;
    } else if( m_d->accountingTAMBill != nullptr ){
        return 4;
    } else if( m_d->accountingLSBill != nullptr ){
        return 3;
    } else if( m_d->accountingLSBills != nullptr ){
        return 3;
    } else {
        return 1;
    }
}

Qt::ItemFlags AttributesModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if( index.column() == 0 ){
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    } else {
        return QAbstractTableModel::flags(index);
    }
}

QVariant AttributesModel::data(const QModelIndex &index, int role) const {
    if( !index.isValid() || !(index.row() < m_d->attributesContainer.size()) ){
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
            return QVariant( m_d->attributesContainer.at(index.row())->name() );
        }
    }
    if( role == Qt::DisplayRole ){
        if( index.column() == 0 ){
            return QVariant( m_d->attributesContainer.at(index.row())->name() );
        }
        if( index.column() < m_d->priceFieldModel->fieldCount() + 1 ){
            if( m_d->bill != nullptr ){
                return QVariant( m_d->bill->amountAttributeStr( m_d->attributesContainer.at(index.row()), index.column() - 1) );
            } else if( m_d->accountingBill != nullptr ){
                if( index.column() == 1 ){
                    return QVariant( m_d->accountingBill->totalAmountToDiscountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                } else if( index.column() == 2 ){
                    return QVariant( m_d->accountingBill->amountNotToDiscountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                } else if( index.column() == 3 ){
                    return QVariant( m_d->accountingBill->totalAmountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                }
            } else if( m_d->accountingTAMBill != nullptr ){
                if( index.column() == 1 ){
                    return QVariant( m_d->accountingTAMBill->totalAmountToDiscountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                } else if( index.column() == 2 ){
                    return QVariant( m_d->accountingTAMBill->amountNotToDiscountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                } else if( index.column() == 3 ){
                    return QVariant( m_d->accountingTAMBill->totalAmountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                }
            } else if( m_d->accountingLSBill != nullptr ){
                if( index.column() == 1 ){
                    return QVariant( m_d->accountingLSBill->PPUTotalToDiscountStr( m_d->attributesContainer.at(index.row()) ) );
                } else if( index.column() == 2 ){
                    return QVariant( m_d->accountingLSBill->accAmountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                }
            } else if( m_d->accountingLSBills != nullptr ){
                if( index.column() == 1 ){
                    return QVariant( m_d->accountingLSBills->projAmountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                } else if( index.column() == 2 ){
                    return QVariant( m_d->accountingLSBills->accAmountAttributeStr( m_d->attributesContainer.at(index.row()) ) );
                }
            }
        }
    }
    return QVariant();
}

bool AttributesModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole && index.row() < m_d->attributesContainer.size() ) {
        if( index.column() == 0 ){
            m_d->attributesContainer.at(index.row())->setName( value.toString() );
            emit(dataChanged(index, index));
            return true;
        }
    }
    return false;
}

QVariant AttributesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if( section == 0 ) {
            return tr("Nome");
        }
        if( m_d->bill != nullptr ){
            if( section < m_d->priceFieldModel->fieldCount() + 1 ){
                return QVariant( m_d->priceFieldModel->amountName(section - 1) );
            }
        } else if( (m_d->accountingBill != nullptr) || (m_d->accountingTAMBill != nullptr) ){
            switch( section ){
            case 1: {
                return QVariant( tr("Importo lordo"));
                break; }
            case 2: {
                return QVariant( tr("Importo non ribassabile"));
                break; }
            case 3: {
                return QVariant( tr("Importo"));
                break; }
            }
        } else if( (m_d->accountingLSBill != nullptr) || (m_d->accountingLSBills != nullptr) ){
            switch( section ){
            case 1: {
                return QVariant( tr("Importo complessivo"));
                break; }
            case 2: {
                return QVariant( tr("Importo contabilizzato"));
                break; }
            }
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool AttributesModel::insertRows(int row, int count){
    if( count < 1 ){
        return false;
    }
    if( row < 0 ){
        row = 0;
    }
    if( row > m_d->attributesContainer.size() ){
        row = m_d->attributesContainer.size();
    }
    beginInsertRows(QModelIndex(), row, row+count-1 );
    for(int i=0; i < count; ++i){
        Attribute * attr = new Attribute( m_d->parser, m_d->priceFieldModel );
        while( attributeId(attr->id()) != nullptr ){
            attr->nextId();
        }
        m_d->insert( row, attr );
    }
    endInsertRows();
    return true;
}

bool AttributesModel::append() {
    return insertRows( m_d->attributesContainer.size() );
}

bool AttributesModel::removeRows(int row, int count) {
    if( count < 1 || row < 0 || row > m_d->attributesContainer.size() ){
        return false;
    }

    if( (row+count) > m_d->attributesContainer.size() ){
        count = m_d->attributesContainer.size() - row;
    }

    beginRemoveRows(QModelIndex(), row, row+count-1);
    for(int i=0; i < count; ++i){
        m_d->removeAndDel( row );
    }
    endRemoveRows();
    return true;
}

bool AttributesModel::clear() {
    return removeRows( 0, m_d->attributesContainer.size() );
}

Attribute *AttributesModel::attribute(int i) {
    if( i >= 0 && i < m_d->attributesContainer.size()){
        return m_d->attributesContainer.value(i);
    }
    return nullptr;
}

Attribute *AttributesModel::attributeId(unsigned int id) {
    for( QList<Attribute *>::iterator i = m_d->attributesContainer.begin(); i != m_d->attributesContainer.end(); ++i ){
        if( (*i)->id() == id ){
            return (*i);
        }
    }
    return nullptr;
}

void AttributesModel::writeXml10(QXmlStreamWriter *writer) const {
    writer->writeStartElement( "BillAttributeModel" );
    for( QList<Attribute *>::iterator i = m_d->attributesContainer.begin(); i != m_d->attributesContainer.end(); ++i ){
        (*i)->writeXml10( writer );
    }
    writer->writeEndElement();
}

void AttributesModel::writeXml20(QXmlStreamWriter *writer) const {
    writer->writeStartElement( "AttributesModel" );
    for( QList<Attribute *>::iterator i = m_d->attributesContainer.begin(); i != m_d->attributesContainer.end(); ++i ){
        (*i)->writeXml20( writer );
    }
    writer->writeEndElement();
}

void AttributesModel::readXml10(QXmlStreamReader *reader) {
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLATTRIBUTEMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "BILLATTRIBUTE" && reader->isStartElement()) {
            if(append()){
                m_d->attributesContainer.last()->loadFromXml10( reader->attributes() );
            }
        }
    }
}

void AttributesModel::readXml20(QXmlStreamReader *reader) {
    while( !reader->atEnd() &&
           !reader->hasError() &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ATTRIBUTESMODEL") ){
        reader->readNext();
        if( reader->name().toString().toUpper() == "ATTRIBUTE" && reader->isStartElement()) {
            if(append()){
                m_d->attributesContainer.last()->loadFromXml20( reader->attributes() );
            }
        }
    }
}
