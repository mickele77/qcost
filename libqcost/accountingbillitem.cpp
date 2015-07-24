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

#include "accountingbillitem.h"

#include "accountingbillitemprivate.h"

#include "accountingpricefieldmodel.h"
#include "accountingprinter.h"
#include "accountingprinter.h"
#include "pricelist.h"
#include "priceitem.h"
#include "measuresmodel.h"
#include "billitemmeasure.h"
#include "attributemodel.h"
#include "attribute.h"
#include "pricefieldmodel.h"
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

AccountingBillItem::AccountingBillItem(AccountingBillItem *parentItem, AccountingBillItem::ItemType iType,
                                       PriceFieldModel * pfm, MathParser * parser ):
    TreeItem(),
    m_d( new AccountingBillItemPrivate(parentItem, iType, pfm, parser ) ){

    if( parentItem != NULL ){
        connect( parentItem, &AccountingBillItem::attributesChanged, this, &AccountingBillItem::attributesChanged );
        connect( parentItem, &AccountingBillItem::discountChanged, this, &AccountingBillItem::discountChanged );
    }

    connect( this, static_cast<void(AccountingBillItem::*)( AccountingBillItem *, QList<int> )>(&AccountingBillItem::hasChildrenChanged), this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::nameChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::textChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::titleChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::priceItemChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::currentPriceDataSetChanged, this, &AccountingBillItem::itemChanged );

    connect( this, &AccountingBillItem::currentPriceDataSetChanged, this, &AccountingBillItem::updateTotalAmountToBeDiscounted );
    connect( this, &AccountingBillItem::currentPriceDataSetChanged, this, &AccountingBillItem::updateAmountNotToBeDiscounted );
    connect( this, &AccountingBillItem::totalAmountToBeDiscountedChanged, this, &AccountingBillItem::updateAmountToBeDiscounted );
    connect( this, &AccountingBillItem::amountNotToBeDiscountedChanged, this, &AccountingBillItem::updateAmountToBeDiscounted );
    connect( this, &AccountingBillItem::amountToBeDiscountedChanged, this, &AccountingBillItem::updateAmountDiscounted );
    connect( this, &AccountingBillItem::discountChanged, this, &AccountingBillItem::updateAmountDiscounted );
    connect( this, &AccountingBillItem::amountDiscountedChanged, this, &AccountingBillItem::updateTotalAmount );
    connect( this, &AccountingBillItem::amountNotToBeDiscountedChanged, this, &AccountingBillItem::updateTotalAmount );

    connect( this, &AccountingBillItem::totalAmountToBeDiscountedChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::amountNotToBeDiscountedChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::amountToBeDiscountedChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::amountDiscountedChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::totalAmountChanged, this, &AccountingBillItem::itemChanged );

    connect( this, &AccountingBillItem::attributesChanged, this, &AccountingBillItem::itemChanged );
}

AccountingBillItem::~AccountingBillItem(){
    emit aboutToBeDeleted();
    delete m_d;
}

AccountingBillItem &AccountingBillItem::operator=(const AccountingBillItem &cp) {
    if( &cp != this ){
        setName( cp.m_d->name );
        setItemType( cp.m_d->itemType);

        if( m_d->itemType == Root ){
            setCurrentPriceDataSet( cp.m_d->currentPriceDataSet);
            setDiscount( cp.m_d->discount);
            if( cp.hasChildren() ){
                removeChildren( 0, childrenCount() );
                for( int i=0; i < childrenCount(); ++i ){
                    appendChildren( cp.m_d->itemType );
                    *( m_d->childrenContainer.last() ) = *(cp.m_d->childrenContainer.at(i));
                }
            }
        }

        if( m_d->itemType == Bill ){
            setDateBegin( m_d->date );
            setDateEnd( m_d->dateEnd );
            if( cp.hasChildren() ){
                removeChildren( 0, childrenCount() );
                for( int i=0; i < childrenCount(); ++i ){
                    appendChildren( cp.m_d->itemType );
                    *( m_d->childrenContainer.last() ) = *(cp.m_d->childrenContainer.at(i));
                }
            }
        }

        if( m_d->itemType == PPU ){
            setPriceItem( cp.m_d->priceItem );
            setQuantity( cp.m_d->quantity );
            if( cp.m_d->measuresModel != NULL ){
                generateMeasuresModel();
                *(m_d->measuresModel) = *(cp.m_d->measuresModel);
            }
        }

        if( m_d->itemType == Comment ){
            setText( cp.m_d->text );
        }
    }

    return *this;
}

int AccountingBillItem::firstPriceFieldCol(){
    return m_d->totalAmountToBeDiscountedCol;
}

QString AccountingBillItem::name(){
    return m_d->name;
}

void AccountingBillItem::setName( const QString & newName ){
    if( m_d->name != newName ){
        m_d->name = newName;
        emit nameChanged( newName );
        emit dataChanged( this, m_d->priceShortDescCol );
    }
}

void AccountingBillItem::setItemType(AccountingBillItem::ItemType iType) {
    if( m_d->itemType != iType ){
        if( m_d->itemType == Root ){
            delete m_d->totalAmountPriceFieldModel;
            m_d->totalAmountPriceFieldModel = NULL;
            delete m_d->noDiscountAmountPriceFieldModel;
            m_d->noDiscountAmountPriceFieldModel = NULL;
        }

        m_d->itemType = iType;
        if( (iType == PPU) || (iType == Comment) ){
            removeChildren( 0, m_d->childrenContainer.size() );
        }

        if( iType == Root ){
            m_d->totalAmountPriceFieldModel = new AccountingPriceFieldModel(&(m_d->totalAmountPriceFieldsList), m_d->priceFieldModel );
            m_d->noDiscountAmountPriceFieldModel = new AccountingPriceFieldModel(&(m_d->noDiscountAmountPriceFieldsList), m_d->priceFieldModel );
        }
        emit itemTypeChanged( iType );
    }
}

TreeItem *AccountingBillItem::parentInternal() {
    return m_d->parentItem;
}

AccountingBillItem *AccountingBillItem::parent() {
    return m_d->parentItem;
}

void AccountingBillItem::setParent(AccountingBillItem * newParent, int position ) {
    if( m_d->parentItem != newParent ){
        if( m_d->parentItem != NULL ){
            m_d->parentItem->removeChildren( childNumber() );
            disconnect( m_d->parentItem, &AccountingBillItem::attributesChanged, this, &AccountingBillItem::attributesChanged );
        }
        m_d->parentItem = newParent;
        if( newParent != NULL ){
            newParent->addChild( this, position);
            connect( m_d->parentItem, &AccountingBillItem::attributesChanged, this, &AccountingBillItem::attributesChanged );
        }
    } else {
        int oldPosition = childNumber();
        if( oldPosition != position ){
            if( oldPosition > position ){
                oldPosition++;
            }
            m_d->parentItem->addChild( this, position );
            m_d->parentItem->removeChildren( oldPosition );
        }
    }
}

void AccountingBillItem::addChild(AccountingBillItem * newChild, int position ) {
    m_d->childrenContainer.insert( position, newChild );
}

AccountingBillItem *AccountingBillItem::itemId( unsigned int itemId ) {
    if( itemId == m_d->id ){
        return this;
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            AccountingBillItem * childItems = (*i)->itemId(itemId);
            if( childItems != NULL ) {
                return childItems;
            }
        }
    }
    return NULL;
}

