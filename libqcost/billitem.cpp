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

#include "billitem.h"

#include "billprinter.h"
#include "pricelist.h"
#include "priceitem.h"
#include "measuresmodel.h"
#include "measure.h"
#include "attributesmodel.h"
#include "attribute.h"
#include "pricefieldmodel.h"
#include "varsmodel.h"
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

#include <cmath>

class BillItemPrivate{
public:
    BillItemPrivate( BillItem * parent, PriceFieldModel * pfm, MathParser * p = NULL, VarsModel *vModel = NULL ):
        parentItem(parent),
        measuresModel( NULL ),
        parser(p),
        priceFieldModel(pfm),
        varsModel( vModel ),
        name(QObject::trUtf8("Titolo")),
        priceItem( NULL ),
        currentPriceDataSet(0),
        quantity(0.0),
        colCount(firstPriceFieldCol+2*pfm->fieldCount()){
        if( parent != NULL ){
            id = 1;
        } else {
            id = 0;
        }
        for( int i=0; i < priceFieldModel->fieldCount(); ++i ){
            amount << 0.0;
        }
    }
    ~BillItemPrivate(){
        for( QList<BillItem *>::iterator i = childrenContainer.begin(); i != childrenContainer.end(); ++i ){
            delete *i;
        }
        if( measuresModel ){
            delete measuresModel;
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
                               BillPrinter::PrintBillItemsOption prItemsOption ){
        if( priceItemToPrint != NULL ){
            if( prItemsOption == BillPrinter::PrintShortDesc ){
                writeCell( cursor, table, centralFormat, txtBlockFormat, priceItemToPrint->shortDescriptionFull() );
            } else if( prItemsOption == BillPrinter::PrintLongDesc ){
                writeCell( cursor, table, centralFormat, txtBlockFormat, priceItemToPrint->longDescriptionFull() );
            } else if( prItemsOption == BillPrinter::PrintShortLongDesc ){
                QList< QPair<QString, QTextCharFormat> > txt;
                txt << qMakePair( priceItemToPrint->shortDescriptionFull(), txtBoldCharFormat );
                txt << qMakePair( "\n" + priceItemToPrint->longDescriptionFull(), txtCharFormat );
                writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
            } else if( prItemsOption == BillPrinter::PrintShortLongDescOpt ){
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
                               BillPrinter::PrintBillItemsOption prItemsOption ){
        writeDescriptionCell( priceItem, cursor, table, centralFormat,
                              txtBlockFormat, txtCharFormat, txtBoldCharFormat,
                              prItemsOption );
    }

    BillItem * parentItem;
    QList<BillItem *> childrenContainer;
    QList<Attribute *> attributes;
    MeasuresModel * measuresModel;
    MathParser * parser;
    PriceFieldModel * priceFieldModel;
    VarsModel * varsModel;

    // se l'oggetto ha figli, l'oggetto diventa un titolo: il titolo è contenuto nell'attributo name
    QString name;
    PriceItem * priceItem;
    // colonna dell'elenco prezzi attiva
    int currentPriceDataSet;
    // quantita'
    double quantity;
    // importo dei campi
    QList<double> amount;

    // indici delle colonne
    static int progNumberCol;
    static int priceCodeCol;
    static int priceShortDescCol;
    static int priceUmCol;
    static int quantityCol;
    static int firstPriceFieldCol;
    int colCount;

    unsigned int id;

    QXmlStreamAttributes tmpAttributes;
};

int BillItemPrivate::progNumberCol = 0;
int BillItemPrivate::priceCodeCol = 1;
int BillItemPrivate::priceShortDescCol = 2;
int BillItemPrivate::priceUmCol = 3;
int BillItemPrivate::quantityCol = 4;
int BillItemPrivate::firstPriceFieldCol = 5;

BillItem::BillItem( PriceItem * p, BillItem *parentItem, PriceFieldModel * pfm, MathParser * parser, VarsModel * vModel ):
    TreeItem(),
    m_d( new BillItemPrivate(parentItem, pfm, parser, vModel ) ){
    setPriceItem(p);
    connect( this, &BillItem::currentPriceDataSetChanged, this, &BillItem::emitPriceDataUpdated );
    connect( this, &BillItem::currentPriceDataSetChanged, this, &BillItem::updateAmounts );

    if( parentItem != NULL ){
        connect( parentItem, &BillItem::attributesChanged, this, &BillItem::attributesChanged );
    }

    connect( this, static_cast<void(BillItem::*)( BillItem *, QList<int> )>(&BillItem::hasChildrenChanged), this, &BillItem::itemChanged );
    connect( this, &BillItem::nameChanged, this, &BillItem::itemChanged );
    connect( this, &BillItem::priceItemChanged, this, &BillItem::itemChanged );
    connect( this, &BillItem::currentPriceDataSetChanged, this, &BillItem::itemChanged );
    connect( this, &BillItem::quantityChanged, this, &BillItem::itemChanged );
    connect( this, static_cast<void(BillItem::*)(int, const QString &)>(&BillItem::amountChanged), this, &BillItem::itemChanged );
    connect( this, &BillItem::attributesChanged, this, &BillItem::itemChanged );
}

BillItem::~BillItem(){
    emit aboutToBeDeleted();
    delete m_d;
}

BillItem &BillItem::operator=(const BillItem &cp) {
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
        setQuantity( cp.m_d->quantity );
        setName( cp.m_d->name );

        if( cp.m_d->measuresModel != NULL ){
            generateMeasuresModel();
            *(m_d->measuresModel) = *(cp.m_d->measuresModel);
        }

        if( rootItem() == cp.rootItem() ){
            m_d->attributes.clear();
            m_d->attributes = cp.m_d->attributes;
        } else {
            m_d->attributes.clear();
        }
    }

    return *this;
}

int BillItem::firstPriceFieldCol(){
    return m_d->firstPriceFieldCol;
}

QString BillItem::name(){
    return m_d->name;
}

void BillItem::setName( const QString & newName ){
    if( m_d->name != newName ){
        m_d->name = newName;
        emit nameChanged( newName );
        emit dataChanged( this, m_d->priceShortDescCol );
    }
}

TreeItem *BillItem::parentInternal() {
    return m_d->parentItem;
}

BillItem *BillItem::parent() {
    return m_d->parentItem;
}

void BillItem::setParent(BillItem * newParent, int position ) {
    if( m_d->parentItem != newParent ){
        if( m_d->parentItem != NULL ){
            m_d->parentItem->removeChild( childNumber() );
            disconnect( m_d->parentItem, &BillItem::attributesChanged, this, &BillItem::attributesChanged );
        }
        m_d->parentItem = newParent;
        if( newParent != NULL ){
            newParent->addChild( this, position);
            connect( m_d->parentItem, &BillItem::attributesChanged, this, &BillItem::attributesChanged );
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

void BillItem::addChild(BillItem * newChild, int position ) {
    if( m_d->priceItem != NULL ){
        m_d->priceItem = NULL;
    }
    m_d->childrenContainer.insert( position, newChild );
}

void BillItem::removeChild( int position ) {
    m_d->childrenContainer.removeAt( position );
}

BillItem *BillItem::itemFromId( unsigned int itemId ) {
    if( itemId == m_d->id ){
        return this;
    } else {
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            BillItem * childItems = (*i)->itemFromId(itemId);
            if( childItems != NULL ) {
                return childItems;
            }
        }
    }
    return NULL;
}

BillItem *BillItem::findItemFromId( unsigned int itemId ) {
    if( m_d->parentItem == NULL ){
        return itemFromId(itemId );
    } else {
        return m_d->parentItem->findItemFromId(itemId);
    }
    return NULL;
}

BillItem *BillItem::itemFromProgCode( const QString & pCode) {
    if( pCode == progCode() ){
        return this;
    } else {
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            BillItem * childItems = (*i)->itemFromProgCode(pCode);
            if( childItems != NULL ) {
                return childItems;
            }
        }
    }
    return NULL;
}

BillItem *BillItem::findItemFromProgCode( const QString & pCode ) {
    if( m_d->parentItem == NULL ){
        return itemFromProgCode(pCode);
    } else {
        return m_d->parentItem->findItemFromProgCode(pCode);
    }
    return NULL;
}

BillItem *BillItem::childItem(int number) {
    return dynamic_cast<BillItem *>(child( number ));
}

void BillItem::setId( unsigned int ii ) {
    m_d->id = ii;
}

VarsModel *BillItem::varsModel() {
    if( m_d->parentItem == NULL ){
        return m_d->varsModel;
    } else {
        return m_d->parentItem->varsModel();
    }
}

const BillItem *BillItem::rootItem() const {
    if( m_d->parentItem == NULL ){
        return this;
    } else {
        return m_d->parentItem->rootItem();
    }
}

unsigned int BillItem::id() {
    return m_d->id;
}

QString BillItem::progCode() const {
    if( m_d->parentItem == NULL ){
        return QString();
    } else {
        if( m_d->parentItem->progCode().isEmpty() ){
            return QString::number( childNumber()+1 );
        } else {
            return m_d->parentItem->progCode() + "." + QString::number( childNumber()+1 );
        }
    }
}

PriceItem *BillItem::priceItem() {
    return m_d->priceItem;
}

double BillItem::quantity() const {
    if( m_d->childrenContainer.size() > 0 ){
        return 0.0;
    } else {
        return m_d->quantity;
    }
}

QString BillItem::quantityStr() const {
    int prec = 2;
    if( m_d->priceItem != NULL ){
        if( m_d->priceItem->unitMeasure() != NULL ){
            prec = m_d->priceItem->unitMeasure()->precision();
        }
    }
    return m_d->toString( quantity(), 'f', prec );
}

double BillItem::amount( int field ) const{
    if( (field > -1) && (field < m_d->amount.size() ) ){
        return m_d->amount.at(field);
    }
    return 0.0;
}

QString BillItem::amountStr(int field) const {
    return m_d->toString( amount(field), 'f', m_d->priceFieldModel->precision(field) );
}

void BillItem::setPriceItem(PriceItem * p) {
    if( m_d->priceItem != p ){
        if( m_d->priceItem != NULL ) {
            disconnect( m_d->priceItem, &PriceItem::codeFullChanged, this, &BillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &BillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &BillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::valueChanged, this, &BillItem::emitPriceDataUpdated );
        }
        PriceItem * oldPriceItem = m_d->priceItem;

        m_d->priceItem = p;

        emit priceItemChanged( oldPriceItem, p );

        if( m_d->priceItem != NULL ){
            connect( m_d->priceItem, &PriceItem::codeFullChanged, this, &BillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &BillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &BillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::valueChanged, this, &BillItem::emitPriceDataUpdated );
        }

        emit dataChanged( this, m_d->priceCodeCol );
        emit dataChanged( this, m_d->priceUmCol );
        for( int i=0; i < m_d->priceFieldModel->fieldCount(); ++i ){
            emit dataChanged( this, m_d->firstPriceFieldCol + 2 * i );
        }

        if( m_d->measuresModel != NULL ){
            m_d->measuresModel->setUnitMeasure( m_d->priceItem->unitMeasure() );
        }

        updateAmounts();
    }
}

void BillItem::setQuantity(double v) {
    if( m_d->measuresModel == NULL ){
        setQuantityPrivate( v );
    }
}

void BillItem::setQuantityPrivate(double v) {
    if( v != m_d->quantity ){
        m_d->quantity = v;
        emit quantityChanged( quantityStr() );
        emit dataChanged( this, m_d->quantityCol );
        updateAmounts();
    }
}

void BillItem::setQuantity(const QString &vstr ) {
    setQuantity( m_d->parser->evaluate( vstr ) );
}

int BillItem::columnCount() const {
    return m_d->colCount;
}

QVariant BillItem::data(int col, int role) const {
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
                return QVariant( progCode() );
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
    } else if( col == m_d->quantityCol ){
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            if( m_d->parentItem == NULL ){
                return QVariant( trUtf8("Quantità") );
            }
            if( hasChildren() ){
                return QVariant();
            } else {
                return QVariant( quantityStr() );
            }
        }
    } else {
        if( role == Qt::TextAlignmentRole ){
            if( m_d->parentItem == NULL ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else {
                return Qt::AlignRight + Qt::AlignVCenter;
            }
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            int pf = (col-m_d->firstPriceFieldCol) / 2;
            if( ((col - m_d->firstPriceFieldCol) % 2) == 0 ){
                // colonna prezzo
                if( m_d->parentItem == NULL ){
                    if( pf < m_d->priceFieldModel->fieldCount() ){
                        return QVariant( m_d->priceFieldModel->priceName(pf) );
                    }
                } else {
                    if( hasChildren() ){
                        return QVariant();
                    } else if( pf < m_d->priceFieldModel->fieldCount() ){
                        if( m_d->priceItem != NULL ){
                            return QVariant( m_d->priceItem->valueStr(pf, m_d->currentPriceDataSet ) );
                        } else {
                            return QVariant( 0.0 );
                        }
                    }
                }
            } else {
                // colonna importo
                if( m_d->parentItem == NULL ){
                    if( pf < m_d->priceFieldModel->fieldCount() ){
                        return QVariant( m_d->priceFieldModel->amountName(pf) );
                    }
                } else {
                    if( pf < m_d->amount.size() ){
                        return QVariant( amountStr(pf) );
                    }
                }
            }
        }
    }
    return QVariant();
}

bool BillItem::setData(int column, const QVariant &value) {
    if( hasChildren() ){
        if( column == m_d->priceShortDescCol ){
            m_d->name = value.toString();
            return true;
        }
    } else {
        if( column == m_d->quantityCol ){
            setQuantity( value.toString() );
            for( int i=0; i < m_d->priceFieldModel->fieldCount(); ++i ){
                emit dataChanged( this, m_d->firstPriceFieldCol + 1 + (i * 2) );
            }
            return true;
        }
    }
    return false;
}

int BillItem::currentPriceDataSet() const{
    if( m_d->parentItem ){
        return m_d->parentItem->currentPriceDataSet();
    } else {
        return m_d->currentPriceDataSet;
    }
}

void BillItem::setCurrentPriceDataSet(int v ) {
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
}

void BillItem::emitPriceDataUpdated() {
    for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->emitPriceDataUpdated();
    }
    if( !hasChildren() ){
        updateAmounts();
        emit dataChanged( this, m_d->priceCodeCol );
        emit dataChanged( this, m_d->priceUmCol );
        for( int i=0; i < m_d->priceFieldModel->fieldCount(); ++i){
            emit dataChanged( this, m_d->firstPriceFieldCol + 2 * i );
        }
    }
}

void BillItem::setUnitMeasure(UnitMeasure *ump) {
    if( m_d->measuresModel != NULL ){
        m_d->measuresModel->setUnitMeasure( ump );
    }
}

void BillItem::updateAmount( int pf ) {
    if( (pf < m_d->amount.size()) && (pf > -1) ){
        double v = 0.0;
        if( hasChildren() ){
            for( QList<BillItem*>::iterator iter = m_d->childrenContainer.begin(); iter != m_d->childrenContainer.end(); ++iter ){
                (*iter)->updateAmount(pf);
                v += (*iter)->amount(pf);
            }
        } else if( m_d->priceItem != NULL ){
            v = UnitMeasure::applyPrecision( m_d->quantity * m_d->priceItem->value( pf, m_d->currentPriceDataSet ), m_d->priceFieldModel->precision(pf) );
        }
        if( v != m_d->amount.at(pf) ){
            m_d->amount[pf] = v;
            emit amountChanged( pf, m_d->toString( v, 'f', 2 ) );
            emit amountChanged( pf, v );
            emit dataChanged( this, m_d->firstPriceFieldCol + (2*pf+1)  );
        }
    }
}

void BillItem::updateAmounts() {
    for( int i=0; i < m_d->amount.size(); ++i){
        updateAmount(i);
    }
}

bool BillItem::isUsingPriceItem(PriceItem *p) {
    if( hasChildren() ){
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
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

QList<PriceItem *> BillItem::usedPriceItems() const {
    QList<PriceItem *> ret;
    appendUsedPriceItems( &ret );
    return ret;
}

void BillItem::appendUsedPriceItems( QList<PriceItem *> * usedPriceItems ) const {
    if( m_d->childrenContainer.size() == 0 ){
        if( !usedPriceItems->contains( m_d->priceItem )){
            usedPriceItems->append( m_d->priceItem );
        }
    } else {
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->appendUsedPriceItems( usedPriceItems );
        }
    }
}

void BillItem::setHasChildrenChanged(BillItem *p, QList<int> indexes) {
    if( m_d->parentItem ){
        // non è l'oggetto root - rimandiamo all'oggetto root
        m_d->parentItem->setHasChildrenChanged( p, indexes );
    } else {
        // è l'oggetto root - emette il segnale di numero figli cambiato
        emit hasChildrenChanged( p, indexes );
    }
}

TreeItem *BillItem::child(int number) {
    if( number >= 0 && number < m_d->childrenContainer.size() ){
        return m_d->childrenContainer[number];
    }
    return NULL;
}

int BillItem::childrenCount() const {
    return m_d->childrenContainer.size();
}

bool BillItem::hasChildren() const {
    return (m_d->childrenContainer.size() > 0);
}

QList<BillItem *> BillItem::allChildren() {
    QList<BillItem *> ret;
    for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        ret.append( *i );
        if( (*i)->hasChildren() ){
            ret.append( (*i)->allChildren() );
        }
    }
    return ret;
}

QList<BillItem *> BillItem::allChildrenWithMeasures() {
    QList<BillItem *> ret;
    for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        if( (*i)->hasChildren() ){
            ret.append( (*i)->allChildrenWithMeasures() );
        } else {
            ret.append( this );
        }
    }
    return ret;
}

