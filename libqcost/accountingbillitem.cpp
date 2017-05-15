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

#include "accountingbillitem.h"

#include "accountingbillitemprivate.h"

#include "accountingtambill.h"
#include "accountinglsbills.h"
#include "accountingtambillitem.h"
#include "accountinglsbillitem.h"
#include "accountingpricefieldmodel.h"
#include "accountingprinter.h"
#include "accountingprinter.h"
#include "pricelist.h"
#include "priceitem.h"
#include "measuresmodel.h"
#include "measure.h"
#include "attributesmodel.h"
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
                                       PriceFieldModel * pfm, MathParser * parser , VarsModel *vModel):
    TreeItem(),
    m_d( new AccountingBillItemPrivate(parentItem, iType, pfm, parser, vModel ) ){

    if( parentItem != NULL ){
        connect( parentItem, &AccountingBillItem::attributesChanged, this, &AccountingBillItem::attributesChanged );
        connect( parentItem, &AccountingBillItem::discountChanged, this, &AccountingBillItem::discountChanged );
    }

    connect( this, static_cast<void(AccountingBillItem::*)( AccountingBillItem *, QList<int> )>(&AccountingBillItem::hasChildrenChanged), this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::dateChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::dateBeginChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::dateEndChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::nameChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::textChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::titleChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::priceItemChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::currentPriceDataSetChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::discountChanged, this, &AccountingBillItem::itemChanged );

    if( m_d->itemType == LumpSum ){
        if( m_d->parentItem != NULL ){
            if( m_d->parentItem->m_d->itemType == Payment ){
                connect( m_d->parentItem, &AccountingBillItem::dateBeginChanged, this, &AccountingBillItem::dateBeginChanged );
                connect( m_d->parentItem, &AccountingBillItem::dateEndChanged, this, &AccountingBillItem::dateEndChanged );
            }
        }
    }

    if( m_d->itemType == PPU || m_d->itemType == LumpSum ){
        connect( this, &AccountingBillItem::quantityChanged, this, &AccountingBillItem::updateTotalAmountToDiscount );
        connect( this, &AccountingBillItem::PPUTotalToDiscountChanged, this, &AccountingBillItem::updateTotalAmountToDiscount );
        connect( this, &AccountingBillItem::quantityChanged, this, &AccountingBillItem::updateAmountNotToDiscount );
        connect( this, &AccountingBillItem::PPUNotToDiscountChanged, this, &AccountingBillItem::updateAmountNotToDiscount );
    }
    connect( this, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBillItem::updateAmountToDiscount );
    connect( this, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::updateAmountToDiscount );
    connect( this, &AccountingBillItem::amountToDiscountChanged, this, &AccountingBillItem::updateAmountDiscounted );
    connect( this, &AccountingBillItem::discountChanged, this, &AccountingBillItem::updateAmountDiscounted );
    connect( this, &AccountingBillItem::amountDiscountedChanged, this, &AccountingBillItem::updateTotalAmount );
    connect( this, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::updateTotalAmount );

    connect( this, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBillItem::amountsChanged );
    connect( this, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::amountsChanged );
    connect( this, &AccountingBillItem::amountToDiscountChanged, this, &AccountingBillItem::amountsChanged );
    connect( this, &AccountingBillItem::amountDiscountedChanged, this, &AccountingBillItem::amountsChanged );
    connect( this, &AccountingBillItem::totalAmountChanged, this, &AccountingBillItem::amountsChanged );

    connect( this, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::amountToDiscountChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::amountDiscountedChanged, this, &AccountingBillItem::itemChanged );
    connect( this, &AccountingBillItem::totalAmountChanged, this, &AccountingBillItem::itemChanged );

    if( m_d->totalAmountPriceFieldModel != NULL ){
        connect( m_d->totalAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingBillItem::itemChanged );
        connect( m_d->totalAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingBillItem::updateTotalAmountToDiscount );
    }
    if( m_d->noDiscountAmountPriceFieldModel != NULL ){
        connect( m_d->noDiscountAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingBillItem::itemChanged );
        connect( m_d->noDiscountAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingBillItem::updateAmountNotToDiscount );
    }

    connect( this, &AccountingBillItem::attributesChanged, this, &AccountingBillItem::itemChanged );

    if( m_d->parentItem == NULL ) {
        connect( m_d->noDiscountAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingBillItem::updatePPUs );
        connect( m_d->totalAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingBillItem::updatePPUs );
    }
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

        if( m_d->itemType == Payment ){
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

        if( rootItem() == cp.rootItem() ){
            m_d->attributes.clear();
            m_d->attributes = cp.m_d->attributes;
        } else {
            m_d->attributes.clear();
        }

    }

    return *this;
}

const AccountingBillItem * AccountingBillItem::rootItem() const {
    if( m_d->parentItem == NULL ){
        return this;
    } else {
        return m_d->parentItem->rootItem();
    }
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
            m_d->parentItem->m_d->childrenContainer.insert( position, this );
            m_d->parentItem->m_d->childrenContainer.removeAt( oldPosition );
        }
    }
}

void AccountingBillItem::addChild(AccountingBillItem * newChild, int position ) {
    m_d->childrenContainer.insert( position, newChild );
    connect( newChild, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged), this, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged) );
    connect( newChild, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBillItem::updateTotalAmountToDiscount );
    connect( newChild, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::updateAmountNotToDiscount );
    connect( this, &AccountingBillItem::currentPriceDataSetChanged, newChild, &AccountingBillItem::setCurrentPriceDataSet );
    connect( newChild, &AccountingBillItem::itemChanged, this, &AccountingBillItem::itemChanged );
}

AccountingBillItem *AccountingBillItem::itemFromId( unsigned int itemId ) {
    if( itemId == m_d->id ){
        return this;
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            AccountingBillItem * childItems = (*i)->itemFromId(itemId);
            if( childItems != NULL ) {
                return childItems;
            }
        }
    }
    return NULL;
}

AccountingBillItem *AccountingBillItem::findItemFromId( unsigned int searchitemId ) {
    if( m_d->parentItem == NULL ){
        return itemFromId(searchitemId );
    } else {
        return m_d->parentItem->findItemFromId(searchitemId);
    }
    return NULL;
}

AccountingBillItem *AccountingBillItem::itemFromProgCode( const QString & pCode) {
    if( pCode == fullProgCode() ){
        return this;
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            AccountingBillItem * childItems = (*i)->itemFromProgCode(pCode);
            if( childItems != NULL ) {
                return childItems;
            }
        }
    }
    return NULL;
}

AccountingBillItem *AccountingBillItem::findItemFromProgCode( const QString & pCode ) {
    if( m_d->parentItem == NULL ){
        return itemFromProgCode(pCode);
    } else {
        return m_d->parentItem->findItemFromProgCode(pCode);
    }
    return NULL;
}

AccountingBillItem *AccountingBillItem::childItem(int number) {
    return dynamic_cast<AccountingBillItem *>(child( number ));
}

void AccountingBillItem::setId( unsigned int ii ) {
    m_d->id = ii;
}

VarsModel * AccountingBillItem::varsModel() {
    if( m_d->itemType == Root ){
        return m_d->varsModel;
    } else {
        return m_d->parentItem->varsModel();
    }
}

unsigned int AccountingBillItem::id() {
    return m_d->id;
}

QString AccountingBillItem::accountingProgCode() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == TimeAndMaterials) || (m_d->itemType == LumpSum)){
        return QString::number( m_d->accountingProgCode );
    }
    return QString();
}

void AccountingBillItem::updateAccountingProgCode() {
    int startCode = 1;
    updateAccountingProgCode( &startCode );
}

void AccountingBillItem::updateAccountingProgCode( int * startCode ) {
    QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin();
    if( i == m_d->childrenContainer.end() ){
        if( (m_d->itemType == PPU) ||
                (m_d->itemType == LumpSum) ||
                (m_d->itemType == TimeAndMaterials) ){
            m_d->accountingProgCode = *startCode;
            (*startCode)++;
        }
    } else {
        while( i!= m_d->childrenContainer.end() ){
            (*i)->updateAccountingProgCode( startCode );
            ++i;
        }
    }
}

QString AccountingBillItem::fullProgCode() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == TimeAndMaterials) || (m_d->itemType == LumpSum)){
        QString ret = progCode();
        if( m_d->parentItem != NULL ){
            ret = QString::number( m_d->parentItem->childNumber()+1 ) + "." + ret;
        }
        return ret;
    }
    return QString();
}

QString AccountingBillItem::progCode() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == TimeAndMaterials) || (m_d->itemType == LumpSum)){
        return QString::number( m_d->progCode );
    }
    return QString();
}

void AccountingBillItem::updateProgCode() {
    for( QList<AccountingBillItem *>::iterator pay = m_d->childrenContainer.begin();
         pay != m_d->childrenContainer.end(); ++pay ){
        int startCode = 1;
        (*pay)->updateProgCode( &startCode );
    }
}

void AccountingBillItem::updateProgCode( int * startCode ) {
    QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin();
    if( i == m_d->childrenContainer.end() ){
        if( (m_d->itemType == PPU) ||
                (m_d->itemType == LumpSum) ||
                (m_d->itemType == TimeAndMaterials) ){
            m_d->progCode = *startCode;
            (*startCode)++;
        }
    }
    while( i!= m_d->childrenContainer.end() ){
        (*i)->updateProgCode( startCode );
        ++i;
    }
}

QList<int> AccountingBillItem::totalAmountPriceFields() const {
    if( m_d->itemType == Root ){
        return *(m_d->totalAmountPriceFieldsList);
    } else if( m_d->parentItem != NULL ){
        return m_d->parentItem->totalAmountPriceFields();
    }
    return QList<int>();
}


void AccountingBillItem::setTotalAmountPriceFields(const QList<int> &newAmountFields) {
    if( m_d->itemType == Root ){
        m_d->totalAmountPriceFieldModel->setPriceFields( newAmountFields );
    }
}


AccountingPriceFieldModel *AccountingBillItem::totalAmountPriceFieldModel() {
    return m_d->totalAmountPriceFieldModel;
}

QList<int> AccountingBillItem::noDiscountAmountPriceFields() const {
    if( m_d->itemType == Root ){
        return *(m_d->noDiscountAmountPriceFieldsList);
    } else if( m_d->parentItem != NULL ){
        return m_d->parentItem->noDiscountAmountPriceFields();
    }
    return QList<int>();
}

void AccountingBillItem::setNoDiscountAmountPriceFields(const QList<int> &newAmountFields) {
    if( m_d->itemType == Root ){
        m_d->noDiscountAmountPriceFieldModel->setPriceFields( newAmountFields );
    }
}