AccountingBillItem *AccountingBillItem::findAccountingItemId( unsigned int searchitemId ) {
    if( m_d->parentItem == NULL ){
        return itemId(searchitemId );
    } else {
        return m_d->parentItem->findAccountingItemId(searchitemId);
    }
    return NULL;
}


AccountingBillItem *AccountingBillItem::childItem(int number) {
    return dynamic_cast<AccountingBillItem *>(child( number ));
}

void AccountingBillItem::setId( unsigned int ii ) {
    m_d->id = ii;
}

unsigned int AccountingBillItem::id() {
    return m_d->id;
}

int AccountingBillItem::progressiveCodeInternal() const {
    if( m_d->parentItem != NULL ){
        /* if( m_d->itemType == WorksProgressItem ){
            if( childNumber() > 0 ){
                return m_d->parentItem->childItem(childNumber()-1)->childrenCount() + \
                        m_d->parentItem->childItem(childNumber()-1)->progressiveCodeInternal();
            } else {
                return 0;
            }
        }
        if( m_d->itemType == MeasureItem ){
            return (childNumber()+1) + m_d->parentItem->progressiveCodeInternal();
        }*/
    }
    return 0;
}

QString AccountingBillItem::progressiveCode() const {
    /*if( m_d->itemType == MeasureItem ){
        return QString::number( progressiveCodeInternal() );
    }*/
    return QString();
}

QList<int> AccountingBillItem::totalAmountPriceFields() {
    if( m_d->itemType == Root ){
        return m_d->totalAmountPriceFieldsList;
    } else if( m_d->parentItem != NULL ){
        return m_d->parentItem->totalAmountPriceFields();
    }
    return QList<int>();
}


void AccountingBillItem::setTotalAmountPriceFields(const QList<int> &newAmountFields) {
    if( m_d->itemType == Root ){
        m_d->totalAmountPriceFieldsList = newAmountFields;
    }
}


AccountingPriceFieldModel *AccountingBillItem::totalAmountPriceFieldModel() {
    return m_d->totalAmountPriceFieldModel;
}

QList<int> AccountingBillItem::noDiscountAmountPriceFields() {
    if( m_d->itemType == Root ){
        return m_d->noDiscountAmountPriceFieldsList;
    } else if( m_d->parentItem != NULL ){
        return m_d->parentItem->noDiscountAmountPriceFields();
    }
    return QList<int>();
}

void AccountingBillItem::setNoDiscountAmountPriceFields(const QList<int> &newAmountFields) {
    if( m_d->itemType == Root ){
        m_d->noDiscountAmountPriceFieldsList = newAmountFields;
    }
}

void AccountingBillItem::updatePPUs() {
    if( m_d->itemType == PPU ){
        double newPPUTotalToBeDiscounted = 0.0;
        double newPPUNotToBeDiscounted = 0.0;
        if( m_d->priceItem != NULL ){
            for( QList<int>::iterator i = m_d->totalAmountPriceFieldsList.begin(); i != m_d->totalAmountPriceFieldsList.end(); ++i){
                newPPUTotalToBeDiscounted += m_d->priceItem->value( (*i), currentPriceDataSet() );
            }
            for( QList<int>::iterator i = m_d->noDiscountAmountPriceFieldsList.begin(); i != m_d->noDiscountAmountPriceFieldsList.end(); ++i){
                newPPUNotToBeDiscounted += m_d->priceItem->value( (*i), currentPriceDataSet() );
            }
        }
        if( newPPUTotalToBeDiscounted != m_d->PPUTotalToBeDiscounted ){
            m_d->PPUTotalToBeDiscounted = newPPUTotalToBeDiscounted;
            emit PPUTotalToBeDiscountedChanged( PPUTotalToBeDiscountedStr() );
        }
        if( newPPUNotToBeDiscounted != m_d->PPUNotToBeDiscounted ){
            m_d->PPUNotToBeDiscounted = newPPUNotToBeDiscounted;
            emit PPUNotToBeDiscountedChanged( PPUNotToBeDiscountedStr() );
        }
    }
}

AccountingPriceFieldModel *AccountingBillItem::noDiscountAmountPriceFieldModel() {
    return m_d->noDiscountAmountPriceFieldModel;
}

void AccountingBillItem::setDate(const QDate &d) {
    if( d.isValid() ){
        if( m_d->date != d ){
            m_d->date = d;
            emit dateChanged( dateStr() );
            emit dataChanged( this, m_d->dateCol );
        }
    }
}

void AccountingBillItem::setDate(const QString &d) {
    if( m_d->parser != NULL ){
        setDate( m_d->parser->evaluateDate(d, QLocale::NarrowFormat) );
    } else {
        setDate( QDate::fromString(d, Qt::DefaultLocaleShortDate) );
    }
}

QDate AccountingBillItem::dateBegin() const {
    return m_d->date;
}

QString AccountingBillItem::dateBeginStr() const {
    if( m_d->parser != NULL ){
        return m_d->parser->toString( m_d->date, QLocale::NarrowFormat );
    }
    return m_d->date.toString();
}

void AccountingBillItem::setDateBegin(const QDate &d) {
    if( d.isValid() ){
        if( m_d->date != d ){
            m_d->date = d;
            emit dateBeginChanged( dateStr() );
        }
    }
}

void AccountingBillItem::setDateBegin(const QString &d) {
    if( m_d->parser != NULL ){
        setDateBegin( m_d->parser->evaluateDate(d, QLocale::NarrowFormat) );
    } else {
        setDateBegin( QDate::fromString(d, Qt::DefaultLocaleShortDate) );
    }
}


QDate AccountingBillItem::dateEnd() const {
    return m_d->dateEnd;
}

QString AccountingBillItem::dateEndStr() const {
    if( m_d->parser != NULL ){
        return m_d->parser->toString( m_d->dateEnd, QLocale::NarrowFormat );
    }
    return m_d->dateEnd.toString();
}

void AccountingBillItem::setDateEnd(const QDate &d) {
    if( d.isValid() ){
        if( m_d->dateEnd != d ){
            m_d->dateEnd = d;
            emit dateEndChanged( dateEndStr() );
            emit dataChanged( this, m_d->dateCol );
        }
    }
}

void AccountingBillItem::setDateEnd(const QString &d) {
    if( m_d->parser != NULL ){
        setDateEnd( m_d->parser->evaluateDate(d, QLocale::NarrowFormat) );
    } else {
        setDateEnd( QDate::fromString(d, Qt::DefaultLocaleShortDate) );
    }
}

int AccountingBillItem::columnCount() const {
    return m_d->colCount;
}

