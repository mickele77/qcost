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

#include "accountinglsbillitem.h"

#include "accountingpricefieldmodel.h"
#include "pricelist.h"
#include "priceitem.h"
#include "measureslsmodel.h"
#include "accountinglsitemmeasure.h"
#include "attributesmodel.h"
#include "attribute.h"
#include "unitmeasure.h"
#include "mathparser.h"

#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QTextTable>
#include <QTextStream>
#include <QTextCursor>
#include <QVariant>
#include <QStringList>
#include <QList>
#include <QDate>

#include <cmath>

class AccountingLSBillItemPrivate{
public:
    AccountingLSBillItemPrivate( AccountingLSBillItem * parent, PriceFieldModel * pfm, MathParser * prs = NULL ):
        parentItem(parent),
        measuresModel( new MeasuresLSModel( prs, NULL ) ),
        parser( prs ),
        name(QObject::trUtf8("Titolo")),
        priceItem( NULL ),
        currentPriceDataSet(0),
        projQuantity(0.0),
        accQuantity(0.0),
        PPU(0.0),
        projAmount(0.0),
        accAmount(0.0),
        percentageAccounted(0.0),
        totalAmountPriceFieldsList(NULL),
        totalAmountPriceFieldModel( NULL ),
        priceFieldModel( pfm ) {
        if( parent != NULL ){
            id = 1;
        } else {
            id = 0;
            totalAmountPriceFieldsList = new QList<int>();
            totalAmountPriceFieldModel = new AccountingPriceFieldModel(totalAmountPriceFieldsList, pfm);
        }
    }
    ~AccountingLSBillItemPrivate(){
        for( QList<AccountingLSBillItem *>::iterator i = childrenContainer.begin(); i != childrenContainer.end(); ++i ){
            delete *i;
        }
        if( measuresModel != NULL ){
            delete measuresModel;
        }
        if( totalAmountPriceFieldsList != NULL ){
            delete totalAmountPriceFieldsList;
        }
    }

    QString	toString(double i, char f = 'g', int prec = 6) const{
        if( parser != NULL ){
            return parser->toString( i, f, prec );
        } else {
            return QString::number( i, f, prec );
        }
    }

    // Funzione di utilita per semplificare il codice
    static void insertEmptyRow( int cellCount, QTextCursor *cursor, const QTextTableCellFormat &leftFormat, const QTextTableCellFormat &centralFormat, const QTextTableCellFormat &rightFormat) {
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

    // Funzione di utilita per semplificare il codice
    static void writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, const QString & txt = QString(), bool nextCell = true ) {
        if( nextCell ){
            cursor->movePosition(QTextCursor::NextCell);
        }
        table->cellAt( *cursor ).setFormat( cellFormat );
        cursor->setBlockFormat( blockFormat );
        if( !txt.isEmpty() ){
            cursor->insertText( txt );
        }
    }

    // Funzione di utilita per semplificare il codice
    static void writeCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &cellFormat, const QTextBlockFormat &blockFormat, QList< QPair<QString, QTextCharFormat> > txt, bool nextCell = true ) {
        if( nextCell ){
            cursor->movePosition(QTextCursor::NextCell);
        }
        table->cellAt( *cursor ).setFormat( cellFormat );
        cursor->setBlockFormat( blockFormat );
        for( QList< QPair<QString, QTextCharFormat> >::iterator i = txt.begin(); i != txt.end(); ++i ){
            cursor->insertText( (*i).first, (*i).second );
        }
    }

    void writeDescriptionCell( PriceItem * priceItemToPrint, QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
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

    void writeDescriptionCell( QTextCursor *cursor, QTextTable * table, const QTextTableCellFormat &centralFormat,
                               const QTextBlockFormat & txtBlockFormat, const QTextCharFormat & txtCharFormat, const QTextCharFormat & txtBoldCharFormat,
                               AccountingPrinter::PrintPPUDescOption prPPUOption ){
        writeDescriptionCell( priceItem, cursor, table, centralFormat,
                              txtBlockFormat, txtCharFormat, txtBoldCharFormat,
                              prPPUOption );
    }

    AccountingLSBillItem * parentItem;
    QList<AccountingLSBillItem *> childrenContainer;
    QList<Attribute *> attributes;
    MeasuresLSModel * measuresModel;
    MathParser * parser;

    // se l'oggetto ha figli, l'oggetto diventa un titolo: il titolo è contenuto nell'attributo name
    QString name;
    PriceItem * priceItem;
    // colonna dell'elenco prezzi attiva
    int currentPriceDataSet;
    // quantita'
    double projQuantity;
    // quantita' contabilizzata
    double accQuantity;
    // prezzo complessivo
    double PPU;
    // Importo complessivo lordo
    double projAmount;
    // Importo complessivo contabilizzato
    double accAmount;

    // Percentuale contabilizzata
    double percentageAccounted;

    // lista dei pricedataset usati per importo totale
    QList<int> * totalAmountPriceFieldsList;
    // modello degli importi totali
    AccountingPriceFieldModel * totalAmountPriceFieldModel;
    PriceFieldModel * priceFieldModel;

    // indici delle colonne
    static int progNumberCol;
    static int priceCodeCol;
    static int priceShortDescCol;
    static int priceUmCol;
    static int priceCol;
    static int projQuantityCol;
    static int projAmountCol;
    static int accQuantityCol;
    static int accAmountCol;
    static int colCount;

    unsigned int id;

    static int amountPrecision;
    static int percentagePrecision;

    QXmlStreamAttributes tmpAttributes;
};

int AccountingLSBillItemPrivate::progNumberCol = 0;
int AccountingLSBillItemPrivate::priceCodeCol = 1;
int AccountingLSBillItemPrivate::priceShortDescCol = 2;
int AccountingLSBillItemPrivate::priceUmCol = 3;
int AccountingLSBillItemPrivate::priceCol = 4;
int AccountingLSBillItemPrivate::projQuantityCol = 5;
int AccountingLSBillItemPrivate::projAmountCol = 6;
int AccountingLSBillItemPrivate::accQuantityCol = 7;
int AccountingLSBillItemPrivate::accAmountCol = 8;
int AccountingLSBillItemPrivate::colCount = AccountingLSBillItemPrivate::accAmountCol + 1;

int AccountingLSBillItemPrivate::amountPrecision = 2;
int AccountingLSBillItemPrivate::percentagePrecision = 6;

AccountingLSBillItem::AccountingLSBillItem( PriceItem * p, AccountingLSBillItem *parentItem, PriceFieldModel * pfm, MathParser * parser ):
    TreeItem(),
    m_d( new AccountingLSBillItemPrivate(parentItem, pfm, parser ) ){
    setPriceItem(p);
    if( parentItem != NULL ){
        connect( parentItem, &AccountingLSBillItem::attributesChanged, this, &AccountingLSBillItem::attributesChanged );
    }

    connect( this, &AccountingLSBillItem::PPUChanged, this, &AccountingLSBillItem::updateProjAmount );
    connect( this, &AccountingLSBillItem::PPUChanged, this, &AccountingLSBillItem::updateAccAmount );

    connect( m_d->measuresModel, &MeasuresLSModel::projQuantityChanged, this, &AccountingLSBillItem::updateProjQuantityPrivate );
    connect( m_d->measuresModel, &MeasuresLSModel::accQuantityChanged, this, &AccountingLSBillItem::updateAccQuantityPrivate );
    connect( m_d->measuresModel, &MeasuresLSModel::modelChanged, this, &AccountingLSBillItem::itemChanged );

    connect( this, &AccountingLSBillItem::projAmountChanged, this, &AccountingLSBillItem::updatePercentageAccounted );
    connect( this, &AccountingLSBillItem::accAmountChanged, this, &AccountingLSBillItem::updatePercentageAccounted );

    connect( this, static_cast<void(AccountingLSBillItem::*)( AccountingLSBillItem *, QList<int> )>(&AccountingLSBillItem::hasChildrenChanged), this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::nameChanged, this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::priceItemChanged, this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::currentPriceDataSetChanged, this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::projQuantityChanged, this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::accQuantityChanged, this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::percentageAccountedChanged, this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::projAmountChanged, this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::accAmountChanged, this, &AccountingLSBillItem::itemChanged );
    connect( this, &AccountingLSBillItem::attributesChanged, this, &AccountingLSBillItem::itemChanged );

    if( m_d->parentItem == NULL ){
        connect( m_d->totalAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingLSBillItem::updatePPU );
    }
}

AccountingLSBillItem::~AccountingLSBillItem(){
    emit aboutToBeDeleted();
    delete m_d;
}

AccountingLSBillItem &AccountingLSBillItem::operator=(const AccountingLSBillItem &cp) {
    if( &cp != this ){
        if( cp.hasChildren() ){
            removeChildren( 0, childrenCount() );
            insertChildren(0, cp.childrenCount() );
            for( int i=0; i < childrenCount(); ++i ){
                *(m_d->childrenContainer.at(i)) = *(cp.m_d->childrenContainer.at(i));
            }
        }

        setPriceItem( cp.m_d->priceItem );
        setCurrentPriceDataSet( cp.m_d->currentPriceDataSet );
        setName( cp.m_d->name );
        *(m_d->measuresModel) = *(cp.m_d->measuresModel);

    }

    return *this;
}

QString AccountingLSBillItem::name() const{
    return m_d->name;
}

void AccountingLSBillItem::setName( const QString & newName ){
    if( m_d->name != newName ){
        m_d->name = newName;
        emit nameChanged( newName );
        emit dataChanged( this, m_d->priceShortDescCol );
    }
}

TreeItem *AccountingLSBillItem::parentInternal() {
    return m_d->parentItem;
}

AccountingLSBillItem *AccountingLSBillItem::parent() {
    return m_d->parentItem;
}