void AccountingBillItem::updatePPUs() {
    if( hasChildren() ){
        for( QList<AccountingBillItem *>::iterator i=m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->updatePPUs();
        }
    } else {
        if( m_d->itemType == PPU ){
            double newPPUTotalToDiscount = 0.0;
            double newPPUNotToDiscount = 0.0;
            if( m_d->priceItem != NULL ){
                QList<int> totAmPriceFields = totalAmountPriceFields();
                for( QList<int>::iterator i = totAmPriceFields.begin(); i != totAmPriceFields.end(); ++i){
                    newPPUTotalToDiscount += m_d->priceItem->value( (*i), currentPriceDataSet() );
                }
                newPPUTotalToDiscount = UnitMeasure::applyPrecision( newPPUTotalToDiscount, m_d->amountPrecision );
                QList<int> noDiscAmPriceFields = noDiscountAmountPriceFields();
                for( QList<int>::iterator i = noDiscAmPriceFields.begin(); i != noDiscAmPriceFields.end(); ++i){
                    newPPUNotToDiscount += m_d->priceItem->value( (*i), currentPriceDataSet() );
                }
                newPPUNotToDiscount = UnitMeasure::applyPrecision( newPPUNotToDiscount, m_d->amountPrecision );
            }
            if( newPPUTotalToDiscount != m_d->PPUTotalToDiscount ){
                m_d->PPUTotalToDiscount = newPPUTotalToDiscount;
                emit PPUTotalToDiscountChanged( PPUTotalToDiscountStr() );
            }
            if( newPPUNotToDiscount != m_d->PPUNotToDiscount ){
                m_d->PPUNotToDiscount = newPPUNotToDiscount;
                emit PPUNotToDiscountChanged( PPUNotToDiscountStr() );
            }
        } else if( m_d->itemType == LumpSum ){
            double newPPUTotalToDiscount = 0.0;
            double newPPUNotToDiscount = 0.0;
            if( m_d->lsBill != NULL ){
                newPPUTotalToDiscount = m_d->lsBill->PPUTotalToDiscount();
                newPPUNotToDiscount = m_d->lsBill->PPUNotToDiscount();
            }
            if( newPPUTotalToDiscount != m_d->PPUTotalToDiscount ){
                m_d->PPUTotalToDiscount = newPPUTotalToDiscount;
                emit PPUTotalToDiscountChanged( PPUTotalToDiscountStr() );
            }
            if( newPPUNotToDiscount != m_d->PPUNotToDiscount ){
                m_d->PPUNotToDiscount = newPPUNotToDiscount;
                emit PPUNotToDiscountChanged( PPUNotToDiscountStr() );
            }
        }
    }
}

AccountingPriceFieldModel *AccountingBillItem::noDiscountAmountPriceFieldModel() {
    return m_d->noDiscountAmountPriceFieldModel;
}

void AccountingBillItem::requestDateBeginChange(const QString &newDateStr) {
    QDate newDate;
    if( m_d->parser != NULL ){
        newDate = m_d->parser->evaluateDate(newDateStr, QLocale::NarrowFormat);
    } else {
        newDate = QDate::fromString(newDateStr, Qt::DefaultLocaleShortDate);
    }

    requestDateBeginChange( newDate);
}

void AccountingBillItem::requestDateBeginChange(const QDate &newDate) {
    emit requestDateBeginChangeSignal( newDate, childNumber() );
}

void AccountingBillItem::requestDateEndChange(const QString &newDateStr) {
    QDate newDate;
    if( m_d->parser != NULL ){
        newDate = m_d->parser->evaluateDate(newDateStr, QLocale::NarrowFormat);
    } else {
        newDate = QDate::fromString(newDateStr, Qt::DefaultLocaleShortDate);
    }

    requestDateEndChange( newDate );
}

void AccountingBillItem::requestDateEndChange(const QDate &newDate) {
    emit requestDateEndChangeSignal( newDate, childNumber() );
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
    if( m_d->itemType == LumpSum ){
        if( m_d->parentItem != NULL ){
            if( m_d->parentItem->m_d->itemType == Payment ){
                return m_d->parentItem->m_d->date;
            }
        }
    }
    return m_d->date;
}

QString AccountingBillItem::dateBeginStr() const {
    if( (m_d->itemType == Payment) || (m_d->itemType == LumpSum) ){
        if( m_d->parser != NULL ){
            return m_d->parser->toString( dateBegin(), QLocale::NarrowFormat );
        }
        return dateBegin().toString();
    } else if( m_d->itemType == TimeAndMaterials && m_d->tamBillItem != NULL ){
        return m_d->tamBillItem->dateBeginStr();
    }
    return QString();
}

void AccountingBillItem::setDateBegin(const QDate &d) {
    if( d.isValid() ){
        if( m_d->itemType == Payment ){
            if( m_d->date != d ){
                m_d->date = d;
                emit dateBeginChanged( dateBeginStr() );
            }
        } else if( m_d->itemType == TimeAndMaterials && m_d->tamBillItem != NULL ){
            m_d->tamBillItem->setDateBegin(d);
            emit dateBeginChanged( dateBeginStr() );
        } else if( m_d->itemType == LumpSum ){
            if( m_d->parentItem != NULL ){
                if( m_d->parentItem->m_d->itemType == Payment ){
                    m_d->parentItem->setDateBegin( d );
                }
            }
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
    if( m_d->itemType == LumpSum ){
        if( m_d->parentItem != NULL ){
            if( m_d->parentItem->m_d->itemType == Payment ){
                return m_d->parentItem->m_d->dateEnd;
            }
        }
    }
    return m_d->dateEnd;
}

QString AccountingBillItem::dateEndStr() const {
    if( (m_d->itemType == Payment) || (m_d->itemType == LumpSum) ){
        if( m_d->parser != NULL ){
            return m_d->parser->toString( dateEnd(), QLocale::NarrowFormat );
        }
        return dateEnd().toString();
    } else if( m_d->itemType == TimeAndMaterials && m_d->tamBillItem != NULL ){
        return m_d->tamBillItem->dateEndStr();
    }
    return QString();
}

void AccountingBillItem::setDateEnd(const QDate &d) {
    if( d.isValid() ){
        if( m_d->itemType == Payment ){
            if( m_d->dateEnd != d ){
                m_d->dateEnd = d;
                emit dateEndChanged( dateEndStr() );
            }
        } else if( m_d->itemType == TimeAndMaterials && m_d->tamBillItem != NULL ){
            m_d->tamBillItem->setDateEnd( d );
            emit dateEndChanged( dateEndStr() );
        } else if( m_d->itemType == LumpSum ){
            if( m_d->parentItem != NULL ){
                if( m_d->parentItem->m_d->itemType == Payment ){
                    m_d->parentItem->setDateEnd( d );
                }
            }
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
            return Qt::AlignHCenter + Qt::AlignVCenter;
        } else if(role == Qt::DisplayRole || role == Qt::EditRole){
            return QVariant();
        }
    } else if( m_d->itemType == PPU ){
        if( col == m_d->progNumberCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( progCode() );
            }
        } else if( col == m_d->dateCol) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( dateStr() );
            }
        } else if( col == m_d->priceCodeCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                if( m_d->priceItem != NULL ){
                    return QVariant( m_d->priceItem->codeFull() );
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceShortDescCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                if( m_d->priceItem != NULL ){
                    return QVariant(m_d->priceItem->shortDescriptionFull());
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceUmCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
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
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( quantityStr() );
            }
        } else if( col == m_d->PPUTotalToDiscountCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( PPUTotalToDiscountStr() );
            }
        } else if( col == m_d->totalAmountToDiscountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( totalAmountToDiscountStr() );
            }
        } else if( col == m_d->PPUNotToDiscountCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( PPUNotToDiscountStr() );
            }
        } else if( col == (m_d->amountNotToDiscountCol) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( amountNotToDiscountStr() );
            }
        } else if( col == m_d->totalAmountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( totalAmountStr() );
            }
        } else {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        }
    } else if( m_d->itemType == LumpSum ){
        if( col == m_d->progNumberCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( progCode() );
            }
        } else if( col == m_d->dateCol) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( dateStr() );
            }
        } else if( col == m_d->priceCodeCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                if( m_d->lsBill != NULL ){
                    return QVariant( m_d->lsBill->code() );
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceShortDescCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                if( m_d->lsBill != NULL ){
                    return QVariant(m_d->lsBill->name());
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceUmCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("%") );
            }
        } else if( col == m_d->quantityCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( quantityStr() );
            }
        } else if( col == m_d->PPUTotalToDiscountCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( PPUTotalToDiscountStr() );
            }
        } else if( col == m_d->totalAmountToDiscountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( totalAmountToDiscountStr() );
            }
        } else if( col == m_d->PPUNotToDiscountCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( PPUNotToDiscountStr() );
            }
        } else if( col == m_d->amountNotToDiscountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( amountNotToDiscountStr() );
            }
        } else if( col == m_d->totalAmountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( totalAmountStr() );
            }
        } else {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        }
    } else if( m_d->itemType == TimeAndMaterials ){
        if( col == m_d->progNumberCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( progCode() );
            }
        } else if( col == m_d->dateCol) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( dateStr() );
            }
        } else if( col == m_d->priceCodeCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        } else if( col == m_d->priceShortDescCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                if( m_d->tamBillItem != NULL ){
                    return QVariant(m_d->tamBillItem->title() );
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceUmCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        } else if( col == m_d->quantityCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        } else if( col == m_d->PPUTotalToDiscount ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        } else if( col == m_d->totalAmountToDiscountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( totalAmountToDiscountStr() );
            }
        } else if( col == m_d->PPUNotToDiscountCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        } else if( col == (m_d->amountNotToDiscountCol) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( amountNotToDiscountStr() );
            }
        } else if( col == m_d->totalAmountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( totalAmountStr() );
            }
        } else {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        }
    } else if( m_d->itemType == Payment ){
        if( role == Qt::FontRole ){
            QFont font;
            font.setBold( true );
            return font;
        }
        if( col == m_d->priceShortDescCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( title() );
            }
        } else if( col == m_d->totalAmountToDiscountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( totalAmountToDiscountStr() );
            }
        } else if( col == m_d->amountNotToDiscountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( amountNotToDiscountStr() );
            }
        } else if( col == ( m_d->totalAmountCol ) ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignRight + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( totalAmountStr() );
            }
        } else {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant();
            }
        }
    } else if( m_d->itemType == Root ){
        if( col == m_d->progNumberCol) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("N.") );
            }
        } else if( col == m_d->dateCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("Data") );
            }
        } else if( col == m_d->priceCodeCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("Codice") );
            }
        } else if( col == m_d->priceShortDescCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant(QObject::trUtf8("Descrizione") );
            }
        }  else if( col == m_d->priceUmCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("UdM") );
            }
        } else if( col == m_d->quantityCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("Quantità") );
            }
        } else if( col == m_d->PPUTotalToDiscountCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("C.U. lordo") );
            }
        } else if( col == m_d->totalAmountToDiscountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("Imp. lordo") );
            }
        } else if( col == m_d->PPUNotToDiscountCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("C.U. non rib.") );
            }
        } else if( col == m_d->amountNotToDiscountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("Imp. non rib.") );
            }
        } else if( col == m_d->totalAmountCol ){
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("Importo") );
            }
        } else {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
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
        if( column == m_d->quantityCol && m_d->measuresModel == NULL ){
            setQuantity( value.toString() );
            emit dataChanged( this, m_d->PPUTotalToDiscountCol);
            emit dataChanged( this, m_d->totalAmountToDiscountCol);
            emit dataChanged( this, m_d->PPUNotToDiscountCol);
            emit dataChanged( this, m_d->amountNotToDiscountCol);
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

QString AccountingBillItem::discountStr() const {
    return QString("%1 %").arg( m_d->toString( discount() * 100.0, m_d->discountPrecision - 2 ) );
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
    updatePPUs();
}

void AccountingBillItem::setDiscount(double newValPurp ) {
    if( m_d->itemType == Root ){
        double newVal = UnitMeasure::applyPrecision( newValPurp, m_d->discountPrecision );
        if( newVal != m_d->discount ){
            m_d->discount = newVal;
            emit discountChanged( discountStr() );
        }
    } else if( m_d->parentItem ){
        m_d->parentItem->setDiscount( newValPurp );
    }
}

void AccountingBillItem::setDiscount(const QString &newVal) {
    QString v = newVal;
    v.remove("%");
    if( m_d->parser != NULL ){
        setDiscount( m_d->parser->evaluate( v ) / 100.0 );
    } else {
        setDiscount( v.toDouble() / 100.0 );
    }
}

void AccountingBillItem::updateTotalAmountToDiscount() {
    double v = 0.0;
    if( (m_d->itemType == Root) || (m_d->itemType == Payment)){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->updateTotalAmountToDiscount();
            v += (*i)->totalAmountToDiscount();
        }
    } else if( (m_d->itemType == PPU) || (m_d->itemType == LumpSum) ) {
        v = UnitMeasure::applyPrecision( m_d->PPUTotalToDiscount * m_d->quantity, m_d->amountPrecision );
    } else if( m_d->itemType == TimeAndMaterials ) {
        if( m_d->tamBillItem != NULL ){
            v = m_d->tamBillItem->totalAmountToDiscount();
        }
    } else if(m_d->itemType == Comment) {
        v = 0.0;
    }
    if( v != m_d->totalAmountToDiscount ){
        m_d->totalAmountToDiscount = v;
        emit totalAmountToDiscountChanged( totalAmountToDiscountStr() );
    }
}

void AccountingBillItem::updateAmountNotToDiscount() {
    double v = 0.0;
    if( (m_d->itemType == Root) || (m_d->itemType == Payment)){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->updateAmountNotToDiscount();
            v += (*i)->amountNotToDiscount();
        }
    } else if( (m_d->itemType == PPU) || (m_d->itemType == LumpSum) ) {
        v = UnitMeasure::applyPrecision( m_d->PPUNotToDiscount * m_d->quantity, m_d->amountPrecision );
    } else if( m_d->itemType == TimeAndMaterials ) {
        if( m_d->tamBillItem != NULL ){
            v = m_d->tamBillItem->amountNotToDiscount();
        }
    } else if(m_d->itemType == Comment) {
        v = 0.0;
    }
    if( v != m_d->amountNotToDiscount ){
        m_d->amountNotToDiscount = v;
        emit amountNotToDiscountChanged( amountNotToDiscountStr() );
    }
}