QVariant AccountingBillItem::data(int col, int role) const {
    if( (col > m_d->colCount) || (col < 0) ||
            (role != Qt::DisplayRole && role != Qt::EditRole &&
             role != Qt::TextAlignmentRole && role != Qt::FontRole) ){
        return QVariant();
    }
    if( m_d->itemType == AccountingBillItem::Comment ) {
        if( role == Qt::FontRole ){
            QFont font;
            font.setItalic( true );
            return font;
        }
        if( col == m_d->priceShortDescCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( text() );
            }
        }
        if( role == Qt::TextAlignmentRole ){
            return Qt::AlignCenter + Qt::AlignVCenter;
        } else { // role == Qt::DisplayRole || role == Qt::EditRole
            return QVariant();
        }
    } else if( m_d->itemType == PPU ){
        if( col == m_d->progNumberCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( progressiveCode() );
            }
        } else if( col == m_d->dateCol) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( dateStr() );
            }
        } else if( col == m_d->priceCodeCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                if( m_d->priceItem != NULL ){
                    return QVariant( m_d->priceItem->codeFull() );
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceShortDescCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                if( m_d->priceItem != NULL ){
                    return QVariant(m_d->priceItem->shortDescriptionFull());
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceUmCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                if( m_d->priceItem != NULL ){
                    if( m_d->priceItem->unitMeasure() != NULL ){
                        return QVariant(m_d->priceItem->unitMeasure()->tag() );
                    } else {
                        return QVariant( "---" );
                    }
                }
            }
        } else if( col == m_d->quantityCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( quantityStr() );
            }
        } else if( col == m_d->totalAmountToBeDiscountedCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( PPUTotalToBeDiscountedStr() );
            }
        } else if( col == (m_d->totalAmountToBeDiscountedCol+1 ) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( totalAmountToBeDiscountedStr() );
            }
        } else if( col == m_d->amountNotToBeDiscountedCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( PPUNotToBeDiscountedStr() );
            }
        } else if( col == (m_d->amountNotToBeDiscountedCol+1) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( amountNotToBeDiscountedStr() );
            }
        } else if( col == m_d->totalAmountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( totalAmountStr() );
            }
        } else {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant();
            }
        }
    } else if( m_d->itemType == Bill ){
        if( role == Qt::FontRole ){
            QFont font;
            font.setBold( true );
            return font;
        }
        if( col == m_d->dateCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( dateBeginStr() );
            }
        } else if( col == m_d->priceShortDescCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( title() );
            }
        } else if( col == (m_d->totalAmountToBeDiscountedCol+1 ) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( totalAmountToBeDiscountedStr() );
            }
        } else if( col == ( m_d->amountNotToBeDiscountedCol+1 ) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( amountNotToBeDiscountedStr() );
            }
        } else if( col == ( m_d->totalAmountCol ) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( totalAmountStr() );
            }
        } else {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant();
            }
        }
    } else if( m_d->itemType == Root ){
        if( col == m_d->progNumberCol) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("N.") );
            }
        } else if( col == m_d->dateCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("Data") );
            }
        } else if( col == m_d->priceCodeCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("Codice") );
            }
        } else if( col == m_d->priceShortDescCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant(QObject::trUtf8("Descrizione") );
            }
        }  else if( col == m_d->priceUmCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("UdM") );
            }
        } else if( col == m_d->quantityCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignCenter + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("Quantità") );
            }
        } else if( col == m_d->totalAmountToBeDiscountedCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("C.U. lordo") );
            }
        } else if( col == ( m_d->totalAmountToBeDiscountedCol+1 ) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("Importo lordo") );
            }
        } else if( col == m_d->amountNotToBeDiscountedCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("C.U. non rib.") );
            }
        } else if( col == (m_d->amountNotToBeDiscountedCol+1) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("Importo non rib.") );
            }
        } else if( col == m_d->totalAmountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant( trUtf8("Importo totale") );
            }
        } else {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else { // role == Qt::DisplayRole || role == Qt::EditRole
                return QVariant();
            }
        }
    }
    return QVariant();
}

bool AccountingBillItem::setData(int column, const QVariant &value) {
    if( m_d->itemType == AccountingBillItem::Comment ) {
        if( column == m_d->priceShortDescCol ){
            setText( value.toString() );
            return true;
        }
    } else if( m_d->itemType == AccountingBillItem::PPU ){
        if( column == m_d->quantityCol ){
            setQuantity( value.toString() );
            emit dataChanged( this, m_d->totalAmountToBeDiscountedCol);
            emit dataChanged( this, m_d->totalAmountToBeDiscountedCol + 1 );
            emit dataChanged( this, m_d->amountNotToBeDiscountedCol);
            emit dataChanged( this, m_d->amountNotToBeDiscountedCol + 1 );
            emit dataChanged( this, m_d->totalAmountCol);
            return true;
        } else {
            if( column == m_d->dateCol ){
                setDate( value.toString() );
                return true;
            }
        }
    }
    return false;
}

int AccountingBillItem::currentPriceDataSet() const{
    if( m_d->itemType == Root ){
        return m_d->currentPriceDataSet;
    } else if( m_d->parentItem ){
        return m_d->parentItem->currentPriceDataSet();
    }
    return 0;
}

double AccountingBillItem::discount() const {
    if( m_d->itemType == Root ){
        return m_d->discount;
    } else if( m_d->parentItem ){
        return m_d->parentItem->discount();
    }
    return 0.0;
}

void AccountingBillItem::setCurrentPriceDataSet(int newVal ) {
    if( m_d->itemType == Root ){
        if( newVal != m_d->currentPriceDataSet ){
            m_d->currentPriceDataSet = newVal;
            emit currentPriceDataSetChanged( newVal );
        }
    } else if( m_d->parentItem ){
        m_d->parentItem->setCurrentPriceDataSet( newVal );
    }
}

void AccountingBillItem::setDiscount(double newVal ) {
    if( m_d->itemType == Root ){
        if( newVal != m_d->discount ){
            m_d->discount = newVal;
            emit discountChanged( newVal );
        }
    } else if( m_d->parentItem ){
        m_d->parentItem->setDiscount( newVal );
    }
}

void AccountingBillItem::updateTotalAmountToBeDiscounted() {
    double v = 0.0;
    if( (m_d->itemType == Root) || (m_d->itemType == Bill)){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            v += (*i)->totalAmountToBeDiscounted();
        }
    } else if(m_d->itemType == PPU) {
        v = UnitMeasure::applyPrecision( m_d->PPUTotalToBeDiscounted * m_d->quantity, m_d->amountPrecision );
    } else if(m_d->itemType == Comment) {
        v = 0.0;
    }
    if( v != m_d->totalAmountToBeDiscounted ){
        m_d->totalAmountToBeDiscounted = v;
        emit totalAmountToBeDiscountedChanged( totalAmountToBeDiscountedStr() );
    }
}

void AccountingBillItem::updateAmountNotToBeDiscounted() {
    double v = 0.0;
    if( (m_d->itemType == Root) || (m_d->itemType == Bill)){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            v += (*i)->amountNotToBeDiscounted();
        }
    } else if(m_d->itemType == PPU) {
        v = UnitMeasure::applyPrecision( m_d->PPUNotToBeDiscounted * m_d->quantity, m_d->amountPrecision );
    } else if(m_d->itemType == Comment) {
        v = 0.0;
    }
    if( v != m_d->amountNotToBeDiscounted ){
        m_d->amountNotToBeDiscounted = v;
        emit amountNotToBeDiscountedChanged( totalAmountToBeDiscountedStr() );
    }
}

void AccountingBillItem::updateAmountToBeDiscounted() {
    double v = m_d->totalAmountToBeDiscounted - m_d->amountNotToBeDiscounted;
    if( v != m_d->amountToBeDiscounted ){
        m_d->amountToBeDiscounted = v;
        emit amountToBeDiscountedChanged( amountToBeDiscountedStr() );
    }
}

void AccountingBillItem::updateAmountDiscounted() {
    double v = UnitMeasure::applyPrecision( m_d->amountToBeDiscounted * (1.0 - discount() ), m_d->amountPrecision );;
    if( v != m_d->amountDiscounted ){
        m_d->amountDiscounted = v;
        emit amountDiscountedChanged( amountDiscountedStr() );
    }
}