void AccountingLSBillItem::setParent(AccountingLSBillItem * newParent, int position ) {
    if( m_d->parentItem != newParent ){
        if( m_d->parentItem != NULL ){
            m_d->parentItem->removeChild( childNumber() );
            disconnect( m_d->parentItem, &AccountingLSBillItem::attributesChanged, this, &AccountingLSBillItem::attributesChanged );
        }
        m_d->parentItem = newParent;
        if( newParent != NULL ){
            newParent->addChild( this, position);
            connect( m_d->parentItem, &AccountingLSBillItem::attributesChanged, this, &AccountingLSBillItem::attributesChanged );
        }
    } else {
        int oldPosition = childNumber();
        if( oldPosition != position ){
            if( oldPosition > position ){
                oldPosition++;
            }
            m_d->parentItem->addChild( this, position );
            m_d->parentItem->removeChild( oldPosition );
        }
    }
}

void AccountingLSBillItem::addChild(AccountingLSBillItem * newChild, int position ) {
    if( m_d->priceItem != NULL ){
        m_d->priceItem = NULL;
    }
    m_d->childrenContainer.insert( position, newChild );
}

void AccountingLSBillItem::removeChild( int position ) {
    m_d->childrenContainer.removeAt( position );
}

AccountingLSBillItem *AccountingLSBillItem::itemId( unsigned int itemId ) {
    if( itemId == m_d->id ){
        return this;
    } else {
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            AccountingLSBillItem * childItems = (*i)->itemId(itemId);
            if( childItems != NULL ) {
                return childItems;
            }
        }
    }
    return NULL;
}

AccountingLSBillItem *AccountingLSBillItem::findItemId( unsigned int itId ) {
    if( m_d->parentItem == NULL ){
        return itemId( itId );
    } else {
        return m_d->parentItem->findItemId( itId );
    }
    return NULL;
}


AccountingLSBillItem *AccountingLSBillItem::childItem(int number) {
    return dynamic_cast<AccountingLSBillItem *>(child( number ));
}

void AccountingLSBillItem::setId( unsigned int ii ) {
    m_d->id = ii;
}

unsigned int AccountingLSBillItem::id() const {
    return m_d->id;
}

QString AccountingLSBillItem::progressiveCode() const {
    if( m_d->parentItem == NULL ){
        return QString();
    } else {
        if( m_d->parentItem->progressiveCode().isEmpty() ){
            return QString::number( childNumber()+1 );
        } else {
            return m_d->parentItem->progressiveCode() + "." + QString::number( childNumber()+1 );
        }
    }
}

PriceItem *AccountingLSBillItem::priceItem() {
    return m_d->priceItem;
}

double AccountingLSBillItem::projQuantity() const {
    return m_d->projQuantity;
}

QString AccountingLSBillItem::projQuantityStr() const {
    int prec = 2;
    if( m_d->priceItem != NULL ){
        if( m_d->priceItem->unitMeasure() != NULL ){
            prec = m_d->priceItem->unitMeasure()->precision();
        }
    }
    return m_d->toString( m_d->projQuantity, 'f', prec );
}

double AccountingLSBillItem::accQuantity() const {
    return m_d->accQuantity;
}

double AccountingLSBillItem::accQuantity( const QDate & dateBegin, const QDate & dateEnd ) const {
    return m_d->measuresModel->accQuantity( dateBegin, dateEnd );
}

QString AccountingLSBillItem::accQuantityStr() const {
    int prec = 2;
    if( m_d->priceItem != NULL ){
        if( m_d->priceItem->unitMeasure() != NULL ){
            prec = m_d->priceItem->unitMeasure()->precision();
        }
    }
    return m_d->toString( m_d->accQuantity, 'f', prec );
}

QString AccountingLSBillItem::accQuantityStr( const QDate & dateBegin, const QDate & dateEnd ) const {
    int prec = 2;
    if( m_d->priceItem != NULL ){
        if( m_d->priceItem->unitMeasure() != NULL ){
            prec = m_d->priceItem->unitMeasure()->precision();
        }
    }
    return m_d->toString( accQuantity(dateBegin, dateEnd), 'f', prec );
}

double AccountingLSBillItem::PPU() const{
    return m_d->PPU;
}

QString AccountingLSBillItem::PPUStr() const {
    return m_d->toString( m_d->PPU, 'f', m_d->amountPrecision );
}

double AccountingLSBillItem::projAmount() const{
    return m_d->projAmount;
}

QString AccountingLSBillItem::projAmountStr() const {
    return m_d->toString( m_d->projAmount, 'f', m_d->amountPrecision );
}

double AccountingLSBillItem::accAmount() const{
    return m_d->accAmount;
}

double AccountingLSBillItem::accAmount(const QDate &dBegin, const QDate &dEnd) const {
    double ret = 0.0;
    if( hasChildren() ){
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->accAmount(dBegin, dEnd);
        }
    } else if( m_d->measuresModel != NULL ){
        ret = m_d->measuresModel->accQuantity( dBegin, dEnd ) * m_d->PPU;
    }
    return ret;
}

QString AccountingLSBillItem::accAmountStr() const {
    return m_d->toString( m_d->accAmount, 'f', m_d->amountPrecision );
}

QString AccountingLSBillItem::accAmountStr( const QDate &dateBegin, const QDate &dateEnd) const {
    return m_d->toString( accAmount(dateBegin, dateEnd ), 'f', m_d->amountPrecision );
}

double AccountingLSBillItem::percentageAccounted() const{
    return m_d->percentageAccounted;
}

QString AccountingLSBillItem::percentageAccountedStr() const {
    return QString("%1 %").arg( m_d->toString( m_d->percentageAccounted * 100.0, 'f', m_d->percentagePrecision ) );
}

int AccountingLSBillItem::percentagePrecision() {
    return AccountingLSBillItemPrivate::percentagePrecision;
}

double AccountingLSBillItem::percentageAccounted(const QDate &dBegin, const QDate &dEnd) const {
    double v = 0.0;
    if( m_d->projAmount != 0.0 ){
        v = UnitMeasure::applyPrecision( accAmount(dBegin, dEnd) / m_d->projAmount, m_d->percentagePrecision+2 );
    }
    return v;
}

void AccountingLSBillItem::setPriceItem(PriceItem * p) {
    if( m_d->priceItem != p ){
        if( m_d->priceItem != NULL ) {
            disconnect( m_d->priceItem, &PriceItem::codeFullChanged, this, &AccountingLSBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &AccountingLSBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &AccountingLSBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::valueChanged, this, &AccountingLSBillItem::updatePPU );
        }
        PriceItem * oldPriceItem = m_d->priceItem;

        m_d->priceItem = p;

        emit priceItemChanged( oldPriceItem, p );

        if( m_d->priceItem != NULL ){
            connect( m_d->priceItem, &PriceItem::codeFullChanged, this, &AccountingLSBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &AccountingLSBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &AccountingLSBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::valueChanged, this, &AccountingLSBillItem::updatePPU );
        }

        emit dataChanged( this, m_d->priceCodeCol );
        emit dataChanged( this, m_d->priceUmCol );

        updatePPU();

        if( m_d->measuresModel != NULL ){
            m_d->measuresModel->setUnitMeasure( m_d->priceItem->unitMeasure() );
        }
    }
}

void AccountingLSBillItem::updatePPU(){
    if( hasChildren() ){
        for( QList<AccountingLSBillItem *>::iterator i=m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->updatePPU();
        }
    } else {
        double newP = 0.0;
        if( m_d->priceItem != NULL ){
            QList<int>  totalAmPriceFields = totalAmountPriceFields();
            for( QList<int>::iterator i=totalAmPriceFields.begin(); i!=totalAmPriceFields.end(); ++i ){
                newP += m_d->priceItem->value( (*i), m_d->currentPriceDataSet );
            }
        }
        if( m_d->PPU != newP ){
            m_d->PPU = newP;
            emit PPUChanged( PPUStr() );
            updateProjAmount();
            updateAccAmount();
        }
    }
}

void AccountingLSBillItem::updateProjQuantityPrivate() {
    double v = m_d->measuresModel->projQuantity();
    if( v != m_d->projQuantity ){
        m_d->projQuantity = v;
        emit projQuantityChanged( projQuantityStr() );
        emit dataChanged( this, m_d->projQuantityCol );
        updateProjAmount();
    }
}

void AccountingLSBillItem::updateAccQuantityPrivate() {
    double v = m_d->measuresModel->accQuantity();
    if( v != m_d->accQuantity ){
        m_d->accQuantity = v;
        emit accQuantityChanged( accQuantityStr() );
        emit dataChanged( this, m_d->projAmountCol );
        updateAccAmount();
    }
}

QList<int> AccountingLSBillItem::totalAmountPriceFields(){
    if( m_d->parentItem != NULL ){
        return m_d->parentItem->totalAmountPriceFields();
    } else {
        return *(m_d->totalAmountPriceFieldsList);
    }
}

void AccountingLSBillItem::setTotalAmountPriceFields(const QList<int> &newAmountFields) {
    if( m_d->parentItem == NULL ){
        *(m_d->totalAmountPriceFieldsList) = newAmountFields;
        updatePPU();
    }
}

AccountingPriceFieldModel *AccountingLSBillItem::totalAmountPriceFieldModel() {
    return m_d->totalAmountPriceFieldModel;
}

int AccountingLSBillItem::columnCount() const {
    return m_d->colCount;
}

