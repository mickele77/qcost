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
#ifndef ACCOUNTINGLSBILLS_H
#define ACCOUNTINGLSBILLS_H

#include "qcost_export.h"

class AccountingLSBill;
class ProjectPriceListParentItem;
class PriceList;
class PriceItem;
class AttributesModel;
class Attribute;
class PriceFieldModel;
class MathParser;
class QXmlStreamReader;
class QXmlStreamWriter;
class QTextStream;

#include <QObject>
#include "projectitem.h"

class AccountingLSBillsPrivate;

class EXPORT_QCOST_LIB_OPT AccountingLSBills : public QObject, public ProjectItem {
    Q_OBJECT
public:
    AccountingLSBills( ProjectItem * parent, PriceFieldModel * pfm, MathParser * p = NULL );
    ~AccountingLSBills();

    int billCount();
    AccountingLSBill *bill( int i );
    AccountingLSBill *billId(unsigned int dd );

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

    /** Importo complessivo lordo */
    double projAmount() const ;
    /** Importo complessivo lordo sotto forma di stringa */
    QString projAmountStr() const ;
    /** Importo complessivo contabilizzato */
    double accAmount() const;
    /** Importo complessivo contabilizzato sotto forma di stringa */
    QString accAmountStr() const;
    /** Percentuale contabilizzata */
    double percentageAccounted() const;
    /** Percentuale contabilizzata sotto forma di stringa */
    QString percentageAccountedStr() const;
    /** Ricalcola gli importi */
    void updateAmounts();

    AttributesModel * attributesModel();
    void activateAttributeModel();
    double projAmountAttribute(Attribute * attr);
    QString projAmountAttributeStr(Attribute * attr);
    double accAmountAttribute(Attribute * attr);
    QString accAmountAttributeStr(Attribute * attr);

signals:
    void beginInsertChildren( ProjectItem * item, int first, int last );
    void endInsertChildren();
    void beginRemoveChildren( ProjectItem * item, int first, int last );
    void endRemoveChildren();
    void modelChanged();

private:
    AccountingLSBillsPrivate * m_d;
};

#endif // ACCOUNTINGLSBILLS_H
