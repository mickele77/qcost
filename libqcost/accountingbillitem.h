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
#ifndef ACCOUNTINGBILLITEM_H
#define ACCOUNTINGBILLITEM_H

#include "library_common.h"

class AccountingTAMBillItem;
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

class AccountingBillItemPrivate;

class EXPORT_LIB_OPT AccountingBillItem :  public QObject, public TreeItem {
    Q_OBJECT
public:
    enum ItemType{
        Root,
        Bill,
        Comment,
        PPU,
        LumpSum,
        TimeAndMaterials
    };

    friend class AccountingBill;
    friend class AccountingTAMBillItem;
    AccountingBillItem( AccountingBillItem * parentItem, AccountingBillItem::ItemType iType, PriceFieldModel * pfm, MathParser * parser = NULL );
    ~AccountingBillItem();

    AccountingBillItem &operator =(const AccountingBillItem &cp);

    int firstPriceFieldCol();

    AccountingBillItem * parent();
    /** ricerca tra gli oggetti figlio uno con id pari a itemId */
    AccountingBillItem * itemId(unsigned int itemId);
    AccountingBillItem * childItem(int number);
    bool isDescending( AccountingBillItem * ancestor );
    void setParent(AccountingBillItem *newParent, int position);

    unsigned int id();
    QString progressiveCode() const;
    QString name();
    int currentPriceDataSet() const;
    double discount() const;
    // dice se il prezzo è usato dall'articolo di computo o da un suo sottoarticolo
    bool isUsingPriceItem( PriceItem * p );
    // restituisce l'elenco degli articoli di prezzo contenuti nell'articolo di computo o nei sottoarticoli
    QList<PriceItem *> usedPriceItems() const;
    // restituisce i sottoarticoli del prezzo associato all'articolo di computo, oltre ad eventuali
    // altri prezzi connessi tramite analisi prezzi
    QList<PriceItem *> connectedPriceItems() const;

    void setLSBill(AccountingLSBill * newLSBill);
    AccountingLSBill * lsBill();

    void setTAMBillItem(AccountingTAMBillItem *newTAMBill);
    AccountingTAMBillItem * tamBillItem();

    QDate date() const;
    QString dateStr() const;

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

    QList<int> totalAmountPriceFields();
    QList<int> noDiscountAmountPriceFields();

    int columnCount() const;
    QList<AccountingBillItem *> allChildren();
    QList<AccountingBillItem *> allChildrenWithMeasures();
    QVariant data(int col, int role = Qt::EditRole ) const;
    bool setData(int column, const QVariant &quantity);
    Qt::ItemFlags flags(int column) const;

    TreeItem *child(int number);
    int childrenCount() const;
    bool hasChildren() const;
    /** Inserisce elementi sotto (se possibile)
        Se l'elemento è Root, aggiunge un lista Bill
        Se l'elemento è Bill, aggiunge una riga PPU */
    bool insertChildren(int position, int count=1);
    virtual bool insertChildren(ItemType iType, int position, int count=1);
    bool appendChildren(ItemType iType, int count=1 );
    bool removeChildren(int position, int count=1);
    bool reset();
    int childNumber() const;

    void writeXml( QXmlStreamWriter * writer );
    void readXml(QXmlStreamReader *reader, PriceList *priceList, AttributeModel *billAttrModel);
    void readXmlTmp(QXmlStreamReader *reader);
    void loadFromXml(const QXmlStreamAttributes &attrs, PriceList *priceList, AttributeModel * billAttrModel);
    void loadFromXmlTmp(const QXmlStreamAttributes &attrs);
    void loadTmpData(PriceList *priceList , AttributeModel *billAttrModel);

    bool containsAttribute( Attribute * attr ) const ;
    bool containsAttributeInherited( Attribute * attr ) const ;
    bool containsAttributeDirect( Attribute * attr ) const;

    void addAttribute( Attribute * attr );
    void removeAttribute( Attribute * attr );
    void removeAllAttributes();

    double totalAmountToBeDiscountedAttribute( Attribute * attr ) const;
    QString totalAmountToBeDiscountedAttributeStr( Attribute * attr ) const;
    double amountNotToBeDiscountedAttribute( Attribute * attr ) const;
    QString amountNotToBeDiscountedAttributeStr( Attribute * attr ) const;
    double totalAmountAttribute( Attribute * attr ) const;
    QString totalAmountAttributeStr( Attribute * attr ) const;