QVariant AccountingLSBillItem::data(int col, int role) const {
    if( (col > m_d->colCount) || (col < 0) ||
            (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::TextAlignmentRole ) ){
        return QVariant();
    }

    if( col == m_d->progNumberCol ){
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else {
                return Qt::AlignLeft + Qt::AlignVCenter;;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("N.") );
            } else {
                return QVariant( progressiveCode() );
            }
        }
    } else if( col == m_d->priceCodeCol ){
        if( role == Qt::TextAlignmentRole ){
            return Qt::AlignLeft + Qt::AlignVCenter;;
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("Codice") );
            } else {
                if( hasChildren() ){
                    return QVariant();
                } else if( m_d->priceItem ){
                    return QVariant( m_d->priceItem->codeFull() );
                } else {
                    return QVariant("---");
                }
            }
        }
    } else if( col == m_d->priceShortDescCol ){
        if( role == Qt::TextAlignmentRole ){
            return Qt::AlignLeft + Qt::AlignVCenter;
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant(QObject::trUtf8("Descrizione") );
            } else {
                if( hasChildren() ){
                    return QVariant( m_d->name );
                } else if( m_d->priceItem != NULL ){
                    return QVariant(m_d->priceItem->shortDescriptionFull());
                } else {
                    return QVariant("---");
                }
            }
        }
    } else if( col == m_d->priceUmCol ){
        if( role == Qt::TextAlignmentRole ){
            return Qt::AlignHCenter + Qt::AlignVCenter;
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("UdM") );
            } else if( hasChildren() ){
                return QVariant();
            } if( m_d->priceItem ){
                if( m_d->priceItem->unitMeasure() != NULL ){
                    return QVariant(m_d->priceItem->unitMeasure()->tag() );
                } else {
                    return QVariant( "---" );
                }
            }
        }
    } else if( col == m_d->priceCol ){
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("Costo Unitario") );
            }
            if( hasChildren() ){
                return QVariant();
            } else {
                return QVariant( PPUStr() );
            }
        }
    } else if( col == m_d->projQuantityCol ){
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("Quantità prog.") );
            }
            if( hasChildren() ){
                return QVariant();
            } else {
                return QVariant( projQuantityStr() );
            }
        }
    } else if( col == m_d->projAmountCol ){
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("Importo prog.") );
            }
            return QVariant( projAmountStr() );
        }
    } else if( col == m_d->accQuantityCol ){
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("Quantità cont.") );
            }
            if( hasChildren() ){
                return QVariant();
            } else {
                return QVariant( accQuantityStr() );
            }
        }
    } else if( col == m_d->accAmountCol ){
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("Importo cont.") );
            }
            return QVariant( accAmountStr() );
        }
    }
    return QVariant();
}

bool AccountingLSBillItem::setData(int column, const QVariant &value) {
    if( hasChildren() ){
        if( column == m_d->priceShortDescCol ){
            m_d->name = value.toString();
            return true;
        }
    }
    return false;
}

int AccountingLSBillItem::currentPriceDataSet() const{
    if( m_d->parentItem ){
        return m_d->parentItem->currentPriceDataSet();
    } else {
        return m_d->currentPriceDataSet;
    }
}

void AccountingLSBillItem::setCurrentPriceDataSet(int v ) {
    if( m_d->parentItem ){
        if( m_d->parentItem->currentPriceDataSet() != v ){
            m_d->parentItem->setCurrentPriceDataSet( v );
        }
    } else {
        if( m_d->currentPriceDataSet != v ){
            m_d->currentPriceDataSet = v;
            emit currentPriceDataSetChanged( m_d->currentPriceDataSet );
        }
    }
    updatePPU();
}

void AccountingLSBillItem::emitPriceDataUpdated() {
    if( !hasChildren() ){
        emit dataChanged( this, m_d->priceCodeCol );
        emit dataChanged( this, m_d->priceShortDescCol );
        emit dataChanged( this, m_d->priceUmCol );
    }
}

void AccountingLSBillItem::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->measuresModel != NULL ){
        m_d->measuresModel->setUnitMeasure( ump );
    }
}

void AccountingLSBillItem::updateProjAmount() {
    double v = 0.0;
    if( hasChildren() ){
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            v += (*i)->projAmount();
        }
    } else {
        v = UnitMeasure::applyPrecision( m_d->projQuantity * m_d->PPU, m_d->amountPrecision );
    }
    if( v != m_d->projAmount ){
        m_d->projAmount = v;
        emit projAmountChanged( projAmountStr() );
    }
}

void AccountingLSBillItem::updateAccAmount() {
    double v = 0.0;
    if( hasChildren() ){
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            v += (*i)->accAmount();
        }
    } else {
        v = UnitMeasure::applyPrecision( m_d->accQuantity * m_d->PPU, m_d->amountPrecision );
    }
    if( v != m_d->accAmount ){
        m_d->accAmount = v;
        emit accAmountChanged( accAmountStr() );
    }
}

void AccountingLSBillItem::updatePercentageAccounted() {
    double v = 0.0;
    if( m_d->projAmount != 0.0 ){
        v = UnitMeasure::applyPrecision( m_d->accAmount / m_d->projAmount, m_d->percentagePrecision+2 );
    }
    if( v != m_d->projAmount ){
        m_d->percentageAccounted = v;
        emit percentageAccountedChanged( percentageAccountedStr() );
    }
}

bool AccountingLSBillItem::isUsingPriceItem(PriceItem *p) {
    if( hasChildren() ){
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            if( (*i)->isUsingPriceItem(p) ){
                return true;
            }
        }
        if( m_d->priceItem == p ){
            m_d->priceItem = NULL;
        }
    } else {
        if( m_d->priceItem == p ){
            return true;
        }
    }
    return false;
}

QList<PriceItem *> AccountingLSBillItem::usedPriceItems() const {
    QList<PriceItem *> ret;
    appendUsedPriceItems( &ret );
    return ret;
}

void AccountingLSBillItem::appendUsedPriceItems( QList<PriceItem *> * usedPriceItems ) const {
    if( m_d->childrenContainer.size() == 0 ){
        if( !usedPriceItems->contains( m_d->priceItem )){
            usedPriceItems->append( m_d->priceItem );
        }
    } else {
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->appendUsedPriceItems( usedPriceItems );
        }
    }
}

void AccountingLSBillItem::setHasChildrenChanged(AccountingLSBillItem * p, QList<int> indexes) {
    if( m_d->parentItem ){
        // non è l'oggetto root - rimandiamo all'oggetto root
        m_d->parentItem->setHasChildrenChanged( p, indexes );
    } else {
        // è l'oggetto root - emette il segnale di numero figli cambiato
        emit hasChildrenChanged( p, indexes );
    }
}

TreeItem *AccountingLSBillItem::child(int number) {
    if( number >= 0 && number < m_d->childrenContainer.size() ){
        return m_d->childrenContainer[number];
    }
    return NULL;
}

int AccountingLSBillItem::childrenCount() const {
    return m_d->childrenContainer.size();
}

bool AccountingLSBillItem::hasChildren() const {
    return (m_d->childrenContainer.size() > 0);
}

QList<AccountingLSBillItem *> AccountingLSBillItem::allChildren() {
    QList<AccountingLSBillItem *> ret;
    for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        ret.append( *i );
        if( (*i)->hasChildren() ){
            ret.append( (*i)->allChildren() );
        }
    }
    return ret;
}

QList<AccountingLSBillItem *> AccountingLSBillItem::allChildrenWithMeasures() {
    QList<AccountingLSBillItem *> ret;
    for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        if( (*i)->hasChildren() ){
            ret.append( (*i)->allChildrenWithMeasures() );
        } else {
            ret.append( this );
        }
    }
    return ret;
}

bool AccountingLSBillItem::insertChildren(PriceItem * p, int position, int count ){
    if (position < 0 || position > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row) {
        AccountingLSBillItem *item = new AccountingLSBillItem( p, this, m_d->priceFieldModel, m_d->parser );
        while( findItemId( item->id() ) != NULL ){
            item->setId( item->id() + 1 );
        }
        m_d->childrenContainer.insert(position, item);
        connect( item, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem *,int)> (&AccountingLSBillItem::dataChanged), this, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem*,int)> (&AccountingLSBillItem::dataChanged) );
        connect( item, &AccountingLSBillItem::projAmountChanged, this, &AccountingLSBillItem::updateProjAmount );
        connect( item, &AccountingLSBillItem::accAmountChanged, this, &AccountingLSBillItem::updateAccAmount );
        connect( this, &AccountingLSBillItem::currentPriceDataSetChanged, item, &AccountingLSBillItem::setCurrentPriceDataSet );
        connect( item, &AccountingLSBillItem::itemChanged, this, &AccountingLSBillItem::itemChanged );
    }

    if( !hadChildren ){
        if( m_d->childrenContainer.size() > 0 ){
            emit hasChildrenChanged( true );
        }
    }

    emit itemChanged();

    return true;
}

bool AccountingLSBillItem::insertChildren(int position, int count) {
    return insertChildren( NULL, position, count );
}

bool AccountingLSBillItem::appendChildren(int count) {
    return insertChildren( m_d->childrenContainer.size(), count );
}

bool AccountingLSBillItem::removeChildren(int position, int count) {
    if (position < 0 || position + count > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row){
        AccountingLSBillItem * item = m_d->childrenContainer.at( position );
        disconnect( item, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem *,int)> (&AccountingLSBillItem::dataChanged), this, static_cast<void(AccountingLSBillItem::*)(AccountingLSBillItem *,int)> (&AccountingLSBillItem::dataChanged) );
        disconnect( item, &AccountingLSBillItem::projAmountChanged, this, &AccountingLSBillItem::updateProjAmount );
        disconnect( this, &AccountingLSBillItem::currentPriceDataSetChanged, item, &AccountingLSBillItem::setCurrentPriceDataSet );
        disconnect( item, &AccountingLSBillItem::itemChanged, this, &AccountingLSBillItem::itemChanged );
        delete item;
        m_d->childrenContainer.removeAt( position );
    }
    if( hadChildren ){
        if( !(m_d->childrenContainer.size() > 0) ){
            emit hasChildrenChanged( false );
        }
    }

    emit itemChanged();

    return true;
}

bool AccountingLSBillItem::reset() {
    return removeChildren( 0, m_d->childrenContainer.size() );
}

int AccountingLSBillItem::childNumber() const {
    if (m_d->parentItem != NULL ) {
        return m_d->parentItem->m_d->childrenContainer.indexOf( const_cast<AccountingLSBillItem *>(this) );
    }
    return 0;
}

MeasuresLSModel * AccountingLSBillItem::measuresModel() {
    return m_d->measuresModel;
}