void AccountingBillItem::updateTotalAmount() {
    double v = m_d->amountDiscounted + m_d->amountNotToBeDiscounted;
    if( v != m_d->totalAmount ) {
        m_d->totalAmount = v;
        emit totalAmountChanged( totalAmountStr() );
    }
}

bool AccountingBillItem::isUsingPriceItem(PriceItem *p) {
    if( (m_d->itemType == Root) || (m_d->itemType == Bill) ){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            if( (*i)->isUsingPriceItem(p) ){
                return true;
            }
        }
    } else if( m_d->itemType == PPU ){
        if( m_d->priceItem == p ){
            return true;
        }
    }
    return false;
}

QList<PriceItem *> AccountingBillItem::usedPriceItems() const {
    QList<PriceItem *> ret;
    appendUsedPriceItems( &ret );
    return ret;
}

void AccountingBillItem::appendUsedPriceItems( QList<PriceItem *> * usedPriceItems ) const {
    if( (m_d->itemType == Root) || (m_d->itemType == Bill)){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->appendUsedPriceItems( usedPriceItems );
        }
    } else if( m_d->itemType == PPU ){
        if( !usedPriceItems->contains( m_d->priceItem )){
            usedPriceItems->append( m_d->priceItem );
        }
    }
}

void AccountingBillItem::setHasChildrenChanged(AccountingBillItem *p, QList<int> indexes) {
    if( m_d->parentItem ){
        // non è l'oggetto root - rimandiamo all'oggetto root
        m_d->parentItem->setHasChildrenChanged( p, indexes );
    } else {
        // è l'oggetto root - emette il segnale di numero figli cambiato
        emit hasChildrenChanged( p, indexes );
    }
}

TreeItem *AccountingBillItem::child(int number) {
    if( number >= 0 && number < m_d->childrenContainer.size() ){
        return m_d->childrenContainer[number];
    }
    return NULL;
}

int AccountingBillItem::childrenCount() const {
    return m_d->childrenContainer.size();
}

bool AccountingBillItem::hasChildren() const {
    return (m_d->childrenContainer.size() > 0);
}

QList<AccountingBillItem *> AccountingBillItem::allChildren() {
    QList<AccountingBillItem *> ret;
    for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        ret.append( *i );
        if( (*i)->hasChildren() ){
            ret.append( (*i)->allChildren() );
        }
    }
    return ret;
}

QList<AccountingBillItem *> AccountingBillItem::allChildrenWithMeasures() {
    QList<AccountingBillItem *> ret;
    for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        if( (*i)->hasChildren() ){
            ret.append( (*i)->allChildrenWithMeasures() );
        } else {
            ret.append( this );
        }
    }
    return ret;
}

bool AccountingBillItem::insertChildren(int position, int count) {
    if( m_d->itemType == Root ){
        return insertChildren( Bill, position, count );
    }
    if( m_d->itemType == Bill ){
        return insertChildren( PPU, position, count );
    }
    return false;
}

bool AccountingBillItem::insertChildren(AccountingBillItem::ItemType iType, int position, int count ){
    if (position < 0 || position > m_d->childrenContainer.size() )
        return false;

    if( (m_d->itemType == Root && (iType == Bill) ) ||
            (m_d->itemType == Bill && (iType == PPU || iType == Comment  || iType == TimeAndMaterials  || iType == LumpSum )) ){

        bool hadChildren = m_d->childrenContainer.size() > 0;

        for (int row = 0; row < count; ++row) {
            AccountingBillItem * item = new AccountingBillItem( this, iType,  m_d->priceFieldModel, m_d->parser );
            while( findAccountingItemId( item->id() ) != NULL ){
                item->setId( item->id() + 1 );
            }
            m_d->childrenContainer.insert(position, item);
            if( iType == Bill ){
                for( int j = position+1; j < m_d->childrenContainer.size(); ++j  ){
                    m_d->childrenContainer.at(j)->emitTitleChanged();
                }
            }
            connect( item, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged), this, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged) );
            connect( item, &AccountingBillItem::totalAmountToBeDiscountedChanged, this, &AccountingBillItem::updateTotalAmountToBeDiscounted );
            connect( item, &AccountingBillItem::amountNotToBeDiscountedChanged, this, &AccountingBillItem::updateAmountNotToBeDiscounted );
            connect( this, &AccountingBillItem::currentPriceDataSetChanged, item, &AccountingBillItem::setCurrentPriceDataSet );
            connect( item, &AccountingBillItem::itemChanged, this, &AccountingBillItem::itemChanged );
        }

        if( !hadChildren ){
            if( m_d->childrenContainer.size() > 0 ){
                emit hasChildrenChanged( true );
            }
        }

        emit itemChanged();

        return true;
    }
    return false;
}

bool AccountingBillItem::appendChildren(ItemType iType, int count) {
    return insertChildren( iType, m_d->childrenContainer.size(), count );
}

bool AccountingBillItem::removeChildren(int position, int count) {
    if (position < 0 || position + count > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row){
        AccountingBillItem * item = m_d->childrenContainer.at( position );
        disconnect( item, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged), this, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged) );
        disconnect( item, &AccountingBillItem::totalAmountToBeDiscountedChanged, this, &AccountingBillItem::updateTotalAmountToBeDiscounted );
        disconnect( item, &AccountingBillItem::amountNotToBeDiscountedChanged, this, &AccountingBillItem::updateAmountNotToBeDiscounted );
        disconnect( this, &AccountingBillItem::currentPriceDataSetChanged, item, &AccountingBillItem::setCurrentPriceDataSet );
        disconnect( item, &AccountingBillItem::itemChanged, this, &AccountingBillItem::itemChanged );
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

bool AccountingBillItem::reset() {
    return removeChildren( 0, m_d->childrenContainer.size() );
}

int AccountingBillItem::childNumber() const {
    if (m_d->parentItem)
        return m_d->parentItem->m_d->childrenContainer.indexOf( const_cast<AccountingBillItem *>(this) );

    return 0;
}

Qt::ItemFlags AccountingBillItem::flags(int column) const {
    if( m_d->itemType == Comment ){
        if( column == m_d->priceShortDescCol ){
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
        } else {
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        }
    } else if( m_d->itemType == PPU ){
        if( column == m_d->progNumberCol ){
            return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        } else if( column == m_d->quantityCol ){
            if( m_d->measuresModel == NULL ){
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
            } else {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            }
        }
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void AccountingBillItem::writeXml(QXmlStreamWriter *writer) {
    if( m_d->itemType == Root ){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml( writer );
        }
    } else if( m_d->itemType == Bill ){
        writer->writeStartElement( "AccountingBillItemBill" );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "dateBegin", m_d->date.toString() );
        writer->writeAttribute( "dateEnd", m_d->dateEnd.toString() );
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml( writer );
        }
        writer->writeEndElement();
    } else if( m_d->itemType == PPU ){
        writer->writeStartElement( "AccountingBillItemPPU" );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "date", m_d->date.toString() );
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
        if( m_d->priceItem != NULL ){
            writer->writeAttribute( "priceItem", QString::number( m_d->priceItem->id() ) );
        }
        writer->writeAttribute( "quantity", QString::number( m_d->quantity ) );

        if( m_d->measuresModel != NULL ){
            m_d->measuresModel->writeXml( writer );
        }
        writer->writeEndElement();
    } else if( m_d->itemType == Comment ){
        writer->writeStartElement( "AccountingBillItemComment" );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "text", m_d->text );
        writer->writeEndElement();
    }
}

