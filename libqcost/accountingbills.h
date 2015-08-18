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
#ifndef ACCOUNTINGBILLS_H
#define ACCOUNTINGBILLS_H

#include "library_common.h"

class AccountingBill;
class ProjectPriceListParentItem;
class PriceList;
class PriceItem;
class AttributeList;
class PriceFieldModel;
class MathParser;
class QXmlStreamReader;
class QXmlStreamWriter;
class QTextStream;

#include <QObject>
#include "projectitem.h"

class AccountingBillsPrivate;

class EXPORT_LIB_OPT AccountingBills : public QObject, public ProjectItem {
    Q_OBJECT
public:
    AccountingBills( ProjectItem * parent, PriceFieldModel * pfm, MathParser * p = NULL );

    int billCount();
    AccountingBill * bill( int i );

    ProjectItem *child(int number);
    int childCount() const;

    int childNumber( ProjectItem *item );

    bool canChildrenBeInserted();
    bool insertChildren(int position, int count = 1);
    bool appendChild();
    bool removeChildren(int position, int count = 1);
    bool clear();

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    bool isUsingPriceList( PriceList * pl );
    bool isUsingPriceItem(PriceItem * p );

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader, ProjectPriceListParentItem *priceLists);

    bool insertPayments( int position, int count=1 );
    bool removeBills( int position, int count=1 );

    void changeBillDateEnd( const QDate & newDate, int position);
    void changeBillDateBegin( const QDate & newDate, int position);

signals:
    void beginInsertChildren( ProjectItem * item, int first, int last );
    void endInsertChildren();
    void beginRemoveChildren( ProjectItem * item, int first, int last );
    void endRemoveChildren();
    void modelChanged();
    void insertPaymentsSignal( int position, int count );
    void removePaymentsSignal( int position, int count );
    void changePaymentDateBeginSignal( const QDate & newDate, int position );
    void changePaymentDateEndSignal( const QDate & newDate, int position );

private:
    AccountingBillsPrivate * m_d;
};

#endif //  ACCOUNTINGBILLS_H
