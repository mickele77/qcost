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
#include "billitemattributemodel.h"

#include "attributesmodel.h"
#include "attribute.h"
#include "billitem.h"

class BillItemAttributeModelPrivate{
public:
    BillItemAttributeModelPrivate(BillItem * item, AttributesModel * attrModel):
        billItem(item),
        attributeModel(attrModel){
    };
    BillItem * billItem;
    AttributesModel * attributeModel;
};

BillItemAttributeModel::BillItemAttributeModel(BillItem * item, AttributesModel * attrModel, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new BillItemAttributeModelPrivate(item, attrModel) ){
}

BillItemAttributeModel::~BillItemAttributeModel(){
    delete m_d;
}

int BillItemAttributeModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    if( m_d->attributeModel != NULL ){
        return m_d->attributeModel->size();
    }
    return 0;
}

int BillItemAttributeModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return 1;
}

Qt::ItemFlags BillItemAttributeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return QAbstractTableModel::flags(index);

    if( m_d->billItem != NULL ){
        Attribute * attr = m_d->attributeModel->attribute(index.row() );
        if( m_d->billItem->containsAttributeInherited( attr ) ){
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
}

QVariant BillItemAttributeModel::data(const QModelIndex &index, int role) const {
    if( m_d->attributeModel != NULL ){
        if( !index.isValid() || !(index.row() < m_d->attributeModel->size()) ){
            return QVariant();
        }
        if( index.column() == 0 ){
            if( role == Qt::DisplayRole || role == Qt::EditRole){
                return QVariant( m_d->attributeModel->attribute(index.row() )->name() );
            } else if( role == Qt::CheckStateRole ){
                if( m_d->billItem != NULL ){
                    if( m_d->billItem->containsAttribute( m_d->attributeModel->attribute(index.row()) ) ){
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

bool BillItemAttributeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if(m_d->attributeModel != NULL){
        if (index.isValid() && index.row() < m_d->attributeModel->size() ) {
            if( role == Qt::CheckStateRole ){
                if( m_d->billItem != NULL ){
                    if( index.column() == 0 ){
                        if( value.toInt() == Qt::Checked ){
                            m_d->billItem->addAttribute( m_d->attributeModel->attribute(index.row() ) );
                        } else {// if( value.toInt() == Qt::Unchecked ){
                            m_d->billItem->removeAttribute( m_d->attributeModel->attribute(index.row()) );
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

QVariant BillItemAttributeModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

bool BillItemAttributeModel::insertRows(int row, int count) {
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

bool BillItemAttributeModel::append() {
    if( m_d->attributeModel != NULL ){
        return insertRows( m_d->attributeModel->size(), 1 );
    }
    return false;
}

bool BillItemAttributeModel::removeRows(int row, int count) {
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

bool BillItemAttributeModel::clear() {
    if( m_d->attributeModel != NULL ){
        return removeRows( 0, m_d->attributeModel->size() );
    }
    return false;
}

void BillItemAttributeModel::setAttributeModel(AttributesModel *attrModel) {
    if( m_d->attributeModel != NULL ){
        disconnect( m_d->attributeModel, &AttributesModel::aboutToBeDeleted, this, &BillItemAttributeModel::setAttributeModelNULL );
    }
    beginResetModel();
    m_d->attributeModel = attrModel;
    m_d->billItem = NULL;
    endResetModel();
    if( m_d->attributeModel != NULL ){
        connect( m_d->attributeModel, &AttributesModel::aboutToBeDeleted, this, &BillItemAttributeModel::setAttributeModelNULL );
    }
}

void BillItemAttributeModel::setAttributeModelNULL() {
    setAttributeModel( NULL );
}

void BillItemAttributeModel::setBillItem( BillItem * item ){
    if( m_d->billItem != NULL ){
        disconnect( m_d->billItem, &BillItem::aboutToBeDeleted, this, &BillItemAttributeModel::setBillItemNULL );
    }
    m_d->billItem = item;
    if( m_d->attributeModel != NULL ){
        if( m_d->attributeModel->size() > 0 ){
            emit dataChanged( createIndex(0,0), createIndex(m_d->attributeModel->size(), 0) );
        }
    }
    if( m_d->billItem != NULL ){
        connect( m_d->billItem, &BillItem::aboutToBeDeleted, this, &BillItemAttributeModel::setBillItemNULL );
    }
}

void BillItemAttributeModel::setBillItemNULL(){
    setBillItem( NULL );
}