void AccountingBillItem::readXml(QXmlStreamReader *reader, PriceList * priceList, AttributeModel * billAttrModel ) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGTAMBILL")  ){
        if( m_d->itemType == Root ){
            if( reader->name().toString().toUpper() == "ACCOUNTINGTAMBILLITEMBILL" && reader->isStartElement()) {
                appendChildren( Bill );
            }
        } else if( m_d->itemType == Bill ){
            if( reader->name().toString().toUpper() == "ACCOUNTINGTAMBILLITEMCOMMENT" && reader->isStartElement()) {
                appendChildren( Comment );
            } else if( reader->name().toString().toUpper() == "ACCOUNTINGTAMBILLITEMPPU" && reader->isStartElement()) {
                appendChildren( PPU );
            }
        }
        m_d->childrenContainer.last()->readXml( reader, priceList, billAttrModel );
        reader->readNext();

    }
}

void AccountingBillItem::readXmlTmp(QXmlStreamReader *reader) {
    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGTAMBILL")  ){
        if( (reader->name().toString().toUpper() == "ACCOUNTINGTAMBILLITEMBILL") &&
                (reader->isStartElement()) &&
                (m_d->itemType == Root) ){
            appendChildren( Bill );
            m_d->childrenContainer.last()->readXmlTmp( reader );
        } else if( (reader->name().toString().toUpper() == "ACCOUNTINGTAMBILLITEMPPU") &&
                   (reader->isStartElement()) &&
                   (m_d->itemType == Bill) ){
            appendChildren( PPU );
            m_d->childrenContainer.last()->readXmlTmp( reader );
        } else if( (reader->name().toString().toUpper() == "ACCOUNTINGTAMBILLITEMCOMMENT") &&
                   reader->isStartElement() &&
                   (m_d->itemType == Bill) ){
            appendChildren( Comment );
            m_d->childrenContainer.last()->readXmlTmp( reader );
        } else if( (reader->name().toString().toUpper() == "MEASURESMODEL") &&
                   reader->isStartElement() &&
                   m_d->itemType == PPU ){
            generateMeasuresModel()->readXml( reader );
        }
        reader->readNext();
    }
}

void AccountingBillItem::loadFromXml(const QXmlStreamAttributes &attrs, PriceList * priceList, AttributeModel * billAttrModel) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "date" ) ){
        m_d->date = QDate::fromString( attrs.value( "date").toString() );
    }
    if( attrs.hasAttribute( "dateBegin" ) ){
        m_d->date = QDate::fromString( attrs.value( "dateBegin").toString() );
    }
    if( attrs.hasAttribute( "dateEnd" ) ){
        m_d->date = QDate::fromString( attrs.value( "dateEnd").toString() );
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

    if( m_d->itemType == Comment ){
        if( attrs.hasAttribute( "text" ) ){
            setText( attrs.value("text").toString() );
        }
    } else if( m_d->itemType == PPU ){
        if( attrs.hasAttribute( "quantity" ) ){
            QString qStr = attrs.value("quantity").toString();
            setQuantity( qStr.toDouble() );
        }
        if( attrs.hasAttribute( "priceItem" ) ){
            if( priceList ){
                setPriceItem( priceList->priceItemId( attrs.value( "priceItem").toUInt() ) );
            }
        }
    }
}

void AccountingBillItem::loadFromXmlTmp(const QXmlStreamAttributes &attrs) {
    m_d->tmpAttributes.clear();
    m_d->tmpAttributes = attrs;
}

void AccountingBillItem::loadTmpData( PriceList *priceList, AttributeModel * billAttrModel) {
    if( !m_d->tmpAttributes.isEmpty() ){
        loadFromXml(m_d->tmpAttributes, priceList, billAttrModel );
        m_d->tmpAttributes.clear();
    }
    for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->loadTmpData( priceList, billAttrModel );
    }
}

QList< QPair<Attribute *, bool> > AccountingBillItem::attributes() {
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

QList<Attribute *> AccountingBillItem::allAttributes() {
    QList<Attribute *> attrs = inheritedAttributes();
    for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
        if( !attrs.contains( *i) ){
            attrs.append( *i );
        }
    }
    return attrs;
}

QList<Attribute *> AccountingBillItem::directAttributes() const {
    return QList<Attribute *>( m_d->attributes );
}

void AccountingBillItem::addAttribute( Attribute * attr ){
    if( !(m_d->attributes.contains( attr )) ){
        m_d->attributes.append( attr );
        emit attributesChanged();
    }
}

void AccountingBillItem::removeAttribute( Attribute * attr ){
    if( m_d->attributes.removeAll( attr ) > 0 ){
        emit attributesChanged();
    }
}

void AccountingBillItem::removeAllAttributes() {
    m_d->attributes.clear();
    emit attributesChanged();
}