Qt::ItemFlags AccountingLSBillItem::flags(int column) const {
    if( column == m_d->progNumberCol ){
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    if( hasChildren() ){
        if( column == m_d->priceShortDescCol ){
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        }
    } else {
        if( column == m_d->projQuantityCol ){
            if( m_d->measuresModel == NULL ){
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
            } else {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            }
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void AccountingLSBillItem::writeXml(QXmlStreamWriter *writer) {
    if( m_d->parentItem != NULL ){
        // se non e' l'elemento root
        writer->writeStartElement( "AccountingLSBillItem" );
        writer->writeAttribute( "id", QString::number(m_d->id) );

        QString attrs;
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( attrs.isEmpty() ){
                attrs = QString::number( (*i)->id() );
            } else {
                attrs = QString( "%1, %2" ).arg( attrs, QString::number( (*i)->id() ) );
            }
        }
        if( !attrs.isEmpty() ){
            writer->writeAttribute( "attributes", attrs );
        }

        if( m_d->childrenContainer.size() > 0 ){
            writer->writeAttribute( "name", m_d->name );
            for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeXml( writer );
            }
        } else {
            if( m_d->priceItem != NULL ){
                writer->writeAttribute( "priceItem", QString::number( m_d->priceItem->id() ) );
            }
            writer->writeAttribute( "quantity", QString::number( m_d->projQuantity ) );
            writer->writeAttribute( "priceDataSet", QString::number(m_d->currentPriceDataSet) );

            if( m_d->measuresModel ){
                m_d->measuresModel->writeXml( writer );
            }
        }

        writer->writeEndElement();
    } else {
        // e' l'elemento root
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml( writer );
        }
    }
}

void AccountingLSBillItem::readXml20(QXmlStreamReader *reader, PriceList * priceList, AttributesModel * billAttrModel ) {
    if( m_d->parentItem != NULL ){
        if(reader->isStartElement() && reader->name().toString().toUpper() == "ACCOUNTINGLSBILLITEM"){
            loadFromXml( reader->attributes(), priceList, billAttrModel );
        }
        reader->readNext();
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGLSBILLITEM")&&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGLSBILL")  ){
        if( reader->name().toString().toUpper() == "ACCOUNTINGLSBILLITEM" && reader->isStartElement()) {
            appendChildren();
            m_d->childrenContainer.last()->readXml20( reader, priceList, billAttrModel );
        }
        if( reader->name().toString().toUpper() == "MEASURESLSMODEL" && reader->isStartElement() ) {
            m_d->measuresModel->readXml( reader );
        }
        reader->readNext();
    }
}

void AccountingLSBillItem::readXmlTmp(QXmlStreamReader *reader) {
    if( m_d->parentItem != NULL ){
        if(reader->isStartElement() && reader->name().toString().toUpper() == "BILLITEM"){
            loadFromXmlTmp( reader->attributes() );
        }
        reader->readNext();
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLITEM")&&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILL")  ){
        if( reader->name().toString().toUpper() == "BILLITEM" && reader->isStartElement()) {
            appendChildren();
            m_d->childrenContainer.last()->readXmlTmp( reader );
        }
        if( reader->name().toString().toUpper() == "MEASURESLSMODEL" && reader->isStartElement() ) {
            m_d->measuresModel->readXml( reader );
        }
        reader->readNext();
    }
}

void AccountingLSBillItem::loadFromXml(const QXmlStreamAttributes &attrs, PriceList * priceList, AttributesModel * billAttrModel) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "name" ) ){
        m_d->name = attrs.value( "name").toString();
    }
    if( attrs.hasAttribute( "attributes" ) ){
        QStringList attributes = attrs.value( "attributes").toString().split(',');
        for( QStringList::iterator i = attributes.begin(); i != attributes.end(); ++i ){
            bool ok = false;
            unsigned int attrId = (*i).toUInt(&ok);
            if( ok ){
                addAttribute( billAttrModel->attributeId( attrId ) );
            }
        }
    }
    if( attrs.hasAttribute( "priceDataSet" ) ){
        setCurrentPriceDataSet( attrs.value( "priceDataSet").toInt() );
    }
    if( attrs.hasAttribute( "priceItem" ) ){
        if( priceList ){
            setPriceItem( priceList->priceItemId( attrs.value( "priceItem").toUInt() ) );
        }
    }
    if( attrs.hasAttribute( "priceDataSet" ) ){
        setCurrentPriceDataSet( attrs.value( "priceDataSet").toInt() );
    }
}

void AccountingLSBillItem::loadFromXmlTmp(const QXmlStreamAttributes &attrs) {
    m_d->tmpAttributes.clear();
    m_d->tmpAttributes = attrs;
}

void AccountingLSBillItem::loadTmpData( PriceList *priceList, AttributesModel * billAttrModel) {
    if( !m_d->tmpAttributes.isEmpty() ){
        loadFromXml(m_d->tmpAttributes, priceList, billAttrModel );
        m_d->tmpAttributes.clear();
    }
    for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->loadTmpData( priceList, billAttrModel );
    }
}

QList< QPair<Attribute *, bool> > AccountingLSBillItem::attributes() {
    QList< QPair<Attribute *, bool> > ret;
    QList<Attribute *> inherhitedAttrs = inheritedAttributes();

    for( QList<Attribute *>::iterator i = inherhitedAttrs.begin(); i != inherhitedAttrs.end(); ++i ){
        ret.append( qMakePair(*i, false) );
    }
    for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
        if( !inherhitedAttrs.contains( *i) ){
            ret.append( qMakePair(*i, true) );
        }
    }
    return ret;
}

QList<Attribute *> AccountingLSBillItem::allAttributes() {
    QList<Attribute *> attrs = inheritedAttributes();
    for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
        if( !attrs.contains( *i) ){
            attrs.append( *i );
        }
    }
    return attrs;
}

QList<Attribute *> AccountingLSBillItem::directAttributes() {
    return QList<Attribute *>( m_d->attributes );
}

void AccountingLSBillItem::addAttribute( Attribute * attr ){
    if( !(m_d->attributes.contains( attr )) ){
        m_d->attributes.append( attr );
        emit attributesChanged();
    }
}

void AccountingLSBillItem::removeAttribute( Attribute * attr ){
    if( m_d->attributes.removeAll( attr ) > 0 ){
        emit attributesChanged();
    }
}

void AccountingLSBillItem::removeAllAttributes() {
    m_d->attributes.clear();
    emit attributesChanged();
}

double AccountingLSBillItem::projAmountAttribute(Attribute *attr ) {
    if( m_d->childrenContainer.size() == 0 ){
        QList<Attribute *> attrs = inheritedAttributes();
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( !attrs.contains( *i) ){
                attrs.append( *i );
            }
        }
        if( attrs.contains(attr) ){
            return projAmount();
        }
    } else {
        double ret = 0.0;
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->projAmountAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingLSBillItem::projAmountAttributeStr(Attribute *attr) {
    QString ret;
    double v = projAmountAttribute( attr );
    if( m_d->parser == NULL ){
        ret = QString::number(v, 'f', m_d->amountPrecision );
    } else {
        ret = m_d->parser->toString( v, 'f', m_d->amountPrecision );
    }
    return ret;
}

double AccountingLSBillItem::accAmountAttribute(Attribute *attr ) {
    if( m_d->childrenContainer.size() == 0 ){
        QList<Attribute *> attrs = inheritedAttributes();
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( !attrs.contains( *i) ){
                attrs.append( *i );
            }
        }
        if( attrs.contains(attr) ){
            return accAmount();
        }
    } else {
        double ret = 0.0;
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->accAmountAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingLSBillItem::accAmountAttributeStr(Attribute *attr) {
    QString ret;
    double v = accAmountAttribute( attr );
    if( m_d->parser == NULL ){
        ret = QString::number(v, 'f', m_d->amountPrecision );
    } else {
        ret = m_d->parser->toString( v, 'f', m_d->amountPrecision );
    }
    return ret;
}

QList<Attribute *> AccountingLSBillItem::inheritedAttributes(){
    QList<Attribute *> ret;
    if( m_d->parentItem != NULL ){
        ret.append( m_d->parentItem->inheritedAttributes() );
    }
    for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
        if( !ret.contains( *i) ){
            ret.append( *i );
        }
    }
    return ret;
}

bool AccountingLSBillItem::containsAttribute(Attribute *attr) {
    if( containsAttributeDirect(attr)){
        return true;
    }
    return containsAttributeInherited( attr );
}

bool AccountingLSBillItem::containsAttributeInherited(Attribute *attr) {
    if( m_d->parentItem != NULL ){
        if( m_d->parentItem->containsAttributeDirect( attr ) ){
            return true;
        } else {
            return m_d->parentItem->containsAttributeInherited( attr );
        }
    }
    return false;
}

bool AccountingLSBillItem::containsAttributeDirect(Attribute *attr) {
    return m_d->attributes.contains( attr );
}

bool AccountingLSBillItem::isDescending(AccountingLSBillItem *ancestor) {
    if( m_d->parentItem == NULL ){
        return (m_d->parentItem == ancestor);
    } else {
        if( m_d->parentItem == ancestor ){
            return true;
        } else {
            return m_d->parentItem->isDescending( ancestor );
        }
    }
}

QList<PriceItem *> AccountingLSBillItem::connectedPriceItems() const {
    QList<PriceItem *> ret;
    for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        ret.append( (*i)->priceItem()->connectedPriceItems() );
    }
    return ret;
}

#include "qtextformatuserdefined.h"