void AccountingBillItem::updateAmountToDiscount() {
    double v = m_d->totalAmountToDiscount - m_d->amountNotToDiscount;
    if( v != m_d->amountToDiscount ){
        m_d->amountToDiscount = v;
        emit amountToDiscountChanged( amountToDiscountStr() );
    }
}

void AccountingBillItem::updateAmountDiscounted() {
    double v = UnitMeasure::applyPrecision( m_d->amountToDiscount * (1.0 - discount() ), m_d->amountPrecision );;
    if( v != m_d->amountDiscounted ){
        m_d->amountDiscounted = v;
        emit amountDiscountedChanged( amountDiscountedStr() );
    }
}

void AccountingBillItem::updateTotalAmount() {
    double v = m_d->amountDiscounted + m_d->amountNotToDiscount;
    if( v != m_d->totalAmount ) {
        m_d->totalAmount = v;
        emit totalAmountChanged( totalAmountStr() );
    }
}

bool AccountingBillItem::isUsingPriceItem(PriceItem *p) {
    if( (m_d->itemType == Root) || (m_d->itemType == Payment) ){
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
    if( (m_d->itemType == Root) || (m_d->itemType == Payment)){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->appendUsedPriceItems( usedPriceItems );
        }
    } else if( m_d->itemType == PPU ){
        if( !usedPriceItems->contains( m_d->priceItem )){
            usedPriceItems->append( m_d->priceItem );
        }
    }
}

QList<AccountingBillItem *> AccountingBillItem::usedItemsPayment( int firstPay, int lastPay, ItemType iType ) const {
    QList<AccountingBillItem *> ret;
    if( m_d->itemType == Root ){
        if( firstPay < 0 ){
            firstPay = 0;
        }
        if( lastPay >= m_d->childrenContainer.size() ){
            lastPay = m_d->childrenContainer.size() - 1;
        }
        for( int i=firstPay; i <= lastPay; i++){
            for( QList<AccountingBillItem *>::iterator line = m_d->childrenContainer.at(i)->m_d->childrenContainer.begin(); line != m_d->childrenContainer.at(i)->m_d->childrenContainer.end(); ++line){
                if( (*line)->itemType() == iType &&
                        !(ret.contains( (*line) ) ) ){
                    ret << (*line);
                }
            }
        }
    }
    return ret;
}

QList<AccountingLSBill *> AccountingBillItem::usedLSBillsPayment( int firstPay, int lastPay ) const {
    QList<AccountingLSBill *> ret;
    if( m_d->itemType == Root ){
        if( firstPay < 0 ){
            firstPay = 0;
        }
        if( lastPay >= m_d->childrenContainer.size() ){
            lastPay = m_d->childrenContainer.size() - 1;
        }
        for( int i=firstPay; i <= lastPay; i++){
            for( QList<AccountingBillItem *>::iterator line = m_d->childrenContainer.at(i)->m_d->childrenContainer.begin(); line != m_d->childrenContainer.at(i)->m_d->childrenContainer.end(); ++ line ){
                if( (*line)->itemType() == LumpSum) {
                    AccountingLSBill * assLSBill = (*line)->lsBill();
                    if( !(ret.contains(assLSBill) ) && (assLSBill != NULL) ){
                        ret << assLSBill;
                    }
                }
            }
        }
    }
    return ret;
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
        return insertChildren( Payment, position, count );
    }
    if( m_d->itemType == Payment ){
        return insertChildren( PPU, position, count );
    }
    return false;
}

bool AccountingBillItem::insertChildren(AccountingBillItem::ItemType iType, int position, int count ){
    if (position < 0 || position > m_d->childrenContainer.size() )
        return false;

    if( (m_d->itemType == Root && (iType == Payment) ) ||
            (m_d->itemType == Payment && (iType == PPU || iType == Comment || iType == TimeAndMaterials || iType == LumpSum )) ){

        bool hadChildren = m_d->childrenContainer.size() > 0;

        for (int row = 0; row < count; ++row) {
            AccountingBillItem * item = new AccountingBillItem( this, iType,  m_d->priceFieldModel, m_d->parser );
            if( iType != Payment && iType != Root ){
                while( findItemFromId( item->id() ) != NULL ){
                    item->setId( item->id() + 1 );
                }
            }
            m_d->childrenContainer.insert(position, item);
            if( iType == Payment ){
                for( int j = position+1; j < m_d->childrenContainer.size(); ++j  ){
                    m_d->childrenContainer.at(j)->emitTitleChanged();
                }
            }
            connect( item, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged), this, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged) );
            connect( item, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBillItem::updateTotalAmountToDiscount );
            connect( item, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::updateAmountNotToDiscount );
            connect( this, &AccountingBillItem::currentPriceDataSetChanged, item, &AccountingBillItem::setCurrentPriceDataSet );
            connect( item, &AccountingBillItem::itemChanged, this, &AccountingBillItem::itemChanged );
        }

        if( !hadChildren ){
            if( m_d->childrenContainer.size() > 0 ){
                emit hasChildrenChanged( true );
            }
        }

        if( (m_d->itemType == Root) && (iType == Payment) ){
            for( int i=position; i < (position+count); ++i ){
                emit paymentInserted( i, m_d->childrenContainer.at(i));
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
    if( count <= 0 ){
        return true;
    }

    if (position < 0 || position + count > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row){
        AccountingBillItem * item = m_d->childrenContainer.at( position );
        disconnect( item, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged), this, static_cast<void(AccountingBillItem::*)(AccountingBillItem*,int)> (&AccountingBillItem::dataChanged) );
        disconnect( item, &AccountingBillItem::totalAmountToDiscountChanged, this, &AccountingBillItem::updateTotalAmountToDiscount );
        disconnect( item, &AccountingBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::updateAmountNotToDiscount );
        disconnect( this, &AccountingBillItem::currentPriceDataSetChanged, item, &AccountingBillItem::setCurrentPriceDataSet );
        disconnect( item, &AccountingBillItem::itemChanged, this, &AccountingBillItem::itemChanged );
        delete item;
        AccountingBillItem * itemRemoved = m_d->childrenContainer.at(position);
        m_d->childrenContainer.removeAt( position );
        if( itemRemoved->itemType() == Payment ){
            emit paymentRemoved( position, itemRemoved );
        }
    }
    if( hadChildren ){
        if( !(m_d->childrenContainer.size() > 0) ){
            emit hasChildrenChanged( false );
        }
    }

    emit itemChanged();

    return true;
}

bool AccountingBillItem::clear() {
    if( m_d->itemType == Root ){
        setDiscount( 0.0 );
        setName( "" );
        QList<int> emptyList;
        setTotalAmountPriceFields( emptyList );
        setNoDiscountAmountPriceFields( emptyList );
        setCurrentPriceDataSet( 0 );
    }
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

void AccountingBillItem::writeXml20(QXmlStreamWriter *writer) {

    if( m_d->itemType == Root ){
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml20( writer );
        }
    } else if( m_d->itemType == Payment ){
        writer->writeStartElement( "AccountingBillItem" );
        writer->writeAttribute( "itemType", QString("Payment") );
        QString attrs = m_d->attributesString();
        if( !attrs.isEmpty() ){
            writer->writeAttribute( "attributes", attrs );
        }
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml20( writer );
        }
        writer->writeEndElement();
    } else if( m_d->itemType == PPU ){
        writer->writeStartElement( "AccountingBillItem" );
        writer->writeAttribute( "itemType", QString("PPU") );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "date", QString::number( m_d->date.toJulianDay() ) );
        if( m_d->priceItem != NULL ){
            writer->writeAttribute( "priceItem", QString::number( m_d->priceItem->id() ) );
        }
        QString attrs = m_d->attributesString();
        if( !attrs.isEmpty() ){
            writer->writeAttribute( "attributes", attrs );
        }

        if( m_d->measuresModel != NULL ){
            m_d->measuresModel->writeXml20( writer );
        } else {
            writer->writeAttribute( "quantity", QString::number( m_d->quantity ) );
        }

        writer->writeEndElement();
    } else if( m_d->itemType == LumpSum ){
        writer->writeStartElement( "AccountingBillItem" );
        writer->writeAttribute( "itemType", QString("LumpSum") );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "date", QString::number( m_d->date.toJulianDay() ) );
        if( m_d->lsBill != NULL ){
            writer->writeAttribute( "lumpSumBill", QString::number(m_d->lsBill->id()) );
        }
        QString attrs = m_d->attributesString();
        if( !attrs.isEmpty() ){
            writer->writeAttribute( "attributes", attrs );
        }
        writer->writeEndElement();
    } else if( m_d->itemType == TimeAndMaterials ){
        writer->writeStartElement( "AccountingBillItem" );
        writer->writeAttribute( "itemType", QString("TimeAndMaterials") );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "date", QString::number( m_d->date.toJulianDay() ) );
        if( m_d->tamBillItem != NULL ){
            writer->writeAttribute( "timeAndMaterials", QString::number(m_d->tamBillItem->id()) );
        }
        QString attrs = m_d->attributesString();
        if( !attrs.isEmpty() ){
            writer->writeAttribute( "attributes", attrs );
        }
        writer->writeEndElement();
    } else if( m_d->itemType == Comment ){
        writer->writeStartElement( "AccountingBillItem" );
        writer->writeAttribute( "itemType", QString("Comment") );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "text", m_d->text );
        writer->writeEndElement();
    }
}