    void writeODTAccountingOnTable(QTextCursor * cursor,
                                   AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                   bool printAmounts = true ) const;
    void writeODTSummaryOnTable( QTextCursor *cursor,
                                 AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                 bool printAmounts = true,
                                 bool writeDetails = true) const;
    void writeODTAttributeAccountingOnTable(QTextCursor *cursor,
                                            AccountingPrinter::AttributePrintOption prOption,
                                            AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                            const QList<Attribute *> &attrsToPrint,
                                            bool printAmounts = false ) const;
    /** Il tipo di elemento */
    ItemType itemType() const;
    /** Nel caso di lista, il titolo della lista */
    virtual QString title() const;
    /** Nel caso di un commento, restituisce il testo del commento */
    QString text() const;
    /** Nel caso di ppu, restituisce il valore della quantita' */
    double quantity() const;
    /** Nel caso di ppu, restituisce il valore della quantita' come stringa */
    QString quantityStr() const;
    /** Nel caso di ppu, imposta il valore della quantita' */
    void setQuantity(double v);
    /** Nel caso di ppu, il prezzo */
    PriceItem *priceItem();
    MeasuresModel * measuresModel();
    MeasuresModel * generateMeasuresModel();
    void removeMeasuresModel();
    /** Nel caso di ppu, prezzo complessivo */
    double PPUTotalToBeDiscounted() const;
    /** Nel caso di ppu, prezzo non ribassabile */
    double PPUNotToBeDiscounted() const;
    /** Nel caso di ppu, prezzo complessivo, stringa */
    QString PPUTotalToBeDiscountedStr() const;
    /** Nel caso di ppu, prezzo non ribassabile, stringa */
    QString PPUNotToBeDiscountedStr() const;
    /** Nel caso di Bill (lista sesttimanale), la data di inizio */
    QDate dateBegin() const;
    /** Nel caso di Bill (lista sesttimanale), la data di inizio */
    QString dateBeginStr() const;
    /** Nel caso di Bill (lista sesttimanale), la data di fine */
    QDate dateEnd() const;
    /** Nel caso di Bill (lista sesttimanale), la data di fine */
    QString dateEndStr() const;
    /** Nel caso di Root, il modelli di campi prezzo totali */
    AccountingPriceFieldModel * totalAmountPriceFieldModel();
    /** Nel caso di Root, il modelli di campi prezzo non soggetti a ribasso */
    AccountingPriceFieldModel * noDiscountAmountPriceFieldModel();

public slots:
    void setCurrentPriceDataSet(int newVal);
    void setDiscount( double newVal );
    void setName(const QString &newName);
    void setItemType(ItemType iType);
    void setDate( const QDate & d );
    void setDate( const QString & d );
    /** Nel caso di un commento, imposta il valore del commento */
    void setText(const QString &t);
    /** Nel caso di PPU, imposta il valore della quantita' */
    void setQuantity(const QString &vstr);
    void setPriceItem( PriceItem * p );
    void setDateBegin( const QDate & d );
    void setDateBegin( const QString & d );
    void setDateEnd( const QDate & d );
    void setDateEnd( const QString & d );
    void setTotalAmountPriceFields(const QList<int> &newAmountFields);
    void setNoDiscountAmountPriceFields(const QList<int> &newAmountFields);
    /** Aggiorna tutti gli importi della contabilita */
    void updatePPUs();

signals:
    /** Segnale emesso quando l'item cambia */
    void itemChanged();
    /** Segnale emesso quando l'item cambia */
    void itemTypeChanged( ItemType );
    /** Segnale emesso nel caso cambi il commento */
    void titleChanged(const QString &t);
    /** Segnale emesso nel caso cambi il commento */
    void textChanged(const QString &t);
    /** Segnale emesso nel caso cambi la quantita' */
    void quantityChanged( const QString &  );
    /** Segnale emesso prima che l'oggetto sia cancellato */
    void aboutToBeDeleted();
    /** segnale emesso se sono stati aggiunti o tolti figli */
    void hasChildrenChanged( bool );
    // segnale emesso quando alcuni figli dell'oggetto sono cambiati
    void hasChildrenChanged( AccountingBillItem *, QList<int> );
    void dataChanged( AccountingBillItem * );
    void dataChanged( AccountingBillItem *, int column);
    void nameChanged( const QString & );
    void priceItemChanged( PriceItem * oldPriceItem, PriceItem * newPriceItem );
    void currentPriceDataSetChanged( int newPriceDataSet );
    void discountChanged( double newDiscount );
    void PPUTotalToBeDiscountedChanged( const QString & newVal );
    void PPUNotToBeDiscountedChanged( const QString & newVal );
    void totalAmountToBeDiscountedChanged( const QString & newVal );
    void amountNotToBeDiscountedChanged( const QString & newVal );
    void amountToBeDiscountedChanged( const QString & newVal );
    void amountDiscountedChanged( const QString & newVal );
    void totalAmountChanged( const QString & newVal );
    void dateChanged( const QString &  );
    void dateBeginChanged( const QString &  );
    void dateEndChanged( const QString & newDateEnd );
    void attributesChanged();