bool BillItem::insertChildren(PriceItem * p, int position, int count ){
    if (position < 0 || position > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row) {
        BillItem *item = new BillItem( p, this, m_d->priceFieldModel, m_d->parser );
        while( findItemFromId( item->id() ) != NULL ){
            item->setId( item->id() + 1 );
        }
        m_d->childrenContainer.insert(position, item);
        connect( item, static_cast<void(BillItem::*)(BillItem*,int)> (&BillItem::dataChanged), this, static_cast<void(BillItem::*)(BillItem*,int)> (&BillItem::dataChanged) );
        connect( item, static_cast<void(BillItem::*)(int,double)>(&BillItem::amountChanged), this, &BillItem::updateAmounts );
        connect( this, &BillItem::currentPriceDataSetChanged, item, &BillItem::setCurrentPriceDataSet );
        connect( item, &BillItem::itemChanged, this, &BillItem::itemChanged );
    }

    if( !hadChildren ){
        if( m_d->childrenContainer.size() > 0 ){
            emit hasChildrenChanged( true );
        }
    }

    emit itemChanged();

    return true;
}

bool BillItem::insertChildren(int position, int count) {
    return insertChildren( NULL, position, count );
}

bool BillItem::appendChildren(int count) {
    return insertChildren( m_d->childrenContainer.size(), count );
}

