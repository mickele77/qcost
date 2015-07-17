#include "accountingbillitemprivate.h"

#include "accountingpricefieldmodel.h"
#include "measuresmodel.h"

int AccountingBillItemPrivate::progNumberCol = 0;
int AccountingBillItemPrivate::dateCol = 4;
int AccountingBillItemPrivate::priceCodeCol = 1;
int AccountingBillItemPrivate::priceShortDescCol = 2;
int AccountingBillItemPrivate::priceUmCol = 3;
int AccountingBillItemPrivate::quantityCol = 5;
int AccountingBillItemPrivate::amountPrecision = 2;
int AccountingBillItemPrivate::totalAmountToBeDiscountedCol = 6;
int AccountingBillItemPrivate::amountNotToBeDiscountedCol = AccountingBillItemPrivate::totalAmountToBeDiscountedCol + 2;
int AccountingBillItemPrivate::totalAmountCol = AccountingBillItemPrivate::amountNotToBeDiscountedCol + 2;

AccountingBillItemPrivate::AccountingBillItemPrivate( AccountingBillItem * parent, AccountingBillItem::ItemType iType, PriceFieldModel * pfm, MathParser * p ):
    parentItem(parent),
    parser(p),
    priceFieldModel(pfm),
    itemType(iType),
    name(QString()),
    date( QDate::currentDate() ),
    dateEnd( QDate::currentDate() ),
    text( QObject::trUtf8("Commento") ),
    quantity(0.0),
    priceItem(NULL),
    PPUTotalToBeDiscounted(0.0),
    PPUNotToBeDiscounted(0.0),
    measuresModel(NULL),
    currentPriceDataSet(0),
    discount(0.0),
    totalAmountPriceFieldsList(QList<int>()),
    noDiscountAmountPriceFieldsList(QList<int>()),
    totalAmountPriceFieldModel( NULL ),
    noDiscountAmountPriceFieldModel( NULL ),
    colCount(totalAmountCol+1),
    totalAmountToBeDiscounted(0.0),
    amountNotToBeDiscounted(0.0),
    amountToBeDiscounted(0.0),
    amountDiscounted(0.0),
    totalAmount(0.0){
    if( parent != NULL ){
        id = 1;
    } else {
        id = 0;
    }
    if( itemType == AccountingBillItem::Root ) {
        totalAmountPriceFieldModel = new AccountingPriceFieldModel(&totalAmountPriceFieldsList, pfm);
        noDiscountAmountPriceFieldModel = new AccountingPriceFieldModel(&noDiscountAmountPriceFieldsList, pfm);
    }

}

AccountingBillItemPrivate::~AccountingBillItemPrivate(){
    if( totalAmountPriceFieldModel != NULL ){
        delete totalAmountPriceFieldModel;
    }
    if( noDiscountAmountPriceFieldModel != NULL ){
        delete noDiscountAmountPriceFieldModel;
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
                                                      AccountingPrinter::PrintAccountingBillOption prItemsOption ){
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
                                                      AccountingPrinter::PrintAccountingBillOption prItemsOption ){
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

