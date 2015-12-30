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
#ifndef PROJECT_H
#define PROJECT_H

#include "qcost_export.h"

class ProjectAccountingParentItem;
class ProjectItem;
class UnitMeasureModel;
class PriceFieldModel;
class PriceList;
class AccountingLSBills;
class AccountingTAMBill;
class Bill;
class MathParser;
class QXmlStreamReader;
class QXmlStreamWriter;

#include <QAbstractItemModel>

class ProjectPrivate;

class EXPORT_QCOST_LIB_OPT Project : public QAbstractItemModel {
    Q_OBJECT
public:
    enum SimpleProjectType{
        ProjectEmpty,
        ProjectSimple,
        ProjectHumanNetNoDiscount
    };

    explicit Project( MathParser * p = NULL, QObject *parent = 0);
    
    ~Project();

    UnitMeasureModel * unitMeasureModel();
    PriceFieldModel * priceFieldModel();

    ProjectAccountingParentItem * accounting();

    int priceListCount();
    PriceList * priceList( int i );

    int billCount();
    Bill * bill( int i );

    AccountingLSBills * accountingLSBills();
    AccountingTAMBill *accountingTAMBill();

    int columnCount(const QModelIndex &) const;
    int rowCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int position, int rows, const QModelIndex &parent);
    bool appendRow( const QModelIndex &parent);
    bool removeRows(int position, int rows, const QModelIndex &parent);
    void clear();
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int column, ProjectItem *item) const;

    ProjectItem *getItem(const QModelIndex &index) const;
    bool canChildrenBeInserted(const QModelIndex &index);

    void writeXml( QXmlStreamWriter * writer );
    void readXml( QXmlStreamReader * reader );

    void createSimpleProject( SimpleProjectType projType = ProjectEmpty );

signals:
    void modelChanged();

private slots:
    void updateData( int column, ProjectItem * item );

    void removeUnitMeasure( int row, int count );
    void removePriceItem( PriceList* pl, int position,int count, const QModelIndex & parent);
    void removePriceList(int position, int count);

    void beginInsertChildren(ProjectItem * item, int first, int last);
    void endInsertChildren();
    void beginRemoveChildren(ProjectItem * item,  int first, int last);
    void endRemoveChildren();
private:
    ProjectPrivate * m_d;
};

#endif // PROJECT_H