bool BillItem::removeChildren(int position, int count) {
    if( count <= 0 ){
        return true;
    }

    if (position < 0 || position + count > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row){
        BillItem * item = m_d->childrenContainer.at( position );
        disconnect( item, static_cast<void(BillItem::*)(BillItem*,int)> (&BillItem::dataChanged), this, static_cast<void(BillItem::*)(BillItem*,int)> (&BillItem::dataChanged) );
        disconnect( item, static_cast<void(BillItem::*)(int,double)>(&BillItem::amountChanged), this, &BillItem::updateAmounts );
        disconnect( this, &BillItem::currentPriceDataSetChanged, item, &BillItem::setCurrentPriceDataSet );
        disconnect( item, &BillItem::itemChanged, this, &BillItem::itemChanged );
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

bool BillItem::reset() {
    return removeChildren( 0, m_d->childrenContainer.size() );
}

int BillItem::childNumber() const {
    if (m_d->parentItem)
        return m_d->parentItem->m_d->childrenContainer.indexOf( const_cast<BillItem *>(this) );

    return 0;
}

MeasuresModel *BillItem::measuresModel() {
    return m_d->measuresModel;
}

MeasuresModel *BillItem::generateMeasuresModel() {
    if( m_d->measuresModel == NULL ){
        UnitMeasure * ump = NULL;
        if( m_d->priceItem != NULL ){
            ump = m_d->priceItem->unitMeasure();
        }
        m_d->measuresModel = new MeasuresModel( this, m_d->parser, ump );
        setQuantity( m_d->measuresModel->quantity() );
        connect( m_d->measuresModel, &MeasuresModel::quantityChanged, this, &BillItem::setQuantityPrivate );
        connect( m_d->measuresModel, &MeasuresModel::modelChanged, this, &BillItem::itemChanged );
        emit itemChanged();
    }
    return m_d->measuresModel;
}

void BillItem::removeMeasuresModel() {
    if( m_d->measuresModel != NULL ){
        disconnect( m_d->measuresModel, &MeasuresModel::quantityChanged, this, &BillItem::setQuantityPrivate );
        disconnect( m_d->measuresModel, &MeasuresModel::modelChanged, this, &BillItem::itemChanged );
        delete m_d->measuresModel;
        m_d->measuresModel = NULL;
        updateAmounts();
    }
}

void BillItem::appendConnectedItems(QList<BillItem *> *itemsList) {
    // aggiunge l'item corrente e tutti i suoi figli, se non presenti
    if( !(itemsList->contains(this)) ){
        itemsList->append(this);
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->appendConnectedItems(itemsList);
        }
    }

    // aggiunge tutti gli item connessi tramite le misure, se non presenti
    QList<BillItem *> connItems = m_d->measuresModel->connectedBillItems();
    for( QList<BillItem *>::iterator i = connItems.begin(); i != connItems.end(); ++i ){
        if( !(itemsList->contains(*i)) ){
            (*i)->appendConnectedItems(itemsList);
        }
    }
}

Qt::ItemFlags BillItem::flags(int column) const {
    if( column == m_d->progNumberCol ){
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    if( hasChildren() ){
        if( column == m_d->priceShortDescCol ){
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        }
    } else {
        if( column == m_d->quantityCol ){
            if( m_d->measuresModel == NULL ){
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
            } else {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            }
        }
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void BillItem::writeXml10(QXmlStreamWriter *writer) const {
    if( m_d->parentItem != NULL ){
        // se non e' l'elemento root
        writer->writeStartElement( "BillItem" );
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
            for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeXml10( writer );
            }
        } else {
            if( m_d->priceItem != NULL ){
                writer->writeAttribute( "priceItem", QString::number( m_d->priceItem->id() ) );
            }
            writer->writeAttribute( "quantity", QString::number( m_d->quantity ) );
            writer->writeAttribute( "priceDataSet", QString::number(m_d->currentPriceDataSet) );

            if( m_d->measuresModel ){
                m_d->measuresModel->writeXml10( writer );
            }
        }

        writer->writeEndElement();
    } else {
        // e' l'elemento root
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml10( writer );
        }
    }
}

void BillItem::readXml10(QXmlStreamReader *reader, PriceList * priceList, AttributesModel *billAttrModel ) {
    if( m_d->parentItem != NULL ){
        if(reader->isStartElement() && reader->name().toString().toUpper() == "BILLITEM"){
            loadXml10( reader->attributes(), priceList, billAttrModel );
        }
        reader->readNext();
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLITEM")&&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILL")  ){
        if( reader->isStartElement() ){
            QString tagUp = reader->name().toString().toUpper();
            if( tagUp == "BILLITEM" ) {
                appendChildren();
                m_d->childrenContainer.last()->readXml10( reader, priceList, billAttrModel );
            }
            if( tagUp == "BILLITEMMEASURESMODEL" ) {
                generateMeasuresModel()->readXml10( reader );
            }
        }
        reader->readNext();
    }
}

