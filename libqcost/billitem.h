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
#ifndef BILLITEM_H
#define BILLITEM_H

#include "qcost_export.h"

class PriceList;
class PriceItem;
class MeasuresModel;
class VarsModel;
class PriceFieldModel;
class MathParser;
class AttributesModel;
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
    BillItem(PriceItem * p, BillItem * parentItem, PriceFieldModel * pfm, MathParser * parser = NULL, VarsModel * vModel = NULL );
    ~BillItem();

    BillItem &operator =(const BillItem &cp);

    int firstPriceFieldCol();
    VarsModel * varsModel();

    const BillItem * rootItem() const;
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
    bool isUsingPriceItem( PriceItem * p ) const;
    // restituisce l'elenco degli articoli di prezzo contenuti nell'articolo di computo o nei sottoarticoli
    QList<PriceItem *> usedPriceItems() const;
    // restituisce i sottoarticoli del prezzo associato all'articolo di computo, oltre ad eventuali
    // altri prezzi connessi tramite analisi prezzi
    QList<PriceItem *> connectedPriceItems() const;

    double quantity() const;
    QString quantityStr() const;
    /** Importo comprensivo di SGUI */
    double amount( int field ) const;
    /** Importo comprensivo di SGUI */
    QString amountStr( int field ) const;
    /** Importo al netto di SGUI */
    double amountNet( int field ) const;
    /** Importo al netto di SGUI */
    QString amountNetStr( int field ) const;
    /** Importo delle spese generali */
    double amountOverheads( int field ) const;
    /** Importo delle spese generali */
    QString amountOverheadsStr( int field ) const;
    /** Importo degli utili di impresa */
    double amountProfits( int field ) const;
    /** Importo degli utili di impresa */
    QString amountProfitsStr( int field ) const;

    /** Ricalcola gli importi delle spese generali e dei profitti */
    bool recalculateOverheadsProfits() const;
    /** Imposta se ricalcolare gli importi delle spese generali e dei profitti */
    void setRecalculateOverheadsProfits( bool newVal = true );

    /** Valore delle spese generali (numero puro) */
    double overheads() const;
    /** Valore percentuale delle spese generali, come stringa */
    QString overheadsStr() const;
    /** Imposta il valore delle spese generali (numero puro) */
    void setOverheads(double newVal);
    /** Imposta il valore percentuale delle spese generali, come strings */
    void setOverheads(const QString & newVal );

    /**  Valore degli utili di impresa (numero puro) */
    double profits() const;
    /** Valore percentuale degli utili di impresa, come stringa */
    QString profitsStr() const;
    /** Imposta utili di impresa (numero puro) */
    void setProfits(double newVal);
    /** Imposta il valore percentuale degli utili di impresa */
    void setProfits(const QString & newVal );

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

    void writeXml10(QXmlStreamWriter *writer) const;
    void readXml10(QXmlStreamReader *reader, PriceList *priceList, AttributesModel *billAttrModel);
    void readXmlTmp10(QXmlStreamReader *reader);

    void writeXml20( QXmlStreamWriter * writer ) const;
    void readXmlTmp20(QXmlStreamReader *reader);
    void readFromXmlTmp20(PriceList *priceList , AttributesModel *billAttrModel);

    bool containsAttribute( Attribute * attr );
    bool containsAttributeInherited( Attribute * attr );
    bool containsAttributeDirect( Attribute * attr );

    void addAttribute( Attribute * attr );
    void removeAttribute( Attribute * attr );
    void removeAllAttributes();
    double amountAttribute( Attribute * attr, int field );
    QString amountAttributeStr( Attribute * attr, int field );

    void writeODTBillOnTable(QTextCursor * cursor,
                              BillPrinter::PrintBillItemsOption prItemsOption,
                              const QList<int> fieldsToPrint ,
                              bool groupPrAm = false,
                              const QString &umTag = QString() );
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


    void loadTmpData10(PriceList *priceList, AttributesModel *billAttrModel);
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
    void amountNetChanged( int, const QString & );
    void amountNetChanged( int, double );
    void amountOverheadsChanged( int, const QString & );
    void amountOverheadsChanged( int, double );
    void amountProfitsChanged( int, const QString & );
    void amountProfitsChanged( int, double );
    void overheadsChanged( const QString & newVal );
    void profitsChanged( const QString & newVal );
    void attributesChanged();