void AccountingLSBillItem::writeODTAccountingOnTable( QTextCursor *cursor,
                                                      const QDate &dateBegin, const QDate &dateEnd,
                                                      AccountingPrinter::PrintLSOption prLSOption,
                                                      AccountingPrinter::PrintPPUDescOption prPPUOption,
                                                      bool writeAmounts ) {
    // spessore del bordo della tabella
    double borderWidth = 1.0f;

    // *** formattazione intestazioni ***
    static QTextBlockFormat headerBlockFormat;
    headerBlockFormat.setAlignment( Qt::AlignHCenter );

    // *** formattazione generica ***
    // unità di misura
    static QTextBlockFormat tagBlockFormat;
    tagBlockFormat.setAlignment( Qt::AlignHCenter );
    // testi
    static QTextBlockFormat txtBlockFormat;
    txtBlockFormat.setAlignment( Qt::AlignLeft );
    static QTextCharFormat txtCharFormat;
    static QTextCharFormat txtBoldCharFormat = txtCharFormat;
    txtBoldCharFormat.setFontWeight( QFont::Bold );
    static QTextCharFormat txtFormulaCharFormat = txtCharFormat;
    txtFormulaCharFormat.setFontItalic( true);
    // numeri
    static QTextBlockFormat numBlockFormat;
    numBlockFormat.setAlignment( Qt::AlignRight );

    // *** formattazione celle generiche ***
    // centrale
    static QTextTableCellFormat centralFormat;
    // sinistra
    static QTextTableCellFormat leftFormat;
    leftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightFormat;
    rightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );
    // quantità computo, centrale
    static QTextTableCellFormat centralQuantityTotalFormat;
    centralQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Double) );
    centralQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    // quantità computo, destra
    static QTextTableCellFormat rightQuantityTotalFormat;
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Double) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle intestazione ***
    // centrale
    static QTextTableCellFormat centralHeaderFormat;
    centralHeaderFormat.setFontWeight( QFont::Bold );
    centralHeaderFormat.setBackground( Qt::lightGray );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );
    // sinistra
    static QTextTableCellFormat leftHeaderFormat = centralHeaderFormat;
    leftHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightHeaderFormat = centralHeaderFormat;
    rightHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle sottotitoli (totali parziali) ***
    // centrale
    static QTextTableCellFormat centralSubTitleFormat;
    centralSubTitleFormat.setFontWeight( QFont::Bold );
    centralSubTitleFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftSubTitleFormat = centralSubTitleFormat;
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightSubTitleFormat = centralSubTitleFormat;
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle totale complessivo ***
    // centrale
    static QTextTableCellFormat centralTitleFormat;
    centralTitleFormat.setFontWeight( QFont::Bold );
    // sinistra
    static QTextTableCellFormat leftTitleFormat = centralTitleFormat;
    leftTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightTitleFormat = centralTitleFormat;
    rightTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle riga chiusura ***
    // centrale
    static QTextTableCellFormat centralBottomFormat;
    centralBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );
    // sinistra
    static QTextTableCellFormat leftBottomFormat = centralBottomFormat;
    leftBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightBottomFormat = centralBottomFormat;
    rightBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    int colCount = 0; // numero complessivo colonne
    if( prLSOption == AccountingPrinter::PrintLSProj ){
        colCount = 5;
        if( writeAmounts ){
            colCount += 2;
        }
    } else if( prLSOption == AccountingPrinter::PrintLSAcc ){
        // numero progressivo + codice + descrizione + unità di misura + data cont. + quantità cont. [+ prezzo + importo cont.]
        colCount = 6;
        if( writeAmounts ){
            colCount += 2;
        }
    } else if( prLSOption == AccountingPrinter::PrintLSProjAcc ){
        // numero progressivo + codice + descrizione + unità di misura [ + prezzo ] +
        // quantita' prog [+importo prog.] + data cont. + quantità cont. [+ importo cont.]
        colCount = 7;
        if( writeAmounts ){
            colCount += 3;
        }
    }

    if( m_d->parentItem == NULL ){
        // *** Intestazione tabella ***

        if( prLSOption == AccountingPrinter::PrintLSProj ){
            // numero progressivo + codice + descrizione + unità di misura + quantità [+ prezzo + importo ]
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N."), false);
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità prog."));
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo prog"));
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità prog."));
            }
        } else if( prLSOption == AccountingPrinter::PrintLSAcc ){
            // numero progressivo + codice + descrizione + unità di misura + data cont. + quantità cont. [+ prezzo + importo cont.]
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N."), false);
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Data"));
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità cont."));
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo cont."));
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità cont."));
            }
        } else if( prLSOption == AccountingPrinter::PrintLSProjAcc ){
            // numero progressivo + codice + descrizione + unità di misura [ + prezzo ] +
            // quantita' prog [+importo prog.] + data cont. + quantità cont. [+ importo cont.]
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N."), false);
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
            }
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità prog."));
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Importo prog."));
            }
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Data cont."));
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità cont."));
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo cont."));
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità cont."));
            }
        }

        // *** Riga vuota ***
        AccountingLSBillItemPrivate::insertEmptyRow( colCount, cursor, leftFormat, centralFormat, rightFormat );

        // *** Scrive il computo dei sottoarticoli ***
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeODTAccountingOnTable( cursor, dateBegin, dateEnd,
                                             prLSOption, prPPUOption, writeAmounts );
        }

        // *** riga dei totali complessivi

        if( writeAmounts ){
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            // n prog.
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            // codice
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            // descrizione
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale complessivo") );
            // unita di misura
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            for( int i=4; i<(colCount-1); ++i ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            }
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, accAmountStr( dateBegin, dateEnd ) );

            // *** Riga di chiusura ***
            AccountingLSBillItemPrivate::insertEmptyRow( colCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
        }
    } else {
        if( hasChildren() ){
            if( prLSOption == AccountingPrinter::PrintLSProj ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                // numero progressivo + codice + descrizione + unità di misura + quantità [+ prezzo + importo ]
                // n prog
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat, progressiveCode() );
                // codice
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                // desc
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, m_d->name );
                // udm
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );

                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
                }
            } else if( prLSOption == AccountingPrinter::PrintLSAcc ){
                // numero progressivo + codice + descrizione + unità di misura + data cont. + quantità cont. [+ prezzo + importo cont.]
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                // n prog
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat, progressiveCode() );
                // articolo prezzo
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                // desc
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, m_d->name );
                // udm
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                // data
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                if( writeAmounts ){
                    // quantita cont
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    // prezzo
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    // importo cont
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
                } else {
                    // quantita cont
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
                }
            } else if( prLSOption == AccountingPrinter::PrintLSProjAcc ){
                // numero progressivo + codice + descrizione + unità di misura [ + prezzo ] +
                // quantita' prog [+importo prog.] + data cont. + quantità cont. [+ importo cont.]
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                // n prog
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat, progressiveCode() );
                // codice
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                // desc
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, m_d->name );
                // udm
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );

                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                }
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                }
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
                }
            }

            // *** Riga vuota ***
            AccountingLSBillItemPrivate::insertEmptyRow( colCount, cursor, leftFormat, centralFormat, rightFormat );

            // *** Scrive il computo dei sottoarticoli ***
            for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTAccountingOnTable( cursor, dateBegin, dateEnd,
                                                 prLSOption, prPPUOption, writeAmounts );
            }

            // *** riga dei totali complessivi ***
            if( writeAmounts ){

                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                if( prLSOption == AccountingPrinter::PrintLSProj ){
                    // numero progressivo + codice + descrizione + unità di misura + quantità [+ prezzo + importo ]
                    AccountingLSBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale %1").arg( m_d->name ) );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );

                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, projAmountStr() );
                } else if( prLSOption == AccountingPrinter::PrintLSAcc ){
                    // numero progressivo + codice + descrizione + unità di misura + data cont. + quantità cont. [+ prezzo + importo cont.]
                    // n prog
                    AccountingLSBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat, progressiveCode() );
                    // codice
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                    // desc
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale %1").arg( m_d->name ) );
                    // udm
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                    // data
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                    // prezzo
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    // quantita cont.
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    // importo cont.
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, accAmountStr( dateBegin, dateEnd) );
                } else if( prLSOption == AccountingPrinter::PrintLSProjAcc ){
                    // numero progressivo + codice + descrizione + unità di misura [ + prezzo ] +
                    // quantita' prog [+importo prog.] + data cont. + quantità cont. [+ importo cont.]
                    AccountingLSBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat, progressiveCode() );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale %1").arg( m_d->name ) );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );

                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat, projAmountStr() );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, accAmountStr( dateBegin, dateEnd) );
                }
                // *** Riga di chiusura ***
                AccountingLSBillItemPrivate::insertEmptyRow( colCount, cursor, leftFormat, centralFormat, rightFormat );
            }
        } else { // !hasChildren()

            writeODTBillLine( cursor, table,
                              dateBegin, dateEnd,
                              prLSOption, prPPUOption,
                              writeAmounts, true,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat, txtFormulaCharFormat );
            AccountingLSBillItemPrivate::insertEmptyRow( colCount, cursor, leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingLSBillItem::printODTSummaryOnTable(QTextCursor *cursor,
                                                  AccountingPrinter::PrintPPUDescOption prItemsOption,
                                                  bool writeAmounts,
                                                  bool writeDetails ) {
    // spessore del bordo della tabella
    double borderWidth = 1.0f;

    // *** formattazione intestazioni ***
    static QTextBlockFormat headerBlockFormat;
    headerBlockFormat.setAlignment( Qt::AlignHCenter );

    // *** formattazione generica ***
    // unità di misura
    static QTextBlockFormat tagBlockFormat;
    tagBlockFormat.setAlignment( Qt::AlignHCenter );
    // testi
    static QTextBlockFormat txtBlockFormat;
    txtBlockFormat.setAlignment( Qt::AlignLeft );
    static QTextCharFormat txtCharFormat;
    static QTextCharFormat txtBoldCharFormat = txtCharFormat;
    txtBoldCharFormat.setFontWeight( QFont::Bold );
    // numeri
    static QTextBlockFormat numBlockFormat;
    numBlockFormat.setAlignment( Qt::AlignRight );

    // *** formattazione celle generiche ***
    // centrale
    static QTextTableCellFormat centralFormat;
    // sinistra
    static QTextTableCellFormat leftFormat;
    leftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightFormat;
    rightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );
    // quantità computo, centrale
    static QTextTableCellFormat centralQuantityTotalFormat;
    centralQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Double) );
    centralQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    // quantità computo, destra
    static QTextTableCellFormat rightQuantityTotalFormat;
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Double) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle intestazione ***
    // centrale
    static QTextTableCellFormat centralHeaderFormat;
    centralHeaderFormat.setFontWeight( QFont::Bold );
    centralHeaderFormat.setBackground( Qt::lightGray );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );
    // sinistra
    static QTextTableCellFormat leftHeaderFormat = centralHeaderFormat;
    leftHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightHeaderFormat = centralHeaderFormat;
    rightHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle sottotitoli (totali parziali) ***
    // centrale
    static QTextTableCellFormat centralSubTitleFormat;
    centralSubTitleFormat.setFontWeight( QFont::Bold );
    centralSubTitleFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftSubTitleFormat = centralSubTitleFormat;
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightSubTitleFormat = centralSubTitleFormat;
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle totale complessivo ***
    // centrale
    static QTextTableCellFormat centralTitleFormat;
    centralTitleFormat.setFontWeight( QFont::Bold );
    // sinistra
    static QTextTableCellFormat leftTitleFormat = centralTitleFormat;
    leftTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightTitleFormat = centralTitleFormat;
    rightTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle riga chiusura ***
    // centrale
    static QTextTableCellFormat centralBottomFormat;
    centralBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );
    // sinistra
    static QTextTableCellFormat leftBottomFormat = centralBottomFormat;
    leftBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightBottomFormat = centralBottomFormat;
    rightBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // numero complessivo colonne
    int cellCount = 4;
    if( writeAmounts ){
        cellCount += 2;
    }

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    // *** Intestazione tabella ***
    AccountingLSBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("Codice"), false );
    AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Denominazione"));
    AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( writeAmounts ){
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importi"));
    } else {
        AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    }

    // *** riga vuota ***
    AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    // *** Righe del sommario ***
    QList<PriceItem *> usedPItems = usedPriceItems();
    for( QList<PriceItem *>::iterator i = usedPItems.begin(); i != usedPItems.end(); ++i){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        if( (*i) != NULL ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, (*i)->codeFull() );
            m_d->writeDescriptionCell( (*i), cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prItemsOption );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }

        QString unitMeasureTag;
        int unitMeasurePrec = 3;
        if( (*i) != NULL ){
            if( (*i)->unitMeasure() != NULL ){
                unitMeasureTag = (*i)->unitMeasure()->tag();
                unitMeasurePrec = (*i)->unitMeasure()->precision();
            }
        }

        if( writeDetails ){
            // *** termina la riga **
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );

            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
        }

        double itemProjTotalAmount = 0.0;
        double itemProjTotalQuantity = 0.0;
        double itemAccTotalAmount = 0.0;
        double itemAccTotalQuantity = 0.0;
        for( QList<AccountingLSBillItem *>::iterator j = m_d->childrenContainer.begin(); j != m_d->childrenContainer.end(); ++j ){
            (*j)->writeODTSummaryLine( *i, cursor,
                                       &itemProjTotalQuantity, &itemProjTotalAmount, &itemAccTotalQuantity, &itemAccTotalAmount,
                                       writeAmounts, writeDetails,
                                       table,
                                       tagBlockFormat, txtBlockFormat, numBlockFormat,
                                       leftFormat, centralFormat, rightFormat );
        }

        if( writeDetails ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( itemAccTotalQuantity, 'f', unitMeasurePrec ));
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUStr() );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemAccTotalAmount, 'f', m_d->amountPrecision ) );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemAccTotalQuantity, 'f', unitMeasurePrec ));
        }

        AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
    }

    // *** Totale complessivo ***
    // *** riga dei totali complessivi
    if( writeAmounts ){

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        // codice
        AccountingLSBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        // descrizione
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale complessivo") );
        // Udm
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        // quantita'
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );

        AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, accAmountStr() );
    }

    // *** Riga di chiusura ***
    AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