void BillItem::readXmlTmp10(QXmlStreamReader *reader) {
    if( m_d->parentItem != NULL ){
        if(reader->isStartElement() && reader->name().toString().toUpper() == "BILLITEM"){
            loadFromXmlTmp10( reader->attributes() );
        }
        reader->readNext();
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLITEM")&&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILL")  ){
        if( reader->isStartElement() ){
            QString tagUp = reader->name().toString().toUpper();
            if( tagUp == "BILLITEM" ) {
                appendChildren();
                m_d->childrenContainer.last()->readXmlTmp10( reader );
            }
            if( tagUp == "BILLITEMMEASURESMODEL" ) {
                generateMeasuresModel()->readXml10( reader );
            }
        }
        reader->readNext();
    }
}

void BillItem::loadXml10(const QXmlStreamAttributes &attrs, PriceList * priceList, AttributesModel *billAttrModel) {
    for( QXmlStreamAttributes::const_iterator attrIter = attrs.begin(); attrIter != attrs.end(); ++attrIter ){
        QString nameUp = attrIter->name().toString().toUpper();
        if( nameUp == "ID" ){
            m_d->id = attrIter->value().toUInt();
        } else if( nameUp == "NAME" ){
            m_d->name = attrIter->value().toString();
        } else if( nameUp == "ATTRIBUTES" ){
            QStringList attributes = attrIter->value().toString().split(',');
            for( QStringList::iterator i = attributes.begin(); i != attributes.end(); ++i ){
                bool ok = false;
                unsigned int attrId = (*i).toUInt(&ok);
                if( ok ){
                    addAttribute( billAttrModel->attributeId( attrId ) );
                }
            }
        } else if( nameUp =="QUANTITY" ){
            QString qStr = attrIter->value().toString();
            setQuantity( qStr.toDouble() );
        } else if( nameUp == "PRICEDATASET"){
            setCurrentPriceDataSet( attrIter->value().toInt() );
        } else if( nameUp == "PRICEITEM" ){
            if( priceList ){
                setPriceItem( priceList->priceItemId( attrs.value( "priceItem").toUInt() ) );
            }
        }
    }
}

void BillItem::loadFromXmlTmp10(const QXmlStreamAttributes &attrs) {
    m_d->tmpAttributes.clear();
    m_d->tmpAttributes = attrs;
}

void BillItem::loadTmpData10( PriceList *priceList, AttributesModel * billAttrModel) {
    if( !m_d->tmpAttributes.isEmpty() ){
        loadXml10(m_d->tmpAttributes, priceList, billAttrModel );
        m_d->tmpAttributes.clear();
    }
    for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->loadTmpData10( priceList, billAttrModel );
    }
}

void BillItem::writeXml20(QXmlStreamWriter *writer) const {
    if( m_d->parentItem != NULL ){
        // se non e' l'elemento root
        writer->writeStartElement( "BillItem" );
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
            for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeXml20( writer );
            }
        } else {
            if( m_d->priceItem != NULL ){
                writer->writeAttribute( "priceItem", QString::number( m_d->priceItem->id() ) );
            }
            writer->writeAttribute( "quantity", QString::number( m_d->quantity ) );
            writer->writeAttribute( "priceDataSet", QString::number(m_d->currentPriceDataSet) );

            if( m_d->measuresModel ){
                m_d->measuresModel->writeXml20( writer );
            }
        }

        writer->writeEndElement();
    } else {
        // e' l'elemento root
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml20( writer );
        }
    }
}