void AccountingBillItem::readXmlTmp20( QXmlStreamReader *reader ) {
    QString tagUp = reader->name().toString().toUpper();

    if( m_d->itemType != Root ){
        if(reader->isStartElement() && tagUp == "ACCOUNTINGBILLITEM"){
            m_d->tmpAttributes.clear();
            m_d->tmpAttributes = reader->attributes();
        }
        reader->readNext();
    }

    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && tagUp == "ACCOUNTINGBILLITEM")&&
           !(reader->isEndElement() && tagUp == "ACCOUNTINGBILL")  ){
        if( m_d->itemType == Root ){
            if( tagUp == "ACCOUNTINGBILLITEM" && reader->isStartElement()) {
                if( reader->attributes().hasAttribute( "itemType" ) ){
                    if( reader->attributes().value( "itemType" ).toString().toUpper() == "PAYMENT" ){
                        appendChildren( Payment );
                        m_d->childrenContainer.last()->readXmlTmp20( reader );
                    }
                }
            }
        } else if( m_d->itemType == Payment ){
            if( tagUp == "ACCOUNTINGBILLITEM" && reader->isStartElement()) {
                if( reader->attributes().hasAttribute( "itemType" ) ){
                    AccountingBillItem::ItemType iType = PPU;
                    if( reader->attributes().value( "itemType" ).toString().toUpper() == "COMMENT" ){
                        iType = Comment;
                    } else if( reader->attributes().value( "itemType" ).toString().toUpper() == "PPU" ){
                        iType = PPU;
                    } else if( reader->attributes().value( "itemType" ).toString().toUpper() == "TIMEANDMATERIALS" ){
                        iType = TimeAndMaterials;
                    } else if( reader->attributes().value( "itemType" ).toString().toUpper() == "LUMPSUM" ){
                        iType = LumpSum;
                    }
                    appendChildren( iType );
                    m_d->childrenContainer.last()->readXmlTmp20( reader );
                }
            }
        }  else if( m_d->itemType == PPU ){
            if( reader->name().toString().toUpper() == "MEASURESMODEL" && reader->isStartElement() ) {
                generateMeasuresModel()->readXmlTmp20( reader );
            }
        }
        reader->readNext();
        tagUp = reader->name().toString().toUpper();
    }
}

void AccountingBillItem::readFromXmlTmp20( AccountingLSBills * lsBills, AccountingTAMBill * tamBill ,PriceList * priceList, AttributesModel * billAttrModel ) {
    if( !m_d->tmpAttributes.isEmpty() ){
        loadFromXml( m_d->tmpAttributes, lsBills, tamBill, priceList, billAttrModel );
        m_d->tmpAttributes.clear();
        if( m_d->measuresModel != NULL ){
            m_d->measuresModel->readFromXmlTmp20();
        }
    }
    for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->readFromXmlTmp20( lsBills, tamBill, priceList, billAttrModel );
    }
}

void AccountingBillItem::loadFromXml( const QXmlStreamAttributes &attrs,
                                      AccountingLSBills * lsBills,
                                      AccountingTAMBill * tamBill,
                                      PriceList * priceList,
                                      AttributesModel * attrModel ) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "date" ) ){
        m_d->date = QDate::fromJulianDay( attrs.value( "date").toString().toLongLong() );
    }
    if( attrs.hasAttribute( "dateBegin" ) ){
        m_d->date = QDate::fromJulianDay( attrs.value( "dateBegin").toString().toLongLong() );
    }
    if( attrs.hasAttribute( "dateEnd" ) ){
        m_d->date = QDate::fromJulianDay( attrs.value( "dateEnd").toString().toLongLong() );
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
                Attribute * attr = attrModel->attributeId( attrId );
                if( attr != NULL ) {
                    addAttribute( attr );
                }
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
    } else if( m_d->itemType == LumpSum ){
        if( attrs.hasAttribute("lumpSumBill") ){
            setLSBill( lsBills->billId( attrs.value("lumpSumBill").toUInt() ) );
        }
    }
    if( m_d->itemType == TimeAndMaterials ){
        if( attrs.hasAttribute("timeAndMaterials") ){
            setTAMBillItem( tamBill->itemId( attrs.value("timeAndMaterials").toUInt() ) );
        }
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

double AccountingBillItem::totalAmountToDiscountAttribute(Attribute *attr ) const {
    if( m_d->childrenContainer.size() == 0 ){
        QList<Attribute *> attrs = inheritedAttributes();
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( !attrs.contains( *i) ){
                attrs.append( *i );
            }
        }
        if( attrs.contains(attr) ){
            return totalAmountToDiscount();
        }
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmountToDiscountAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingBillItem::totalAmountToDiscountAttributeStr(Attribute *attr ) const {
    int prec = m_d->amountPrecision;
    QString ret;
    double v = totalAmountToDiscountAttribute( attr );
    if( m_d->parser == NULL ){
        ret = QString::number(v, 'f', prec );
    } else {
        ret = m_d->parser->toString( v, 'f', prec );
    }
    return ret;
}

double AccountingBillItem::amountNotToDiscountAttribute(Attribute *attr ) const {
    if( m_d->childrenContainer.size() == 0 ){
        QList<Attribute *> attrs = inheritedAttributes();
        for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
            if( !attrs.contains( *i) ){
                attrs.append( *i );
            }
        }
        if( attrs.contains(attr) ){
            return amountNotToDiscount();
        }
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountNotToDiscountAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingBillItem::amountNotToDiscountAttributeStr(Attribute *attr ) const {
    int prec = m_d->amountPrecision;
    QString ret;
    double v = amountNotToDiscountAttribute( attr );
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
            (m_d->itemType == AccountingBillItem::Payment)   ){
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

void AccountingBillItem::appendConnectedItems(QList<AccountingBillItem *> *itemsList) {
    // aggiunge l'item corrente e tutti i suoi figli, se non presenti
    if( !(itemsList->contains(this)) ){
        itemsList->append(this);
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->appendConnectedItems(itemsList);
        }
    }

    // aggiunge tutti gli item connessi tramite le misure, se non presenti
    QList<AccountingBillItem *> connItems = m_d->measuresModel->connectedAccBillItems();
    for( QList<AccountingBillItem *>::iterator i = connItems.begin(); i != connItems.end(); ++i ){
        if( !(itemsList->contains(*i)) ){
            (*i)->appendConnectedItems(itemsList);
        }
    }
}

void AccountingBillItem::setLSBill(AccountingLSBill *newLSBill) {
    if( m_d->lsBill != newLSBill ){
        if( m_d->lsBill != NULL ){
            disconnect( m_d->lsBill, &AccountingLSBill::modelChanged, this, &AccountingBillItem::updateLSQuantity );
            disconnect( m_d->lsBill, &AccountingLSBill::PPUTotalToDiscountChanged, this, &AccountingBillItem::updatePPUs );
            disconnect( m_d->lsBill, &AccountingLSBill::PPUNotToDiscountChanged, this, &AccountingBillItem::updatePPUs );
            disconnect( m_d->lsBill, &AccountingLSBill::aboutToBeDeleted, this, &AccountingBillItem::setLSBillNULL );
        }
        m_d->lsBill = newLSBill;
        updateLSQuantity();
        updatePPUs();
        if( m_d->lsBill != NULL ){
            connect( m_d->lsBill, &AccountingLSBill::modelChanged, this, &AccountingBillItem::updateLSQuantity );
            connect( m_d->lsBill, &AccountingLSBill::PPUTotalToDiscountChanged, this, &AccountingBillItem::updatePPUs );
            connect( m_d->lsBill, &AccountingLSBill::PPUNotToDiscountChanged, this, &AccountingBillItem::updatePPUs );
            connect( m_d->lsBill, &AccountingLSBill::aboutToBeDeleted, this, &AccountingBillItem::setLSBillNULL );
        }
        emit lsBillChanged( m_d->lsBill );
    }
}

void AccountingBillItem::updateLSQuantity() {
    if( m_d->itemType == AccountingBillItem::LumpSum ) {
        double newQuantity = 0.0;
        if(m_d->lsBill != NULL){
            if( m_d->parentItem != NULL ){
                if( m_d->parentItem->itemType() == AccountingBillItem::Payment ){
                    newQuantity = m_d->lsBill->percentageAccounted( m_d->parentItem->dateBegin(), m_d->parentItem->dateEnd() );
                }
            }
        }
        if( newQuantity != m_d->quantity ){
            m_d->quantity = newQuantity;
            emit quantityChanged( quantityStr() );
        }
    }
}

void AccountingBillItem::setLSBillNULL() {
    setLSBill( NULL );
}

AccountingLSBill *AccountingBillItem::lsBill() {
    return m_d->lsBill;
}

void AccountingBillItem::setTAMBillItem(AccountingTAMBillItem *newTAMBillItem) {
    if( m_d->tamBillItem != newTAMBillItem ){
        if( m_d->tamBillItem != NULL ){
            disconnect( m_d->tamBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingBillItem::updateTotalAmountToDiscount );
            disconnect( m_d->tamBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::updateAmountNotToDiscount );
            disconnect( m_d->tamBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingBillItem::setTAMBillItemNULL );
        }
        m_d->tamBillItem = newTAMBillItem;
        if( m_d->tamBillItem != NULL ){
            connect( m_d->tamBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingBillItem::updateTotalAmountToDiscount );
            connect( m_d->tamBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingBillItem::updateAmountNotToDiscount );
            connect( m_d->tamBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingBillItem::setTAMBillItemNULL );
        }
        emit tamBillItemChanged( m_d->tamBillItem );
        emit dateBeginChanged( dateBeginStr() );
        emit dateEndChanged( dateEndStr() );
        updateTotalAmountToDiscount();
        updateAmountToDiscount();
    }
}

void AccountingBillItem::setTAMBillItemNULL() {
    setTAMBillItem( NULL );
}

AccountingTAMBillItem *AccountingBillItem::tamBillItem() {
    return m_d->tamBillItem;
}

QDate AccountingBillItem::date() const {
    return m_d->date;
}

QString AccountingBillItem::dateStr() const {
    if( (m_d->itemType == Root) ||
            (m_d->itemType == Comment) ||
            (m_d->itemType == Payment) ){
        return QString();
    }
    if( m_d->parser != NULL ){
        return m_d->parser->toString( m_d->date, QLocale::NarrowFormat );
    }
    return m_d->date.toString();
}

double AccountingBillItem::totalAmountToDiscount() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == LumpSum) ||
            (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->totalAmountToDiscount;
    } else if( (m_d->itemType == TimeAndMaterials) && (m_d->tamBillItem != NULL) ){
        return m_d->tamBillItem->totalAmountToDiscount();
    }
    return 0.0;
}

double AccountingBillItem::totalAmountToDiscount(AccountingBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return totalAmountToDiscount();
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmountToDiscount( iType );
        }
        return ret;
    }
    return 0.0;
}