double AccountingBillItem::totalAmountToBeDiscountedAttribute(Attribute *attr ) const {
    if( m_d->childrenContainer.size() == 0 ){
        QList<Attribute *> attrs = inheritedAttributes();
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( !attrs.contains( *i) ){
                attrs.append( *i );
            }
        }
        if( attrs.contains(attr) ){
            return totalAmountToBeDiscounted();
        }
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmountToBeDiscountedAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingBillItem::totalAmountToBeDiscountedAttributeStr(Attribute *attr ) const {
    int prec = m_d->amountPrecision;
    QString ret;
    double v = totalAmountToBeDiscountedAttribute( attr );
    if( m_d->parser == NULL ){
        ret = QString::number(v, 'f', prec );
    } else {
        ret = m_d->parser->toString( v, 'f', prec );
    }
    return ret;
}

double AccountingBillItem::amountNotToBeDiscountedAttribute(Attribute *attr ) const {
    if( m_d->childrenContainer.size() == 0 ){
        QList<Attribute *> attrs = inheritedAttributes();
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( !attrs.contains( *i) ){
                attrs.append( *i );
            }
        }
        if( attrs.contains(attr) ){
            return amountNotToBeDiscounted();
        }
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountNotToBeDiscountedAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingBillItem::amountNotToBeDiscountedAttributeStr(Attribute *attr ) const {
    int prec = m_d->amountPrecision;
    QString ret;
    double v = amountNotToBeDiscountedAttribute( attr );
    if( m_d->parser == NULL ){
        ret = QString::number(v, 'f', prec );
    } else {
        ret = m_d->parser->toString( v, 'f', prec );
    }
    return ret;
}

double AccountingBillItem::totalAmountAttribute(Attribute *attr ) const {
    if( m_d->childrenContainer.size() == 0 ){
        QList<Attribute *> attrs = inheritedAttributes();
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( !attrs.contains( *i) ){
                attrs.append( *i );
            }
        }
        if( attrs.contains(attr) ){
            return totalAmount();
        }
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmountAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingBillItem::totalAmountAttributeStr(Attribute *attr ) const {
    int prec = m_d->amountPrecision;
    QString ret;
    double v = totalAmountAttribute( attr );
    if( m_d->parser == NULL ){
        ret = QString::number(v, 'f', prec );
    } else {
        ret = m_d->parser->toString( v, 'f', prec );
    }
    return ret;
}

QList<Attribute *> AccountingBillItem::inheritedAttributes() const {
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

bool AccountingBillItem::containsAttribute(Attribute *attr) const {
    if( containsAttributeDirect(attr)){
        return true;
    }
    return containsAttributeInherited( attr );
}

bool AccountingBillItem::containsAttributeInherited(Attribute *attr) const {
    if( m_d->parentItem != NULL ){
        if( m_d->parentItem->containsAttributeDirect( attr ) ){
            return true;
        } else {
            return m_d->parentItem->containsAttributeInherited( attr );
        }
    }
    return false;
}

bool AccountingBillItem::containsAttributeDirect(Attribute *attr) const {
    return m_d->attributes.contains( attr );
}

bool AccountingBillItem::isDescending(AccountingBillItem *ancestor) {
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

QList<PriceItem *> AccountingBillItem::connectedPriceItems() const {
    QList<PriceItem *> ret;

    if( m_d->itemType == AccountingBillItem::PPU ){
        return m_d->priceItem->connectedPriceItems();
    }

    if( (m_d->itemType == AccountingBillItem::Root) ||
            (m_d->itemType == AccountingBillItem::Bill)   ){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            QList<PriceItem *> conPI = (*i)->connectedPriceItems();
            for(QList<PriceItem *>::iterator j = conPI.begin(); j != conPI.end(); ++j ){
                if( !ret.contains(*j) ){
                    ret.append(*j);
                }
            }
        }
    }
    return ret;
}

void AccountingBillItem::setLSBill(AccountingLSBill *newLSBill) {
    if( m_d->lsBill != newLSBill ){
        m_d->lsBill = newLSBill;
        emit lsBillChanged( m_d->lsBill );
    }
}

AccountingLSBill *AccountingBillItem::lsBill() {
    return m_d->lsBill;
}

void AccountingBillItem::setTAMBill(AccountingTAMBill *newTAMBill) {
    if( m_d->tamBill != newTAMBill ){
        m_d->tamBill = newTAMBill;
        emit tamBillChanged( m_d->tamBill );
    }
}

AccountingTAMBill *AccountingBillItem::tamBill() {
    return m_d->tamBill;
}

QDate AccountingBillItem::date() const {
    return m_d->date;
}

QString AccountingBillItem::dateStr() const {
    if( m_d->parser != NULL ){
        return m_d->parser->toString( m_d->date, QLocale::NarrowFormat );
    }
    return m_d->date.toString();
}

double AccountingBillItem::totalAmountToBeDiscounted() const {
    return m_d->totalAmountToBeDiscounted;
}

double AccountingBillItem::amountNotToBeDiscounted() const {
    return m_d->amountNotToBeDiscounted;
}

double AccountingBillItem::amountToBeDiscounted() const {
    return m_d->amountToBeDiscounted;
}

double AccountingBillItem::amountDiscounted() const {
    return m_d->amountDiscounted;
}

double AccountingBillItem::totalAmount() const {
    return m_d->totalAmount;
}

QString AccountingBillItem::totalAmountToBeDiscountedStr() const {
    return m_d->toString( m_d->totalAmountToBeDiscounted, 'f', m_d->amountPrecision );
}

QString AccountingBillItem::amountNotToBeDiscountedStr() const{
    return m_d->toString( m_d->amountNotToBeDiscounted, 'f', m_d->amountPrecision );
}

QString AccountingBillItem::amountToBeDiscountedStr() const {
    return m_d->toString( m_d->amountToBeDiscounted, 'f', m_d->amountPrecision );
}

QString AccountingBillItem::amountDiscountedStr() const {
    return m_d->toString( m_d->amountDiscounted, 'f', m_d->amountPrecision );
}

QString AccountingBillItem::totalAmountStr() const {
    return m_d->toString( m_d->totalAmount, 'f', m_d->amountPrecision );
}

QString AccountingBillItem::title() const{
    if( m_d->itemType == Bill ){
        return trUtf8("S.A.L. N.%1 (%2-%3)").arg(QString::number(childNumber()+1), dateBeginStr(), dateEndStr() );
    }
    return QString();
}

QString AccountingBillItem::text() const {
    return m_d->text;
}

void AccountingBillItem::setText( const QString &t ){
    if( t !=  m_d->text ){
        m_d->text = t;
        emit itemChanged();
        emit textChanged(m_d->text);
        emit dataChanged( this, m_d->priceShortDescCol );
    }
}

PriceItem *AccountingBillItem::priceItem() {
    return m_d->priceItem;
}

void AccountingBillItem::setPriceItem(PriceItem * p) {
    if( m_d->priceItem != p ){
        if( m_d->priceItem != NULL ) {
            disconnect( m_d->priceItem, &PriceItem::codeFullChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::valueChanged, this, &AccountingBillItem::emitPriceDataUpdated );
        }
        PriceItem * oldPriceItem = m_d->priceItem;

        m_d->priceItem = p;

        emit priceItemChanged( oldPriceItem, p );

        if( m_d->priceItem != NULL ){
            connect( m_d->priceItem, &PriceItem::codeFullChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::valueChanged, this, &AccountingBillItem::emitPriceDataUpdated );
        }

        emit itemChanged();
        emit dataChanged( this, m_d->priceCodeCol );
        emit dataChanged( this, m_d->priceUmCol );
        emit dataChanged( this, m_d->totalAmountToBeDiscountedCol );
        emit dataChanged( this, m_d->totalAmountToBeDiscountedCol+1 );
        emit dataChanged( this, m_d->amountNotToBeDiscountedCol );
        emit dataChanged( this, m_d->amountNotToBeDiscountedCol+1 );
        emit dataChanged( this, m_d->totalAmountCol );

        if( m_d->measuresModel != NULL ){
            m_d->measuresModel->setUnitMeasure( m_d->priceItem->unitMeasure() );
        }

        updateTotalAmountToBeDiscounted();
        updateAmountNotToBeDiscounted();
    }
}

double AccountingBillItem::quantity() const {
    return m_d->quantity;
}

QString AccountingBillItem::quantityStr() const {
    int prec = 2;
    if( m_d->priceItem != NULL ){
        if( m_d->priceItem->unitMeasure() != NULL ){
            prec = m_d->priceItem->unitMeasure()->precision();
        }
    }
    return m_d->toString( m_d->quantity, 'f', prec );
}

void AccountingBillItem::setQuantityPrivate(double v) {
    if( v != m_d->quantity ){
        m_d->quantity = v;
        emit quantityChanged( quantityStr() );
        emit itemChanged();
        emit dataChanged( this, m_d->quantityCol );
        updateTotalAmountToBeDiscounted();
        updateAmountNotToBeDiscounted();
    }
}

void AccountingBillItem::setQuantity(double v) {
    if( m_d->measuresModel == NULL ){
        setQuantityPrivate( v );
    }
}

void AccountingBillItem::setQuantity(const QString &vstr ) {
    if( m_d->parser != NULL ){
        setQuantity( m_d->parser->evaluate( vstr ) );
    } else {
        setQuantity( vstr.toDouble() );
    }
}

double AccountingBillItem::PPUTotalToBeDiscounted() const {
    return m_d->PPUTotalToBeDiscounted;
}

double AccountingBillItem::PPUNotToBeDiscounted() const {
    return m_d->PPUNotToBeDiscounted;
}

QString AccountingBillItem::PPUTotalToBeDiscountedStr() const {
    return m_d->toString( m_d->PPUTotalToBeDiscounted, 'f', m_d->amountPrecision );
}

QString AccountingBillItem::PPUNotToBeDiscountedStr() const {
    return m_d->toString( m_d->PPUNotToBeDiscounted, 'f', m_d->amountPrecision );
}

void AccountingBillItem::emitPriceDataUpdated() {
    updateTotalAmountToBeDiscounted();
    updateAmountNotToBeDiscounted();
    emit dataChanged( this, m_d->priceCodeCol );
    emit dataChanged( this, m_d->priceUmCol );
    emit dataChanged( this, m_d->totalAmountToBeDiscountedCol);
    emit dataChanged( this, m_d->totalAmountToBeDiscountedCol + 1 );
    emit dataChanged( this, m_d->amountNotToBeDiscountedCol);
    emit dataChanged( this, m_d->amountNotToBeDiscountedCol + 1 );
    emit dataChanged( this, m_d->totalAmountCol);
}

void AccountingBillItem::emitTitleChanged() {
    emit titleChanged( title() );
    emit dataChanged( this, m_d->priceShortDescCol );
}

MeasuresModel *AccountingBillItem::measuresModel() {
    return m_d->measuresModel;
}

MeasuresModel *AccountingBillItem::generateMeasuresModel() {
    if( m_d->measuresModel == NULL ){
        UnitMeasure * ump = NULL;
        if( m_d->priceItem != NULL ){
            ump = m_d->priceItem->unitMeasure();
        }
        m_d->measuresModel = new MeasuresModel( m_d->parser, ump );
        setQuantity( m_d->measuresModel->quantity() );
        connect( m_d->measuresModel, &MeasuresModel::quantityChanged, this, &AccountingBillItem::setQuantityPrivate );
        connect( m_d->measuresModel, &MeasuresModel::modelChanged, this, &AccountingBillItem::itemChanged );
        emit itemChanged();
    }
    return m_d->measuresModel;
}

void AccountingBillItem::removeMeasuresModel() {
    if( m_d->measuresModel != NULL ){
        disconnect( m_d->measuresModel, &MeasuresModel::quantityChanged, this, &AccountingBillItem::setQuantityPrivate );
        disconnect( m_d->measuresModel, &MeasuresModel::modelChanged, this, &AccountingBillItem::itemChanged );
        delete m_d->measuresModel;
        m_d->measuresModel = NULL;
        updateTotalAmountToBeDiscounted();
        updateAmountNotToBeDiscounted();
    }
}

#include "qtextformatuserdefined.h"

void AccountingBillItem::writeODTAccountingOnTable( QTextCursor *cursor,
                                                    AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                                    bool printAmounts ) const {
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
    int cellCount = 6;
    if( printAmounts ){
        cellCount += 2;
    }

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    if( m_d->parentItem == NULL ){
        // *** Riga di intestazione ***
        AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N."), false);
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Data") );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazioni dei lavori"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
        if( printAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        }

        // *** Riga vuota ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        // *** Scrive il computo dei sottoarticoli ***
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeODTAccountingOnTable( cursor, prItemsOption, printAmounts );
        }

        // *** riga dei totali complessivi
        if( printAmounts ){
            // riga vuota
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale complessivo") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );

            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountStr() );

        }

        // *** Riga di chiusura ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );

    } else {
        if( hasChildren() ){
            // ci sono sottoarticoli
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat, progressiveCode() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, m_d->name );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            if( printAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            }

            for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTAccountingOnTable( cursor, prItemsOption, printAmounts );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale %1").arg( m_d->name ) );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            if( printAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, totalAmountToBeDiscountedStr() );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            }
        } else { // !hasChildren()
            writeODTBillLine( prItemsOption,
                              true, printAmounts,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingBillItem::writeODTSummaryOnTable( QTextCursor *cursor,
                                                 AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                                 bool printAmounts,
                                                 bool writeDetails ) const {
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
    if( printAmounts ) {
        cellCount += 2;
    }

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    // *** Intestazione tabella ***
    AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("Codice"), false );
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Denominazione"));
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( printAmounts ){
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importi"));
    } else {
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    }

    // *** riga vuota ***
    AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    // *** Righe del sommario ***
    QList<PriceItem *> usedPItems = usedPriceItems();
    for( QList<PriceItem *>::iterator i = usedPItems.begin(); i != usedPItems.end(); ++i){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        if( (*i) != NULL ){
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, (*i)->codeFull() );
            m_d->writeDescriptionCell( (*i), cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prItemsOption );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
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
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            if( printAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            }
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
        }

        QList<double> fieldsAmount;
        fieldsAmount << 0.0 << 0.0 << 0.0;
        double itemTotalQuantity = 0.0;
        for( QList<AccountingBillItem *>::iterator j = m_d->childrenContainer.begin(); j != m_d->childrenContainer.end(); ++j ){
            (*j)->writeODTSummaryLine( *i, cursor, &itemTotalQuantity, &fieldsAmount, printAmounts, writeDetails,
                                       table,
                                       tagBlockFormat, txtBlockFormat, numBlockFormat,
                                       leftFormat, centralFormat, rightFormat );
        }

        if( writeDetails ){
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

        if( printAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
            if( (*i)!= NULL ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToBeDiscountedStr() );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            }
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToBeDiscountedStr() );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
        }

        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
    }

    // *** Totale complessivo ***
    // *** riga dei totali complessivi
    if( printAmounts ){
        // riga vuota
        // AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        // codice
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        // descrizione
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale Lordo") );
        // Udm
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        // quantita'
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        // prezzo + importo
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToBeDiscountedStr() );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        // codice
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        // descrizione
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non ribassabile") );
        // Udm
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        // quantita'
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        // prezzo + importo
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountNotToBeDiscountedStr() );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        // codice
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        // descrizione
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale Netto") );
        // Udm
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        // quantita'
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        // prezzo + importo
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountStr() );
    }

    // *** Riga di chiusura ***
    AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

