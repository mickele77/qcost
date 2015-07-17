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
#ifndef ACCOUNTINGITEMATTRIBUTEMODEL_H
#define ACCOUNTINGITEMATTRIBUTEMODEL_H

class AccountingBillItem;
class AccountingTAMBillItem;
class AccountingLSBillItem;
class AttributeModel;

class AccountingItemAttributeModelModelPrivate;

#include <QAbstractTableModel>

class AccountingItemAttributeModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit AccountingItemAttributeModel(AttributeModel *attrModel, QObject *parent = 0);
    ~AccountingItemAttributeModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool insertRows(int row, int count=1);
    bool append();

    bool removeRows(int row, int count=1);
    bool clear();

    void setAttributeModel(AttributeModel *attrModel );
    void setAccountingItem(AccountingBillItem *item);
    void setAccountingItem(AccountingTAMBillItem *item);
    void setAccountingItem(AccountingLSBillItem *item);
private slots:
    void setAttributeModelNULL();
    void setAccountingItemNULL();
private:
    AccountingItemAttributeModelModelPrivate * m_d;
};

#endif // ACCOUNTINGITEMATTRIBUTEMODEL_H
