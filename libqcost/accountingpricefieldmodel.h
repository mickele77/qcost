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
#ifndef ACCOUNTINGPRICEFIELDMODEL_H
#define ACCOUNTINGPRICEFIELDMODEL_H

#include "library_common.h"

class AccountingPriceFieldModelPrivate;
class PriceFieldModel;

#include <QAbstractTableModel>

class EXPORT_LIB_OPT AccountingPriceFieldModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit AccountingPriceFieldModel(QList<int> *selList, PriceFieldModel * pfm, QObject *parent = 0);
    ~AccountingPriceFieldModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);



    void setPriceFields(const QList<int> &newAmountFields);
signals:
    void modelChanged();

private:
    AccountingPriceFieldModelPrivate * m_d;
};

#endif // ACCOUNTINGPRICEFIELDMODEL_H