double AccountingBillItem::totalAmountToDiscountPayment(int firstPay, int lastPay, AccountingBillItem::ItemType iType) const {
    double ret = 0.0;
    if( m_d->itemType == Root ){
        if( firstPay < 0 ){
            firstPay = 0;
        }
        if( lastPay >= m_d->childrenContainer.size() ){
            lastPay = m_d->childrenContainer.size() - 1;
        }
        for( int i=firstPay; i <= lastPay; i++){
            ret += m_d->childrenContainer.at(i)->totalAmountToDiscount(iType);
        }
    }
    return ret;
}

double AccountingBillItem::amountNotToDiscount() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == LumpSum) ||
            (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->amountNotToDiscount;
    } else if( (m_d->itemType == TimeAndMaterials) && (m_d->tamBillItem != NULL) ){
        return m_d->tamBillItem->amountNotToDiscount();
    }
    return 0.0;
}

double AccountingBillItem::amountNotToDiscount(AccountingBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return amountNotToDiscount();
    } else if( !(m_d->childrenContainer.isEmpty()) ){
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountNotToDiscount( iType );
        }
        return ret;
    }
    return 0.0;
}

double AccountingBillItem::amountNotToDiscountPayment(int firstPay, int lastPay, AccountingBillItem::ItemType iType ) const {
    double ret = 0.0;
    if( m_d->itemType == Root ){
        if( firstPay < 0 ){
            firstPay = 0;
        }
        if( lastPay >= m_d->childrenContainer.size() ){
            lastPay = m_d->childrenContainer.size() - 1;
        }
        for( int i=firstPay; i <= lastPay; i++){
            ret += m_d->childrenContainer.at(i)->amountNotToDiscount(iType);
        }
    }
    return ret;
}

double AccountingBillItem::amountToDiscount() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == LumpSum) ||
            (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->amountToDiscount;
    } else if( (m_d->itemType == TimeAndMaterials) && (m_d->tamBillItem != NULL) ){
        return m_d->tamBillItem->amountToDiscount();
    }
    return 0.0;
}

double AccountingBillItem::amountToDiscount(AccountingBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return amountToDiscount();
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountToDiscount( iType );
        }
        return ret;
    }
    return 0.0;
}

double AccountingBillItem::amountDiscounted() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == LumpSum) ||
            (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->amountDiscounted;
    } else if( (m_d->itemType == TimeAndMaterials) && (m_d->tamBillItem != NULL) ){
        return m_d->tamBillItem->amountDiscounted();
    }
    return 0.0;
}

double AccountingBillItem::amountDiscounted(AccountingBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return amountDiscounted();
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountDiscounted( iType );
        }
        return ret;
    }
    return 0.0;
}

double AccountingBillItem::totalAmount() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == LumpSum) ||
            (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->totalAmount;
    } else if( (m_d->itemType == TimeAndMaterials) && (m_d->tamBillItem != NULL) ){
        return m_d->tamBillItem->totalAmount();
    }
    return 0.0;
}

double AccountingBillItem::totalAmount(AccountingBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return totalAmount();
    } else {
        double ret = 0.0;
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmount( iType );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingBillItem::totalAmountToDiscountStr() const {
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( totalAmountToDiscount(), 'f', m_d->amountPrecision );
}

QString AccountingBillItem::amountNotToDiscountStr() const{
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( amountNotToDiscount(), 'f', m_d->amountPrecision );
}

QString AccountingBillItem::amountToDiscountStr() const {
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( amountToDiscount(), 'f', m_d->amountPrecision );
}

QString AccountingBillItem::amountDiscountedStr() const {
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( amountDiscounted(), 'f', m_d->amountPrecision );
}

QString AccountingBillItem::totalAmountStr() const {
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( totalAmount(), 'f', m_d->amountPrecision );
}

QString AccountingBillItem::amountStr(int i ) const {
    switch(i){
    case 0:{
        return totalAmountToDiscountStr();
    }
    case 1:{
        return amountNotToDiscountStr();
    }
    case 2:{
        return amountToDiscountStr();
    }
    case 3:{
        return amountDiscountedStr();
    }
    case 4:{
        return totalAmountStr();
    }
    default:{
        return totalAmountToDiscountStr();
    }
    }
}

QString AccountingBillItem::title() const{
    if( m_d->itemType == Payment ){
        return trUtf8("S.A.L. N.%1").arg( QString::number(childNumber()+1) );
    } else if( m_d->itemType == TimeAndMaterials ){
        if( m_d->tamBillItem != NULL ){
            return m_d->tamBillItem->title();
        }
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
            disconnect( m_d->priceItem, &PriceItem::valueChanged, this, &AccountingBillItem::updatePPUs );
        }
        PriceItem * oldPriceItem = m_d->priceItem;

        m_d->priceItem = p;

        emit priceItemChanged( oldPriceItem, p );

        if( m_d->priceItem != NULL ){
            connect( m_d->priceItem, &PriceItem::codeFullChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &AccountingBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::valueChanged, this, &AccountingBillItem::updatePPUs );
        }

        emit itemChanged();
        emit dataChanged( this, m_d->priceCodeCol );
        emit dataChanged( this, m_d->priceUmCol );
        emit dataChanged( this, m_d->PPUTotalToDiscountCol );
        emit dataChanged( this, m_d->totalAmountToDiscountCol );
        emit dataChanged( this, m_d->PPUNotToDiscountCol );
        emit dataChanged( this, m_d->amountNotToDiscountCol );
        emit dataChanged( this, m_d->totalAmountCol );

        if( m_d->measuresModel != NULL ){
            m_d->measuresModel->setUnitMeasure( m_d->priceItem->unitMeasure() );
        }

        updatePPUs();
    }
}

double AccountingBillItem::quantity() const {
    if( m_d->childrenContainer.size() > 0 ){
        return 0.0;
    } else {
        return m_d->quantity;
    }
}

QString AccountingBillItem::quantityStr() const {
    if( m_d->itemType == PPU ){
        int prec = 2;
        if( m_d->priceItem != NULL ){
            if( m_d->priceItem->unitMeasure() != NULL ){
                prec = m_d->priceItem->unitMeasure()->precision();
            }
        }
        return m_d->toString( quantity(), 'f', prec );
    }
    if( m_d->itemType == LumpSum ){
        return QString("%1 %").arg( m_d->toString(m_d->quantity * 100.0, 'f', AccountingLSBillItem::percentagePrecision() ) );
    }
    return QString();
}

void AccountingBillItem::setQuantityPrivate(double v) {
    if( v != m_d->quantity ){
        m_d->quantity = v;
        emit quantityChanged( quantityStr() );
        emit itemChanged();
        emit dataChanged( this, m_d->quantityCol );
        updateTotalAmountToDiscount();
        updateAmountNotToDiscount();
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

double AccountingBillItem::PPUTotalToDiscount() const {
    if( m_d->itemType == LumpSum ){
        if( m_d->lsBill != NULL ){
            return m_d->lsBill->PPUTotalToDiscount();
        }
    }
    return m_d->PPUTotalToDiscount;
}

double AccountingBillItem::PPUNotToDiscount() const {
    if( m_d->itemType == LumpSum ){
        if( m_d->lsBill != NULL ){
            return m_d->lsBill->PPUNotToDiscount();
        }
    }
    return m_d->PPUNotToDiscount;
}

QString AccountingBillItem::PPUTotalToDiscountStr() const {
    return m_d->toString( PPUTotalToDiscount(), 'f', m_d->amountPrecision );
}

QString AccountingBillItem::PPUNotToDiscountStr() const {
    return m_d->toString( PPUNotToDiscount(), 'f', m_d->amountPrecision );
}

void AccountingBillItem::emitPriceDataUpdated() {
    updateTotalAmountToDiscount();
    updateAmountNotToDiscount();
    emit dataChanged( this, m_d->priceCodeCol );
    emit dataChanged( this, m_d->priceUmCol );
    emit dataChanged( this, m_d->PPUTotalToDiscountCol);
    emit dataChanged( this, m_d->totalAmountToDiscountCol );
    emit dataChanged( this, m_d->PPUNotToDiscountCol);
    emit dataChanged( this, m_d->amountNotToDiscountCol );
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
        m_d->measuresModel = new MeasuresModel( this, m_d->parser, ump );
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
        updateTotalAmountToDiscount();
        updateAmountNotToDiscount();
    }
}

#include "qtextformatuserdefined.h"

void AccountingBillItem::writeODTAccountingOnTable( QTextCursor *cursor,
                                                    int payToPrint,
                                                    AccountingPrinter::PrintPPUDescOption prPPUDescOption ) const {
    // spessore del bordo della tabella
    static double borderWidth = 1.0f;

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
    static QTextCharFormat txtCommentCharFormat = txtCharFormat;
    txtCommentCharFormat.setFontItalic(true);
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

    static QTextTableCellFormat centralUpHeaderFormat = centralHeaderFormat;
    centralUpHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_None) );
    centralUpHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(0.0) );

    static QTextTableCellFormat centralDownHeaderFormat = centralHeaderFormat;
    centralDownHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_None) );
    centralDownHeaderFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(0.0) );

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
    // centralSubTitleFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftSubTitleFormat = centralSubTitleFormat;
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightSubTitleFormat = centralSubTitleFormat;
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle commento ***
    // centrale
    static QTextTableCellFormat centralCommentFormat;
    centralCommentFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftCommentFormat = centralCommentFormat;
    leftCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightCommentFormat = centralCommentFormat;
    rightCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

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
    int cellCount = 10;

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    if( m_d->itemType == Root ){
        // *** Riga di intestazione ***
        AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N.Ord."), false );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Data") );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralUpHeaderFormat, headerBlockFormat, trUtf8("Libr.Mis."));
        AccountingBillItemPrivate::writeCell( cursor, table, centralUpHeaderFormat, headerBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralDownHeaderFormat, headerBlockFormat, trUtf8("N."));
        AccountingBillItemPrivate::writeCell( cursor, table, centralDownHeaderFormat, headerBlockFormat, trUtf8("N.Ord."));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );

        table->mergeCells( 0, 0, 2, 1  );
        table->mergeCells( 0, 1, 2, 1  );
        table->mergeCells( 0, 2, 2, 1  );
        table->mergeCells( 0, 3, 2, 1  );
        table->mergeCells( 0, 4, 1, 2  );
        table->mergeCells( 0, 6, 2, 1  );
        table->mergeCells( 0, 7, 2, 1  );
        table->mergeCells( 0, 8, 2, 1  );
        table->mergeCells( 0, 9, 2, 1  );

        if( payToPrint > 0 ){
            // *** Riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Riporto:") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );


            double prevTotAmToDisc = 0.0;
            double prevAmNotToDisc = 0.0;
            for( int i=0; i < payToPrint; ++i ){
                prevTotAmToDisc += m_d->childrenContainer.at(i)->totalAmountToDiscount();
                prevAmNotToDisc += m_d->childrenContainer.at(i)->amountNotToDiscount();
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( prevTotAmToDisc, 'f', m_d->amountPrecision ) );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( prevAmNotToDisc, 'f', m_d->amountPrecision ) );
        }

        // *** Riga vuota ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        // *** Scrive il computo dei sottoarticoli ***
        if( payToPrint < 0 ){
            for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTAccountingOnTable( cursor, 0, prPPUDescOption );
            }
        } else {
            if( payToPrint < m_d->childrenContainer.size() ){
                m_d->childrenContainer.at(payToPrint)->writeODTAccountingOnTable( cursor, 0, prPPUDescOption );
            }
        }

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountNotToDiscountStr() );

        // *** Riga di chiusura ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );

    } else {
        if( m_d->itemType == Payment ){

            // ci sono sottoarticoli
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, title() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );

            for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTAccountingOnTable( cursor, 0, prPPUDescOption );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale lordo %1").arg( title() ) );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

            if( amountNotToDiscount() != 0.0 ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountNotToDiscountStr() );
            }
        } else { // !hasChildren()
            if( m_d->itemType == Comment ){
                // do nothing
            } else {
                writeODTBillLine( AccountingPrinter::PrintAllAmounts, prPPUDescOption,
                                  true,
                                  cursor, table,
                                  tagBlockFormat, txtBlockFormat, numBlockFormat,
                                  leftFormat, centralFormat, rightFormat,
                                  centralQuantityTotalFormat, rightQuantityTotalFormat,
                                  txtCharFormat, txtCommentCharFormat,
                                  true );
            }

            // *** Riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingBillItem::writeODTMeasuresOnTable( QTextCursor *cursor, int payToPrint,
                                                  AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                  AccountingPrinter::PrintPPUDescOption prPPUDescOption ) const {
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
    static QTextCharFormat txtCommentCharFormat = txtCharFormat;
    txtCommentCharFormat.setFontItalic(true);
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
    // centralSubTitleFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftSubTitleFormat = centralSubTitleFormat;
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightSubTitleFormat = centralSubTitleFormat;
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle commento ***
    // centrale
    static QTextTableCellFormat centralCommentFormat;
    centralCommentFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftCommentFormat = centralCommentFormat;
    leftCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightCommentFormat = centralCommentFormat;
    rightCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

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
    if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
        cellCount += 2;
    }

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    if( m_d->itemType == Root ){
        // *** Riga di intestazione ***
        AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N."), false);
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Data") );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));
        }

        // *** Riga vuota ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        // *** Scrive il computo dei sottoarticoli ***
        if( payToPrint < 0 ){
            for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTMeasuresOnTable( cursor, 0, prAmountsOption, prPPUDescOption );
            }
        } else {
            if( payToPrint < m_d->childrenContainer.size() ){
                m_d->childrenContainer.at(payToPrint)->writeODTMeasuresOnTable( cursor, 0, prAmountsOption, prPPUDescOption );
            }
        }

        // *** riga dei totali complessivi
        if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountNotToDiscountStr() );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale soggetto a ribasso lordo") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountToDiscountStr() );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale soggetto a ribasso netto") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountDiscountedStr() );
            // *** Riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale complessivo") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountStr() );
        } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountNotToDiscountStr() );

        } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

        }

        // *** Riga di chiusura ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );

    } else {
        if( m_d->itemType == Payment ){

            // ci sono sottoarticoli
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, title() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            }

            // *** Riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTMeasuresOnTable( cursor, 0, prAmountsOption, prPPUDescOption );
            }

            // *** Riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso %1").arg( title() ) );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountNotToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale lordo %1").arg( title() ) );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountNotToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale lordo %1").arg( title() ) );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

                if( amountNotToDiscount() != 0.0 ){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );

                    AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountNotToDiscountStr() );
                }
            }
            // *** Riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
        } else { // !hasChildren()
            if( m_d->itemType == Comment ){
                writeODTBillLine( prAmountsOption, prPPUDescOption,
                                  true,
                                  cursor, table,
                                  tagBlockFormat, txtBlockFormat, numBlockFormat,
                                  leftCommentFormat, centralCommentFormat, rightCommentFormat,
                                  centralQuantityTotalFormat, rightQuantityTotalFormat,
                                  txtCharFormat, txtCommentCharFormat );
            } else {
                writeODTBillLine( prAmountsOption, prPPUDescOption,
                                  true,
                                  cursor, table,
                                  tagBlockFormat, txtBlockFormat, numBlockFormat,
                                  leftFormat, centralFormat, rightFormat,
                                  centralQuantityTotalFormat, rightQuantityTotalFormat,
                                  txtCharFormat, txtCommentCharFormat );
            }

            // *** Riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingBillItem::writeODTPaymentOnTable( QTextCursor *cursor,
                                                 int payToPrint,
                                                 AccountingPrinter::PrintPPUDescOption prPPUDescOption  ) const {
    writeODTSummaryOnTable( cursor, payToPrint, AccountingPrinter::PrintAllAmounts, prPPUDescOption, false );
}