    void lsBillChanged( AccountingLSBill * newLSBill );
    void tamBillItemChanged( AccountingTAMBillItem * newTAMBill );

protected:
    AccountingBillItemPrivate * m_d;

    int progressiveCodeInternal() const;

    /** Ricerca all'interno del computo un AccountingMeasure con id pari a itemId */
    AccountingBillItem *findAccountingItemId(unsigned int itemId);

    void setId( unsigned int ii );

    void setHasChildrenChanged(AccountingBillItem * p , QList<int> indexes);
    TreeItem * parentInternal();

    void addChild(AccountingBillItem *newChild, int position);

    void appendUsedPriceItems( QList<PriceItem *> * usedPriceItems ) const;

    void writeODTSummaryLine( PriceItem * priceItem,
                              QTextCursor *cursor,
                              double * itemTotalQuantity,
                              QList<double> * fieldsValue,
                              bool printAmounts,
                              bool writeDetails,
                              QTextTable *table,
                              QTextBlockFormat &tagBlockFormat,
                              QTextBlockFormat & txtBlockFormat,
                              QTextBlockFormat & numBlockFormat,
                              QTextTableCellFormat & leftFormat,
                              QTextTableCellFormat & centralFormat,
                              QTextTableCellFormat & rightFormat ) const;

    void writeODTAttributeBillLineSimple( AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                          QList<double> * fieldsAmounts,
                                          Attribute * attrsToPrint,
                                          bool printAmounts,
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
                                          QTextCharFormat &txtBoldCharFormat ) const;
    void writeODTAttributeBillLineIntersection( AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                                QList<double> * fieldsAmounts,
                                                const QList<Attribute *> &attrsToPrint,
                                                bool printAmounts,
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
                                                QTextCharFormat &txtBoldCharFormat ) const;
    void writeODTAttributeBillLineUnion( AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                         QList<double> * fieldsAmounts,
                                         const QList<Attribute *> &attrsToPrint,
                                         bool printAmounts,
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
                                         QTextCharFormat &txtBoldCharFormat) const;
    void writeODTBillLine( AccountingPrinter::PrintAccountingBillOption prItemsOption,
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
                           QTextCharFormat & txtBoldCharFormat ) const;

    /** restituisce tutti gli attributi dell'elemento; se la var bool è false,
        vuol dire che l'attribute è ereditato, se è true vuol dire che è un
        attributo direttamente imposto */
    QList< QPair<Attribute *, bool> > attributes();
    QList<Attribute *> allAttributes();
    QList<Attribute *> directAttributes() const;
    QList<Attribute *> inheritedAttributes() const;

protected slots:
    void setQuantityPrivate(double v);
    void emitPriceDataUpdated();
    void emitTitleChanged();

    void updateTotalAmountToBeDiscounted();
    void updateAmountNotToBeDiscounted();
    void updateAmountToBeDiscounted();
    void updateAmountDiscounted();
    void updateTotalAmount();

    // nel caso di lumpsum, azzera l'oggetto AccountingLSBill associato
    void setLSBillNULL();
    // nel caso di tam, azzera l'oggetto AccountingBillItem associato
    void setTAMBillItemNULL();
};

#endif // ACCOUNTINGBILLITEM_H
