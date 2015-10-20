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

#include "qcost_export.h"

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
#include <QAbstractItemModel>

class AccountingBillPrivate;

/**
* @class AccountingBill
*
* @brief Classe usata per modellizzare un libretto delle misure
*
* Questa classe viene impiegata per modellizzare i libretti delle misure.
* All'interno di una contabilità è possibile avere più libretti delle misure
* (vedi D.P.R. 207/2010).
*
* @author Michele Mocciola
*
*/

class EXPORT_QCOST_LIB_OPT AccountingBill : public QAbstractItemModel, public ProjectItem {
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

    void nextId();
    unsigned int id();

    /** Nome associato al libretto delle misure */
    QString name();
    /** Informazioni testuali di vario tipo associate al libretto delle misure */
    QString description();

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
    QString discountStr();
    void setDiscount(double newVal);
    void setDiscount(const QString & newVal);

    ProjectItem *child(int number);
    int childCount() const;

    int childNumber( ProjectItem * item );

    bool clear();
    bool canChildrenBeInserted();
    bool insertChildren(int position, int count);
    bool removeChildren(int position, int count);

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertItems(AccountingBillItem::ItemType mt, int position = -1, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool insertPayments(int inputPos = -1, int count = 1);
    bool removeItems(int position = -1, int rows = 1, const QModelIndex &parent = QModelIndex() );
    bool removePayments(int position, int rows=1);

    AccountingBillItem *item(const QModelIndex &index ) const;
    AccountingBillItem *item(int childNum, const QModelIndex &parentIndex = QModelIndex() );
    AccountingBillItem *lastItem( const QModelIndex &parentIndex = QModelIndex() );
    AccountingBillItem *itemId(unsigned int itemId);

    QModelIndex parent(const QModelIndex &index) const;
    QModelIndex index(int row, int col, const QModelIndex &parent) const;
    QModelIndex index(AccountingBillItem *item, int column) const;

    bool moveRows(const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationChild);

    double totalAmountToDiscount() const;
    double amountNotToDiscount() const;
    double amountToDiscount() const;
    double amountDiscounted() const;
    double totalAmount() const;
    QString totalAmountToDiscountStr() const;
    QString amountNotToDiscountStr() const;
    QString amountToDiscountStr() const;
    QString amountDiscountedStr() const;
    QString totalAmountStr() const;

    AttributeModel * attributeModel();

    double totalAmountToDiscountAttribute( Attribute * attr ) const;
    QString totalAmountToDiscountAttributeStr( Attribute * attr ) const;
    double amountNotToDiscountAttribute( Attribute * attr ) const;
    QString amountNotToDiscountAttributeStr( Attribute * attr ) const;
    double totalAmountAttribute( Attribute * attr ) const;
    QString totalAmountAttributeStr( Attribute * attr ) const;

    bool isUsingPriceItem( PriceItem * p );
    bool isUsingPriceList( PriceList * pl );

    void setBillDateEnd( const QDate & newDate, int position);
    void setBillDateBegin( const QDate & newDate, int position);

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader, ProjectPriceListParentItem * priceLists);
    void loadFromXml(const QXmlStreamAttributes &attrs, ProjectPriceListParentItem * priceLists);
    void loadTmpData( PriceList *priceList );

    void loadTmpData(ProjectPriceListParentItem *priceLists , AccountingLSBills *lsBills, AccountingTAMBill *tamBill);
    void loadFromXmlTmp(const QXmlStreamAttributes &attrs);
    void setTmpData(ProjectPriceListParentItem *priceLists);

    QList<PriceItem *> connectedPriceItems();

    /**
     * @brief Stampa il libretto delle misure
     * @param cursor
     * @param payToPrint S.A.L. da stampare (se è < 0 li stampa tutti)
     * @param prAmountsOption opzione per la stampa degli importi
     * @param prPPUDescOption opzione per la stampa dei prezzi
     */
    void writeODTAccountingOnTable( QTextCursor * cursor,
                                    int payToPrint,
                                    AccountingPrinter::PrintAmountsOption prAmountsOption,
                                    AccountingPrinter::PrintPPUDescOption prPPUDescOption) const;
    /**
     * @brief Stampa il S.A.L.
     * @param cursor
     * @param payToPrint S.A.L. da stampare (se è < 0 li stampa tutti)
     * @param prPPUDescOption opzione per la stampa dei prezzi
     */
    void writeODTPaymentOnTable( QTextCursor * cursor,
                                 int payToPrint,
                                 AccountingPrinter::PrintPPUDescOption prPPUDescOption) const;

    /**
     * @brief Stampa il sommario della contabilità
     * @param cursor
     * #param payToPrint S.A.L. da stampare (se è < 0 li stampa tutti)
     * @param prAmountsOption
     * @param prItemsOption
     * @param writeDetails scrive i dettaglio del sommario
     */
    void writeODTSummaryOnTable( QTextCursor * cursor,
                                 int payToPrint,
                                 AccountingPrinter::PrintAmountsOption prAmountsOption,
                                 AccountingPrinter::PrintPPUDescOption prItemsOption,
                                 bool writeDetails = true ) const;

    void writeODTAttributeAccountingOnTable( QTextCursor *cursor,
                                             AccountingPrinter::AttributePrintOption prOption,
                                             AccountingPrinter::PrintAmountsOption prAmountsOption,
                                             AccountingPrinter::PrintPPUDescOption prItemsOption,
                                             const QList<Attribute *> &attrsToPrint ) const;

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

    void requestInsertBills( int position, int count );
    void requestRemoveBills( int position, int count );
    void requestDateBeginChange( const QDate &newDate, int position );
    void requestDateEndChange( const QDate &newDate, int position );

    void totalAmountToDiscountChanged( const QString & newVal );
    void amountNotToDiscountChanged( const QString & newVal );
    void amountToDiscountChanged( const QString & newVal );
    void amountDiscountedChanged( const QString & newVal );
    void totalAmountChanged( const QString & newVal );

    void modelChanged();

    void discountChanged( const QString & );

private slots:
    void updateValue(AccountingBillItem *item, int column);

private:
    AccountingBillPrivate * m_d;
};

#endif // ACCOUNTINGBILL_H
