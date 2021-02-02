#ifndef ACCOUNTINGBILLITEMPRIVATE_H
#define ACCOUNTINGBILLITEMPRIVATE_H

class AccountingPriceFieldModel;

#include "varsmodel.h"
#include "accountinglsbill.h"
#include "accountingbillitem.h"
#include "pricefieldmodel.h"
#include "priceitem.h"
#include "mathparser.h"

#include <QXmlStreamAttributes>
#include <QDate>
#include <QTextCursor>
#include <QTextTable>

class AccountingBillItemPrivate{
public:
    AccountingBillItemPrivate( AccountingBillItem * parent, AccountingBillItem::ItemType iType,
                               PriceFieldModel * pfm, MathParser * p = nullptr, VarsModel * vModel = nullptr );
    ~AccountingBillItemPrivate();
    QString	toString(double i, char f = 'g', int prec = 6) const;
    QString percentageToString( double v ) const;

    /* *** Funzioni di utilita per semplificare il codice *** */
    static void insertEmptyRow( int cellCount, QTextCursor *cursor, const QTextTableCellFormat &leftFormat, const QTextTableCellFormat &centralFormat, const QTextTableCellFormat &rightFormat);
    static void writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, const QString & txt = QString(), bool nextCell = true );
    static void writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, QList< QPair<QString, QTextCharFormat> > txt, bool nextCell = true );
    void writeDescriptionCell( PriceItem * priceItemToPrint, QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
                               const QTextBlockFormat & txtBlockFormat, const QTextCharFormat & txtCharFormat, const QTextCharFormat & txtBoldCharFormat,
                               AccountingPrinter::PrintPPUDescOption prPPUDescOption );
    void writeDescriptionCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
                               const QTextBlockFormat & txtBlockFormat, const QTextCharFormat & txtCharFormat, const QTextCharFormat & txtBoldCharFormat,
                               AccountingPrinter::PrintPPUDescOption prItemsOption );

    AccountingBillItem * parentItem;
    QList<AccountingBillItem *> childrenContainer;
    QList<Attribute *> attributes;
    MathParser * parser;
    /** Numero progressivo all'interno del registro di contabilità.
     * E' un numero progressivo unico indipendente dal numero del
     * registro di contabilità. */
    int accountingProgCode;
    int progCode;
    PriceFieldModel * priceFieldModel;
    // modello delle variabili associato
    VarsModel * varsModel;

    // Il tipo di elemento
    AccountingBillItem::ItemType itemType;
    // se l'oggetto ha figli, l'oggetto diventa un titolo: il titolo è contenuto nell'attributo name
    QString name;
    // data
    QDate date;
    // Nel caso di lista, data di fine
    QDate dateEnd;
    // nel caso di commento, il testo del commento
    QString text;
    // nel caso di linea con misure, la quantita'
    double quantity;
    // nel caso di linea con misure, il prezzo unitario
    PriceItem * priceItem;
    // prezzo totale lordo
    double PPUTotalToDiscount;
    // prezzo non soggetto a ribasso
    double PPUNotToDiscount;
    // nel caso di linea con misure, il modello delle misure
    MeasuresModel * measuresModel;
    // nel caso di elemento Root - colonna dell'elenco prezzi attiva
    int currentPriceDataSet;
    // nel caso di elemento Root - ribasso
    double discount;
    // nel caso di elemento Root - lista dei pricedataset usati per importo totale
    QList<int> * totalAmountPriceFieldsList;
    // nel caso di elemento Root - lista dei pricedataset usati per importo non soggetto a ribasso
    QList<int> * noDiscountAmountPriceFieldsList;
    // nel caso di elemento Root - modello degli importi totali
    AccountingPriceFieldModel * totalAmountPriceFieldModel;
    // nel caso di elemento Root - modello degli importi non soggetto a ribasso
    AccountingPriceFieldModel * noDiscountAmountPriceFieldModel;
    // nel caso di elemento LumpSum - puntatore alla categoria
    AccountingLSBill * lsBill;
    // nel caso di elemento TimeAndMaterial - puntatore alla lista
    AccountingTAMBillItem * tamBillItem;

    // indici delle colonne
    static int progNumberCol;
    static int dateCol;
    static int priceCodeCol;
    static int priceShortDescCol;
    static int priceUmCol;
    static int quantityCol;
    static int PPUTotalToDiscountCol;
    static int totalAmountToDiscountCol;
    static int PPUNotToDiscountCol;
    static int amountNotToDiscountCol;
    static int totalAmountCol;
    static int discountPrecision;
    int colCount;

    double totalAmountToDiscount;
    double amountNotToDiscount;
    double amountToDiscount;
    double amountDiscounted;
    double totalAmount;
    static int amountPrecision;

    // id dell'elemento
    unsigned int id;

    QXmlStreamAttributes tmpAttributes;
    QString attributesString();
};

#endif // ACCOUNTINGBILLITEMPRIVATE_H
