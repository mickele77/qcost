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
#ifndef BILLITEM_H
#define BILLITEM_H

#include "qcost_export.h"

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

#include "billprinter.h"
#include "treeitem.h"
#include <QObject>

class BillItemPrivate;

class EXPORT_QCOST_LIB_OPT BillItem :  public QObject, public TreeItem {
    Q_OBJECT
public:
    friend class Bill;
    BillItem( PriceItem * p, BillItem * parentItem, PriceFieldModel * pfm, MathParser * parser = NULL );
    ~BillItem();

    BillItem &operator =(const BillItem &cp);

    int firstPriceFieldCol();

    BillItem * parent();
    /** ricerca tra gli oggetti figlio uno con id pari a itemId */
    BillItem * itemFromId(unsigned int itemId);
    /** Ricerca all'interno del computo un BillItem con id pari a itemId */
    BillItem *findItemFromId(unsigned int itemId);
    /** ricerca tra gli oggetti figlio uno con progCode pari a pCode */
    BillItem * itemFromProgCode(const QString &pCode);
    /** Ricerca all'interno del computo un BillItem con progCode pari a pCode */
    BillItem *findItemFromProgCode(const QString & pCode);
    /** Restituisce elemento l'elemento figlio numero number */
    BillItem * childItem(int number);
    /** Ci dice se l'elemento discende dall'elemento ancestor */
    bool isDescending( BillItem * ancestor );
    /** Imposta il genitore dell'oggetto a newParent */
    void setParent(BillItem *newParent, int position);

    unsigned int id();
    QString progCode() const;
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
    double amount( int field ) const;
    QString amountStr( int field ) const;

    int columnCount() const;
    QList<BillItem *> allChildren();
    QList<BillItem *> allChildrenWithMeasures();
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

    void appendConnectedItems( QList<BillItem *> * itemsList );

    void writeXml( QXmlStreamWriter * writer );
    void readXmlTmp(QXmlStreamReader *reader);
    void readFromXmlTmp(PriceList *priceList , AttributeModel *billAttrModel);

    bool containsAttribute( Attribute * attr );
    bool containsAttributeInherited( Attribute * attr );
    bool containsAttributeDirect( Attribute * attr );

    void addAttribute( Attribute * attr );
    void removeAttribute( Attribute * attr );
    void removeAllAttributes();
    double amountAttribute( Attribute * attr, int field );
    QString amountAttributeStr( Attribute * attr, int field );

    void writeODTBillOnTable( QTextCursor * cursor,
                              BillPrinter::PrintBillItemsOption prItemsOption,
                              const QList<int> fieldsToPrint ,
                              bool groupPrAm);
    void writeODTSummaryOnTable( QTextCursor *cursor,
                                 BillPrinter::PrintBillItemsOption prItemsOption,
                                 const QList<int> fieldsToPrint,
                                 bool groupPrAm,
                                 bool writeDetails);
    void writeODTAttributeBillOnTable( QTextCursor *cursor,
                                       BillPrinter::AttributePrintOption prOption,
                                       BillPrinter::PrintBillItemsOption prItemsOption,
                                       const QList<int> &fieldsToPrint,
                                       const QList<Attribute *> &attrsToPrint,
                                       bool groupPrAm = false );


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
    void hasChildrenChanged( BillItem *, QList<int> );
    void dataChanged( BillItem * );
    void dataChanged( BillItem *, int column);
    void nameChanged( const QString & );
    void priceItemChanged( PriceItem * oldPriceItem, PriceItem * newPriceItem );
    void currentPriceDataSetChanged( int newPriceDataSet );
    void quantityChanged( const QString &  );
    void amountChanged( int, const QString & );
    void amountChanged( int, double );
    void attributesChanged();

private:
    BillItemPrivate * m_d;

    void setId( unsigned int ii );

    void setHasChildrenChanged(BillItem * p , QList<int> indexes);
    TreeItem * parentInternal();

    void addChild(BillItem *newChild, int position);
    void removeChild(int position);
    void insertAmount(int pf);
    void removeAmount(int pf);
    void updateAmount(int pf);

    void appendUsedPriceItems( QList<PriceItem *> * usedPriceItems ) const;

    void loadXml(const QXmlStreamAttributes &attrs, PriceList *priceList, AttributeModel * billAttrModel);

    void writeODTSummaryLine(PriceItem * priceItem,
                             QTextCursor *cursor,
                             const QList<int> fieldsToPrint,
                             double * itemTotalQuantity,
                             QList<double> * fieldsValue,
                             bool writeDetails,
                             QTextTable *table,
                             QTextBlockFormat &tagBlockFormat,
                             QTextBlockFormat & txtBlockFormat,
                             QTextBlockFormat & numBlockFormat,
                             QTextTableCellFormat & leftFormat,
                             QTextTableCellFormat & centralFormat,
                             QTextTableCellFormat & rightFormat);

    void writeODTAttributeBillLineSimple( BillPrinter::PrintBillItemsOption prItemsOption,
                                          QList<double> * fieldsAmounts,
                                          const QList<int> &fieldsToPrint,
                                          Attribute * attrsToPrint,
                                          bool groupPrAm,
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
    void writeODTAttributeBillLineIntersection( BillPrinter::PrintBillItemsOption prItemsOption,
                                                QList<double> * fieldsAmounts,
                                                const QList<int> &fieldsToPrint,
                                                const QList<Attribute *> &attrsToPrint,
                                                bool groupPrAm,
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
    void writeODTAttributeBillLineUnion( BillPrinter::PrintBillItemsOption prItemsOption,
                                         QList<double> * fieldsAmounts,
                                         const QList<int> &fieldsToPrint,
                                         const QList<Attribute *> &attrsToPrint,
                                         bool groupPrAm,
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
    void writeODTBillLine( BillPrinter::PrintBillItemsOption prItemsOption,
                           bool writeProgCode,
                           const QList<int> &fieldsToPrint,
                           bool groupPrAm,
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
    void updateAmounts();
    void setQuantityPrivate(double v);
};

#endif // BILLITEM_H
