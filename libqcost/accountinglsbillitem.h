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
#ifndef ACCOUNTINGLSBILLITEM_H
#define ACCOUNTINGLSBILLITEM_H

#include "library_common.h"

class AccountingPriceFieldModel;
class PriceList;
class PriceItem;
class MeasuresModel;
class PriceFieldModel;
class MathParser;
class AttributeModel;
class Attribute;
class UnitMeasure;

class QTextCursor;
class QTextStream;
class QTextTable;
class QTextTableCellFormat;
class QTextBlockFormat;
class QTextCharFormat;
class QXmlStreamAttributes;
class QXmlStreamWriter;
class QXmlStreamReader;

#include "accountingprinter.h"
#include "treeitem.h"
#include <QObject>

class AccountingLSBillItemPrivate;

class EXPORT_LIB_OPT AccountingLSBillItem :  public QObject, public TreeItem {
    Q_OBJECT
public:
    friend class Bill;
    AccountingLSBillItem( PriceItem * p, AccountingLSBillItem * parentItem, PriceFieldModel * pfm, MathParser * parser = NULL );
    ~AccountingLSBillItem();

    AccountingLSBillItem &operator =(const AccountingLSBillItem &cp);

    AccountingLSBillItem * parent();
    /** ricerca tra gli oggetti figlio uno con id pari a itemId */
    AccountingLSBillItem * itemId(unsigned int itemId);
    AccountingLSBillItem * childItem(int number);
    bool isDescending( AccountingLSBillItem * ancestor );
    void setParent(AccountingLSBillItem *newParent, int position);

    unsigned int id();
    QString progressiveCode() const;
    QString name();
    int currentPriceDataSet() const;
    PriceItem * priceItem();
    // dice se il prezzo è usato dall'articolo di computo o da un suo sottoarticolo
    bool isUsingPriceItem( PriceItem * p );
    // restituisce l'elenco degli articoli di prezzo contenuti nell'articolo di computo o nei sottoarticoli
    QList<PriceItem *> usedPriceItems() const;
    // restituisce i sottoarticoli del prezzo associato all'articolo di computo, oltre ad eventuali
    // altri prezzi connessi tramite analisi prezzi
    QList<PriceItem *> connectedPriceItems() const;

    double quantity() const;
    QString quantityStr() const;
    double PPUTotal() const;
    QString PPUTotalStr() const;
    double totalAmount() const;
    QString totalAmountStr() const;

    QList<int> totalAmountPriceFields();
    void setTotalAmountPriceFields(const QList<int> &newAmountFields);
    AccountingPriceFieldModel *totalAmountPriceFieldModel();

    int columnCount() const;
    QList<AccountingLSBillItem *> allChildren();
    QList<AccountingLSBillItem *> allChildrenWithMeasures();
    QVariant data(int col, int role = Qt::EditRole ) const;
    bool setData(int column, const QVariant &quantity);
    Qt::ItemFlags flags(int column) const;

    TreeItem *child(int number);
    int childrenCount() const;
    bool hasChildren() const;
    bool insertChildren(PriceItem *p, int position, int count);
    bool insertChildren( int position, int count);
    bool appendChildren( int count=1 );
    bool removeChildren(int position, int count);
    bool reset();
    int childNumber() const;

    MeasuresModel * measuresModel();
    MeasuresModel * generateMeasuresModel();
    void removeMeasuresModel();

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader, PriceList *priceList, AttributeModel * billAttrModel);
    void readXmlTmp(QXmlStreamReader *reader);
    void loadFromXml(const QXmlStreamAttributes &attrs, PriceList *priceList, AttributeModel * billAttrModel);
    void loadFromXmlTmp(const QXmlStreamAttributes &attrs);
    void loadTmpData(PriceList *priceList , AttributeModel *billAttrModel);

    bool containsAttribute( Attribute * attr );
    bool containsAttributeInherited( Attribute * attr );
    bool containsAttributeDirect( Attribute * attr );

    void addAttribute( Attribute * attr );
    void removeAttribute( Attribute * attr );
    void removeAllAttributes();
    double totalAmountAttribute(Attribute * attr);
    QString totalAmountAttributeStr( Attribute * attr );

    void writeODTBillOnTable( QTextCursor * cursor,
                              AccountingPrinter::PrintAccountingBillOption prItemsOption,
                              bool writeAmounts);
    void writeODTSummaryOnTable(QTextCursor *cursor,
                                AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                bool writeAmounts,
                                bool writeDetails);
    void writeODTAttributeBillOnTable( QTextCursor *cursor,
                                       AccountingPrinter::AttributePrintOption prOption,
                                       AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                       const QList<Attribute *> &attrsToPrint,
                                       bool writeAmounts = false );


public slots:
    void setCurrentPriceDataSet( int );
    void setName(const QString &newName);
    void setPriceItem( PriceItem * p );
    void setQuantity( double );
    void setQuantity( const QString & );

signals:
    void itemChanged();
    void aboutToBeDeleted();
    // segnale emesso se sono stati aggiunti o tolti figli
    void hasChildrenChanged( bool );
    // segnale emesso quando alcuni figli dell'oggetto sono cambiati
    void hasChildrenChanged( AccountingLSBillItem *, QList<int> );
    void dataChanged( AccountingLSBillItem * );
    void dataChanged( AccountingLSBillItem *, int column);
    void nameChanged( const QString & );
    void priceItemChanged( PriceItem * oldPriceItem, PriceItem * newPriceItem );
    void currentPriceDataSetChanged( int newPriceDataSet );
    void quantityChanged( const QString &  );
    void PPUTotalChanged( const QString & );
    void totalAmountChanged( const QString & );
    void attributesChanged();