private:
    BillItemPrivate * m_d;

    void setId( unsigned int ii );

    void setHasChildrenChanged(BillItem * p , QList<int> indexes);
    TreeItem * parentInternal();

    void addChild(BillItem *newChild, int position);
    void removeChild(int position);
    void insertField(int pf);
    void removeField(int pf);
    void updateAmount(int pf);

    void appendUsedPriceItems( QList<PriceItem *> * usedPriceItems ) const;

    void loadXml10(const QXmlStreamAttributes &attrs, PriceList *priceList, AttributesModel *billAttrModel);
    void loadXml20(const QXmlStreamAttributes &attrs, PriceList *priceList, AttributesModel * billAttrModel);

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

    ///
    /// \brief writeODTBillTotalLine
    /// \param fieldsToPrint
    /// \param groupPrAm
    /// \param cursor
    /// \param table
    /// \param tagBlockFormat
    /// \param txtBlockFormat
    /// \param numBlockFormat
    /// \param leftTitleFormat
    /// \param centralTitleFormat
    /// \param rightTitleFormat
    /// \param totalName Nome del totale che si stampa
    /// \param umTag
    /// \param total importo totale che si stampa
    ///
    void writeODTBillTotalLine(const QList<int> &fieldsToPrint,
                                bool groupPrAm,
                                QTextCursor *cursor,
                                QTextTable *table,
                                QTextBlockFormat &tagBlockFormat,
                                QTextBlockFormat &txtBlockFormat,
                                QTextBlockFormat &numBlockFormat,
                                QTextTableCellFormat &leftTitleFormat,
                                QTextTableCellFormat &centralTitleFormat,
                                QTextTableCellFormat &rightTitleFormat,
                                const QString &totalName,
                                const QString &umTag,
                                const QList<QString> &totals);

    ///
    /// \brief writeODTBillLine
    /// \param prItemsOption
    /// \param writeProgCode
    /// \param fieldsToPrint
    /// \param groupPrAm raggruppa prezzi e importi
    /// \param printValNet stampa prezzi e importi al nett di SGUI
    /// \param cursor
    /// \param table
    /// \param tagBlockFormat
    /// \param txtBlockFormat
    /// \param numBlockFormat
    /// \param leftFormat
    /// \param centralFormat
    /// \param rightFormat
    /// \param centralQuantityTotalFormat
    /// \param rightQuantityTotalFormat
    /// \param txtCharFormat
    /// \param txtBoldCharFormat
    ///
    void writeODTBillLine( BillPrinter::PrintBillItemsOption prItemsOption,
                           bool writeProgCode,
                           const QList<int> &fieldsToPrint,
                           bool groupPrAm, bool printValNet,
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

    ///
    /// \brief attributes
    /// \return restituisce tutti gli attributi dell'elemento; se la var bool è false,
    ///         vuol dire che l'attribute è ereditato, se è true vuol dire che è un
    ///         attributo direttamente imposto
    ///
    QList< QPair<Attribute *, bool> > attributes();
    QList<Attribute *> allAttributes();
    QList<Attribute *> directAttributes();
    QList<Attribute *> inheritedAttributes();
    void loadFromXmlTmp10(const QXmlStreamAttributes &attrs);
private slots:
    void emitPriceDataUpdated();
    void setUnitMeasure( UnitMeasure * ump );
    void updateAmounts();
    void setQuantityPrivate(double v);
};

#endif // BILLITEM_H