void AccountingBillItem::writeODTSummaryLine(PriceItem * priceItem,
                                             QTextCursor *cursor,
                                             double * itemTotalQuantity,
                                             QList<double> * fieldsValue,
                                             bool printAmounts,
                                             bool writeDetails,
                                             QTextTable *table,
                                             QTextBlockFormat & tagBlockFormat,
                                             QTextBlockFormat & txtBlockFormat,
                                             QTextBlockFormat & numBlockFormat,
                                             QTextTableCellFormat & leftFormat,
                                             QTextTableCellFormat & centralFormat,
                                             QTextTableCellFormat & rightFormat ) const {
    if( m_d->childrenContainer.size() == 0 ){
        if( priceItem == m_d->priceItem ){
            (*itemTotalQuantity) += quantity();
            fieldsValue[0] += totalAmountToBeDiscounted();
            fieldsValue[1] += amountNotToBeDiscounted();
            fieldsValue[2] += totalAmount();

            if( writeDetails ){
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, progressiveCode() );

                QString unitMeasureTag;
                if( m_d->priceItem != NULL ){
                    if( m_d->priceItem->unitMeasure() != NULL ){
                        unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                    }
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                if( printAmounts ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                } else {
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
                }

                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
            }
        }
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTSummaryLine( priceItem, cursor, itemTotalQuantity, fieldsValue, printAmounts, writeDetails,
                                       table,
                                       tagBlockFormat, txtBlockFormat, numBlockFormat,
                                       leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingBillItem::writeODTAttributeAccountingOnTable(QTextCursor *cursor,
                                                            AccountingPrinter::AttributePrintOption prOption, AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                                            const QList<Attribute *> &attrsToPrint,
                                                            bool printAmounts) const {
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
    if( printAmounts ){
        cellCount += 2;
    }


    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    // *** Intestazione tabella ***
    AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("Codice"), false );
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Denominazione"));
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( printAmounts ){
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));
    } else {
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    }

    // *** riga vuota ***
    AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    QList<double> fieldsAmounts;
    fieldsAmounts << 0.0 << 0.0 << 0.0;

    if( prOption == AccountingPrinter::AttributePrintSimple ){
        // *** Righe del computo suddivise per attributo ***
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            for( QList<double>::iterator j = fieldsAmounts.begin(); j != fieldsAmounts.end(); ++j ){
                *j = 0.0;
            }
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            // cursor->movePosition(QTextCursor::NextCell);

            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, (*i)->name() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            if( printAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            }

            // *** riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            writeODTAttributeBillLineSimple( prItemsOption,
                                             &fieldsAmounts,
                                             *i, printAmounts,
                                             cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                             leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                             txtCharFormat, txtBoldCharFormat );

            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale %1").arg((*i)->name()) );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            if( printAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToBeDiscountedStr() );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            }

            if( i != -- attrsToPrint.end()){
                // *** riga vuota ***
                AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
                // *** riga vuota ***
                AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            }
        }
    } else if( prOption == AccountingPrinter::AttributePrintUnion ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);

        AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        QString title(trUtf8("Unione"));
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            title = QString("%1 %2").arg(title, (*i)->name() );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, title );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        if( printAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario") );
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo") );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        }

        writeODTAttributeBillLineUnion( prItemsOption,
                                        &fieldsAmounts, attrsToPrint, printAmounts,
                                        cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                        leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                        txtCharFormat, txtBoldCharFormat );

        // *** riga vuota ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , title );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( printAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToBeDiscountedStr() );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        }
    } else if( prOption == AccountingPrinter::AttributePrintIntersection ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);

        AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        QString title(trUtf8("Intersezione"));
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            title = QString("%1 %2").arg(title, (*i)->name() );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, title );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        if( printAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario") );
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo") );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        }

        writeODTAttributeBillLineIntersection( prItemsOption,
                                               &fieldsAmounts, attrsToPrint, printAmounts,
                                               cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                               leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                               txtCharFormat, txtBoldCharFormat );

        // *** riga vuota ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , title );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( printAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToBeDiscountedStr() );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        }
    }

    // *** Riga di chiusura ***
    AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