void AccountingLSBillItem::writeODTSummaryLine( PriceItem * priceItem,
                                                QTextCursor *cursor,
                                                double * itemProjTotalQuantity,
                                                double * itemProjTotalAmount,
                                                double * itemAccTotalQuantity,
                                                double * itemAccTotalAmount,
                                                bool writeAmounts,
                                                bool writeDetails,
                                                QTextTable *table,
                                                QTextBlockFormat & tagBlockFormat,
                                                QTextBlockFormat & txtBlockFormat,
                                                QTextBlockFormat & numBlockFormat,
                                                QTextTableCellFormat & leftFormat,
                                                QTextTableCellFormat & centralFormat,
                                                QTextTableCellFormat & rightFormat ) {
    if( m_d->childrenContainer.size() == 0 ){
        if( priceItem == m_d->priceItem ){
            (*itemProjTotalQuantity) += projQuantity();
            (*itemProjTotalAmount) += projAmount();
            (*itemAccTotalQuantity) += accQuantity();
            (*itemAccTotalAmount) += accAmount();
            if( writeDetails ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, progressiveCode() );

                QString unitMeasureTag;
                if( m_d->priceItem != NULL ){
                    if( m_d->priceItem->unitMeasure() != NULL ){
                        unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                    }
                }
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, accQuantityStr() );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, accQuantityStr() );
                }

                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
            }
        }
    } else {
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTSummaryLine( priceItem, cursor,
                                       itemProjTotalQuantity, itemProjTotalAmount, itemAccTotalQuantity, itemAccTotalAmount,
                                       writeAmounts, writeDetails,
                                       table,
                                       tagBlockFormat, txtBlockFormat, numBlockFormat,
                                       leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingLSBillItem::printODTAttributeBillOnTable(QTextCursor *cursor,
                                                        AccountingPrinter::AttributePrintOption prOption,
                                                        AccountingPrinter::PrintPPUDescOption prItemsOption,
                                                        const QList<Attribute *> &attrsToPrint,
                                                        bool writeAmounts) {
    // spessore del bordo della tabella
    double borderWidth = 1.0f;

    // *** formattazione intestazioni ***
    static QTextBlockFormat headerBlockFormat;
    headerBlockFormat.setAlignment( Qt::AlignHCenter );

    // *** formattazione generica ***
    // unità di misura
    static QTextBlockFormat tagBlockFormat;
    tagBlockFormat.setAlignment( Qt::AlignHCenter );
    // testi
    static QTextBlockFormat txtBlockFormat;
    txtBlockFormat.setAlignment( Qt::AlignLeft );
    static QTextCharFormat txtCharFormat;
    static QTextCharFormat txtBoldCharFormat = txtCharFormat;
    txtBoldCharFormat.setFontWeight( QFont::Bold );
    // numeri
    static QTextBlockFormat numBlockFormat;
    numBlockFormat.setAlignment( Qt::AlignRight );

    // *** formattazione celle generiche ***
    // centrale
    static QTextTableCellFormat centralFormat;
    // sinistra
    static QTextTableCellFormat leftFormat;
    leftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightFormat;
    rightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );
    // quantità computo, centrale
    static QTextTableCellFormat centralQuantityTotalFormat = centralFormat;
    centralQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Double) );
    centralQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    // quantità computo, destra
    static QTextTableCellFormat rightQuantityTotalFormat;
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Double) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightQuantityTotalFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle intestazione ***
    // centrale
    static QTextTableCellFormat centralHeaderFormat;
    centralHeaderFormat.setFontWeight( QFont::Bold );
    centralHeaderFormat.setBackground( Qt::lightGray );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );
    // sinistra
    static QTextTableCellFormat leftHeaderFormat = centralHeaderFormat;
    leftHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightHeaderFormat = centralHeaderFormat;
    rightHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle sottotitoli (totali parziali) ***
    // centrale
    static QTextTableCellFormat centralSubTitleFormat;
    centralSubTitleFormat.setFontWeight( QFont::Bold );
    centralSubTitleFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftSubTitleFormat = centralSubTitleFormat;
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightSubTitleFormat = centralSubTitleFormat;
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle totale complessivo ***
    // centrale
    static QTextTableCellFormat centralTitleFormat;
    centralTitleFormat.setFontWeight( QFont::Bold );
    // sinistra
    static QTextTableCellFormat leftTitleFormat = centralTitleFormat;
    leftTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightTitleFormat = centralTitleFormat;
    rightTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle riga chiusura ***
    // centrale
    static QTextTableCellFormat centralBottomFormat;
    centralBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    centralBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );
    // sinistra
    static QTextTableCellFormat leftBottomFormat = centralBottomFormat;
    leftBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightBottomFormat = centralBottomFormat;
    rightBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightBottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // numero complessivo colonne
    int cellCount = 4;
    if( writeAmounts ){
        cellCount += 2;
    }

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    // *** Intestazione tabella ***
    AccountingLSBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("Codice"), false );
    AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Denominazione"));
    AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( writeAmounts ){
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, PPUStr() );
        AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, accAmountStr() );
    } else {
        AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    }

    // *** riga vuota ***
    AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    double itemProjTotalAmount = 0.0;
    double itemAccTotalAmount = 0.0;

    if( prOption ==AccountingPrinter::AttributePrintSimple ){
        // *** Righe del computo suddivise per attributo ***
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            double itemProjTotalAmount = 0.0;
            double itemAccTotalAmount = 0.0;
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            // cursor->movePosition(QTextCursor::NextCell);

            AccountingLSBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, (*i)->name() );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            }

            // *** riga vuota ***
            AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            writeODTAttributeBillLineSimple( prItemsOption,
                                             &itemProjTotalAmount, &itemAccTotalAmount,
                                             *i, writeAmounts,
                                             cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                             leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                             txtCharFormat, txtBoldCharFormat );

            AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingLSBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale %1").arg((*i)->name()) );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( itemAccTotalAmount, 'f', m_d->amountPrecision) );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            }

            if( i != -- attrsToPrint.end()){
                // *** riga vuota ***
                AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
                // *** riga vuota ***
                AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            }
        }
    } else if( prOption == AccountingPrinter::AttributePrintUnion ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);

        AccountingLSBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        QString title(trUtf8("Unione"));
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            title = QString("%1 %2").arg(title, (*i)->name() );
        }
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, title );
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario") );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importi") );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        }

        writeODTAttributeBillLineUnion( prItemsOption,
                                        &itemProjTotalAmount, &itemAccTotalAmount,
                                        writeAmounts, attrsToPrint,
                                        cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                        leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                        txtCharFormat, txtBoldCharFormat );

        // *** riga vuota ***
        AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        AccountingLSBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , title );
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( itemAccTotalAmount, 'f', m_d->amountPrecision ) );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        }
    } else if( prOption == AccountingPrinter::AttributePrintIntersection ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);

        AccountingLSBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        QString title(trUtf8("Intersezione"));
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            title = QString("%1 %2").arg(title, (*i)->name() );
        }
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, title );
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario") );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importi") );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        }

        writeODTAttributeBillLineIntersection( prItemsOption,
                                               &itemProjTotalAmount, &itemAccTotalAmount,
                                               attrsToPrint, writeAmounts,
                                               cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                               leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                               txtCharFormat, txtBoldCharFormat );

        // *** riga vuota ***
        AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        AccountingLSBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , title );
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( itemAccTotalAmount, 'f', m_d->amountPrecision ) );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        }
    }

    // *** Riga di chiusura ***
    AccountingLSBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

