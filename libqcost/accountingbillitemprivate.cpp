#include "accountingbillitemprivate.h"

#include "accountinglsbillitem.h"
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

AccountingBillItemPrivate::AccountingBillItemPrivate( AccountingBillItem * parent, AccountingBillItem::ItemType iType, PriceFieldModel * pfm, MathParser * p, VarsModel * vModel ):
    parentItem(parent),
    parser(p),
    accountingProgCode(-1),
    progCode(-1),
    priceFieldModel(pfm),
    varsModel(vModel),
    itemType(iType),
    name(QString()),
    date( QDate::currentDate() ),
    dateEnd( QDate::currentDate() ),
    text( QObject::tr("Commento") ),
    quantity(0.0),
    priceItem(nullptr),
    PPUTotalToDiscount(0.0),
    PPUNotToDiscount(0.0),
    measuresModel(nullptr),
    currentPriceDataSet(0),
    discount(0.0),
    totalAmountPriceFieldsList(nullptr),
    noDiscountAmountPriceFieldsList(nullptr),
    totalAmountPriceFieldModel( nullptr ),
    noDiscountAmountPriceFieldModel( nullptr ),
    lsBill(nullptr),
    tamBillItem(nullptr),
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
    if( totalAmountPriceFieldModel != nullptr ){
        delete totalAmountPriceFieldModel;
    }
    if( noDiscountAmountPriceFieldModel != nullptr ){
        delete noDiscountAmountPriceFieldModel;
    }
    if( totalAmountPriceFieldsList != nullptr ){
        delete totalAmountPriceFieldsList;
    }
    if( noDiscountAmountPriceFieldsList != nullptr ){
        delete noDiscountAmountPriceFieldsList;
    }
    if( measuresModel != nullptr ){
        delete measuresModel;
    }
    for( QList<AccountingBillItem *>::iterator i = childrenContainer.begin(); i != childrenContainer.end(); ++i ){
        delete *i;
    }
}

QString	AccountingBillItemPrivate::toString(double i, char f, int prec ) const{
    if( parser != nullptr ){
        return parser->toString( i, f, prec );
    } else {
        return QString::number( i, f, prec );
    }
}

QString AccountingBillItemPrivate::percentageToString(double v) const {
    return QString("%1 %").arg( toString(v * 100.0, 'f', AccountingLSBillItem::percentagePrecision() ) );
}

void AccountingBillItemPrivate::writeDescriptionCell(PriceItem * priceItemToPrint, QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
                                                      const QTextBlockFormat & txtBlockFormat, const QTextCharFormat & txtCharFormat, const QTextCharFormat & txtBoldCharFormat,
                                                      AccountingPrinter::PrintPPUDescOption prPPUDescOption ){
    if( priceItemToPrint != nullptr ){
        if( prPPUDescOption == AccountingPrinter::PrintShortDesc ){
            writeCell( cursor, table, centralFormat, txtBlockFormat, priceItemToPrint->shortDescriptionFull() );
        } else if( prPPUDescOption == AccountingPrinter::PrintLongDesc ){
            writeCell( cursor, table, centralFormat, txtBlockFormat, priceItemToPrint->longDescriptionFull() );
        } else if( prPPUDescOption == AccountingPrinter::PrintShortLongDesc ){
            QList< QPair<QString, QTextCharFormat> > txt;
            txt << qMakePair( priceItemToPrint->shortDescriptionFull(), txtBoldCharFormat );
            txt << qMakePair( "\n" + priceItemToPrint->longDescriptionFull(), txtCharFormat );
            writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
        } else if( prPPUDescOption == AccountingPrinter::PrintShortLongDescOpt ){
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