void BillItem::readXmlTmp20(QXmlStreamReader *reader) {
    if( m_d->parentItem != NULL ){
        if(reader->isStartElement() && reader->name().toString().toUpper() == "BILLITEM"){
            m_d->tmpAttributes.clear();
            m_d->tmpAttributes = reader->attributes();
        }
        reader->readNext();
    }
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILLITEM")&&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "BILL")  ){
        if( reader->name().toString().toUpper() == "BILLITEM" && reader->isStartElement()) {
            appendChildren();
            m_d->childrenContainer.last()->readXmlTmp20( reader );
        }
        if( reader->name().toString().toUpper() == "MEASURESMODEL" && reader->isStartElement() ) {
            generateMeasuresModel()->readXmlTmp20( reader );
        }
        reader->readNext();
    }
}

void BillItem::loadXml20(const QXmlStreamAttributes &attrs, PriceList * priceList, AttributesModel * billAttrModel) {
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
    if( attrs.hasAttribute( "quantity" ) ){
        QString qStr = attrs.value("quantity").toString();
        setQuantity( qStr.toDouble() );
    }
    if( attrs.hasAttribute( "priceDataSet" ) ){
        setCurrentPriceDataSet( attrs.value( "priceDataSet").toInt() );
    }
    if( attrs.hasAttribute( "priceItem" ) ){
        if( priceList ){
            setPriceItem( priceList->priceItemId( attrs.value( "priceItem").toUInt() ) );
        }
    }
}

void BillItem::readFromXmlTmp20( PriceList *priceList, AttributesModel * billAttrModel) {
    if( !m_d->tmpAttributes.isEmpty() ){
        loadXml20(m_d->tmpAttributes, priceList, billAttrModel );
        m_d->tmpAttributes.clear();
        if( m_d->measuresModel != NULL ){
            m_d->measuresModel->readFromXmlTmp20();
        }
    }
    for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->readFromXmlTmp20( priceList, billAttrModel );
    }
}

QList< QPair<Attribute *, bool> > BillItem::attributes() {
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

QList<Attribute *> BillItem::allAttributes() {
    QList<Attribute *> attrs = inheritedAttributes();
    for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
        if( !attrs.contains( *i) ){
            attrs.append( *i );
        }
    }
    return attrs;
}

QList<Attribute *> BillItem::directAttributes() {
    return QList<Attribute *>( m_d->attributes );
}

void BillItem::addAttribute( Attribute * attr ){
    if( !(m_d->attributes.contains( attr )) ){
        m_d->attributes.append( attr );
        emit attributesChanged();
    }
}

void BillItem::removeAttribute( Attribute * attr ){
    if( m_d->attributes.removeAll( attr ) > 0 ){
        emit attributesChanged();
    }
}

void BillItem::removeAllAttributes() {
    m_d->attributes.clear();
    emit attributesChanged();
}

double BillItem::amountAttribute(Attribute *attr, int field ) {
    if( m_d->childrenContainer.size() == 0 ){
        QList<Attribute *> attrs = inheritedAttributes();
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( !attrs.contains( *i) ){
                attrs.append( *i );
            }
        }
        if( attrs.contains(attr) ){
            return amount( field );
        }
    } else {
        double ret = 0.0;
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountAttribute( attr, field );
        }
        return ret;
    }
    return 0.0;
}

QString BillItem::amountAttributeStr(Attribute *attr, int field ) {
    int prec = m_d->priceFieldModel->precision( field );
    QString ret;
    double v = amountAttribute( attr, field );
    if( m_d->parser == NULL ){
        ret = QString::number(v, 'f', prec );
    } else {
        ret = m_d->parser->toString( v, 'f', prec );
    }
    return ret;
}

QList<Attribute *> BillItem::inheritedAttributes(){
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

bool BillItem::containsAttribute(Attribute *attr) {
    if( containsAttributeDirect(attr)){
        return true;
    }
    return containsAttributeInherited( attr );
}

bool BillItem::containsAttributeInherited(Attribute *attr) {
    if( m_d->parentItem != NULL ){
        if( m_d->parentItem->containsAttributeDirect( attr ) ){
            return true;
        } else {
            return m_d->parentItem->containsAttributeInherited( attr );
        }
    }
    return false;
}

bool BillItem::containsAttributeDirect(Attribute *attr) {
    return m_d->attributes.contains( attr );
}

bool BillItem::isDescending(BillItem *ancestor) {
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

QList<PriceItem *> BillItem::connectedPriceItems() const {
    QList<PriceItem *> ret;
    for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        ret.append( (*i)->priceItem()->connectedPriceItems() );
    }
    return ret;
}

void BillItem::insertAmount(int pf){
    double v = 0.0;
    if( m_d->priceItem != NULL ){
        v = UnitMeasure::applyPrecision( m_d->quantity * m_d->priceItem->value( pf, m_d->currentPriceDataSet ), m_d->priceFieldModel->precision(pf) );
    }
    m_d->amount.insert(pf, v);
    for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->insertAmount( pf );
    }
    m_d->colCount += 2;
}

void BillItem::removeAmount(int pf){
    m_d->amount.removeAt( pf );
    for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->removeAmount( pf );
    }
    m_d->colCount -= 2;
}

#include "qtextformatuserdefined.h"

