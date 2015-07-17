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
#include "pricelistdelegate.h"

#include "unitmeasuremodel.h"
#include "unitmeasure.h"
#include <QComboBox>
#include <QVariant>

class PriceListDelegatePrivate{
public:
    PriceListDelegatePrivate(UnitMeasureModel * uml):
        unitMeasureModel(uml){
    };
    UnitMeasureModel * unitMeasureModel;
};

PriceListDelegate::PriceListDelegate(UnitMeasureModel * uml, QObject *parent) :
    QStyledItemDelegate(parent),
    m_d( new PriceListDelegatePrivate(uml)) {
}

PriceListDelegate::~PriceListDelegate() {
    delete m_d;
}

QWidget *PriceListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if(index.column() != 2){
        return QStyledItemDelegate::createEditor(parent, option, index);
    } else {
        QComboBox *cb = new QComboBox(parent);
        cb->addItem( QString(), qVariantFromValue( (void *) NULL ) );
        for(int i=0; i < m_d->unitMeasureModel->size(); ++i ){
            UnitMeasure * ump = m_d->unitMeasureModel->unitMeasure(i);
            QVariant umpVar = qVariantFromValue( (void *) ump );
            cb->addItem( ump->tag(), umpVar );
        }
        return cb;
    }
}

void PriceListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
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

void PriceListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    if(QComboBox *cb = qobject_cast<QComboBox *>(editor)) {
        model->setData(index, cb->itemData( cb->currentIndex() ), Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
