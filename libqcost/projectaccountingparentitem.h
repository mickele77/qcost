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
#ifndef PROJECTACCOUNTINGPARENTITEM_H
#define PROJECTACCOUNTINGPARENTITEM_H

#include "qcost_export.h"

class PaymentDataModel;
class ProjectPriceListParentItem;
class AccountingBills;
class AccountingLSBills;
class AccountingTAMBill;
class PriceList;
class PriceItem;
class AttributeList;
class PriceFieldModel;
class MathParser;
class PaymentData;

class QXmlStreamReader;
class QXmlStreamWriter;
class QTextStream;

#include <QObject>
#include "projectrootitem.h"

class ProjectAccountingParentItemPrivate;

class EXPORT_QCOST_LIB_OPT ProjectAccountingParentItem : public ProjectRootItem {
    Q_OBJECT
public:
    ProjectAccountingParentItem( ProjectItem * parent, PriceFieldModel * pfm, MathParser * p = NULL );
    ~ProjectAccountingParentItem();

    int paymentDatasCount();
    PaymentData * paymentData( int pos );
    AccountingBills * accountingBills();
    AccountingLSBills * lumpSumBills();
    AccountingTAMBill * timeAndMaterialBill();

    ProjectItem *child(int number);
    int childCount() const;

    int childNumber( ProjectItem *item );

    bool canChildrenBeInserted();

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    bool clear();

    bool isUsingPriceList( PriceList * pl );
    bool isUsingPriceItem(PriceItem * p );

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader, ProjectPriceListParentItem *priceLists);

    PaymentDataModel *dataModel();

    void updateAmounts();

    double totalAmountToDiscount();
    double amountNotToDiscount();
    double amountToDiscount();
    double amountDiscounted();
    double totalAmount();

    QString totalAmountToDiscountStr();
    QString amountNotToDiscountStr();
    QString amountToDiscountStr();
    QString amountDiscountedStr();
    QString totalAmountStr();

public slots:
    void insertPayments(int position, int count);
    void removeBills(int position, int count);
    void changeBillDateEnd( const QDate & newDate, int position);
    void changeBillDateBegin( const QDate & newDate, int position);

signals:
    void beginInsertChildren( ProjectItem * item, int first, int last );
    void endInsertChildren();
    void beginRemoveChildren( ProjectItem * item, int first, int last );
    void endRemoveChildren();
    void modelChanged();

private:
    ProjectAccountingParentItemPrivate * m_d;
};

#endif // PROJECTACCOUNTINGPARENTITEM_H
