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
#include "accountingpricefieldmodel.h"

#include "pricefieldmodel.h"

class AccountingPriceFieldModelPrivate{
public:
    AccountingPriceFieldModelPrivate(QList<int> * selList, PriceFieldModel * pfm):
        priceFieldModel(pfm),
        selectedRows(selList){
    }
    ~AccountingPriceFieldModelPrivate(){
    }

    PriceFieldModel * priceFieldModel;
    QList<int> * selectedRows;
};

AccountingPriceFieldModel::AccountingPriceFieldModel(QList<int> * selList, PriceFieldModel * pfm, QObject *parent) :
    QAbstractTableModel(parent),
    m_d(new AccountingPriceFieldModelPrivate(selList, pfm)){
}

AccountingPriceFieldModel::~AccountingPriceFieldModel() {
    delete m_d;
}

QVariant AccountingPriceFieldModel::data(const QModelIndex &index, int role) const {
    if( !index.isValid() || !(index.row() < m_d->priceFieldModel->rowCount()) ){
        return QVariant();
    }
    if( role == Qt::CheckStateRole ){
        if( index.column() == 0 ){
            if( m_d->selectedRows->contains(index.row()) ){
                return QVariant( Qt::Checked );
            } else {
                return QVariant( Qt::Unchecked );
            }
        }
    }

    if( (role == Qt::DisplayRole) ){
        if( index.column() == 0 ){
            return QVariant( m_d->priceFieldModel->priceName(index.row() ) );
        }
    }
    return QVariant();
}

QVariant AccountingPriceFieldModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal ) {
        if( section == 0 ) {
            return trUtf8("Nome campo");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}

int AccountingPriceFieldModel::rowCount(const QModelIndex & p) const {
    return m_d->priceFieldModel->rowCount(p);
}

int AccountingPriceFieldModel::columnCount(const QModelIndex &) const {
    return 1;
}

Qt::ItemFlags AccountingPriceFieldModel::flags(const QModelIndex &index) const {
    if( !index.isValid() || !(index.row() < m_d->priceFieldModel->rowCount()) ){
        return QAbstractTableModel::flags(index);
    }

    if( index.column() == 0 ){
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool AccountingPriceFieldModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && index.row() < m_d->priceFieldModel->rowCount() ) {
        if( index.column() == 0 ){
            if( role == Qt::CheckStateRole ){
                if(value.toInt() == Qt::Checked){
                    if( !(m_d->selectedRows->contains(index.row()))){
                        m_d->selectedRows->append( index.row() );
                    }
                } else { // value.toInt() != Qt::Checked
                    m_d->selectedRows->removeAll( index.row() );
                }
                emit modelChanged();
                return true;
            }
        }
    }
    return false;
}