void BillItem::writeODTBillOnTable( QTextCursor *cursor,
                                    BillPrinter::PrintBillItemsOption prItemsOption,
                                    const QList<int> fieldsToPrint,
                                    bool groupPrAm  ) {
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
    int cellCount = 5 + 2 * fieldsToPrint.size();

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    if( m_d->parentItem == NULL ){
        // *** Riga di intestazione ***
        BillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N."), false);
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
        if( fieldsToPrint.size() == 0 ){
            BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        } else { // fieldsToPrint.size() > 0
            BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        }

        if( groupPrAm  ){
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
            }
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
                }
            }
        } else {
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
                    BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
                    BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
                }
            }
        }

        // *** Riga vuota ***
        BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        // *** Scrive il computo dei sottoarticoli ***
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeODTBillOnTable( cursor, prItemsOption, fieldsToPrint, groupPrAm );
        }

        // *** riga dei totali complessivi
        if( fieldsToPrint.size() > 0 ){
            // riga vuota
            BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            BillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale complessivo") );
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );

            if( groupPrAm ){
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                }
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    if( i == fieldsToPrint.size() - 1 ){
                        BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                    }
                }
            } else {
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    if( i == fieldsToPrint.size() - 1 ){
                        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                        BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                    }
                }
            }
        }

        // *** Riga di chiusura ***
        BillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );

    } else {
        if( hasChildren() ){
            // ci sono sottoarticoli
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            BillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat, progCode() );
            BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, m_d->name );
            BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            if( fieldsToPrint.size() == 0 ){
                BillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            } else { // fieldsToPrint.size() > 0
                BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            }
            if( groupPrAm ){
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                }
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    if( i == fieldsToPrint.size() - 1 ){
                        BillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    }
                }
            } else {
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    if( i == fieldsToPrint.size() - 1 ){
                        BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                        BillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                        BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    }
                }
            }

            for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTBillOnTable( cursor, prItemsOption, fieldsToPrint, groupPrAm );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            BillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale %1").arg( m_d->name ) );
            BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            if( fieldsToPrint.size() == 0 ){
                BillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            } else { // fieldsToPrint.size() > 0
                BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            }
            if( groupPrAm ){
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                }
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    if( i == fieldsToPrint.size() - 1 ){
                        BillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                    }
                }
            } else {
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    if( i == fieldsToPrint.size() - 1 ){
                        BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                        BillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                        BillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                    }
                }
            }
        } else { // !hasChildren()
            writeODTBillLine( prItemsOption,
                              true, fieldsToPrint, groupPrAm,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    }
}

void BillItem::writeODTSummaryOnTable( QTextCursor *cursor,
                                       BillPrinter::PrintBillItemsOption prItemsOption,
                                       const QList<int> fieldsToPrint,
                                       bool groupPrAm,
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
    int cellCount = 4 + 2 * fieldsToPrint.size();

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    // *** Intestazione tabella ***
    BillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("Codice"), false );
    BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Denominazione"));
    BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( fieldsToPrint.size() == 0 ){
        BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    } else { // fieldsToPrint.size() > 0
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    }
    if( groupPrAm  ){
        for( int i=0; i < fieldsToPrint.size(); ++i ){
            BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
        }
        for( int i=0; i < fieldsToPrint.size(); ++i ){
            if( i == fieldsToPrint.size() - 1 ){
                BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
            }
        }
    } else {
        for( int i=0; i < fieldsToPrint.size(); ++i ){
            if( i == fieldsToPrint.size() - 1 ){
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
                BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
            }
        }
    }

    // *** riga vuota ***
    BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    // *** Righe del sommario ***
    QList<PriceItem *> usedPItems = usedPriceItems();
    for( QList<PriceItem *>::iterator i = usedPItems.begin(); i != usedPItems.end(); ++i){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        if( (*i) != NULL ){
            BillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, (*i)->codeFull() );
            m_d->writeDescriptionCell( (*i), cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prItemsOption );
        } else {
            BillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
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
            BillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            if( fieldsToPrint.size() == 0 ){
                BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            } else { // fieldsToPrint.size() > 0
                BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            }
            for( int j = 0; j < fieldsToPrint.size() * 2; j++ ){
                if( j < (fieldsToPrint.size() * 2 -1) ){
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                } else {
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                }
            }
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
        }

        QList<double> fieldsAmount;
        for( int j=0; j < fieldsToPrint.size(); ++j ){
            fieldsAmount.append( 0.0 );
        }
        double itemTotalQuantity = 0.0;
        for( QList<BillItem *>::iterator j = m_d->childrenContainer.begin(); j != m_d->childrenContainer.end(); ++j ){
            (*j)->writeODTSummaryLine( *i, cursor, fieldsToPrint, &itemTotalQuantity, &fieldsAmount, writeDetails,
                                       table,
                                       tagBlockFormat, txtBlockFormat, numBlockFormat,
                                       leftFormat, centralFormat, rightFormat );
        }

        if( writeDetails ){
            BillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }
        BillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

        if( fieldsToPrint.size() == 0 ){
            BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
        } else { // fieldsToPrint.size() > 0
            BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
        }

        if( groupPrAm ){
            for( int j=0; j < fieldsToPrint.size(); ++j ){
                if( (*i) != NULL ){
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*i)->valueStr(j, currentPriceDataSet() ) );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                }
            }
            for( int j=0; j < fieldsToPrint.size(); ++j ){
                if( j == fieldsToPrint.size() - 1 ){
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( fieldsAmount.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( fieldsAmount.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
                }
            }
        } else {
            for( int j=0; j < fieldsToPrint.size(); ++j ){
                if( j == fieldsToPrint.size() - 1 ){
                    if( (*i)!= NULL ){
                        BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*i)->valueStr(j, currentPriceDataSet() ) );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    }
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( fieldsAmount.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
                } else {
                    if( (*i)!= NULL ){
                        BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*i)->valueStr(j, currentPriceDataSet() ) );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    }
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( fieldsAmount.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
                }
            }
        }
        BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
    }

    // *** Totale complessivo ***
    // *** riga dei totali complessivi
    if( fieldsToPrint.size() > 0 ){
        // riga vuota
        // BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        // codice
        BillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        // descrizione
        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale complessivo") );
        // Udm
        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        // quantita'
        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );

        // prezzo + importo
        if( groupPrAm ){
            for( int j=0; j < fieldsToPrint.size(); ++j ){
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            }
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(i)) );
                }
            }
        } else {
            for( int j=0; j < fieldsToPrint.size(); ++j ){
                if( j == fieldsToPrint.size() - 1 ){
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(j)) );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat, amountStr(fieldsToPrint.at(j)) );
                }
            }
        }
    }

    // *** Riga di chiusura ***
    BillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

void BillItem::writeODTSummaryLine(PriceItem * priceItem,
                                   QTextCursor *cursor,
                                   const QList<int> fieldsToPrint,
                                   double * itemTotalQuantity,
                                   QList<double> * fieldsValue,
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
            (*itemTotalQuantity) += quantity();
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                (*fieldsValue)[i] += amount( i );
            }
            if( writeDetails ){
                BillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, progCode() );

                QString unitMeasureTag;
                if( m_d->priceItem != NULL ){
                    if( m_d->priceItem->unitMeasure() != NULL ){
                        unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                    }
                }
                BillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                if( fieldsToPrint.size() == 0 ){
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
                } else { // fieldsToPrint.size() > 0
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                }

                for( int j=0; j < (2*fieldsToPrint.size()); ++j ){
                    if( j < (2*fieldsToPrint.size()-1) ){
                        BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    } else {
                        BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                    }
                }
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
            }
        }
    } else {
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTSummaryLine( priceItem, cursor, fieldsToPrint, itemTotalQuantity, fieldsValue, writeDetails,
                                       table,
                                       tagBlockFormat, txtBlockFormat, numBlockFormat,
                                       leftFormat, centralFormat, rightFormat );
        }
    }
}

