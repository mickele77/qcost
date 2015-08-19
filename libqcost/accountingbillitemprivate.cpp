#include "accountingbillitemprivate.h"

#include "accountingpricefieldmodel.h"
#include "measuresmodel.h"
#include "attribute.h"

int AccountingBillItemPrivate::progNumberCol = 0;
int AccountingBillItemPrivate::priceCodeCol = 1;
int AccountingBillItemPrivate::priceShortDescCol = 2;
int AccountingBillItemPrivate::priceUmCol = 3;
int AccountingBillItemPrivate::dateCol = 4;
int AccountingBillItemPrivate::quantityCol = 5;
int AccountingBillItemPrivate::PPUTotalToDiscountCol = 6;
int AccountingBillItemPrivate::totalAmountToDiscountCol = 7;
int AccountingBillItemPrivate::PPUNotToDiscountCol = 8;
int AccountingBillItemPrivate::amountNotToDiscountCol = 9;
int AccountingBillItemPrivate::totalAmountCol = 10;
int AccountingBillItemPrivate::amountPrecision = 2;
int AccountingBillItemPrivate::discountPrecision = 8;

AccountingBillItemPrivate::AccountingBillItemPrivate( AccountingBillItem * parent, AccountingBillItem::ItemType iType, PriceFieldModel * pfm, MathParser * p ):
    parentItem(parent),
    parser(p),
    priceFieldModel(pfm),
    progressiveCode(-1),
    itemType(iType),
    name(QString()),
    date( QDate::currentDate() ),
    dateEnd( QDate::currentDate() ),
    text( QObject::trUtf8("Commento") ),
    quantity(0.0),
    priceItem(NULL),
    PPUTotalToDiscount(0.0),
    PPUNotToDiscount(0.0),
    measuresModel(NULL),
    currentPriceDataSet(0),
    discount(0.0),
    totalAmountPriceFieldsList(NULL),
    noDiscountAmountPriceFieldsList(NULL),
    totalAmountPriceFieldModel( NULL ),
    noDiscountAmountPriceFieldModel( NULL ),
    lsBill(NULL),
    tamBillItem(NULL),
    colCount(totalAmountCol+1),
    totalAmountToDiscount(0.0),
    amountNotToDiscount(0.0),
    amountToDiscount(0.0),
    amountDiscounted(0.0),
    totalAmount(0.0){
    if( (itemType == AccountingBillItem::Root) ||
            (itemType == AccountingBillItem::Payment) ){
        id = 0;
    } else {
        id = 1;
    }
    if( itemType == AccountingBillItem::Root ) {
        totalAmountPriceFieldsList = new QList<int>();
        totalAmountPriceFieldModel = new AccountingPriceFieldModel(totalAmountPriceFieldsList, pfm);
        noDiscountAmountPriceFieldsList = new QList<int>();
        noDiscountAmountPriceFieldModel = new AccountingPriceFieldModel(noDiscountAmountPriceFieldsList, pfm);
    }
}

AccountingBillItemPrivate::~AccountingBillItemPrivate(){
    if( totalAmountPriceFieldModel != NULL ){
        delete totalAmountPriceFieldModel;
    }
    if( noDiscountAmountPriceFieldModel != NULL ){
        delete noDiscountAmountPriceFieldModel;
    }
    if( totalAmountPriceFieldsList != NULL ){
        delete totalAmountPriceFieldsList;
    }
    if( noDiscountAmountPriceFieldsList != NULL ){
        delete noDiscountAmountPriceFieldsList;
    }
    if( measuresModel != NULL ){
        delete measuresModel;
    }
    for( QList<AccountingBillItem *>::iterator i = childrenContainer.begin(); i != childrenContainer.end(); ++i ){
        delete *i;
    }
}

QString	AccountingBillItemPrivate::toString(double i, char f, int prec ) const{
    if( parser != NULL ){
        return parser->toString( i, f, prec );
    } else {
        return QString::number( i, f, prec );
    }
}

