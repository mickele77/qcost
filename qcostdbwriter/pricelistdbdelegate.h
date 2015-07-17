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
#ifndef PRICELISTDBDELEGATE_H
#define PRICELISTDBDELEGATE_H

#include <QStyledItemDelegate>

class PriceListDBDelegatePrivate;
class PriceListDBModel;

class PriceListDBDelegate : public QStyledItemDelegate
{
public:
    explicit PriceListDBDelegate(PriceListDBModel *m, QObject *parent = 0);
    ~PriceListDBDelegate();

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    void setEditorData ( QWidget *editor, const QModelIndex &index ) const;
    void setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

private:
    PriceListDBDelegatePrivate * m_d;
};

#endif // PRICELISTDBDELEGATE_H