void AccountingBillItem::writeODTAccountingSummaryOnTable( QTextCursor *cursor,
                                                           int payToPrint,
                                                           AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                           AccountingPrinter::PrintPPUDescOption prPPUDescOption ) const {
    writeODTSummaryOnTable( cursor, payToPrint, prAmountsOption, prPPUDescOption, true );
}

void AccountingBillItem::writeODTSummaryOnTable( QTextCursor *cursor,
                                                 int payToPrint,
                                                 AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                 AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                                 bool writeAccountingProgCode ) const {
    // Questo metodo ha senso solo se eseguito dall'elemento Root
    if( m_d->itemType != Root ){
        return;
    }

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
    static QTextCharFormat txtCommentCharFormat = txtCharFormat;
    txtCommentCharFormat.setFontItalic(true);
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
    // centralSubTitleFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftSubTitleFormat = centralSubTitleFormat;
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightSubTitleFormat = centralSubTitleFormat;
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightSubTitleFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

    // *** formattazione celle commento ***
    // centrale
    static QTextTableCellFormat centralCommentFormat;
    centralCommentFormat.setFontItalic( true );
    // sinistra
    static QTextTableCellFormat leftCommentFormat = centralCommentFormat;
    leftCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    leftCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
    // destra
    static QTextTableCellFormat rightCommentFormat = centralCommentFormat;
    rightCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
    rightCommentFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );

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
    if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
        cellCount += 2;
    }
    if( writeAccountingProgCode ){
        cellCount++;
    }

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    // *** Riga di intestazione ***
    // numero progressivo + codice + descrizione + unità di misura + quantità + prezzo + importo
    AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N.Ord."), false);
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
    if( writeAccountingProgCode ){
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("N.Ord. Reg."));
    }
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    } else {
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));
    }

    // *** Riga vuota ***
    AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    // *** Scrive le righe del S.A.L. ***

    // se payToPrint è negativo, stampa l'ultimo SAL
    if( payToPrint < 0 ){
        payToPrint = m_d->childrenContainer.size() - 1;
    }

    double totAmToDiscount = 0.0;
    double amNotToDiscount = 0.0;
    int nProg = 1;

    /* *** OPERE A CORPO *** */
    QList<AccountingLSBill *> usedLSBills = usedLSBillsPayment( 0, payToPrint );
    if( !usedLSBills.isEmpty()  ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("OPERE A CORPO") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        }
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        bool amNotToDiscountPresent = false;
        for( QList<AccountingLSBill *>::iterator billIter = usedLSBills.begin(); billIter != usedLSBills.end(); ++billIter ){
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, QString::number(nProg++) );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, (*billIter)->code() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, (*billIter)->name() );

            double itemTotalQuantity = 0.0;
            for( QList<AccountingBillItem *>::iterator pay=m_d->childrenContainer.begin(); pay != m_d->childrenContainer.end(); ++pay ){
                for( QList<AccountingBillItem *>::iterator line=(*pay)->m_d->childrenContainer.begin(); line != (*pay)->m_d->childrenContainer.end(); ++line ){
                    if( (*line)->itemType() == LumpSum ){
                        if( (*line)->m_d->lsBill == (*billIter) ){
                            if( writeAccountingProgCode ){
                                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, (*line)->accountingProgCode() );
                                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, trUtf8("%") );
                                if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, (*line)->quantityStr() );
                                } else {
                                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*line)->quantityStr() );
                                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*billIter)->PPUTotalToDiscountStr() );
                                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, (*line)->totalAmountToDiscountStr() );
                                }
                                table->appendRows(1);
                                cursor->movePosition(QTextCursor::PreviousRow );
                                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

                                if( (*billIter)->PPUNotToDiscount() != 0.0 && m_d->discount != 0.0 && prAmountsOption != AccountingPrinter::PrintNoAmount ){
                                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, trUtf8("%") );
                                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*line)->quantityStr() );
                                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*billIter)->PPUNotToDiscountStr() );
                                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, (*line)->amountNotToDiscountStr() );
                                    table->appendRows(1);
                                    cursor->movePosition(QTextCursor::PreviousRow );
                                    AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                                }
                            }
                            itemTotalQuantity += (*line)->quantity();
                        }
                    }
                }
            }

            if( writeAccountingProgCode ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            }
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, trUtf8("%") );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->percentageToString( itemTotalQuantity ) );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->percentageToString( itemTotalQuantity ) );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*billIter)->PPUTotalToDiscountStr() );
                double itemAmTotalToDisc = 0.0;
                if( prAmountsOption == AccountingPrinter::PrintAllAmounts || prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                    itemAmTotalToDisc = UnitMeasure::applyPrecision( (*billIter)->PPUTotalToDiscount()  * itemTotalQuantity, m_d->amountPrecision);
                } else { // prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount
                    itemAmTotalToDisc = UnitMeasure::applyPrecision( (*billIter)->PPUNotToDiscount()  * itemTotalQuantity, m_d->amountPrecision);
                }
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemAmTotalToDisc, 'f', m_d->amountPrecision ) );
            }

            if( (*billIter)->PPUNotToDiscount() != 0.0 && m_d->discount != 0.0 &&
                    prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                amNotToDiscountPresent = true;
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
                if( writeAccountingProgCode ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, trUtf8("%") );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->percentageToString( itemTotalQuantity ) );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, (*billIter)->PPUNotToDiscountStr()  );
                double itemAmNotToDiscount = UnitMeasure::applyPrecision( (*billIter)->PPUNotToDiscount()  * itemTotalQuantity, m_d->amountPrecision);
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemAmNotToDiscount, 'f', m_d->amountPrecision ) );
            }

            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
        }

        if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
            double lsTotAmToDiscount = totalAmountToDiscountPayment( 0, payToPrint, LumpSum );
            totAmToDiscount += lsTotAmToDiscount;
            double lsAmNotToDiscount = amountNotToDiscountPayment( 0, payToPrint, LumpSum );
            amNotToDiscount += lsAmNotToDiscount;

            if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ||
                    prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                if( m_d->discount != 0.0 ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a corpo - Totale al lordo del ribasso") );
                } else {
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a corpo - Totale") );
                }
                if( writeAccountingProgCode ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, m_d->toString( lsTotAmToDiscount, 'f', m_d->amountPrecision ) );
            }

            if( (prAmountsOption == AccountingPrinter::PrintAllAmounts && amNotToDiscountPresent) ||
                    prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a corpo - Totale non soggetto a ribasso") );
                if( writeAccountingProgCode ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, m_d->toString( lsAmNotToDiscount, 'f', m_d->amountPrecision ) );
            }
        }

        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
    }

    /* *** OPERE A MISURA *** */
    QList<AccountingBillItem *> usedItemsList = usedItemsPayment( 0, payToPrint, PPU );
    if( !usedItemsList.isEmpty() ){
        QList<PriceItem *> usedPItemsList;
        for( QList<AccountingBillItem *>::iterator i=m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            QList<PriceItem *> payUsedPItems = (*i)->usedPriceItems();
            for( QList<PriceItem *>::iterator usedPItem=payUsedPItems.begin(); usedPItem != payUsedPItems.end(); ++usedPItem ){
                if( !usedPItemsList.contains(*usedPItem ) && ((*usedPItem) != NULL)){
                    usedPItemsList << *usedPItem;
                }
            }
        }
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("OPERE A MISURA") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        }
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        bool amNotToDiscountPresent = false;
        for( QList<PriceItem *>::iterator pItem = usedPItemsList.begin(); pItem != usedPItemsList.end(); ++pItem){
            double pricePPUTotalToDiscount = 0.0;
            QList<int> totAmPriceFields = totalAmountPriceFields();
            for( QList<int>::iterator i = totAmPriceFields.begin(); i != totAmPriceFields.end(); ++i){
                pricePPUTotalToDiscount += (*pItem)->value( (*i), currentPriceDataSet() );
            }
            pricePPUTotalToDiscount = UnitMeasure::applyPrecision( pricePPUTotalToDiscount, m_d->amountPrecision );

            double pricePPUNotToDiscount = 0.0;
            QList<int> noDiscAmPriceFields = noDiscountAmountPriceFields();
            for( QList<int>::iterator i = noDiscAmPriceFields.begin(); i != noDiscAmPriceFields.end(); ++i){
                pricePPUNotToDiscount += (*pItem)->value( (*i), currentPriceDataSet() );
            }
            pricePPUNotToDiscount = UnitMeasure::applyPrecision( pricePPUNotToDiscount, m_d->amountPrecision );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, QString::number(nProg++) );
            if( (*pItem) != NULL ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, (*pItem)->codeFull() );
                m_d->writeDescriptionCell( (*pItem), cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prPPUDescOption );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            }

            QString unitMeasureTag;
            int unitMeasurePrec = 3;
            if( (*pItem) != NULL ){
                if( (*pItem)->unitMeasure() != NULL ){
                    unitMeasureTag = (*pItem)->unitMeasure()->tag();
                    unitMeasurePrec = (*pItem)->unitMeasure()->precision();
                }
            }

            double itemTotalQuantity = 0.0;
            int printedItems = 0;
            if( writeAccountingProgCode ){
                m_d->childrenContainer.at(payToPrint)->writeODTAccountingSummaryLine( *pItem, &printedItems, &itemTotalQuantity,
                                                                                      prAmountsOption != AccountingPrinter::PrintNoAmount,
                                                                                      cursor, table,
                                                                                      tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                                                      leftFormat, centralFormat, rightFormat );
            } else {
                m_d->childrenContainer.at(payToPrint)->writeODTSummaryLine( *pItem, cursor, &itemTotalQuantity,
                                                                            prAmountsOption != AccountingPrinter::PrintNoAmount, false,
                                                                            table,
                                                                            tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                                            leftFormat, centralFormat, rightFormat );
            }

            if( writeAccountingProgCode ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            }
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( pricePPUTotalToDiscount, 'f', m_d->amountPrecision ) );
                double itemTotAmToDiscount = 0.0;
                if( prAmountsOption == AccountingPrinter::PrintAllAmounts || prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                    itemTotAmToDiscount = UnitMeasure::applyPrecision( pricePPUTotalToDiscount * itemTotalQuantity, m_d->amountPrecision);
                } else { // prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount
                    itemTotAmToDiscount = UnitMeasure::applyPrecision( pricePPUNotToDiscount * itemTotalQuantity, m_d->amountPrecision);
                }
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemTotAmToDiscount, 'f', m_d->amountPrecision ) );
            }

            if( pricePPUNotToDiscount != 0.0 && m_d->discount != 0.0 &&
                    prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                amNotToDiscountPresent = true;

                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
                if( writeAccountingProgCode ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( pricePPUNotToDiscount, 'f', m_d->amountPrecision ) );
                double itemAmNotToDiscount = UnitMeasure::applyPrecision( pricePPUNotToDiscount * itemTotalQuantity, m_d->amountPrecision);
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemAmNotToDiscount, 'f', m_d->amountPrecision ) );
                AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            }
        }

        if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
            double ppuTotAmToDiscount = totalAmountToDiscountPayment( 0, payToPrint, PPU );
            totAmToDiscount += ppuTotAmToDiscount;
            double ppuAmNotToDiscount = amountNotToDiscountPayment( 0, payToPrint, PPU );
            amNotToDiscount += ppuAmNotToDiscount;

            if( prAmountsOption == AccountingPrinter::PrintAllAmounts ||
                    prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){

                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                if( m_d->discount != 0.0 ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a misura - Totale al lordo del ribasso") );
                } else {
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a misura - Totale") );
                }
                if( writeAccountingProgCode ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, m_d->toString( ppuTotAmToDiscount, 'f', m_d->amountPrecision ) );
            }

            if( (amNotToDiscountPresent && prAmountsOption == AccountingPrinter::PrintAllAmounts) ||
                    (prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount) ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a misura - Totale non soggetto a ribasso") );
                if( writeAccountingProgCode ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, m_d->toString( ppuAmNotToDiscount, 'f', m_d->amountPrecision ) );
            }
        }

        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
    }

    /* *** LISTE IN ECONOMIA *** */
    usedItemsList = usedItemsPayment( 0, payToPrint, TimeAndMaterials );
    if( !usedItemsList.isEmpty() ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("LISTE IN ECONOMIA") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        bool amNotToDiscountPresent = false;
        for( QList<AccountingBillItem *>::iterator line=usedItemsList.begin(); line != usedItemsList.end(); ++line ){
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, QString::number(nProg++) );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, (*line)->title() );
            if( writeAccountingProgCode ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, (*line)->accountingProgCode() );
            }
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                if( prAmountsOption == AccountingPrinter::PrintAllAmounts ||
                        prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, (*line)->totalAmountToDiscountStr() );
                } else { // prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, (*line)->amountNotToDiscountStr() );
                }
            }

            if( (*line)->amountToDiscount() != 0.0 && m_d->discount != 0.0 &&
                    prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                amNotToDiscountPresent = true;
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
                if( writeAccountingProgCode ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat);
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, (*line)->amountNotToDiscountStr() );
            }

            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
        }

        if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
            double tamTotAmToDiscount = totalAmountToDiscountPayment( 0, payToPrint, TimeAndMaterials );
            totAmToDiscount += tamTotAmToDiscount;
            double tamAmNotToDiscount = amountNotToDiscountPayment( 0, payToPrint, TimeAndMaterials );
            amNotToDiscount += tamAmNotToDiscount;


            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            if( m_d->discount != 0.0 ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Liste in economia - Totale al lordo del ribasso") );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Liste in economia - Totale") );
            }
            if( writeAccountingProgCode ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            }
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, m_d->toString( tamTotAmToDiscount, 'f', m_d->amountPrecision ) );

            if( amNotToDiscountPresent ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Liste in economia - Totale non soggetto a ribasso") );
                if( writeAccountingProgCode ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, m_d->toString( tamAmNotToDiscount, 'f', m_d->amountPrecision ) );
            }
        }

        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
    }

    /* *** TOTALI COMPLESSIVI *** */

    if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totAmToDiscount, 'f', m_d->amountPrecision ) );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amNotToDiscount, 'f', m_d->amountPrecision ) );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale soggetto a ribasso lordo") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        double amToDiscount = UnitMeasure::applyPrecision( totAmToDiscount - amNotToDiscount, m_d->amountPrecision);
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amToDiscount, 'f', m_d->amountPrecision ) );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale soggetto a ribasso netto") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        double amDiscounted = UnitMeasure::applyPrecision( amToDiscount * (1.0 - discount() ), m_d->amountPrecision );;
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amDiscounted, 'f', m_d->amountPrecision ) );
        // *** Riga vuota ***
        AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale") );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale S.A.L.") );
        }
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        double totAm = amNotToDiscount + amDiscounted;
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totAm, 'f', m_d->amountPrecision ) );
    } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totAmToDiscount, 'f', m_d->amountPrecision ) );
    } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
        if( writeAccountingProgCode ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amNotToDiscount, 'f', m_d->amountPrecision ) );
    }

    // *** Riga di chiusura ***
    AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