void AccountingLSBillItem::writeODTAttributeBillLineSimple(AccountingPrinter::PrintPPUDescOption prItemsOption,
                                                           double * itemProjTotalAmount, double * itemAccTotalAmount,
                                                           Attribute *attrsToPrint,
                                                           bool writeAmounts,
                                                           QTextCursor *cursor,
                                                           QTextTable *table,
                                                           QTextBlockFormat &tagBlockFormat,
                                                           QTextBlockFormat &txtBlockFormat,
                                                           QTextBlockFormat &numBlockFormat,
                                                           QTextTableCellFormat &leftFormat,
                                                           QTextTableCellFormat &centralFormat,
                                                           QTextTableCellFormat &rightFormat,
                                                           QTextTableCellFormat &centralQuantityTotalFormat,
                                                           QTextTableCellFormat &rightQuantityTotalFormat,
                                                           QTextCharFormat & txtCharFormat,
                                                           QTextCharFormat & txtBoldCharFormat ) {

    if( !hasChildren() ){
        if( containsAttribute( attrsToPrint ) ){
            *itemProjTotalAmount += projAmount();
            *itemAccTotalAmount += accAmount();
            writeODTBillLine( prItemsOption,
                              false, writeAmounts,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineSimple( prItemsOption,
                                                   itemProjTotalAmount, itemAccTotalAmount,
                                                   attrsToPrint, writeAmounts,
                                                   cursor, table,
                                                   tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                   leftFormat, centralFormat, rightFormat,
                                                   centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                   txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingLSBillItem::writeODTAttributeBillLineUnion( AccountingPrinter::PrintPPUDescOption prItemsOption,
                                                           double * itemProjTotalAmount, double * itemAccTotalAmount,
                                                           bool writeAmounts,
                                                           const QList<Attribute *> &attrsToPrint,
                                                           QTextCursor *cursor,
                                                           QTextTable *table,
                                                           QTextBlockFormat &tagBlockFormat,
                                                           QTextBlockFormat &txtBlockFormat,
                                                           QTextBlockFormat &numBlockFormat,
                                                           QTextTableCellFormat &leftFormat,
                                                           QTextTableCellFormat &centralFormat,
                                                           QTextTableCellFormat &rightFormat,
                                                           QTextTableCellFormat &centralQuantityTotalFormat,
                                                           QTextTableCellFormat &rightQuantityTotalFormat,
                                                           QTextCharFormat &txtCharFormat,
                                                           QTextCharFormat &txtBoldCharFormat) {
    if( !hasChildren() ){
        bool unionOk = false;
        QList<Attribute *>::const_iterator i = attrsToPrint.begin();
        while( !unionOk && i != attrsToPrint.end() ){
            if( containsAttribute( *i) ){
                unionOk = true;
            }
            ++i;
        }
        if( unionOk ){
            *itemProjTotalAmount += projAmount();
            *itemAccTotalAmount += accAmount();
            writeODTBillLine( prItemsOption,
                              false, writeAmounts,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineUnion( prItemsOption,
                                                  itemProjTotalAmount, itemAccTotalAmount,
                                                  writeAmounts, attrsToPrint,
                                                  cursor, table,
                                                  tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                  leftFormat, centralFormat, rightFormat,
                                                  centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                  txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingLSBillItem::writeODTAttributeBillLineIntersection( AccountingPrinter::PrintPPUDescOption prItemsOption,
                                                                  double * itemProjTotalAmount, double * itemAccTotalAmount,
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
                                                                  QTextCharFormat &txtBoldCharFormat ){
    if( !hasChildren() ){
        bool intersectionOk = true;
        QList<Attribute *>::const_iterator i = attrsToPrint.begin();
        while( intersectionOk && i != attrsToPrint.end() ){
            if( !containsAttribute( *i) ){
                intersectionOk = false;
            }
            ++i;
        }
        if( intersectionOk ){
            *itemProjTotalAmount += projAmount();
            *itemAccTotalAmount += accAmount();
            writeODTBillLine( prItemsOption,
                              false, writeAmounts,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingLSBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineIntersection( prItemsOption,
                                                         itemProjTotalAmount, itemAccTotalAmount,
                                                         attrsToPrint, writeAmounts,
                                                         cursor, table,
                                                         tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                         leftFormat, centralFormat, rightFormat,
                                                         centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                         txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingLSBillItem::writeODTBillLine( QTextCursor *cursor,
                                             QTextTable *table,
                                             const QDate &dateBegin, const QDate &dateEnd,
                                             AccountingPrinter::PrintLSOption prLSOption,
                                             AccountingPrinter::PrintPPUDescOption prPPUOption,
                                             bool writeAmounts,
                                             bool writeProgCode,
                                             QTextBlockFormat &tagBlockFormat,
                                             QTextBlockFormat & txtBlockFormat,
                                             QTextBlockFormat & numBlockFormat,
                                             QTextTableCellFormat & leftFormat,
                                             QTextTableCellFormat & centralFormat,
                                             QTextTableCellFormat & rightFormat,
                                             QTextTableCellFormat & centralQuantityTotalFormat,
                                             QTextTableCellFormat & rightQuantityTotalFormat,
                                             QTextCharFormat & txtCharFormat,
                                             QTextCharFormat & txtBoldCharFormat,
                                             QTextCharFormat & txtFormulaCharFormat ) {
    // se abbiamo invicato questo metodo non ci sono sottoarticoli
    if( prLSOption == AccountingPrinter::PrintLSProj ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        // [numero progressivo] + codice + descrizione + unità di misura + quantità [+ prezzo + importo ]
        if( writeProgCode ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progressiveCode()  );
        }
        if( m_d->priceItem ){
            if( writeProgCode ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->codeFull() );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, m_d->priceItem->codeFull() );
            }
            m_d->writeDescriptionCell( cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prPPUOption );
        } else {
            if( writeProgCode ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            }
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }
        // celle vuote
        // tag unita misura
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
        if( writeAmounts ){
            // quantita'
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            // prezzo
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            // importo
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
        } else {
            // quantita'
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
        }

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        if( writeProgCode ){
            // numero progressivo
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
            // codice
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        } else {
            // codice
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
        }

        // tag unita di misura
        QString unitMeasureTag;
        if( m_d->priceItem ){
            if( m_d->priceItem->unitMeasure()){
                unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
            }
        }

        for( int i=0; i < m_d->measuresModel->measuresCount(); ++i ){
            AccountingLSItemMeasure * measure = m_d->measuresModel->measure(i);

            // formula senza spazi bianchi
            QString realFormula;
            if( measure != NULL ){
                realFormula = measure->projFormula();
                realFormula.remove(" ");
            }

            // misure
            cursor->movePosition(QTextCursor::NextCell);
            table->cellAt( *cursor ).setFormat( centralFormat );
            cursor->setBlockFormat( txtBlockFormat );
            if( !(measure->comment().isEmpty() ) ){
                cursor->insertText( measure->comment() );
            }
            if( !(realFormula.isEmpty()) ){
                cursor->insertBlock();
                cursor->setCharFormat( txtFormulaCharFormat );
                cursor->insertText( measure->projFormula() );

                // unita di misura
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                // quantità
                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->accQuantityStr() );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, measure->accQuantityStr() );
                }
            } else {
                // celle vuote
                // unita di misura
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                // quantità
                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                }
            }

            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
            }

            // inserisce riga
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            if( writeProgCode ){
                // numero progressivo
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                // codice
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            }
        }

        // descrizione breve
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

        // tag unita di misura
        AccountingLSBillItemPrivate::writeCell( cursor, table,  centralFormat, tagBlockFormat, unitMeasureTag );

        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, projQuantityStr() );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUStr() );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, projAmountStr() );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightQuantityTotalFormat, numBlockFormat, projQuantityStr() );
        }
    } else if( prLSOption == AccountingPrinter::PrintLSAcc ){
        if( accQuantity(dateBegin, dateEnd) != 0.0 ){
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            // [numero progressivo] + codice + descrizione + unità di misura + data cont. + quantità cont. [+ prezzo + importo cont.]
            if( writeProgCode ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progressiveCode()  );
            }
            if( m_d->priceItem ){
                if( writeProgCode ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->codeFull() );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, m_d->priceItem->codeFull() );
                }
                m_d->writeDescriptionCell( cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prPPUOption );
            } else {
                if( writeProgCode ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                }
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            }
            // celle vuote
            // tag unita misura
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            // data
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            if( writeAmounts ){
                // quantita'
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                // prezzo
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                // importo
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
            } else {
                // quantita'
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            if( writeProgCode ){
                // numero progressivo
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                // codice
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            } else {
                // codice
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            }

            // tag unita di misura
            QString unitMeasureTag;
            if( m_d->priceItem ){
                if( m_d->priceItem->unitMeasure()){
                    unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                }
            }

            for( int i=0; i < m_d->measuresModel->measuresCount(); ++i ){
                AccountingLSItemMeasure * measure = m_d->measuresModel->measure(i);

                if( measure != NULL ){
                    if( measure->accDate() >= dateBegin && measure->accDate() <= dateEnd ){
                        // formula senza spazi bianchi
                        QString realFormula = measure->accFormula();
                        realFormula.remove(" ");

                        // commento + misure
                        cursor->movePosition(QTextCursor::NextCell);
                        table->cellAt( *cursor ).setFormat( centralFormat );
                        cursor->setBlockFormat( txtBlockFormat );
                        // commento
                        if( !(measure->comment().isEmpty()) ){
                            cursor->insertText( measure->comment() );
                        }
                        if( !(realFormula.isEmpty()) ){
                            // formula
                            cursor->insertBlock();
                            cursor->setCharFormat( txtFormulaCharFormat );
                            cursor->insertText( measure->accFormula() );

                            // unita di misura
                            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );
                            // data
                            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->accDateStr() );
                            // quantità
                            if( writeAmounts ){
                                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->accQuantityStr() );
                            } else {
                                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, measure->accQuantityStr() );
                            }
                        } else {
                            // celle vuote
                            // unita di misura
                            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                            // data
                            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                            // quantità
                            if( writeAmounts ){
                                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                            } else {
                                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                            }
                        }

                        if( writeAmounts ){
                            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
                        }

                        // inserisce riga
                        table->appendRows(1);
                        cursor->movePosition(QTextCursor::PreviousRow );

                        if( writeProgCode ){
                            // numero progressivo
                            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                            // codice
                            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                        } else {
                            // codice
                            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                        }
                    }
                }
            }

            // descrizione breve
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            // tag unita di misura
            AccountingLSBillItemPrivate::writeCell( cursor, table,  centralFormat, tagBlockFormat, unitMeasureTag );
            // data
            AccountingLSBillItemPrivate::writeCell( cursor, table,  centralFormat, numBlockFormat );
            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, accQuantityStr( dateBegin, dateEnd ) );
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUStr() );
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, accAmountStr(dateBegin, dateEnd) );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightQuantityTotalFormat, numBlockFormat, accQuantityStr( dateBegin, dateEnd ) );
            }
        }
    } else if( prLSOption == AccountingPrinter::PrintLSProjAcc ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        // numero progressivo + codice + descrizione + unità di misura [ + prezzo ] +
        // quantita' prog [+importo prog.] + data cont. + quantità cont. [+ importo cont.]
        if( writeProgCode ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progressiveCode()  );
        }
        if( m_d->priceItem ){
            if( writeProgCode ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->codeFull() );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, m_d->priceItem->codeFull() );
            }
            m_d->writeDescriptionCell( cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prPPUOption );
        } else {
            if( writeProgCode ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            }
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }

        // tag unita misura
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
        // prezzo
        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
        }
        // quantita' prog
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
        // importo prog.
        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat  );
        }
        // data cont.
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
        // quantita' cont.
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
        // importo cont.
        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
        }

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        if( writeProgCode ){
            // numero progressivo
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
            // codice
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        } else {
            // codice
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
        }

        // tag unita di misura
        QString unitMeasureTag;
        if( m_d->priceItem ){
            if( m_d->priceItem->unitMeasure()){
                unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
            }
        }

        for( int i=0; i < m_d->measuresModel->measuresCount(); ++i ){
            AccountingLSItemMeasure * measure = m_d->measuresModel->measure(i);

            if( measure != NULL ){
                // formula senza spazi bianchi
                QString projFormula = measure->projFormula();
                projFormula.remove(" ");
                QString accFormula = measure->accFormula();
                accFormula.remove(" ");

                // misure
                if( accFormula.isEmpty() && projFormula.isEmpty() ){
                    // indicazione lavorazioni
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() );
                    // unita di misura
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                    // prezzo
                    if( writeAmounts ){
                        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    }
                    // quantità prog
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    // importo prog.
                    if( writeAmounts ){
                        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    }
                    // data
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    if( writeAmounts ){
                        // quantità acc
                        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                        // importo acc
                        AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
                    } else {
                        // quantità acc
                        AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                    }
                } else {
                    // indicazione lavorazioni
                    cursor->movePosition(QTextCursor::NextCell);
                    table->cellAt( *cursor ).setFormat( centralFormat );
                    cursor->setBlockFormat( txtBlockFormat );
                    if( !(measure->comment().isEmpty()) &&
                            ( ((measure->accDate() >= dateBegin) && (measure->accDate() <= dateEnd)) &&
                              (!(projFormula.isEmpty()) || !(accFormula.isEmpty())) ) ){
                        cursor->insertText( measure->comment() );
                    }
                    cursor->insertBlock();
                    cursor->setCharFormat( txtFormulaCharFormat );
                    if( !(projFormula.isEmpty()) ){
                        cursor->insertText( "(1) " + measure->projFormula() );
                    }
                    if( (measure->accDate() >= dateBegin) && (measure->accDate() <= dateEnd) && !(accFormula.isEmpty()) ){
                        if( !(projFormula.isEmpty()) ){
                            cursor->insertText( "\n" );
                        }
                        cursor->insertText( "(2) " + measure->accFormula());
                    }

                    // unita di misura
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );
                    // prezzo
                    if( writeAmounts ){
                        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    }
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->projQuantityStr() );
                    // importo prog.
                    if( writeAmounts ){
                        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    }
                    if( (measure->accDate() >= dateBegin) && (measure->accDate() <= dateEnd) && !(measure->accFormula().isEmpty()) ){
                        // data
                        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->accDateStr() );
                        // quantità acc
                        if( writeAmounts ){
                            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->accQuantityStr()  );
                            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
                        } else {
                            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, measure->accQuantityStr() );
                        }
                    } else {
                        // data
                        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                        if( writeAmounts ){
                            // quantità acc
                            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                            // importo acc
                            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
                        } else {
                            // quantità acc
                            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                        }
                    }
                }

                // inserisce riga
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                if( writeProgCode ){
                    // numero progressivo
                    AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                    // codice
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                }
            }
        }

        // descrizione breve
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

        // tag unita di misura
        AccountingLSBillItemPrivate::writeCell( cursor, table,  centralFormat, tagBlockFormat, unitMeasureTag );

        if( writeAmounts ){
            // prezzo
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUStr()  );
            // quantita' progetto
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, projQuantityStr() );
            // importo progetto
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, projAmountStr() );
            // data
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat  );
            // quantita' contabile
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, accQuantityStr( dateBegin, dateEnd ) );
            // importo contabile
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, accAmountStr(dateBegin, dateEnd) );
        } else {
            // quantita' di progetto
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, projQuantityStr() );
            // data
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat  );
            // quantita' cont.
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightQuantityTotalFormat, numBlockFormat, accQuantityStr( dateBegin, dateEnd ) );
        }
    }
}