void BillItem::writeODTAttributeBillOnTable(QTextCursor *cursor,
                                            BillPrinter::AttributePrintOption prOption, BillPrinter::PrintBillItemsOption prItemsOption,
                                            const QList<int> &fieldsToPrint,
                                            const QList<Attribute *> &attrsToPrint,
                                            bool groupPrAm) {
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
    int cellCount = 4 + 2 * fieldsToPrint.size();

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    // *** Intestazione tabella ***
    BillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("Codice"), false );
    BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Denominazione"));
    BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( fieldsToPrint.size() == 0 ){
        BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    } else { // fieldsToPrint.size() > 0
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    }
    if( groupPrAm  ){
        for( int i=0; i < fieldsToPrint.size(); ++i ){
            BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
        }
        for( int i=0; i < fieldsToPrint.size(); ++i ){
            if( i == fieldsToPrint.size() - 1 ){
                BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
            }
        }
    } else {
        for( int i=0; i < fieldsToPrint.size(); ++i ){
            if( i == fieldsToPrint.size() - 1 ){
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
                BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->amountName( fieldsToPrint.at(i)) );
            }
        }
    }

    // *** riga vuota ***
    BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    QList<double> fieldsAmounts;
    for( QList<int>::const_iterator i = fieldsToPrint.begin(); i != fieldsToPrint.end(); ++i ){
        fieldsAmounts.append( 0.0 );
    }

    if( prOption == BillPrinter::AttributePrintSimple ){
        // *** Righe del computo suddivise per attributo ***
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            for( QList<double>::iterator j = fieldsAmounts.begin(); j != fieldsAmounts.end(); ++j ){
                *j = 0.0;
            }
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            // cursor->movePosition(QTextCursor::NextCell);

            BillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, (*i)->name() );
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            if( fieldsToPrint.size() == 0 ){
                BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            } else { // fieldsToPrint.size() > 0
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            }
            for( int j=0; j < 2*fieldsToPrint.size(); ++j ){
                if( j == 2*fieldsToPrint.size() -1 ){
                    BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                }
            }
            // *** riga vuota ***
            BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            writeODTAttributeBillLineSimple( prItemsOption,
                                             &fieldsAmounts, fieldsToPrint, *i, groupPrAm,
                                             cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                             leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                             txtCharFormat, txtBoldCharFormat );

            BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            BillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale %1").arg((*i)->name()) );
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            if( fieldsToPrint.size() == 0 ){
                BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            } else { // fieldsToPrint.size() > 0
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            }
            for( int j=0; j < fieldsAmounts.size(); ++j ){
                if( j < (fieldsAmounts.size()-1) ){
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat, m_d->toString( fieldsAmounts.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( fieldsAmounts.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
                }
            }

            if( i != -- attrsToPrint.end()){
                // *** riga vuota ***
                BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
                // *** riga vuota ***
                BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            }
        }
    } else if( prOption == BillPrinter::AttributePrintUnion ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);

        BillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        QString title(trUtf8("Unione"));
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            title = QString("%1 %2").arg(title, (*i)->name() );
        }
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, title );
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        if( fieldsToPrint.size() == 0 ){
            BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        } else { // fieldsToPrint.size() > 0
            BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        }
        for( int i=0; i < 2*fieldsToPrint.size(); ++i ){
            if( i == 2*fieldsToPrint.size() -1 ){
                BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
            }
        }

        writeODTAttributeBillLineUnion( prItemsOption,
                                        &fieldsAmounts, fieldsToPrint, attrsToPrint, groupPrAm,
                                        cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                        leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                        txtCharFormat, txtBoldCharFormat );

        // *** riga vuota ***
        BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        BillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , title );
        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( fieldsToPrint.size() == 0 ){
            BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        } else { // fieldsToPrint.size() > 0
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        }
        for( int j=0; j < fieldsAmounts.size(); ++j ){
            if( j < fieldsToPrint.size() ){
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat, m_d->toString( fieldsAmounts.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( fieldsAmounts.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
            }
        }
    } else if( prOption == BillPrinter::AttributePrintIntersection ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);

        BillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        QString title(trUtf8("Intersezione"));
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            title = QString("%1 %2").arg(title, (*i)->name() );
        }
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, title );
        BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        if( fieldsToPrint.size() == 0 ){
            BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        } else { // fieldsToPrint.size() > 0
            BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        }
        for( int i=0; i < 2*fieldsToPrint.size(); ++i ){
            if( i == 2*fieldsToPrint.size() -1 ){
                BillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, m_d->priceFieldModel->priceName( fieldsToPrint.at(i)) );
            }
        }

        writeODTAttributeBillLineIntersection( prItemsOption,
                                               &fieldsAmounts, fieldsToPrint, attrsToPrint, groupPrAm,
                                               cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                               leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                               txtCharFormat, txtBoldCharFormat );

        // *** riga vuota ***
        BillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        BillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , title );
        BillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( fieldsToPrint.size() == 0 ){
            BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        } else { // fieldsToPrint.size() > 0
            BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        }
        for( int j=0; j < fieldsAmounts.size(); ++j ){
            if( j < fieldsToPrint.size() ){
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat, m_d->toString( fieldsAmounts.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                BillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( fieldsAmounts.at(j), 'f', m_d->priceFieldModel->precision( fieldsToPrint.at(j) ) ) );
            }
        }
    }

    // *** Riga di chiusura ***
    BillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

void BillItem::writeODTAttributeBillLineSimple( BillPrinter::PrintBillItemsOption prItemsOption,
                                                QList<double> * fieldsAmounts,
                                                const QList<int> &fieldsToPrint,
                                                Attribute *attrsToPrint,
                                                bool groupPrAm,
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
            for( int i = 0; i < fieldsToPrint.size(); ++i ){
                if( i >= fieldsAmounts->size() ){
                    fieldsAmounts->append(0.0);
                }
                (*fieldsAmounts)[i] += amount(fieldsToPrint.at(i) );
            }
            writeODTBillLine( prItemsOption,
                              false, fieldsToPrint, groupPrAm,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineSimple( prItemsOption,
                                                   fieldsAmounts, fieldsToPrint, attrsToPrint, groupPrAm,
                                                   cursor, table,
                                                   tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                   leftFormat, centralFormat, rightFormat,
                                                   centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                   txtCharFormat, txtBoldCharFormat );
        }
    }
}

void BillItem::writeODTAttributeBillLineUnion( BillPrinter::PrintBillItemsOption prItemsOption,
                                               QList<double> * fieldsAmounts,
                                               const QList<int> &fieldsToPrint,
                                               const QList<Attribute *> &attrsToPrint,
                                               bool groupPrAm,
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
            for( int i = 0; i < fieldsToPrint.size(); ++i ){
                if( i >= fieldsAmounts->size() ){
                    fieldsAmounts->append(0.0);
                }
                (*fieldsAmounts)[i] += amount(fieldsToPrint.at(i) );
            }
            writeODTBillLine( prItemsOption,
                              false, fieldsToPrint, groupPrAm,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineUnion( prItemsOption,
                                                  fieldsAmounts, fieldsToPrint, attrsToPrint, groupPrAm,
                                                  cursor, table,
                                                  tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                  leftFormat, centralFormat, rightFormat,
                                                  centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                  txtCharFormat, txtBoldCharFormat );
        }
    }
}