void AccountingBillItemPrivate::writeDescriptionCell( PriceItem * priceItemToPrint, QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
                                                      const QTextBlockFormat & txtBlockFormat, const QTextCharFormat & txtCharFormat, const QTextCharFormat & txtBoldCharFormat,
                                                      AccountingPrinter::PrintPPUDescOption prItemsOption ){
    if( priceItemToPrint != NULL ){
        if( prItemsOption == AccountingPrinter::PrintShortDesc ){
            writeCell( cursor, table, centralFormat, txtBlockFormat, priceItemToPrint->shortDescriptionFull() );
        } else if( prItemsOption == AccountingPrinter::PrintLongDesc ){
            writeCell( cursor, table, centralFormat, txtBlockFormat, priceItemToPrint->longDescriptionFull() );
        } else if( prItemsOption == AccountingPrinter::PrintShortLongDesc ){
            QList< QPair<QString, QTextCharFormat> > txt;
            txt << qMakePair( priceItemToPrint->shortDescriptionFull(), txtBoldCharFormat );
            txt << qMakePair( "\n" + priceItemToPrint->longDescriptionFull(), txtCharFormat );
            writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
        } else if( prItemsOption == AccountingPrinter::PrintShortLongDescOpt ){
            QString sDesc = priceItemToPrint->shortDescriptionFull();
            while( sDesc.endsWith(" ")){
                sDesc.remove( sDesc.length()-1, 1);
            }
            if( sDesc.endsWith( "...") ){
                sDesc.remove( sDesc.length()-3, 3);
            }
            QString lDesc = priceItemToPrint->longDescriptionFull();
            if( lDesc.startsWith(sDesc) ){
                lDesc.remove( 0, sDesc.length() );
            }
            QList< QPair<QString, QTextCharFormat> > txt;
            txt << qMakePair( sDesc, txtBoldCharFormat );
            txt << qMakePair( "\n" + lDesc, txtCharFormat );
            writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
        }
    } else {
        writeCell( cursor, table, centralFormat, txtBlockFormat );
    }
}

void AccountingBillItemPrivate::writeDescriptionCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
                                                      const QTextBlockFormat & txtBlockFormat, const QTextCharFormat & txtCharFormat, const QTextCharFormat & txtBoldCharFormat,
                                                      AccountingPrinter::PrintPPUDescOption prItemsOption ){
    writeDescriptionCell( priceItem, cursor, table, centralFormat,
                          txtBlockFormat, txtCharFormat, txtBoldCharFormat,
                          prItemsOption );
}

/* *** Funzioni di utilita per semplificare il codice *** */
void AccountingBillItemPrivate::insertEmptyRow( int cellCount, QTextCursor *cursor, const QTextTableCellFormat &leftFormat, const QTextTableCellFormat &centralFormat, const QTextTableCellFormat &rightFormat) {
    QTextTable *table = cursor->currentTable();
    table->appendRows(1);
    cursor->movePosition(QTextCursor::PreviousRow );
    cursor->movePosition(QTextCursor::NextCell);
    table->cellAt( *cursor ).setFormat( leftFormat );

    for( int i=0; i<cellCount-2; ++i ){
        cursor->movePosition(QTextCursor::NextCell);
        table->cellAt( *cursor ).setFormat( centralFormat );
    }
    cursor->movePosition(QTextCursor::NextCell);
    table->cellAt( *cursor ).setFormat( rightFormat );
}

void AccountingBillItemPrivate::writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, const QString & txt, bool nextCell ) {
    if( nextCell ){
        cursor->movePosition(QTextCursor::NextCell);
    }
    table->cellAt( *cursor ).setFormat( cellFormat );
    cursor->setBlockFormat( blockFormat );
    if( !txt.isEmpty() ){
        cursor->insertText( txt );
    }
}

void AccountingBillItemPrivate::writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, QList< QPair<QString, QTextCharFormat> > txt, bool nextCell ) {
    if( nextCell ){
        cursor->movePosition(QTextCursor::NextCell);
    }
    table->cellAt( *cursor ).setFormat( cellFormat );
    cursor->setBlockFormat( blockFormat );
    for( QList< QPair<QString, QTextCharFormat> >::iterator i = txt.begin(); i != txt.end(); ++i ){
        cursor->insertText( (*i).first, (*i).second );
    }
}

QString AccountingBillItemPrivate::attributesString() {
    QString attrs;
    for( QList<Attribute *>::iterator i = attributes.begin(); i != attributes.end(); ++i ){
        if( attrs.isEmpty() ){
            attrs = QString::number( (*i)->id() );
        } else {
            attrs = QString( "%1, %2" ).arg( attrs, QString::number( (*i)->id() ) );
        }
    }
    return attrs;
}