private:
    AccountingLSBillItemPrivate * m_d;

    /** Ricerca all'interno del computo un AccountingLSBillItem con id pari a itemId */
    AccountingLSBillItem *findItemId(unsigned int itId);

    void setId( unsigned int ii );

    void setHasChildrenChanged(AccountingLSBillItem * p , QList<int> indexes);
    TreeItem * parentInternal();

    void addChild(AccountingLSBillItem *newChild, int position);
    void removeChild(int position);

    void appendUsedPriceItems( QList<PriceItem *> * usedPriceItems ) const;

    void writeODTSummaryLine(PriceItem * priceItem,
                             QTextCursor *cursor,
                             double * itemTotalQuantity, double *itemTotalAmount,
                             bool writeAmounts,
                             bool writeDetails,
                             QTextTable *table,
                             QTextBlockFormat &tagBlockFormat,
                             QTextBlockFormat & txtBlockFormat,
                             QTextBlockFormat & numBlockFormat,
                             QTextTableCellFormat & leftFormat,
                             QTextTableCellFormat & centralFormat,
                             QTextTableCellFormat & rightFormat );
    void writeODTAttributeBillLineSimple(AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                          double *itemTotalAmount,
                                          Attribute * attrsToPrint,
                                          bool writeAmounts,
                                          QTextCursor *cursor,
                                          QTextTable *table,
                                          QTextBlockFormat &tagBlockFormat,
                                          QTextBlockFormat & txtBlockFormat,
                                          QTextBlockFormat & numBlockFormat,
                                          QTextTableCellFormat & leftFormat,
                                          QTextTableCellFormat & centralFormat,
                                          QTextTableCellFormat & rightFormat,
                                          QTextTableCellFormat & centralQuantityTotalFormat,
                                          QTextTableCellFormat & rightQuantityTotalFormat,
                                          QTextCharFormat &txtCharFormat,
                                          QTextCharFormat &txtBoldCharFormat );
    void writeODTAttributeBillLineIntersection( AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                                double *itemTotalAmount,
                                                const QList<Attribute *> &attrsToPrint,
                                                bool writeAmounts,
                                                QTextCursor *cursor,
                                                QTextTable *table,
                                                QTextBlockFormat &tagBlockFormat,
                                                QTextBlockFormat & txtBlockFormat,
                                                QTextBlockFormat & numBlockFormat,
                                                QTextTableCellFormat & leftFormat,
                                                QTextTableCellFormat & centralFormat,
                                                QTextTableCellFormat & rightFormat,
                                                QTextTableCellFormat & centralQuantityTotalFormat,
                                                QTextTableCellFormat & rightQuantityTotalFormat,
                                                QTextCharFormat &txtCharFormat,
                                                QTextCharFormat &txtBoldCharFormat );
    void writeODTAttributeBillLineUnion(AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                         double *itemTotalAmount,
                                         bool writeAmounts,
                                         const QList<Attribute *> &attrsToPrint,
                                         QTextCursor *cursor,
                                         QTextTable *table,
                                         QTextBlockFormat &tagBlockFormat,
                                         QTextBlockFormat & txtBlockFormat,
                                         QTextBlockFormat & numBlockFormat,
                                         QTextTableCellFormat & leftFormat,
                                         QTextTableCellFormat & centralFormat,
                                         QTextTableCellFormat & rightFormat,
                                         QTextTableCellFormat & centralQuantityTotalFormat,
                                         QTextTableCellFormat & rightQuantityTotalFormat,
                                         QTextCharFormat &txtCharFormat,
                                         QTextCharFormat &txtBoldCharFormat);
    void writeODTBillLine(AccountingPrinter::PrintAccountingBillOption prItemsOption,
                          bool writeProgCode,
                          bool writeAmounts,
                          QTextCursor *cursor,
                          QTextTable *table,
                          QTextBlockFormat &tagBlockFormat,
                          QTextBlockFormat & txtBlockFormat,
                          QTextBlockFormat & numBlockFormat,
                          QTextTableCellFormat & leftFormat,
                          QTextTableCellFormat & centralFormat,
                          QTextTableCellFormat & rightFormat,
                          QTextTableCellFormat & centralQuantityTotalFormat,
                          QTextTableCellFormat & rightQuantityTotalFormat,
                          QTextCharFormat & txtCharFormat,
                          QTextCharFormat & txtBoldCharFormat );

    /** restituisce tutti gli attributi dell'elemento; se la var bool è false,
        vuol dire che l'attribute è ereditato, se è true vuol dire che è un
        attributo direttamente imposto */
    QList< QPair<Attribute *, bool> > attributes();
    QList<Attribute *> allAttributes();
    QList<Attribute *> directAttributes();
    QList<Attribute *> inheritedAttributes();
private slots:
    void emitPriceDataUpdated();
    void setUnitMeasure( UnitMeasure * ump );
    void updateTotalAmount();
    void setQuantityPrivate(double v);
};

#endif // ACCOUNTINGLSBILLITEM_H
