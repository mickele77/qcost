#include "accountingtambillitemprivate.h"

#include "accountingpricefieldmodel.h"
#include "accountinglsbillitem.h"
#include "accountingtammeasuresmodel.h"
#include "attribute.h"

int AccountingTAMBillItemPrivate::progNumberCol = 0;
int AccountingTAMBillItemPrivate::priceCodeCol = 1;
int AccountingTAMBillItemPrivate::priceShortDescCol = 2;
int AccountingTAMBillItemPrivate::priceUmCol = 3;
int AccountingTAMBillItemPrivate::startDateCol = 4;
int AccountingTAMBillItemPrivate::endDateCol = 5;
int AccountingTAMBillItemPrivate::quantityCol = 6;
int AccountingTAMBillItemPrivate::PPUTotalToDiscountCol = 7;
int AccountingTAMBillItemPrivate::totalAmountToDiscountCol = 8;
int AccountingTAMBillItemPrivate::PPUNotToDiscountCol = 9;
int AccountingTAMBillItemPrivate::amountNotToDiscountCol = 10;
int AccountingTAMBillItemPrivate::totalAmountCol = 11;
int AccountingTAMBillItemPrivate::amountPrecision = 2;
int AccountingTAMBillItemPrivate::discountPrecision = 8;

AccountingTAMBillItemPrivate::AccountingTAMBillItemPrivate( AccountingTAMBillItem * parent, AccountingTAMBillItem::ItemType iType, PriceFieldModel * pfm, MathParser * p, VarsModel * vModel ):
    parentItem(parent),
    parser(p),
    accountingProgCode(-1),
    progCode(-1),
    priceFieldModel(pfm),
    varsModel(vModel),
    itemType(iType),
    name(QString()),
    startDate( NULL ),
    endDate( NULL ),
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
    colCount(totalAmountCol+1),
    totalAmountToDiscount(0.0),
    amountNotToDiscount(0.0),
    amountToDiscount(0.0),
    amountDiscounted(0.0),
    totalAmount(0.0){
    if( (itemType == AccountingTAMBillItem::Root) ||
            (itemType == AccountingTAMBillItem::Payment) ){
        id = 0;
    } else {
        id = 1;
    }
    if( itemType == AccountingTAMBillItem::Root ) {
        totalAmountPriceFieldsList = new QList<int>();
        totalAmountPriceFieldModel = new AccountingPriceFieldModel(totalAmountPriceFieldsList, pfm);
        noDiscountAmountPriceFieldsList = new QList<int>();
        noDiscountAmountPriceFieldModel = new AccountingPriceFieldModel(noDiscountAmountPriceFieldsList, pfm);
    }
    if( itemType == AccountingTAMBillItem::Payment ) {
        startDate = new QDate();
        *startDate = QDate::currentDate();
        endDate = new QDate();
        *endDate = QDate::currentDate();
        *endDate = endDate->addDays( 6 );
    }
}

AccountingTAMBillItemPrivate::~AccountingTAMBillItemPrivate(){
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
    for( QList<AccountingTAMBillItem *>::iterator i = childrenContainer.begin(); i != childrenContainer.end(); ++i ){
        delete *i;
    }
    if( startDate != NULL ) {
        delete startDate;
    }
    if( endDate != NULL ){
        delete endDate;
    }
}

QString	AccountingTAMBillItemPrivate::toString(double i, char f, int prec ) const{
    if( parser != NULL ){
        return parser->toString( i, f, prec );
    } else {
        return QString::number( i, f, prec );
    }
}

QString AccountingTAMBillItemPrivate::percentageToString(double v) const {
    return QString("%1 %").arg( toString(v * 100.0, 'f', AccountingLSBillItem::percentagePrecision() ) );
}

void AccountingTAMBillItemPrivate::writeDescriptionCell(PriceItem * priceItemToPrint, QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
                                                      const QTextBlockFormat & txtBlockFormat, const QTextCharFormat & txtCharFormat, const QTextCharFormat & txtBoldCharFormat,
                                                      AccountingPrinter::PrintPPUDescOption prPPUDescOption ){
    if( priceItemToPrint != NULL ){
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

void AccountingTAMBillItemPrivate::writeDescriptionCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
                                                      const QTextBlockFormat & txtBlockFormat, const QTextCharFormat & txtCharFormat, const QTextCharFormat & txtBoldCharFormat,
                                                      AccountingPrinter::PrintPPUDescOption prItemsOption ){
    writeDescriptionCell( priceItem, cursor, table, centralFormat,
                          txtBlockFormat, txtCharFormat, txtBoldCharFormat,
                          prItemsOption );
}

/* *** Funzioni di utilita per semplificare il codice *** */
void AccountingTAMBillItemPrivate::insertEmptyRow( int cellCount, QTextCursor *cursor, const QTextTableCellFormat &leftFormat, const QTextTableCellFormat &centralFormat, const QTextTableCellFormat &rightFormat) {
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

void AccountingTAMBillItemPrivate::writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, const QString & txt, bool nextCell ) {
    if( nextCell ){
        cursor->movePosition(QTextCursor::NextCell);
    }
    table->cellAt( *cursor ).setFormat( cellFormat );
    cursor->setBlockFormat( blockFormat );
    if( !txt.isEmpty() ){
        cursor->insertText( txt );
    }
}

void AccountingTAMBillItemPrivate::writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, QList< QPair<QString, QTextCharFormat> > txt, bool nextCell ) {
    if( nextCell ){
        cursor->movePosition(QTextCursor::NextCell);
    }
    table->cellAt( *cursor ).setFormat( cellFormat );
    cursor->setBlockFormat( blockFormat );
    for( QList< QPair<QString, QTextCharFormat> >::iterator i = txt.begin(); i != txt.end(); ++i ){
        cursor->insertText( (*i).first, (*i).second );
    }
}

QString AccountingTAMBillItemPrivate::attributesString() {
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
