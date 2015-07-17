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
#include "billattributeselectmodel.h"

#include "billattributemodel.h"
#include "billattribute.h"

class BillAttributeSelectModelPrivate{
public:
    BillAttributeSelectModelPrivate( BillAttributeModel * m ):
        model(m){
        for( int i=0; i < model->size(); ++i){
            selected << false;
        }
    };
    BillAttributeModel * model;
    QList<bool> selected;
};


BillAttributeSelectModel::BillAttributeSelectModel(BillAttributeModel *m, QObject *parent) :
    QAbstractTableModel(parent),
    m_d( new BillAttributeSelectModelPrivate(m) ){
}

BillAttributeSelectModel::~BillAttributeSelectModel(){
    delete m_d;
}

QList<BillAttribute *> BillAttributeSelectModel::selectedAttributes(){
    QList<BillAttribute *> ret;
    for( int i=0; i < m_d->selected.size(); ++i){
        if( m_d->selected.at(i) && i < m_d->model->size() ){
            ret << m_d->model->attribute(i);
        }
    }
    return ret;
}

int BillAttributeSelectModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return m_d->model->size();
}

int BillAttributeSelectModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED( parent );
    return 1;
}

Qt::ItemFlags BillAttributeSelectModel::flags(const QModelIndex &index) const {
    if( index.column() == 0 && index.isValid() ){
        return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
    }
    return QAbstractTableModel::flags(index);
}

QVariant BillAttributeSelectModel::data(const QModelIndex &index, int role) const {
    if( index.isValid() ){
        if( (role == Qt::DisplayRole) || (role == Qt::EditRole) ){
            if( index.row() < m_d->model->size() ){
                return QVariant(m_d->model->attribute(index.row() )->name());
            }
        }
        if( role == Qt::CheckStateRole ){
            if( index.row() < m_d->selected.size() ){
                if( m_d->selected.at(index.row()) ){
                    return QVariant( Qt::Checked );
                } else {
                    return QVariant( Qt::Unchecked );
                }
            }
        }
    }
    return QVariant();
}

bool BillAttributeSelectModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if( index.isValid() ){
        if( role == Qt::CheckStateRole ){
            if( index.row() < m_d->selected.size() ){
                m_d->selected.replace( index.row(), value.toInt() == Qt::Checked );
                return true;
            }
        }
    }
    return false;
}

QVariant BillAttributeSelectModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if( section == 0 ) {
            return trUtf8("Etichetta");
        }
    } else if( orientation == Qt::Vertical ){
        return QVariant( section + 1 );
    }
    return QVariant();
}
