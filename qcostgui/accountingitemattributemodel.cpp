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
#include "accountingitemattributemodel.h"

#include "attributemodel.h"
#include "attribute.h"
#include "accountingbillitem.h"
#include "accountingtambillitem.h"
#include "accountinglsbillitem.h"

class AccountingItemAttributeModelModelPrivate{
public:
    AccountingItemAttributeModelModelPrivate( AttributeModel * attrModel):
        accountingBillItem(NULL),
        accountingTAMBillItem(NULL),
        accountingLSBillItem(NULL),
        attributeModel(attrModel){
    }
    AccountingBillItem * accountingBillItem;
    AccountingTAMBillItem * accountingTAMBillItem;
    AccountingLSBillItem * accountingLSBillItem;
    AttributeModel * attributeModel;
};

AccountingItemAttributeModel::AccountingItemAttributeModel(AttributeModel *attrModel, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AccountingItemAttributeModelModelPrivate(attrModel) ){
}

AccountingItemAttributeModel::~AccountingItemAttributeModel(){
    delete m_d;
}

int AccountingItemAttributeModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    if( m_d->attributeModel != NULL ){
        return m_d->attributeModel->size();
    }
    return 0;
}

int AccountingItemAttributeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return 1;
}

Qt::ItemFlags AccountingItemAttributeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return QAbstractTableModel::flags(index);

    if( m_d->accountingBillItem != NULL ){
        Attribute * attr = m_d->attributeModel->attribute(index.row() );
        if( m_d->accountingBillItem->containsAttributeInherited( attr ) ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

QVariant AccountingItemAttributeModel::data(const QModelIndex &index, int role) const {
    if( m_d->attributeModel != NULL ){
        if( !index.isValid() || !(index.row() < m_d->attributeModel->size()) ){
            return QVariant();
        }
        if( index.column() == 0 ){
            if( role == Qt::DisplayRole || role == Qt::EditRole){
                return QVariant( m_d->attributeModel->attribute(index.row() )->name() );
            } else if( role == Qt::CheckStateRole ){
                if( m_d->accountingBillItem != NULL ){
                    if( m_d->accountingBillItem->containsAttribute( m_d->attributeModel->attribute(index.row()) ) ){
                        return QVariant( Qt::Checked );
                    } else {
                        return QVariant( Qt::Unchecked );
                    }
                } else {
                    return QVariant( Qt::Unchecked );
                }
            }
        }
    }
    return QVariant();
}

bool AccountingItemAttributeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(m_d->attributeModel != NULL){
        if (index.isValid() && index.row() < m_d->attributeModel->size() ) {
            if( role == Qt::CheckStateRole ){
                if( m_d->accountingBillItem != NULL ){
                    if( index.column() == 0 ){
                        if( value.toInt() == Qt::Checked ){
                            m_d->accountingBillItem->addAttribute( m_d->attributeModel->attribute(index.row() ) );
                        } else {// if( value.toInt() == Qt::Unchecked ){
                            m_d->accountingBillItem->removeAttribute( m_d->attributeModel->attribute(index.row()) );
                        }
                        emit dataChanged( index, index );
                        return true;
                    }
                }
            }
            if( role == Qt::EditRole ){
                m_d->attributeModel->attribute( index.row() )->setName( value.toString() );
                emit dataChanged( index, index );
                return true;
            }
        }
    }
    return false;
}

QVariant AccountingItemAttributeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if(section == 0 ) {
            return trUtf8("Etichetta");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool AccountingItemAttributeModel::insertRows(int row, int count) {
    if( m_d->attributeModel != NULL ){
        if( count < 1 ){
            return false;
        }
        if( row < 0 ){
            row = 0;
        }
        if( row > m_d->attributeModel->size() ){
            row = m_d->attributeModel->size();
        }
        beginInsertRows(QModelIndex(), row, row+count-1 );
        m_d->attributeModel->insertRows( row, count );
        endInsertRows();
        return true;
    }
    return false;
}

bool AccountingItemAttributeModel::append() {
    if( m_d->attributeModel != NULL ){
        return insertRows( m_d->attributeModel->size(), 1 );
    }
    return false;
}

bool AccountingItemAttributeModel::removeRows(int row, int count) {
    if( m_d->attributeModel != NULL ){
        if( count < 1 || row < 0 || row > m_d->attributeModel->size() ){
            return false;
        }

        if( (row+count) > m_d->attributeModel->size() ){
            count = m_d->attributeModel->size() - row;
        }

        beginRemoveRows(QModelIndex(), row, row+count-1);
        m_d->attributeModel->removeRows( row, count );
        endRemoveRows();
        return true;
    }
    return false;
}

bool AccountingItemAttributeModel::clear() {
    if( m_d->attributeModel != NULL ){
        return removeRows( 0, m_d->attributeModel->size() );
    }
    return false;
}

void AccountingItemAttributeModel::setAttributeModel(AttributeModel *attrModel) {
    if( m_d->attributeModel != NULL ){
        disconnect( m_d->attributeModel, &AttributeModel::aboutToBeDeleted, this, &AccountingItemAttributeModel::setAttributeModelNULL );
    }
    beginResetModel();
    m_d->attributeModel = attrModel;
    setItemNULL();
    endResetModel();
    if( m_d->attributeModel != NULL ){
        connect( m_d->attributeModel, &AttributeModel::aboutToBeDeleted, this, &AccountingItemAttributeModel::setAttributeModelNULL );
    }
}

void AccountingItemAttributeModel::setAttributeModelNULL() {
    setAttributeModel( NULL );
}

void AccountingItemAttributeModel::setItem( AccountingBillItem * item ){
    if( m_d->accountingBillItem != item || m_d->accountingTAMBillItem != NULL || m_d->accountingLSBillItem != NULL ){
        if( m_d->accountingBillItem != NULL ){
            disconnect( m_d->accountingBillItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
        if( m_d->accountingTAMBillItem != NULL ){
            disconnect( m_d->accountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
        if( m_d->accountingLSBillItem != NULL ){
            disconnect( m_d->accountingLSBillItem, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }

        m_d->accountingBillItem = item;
        m_d->accountingTAMBillItem = NULL;
        m_d->accountingLSBillItem = NULL;

        if( m_d->attributeModel != NULL ){
            if( m_d->attributeModel->size() > 0 ){
                emit dataChanged( createIndex(0,0), createIndex(m_d->attributeModel->size(), 0) );
            }
        }
        if( m_d->accountingBillItem != NULL ){
            connect( m_d->accountingBillItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
    }
}

void AccountingItemAttributeModel::setItem( AccountingTAMBillItem * item ){
    if( m_d->accountingTAMBillItem != item || m_d->accountingBillItem != NULL || m_d->accountingLSBillItem != NULL ){
        if( m_d->accountingBillItem != NULL ){
            disconnect( m_d->accountingBillItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
        if( m_d->accountingTAMBillItem != NULL ){
            disconnect( m_d->accountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
        if( m_d->accountingLSBillItem != NULL ){
            disconnect( m_d->accountingLSBillItem, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }

        m_d->accountingBillItem = NULL;
        m_d->accountingTAMBillItem = item;
        m_d->accountingLSBillItem = NULL;

        if( m_d->attributeModel != NULL ){
            if( m_d->attributeModel->size() > 0 ){
                emit dataChanged( createIndex(0,0), createIndex(m_d->attributeModel->size(), 0) );
            }
        }
        if( m_d->accountingTAMBillItem != NULL ){
            connect( m_d->accountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
    }
}

void AccountingItemAttributeModel::setItem(AccountingLSBillItem *item) {
    if( m_d->accountingLSBillItem != item || m_d->accountingTAMBillItem != NULL || m_d->accountingBillItem != NULL ){
        if( m_d->accountingBillItem != NULL ){
            disconnect( m_d->accountingBillItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
        if( m_d->accountingTAMBillItem != NULL ){
            disconnect( m_d->accountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
        if( m_d->accountingLSBillItem != NULL ){
            disconnect( m_d->accountingLSBillItem, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
        m_d->accountingBillItem = NULL;
        m_d->accountingTAMBillItem = NULL;
        m_d->accountingLSBillItem = item;

        if( m_d->attributeModel != NULL ){
            if( m_d->attributeModel->size() > 0 ){
                emit dataChanged( createIndex(0,0), createIndex(m_d->attributeModel->size(), 0) );
            }
        }
        if( m_d->accountingLSBillItem != NULL ){
            connect( m_d->accountingLSBillItem, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemNULL );
        }
    }
}

void AccountingItemAttributeModel::setItemNULL(){
    setItem( (AccountingLSBillItem *)(NULL));
    setItem( (AccountingTAMBillItem *)(NULL));
    setItem( (AccountingBillItem *)(NULL));
}