AccountingBillItem::ItemType AccountingBillItem::itemType() const {
    return m_d->itemType;
}

void AccountingBillItem::writeODTAttributeBillLineSimple(AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                                         QList<double> * fieldsAmounts,
                                                         Attribute *attrsToPrint,
                                                         bool printAmounts,
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
                                                         QTextCharFormat & txtBoldCharFormat ) const {

    if( !hasChildren() ){
        if( containsAttribute( attrsToPrint ) ){
            while( fieldsAmounts->size() > 3 ){
                fieldsAmounts->removeLast();
            }
            while( fieldsAmounts->size() < 3 ){
                fieldsAmounts->append( 0.0 );
            }
            fieldsAmounts[0] += totalAmountToBeDiscounted();
            fieldsAmounts[1] += amountNotToBeDiscounted();
            fieldsAmounts[2] += totalAmount();

            writeODTBillLine( prItemsOption,
                              false, printAmounts,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineSimple( prItemsOption,
                                                   fieldsAmounts, attrsToPrint, printAmounts,
                                                   cursor, table,
                                                   tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                   leftFormat, centralFormat, rightFormat,
                                                   centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                   txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingBillItem::writeODTAttributeBillLineUnion( AccountingPrinter::PrintAccountingBillOption prItemsOption,
                                                         QList<double> * fieldsAmounts,
                                                         const QList<Attribute *> &attrsToPrint,
                                                         bool printAmounts,
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
                                                         QTextCharFormat &txtBoldCharFormat) const {
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
            fieldsAmounts[0] += totalAmountToBeDiscounted();
            fieldsAmounts[1] += amountNotToBeDiscounted();
            fieldsAmounts[2] += totalAmount();
            writeODTBillLine( prItemsOption,
                              false, printAmounts,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineUnion( prItemsOption,
                                                  fieldsAmounts, attrsToPrint,
                                                  printAmounts,
                                                  cursor, table,
                                                  tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                  leftFormat, centralFormat, rightFormat,
                                                  centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                  txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingBillItem::writeODTAttributeBillLineIntersection( AccountingPrinter::PrintAccountingBillOption prItemsOption,
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
                                                                QTextCharFormat &txtBoldCharFormat ) const{
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
            fieldsAmounts[0] += totalAmountToBeDiscounted();
            fieldsAmounts[1] += amountNotToBeDiscounted();
            fieldsAmounts[2] += totalAmount();
            writeODTBillLine( prItemsOption,
                              false, printAmounts,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineIntersection( prItemsOption,
                                                         fieldsAmounts, attrsToPrint, printAmounts,
                                                         cursor, table,
                                                         tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                         leftFormat, centralFormat, rightFormat,
                                                         centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                         txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingBillItem::writeODTBillLine( AccountingPrinter::PrintAccountingBillOption prItemsOption,
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
                                           QTextCharFormat & txtBoldCharFormat) const {
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
        AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progressiveCode()  );
    }

    if( m_d->priceItem ){
        if( writeProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->codeFull() );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, m_d->priceItem->codeFull() );
        }
        m_d->writeDescriptionCell( cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prItemsOption );
        // AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->shortDescriptionFull() );
    } else {
        if( writeProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
    }

    if( m_d->measuresModel != NULL ){
        // celle vuote
        // tag unita misura
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

        if( writeAmounts ){
            // quantita'
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            // campi
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
        } else {
            // quantita'
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
        }

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );

        if( writeProgCode ){
            // numero progressivo
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
            // codice
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        } else {
            // codice
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
        }

        // tag unita di misura
        QString unitMeasureTag;
        if( m_d->priceItem ){
            if( m_d->priceItem->unitMeasure()){
                unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
            }
        }

        for( int i=0; i < m_d->measuresModel->billItemMeasureCount(); ++i ){
            BillItemMeasure * measure = m_d->measuresModel->measure(i);

            // formula senza spazi bianchi
            QString realFormula;
            if( measure != NULL ){
                realFormula = measure->formula();
                realFormula.remove(" ");
            }

            // misure
            if( measure != NULL ){
                if( realFormula.isEmpty() ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() );
                } else {
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() + " (" + measure->formula() + ")");
                }
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat);
            }

            if( realFormula.isEmpty() || measure == NULL ){
                // unita di misura
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

                // quantità
                if( writeAmounts ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                } else {
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                }
            } else {
                // unita di misura
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                // quantità
                if( writeAmounts ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->quantityStr() );
                } else {
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, measure->quantityStr() );
                }
            }

            // celle vuote
            if( writeAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
            }

            // inserisce riga
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            if( writeProgCode ){
                // numero progressivo
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                // codice
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            }
        }

        // descrizione breve
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

        // tag unita di misura
        AccountingBillItemPrivate::writeCell( cursor, table,  centralFormat, tagBlockFormat, unitMeasureTag );

        // quantita totale
        if( writeAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, quantityStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToBeDiscountedStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToBeDiscountedStr() );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, rightQuantityTotalFormat, numBlockFormat, quantityStr() );
        }

    } else { // m_d->linesModel == NULL
        QString unitMeasureTag;
        if( m_d->priceItem ){
            if( m_d->priceItem->unitMeasure()){
                unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
            }
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

        if( writeAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToBeDiscountedStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToBeDiscountedStr() );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
        }

    }
}