void AccountingLSBillItem::writeODTBillLine( AccountingPrinter::PrintPPUDescOption prItemsOption,
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
                                             QTextCharFormat & txtBoldCharFormat) {

    // Numero di colonne contenute
    int colCount = 0;
    if( writeProgCode ){
        colCount = 5;
    } else {
        colCount = 4;
    }
    if( writeAmounts ){
        colCount += 2;
    }

    // non ci sono sottoarticoli
    table->appendRows(1);
    cursor->movePosition(QTextCursor::PreviousRow );

    if( writeProgCode ){
        AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progressiveCode()  );
    }

    if( m_d->priceItem ){
        if( writeProgCode ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->codeFull() );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, m_d->priceItem->codeFull() );
        }
        m_d->writeDescriptionCell( cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prItemsOption );
    } else {
        if( writeProgCode ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
        }
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
    }

    if( m_d->measuresModel != NULL ){
        // celle vuote
        // tag unita misura
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
        if( writeAmounts ){
            // quantita'
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
        } else {
            // quantita'
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
        }

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        if( writeProgCode ){
            // numero progressivo
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
            // codice
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        } else {
            // codice
            AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
        }

        // tag unita di misura
        QString unitMeasureTag;
        if( m_d->priceItem ){
            if( m_d->priceItem->unitMeasure()){
                unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
            }
        }

        for( int i=0; i < m_d->measuresModel->measuresCount(); ++i ){
            AccountingLSItemMeasure * measure = m_d->measuresModel->measure(i);

            // formula senza spazi bianchi
            QString realFormula;
            if( measure != NULL ){
                realFormula = measure->accFormula();
                realFormula.remove(" ");
            }

            // misure
            if( measure != NULL ){
                if( realFormula.isEmpty() ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() + " (" + measure->accFormula() + ")");
                }
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat);
            }

            if( realFormula.isEmpty() || measure == NULL ){
                // unita di misura
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

                // quantità
                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                }
            } else {
                // unita di misura
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                // quantità
                if( writeAmounts ){
                    AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->accQuantityStr() );
                } else {
                    AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, measure->accQuantityStr() );
                }
            }

            if( writeAmounts ){
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
            }

            // inserisce riga
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            if( writeProgCode ){
                // numero progressivo
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                // codice
                AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            } else {
                AccountingLSBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            }
        }

        // descrizione breve
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

        // tag unita di misura
        AccountingLSBillItemPrivate::writeCell( cursor, table,  centralFormat, tagBlockFormat, unitMeasureTag );

        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUStr() );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, accQuantityStr() );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, accAmountStr() );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightQuantityTotalFormat, numBlockFormat, accQuantityStr() );
        }
    } else { // m_d->linesModel == NULL
        QString unitMeasureTag;
        if( m_d->priceItem ){
            if( m_d->priceItem->unitMeasure()){
                unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
            }
        }
        AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

        if( writeAmounts ){
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUStr() );
            AccountingLSBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, accQuantityStr() );
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, accAmountStr()  );
        } else {
            AccountingLSBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, accQuantityStr() );
        }
    }
    if( m_d->parentItem->m_d->parentItem == NULL ){
        AccountingLSBillItemPrivate::insertEmptyRow( colCount, cursor, leftFormat, centralFormat, rightFormat );
    }
}
