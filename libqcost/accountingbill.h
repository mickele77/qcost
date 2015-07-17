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
#ifndef ACCOUNTINGBILL_H
#define ACCOUNTINGBILL_H

#include "library_common.h"

class ProjectPriceListParentItem;
class PriceList;
class PriceItem;
class Attribute;
class AttributeModel;
class PriceFieldModel;
class MathParser;
class QXmlStreamAttributes;
class QXmlStreamWriter;
class QXmlStreamReader;
class QTextStream;
class QString;
class QTextCursor;
class AccountingPriceFieldModel;

#include "accountingbillitem.h"
#include "accountingprinter.h"
#include "projectitem.h"
#include <QAbstractTableModel>

class AccountingBillPrivate;

/**

*/

class EXPORT_LIB_OPT AccountingBill : public QAbstractTableModel, public ProjectItem {
    Q_OBJECT
public:
    enum SetPriceListMode{
        None,
        SearchAndAdd,
        Add,
        Search,
        NULLPriceItem,
        ResetBill
    };

    AccountingBill( const QString & n, ProjectItem *parent, PriceFieldModel *pfm, MathParser * parser = NULL );
    AccountingBill( AccountingBill & );

    AccountingBill & operator= (const AccountingBill & cp );

    virtual ~AccountingBill();

    QString name();
    QString description();

    ProjectItem *child(int number);
    int childCount() const;

    int childNumber( ProjectItem * item );

    bool reset();
    bool canChildrenBeInserted();
    bool insertChildren(int position, int count);
    bool removeChildren(int position, int count);

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    QList<int> totalAmountPriceFields();
    void setTotalAmountPriceFields( const QList<int> & newAmountFields);
    AccountingPriceFieldModel * totalAmountPriceFieldModel();
    QList<int> noDiscountAmountPriceFields();
    void setNoDiscountAmountPriceFields( const QList<int> & newAmountFields);
    AccountingPriceFieldModel * noDiscountAmountPriceFieldModel();

    void setPriceList( PriceList * pl, SetPriceListMode plMode = SearchAndAdd );
    PriceList * priceList();

    int priceDataSet();

    double discount();
    void setDiscount(double newVal);

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertItems(AccountingBillItem::ItemType mt, int position = -1, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool removeItems(int position = -1, int rows = 1, const QModelIndex &parent = QModelIndex() );

    AccountingBillItem *item(const QModelIndex &index ) const;
    AccountingBillItem *item(int childNum, const QModelIndex &parentIndex = QModelIndex() );
    AccountingBillItem *lastAccountingMeasure( const QModelIndex &parentIndex = QModelIndex() );

    AccountingBillItem *itemId(unsigned int itemId);
    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex index(AccountingBillItem *item, int column) const;

    bool moveRows(const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationChild);

    double totalAmountToBeDiscounted() const;
    double amountNotToBeDiscounted() const;
    double amountToBeDiscounted() const;
    double amountDiscounted() const;
    double totalAmount() const;
    QString totalAmountToBeDiscountedStr() const;
    QString amountNotToBeDiscountedStr() const;
    QString amountToBeDiscountedStr() const;
    QString amountDiscountedStr() const;
    QString totalAmountStr() const;

    AttributeModel * attributeModel();

    double totalAmountToBeDiscountedAttribute( Attribute * attr ) const;
    QString totalAmountToBeDiscountedAttributeStr( Attribute * attr ) const;
    double amountNotToBeDiscountedAttribute( Attribute * attr ) const;
    QString amountNotToBeDiscountedAttributeStr( Attribute * attr ) const;
    double totalAmountAttribute( Attribute * attr ) const;
    QString totalAmountAttributeStr( Attribute * attr ) const;

    bool isUsingPriceItem( PriceItem * p );
    bool isUsingPriceList( PriceList * pl );

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists);
    void readXmlTmp(QXmlStreamReader *reader);
    void loadFromXml(const QXmlStreamAttributes &attrs, ProjectPriceListParentItem * priceLists);
    void loadTmpData( PriceList *priceList );

    void nextId();
    unsigned int id();

    void loadTmpData( ProjectPriceListParentItem *priceLists );
    void loadFromXmlTmp(const QXmlStreamAttributes &attrs);
    void setTmpData(ProjectPriceListParentItem *priceLists);

    QList<PriceItem *> connectedPriceItems();

    void writeODTAccountingOnTable(QTextCursor * cursor,
                                   AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                   bool printAmounts = true ) const;
    void writeODTSummaryOnTable( QTextCursor * cursor,
                                 AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                 bool printAmounts = true,
                                 bool writeDetails = true ) const;

    void writeODTAttributeAccountingOnTable(QTextCursor *cursor,
                                            AccountingPrinter::AttributePrintOption prOption,
                                            AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                            const QList<Attribute *> &attrsToPrint,
                                            bool printAmounts = true ) const;
    void insertStandardAttributes();

public slots:
    void setName( const QString & n);
    void setDescription( const QString & value );
    void setPriceDataSet( int );

signals:
    void aboutToBeDeleted();
    void nameChanged(  const QString & );
    void descriptionChanged(  const QString & );
    void priceListChanged( PriceList * );

    void totalAmountToBeDiscountedChanged( const QString & newVal );
    void amountNotToBeDiscountedChanged( const QString & newVal );
    void amountToBeDiscountedChanged( const QString & newVal );
    void amountDiscountedChanged( const QString & newVal );
    void totalAmountChanged( const QString & newVal );

    void modelChanged();

    void discountChanged( double );

private slots:
    void updateValue(AccountingBillItem *item, int column);

private:
    AccountingBillPrivate * m_d;
};

#endif // ACCOUNTINGBILL_H