void AccountingBillItem::writeODTAccountingSummaryLine( PriceItem * priceItem,
                                                        int * printedItems,
                                                        double * itemTotalQuantity,
                                                        bool printAmounts,
                                                        QTextCursor *cursor,
                                                        QTextTable *table,
                                                        QTextBlockFormat & tagBlockFormat,
                                                        QTextBlockFormat & txtBlockFormat,
                                                        QTextBlockFormat & numBlockFormat,
                                                        QTextTableCellFormat & leftFormat,
                                                        QTextTableCellFormat & centralFormat,
                                                        QTextTableCellFormat & rightFormat ) const {
    if( m_d->childrenContainer.size() == 0 ){
        if( priceItem == m_d->priceItem ){
            (*printedItems)++;
            (*itemTotalQuantity) += quantity();

            // num prog del registro
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, accountingProgCode() );

            QString unitMeasureTag;
            if( m_d->priceItem != NULL ){
                if( m_d->priceItem->unitMeasure() != NULL ){
                    unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                }
            }
            // UdM
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );
            if( printAmounts ){
                // quantita
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                // costo unitario
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                // importo
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            } else {
                // quantita
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            // numprog
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            // codice prezzo
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            // descrizione prezzo
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAccountingSummaryLine( priceItem, printedItems, itemTotalQuantity, printAmounts,
                                                 cursor, table,
                                                 tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                 leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingBillItem::writeODTSummaryLine( PriceItem * priceItem,
                                              QTextCursor *cursor,
                                              double * itemTotalQuantity,
                                              bool printAmounts,
                                              bool writeDetails,
                                              QTextTable *table,
                                              QTextBlockFormat & tagBlockFormat,
                                              QTextBlockFormat & txtBlockFormat,
                                              QTextBlockFormat & numBlockFormat,
                                              QTextTableCellFormat & leftFormat,
                                              QTextTableCellFormat & centralFormat,
                                              QTextTableCellFormat & rightFormat ) const {

    // TODO
    if( m_d->childrenContainer.size() == 0 ){
        if( priceItem == m_d->priceItem ){
            (*itemTotalQuantity) += quantity();

            if( writeDetails ){
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, progCode() );

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
            (*i)->writeODTSummaryLine( priceItem, cursor, itemTotalQuantity, printAmounts, writeDetails,
                                       table,
                                       tagBlockFormat, txtBlockFormat, numBlockFormat,
                                       leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingBillItem::writeODTAttributeAccountingOnTable( QTextCursor *cursor,
                                                             AccountingPrinter::AttributePrintOption prOption,
                                                             AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                             AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                                             const QList<Attribute *> &attrsToPrint ) const {
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
    // centralSubTitleFormat.setFontItalic( true );
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
    if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
        cellCount += 2;
    }


    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    // *** Intestazione tabella ***
    AccountingBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("Codice"), false );
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Denominazione"));
    AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    } else {
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));
    }

    // *** riga vuota ***
    AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    double totalAmountToDiscountTotal = 0.0;
    double amountNotToDiscountTotal = 0.0;

    if( prOption == AccountingPrinter::AttributePrintSimple ){
        // *** Righe del computo suddivise per attributo ***
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            totalAmountToDiscountTotal = 0.0;
            amountNotToDiscountTotal = 0.0;

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            // cursor->movePosition(QTextCursor::NextCell);

            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, (*i)->name() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            } else {
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            }

            // *** riga vuota ***
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            writeODTAttributeBillLineSimple( prAmountsOption,
                                             prPPUDescOption,
                                             &totalAmountToDiscountTotal,
                                             &amountNotToDiscountTotal,
                                             *i,
                                             cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                             leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                             txtCharFormat, txtBoldCharFormat );

            if( discount() == 0.0 ){
                AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
            } else {
                AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
                if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );
                    AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
                }

                if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );
                    AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale non soggetto a ribasso") );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amountNotToDiscountTotal, 'f', m_d->amountPrecision) );
                }
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
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario") );
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo") );
        }

        writeODTAttributeBillLineUnion( prAmountsOption,
                                        prPPUDescOption,
                                        &totalAmountToDiscountTotal,
                                        &amountNotToDiscountTotal,
                                        attrsToPrint,
                                        cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                        leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                        txtCharFormat, txtBoldCharFormat );

        if( discount() == 0.0 ){
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
        } else {
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
            }

            if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale non soggetto a ribasso") );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amountNotToDiscountTotal, 'f', m_d->amountPrecision) );
            }
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
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario") );
            AccountingBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo") );
        }

        writeODTAttributeBillLineIntersection( prAmountsOption,
                                               prPPUDescOption,
                                               &totalAmountToDiscountTotal,
                                               &amountNotToDiscountTotal,
                                               attrsToPrint,
                                               cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                               leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                               txtCharFormat, txtBoldCharFormat );

        if( discount() == 0.0 ){
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
        } else {
            AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
            }

            if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale non soggetto a ribasso") );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amountNotToDiscountTotal, 'f', m_d->amountPrecision) );
            }
        }
    }

    // *** Riga di chiusura ***
    AccountingBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

