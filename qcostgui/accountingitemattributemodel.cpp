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
#include "accountingitemattributemodel.h"

#include "attributesmodel.h"
#include "attribute.h"
#include "accountingbillitem.h"
#include "accountingtambillitem.h"
#include "accountinglsbillitem.h"

class AccountingItemAttributeModelModelPrivate{
public:
    AccountingItemAttributeModelModelPrivate( AttributesModel * attrModel):
        accountingBillItem(nullptr),
        accountingTAMBillItem(nullptr),
        accountingLSBillItem(nullptr),
        attributeModel(attrModel){
    }
    AccountingBillItem * accountingBillItem;
    AccountingTAMBillItem * accountingTAMBillItem;
    AccountingLSBillItem * accountingLSBillItem;
    AttributesModel * attributeModel;
};

AccountingItemAttributeModel::AccountingItemAttributeModel(AttributesModel *attrModel, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new AccountingItemAttributeModelModelPrivate(attrModel) ){
}

AccountingItemAttributeModel::~AccountingItemAttributeModel(){
    delete m_d;
}

int AccountingItemAttributeModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    if( m_d->attributeModel != nullptr ){
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

    if( m_d->accountingBillItem != nullptr ){
        Attribute * attr = m_d->attributeModel->attribute(index.row() );
        if( m_d->accountingBillItem->containsAttributeInherited( attr ) ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

QVariant AccountingItemAttributeModel::data(const QModelIndex &index, int role) const {
    if( m_d->attributeModel != nullptr ){
        if( !index.isValid() || !(index.row() < m_d->attributeModel->size()) ){
            return QVariant();
        }
        if( index.column() == 0 ){
            if( role == Qt::DisplayRole || role == Qt::EditRole){
                return QVariant( m_d->attributeModel->attribute(index.row() )->name() );
            } else if( role == Qt::CheckStateRole ){
                if( m_d->accountingBillItem != nullptr ){
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
    if(m_d->attributeModel != nullptr){
        if (index.isValid() && index.row() < m_d->attributeModel->size() ) {
            if( role == Qt::CheckStateRole ){
                if( m_d->accountingBillItem != nullptr ){
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
            return tr("Etichetta");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

bool AccountingItemAttributeModel::insertRows(int row, int count) {
    if( m_d->attributeModel != nullptr ){
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
    if( m_d->attributeModel != nullptr ){
        return insertRows( m_d->attributeModel->size(), 1 );
    }
    return false;
}

bool AccountingItemAttributeModel::removeRows(int row, int count) {
    if( m_d->attributeModel != nullptr ){
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
    if( m_d->attributeModel != nullptr ){
        return removeRows( 0, m_d->attributeModel->size() );
    }
    return false;
}

void AccountingItemAttributeModel::setAttributeModel(AttributesModel *attrModel) {
    if( m_d->attributeModel != nullptr ){
        disconnect( m_d->attributeModel, &AttributesModel::aboutToBeDeleted, this, &AccountingItemAttributeModel::setAttributeModelnullptr );
    }
    beginResetModel();
    m_d->attributeModel = attrModel;
    setItemnullptr();
    endResetModel();
    if( m_d->attributeModel != nullptr ){
        connect( m_d->attributeModel, &AttributesModel::aboutToBeDeleted, this, &AccountingItemAttributeModel::setAttributeModelnullptr );
    }
}

void AccountingItemAttributeModel::setAttributeModelnullptr() {
    setAttributeModel( nullptr );
}

void AccountingItemAttributeModel::setItem( AccountingBillItem * item ){
    if( m_d->accountingBillItem != item || m_d->accountingTAMBillItem != nullptr || m_d->accountingLSBillItem != nullptr ){
        if( m_d->accountingBillItem != nullptr ){
            disconnect( m_d->accountingBillItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
        if( m_d->accountingTAMBillItem != nullptr ){
            disconnect( m_d->accountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
        if( m_d->accountingLSBillItem != nullptr ){
            disconnect( m_d->accountingLSBillItem, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }

        m_d->accountingBillItem = item;
        m_d->accountingTAMBillItem = nullptr;
        m_d->accountingLSBillItem = nullptr;

        if( m_d->attributeModel != nullptr ){
            if( m_d->attributeModel->size() > 0 ){
                emit dataChanged( createIndex(0,0), createIndex(m_d->attributeModel->size(), 0) );
            }
        }
        if( m_d->accountingBillItem != nullptr ){
            connect( m_d->accountingBillItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
    }
}

void AccountingItemAttributeModel::setItem( AccountingTAMBillItem * item ){
    if( m_d->accountingTAMBillItem != item || m_d->accountingBillItem != nullptr || m_d->accountingLSBillItem != nullptr ){
        if( m_d->accountingBillItem != nullptr ){
            disconnect( m_d->accountingBillItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
        if( m_d->accountingTAMBillItem != nullptr ){
            disconnect( m_d->accountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
        if( m_d->accountingLSBillItem != nullptr ){
            disconnect( m_d->accountingLSBillItem, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }

        m_d->accountingBillItem = nullptr;
        m_d->accountingTAMBillItem = item;
        m_d->accountingLSBillItem = nullptr;

        if( m_d->attributeModel != nullptr ){
            if( m_d->attributeModel->size() > 0 ){
                emit dataChanged( createIndex(0,0), createIndex(m_d->attributeModel->size(), 0) );
            }
        }
        if( m_d->accountingTAMBillItem != nullptr ){
            connect( m_d->accountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
    }
}

void AccountingItemAttributeModel::setItem(AccountingLSBillItem *item) {
    if( m_d->accountingLSBillItem != item || m_d->accountingTAMBillItem != nullptr || m_d->accountingBillItem != nullptr ){
        if( m_d->accountingBillItem != nullptr ){
            disconnect( m_d->accountingBillItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
        if( m_d->accountingTAMBillItem != nullptr ){
            disconnect( m_d->accountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
        if( m_d->accountingLSBillItem != nullptr ){
            disconnect( m_d->accountingLSBillItem, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
        m_d->accountingBillItem = nullptr;
        m_d->accountingTAMBillItem = nullptr;
        m_d->accountingLSBillItem = item;

        if( m_d->attributeModel != nullptr ){
            if( m_d->attributeModel->size() > 0 ){
                emit dataChanged( createIndex(0,0), createIndex(m_d->attributeModel->size(), 0) );
            }
        }
        if( m_d->accountingLSBillItem != nullptr ){
            connect( m_d->accountingLSBillItem, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingItemAttributeModel::setItemnullptr );
        }
    }
}

void AccountingItemAttributeModel::setItemnullptr(){
    setItem( (AccountingLSBillItem *)(nullptr));
    setItem( (AccountingTAMBillItem *)(nullptr));
    setItem( (AccountingBillItem *)(nullptr));
}
