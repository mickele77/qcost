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
class MeasuresLSModel;
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

    double projQuantity() const;
    QString projQuantityStr() const;
    double accQuantity() const;
    QString accQuantityStr() const;

    double PPU() const;
    QString PPUStr() const;

    double projAmount() const;
    QString projAmountStr() const;
    double accAmount() const;
    double accAmount(const QDate &dBegin, const QDate &dEnd) const;
    QString accAmountStr() const;
    double percentageAccounted() const;
    QString percentageAccountedStr() const;

    double percentageAccounted(const QDate &dBegin, const QDate &dEnd) const;
    static int percentagePrecision();

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

    MeasuresLSModel *measuresModel();

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
    double projAmountAttribute(Attribute * attr);
    QString projAmountAttributeStr( Attribute * attr );
    double accAmountAttribute(Attribute * attr);
    QString accAmountAttributeStr( Attribute * attr );

    void writeODTBillOnTable( QTextCursor * cursor,
                              AccountingPrinter::PrintPPUDescOption prItemsOption,
                              bool writeAmounts);
    void writeODTSummaryOnTable(QTextCursor *cursor,
                                AccountingPrinter::PrintPPUDescOption prItemsOption,
                                bool writeAmounts,
                                bool writeDetails);
    void writeODTAttributeBillOnTable( QTextCursor *cursor,
                                       AccountingPrinter::AttributePrintOption prOption,
                                       AccountingPrinter::PrintPPUDescOption prItemsOption,
                                       const QList<Attribute *> &attrsToPrint,
                                       bool writeAmounts = false );


public slots:
    void setCurrentPriceDataSet( int );
    void setName(const QString &newName);
    void setPriceItem( PriceItem * p );

signals:
    // segnale emesso quando l'oggetto cambia
    void itemChanged();
    // segnale emesso quando l'oggetto sta per essere eliminato
    void aboutToBeDeleted();
    // segnale emesso se sono stati aggiunti o tolti figli
    void hasChildrenChanged( bool );
    // segnale emesso quando alcuni figli dell'oggetto sono cambiati
    void hasChildrenChanged( AccountingLSBillItem *, QList<int> );
    // segnale emesso quanto cambia un dato
    void dataChanged( AccountingLSBillItem * );
    // segnale emesso quanto cambia il dato della colonna col
    void dataChanged( AccountingLSBillItem *, int col );
    void nameChanged( const QString & );
    void priceItemChanged( PriceItem * oldPriceItem, PriceItem * newPriceItem );
    void currentPriceDataSetChanged( int newPriceDataSet );
    void PPUChanged( const QString & );
    void projQuantityChanged( const QString &  );
    void projAmountChanged( const QString & );
    void accQuantityChanged( const QString &  );
    void accAmountChanged( const QString & );
    void percentageAccountedChanged( const QString & );
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
                             double * itemProjTotalQuantity, double *itemProjTotalAmount, double *itemAccTotalQuantity, double *itemAccTotalAmount,
                             bool writeAmounts,
                             bool writeDetails,
                             QTextTable *table,
                             QTextBlockFormat &tagBlockFormat,
                             QTextBlockFormat & txtBlockFormat,
                             QTextBlockFormat & numBlockFormat,
                             QTextTableCellFormat & leftFormat,
                             QTextTableCellFormat & centralFormat,
                             QTextTableCellFormat & rightFormat );
    void writeODTAttributeBillLineSimple( AccountingPrinter::PrintPPUDescOption prItemsOption,
                                          double *itemProjTotalAmount, double *itemAccTotalAmount,
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
    void writeODTAttributeBillLineIntersection( AccountingPrinter::PrintPPUDescOption prItemsOption,
                                                double *itemProjTotalAmount, double *itemAccTotalAmount,
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
    void writeODTAttributeBillLineUnion( AccountingPrinter::PrintPPUDescOption prItemsOption,
                                         double *itemProjTotalAmount, double *itemAccTotalAmount,
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
    void writeODTBillLine(AccountingPrinter::PrintPPUDescOption prItemsOption,
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
    void updatePPU();
private slots:
    void emitPriceDataUpdated();
    void setUnitMeasure( UnitMeasure * ump );
    void updateProjQuantityPrivate();
    void updateAccQuantityPrivate();

    void updateProjAmount();
    void updateAccAmount();
    void updatePercentageAccounted();
};

#endif // ACCOUNTINGLSBILLITEM_H