AccountingBillItem::ItemType AccountingBillItem::itemType() const {
    return m_d->itemType;
}

void AccountingBillItem::writeODTAttributeBillLineSimple( AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                          AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                                          double *totalAmountToDiscountTotal,
                                                          double *amountNotToDiscountTotal,
                                                          Attribute *attrsToPrint,
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
            (*totalAmountToDiscountTotal) += totalAmountToDiscount();
            (*amountNotToDiscountTotal) += amountNotToDiscount();

            writeODTBillLine( prAmountsOption,
                              prPPUDescOption,
                              false,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineSimple( prAmountsOption, prPPUDescOption,
                                                   totalAmountToDiscountTotal, amountNotToDiscountTotal,
                                                   attrsToPrint,
                                                   cursor, table,
                                                   tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                   leftFormat, centralFormat, rightFormat,
                                                   centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                   txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingBillItem::writeODTAttributeBillLineUnion( AccountingPrinter::PrintAmountsOption prAmountsOption, AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                                         double *totalAmountToDiscountTotal,
                                                         double *amountNotToDiscountTotal,
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
            (*totalAmountToDiscountTotal) += totalAmountToDiscount();
            (*amountNotToDiscountTotal) += amountNotToDiscount();
            writeODTBillLine( prAmountsOption, prPPUDescOption,
                              false,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineUnion( prAmountsOption, prPPUDescOption,
                                                  totalAmountToDiscountTotal, amountNotToDiscountTotal,
                                                  attrsToPrint,
                                                  cursor, table,
                                                  tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                  leftFormat, centralFormat, rightFormat,
                                                  centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                  txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingBillItem::writeODTAttributeBillLineIntersection(AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                               AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                                               double * totalAmountToDiscountTotal,
                                                               double * amountNotToDiscountTotal,
                                                               const QList<Attribute *> &attrsToPrint,
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
            (*totalAmountToDiscountTotal) += totalAmountToDiscount();
            (*amountNotToDiscountTotal) += amountNotToDiscount();
            writeODTBillLine( prAmountsOption, prPPUDescOption,
                              false,
                              cursor, table,
                              tagBlockFormat, txtBlockFormat, numBlockFormat,
                              leftFormat, centralFormat, rightFormat,
                              centralQuantityTotalFormat, rightQuantityTotalFormat,
                              txtCharFormat, txtBoldCharFormat );
        }
    } else {
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAttributeBillLineIntersection( prAmountsOption, prPPUDescOption,
                                                         totalAmountToDiscountTotal, amountNotToDiscountTotal,
                                                         attrsToPrint,
                                                         cursor, table,
                                                         tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                         leftFormat, centralFormat, rightFormat,
                                                         centralQuantityTotalFormat, rightQuantityTotalFormat,
                                                         txtCharFormat, txtBoldCharFormat );
        }
    }
}

void AccountingBillItem::writeODTBillLine(AccountingPrinter::PrintAmountsOption prAmountsOption,
                                          AccountingPrinter::PrintPPUDescOption prItemsOption,
                                          bool writeProgCode,
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
                                          QTextCharFormat & txtAmNotToDiscCharFormat,
                                          bool writeAccounting ) const {
    // Numero di colonne contenute
    int colCount = 0;

    if( writeProgCode ){
        colCount = 5;
    } else {
        colCount = 4;
    }

    if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
        colCount += 2;
    }

    // si suppone che non ci sono sottoarticoli
    table->appendRows(1);
    cursor->movePosition(QTextCursor::PreviousRow );


    if( m_d->itemType == PPU ){
        if( writeProgCode ){
            if( writeAccounting ){
                // codice progressivo
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, accountingProgCode()  );
            } else {
                // codice progressivo
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progCode()  );
            }
            // data
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, dateStr()  );
        } else {
            // data
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, dateStr()  );
        }


        if( m_d->priceItem ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->codeFull() );
            m_d->writeDescriptionCell( cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtCharFormat, prItemsOption );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }

        if( m_d->measuresModel != NULL && !writeAccounting ){
            // celle vuote
            // tag unita misura
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                // quantita'
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            } else {
                // quantita'
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                // prezzo
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                // importo
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            if( writeProgCode ){
                // numero progressivo
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                // data
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            } else {
                // data
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
            }

            // codice
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

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
                        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() );
                    } else {
                        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() + " (" + measure->effectiveFormula() + ")");
                    }
                } else {
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat);
                }

                if( realFormula.isEmpty() || measure == NULL ){
                    // unita di misura
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

                    // quantità
                    if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                        AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                    } else {
                        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    }
                } else {
                    // unita di misura
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                    // quantità
                    if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                        AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, measure->quantityStr() );
                    } else {
                        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->quantityStr() );
                    }
                }

                // celle vuote
                if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
                }

                // inserisce riga
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                if( writeProgCode ){
                    // numero progressivo
                    AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                    // data
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                    // codice
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                } else {
                    // data
                    AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                    // codice
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                }
            }

            // descrizione
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

            // tag unita di misura
            AccountingBillItemPrivate::writeCell( cursor, table,  centralFormat, tagBlockFormat, unitMeasureTag );

            // quantita totale
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingBillItemPrivate::writeCell( cursor, table, rightQuantityTotalFormat, numBlockFormat, quantityStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, quantityStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, quantityStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUNotToDiscountStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountNotToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, quantityStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );

                if( PPUNotToDiscount() != 0.0 ){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );

                    QList< QPair<QString, QTextCharFormat> > txt;
                    txt << qMakePair( QString(), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, txt );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( trUtf8("di cui non soggetto a ribasso"), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( unitMeasureTag, txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( quantityStr(), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( PPUNotToDiscountStr(), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( amountNotToDiscountStr(), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, txt );
                }
            }
        } else { // m_d->linesModel == NULL
            if( writeAccounting ){
                QString nLibr;
                if( m_d->parentItem != NULL ){
                    nLibr = QString::number( m_d->parentItem->childNumber() + 1 );
                }
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, nLibr );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, progCode() );
            }
            QString unitMeasureTag;
            if( m_d->priceItem ){
                if( m_d->priceItem->unitMeasure()){
                    unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                }
            }
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

            // quantita totale
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUNotToDiscountStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountNotToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );

                if( PPUNotToDiscount() != 0.0 ){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );

                    QList< QPair<QString, QTextCharFormat> > txt;
                    txt << qMakePair( QString(), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, txt );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( trUtf8("di cui non soggetto a ribasso"), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                    if( writeAccounting ){
                        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                    }
                    txt.clear();
                    txt << qMakePair( unitMeasureTag, txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( quantityStr(), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( PPUNotToDiscountStr(), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( amountNotToDiscountStr(), txtAmNotToDiscCharFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, txt );
                }
            }
        }
    } else if( m_d->itemType == Comment ){
        if( writeProgCode ){
            // codice progressivo
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat  );
            // data
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat  );
        } else {
            // data
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat  );
        }


        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, text() );

        if( writeAccounting ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
        } else {
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
        }
    } else if( m_d->itemType == LumpSum){
        if( writeProgCode ){
            // codice progressivo
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progCode()  );
            // data
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, dateStr()  );
        } else {
            // data
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, dateStr()  );
        }


        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, title() );
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

        if( writeAccounting ){
            QString nLibr;
            if( m_d->parentItem != NULL ){
                nLibr = QString::number( m_d->parentItem->childNumber() + 1 );
            }
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, nLibr );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, progCode() );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, trUtf8("%") );

        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
        } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );
        } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUNotToDiscountStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountNotToDiscountStr() );
        } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );

            if( PPUNotToDiscount() != 0.0 ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                QList< QPair<QString, QTextCharFormat> > txt;
                txt << qMakePair( QString(), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, txt );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                txt.clear();
                txt << qMakePair( trUtf8("di cui non soggetto a ribasso"), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                if( writeAccounting ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                }
                txt.clear();
                txt << qMakePair( trUtf8("%"), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                txt.clear();
                txt << qMakePair( quantityStr(), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                txt.clear();
                txt << qMakePair( PPUNotToDiscountStr(), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                txt.clear();
                txt << qMakePair( amountNotToDiscountStr(), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, txt );
            }
        }
    } else if( m_d->itemType == TimeAndMaterials ){
        if( writeProgCode ){
            // codice progressivo
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progCode()  );
            // data
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, dateStr()  );
        } else {
            // data
            AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, dateStr()  );
        }

        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, title() );

        if( writeAccounting ){
            QString nLibr;
            if( m_d->parentItem != NULL ){
                nLibr = QString::number( m_d->parentItem->childNumber() + 1 );
            }
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, nLibr );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, progCode() );
        }
        AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
        } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );
        } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountNotToDiscountStr() );
        } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );
            if( amountNotToDiscount() != 0.0 ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                QList< QPair<QString, QTextCharFormat> > txt;
                txt << qMakePair( QString(), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, txt );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                txt.clear();
                txt << qMakePair( trUtf8("di cui non soggetto a ribasso"), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                if( writeAccounting ){
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                    AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                }
                txt.clear();
                txt << qMakePair( QString(), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                AccountingBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                txt.clear();
                txt << qMakePair( amountNotToDiscountStr(), txtAmNotToDiscCharFormat );
                AccountingBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, txt );
            }
        }
    }
}
