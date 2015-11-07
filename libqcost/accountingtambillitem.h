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
#ifndef ACCOUNTINGTAMBILLITEM_H
#define ACCOUNTINGTAMBILLITEM_H

#include "qcost_export.h"

#include "accountingbillitem.h"

class AccountingTAMBillItemPrivate;

class EXPORT_QCOST_LIB_OPT AccountingTAMBillItem : public AccountingBillItem {
    Q_OBJECT
public:
    friend class AccountingTAMBill;
    AccountingTAMBillItem( AccountingTAMBillItem * parentItem, AccountingBillItem::ItemType iType, PriceFieldModel * pfm, MathParser * parser = NULL );
    ~AccountingTAMBillItem();

    /** Nel caso di lista, il titolo della lista */
    QString title() const;

    bool insertChildren(ItemType iType, int position, int count=1);

    void readXml(QXmlStreamReader *reader, PriceList *priceList, AttributeModel *attrModel);
    void writeXml(QXmlStreamWriter *writer);
};

#endif // ACCOUNTINGTAMBILLITEM_H