void BillItem::writeODTAttributeBillLineIntersection( BillPrinter::PrintBillItemsOption prItemsOption,
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
            for( int i = 0; i < fieldsToPrint.size(); ++i ){
                if( i >= fieldsAmounts->size() ){
                    fieldsAmounts->append(0.0);
                }
                (*fieldsAmounts)[i] += amount(fieldsToPrint.at(i) );
            }
            writeODTBillLine( prItemsOption,
                              false, fieldsToPrint, groupPrAm,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<BillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineIntersection( prItemsOption,
                                                         fieldsAmounts, fieldsToPrint, attrsToPrint, groupPrAm,
                                                         cursor, table,
                                                         tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                         leftFormat, centralFormat, rightFormat,
                                                         centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                         txtCharFormat, txtBoldCharFormat );
        }
    }
}

void BillItem::writeODTBillLine( BillPrinter::PrintBillItemsOption prItemsOption,
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
                                 QTextCharFormat & txtBoldCharFormat) {

    // Numero di colonne contenute
    int colCount = 0;
    if( writeProgCode ){
        colCount = 5 + 2 * fieldsToPrint.size();
    } else {
        colCount = 4 + 2 * fieldsToPrint.size();
    }

    // non ci sono sottoarticoli
    table->appendRows(1);
    cursor->movePosition(QTextCursor::PreviousRow );

    if( writeProgCode ){
        BillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progCode()  );
    }

    if( m_d->priceItem ){
        if( writeProgCode ){
            BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->codeFull() );
        } else {
            BillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, m_d->priceItem->codeFull() );
        }
        m_d->writeDescriptionCell( cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prItemsOption );
        // BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->shortDescriptionFull() );
    } else {
        if( writeProgCode ){
            BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        } else {
            BillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
        }
        BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
    }

    if( m_d->measuresModel != NULL ){
        // celle vuote
        // tag unita misura
        BillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
        // quantita'
        BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
        // campi
        for( int i=0; i < fieldsToPrint.size(); ++i ){
            if( i == fieldsToPrint.size() - 1 ){
                BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
            } else {
                BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            }
        }

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        if( writeProgCode ){
            // numero progressivo
            BillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
            // codice
            BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        } else {
            // codice
            BillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
        }

        // tag unita di misura
        QString unitMeasureTag;
        if( m_d->priceItem ){
            if( m_d->priceItem->unitMeasure()){
                unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
            }
        }

        for( int i=0; i < m_d->measuresModel->measuresCount(); ++i ){
            Measure * measure = m_d->measuresModel->measure(i);

            // formula senza spazi bianchi
            QString realFormula;
            if( measure != NULL ){
                realFormula = measure->formula();
                realFormula.remove(" ");
            }

            // misure
            if( measure != NULL ){
                if( realFormula.isEmpty() ){
                    BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() + " (" + measure->effectiveFormula() + ")");
                }
            } else {
                BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat);
            }

            if( realFormula.isEmpty() || measure == NULL ){
                // unita di misura
                BillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

                // quantità
                if( fieldsToPrint.size() == 0 ){
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                }
            } else {
                // unita di misura
                BillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                // quantità
                if( fieldsToPrint.size() == 0 ){
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, measure->quantityStr() );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->quantityStr() );
                }
            }

            // celle vuote
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                }
            }

            // inserisce riga
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            if( writeProgCode ){
                // numero progressivo
                BillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                // codice
                BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            } else {
                BillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            }
        }

        // descrizione breve
        BillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

        // tag unita di misura
        BillItemPrivate::writeCell( cursor, table,  centralFormat, tagBlockFormat, unitMeasureTag );

        // quantita totale
        if( fieldsToPrint.size() == 0 ){
            BillItemPrivate::writeCell( cursor, table, rightQuantityTotalFormat, numBlockFormat, quantityStr() );
        } else { // fieldsToPrint.size() > 0
            BillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, quantityStr() );
        }

        if( groupPrAm ){
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                QString priceValueStr = m_d->priceItem == NULL ? "" : m_d->priceItem->valueStr( fieldsToPrint.at(i), currentPriceDataSet() );
                BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, priceValueStr );
            }
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountStr(fieldsToPrint.at(i))  );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, amountStr(fieldsToPrint.at(i))  );
                }
            }
        } else {
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    QString priceValueStr = m_d->priceItem == NULL ? "" : m_d->priceItem->valueStr( fieldsToPrint.at(i), currentPriceDataSet() );
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, priceValueStr );
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountStr(fieldsToPrint.at(i))  );
                } else {
                    QString priceValueStr = m_d->priceItem == NULL ? "" : m_d->priceItem->valueStr( fieldsToPrint.at(i), currentPriceDataSet() );
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, priceValueStr );
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, amountStr(fieldsToPrint.at(i))  );
                }
            }
        }
    } else { // m_d->linesModel == NULL
        QString unitMeasureTag;
        if( m_d->priceItem ){
            if( m_d->priceItem->unitMeasure()){
                unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
            }
        }
        BillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

        if( fieldsToPrint.size() == 0 ){
            BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
        } else { // fieldsToPrint.size() > 0
            BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
        }

        if( groupPrAm ){
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                QString priceValueStr = m_d->priceItem == NULL ? "" : m_d->priceItem->valueStr( fieldsToPrint.at(i), currentPriceDataSet() );
                BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, priceValueStr );
            }
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountStr(fieldsToPrint.at(i))  );
                } else {
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, amountStr(fieldsToPrint.at(i))  );
                }
            }
        } else {
            for( int i=0; i < fieldsToPrint.size(); ++i ){
                if( i == fieldsToPrint.size() - 1 ){
                    QString priceValueStr = m_d->priceItem == NULL ? "" : m_d->priceItem->valueStr( fieldsToPrint.at(i), currentPriceDataSet() );
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, priceValueStr );
                    BillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountStr(fieldsToPrint.at(i))  );
                } else {
                    QString priceValueStr = m_d->priceItem == NULL ? "" : m_d->priceItem->valueStr( fieldsToPrint.at(i), currentPriceDataSet() );
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, priceValueStr );
                    BillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, amountStr(fieldsToPrint.at(i))  );
                }
            }
        }
    }
    if( m_d->parentItem->m_d->parentItem == NULL ){
        BillItemPrivate::insertEmptyRow( colCount, cursor, leftFormat, centralFormat, rightFormat );
    }
}
