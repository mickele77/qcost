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
#ifndef ACCOUNTINGLSBILL_H
#define ACCOUNTINGLSBILL_H

#include "qcost_export.h"

class AccountingPriceFieldModel;
class ProjectPriceListParentItem;
class PriceList;
class PriceItem;
class AccountingLSBillItem;
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

#include "accountingprinter.h"
#include "projectitem.h"
#include <QAbstractItemModel>

class AccountingLSBillPrivate;

class EXPORT_QCOST_LIB_OPT AccountingLSBill : public QAbstractItemModel, public ProjectItem {
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

    AccountingLSBill( const QString &c, const QString &n,
                      ProjectItem *parent, PriceFieldModel *pfm, MathParser * parser = NULL );
    AccountingLSBill( AccountingLSBill & );

    AccountingLSBill & operator= (const AccountingLSBill & cp );

    virtual ~AccountingLSBill();

    QString code() const;
    QString name() const;
    QString description() const;
    double PPUTotalToDiscount() const;
    QString PPUTotalToDiscountStr() const;
    double PPUNotToDiscount() const;
    QString PPUNotToDiscountStr() const;

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

    void setPriceList( PriceList * pl, SetPriceListMode plMode = SearchAndAdd );
    PriceList * priceList();

    int priceDataSet();

    QList<int> totalAmountPriceFields();
    void setTotalAmountPriceFields( const QList<int> & newAmountFields);
    AccountingPriceFieldModel * totalAmountPriceFieldModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertBillItems(PriceItem * p, int position = -1, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool removeItems(int position = -1, int rows = 1, const QModelIndex &parent = QModelIndex() );

    AccountingLSBillItem *item(const QModelIndex &index ) const;
    AccountingLSBillItem *item(int childNum, const QModelIndex &parentIndex = QModelIndex() );
    AccountingLSBillItem *lastItem( const QModelIndex &parentIndex = QModelIndex() );

    AccountingLSBillItem *itemId(unsigned int itemId);
    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex index(AccountingLSBillItem *item, int column) const;

    bool moveRows(const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationChild);

    double projAmount() const ;
    QString projAmountStr() const ;
    double accAmount() const;
    QString accAmountStr() const;
    double percentageAccounted() const;
    QString percentageAccountedStr() const;

    double percentageAccounted( const QDate & dBegin, const QDate & dEnd ) const;

    AttributeModel * attributeModel();
    double totalAmountAttribute(Attribute * attr);
    QString totalAmountAttributeStr(Attribute * attr);

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

    void writeODTAccountingOnTable( QTextCursor * cursor,
                                    const QDate & dateBegin, const QDate & dateEnd,
                                    AccountingPrinter::PrintLSOption prLSOption,
                                    AccountingPrinter::PrintPPUDescOption prPPUOption,
                                    bool writeAmounts = true );

    void writeODTSummaryOnTable( QTextCursor * cursor,
                                 AccountingPrinter::PrintPPUDescOption prItemsOption,
                                 bool writeAmounts = false ,
                                 bool writeDetails = true );

    void writeODTAttributeBillOnTable( QTextCursor *cursor,
                                       AccountingPrinter::AttributePrintOption prOption,
                                       AccountingPrinter::PrintPPUDescOption prPPUOption,
                                       const QList<Attribute *> &attrsToPrint,
                                       bool writeAmounts = true );
public slots:
    void setCode( const QString & n);
    void setName( const QString & n);
    void setDescription( const QString & value );
    void setPPUTotalToDiscount(const QString & newValue );
    void setPPUNotToDiscount( const QString & newValue );
    void setPriceDataSet( int );

signals:
    void aboutToBeDeleted();

    void codeChanged(  const QString & );
    void nameChanged(  const QString & );
    void descriptionChanged( const QString & );
    void PPUTotalToDiscountChanged( const QString & );
    void PPUNotToDiscountChanged( const QString & );
    void priceListChanged( PriceList * );

    void projAmountChanged( const QString & newVal );
    void accAmountChanged( const QString & newVal );
    void percentageAccountedChanged( const QString & newVal );

    void modelChanged();

private slots:
    void updateData(AccountingLSBillItem *item, int column);
private:
    AccountingLSBillPrivate * m_d;
};

#endif // ACCOUNTINGLSBILL_H
