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
#include "pricelistdbdelegate.h"

#include "pricelistdbmodel.h"

#include <QComboBox>
#include <QVariant>

class PriceListDBDelegatePrivate{
public:
    PriceListDBDelegatePrivate(PriceListDBModel * m ):
        model(m){
    };
    PriceListDBModel * model;
};

PriceListDBDelegate::PriceListDBDelegate(PriceListDBModel *m, QObject *parent) :
    QStyledItemDelegate(parent),
    m_d( new PriceListDBDelegatePrivate(m)) {
}

PriceListDBDelegate::~PriceListDBDelegate() {
}

QWidget *PriceListDBDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if(index.column() == m_d->model->unitMeasureColumn() ){
        QComboBox *cb = new QComboBox(parent);
        QList< QPair<QString, int> > itemsList = m_d->model->unitMeasureList();
        for(int i=0; i < itemsList.size(); ++i ){
            cb->addItem( itemsList.at(i).first, QVariant(itemsList.at(i).second) );
        }
        return cb;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void PriceListDBDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    if(QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        QVariant v = index.data(Qt::EditRole);
        int cbIndex = cb->findData( v );
        if(cbIndex >= 0){
            cb->setCurrentIndex(cbIndex);
        }
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void PriceListDBDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if(QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, cb->itemData( cb->currentIndex() ), Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
