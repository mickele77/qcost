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

#include "accountingtambillitem.h"

#include "accountingtambillitemprivate.h"

#include "accountingtambill.h"
#include "accountingtambillitem.h"
#include "accountingpricefieldmodel.h"
#include "accountingprinter.h"
#include "accountingprinter.h"
#include "pricelist.h"
#include "priceitem.h"
#include "accountingtammeasuresmodel.h"
#include "accountingtammeasure.h"
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


AccountingTAMBillItem::AccountingTAMBillItem(AccountingTAMBillItem *parentItem, AccountingTAMBillItem::ItemType iType,
                                             PriceFieldModel * pfm, MathParser * parser , VarsModel *vModel):
    TreeItem(),
    m_d( new AccountingTAMBillItemPrivate(parentItem, iType, pfm, parser, vModel ) ){

    if( parentItem != nullptr ){
        connect( parentItem, &AccountingTAMBillItem::attributesChanged, this, &AccountingTAMBillItem::attributesChanged );
        connect( parentItem, &AccountingTAMBillItem::discountChanged, this, &AccountingTAMBillItem::discountChanged );
    }

    connect( this, static_cast<void(AccountingTAMBillItem::*)( AccountingTAMBillItem *, QList<int> )>(&AccountingTAMBillItem::hasChildrenChanged), this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::startDateChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::endDateChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::nameChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::textChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::titleChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::priceItemChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::currentPriceDataSetChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::discountChanged, this, &AccountingTAMBillItem::itemChanged );

    if( m_d->itemType == PPU ){
        connect( this, &AccountingTAMBillItem::quantityChanged, this, &AccountingTAMBillItem::updateTotalAmountToDiscount );
        connect( this, &AccountingTAMBillItem::PPUTotalToDiscountChanged, this, &AccountingTAMBillItem::updateTotalAmountToDiscount );
        connect( this, &AccountingTAMBillItem::quantityChanged, this, &AccountingTAMBillItem::updateAmountNotToDiscount );
        connect( this, &AccountingTAMBillItem::PPUNotToDiscountChanged, this, &AccountingTAMBillItem::updateAmountNotToDiscount );
    }
    connect( this, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBillItem::updateAmountToDiscount );
    connect( this, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBillItem::updateAmountToDiscount );
    connect( this, &AccountingTAMBillItem::amountToDiscountChanged, this, &AccountingTAMBillItem::updateAmountDiscounted );
    connect( this, &AccountingTAMBillItem::discountChanged, this, &AccountingTAMBillItem::updateAmountDiscounted );
    connect( this, &AccountingTAMBillItem::amountDiscountedChanged, this, &AccountingTAMBillItem::updateTotalAmount );
    connect( this, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBillItem::updateTotalAmount );

    connect( this, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBillItem::amountsChanged );
    connect( this, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBillItem::amountsChanged );
    connect( this, &AccountingTAMBillItem::amountToDiscountChanged, this, &AccountingTAMBillItem::amountsChanged );
    connect( this, &AccountingTAMBillItem::amountDiscountedChanged, this, &AccountingTAMBillItem::amountsChanged );
    connect( this, &AccountingTAMBillItem::totalAmountChanged, this, &AccountingTAMBillItem::amountsChanged );

    connect( this, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::amountToDiscountChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::amountDiscountedChanged, this, &AccountingTAMBillItem::itemChanged );
    connect( this, &AccountingTAMBillItem::totalAmountChanged, this, &AccountingTAMBillItem::itemChanged );

    if( m_d->totalAmountPriceFieldModel != nullptr ){
        connect( m_d->totalAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingTAMBillItem::itemChanged );
        connect( m_d->totalAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingTAMBillItem::updateTotalAmountToDiscount );
    }
    if( m_d->noDiscountAmountPriceFieldModel != nullptr ){
        connect( m_d->noDiscountAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingTAMBillItem::itemChanged );
        connect( m_d->noDiscountAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingTAMBillItem::updateAmountNotToDiscount );
    }

    connect( this, &AccountingTAMBillItem::attributesChanged, this, &AccountingTAMBillItem::itemChanged );

    if( m_d->itemType == Payment ) {
        connect( m_d->noDiscountAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingTAMBillItem::updatePPUs );
        connect( m_d->totalAmountPriceFieldModel, &AccountingPriceFieldModel::modelChanged, this, &AccountingTAMBillItem::updatePPUs );
        m_d->startDate = new QDate();
        m_d->endDate = new QDate();
    }
}

AccountingTAMBillItem::~AccountingTAMBillItem(){
    emit aboutToBeDeleted();
    delete m_d;
}

QString AccountingTAMBillItem::title() const{
    if( m_d->itemType == Payment ){
        return trUtf8("Lista N.%1 (%2-%3)").arg(QString::number(childNumber()+1), startDateStr(), endDateStr() );
    }
    return QString();
}

AccountingTAMBillItem &AccountingTAMBillItem::operator=(const AccountingTAMBillItem &cp) {
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

        if( m_d->itemType == PPU ){
            setPriceItem( cp.m_d->priceItem );
            setQuantity( cp.m_d->quantity );
            if( cp.m_d->measuresModel != nullptr ){
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

const AccountingTAMBillItem * AccountingTAMBillItem::rootItem() const {
    if( m_d->parentItem == nullptr ){
        return this;
    } else {
        return m_d->parentItem->rootItem();
    }
}

QString AccountingTAMBillItem::name(){
    return m_d->name;
}

void AccountingTAMBillItem::setName( const QString & newName ){
    if( m_d->name != newName ){
        m_d->name = newName;
        emit nameChanged( newName );
        emit dataChanged( this, m_d->priceShortDescCol );
    }
}

void AccountingTAMBillItem::setItemType(AccountingTAMBillItem::ItemType iType) {
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

TreeItem *AccountingTAMBillItem::parentInternal() {
    return m_d->parentItem;
}

AccountingTAMBillItem *AccountingTAMBillItem::parent() {
    return m_d->parentItem;
}

void AccountingTAMBillItem::setParent(AccountingTAMBillItem * newParent, int position ) {
    if( m_d->parentItem != newParent ){
        if( m_d->parentItem != nullptr ){
            m_d->parentItem->removeChildren( childNumber() );
            disconnect( m_d->parentItem, &AccountingTAMBillItem::attributesChanged, this, &AccountingTAMBillItem::attributesChanged );
        }
        m_d->parentItem = newParent;
        if( newParent != nullptr ){
            newParent->addChild( this, position);
            connect( m_d->parentItem, &AccountingTAMBillItem::attributesChanged, this, &AccountingTAMBillItem::attributesChanged );
        }
        emit itemChanged();
    } else {
        int oldPosition = childNumber();
        if( oldPosition != position ){
            if( oldPosition > position ){
                oldPosition++;
            }
            m_d->parentItem->m_d->childrenContainer.insert( position, this );
            m_d->parentItem->m_d->childrenContainer.removeAt( oldPosition );
            emit itemChanged();
        }
    }
}

void AccountingTAMBillItem::addChild(AccountingTAMBillItem * newChild, int position ) {
    m_d->childrenContainer.insert( position, newChild );
    connect( newChild, static_cast<void(AccountingTAMBillItem::*)(AccountingTAMBillItem*,int)> (&AccountingTAMBillItem::dataChanged), this, static_cast<void(AccountingTAMBillItem::*)(AccountingTAMBillItem*,int)> (&AccountingTAMBillItem::dataChanged) );
    connect( newChild, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBillItem::updateTotalAmountToDiscount );
    connect( newChild, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBillItem::updateAmountNotToDiscount );
    connect( this, &AccountingTAMBillItem::currentPriceDataSetChanged, newChild, &AccountingTAMBillItem::setCurrentPriceDataSet );
    connect( newChild, &AccountingTAMBillItem::itemChanged, this, &AccountingTAMBillItem::itemChanged );
}

AccountingTAMBillItem *AccountingTAMBillItem::itemFromId( unsigned int itemId ) {
    if( itemId == m_d->id ){
        return this;
    } else {
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            AccountingTAMBillItem * childItems = (*i)->itemFromId(itemId);
            if( childItems != nullptr ) {
                return childItems;
            }
        }
    }
    return NULL;
}

AccountingTAMBillItem *AccountingTAMBillItem::findItemFromId( unsigned int searchitemId ) {
    if( m_d->parentItem == nullptr ){
        return itemFromId(searchitemId );
    } else {
        return m_d->parentItem->findItemFromId(searchitemId);
    }
    return NULL;
}

AccountingTAMBillItem *AccountingTAMBillItem::itemFromProgCode( const QString & pCode) {
    if( pCode == fullProgCode() ){
        return this;
    } else {
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            AccountingTAMBillItem * childItems = (*i)->itemFromProgCode(pCode);
            if( childItems != nullptr ) {
                return childItems;
            }
        }
    }
    return NULL;
}

AccountingTAMBillItem *AccountingTAMBillItem::findItemFromProgCode( const QString & pCode ) {
    if( m_d->parentItem == nullptr ){
        return itemFromProgCode(pCode);
    } else {
        return m_d->parentItem->findItemFromProgCode(pCode);
    }
    return NULL;
}

AccountingTAMBillItem *AccountingTAMBillItem::childItem(int number) {
    return dynamic_cast<AccountingTAMBillItem *>(child( number ));
}

void AccountingTAMBillItem::setId( unsigned int ii ) {
    m_d->id = ii;
}

VarsModel * AccountingTAMBillItem::varsModel() {
    if( m_d->itemType == Root ){
        return m_d->varsModel;
    } else {
        return m_d->parentItem->varsModel();
    }
}

unsigned int AccountingTAMBillItem::id() {
    return m_d->id;
}

QString AccountingTAMBillItem::accountingProgCode() const {
    if( m_d->itemType == PPU ){
        return QString::number( m_d->accountingProgCode );
    }
    return QString();
}

void AccountingTAMBillItem::updateAccountingProgCode() {
    int startCode = 1;
    updateAccountingProgCode( &startCode );
}

void AccountingTAMBillItem::updateAccountingProgCode( int * startCode ) {
    QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin();
    if( i == m_d->childrenContainer.end() ){
        if( m_d->itemType == PPU ){
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

QString AccountingTAMBillItem::fullProgCode() const {
    if( m_d->itemType == PPU ){
        QString ret = progCode();
        if( m_d->parentItem != nullptr ){
            ret = QString::number( m_d->parentItem->childNumber()+1 ) + "." + ret;
        }
        return ret;
    } else if( m_d->itemType == Payment ){
        QString ret;
        if( m_d->parentItem != nullptr ){
            ret = QString::number( m_d->parentItem->childNumber()+1 );
        }
        return ret;
    }
    return QString();
}

QString AccountingTAMBillItem::progCode() const {
    if( m_d->itemType == PPU ){
        return QString::number( m_d->progCode );
    }
    return QString();
}

void AccountingTAMBillItem::updateProgCode() {
    for( QList<AccountingTAMBillItem *>::iterator pay = m_d->childrenContainer.begin();
         pay != m_d->childrenContainer.end(); ++pay ){
        int startCode = 1;
        (*pay)->updateProgCode( &startCode );
    }
}

void AccountingTAMBillItem::updateProgCode( int * startCode ) {
    QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin();
    if( i == m_d->childrenContainer.end() ){
        if( m_d->itemType == PPU ){
            m_d->progCode = *startCode;
            (*startCode)++;
        }
    }
    while( i!= m_d->childrenContainer.end() ){
        (*i)->updateProgCode( startCode );
        ++i;
    }
}

void AccountingTAMBillItem::updateDaysCount() {
    m_d->daysCount = m_d->startDate->daysTo( *(m_d->endDate) ) + 1;
    if ( m_d->daysCount < 1 ){
        m_d->daysCount = 1;
    }
}

QList<int> AccountingTAMBillItem::totalAmountPriceFields() const {
    if( m_d->itemType == Root ){
        return *(m_d->totalAmountPriceFieldsList);
    } else if( m_d->parentItem != nullptr ){
        return m_d->parentItem->totalAmountPriceFields();
    }
    return QList<int>();
}


void AccountingTAMBillItem::setTotalAmountPriceFields(const QList<int> &newAmountFields) {
    if( m_d->itemType == Root ){
        m_d->totalAmountPriceFieldModel->setPriceFields( newAmountFields );
    }
}


AccountingPriceFieldModel *AccountingTAMBillItem::totalAmountPriceFieldModel() {
    return m_d->totalAmountPriceFieldModel;
}

QList<int> AccountingTAMBillItem::noDiscountAmountPriceFields() const {
    if( m_d->itemType == Root ){
        return *(m_d->noDiscountAmountPriceFieldsList);
    } else if( m_d->parentItem != nullptr ){
        return m_d->parentItem->noDiscountAmountPriceFields();
    }
    return QList<int>();
}

void AccountingTAMBillItem::setNoDiscountAmountPriceFields(const QList<int> &newAmountFields) {
    if( m_d->itemType == Root ){
        m_d->noDiscountAmountPriceFieldModel->setPriceFields( newAmountFields );
    }
}

void AccountingTAMBillItem::updatePPUs() {
    if( hasChildren() ){
        for( QList<AccountingTAMBillItem *>::iterator i=m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->updatePPUs();
        }
    } else {
        if( m_d->itemType == PPU ){
            double newPPUTotalToDiscount = 0.0;
            double newPPUNotToDiscount = 0.0;
            if( m_d->priceItem != nullptr ){
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
        }
    }
}

AccountingPriceFieldModel *AccountingTAMBillItem::noDiscountAmountPriceFieldModel() {
    return m_d->noDiscountAmountPriceFieldModel;
}

void AccountingTAMBillItem::requestDateBeginChange(const QString &newDateStr) {
    QDate newDate;
    if( m_d->parser != nullptr ){
        newDate = m_d->parser->evaluateDate(newDateStr, QLocale::NarrowFormat);
    } else {
        newDate = QDate::fromString(newDateStr, Qt::DefaultLocaleShortDate);
    }

    requestDateBeginChange( newDate);
}

void AccountingTAMBillItem::requestDateBeginChange(const QDate &newDate) {
    emit requestDateBeginChangeSignal( newDate, childNumber() );
}

void AccountingTAMBillItem::requestDateEndChange(const QString &newDateStr) {
    QDate newDate;
    if( m_d->parser != nullptr ){
        newDate = m_d->parser->evaluateDate(newDateStr, QLocale::NarrowFormat);
    } else {
        newDate = QDate::fromString(newDateStr, Qt::DefaultLocaleShortDate);
    }

    requestDateEndChange( newDate );
}

void AccountingTAMBillItem::requestDateEndChange(const QDate &newDate) {
    emit requestDateEndChangeSignal( newDate, childNumber() );
}

int AccountingTAMBillItem::columnCount() const {
    return m_d->colCount;
}

QVariant AccountingTAMBillItem::data(int col, int role) const {
    if( (col > m_d->colCount) || (col < 0) ||
            (role != Qt::DisplayRole && role != Qt::EditRole &&
             role != Qt::TextAlignmentRole && role != Qt::FontRole) ){
        return QVariant();
    }

    if( m_d->itemType == AccountingTAMBillItem::Comment ) {
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
        } else if( col == m_d->startDateCol) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( startDateStr() );
            }
        } else if( col == m_d->endDateCol) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( endDateStr() );
            }
        } else if( col == m_d->priceCodeCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                if( m_d->priceItem != nullptr ){
                    return QVariant( m_d->priceItem->codeFull() );
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceShortDescCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignLeft + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                if( m_d->priceItem != nullptr ){
                    return QVariant(m_d->priceItem->shortDescriptionFull());
                } else {
                    return QVariant("---");
                }
            }
        } else if( col == m_d->priceUmCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                if( m_d->priceItem != nullptr ){
                    if( m_d->priceItem->unitMeasure() != nullptr ){
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
        } else if( col == m_d->startDateCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("Inizio") );
            }
        } else if( col == m_d->endDateCol ) {
            if( role == Qt::TextAlignmentRole ){
                return Qt::AlignHCenter + Qt::AlignVCenter;;
            } else if(role == Qt::DisplayRole || role == Qt::EditRole) {
                return QVariant( trUtf8("Fine") );
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

bool AccountingTAMBillItem::setData(int column, const QVariant &value) {
    if( m_d->itemType == AccountingTAMBillItem::Comment ) {
        if( column == m_d->priceShortDescCol ){
            setText( value.toString() );
            return true;
        }
    } else if( m_d->itemType == AccountingTAMBillItem::PPU ){
        if( column == m_d->quantityCol && m_d->measuresModel == nullptr ){
            setQuantity( value.toString() );
            emit dataChanged( this, m_d->PPUTotalToDiscountCol);
            emit dataChanged( this, m_d->totalAmountToDiscountCol);
            emit dataChanged( this, m_d->PPUNotToDiscountCol);
            emit dataChanged( this, m_d->amountNotToDiscountCol);
            emit dataChanged( this, m_d->totalAmountCol);
            return true;
        } else {
            if( column == m_d->startDateCol ){
                setStartDate( value.toString() );
                return true;
            }
            if( column == m_d->endDateCol ){
                setEndDate( value.toString() );
                return true;
            }
        }
    }
    return false;
}

int AccountingTAMBillItem::currentPriceDataSet() const{
    if( m_d->itemType == Root ){
        return m_d->currentPriceDataSet;
    } else if( m_d->parentItem ){
        return m_d->parentItem->currentPriceDataSet();
    }
    return 0;
}

double AccountingTAMBillItem::discount() const {
    if( m_d->itemType == Root ){
        return m_d->discount;
    } else if( m_d->parentItem ){
        return m_d->parentItem->discount();
    }
    return 0.0;
}

QString AccountingTAMBillItem::discountStr() const {
    return QString("%1 %").arg( m_d->toString( discount() * 100.0, m_d->discountPrecision - 2 ) );
}

void AccountingTAMBillItem::setCurrentPriceDataSet(int newVal ) {
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

void AccountingTAMBillItem::setDiscount(double newValPurp ) {
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

void AccountingTAMBillItem::setDiscount(const QString &newVal) {
    QString v = newVal;
    v.remove("%");
    if( m_d->parser != nullptr ){
        setDiscount( m_d->parser->evaluate( v ) / 100.0 );
    } else {
        setDiscount( v.toDouble() / 100.0 );
    }
}

void AccountingTAMBillItem::updateTotalAmountToDiscount() {
    double v = 0.0;
    if( (m_d->itemType == Root) || (m_d->itemType == Payment)){
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->updateTotalAmountToDiscount();
            v += (*i)->totalAmountToDiscount();
        }
    } else if( m_d->itemType == PPU ) {
        v = UnitMeasure::applyPrecision( m_d->PPUTotalToDiscount * m_d->quantity, m_d->amountPrecision );
    } else if(m_d->itemType == Comment) {
        v = 0.0;
    }
    if( v != m_d->totalAmountToDiscount ){
        m_d->totalAmountToDiscount = v;
        emit totalAmountToDiscountChanged( totalAmountToDiscountStr() );
    }
}

void AccountingTAMBillItem::updateAmountNotToDiscount() {
    double v = 0.0;
    if( (m_d->itemType == Root) || (m_d->itemType == Payment)){
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->updateAmountNotToDiscount();
            v += (*i)->amountNotToDiscount();
        }
    } else if( m_d->itemType == PPU ) {
        v = UnitMeasure::applyPrecision( m_d->PPUNotToDiscount * m_d->quantity, m_d->amountPrecision );
    } else if(m_d->itemType == Comment) {
        v = 0.0;
    }
    if( v != m_d->amountNotToDiscount ){
        m_d->amountNotToDiscount = v;
        emit amountNotToDiscountChanged( amountNotToDiscountStr() );
    }
}

void AccountingTAMBillItem::updateAmountToDiscount() {
    double v = m_d->totalAmountToDiscount - m_d->amountNotToDiscount;
    if( v != m_d->amountToDiscount ){
        m_d->amountToDiscount = v;
        emit amountToDiscountChanged( amountToDiscountStr() );
    }
}

void AccountingTAMBillItem::updateAmountDiscounted() {
    double v = UnitMeasure::applyPrecision( m_d->amountToDiscount * (1.0 - discount() ), m_d->amountPrecision );;
    if( v != m_d->amountDiscounted ){
        m_d->amountDiscounted = v;
        emit amountDiscountedChanged( amountDiscountedStr() );
    }
}

void AccountingTAMBillItem::updateTotalAmount() {
    double v = m_d->amountDiscounted + m_d->amountNotToDiscount;
    if( v != m_d->totalAmount ) {
        m_d->totalAmount = v;
        emit totalAmountChanged( totalAmountStr() );
    }
}

bool AccountingTAMBillItem::isUsingPriceItem(PriceItem *p) {
    if( (m_d->itemType == Root) || (m_d->itemType == Payment) ){
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
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

QList<PriceItem *> AccountingTAMBillItem::usedPriceItems() const {
    QList<PriceItem *> ret;
    appendUsedPriceItems( &ret );
    return ret;
}

void AccountingTAMBillItem::appendUsedPriceItems( QList<PriceItem *> * usedPriceItems ) const {
    if( (m_d->itemType == Root) || (m_d->itemType == Payment)){
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->appendUsedPriceItems( usedPriceItems );
        }
    } else if( m_d->itemType == PPU ){
        if( !usedPriceItems->contains( m_d->priceItem )){
            usedPriceItems->append( m_d->priceItem );
        }
    }
}

QList<AccountingTAMBillItem *> AccountingTAMBillItem::usedItemsPayment( int firstPay, int lastPay, ItemType iType ) const {
    QList<AccountingTAMBillItem *> ret;
    if( m_d->itemType == Root ){
        if( firstPay < 0 ){
            firstPay = 0;
        }
        if( lastPay >= m_d->childrenContainer.size() ){
            lastPay = m_d->childrenContainer.size() - 1;
        }
        for( int i=firstPay; i <= lastPay; i++){
            for( QList<AccountingTAMBillItem *>::iterator line = m_d->childrenContainer.at(i)->m_d->childrenContainer.begin(); line != m_d->childrenContainer.at(i)->m_d->childrenContainer.end(); ++line){
                if( (*line)->itemType() == iType &&
                        !(ret.contains( (*line) ) ) ){
                    ret << (*line);
                }
            }
        }
    }
    return ret;
}

void AccountingTAMBillItem::setHasChildrenChanged(AccountingTAMBillItem *p, QList<int> indexes) {
    if( m_d->parentItem ){
        // non è l'oggetto root - rimandiamo all'oggetto root
        m_d->parentItem->setHasChildrenChanged( p, indexes );
    } else {
        // è l'oggetto root - emette il segnale di numero figli cambiato
        emit hasChildrenChanged( p, indexes );
    }
}

TreeItem *AccountingTAMBillItem::child(int number) {
    if( number >= 0 && number < m_d->childrenContainer.size() ){
        return m_d->childrenContainer[number];
    }
    return NULL;
}

int AccountingTAMBillItem::childrenCount() const {
    return m_d->childrenContainer.size();
}

bool AccountingTAMBillItem::hasChildren() const {
    return (m_d->childrenContainer.size() > 0);
}

QList<AccountingTAMBillItem *> AccountingTAMBillItem::allChildren() {
    QList<AccountingTAMBillItem *> ret;
    for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        ret.append( *i );
        if( (*i)->hasChildren() ){
            ret.append( (*i)->allChildren() );
        }
    }
    return ret;
}

QList<AccountingTAMBillItem *> AccountingTAMBillItem::allChildrenWithMeasures() {
    QList<AccountingTAMBillItem *> ret;
    for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
        if( (*i)->hasChildren() ){
            ret.append( (*i)->allChildrenWithMeasures() );
        } else {
            ret.append( this );
        }
    }
    return ret;
}

bool AccountingTAMBillItem::insertChildren(int position, int count) {
    if( m_d->itemType == Root ){
        return insertChildren( Payment, position, count );
    }
    if( m_d->itemType == Payment ){
        return insertChildren( PPU, position, count );
    }
    return false;
}


bool AccountingTAMBillItem::insertChildren(AccountingTAMBillItem::ItemType iType, int position, int count ){
    if (position < 0 || position > m_d->childrenContainer.size() )
        return false;

    if( (m_d->itemType == Root && (iType == Payment) ) ||
            (m_d->itemType == Payment && (iType == PPU || iType == Comment)) ){

        bool hadChildren = m_d->childrenContainer.size() > 0;

        for (int row = 0; row < count; ++row) {
            AccountingTAMBillItem * item = new AccountingTAMBillItem( this, iType,  m_d->priceFieldModel, m_d->parser );
            while( findItemFromId( item->id() ) != nullptr ){
                item->setId( item->id() + 1 );
            }
            m_d->childrenContainer.insert(position, item);
            if( iType == Payment ){
                for( int j = position+1; j < m_d->childrenContainer.size(); ++j  ){
                    m_d->childrenContainer.at(j)->emitTitleChanged();
                }
            }
            connect( item, static_cast<void(AccountingTAMBillItem::*)(AccountingTAMBillItem*,int)> (&AccountingTAMBillItem::dataChanged), this, static_cast<void(AccountingTAMBillItem::*)(AccountingTAMBillItem*,int)> (&AccountingTAMBillItem::dataChanged) );
            connect( item, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBillItem::updateTotalAmountToDiscount );
            connect( item, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBillItem::updateAmountNotToDiscount );
            connect( this, &AccountingTAMBillItem::currentPriceDataSetChanged, item, &AccountingTAMBillItem::setCurrentPriceDataSet );
            connect( item, &AccountingTAMBillItem::itemChanged, this, &AccountingTAMBillItem::itemChanged );
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

bool AccountingTAMBillItem::appendChildren(ItemType iType, int count) {
    return insertChildren( iType, m_d->childrenContainer.size(), count );
}

bool AccountingTAMBillItem::removeChildren(int position, int count) {
    if( count <= 0 ){
        return true;
    }

    if (position < 0 || position + count > m_d->childrenContainer.size())
        return false;

    bool hadChildren = m_d->childrenContainer.size() > 0;

    for (int row = 0; row < count; ++row){
        AccountingTAMBillItem * itemToRemove = m_d->childrenContainer.at( position );
        disconnect( itemToRemove, static_cast<void(AccountingTAMBillItem::*)(AccountingTAMBillItem*,int)> (&AccountingTAMBillItem::dataChanged), this, static_cast<void(AccountingTAMBillItem::*)(AccountingTAMBillItem*,int)> (&AccountingTAMBillItem::dataChanged) );
        disconnect( itemToRemove, &AccountingTAMBillItem::totalAmountToDiscountChanged, this, &AccountingTAMBillItem::updateTotalAmountToDiscount );
        disconnect( itemToRemove, &AccountingTAMBillItem::amountNotToDiscountChanged, this, &AccountingTAMBillItem::updateAmountNotToDiscount );
        disconnect( this, &AccountingTAMBillItem::currentPriceDataSetChanged, itemToRemove, &AccountingTAMBillItem::setCurrentPriceDataSet );
        disconnect( itemToRemove, &AccountingTAMBillItem::itemChanged, this, &AccountingTAMBillItem::itemChanged );
        AccountingTAMBillItem::ItemType itemType = itemToRemove->itemType();
        delete itemToRemove;
        m_d->childrenContainer.removeAt( position );
        if( itemType == Payment ){
            emit paymentRemoved( position, itemToRemove );
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

bool AccountingTAMBillItem::clear() {
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

int AccountingTAMBillItem::childNumber() const {
    if (m_d->parentItem)
        return m_d->parentItem->m_d->childrenContainer.indexOf( const_cast<AccountingTAMBillItem *>(this) );

    return 0;
}

Qt::ItemFlags AccountingTAMBillItem::flags(int column) const {
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
            if( m_d->measuresModel == nullptr ){
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
            } else {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            }
        }
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void AccountingTAMBillItem::writeXml20(QXmlStreamWriter *writer) {

    if( m_d->itemType == Root ){
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml20( writer );
        }
    } else if( m_d->itemType == Payment ){
        writer->writeStartElement( "AccountingTAMBillItem" );
        writer->writeAttribute( "itemType", QString("Payment") );
        writer->writeAttribute( "startDate", QString::number( m_d->startDate->toJulianDay() ) );
        writer->writeAttribute( "endDate", QString::number( m_d->endDate->toJulianDay() ) );
        QString attrs = m_d->attributesString();
        if( !attrs.isEmpty() ){
            writer->writeAttribute( "attributes", attrs );
        }
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml20( writer );
        }
        writer->writeEndElement();
    } else if( m_d->itemType == PPU ){
        writer->writeStartElement( "AccountingTAMBillItem" );
        writer->writeAttribute( "itemType", QString("PPU") );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        if( m_d->priceItem != nullptr ){
            writer->writeAttribute( "priceItem", QString::number( m_d->priceItem->id() ) );
        }
        QString attrs = m_d->attributesString();
        if( !attrs.isEmpty() ){
            writer->writeAttribute( "attributes", attrs );
        }

        if( m_d->measuresModel != nullptr ){
            m_d->measuresModel->writeXml20( writer );
        } else {
            writer->writeAttribute( "quantity", QString::number( m_d->quantity ) );
        }

        writer->writeEndElement();
    } else if( m_d->itemType == Comment ){
        writer->writeStartElement( "AccountingTAMBillItem" );
        writer->writeAttribute( "itemType", QString("Comment") );
        writer->writeAttribute( "id", QString::number(m_d->id) );
        writer->writeAttribute( "text", m_d->text );
        writer->writeEndElement();
    }
}

void AccountingTAMBillItem::readXmlTmp20( QXmlStreamReader *reader ) {
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
           !(reader->isEndElement() && tagUp == "ACCOUNTINGTAMBILLITEM")&&
           !(reader->isEndElement() && tagUp == "ACCOUNTINGTAMBILL")  ){
        if( m_d->itemType == Root ){
            if( tagUp == "ACCOUNTINGTAMBILLITEM" && reader->isStartElement()) {
                if( reader->attributes().hasAttribute( "itemType" ) ){
                    if( reader->attributes().value( "itemType" ).toString().toUpper() == "PAYMENT" ){
                        appendChildren( Payment );
                        m_d->childrenContainer.last()->readXmlTmp20( reader );
                    }
                }
            }
        } else if( m_d->itemType == Payment ){
            if( tagUp == "ACCOUNTINGTAMBILLITEM" && reader->isStartElement()) {
                if( reader->attributes().hasAttribute( "itemType" ) ){
                    AccountingTAMBillItem::ItemType iType = PPU;
                    if( reader->attributes().value( "itemType" ).toString().toUpper() == "COMMENT" ){
                        iType = Comment;
                    } else if( reader->attributes().value( "itemType" ).toString().toUpper() == "PPU" ){
                        iType = PPU;
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

void AccountingTAMBillItem::readFromXmlTmp20( PriceList * priceList, AttributesModel * billAttrModel ) {
    if( !m_d->tmpAttributes.isEmpty() ){
        loadFromXml( m_d->tmpAttributes, priceList, billAttrModel );
        m_d->tmpAttributes.clear();
        if( m_d->measuresModel != nullptr ){
            m_d->measuresModel->readFromXmlTmp20();
        }
    }
    for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
        (*i)->readFromXmlTmp20( priceList, billAttrModel );
    }
}

void AccountingTAMBillItem::loadFromXml(const QXmlStreamAttributes &attrs,
                                        PriceList * priceList,
                                        AttributesModel * attrModel ) {
    if( attrs.hasAttribute( "id" ) ){
        m_d->id = attrs.value( "id").toUInt();
    }
    if( attrs.hasAttribute( "startDate" ) ){
        if( m_d->itemType == Payment ){
            *(m_d->startDate) = QDate::fromJulianDay( attrs.value( "startDate").toString().toLongLong() );
        }
    }
    if( attrs.hasAttribute( "endDate" ) ){
        if( m_d->itemType == Payment ){
            *(m_d->endDate) = QDate::fromJulianDay( attrs.value( "endDate").toString().toLongLong() );
        }
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
                if( attr != nullptr ) {
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
    }
}

QList< QPair<Attribute *, bool> > AccountingTAMBillItem::attributes() {
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

QList<Attribute *> AccountingTAMBillItem::allAttributes() {
    QList<Attribute *> attrs = inheritedAttributes();
    for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
        if( !attrs.contains( *i) ){
            attrs.append( *i );
        }
    }
    return attrs;
}

QList<Attribute *> AccountingTAMBillItem::directAttributes() const {
    return QList<Attribute *>( m_d->attributes );
}

void AccountingTAMBillItem::addAttribute( Attribute * attr ){
    if( !(m_d->attributes.contains( attr )) ){
        m_d->attributes.append( attr );
        emit attributesChanged();
    }
}

void AccountingTAMBillItem::removeAttribute( Attribute * attr ){
    if( m_d->attributes.removeAll( attr ) > 0 ){
        emit attributesChanged();
    }
}

void AccountingTAMBillItem::removeAllAttributes() {
    m_d->attributes.clear();
    emit attributesChanged();
}

double AccountingTAMBillItem::totalAmountToDiscountAttribute(Attribute *attr ) const {
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
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmountToDiscountAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingTAMBillItem::totalAmountToDiscountAttributeStr(Attribute *attr ) const {
    int prec = m_d->amountPrecision;
    QString ret;
    double v = totalAmountToDiscountAttribute( attr );
    if( m_d->parser == nullptr ){
        ret = QString::number(v, 'f', prec );
    } else {
        ret = m_d->parser->toString( v, 'f', prec );
    }
    return ret;
}

double AccountingTAMBillItem::amountNotToDiscountAttribute(Attribute *attr ) const {
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
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountNotToDiscountAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingTAMBillItem::amountNotToDiscountAttributeStr(Attribute *attr ) const {
    int prec = m_d->amountPrecision;
    QString ret;
    double v = amountNotToDiscountAttribute( attr );
    if( m_d->parser == nullptr ){
        ret = QString::number(v, 'f', prec );
    } else {
        ret = m_d->parser->toString( v, 'f', prec );
    }
    return ret;
}

double AccountingTAMBillItem::totalAmountAttribute(Attribute *attr ) const {
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
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmountAttribute( attr );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingTAMBillItem::totalAmountAttributeStr(Attribute *attr ) const {
    int prec = m_d->amountPrecision;
    QString ret;
    double v = totalAmountAttribute( attr );
    if( m_d->parser == nullptr ){
        ret = QString::number(v, 'f', prec );
    } else {
        ret = m_d->parser->toString( v, 'f', prec );
    }
    return ret;
}

QList<Attribute *> AccountingTAMBillItem::inheritedAttributes() const {
    QList<Attribute *> ret;
    if( m_d->parentItem != nullptr ){
        ret.append( m_d->parentItem->inheritedAttributes() );
    }
    for( QList<Attribute *>::iterator i = m_d->attributes.begin(); i != m_d->attributes.end(); ++i ){
        if( !ret.contains( *i) ){
            ret.append( *i );
        }
    }
    return ret;
}

bool AccountingTAMBillItem::containsAttribute(Attribute *attr) const {
    if( containsAttributeDirect(attr)){
        return true;
    }
    return containsAttributeInherited( attr );
}

bool AccountingTAMBillItem::containsAttributeInherited(Attribute *attr) const {
    if( m_d->parentItem != nullptr ){
        if( m_d->parentItem->containsAttributeDirect( attr ) ){
            return true;
        } else {
            return m_d->parentItem->containsAttributeInherited( attr );
        }
    }
    return false;
}

bool AccountingTAMBillItem::containsAttributeDirect(Attribute *attr) const {
    return m_d->attributes.contains( attr );
}

bool AccountingTAMBillItem::isDescending(AccountingTAMBillItem *ancestor) {
    if( m_d->parentItem == nullptr ){
        return (m_d->parentItem == ancestor);
    } else {
        if( m_d->parentItem == ancestor ){
            return true;
        } else {
            return m_d->parentItem->isDescending( ancestor );
        }
    }
}

QList<PriceItem *> AccountingTAMBillItem::connectedPriceItems() const {
    QList<PriceItem *> ret;

    if( m_d->itemType == AccountingTAMBillItem::PPU ){
        return m_d->priceItem->connectedPriceItems();
    }

    if( (m_d->itemType == AccountingTAMBillItem::Root) ||
            (m_d->itemType == AccountingTAMBillItem::Payment)   ){
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
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


double AccountingTAMBillItem::totalAmountToDiscount() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->totalAmountToDiscount;
    }
    return 0.0;
}

double AccountingTAMBillItem::totalAmountToDiscount(AccountingTAMBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return totalAmountToDiscount();
    } else {
        double ret = 0.0;
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmountToDiscount( iType );
        }
        return ret;
    }
    return 0.0;
}

double AccountingTAMBillItem::totalAmountToDiscountPayment(int firstPay, int lastPay, AccountingTAMBillItem::ItemType iType) const {
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

double AccountingTAMBillItem::amountNotToDiscount() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->amountNotToDiscount;
    }
    return 0.0;
}

double AccountingTAMBillItem::amountNotToDiscount(AccountingTAMBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return amountNotToDiscount();
    } else if( !(m_d->childrenContainer.isEmpty()) ){
        double ret = 0.0;
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountNotToDiscount( iType );
        }
        return ret;
    }
    return 0.0;
}

double AccountingTAMBillItem::amountNotToDiscountPayment(int firstPay, int lastPay, AccountingTAMBillItem::ItemType iType ) const {
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

double AccountingTAMBillItem::amountToDiscount() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->amountToDiscount;
    }
    return 0.0;
}

double AccountingTAMBillItem::amountToDiscount(AccountingTAMBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return amountToDiscount();
    } else {
        double ret = 0.0;
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountToDiscount( iType );
        }
        return ret;
    }
    return 0.0;
}

double AccountingTAMBillItem::amountDiscounted() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->amountDiscounted;
    }
    return 0.0;
}

double AccountingTAMBillItem::amountDiscounted(AccountingTAMBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return amountDiscounted();
    } else {
        double ret = 0.0;
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->amountDiscounted( iType );
        }
        return ret;
    }
    return 0.0;
}

double AccountingTAMBillItem::totalAmount() const {
    if( (m_d->itemType == PPU) || (m_d->itemType == Payment) || (m_d->itemType == Root) ){
        return m_d->totalAmount;
    }
    return 0.0;
}

double AccountingTAMBillItem::totalAmount(AccountingTAMBillItem::ItemType iType) const {
    if( iType == m_d->itemType ){
        return totalAmount();
    } else {
        double ret = 0.0;
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            ret += (*i)->totalAmount( iType );
        }
        return ret;
    }
    return 0.0;
}

QString AccountingTAMBillItem::totalAmountToDiscountStr() const {
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( totalAmountToDiscount(), 'f', m_d->amountPrecision );
}

QString AccountingTAMBillItem::amountNotToDiscountStr() const{
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( amountNotToDiscount(), 'f', m_d->amountPrecision );
}

QString AccountingTAMBillItem::amountToDiscountStr() const {
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( amountToDiscount(), 'f', m_d->amountPrecision );
}

QString AccountingTAMBillItem::amountDiscountedStr() const {
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( amountDiscounted(), 'f', m_d->amountPrecision );
}

QString AccountingTAMBillItem::totalAmountStr() const {
    if( m_d->itemType == Comment ){
        return QString();
    }
    return m_d->toString( totalAmount(), 'f', m_d->amountPrecision );
}

QString AccountingTAMBillItem::amountStr(int i ) const {
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

QString AccountingTAMBillItem::text() const {
    return m_d->text;
}

void AccountingTAMBillItem::setText( const QString &t ){
    if( t !=  m_d->text ){
        m_d->text = t;
        emit itemChanged();
        emit textChanged(m_d->text);
        emit dataChanged( this, m_d->priceShortDescCol );
    }
}

PriceItem *AccountingTAMBillItem::priceItem() {
    return m_d->priceItem;
}

void AccountingTAMBillItem::setPriceItem(PriceItem * p) {
    if( m_d->priceItem != p ){
        if( m_d->priceItem != nullptr ) {
            disconnect( m_d->priceItem, &PriceItem::codeFullChanged, this, &AccountingTAMBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &AccountingTAMBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &AccountingTAMBillItem::emitPriceDataUpdated );
            disconnect( m_d->priceItem, &PriceItem::valueChanged, this, &AccountingTAMBillItem::updatePPUs );
        }
        PriceItem * oldPriceItem = m_d->priceItem;

        m_d->priceItem = p;

        emit priceItemChanged( oldPriceItem, p );

        if( m_d->priceItem != nullptr ){
            connect( m_d->priceItem, &PriceItem::codeFullChanged, this, &AccountingTAMBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::shortDescriptionFullChanged, this, &AccountingTAMBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::unitMeasureChanged, this, &AccountingTAMBillItem::emitPriceDataUpdated );
            connect( m_d->priceItem, &PriceItem::valueChanged, this, &AccountingTAMBillItem::updatePPUs );
        }

        emit itemChanged();
        emit dataChanged( this, m_d->priceCodeCol );
        emit dataChanged( this, m_d->priceUmCol );
        emit dataChanged( this, m_d->PPUTotalToDiscountCol );
        emit dataChanged( this, m_d->totalAmountToDiscountCol );
        emit dataChanged( this, m_d->PPUNotToDiscountCol );
        emit dataChanged( this, m_d->amountNotToDiscountCol );
        emit dataChanged( this, m_d->totalAmountCol );

        if( m_d->measuresModel != nullptr ){
            m_d->measuresModel->setUnitMeasure( m_d->priceItem->unitMeasure() );
        }

        updatePPUs();
    }
}

double AccountingTAMBillItem::quantity() const {
    if( m_d->childrenContainer.size() > 0 ){
        return 0.0;
    } else {
        return m_d->quantity;
    }
}

QString AccountingTAMBillItem::quantityStr() const {
    if( m_d->itemType == PPU ){
        int prec = 2;
        if( m_d->priceItem != nullptr ){
            if( m_d->priceItem->unitMeasure() != nullptr ){
                prec = m_d->priceItem->unitMeasure()->precision();
            }
        }
        return m_d->toString( quantity(), 'f', prec );
    }
    return QString();
}

void AccountingTAMBillItem::setQuantityPrivate(double v) {
    if( v != m_d->quantity ){
        m_d->quantity = v;
        emit quantityChanged( quantityStr() );
        emit itemChanged();
        emit dataChanged( this, m_d->quantityCol );
        updateTotalAmountToDiscount();
        updateAmountNotToDiscount();
    }
}

void AccountingTAMBillItem::setQuantity(double v) {
    if( m_d->measuresModel == nullptr ){
        setQuantityPrivate( v );
    }
}

void AccountingTAMBillItem::setQuantity(const QString &vstr ) {
    if( m_d->parser != nullptr ){
        setQuantity( m_d->parser->evaluate( vstr ) );
    } else {
        setQuantity( vstr.toDouble() );
    }
}

double AccountingTAMBillItem::PPUTotalToDiscount() const {
    return m_d->PPUTotalToDiscount;
}

double AccountingTAMBillItem::PPUNotToDiscount() const {
    return m_d->PPUNotToDiscount;
}

QString AccountingTAMBillItem::PPUTotalToDiscountStr() const {
    return m_d->toString( PPUTotalToDiscount(), 'f', m_d->amountPrecision );
}

QString AccountingTAMBillItem::PPUNotToDiscountStr() const {
    return m_d->toString( PPUNotToDiscount(), 'f', m_d->amountPrecision );
}

void AccountingTAMBillItem::emitPriceDataUpdated() {
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

void AccountingTAMBillItem::emitTitleChanged() {
    emit titleChanged( title() );
    emit dataChanged( this, m_d->priceShortDescCol );
}

AccountingTAMMeasuresModel *AccountingTAMBillItem::measuresModel() {
    return m_d->measuresModel;
}

AccountingTAMMeasuresModel *AccountingTAMBillItem::generateMeasuresModel() {
    if( m_d->measuresModel == nullptr ){
        UnitMeasure * ump = NULL;
        if( m_d->priceItem != nullptr ){
            ump = m_d->priceItem->unitMeasure();
        }
        m_d->measuresModel = new AccountingTAMMeasuresModel( this, m_d->parser, ump );
        setQuantity( m_d->measuresModel->quantity() );
        connect( m_d->measuresModel, &AccountingTAMMeasuresModel::quantityChanged, this, &AccountingTAMBillItem::setQuantityPrivate );
        connect( m_d->measuresModel, &AccountingTAMMeasuresModel::modelChanged, this, &AccountingTAMBillItem::itemChanged );
        emit itemChanged();
    }
    return m_d->measuresModel;
}

void AccountingTAMBillItem::removeMeasuresModel() {
    if( m_d->measuresModel != nullptr ){
        disconnect( m_d->measuresModel, &AccountingTAMMeasuresModel::quantityChanged, this, &AccountingTAMBillItem::setQuantityPrivate );
        disconnect( m_d->measuresModel, &AccountingTAMMeasuresModel::modelChanged, this, &AccountingTAMBillItem::itemChanged );
        delete m_d->measuresModel;
        m_d->measuresModel = NULL;
        updateTotalAmountToDiscount();
        updateAmountNotToDiscount();
    }
}

#include "qtextformatuserdefined.h"

void AccountingTAMBillItem::writeODTAccountingOnTable( QTextCursor *cursor,
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
    int cellCount = 7 + m_d->daysCount;

    // puntatore alla tabella (comodita')
    QTextTable *table = cursor->currentTable();

    if( m_d->itemType == Root ){
        // *** Riga di intestazione ***
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N.Ord."), false );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
        QDate dateToPrint = *(m_d->startDate);
        for( int i=0; i < m_d->daysCount; ++i ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, dateToPrint.toString( "dd/MM") );
            dateToPrint = dateToPrint.addDays( 1 );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));

        // *** Riga vuota ***
        AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        // *** Scrive il computo dei sottoarticoli ***
        if( payToPrint < 0 ){
            for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTAccountingOnTable( cursor, 0, prPPUDescOption );
            }
        } else {
            if( payToPrint < m_d->childrenContainer.size() ){
                m_d->childrenContainer.at(payToPrint)->writeODTAccountingOnTable( cursor, 0, prPPUDescOption );
            }
        }

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        for( int i=0; i < m_d->daysCount; ++i ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        for( int i=0; i < m_d->daysCount; ++i ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountNotToDiscountStr() );

        // *** Riga di chiusura ***
        AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );

    } else {
        if( m_d->itemType == Payment ){

            // ci sono sottoarticoli
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, title() );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            for( int i=0; i < m_d->daysCount; ++i ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            }
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );

            for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTAccountingOnTable( cursor, 0, prPPUDescOption );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale lordo %1").arg( title() ) );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            for( int i=0; i < m_d->daysCount; ++i ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            }
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

            if( amountNotToDiscount() != 0.0 ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                for( int i=0; i < m_d->daysCount; ++i ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                }
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountNotToDiscountStr() );
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
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingTAMBillItem::writeODTMeasuresOnTable( QTextCursor *cursor, int payToPrint,
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
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N."), false);
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Data") );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        } else {
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));
        }

        // *** Riga vuota ***
        AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        // *** Scrive il computo dei sottoarticoli ***
        if( payToPrint < 0 ){
            for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
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
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountNotToDiscountStr() );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale soggetto a ribasso lordo") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountToDiscountStr() );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale soggetto a ribasso netto") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountDiscountedStr() );
            // *** Riga vuota ***
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale complessivo") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountStr() );
        } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, amountNotToDiscountStr() );

        } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

        }

        // *** Riga di chiusura ***
        AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );

    } else {
        if( m_d->itemType == Payment ){

            // ci sono sottoarticoli
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, title() );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            } else {
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            }

            // *** Riga vuota ***
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
                (*i)->writeODTMeasuresOnTable( cursor, 0, prAmountsOption, prPPUDescOption );
            }

            // *** Riga vuota ***
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat );
            } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso %1").arg( title() ) );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountNotToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale lordo %1").arg( title() ) );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountNotToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Totale lordo %1").arg( title() ) );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, totalAmountToDiscountStr() );

                if( amountNotToDiscount() != 0.0 ){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );

                    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, amountNotToDiscountStr() );
                }
            }
            // *** Riga vuota ***
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
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
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingTAMBillItem::writeODTPaymentOnTable( QTextCursor *cursor,
                                                    int payToPrint,
                                                    AccountingPrinter::PrintPPUDescOption prPPUDescOption  ) const {
    writeODTSummaryOnTable( cursor, payToPrint, AccountingPrinter::PrintAllAmounts, prPPUDescOption, false );
}

void AccountingTAMBillItem::writeODTAccountingSummaryOnTable( QTextCursor *cursor,
                                                              int payToPrint,
                                                              AccountingPrinter::PrintAmountsOption prAmountsOption,
                                                              AccountingPrinter::PrintPPUDescOption prPPUDescOption ) const {
    writeODTSummaryOnTable( cursor, payToPrint, prAmountsOption, prPPUDescOption, true );
}

void AccountingTAMBillItem::writeODTSummaryOnTable( QTextCursor *cursor,
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
    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("N.Ord."), false);
    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Art.Elenco") );
    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Indicazione dei lavori"));
    if( writeAccountingProgCode ){
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("N.Ord. Reg."));
    }
    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    } else {
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));
    }

    // *** Riga vuota ***
    AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

    // *** Scrive le righe del S.A.L. ***

    // se payToPrint è negativo, stampa l'ultimo SAL
    if( payToPrint < 0 ){
        payToPrint = m_d->childrenContainer.size() - 1;
    }

    double totAmToDiscount = 0.0;
    double amNotToDiscount = 0.0;
    int nProg = 1;

    /* *** OPERE A MISURA *** */
    QList<AccountingTAMBillItem *> usedItemsList = usedItemsPayment( 0, payToPrint, PPU );
    if( !usedItemsList.isEmpty() ){
        QList<PriceItem *> usedPItemsList;
        for( QList<AccountingTAMBillItem *>::iterator i=m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            QList<PriceItem *> payUsedPItems = (*i)->usedPriceItems();
            for( QList<PriceItem *>::iterator usedPItem=payUsedPItems.begin(); usedPItem != payUsedPItems.end(); ++usedPItem ){
                if( !usedPItemsList.contains(*usedPItem ) && ((*usedPItem) != NULL)){
                    usedPItemsList << *usedPItem;
                }
            }
        }
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("OPERE A MISURA") );
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        } else {
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
        }
        AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

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

            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, QString::number(nProg++) );
            if( (*pItem) != nullptr ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, (*pItem)->codeFull() );
                m_d->writeDescriptionCell( (*pItem), cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtBoldCharFormat, prPPUDescOption );
            } else {
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            }

            QString unitMeasureTag;
            int unitMeasurePrec = 3;
            if( (*pItem) != nullptr ){
                if( (*pItem)->unitMeasure() != nullptr ){
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
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            }
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
            } else {
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( pricePPUTotalToDiscount, 'f', m_d->amountPrecision ) );
                double itemTotAmToDiscount = 0.0;
                if( prAmountsOption == AccountingPrinter::PrintAllAmounts || prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                    itemTotAmToDiscount = UnitMeasure::applyPrecision( pricePPUTotalToDiscount * itemTotalQuantity, m_d->amountPrecision);
                } else { // prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount
                    itemTotAmToDiscount = UnitMeasure::applyPrecision( pricePPUNotToDiscount * itemTotalQuantity, m_d->amountPrecision);
                }
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemTotAmToDiscount, 'f', m_d->amountPrecision ) );
            }

            if( pricePPUNotToDiscount != 0.0 && m_d->discount != 0.0 &&
                    prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                amNotToDiscountPresent = true;

                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, trUtf8("di cui non soggetto a ribasso") );
                if( writeAccountingProgCode ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                }
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( itemTotalQuantity, 'f', unitMeasurePrec ));
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, m_d->toString( pricePPUNotToDiscount, 'f', m_d->amountPrecision ) );
                double itemAmNotToDiscount = UnitMeasure::applyPrecision( pricePPUNotToDiscount * itemTotalQuantity, m_d->amountPrecision);
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, m_d->toString( itemAmNotToDiscount, 'f', m_d->amountPrecision ) );
                AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
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
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                if( m_d->discount != 0.0 ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a misura - Totale al lordo del ribasso") );
                } else {
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a misura - Totale") );
                }
                if( writeAccountingProgCode ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                }
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, m_d->toString( ppuTotAmToDiscount, 'f', m_d->amountPrecision ) );
            }

            if( (amNotToDiscountPresent && prAmountsOption == AccountingPrinter::PrintAllAmounts) ||
                    (prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount) ){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, txtBlockFormat, trUtf8("Opere a misura - Totale non soggetto a ribasso") );
                if( writeAccountingProgCode ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                }
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralSubTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightSubTitleFormat, numBlockFormat, m_d->toString( ppuAmNotToDiscount, 'f', m_d->amountPrecision ) );
            }
        }

        AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
    }

    /* *** TOTALI COMPLESSIVI *** */

    if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totAmToDiscount, 'f', m_d->amountPrecision ) );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amNotToDiscount, 'f', m_d->amountPrecision ) );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale soggetto a ribasso lordo") );
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        double amToDiscount = UnitMeasure::applyPrecision( totAmToDiscount - amNotToDiscount, m_d->amountPrecision);
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amToDiscount, 'f', m_d->amountPrecision ) );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale soggetto a ribasso netto") );
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        double amDiscounted = UnitMeasure::applyPrecision( amToDiscount * (1.0 - discount() ), m_d->amountPrecision );;
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amDiscounted, 'f', m_d->amountPrecision ) );
        // *** Riga vuota ***
        AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale") );
        } else {
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale S.A.L.") );
        }
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        double totAm = amNotToDiscount + amDiscounted;
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totAm, 'f', m_d->amountPrecision ) );
    } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale al lordo del ribasso") );
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totAmToDiscount, 'f', m_d->amountPrecision ) );
    } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, trUtf8("Totale non soggetto a ribasso") );
        if( writeAccountingProgCode ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amNotToDiscount, 'f', m_d->amountPrecision ) );
    }

    // *** Riga di chiusura ***
    AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

void AccountingTAMBillItem::writeODTAccountingSummaryLine( PriceItem * priceItem,
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
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, accountingProgCode() );

            QString unitMeasureTag;
            if( m_d->priceItem != nullptr ){
                if( m_d->priceItem->unitMeasure() != nullptr ){
                    unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                }
            }
            // UdM
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );
            if( printAmounts ){
                // quantita
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                // costo unitario
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                // importo
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            } else {
                // quantita
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            // numprog
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            // codice prezzo
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            // descrizione prezzo
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }
    } else {
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTAccountingSummaryLine( priceItem, printedItems, itemTotalQuantity, printAmounts,
                                                 cursor, table,
                                                 tagBlockFormat, txtBlockFormat, numBlockFormat,
                                                 leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingTAMBillItem::writeODTSummaryLine( PriceItem * priceItem,
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
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, progCode() );

                QString unitMeasureTag;
                if( m_d->priceItem != nullptr ){
                    if( m_d->priceItem->unitMeasure() != nullptr ){
                        unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                    }
                }
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                if( printAmounts ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
                } else {
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
                }

                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
            }
        }
    } else {
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
            (*i)->writeODTSummaryLine( priceItem, cursor, itemTotalQuantity, printAmounts, writeDetails,
                                       table,
                                       tagBlockFormat, txtBlockFormat, numBlockFormat,
                                       leftFormat, centralFormat, rightFormat );
        }
    }
}

void AccountingTAMBillItem::writeODTAttributeAccountingOnTable( QTextCursor *cursor,
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
    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat, trUtf8("Codice"), false );
    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Denominazione"));
    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Unità di Misura"));
    if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
    } else {
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Quantità"));
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario"));
        AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo"));
    }

    // *** riga vuota ***
    AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

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

            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat, (*i)->name() );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            } else {
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            }

            // *** riga vuota ***
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );

            writeODTAttributeBillLineSimple( prAmountsOption,
                                             prPPUDescOption,
                                             &totalAmountToDiscountTotal,
                                             &amountNotToDiscountTotal,
                                             *i,
                                             cursor, table, tagBlockFormat, txtBlockFormat, numBlockFormat,
                                             leftFormat, centralFormat, rightFormat, centralQuantityTotalFormat, rightQuantityTotalFormat,
                                             txtCharFormat, txtBoldCharFormat );

            if( discount() == 0.0 ){
                AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
            } else {
                AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
                if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
                }

                if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale non soggetto a ribasso") );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amountNotToDiscountTotal, 'f', m_d->amountPrecision) );
                }
            }

            if( i != -- attrsToPrint.end()){
                // *** riga vuota ***
                AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
                // *** riga vuota ***
                AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            }
        }
    } else if( prOption == AccountingPrinter::AttributePrintUnion ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);

        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        QString title(trUtf8("Unione"));
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            title = QString("%1 %2").arg(title, (*i)->name() );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, title );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        } else {
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo") );
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
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
        } else {
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
            }

            if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale non soggetto a ribasso") );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amountNotToDiscountTotal, 'f', m_d->amountPrecision) );
            }
        }
    } else if( prOption == AccountingPrinter::AttributePrintIntersection ){
        table->appendRows(1);
        cursor->movePosition(QTextCursor::PreviousRow );
        cursor->movePosition(QTextCursor::NextCell);

        AccountingTAMBillItemPrivate::writeCell( cursor, table, leftHeaderFormat, headerBlockFormat );
        QString title(trUtf8("Intersezione"));
        for( QList<Attribute *>::const_iterator i = attrsToPrint.begin(); i != attrsToPrint.end(); ++i){
            title = QString("%1 %2").arg(title, (*i)->name() );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, title );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat );
        } else {
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralHeaderFormat, headerBlockFormat, trUtf8("Costo Unitario") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightHeaderFormat, headerBlockFormat, trUtf8("Importo") );
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
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
        } else {
            AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftFormat, centralFormat, rightFormat );
            if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale al lordo del ribasso") );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( totalAmountToDiscountTotal, 'f', m_d->amountPrecision) );
            }

            if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount || prAmountsOption == AccountingPrinter::PrintAllAmounts){
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftTitleFormat, txtBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, txtBlockFormat , trUtf8("Totale non soggetto a ribasso") );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, tagBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralTitleFormat, numBlockFormat );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightTitleFormat, numBlockFormat, m_d->toString( amountNotToDiscountTotal, 'f', m_d->amountPrecision) );
            }
        }
    }

    // *** Riga di chiusura ***
    AccountingTAMBillItemPrivate::insertEmptyRow( cellCount, cursor, leftBottomFormat, centralBottomFormat, rightBottomFormat );
}

AccountingTAMBillItem::ItemType AccountingTAMBillItem::itemType() const {
    return m_d->itemType;
}

void AccountingTAMBillItem::writeODTAttributeBillLineSimple( AccountingPrinter::PrintAmountsOption prAmountsOption,
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
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
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

void AccountingTAMBillItem::writeODTAttributeBillLineUnion( AccountingPrinter::PrintAmountsOption prAmountsOption, AccountingPrinter::PrintPPUDescOption prPPUDescOption,
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
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
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

void AccountingTAMBillItem::writeODTAttributeBillLineIntersection(AccountingPrinter::PrintAmountsOption prAmountsOption,
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
        for( QList<AccountingTAMBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i ){
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

void AccountingTAMBillItem::writeODTBillLine(AccountingPrinter::PrintAmountsOption prAmountsOption,
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
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, accountingProgCode()  );
            } else {
                // codice progressivo
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, progCode()  );
            }
        }


        if( m_d->priceItem ){
            if( writeProgCode ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, m_d->priceItem->codeFull() );
            } else {
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat, m_d->priceItem->codeFull() );
            }
            m_d->writeDescriptionCell( cursor, table, centralFormat, txtBlockFormat, txtCharFormat, txtCharFormat, prItemsOption );
        } else {
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        }

        if( m_d->measuresModel != nullptr && !writeAccounting ){
            // celle vuote
            // tag unita misura
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

            for( int i=0; i < m_d->daysCount; ++i ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            }

            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                // quantita'
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
            } else {
                // quantita'
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                // prezzo
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                // importo
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
            }

            table->appendRows(1);
            cursor->movePosition(QTextCursor::PreviousRow );

            if( writeProgCode ){
                // numero progressivo
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                // codice
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
            } else {
                // codice
                AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
            }


            // tag unita di misura
            QString unitMeasureTag;
            if( m_d->priceItem ){
                if( m_d->priceItem->unitMeasure()){
                    unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                }
            }

            for( int i=0; i < m_d->measuresModel->measuresCount(); ++i ){
                AccountingTAMMeasure * measure = m_d->measuresModel->measure(i);

                // unita di misura
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

                // misure
                if( measure != nullptr ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, measure->comment() );
                    for( int i=0; i < m_d->daysCount; ++i ){
                        if( measure != nullptr ){
                            QString realFormula = measure->formula(i);
                            realFormula.remove(" ");
                            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, realFormula );
                        }
                    }
                } else {
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat);
                    for( int i=0; i < m_d->daysCount; ++i ){
                        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat);
                    }
                }


                // quantità
                if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, measure->quantityStr() );
                } else {
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, measure->quantityStr() );
                }

                // celle vuote
                if( prAmountsOption != AccountingPrinter::PrintNoAmount ){
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat  );
                }

                // inserisce riga
                table->appendRows(1);
                cursor->movePosition(QTextCursor::PreviousRow );

                if( writeProgCode ){
                    // numero progressivo
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat );
                    // codice
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
                } else {
                    // codice
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, txtBlockFormat );
                }
            }

            // descrizione
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );

            // tag unita di misura
            AccountingTAMBillItemPrivate::writeCell( cursor, table,  centralFormat, tagBlockFormat, unitMeasureTag );

            for( int i=0; i<m_d->daysCount; ++i )            {
                // data
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            }

            // quantita totale
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightQuantityTotalFormat, numBlockFormat, quantityStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, quantityStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, quantityStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUNotToDiscountStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountNotToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralQuantityTotalFormat, numBlockFormat, quantityStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );

                if( PPUNotToDiscount() != 0.0 ){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );

                    QList< QPair<QString, QTextCharFormat> > txt;
                    txt << qMakePair( QString(), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, txt );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( trUtf8("di cui non soggetto a ribasso"), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( unitMeasureTag, txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( quantityStr(), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( PPUNotToDiscountStr(), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( amountNotToDiscountStr(), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, txt );
                }
            }
        } else { // m_d->linesModel == NULL
            if( writeAccounting ){
                QString nLibr;
                if( m_d->parentItem != nullptr ){
                    nLibr = QString::number( m_d->parentItem->childNumber() + 1 );
                }
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, nLibr );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, progCode() );
            }
            QString unitMeasureTag;
            if( m_d->priceItem ){
                if( m_d->priceItem->unitMeasure()){
                    unitMeasureTag = m_d->priceItem->unitMeasure()->tag();
                }
            }
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, unitMeasureTag );

            // quantita totale
            if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, quantityStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintTotalAmountsToDiscount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAmountsNotToDiscount ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUNotToDiscountStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, amountNotToDiscountStr() );
            } else if( prAmountsOption == AccountingPrinter::PrintAllAmounts ){
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, quantityStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, PPUTotalToDiscountStr() );
                AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, totalAmountToDiscountStr() );

                if( PPUNotToDiscount() != 0.0 ){
                    table->appendRows(1);
                    cursor->movePosition(QTextCursor::PreviousRow );

                    QList< QPair<QString, QTextCharFormat> > txt;
                    txt << qMakePair( QString(), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat, txt );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( trUtf8("di cui non soggetto a ribasso"), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, txt );
                    if( writeAccounting ){
                        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
                    }
                    txt.clear();
                    txt << qMakePair( unitMeasureTag, txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( quantityStr(), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( PPUNotToDiscountStr(), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat, txt );
                    txt.clear();
                    txt << qMakePair( amountNotToDiscountStr(), txtAmNotToDiscCharFormat );
                    AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat, txt );
                }
            }
        }
    } else if( m_d->itemType == Comment ){
        if( writeProgCode ){
            // codice progressivo
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat  );
            // data
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat  );
        } else {
            // data
            AccountingTAMBillItemPrivate::writeCell( cursor, table, leftFormat, tagBlockFormat  );
        }


        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat );
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, txtBlockFormat, text() );

        if( writeAccounting ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );
        }
        AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, tagBlockFormat );

        if( prAmountsOption == AccountingPrinter::PrintNoAmount ){
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
        } else {
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, centralFormat, numBlockFormat );
            AccountingTAMBillItemPrivate::writeCell( cursor, table, rightFormat, numBlockFormat );
        }
    }
}

QDate AccountingTAMBillItem::startDate() const {
    if( m_d->itemType == Payment ){
        return *(m_d->startDate);
    } else if( m_d->parentItem != nullptr ){
        return m_d->parentItem->startDate();
    }
    return QDate();
}

QString AccountingTAMBillItem::startDateStr() const {
    if( (m_d->itemType == Root) ||
            (m_d->itemType == Comment) ||
            (m_d->itemType == PPU) ){
        return QString();
    }
    if( m_d->parser != nullptr ){
        return m_d->parser->toString( *(m_d->startDate), QLocale::NarrowFormat );
    }
    return m_d->startDate->toString();
}

void AccountingTAMBillItem::setStartDate(const QDate &newStDate) {
    if( newStDate.isValid() ){
        if( m_d->itemType == Payment ){
            if( *(m_d->startDate) != newStDate ){
                *(m_d->startDate) = newStDate;
                emit startDateChanged( startDateStr() );
                updateDaysCount();
            }
        } else if( m_d->parentItem != nullptr ){
            m_d->parentItem->setStartDate(newStDate);
        }
    }
}

void AccountingTAMBillItem::setStartDate(const QString &newStDate) {
    if( m_d->parser != nullptr ){
        setStartDate( m_d->parser->evaluateDate(newStDate, QLocale::NarrowFormat) );
    } else {
        setStartDate( QDate::fromString(newStDate, Qt::DefaultLocaleShortDate) );
    }
}

QDate AccountingTAMBillItem::endDate() const {
    if( m_d->itemType == Payment ){
        return *(m_d->endDate);
    } else if( m_d->parentItem != nullptr ){
        return m_d->parentItem->endDate();
    }
    return QDate();
}

QString AccountingTAMBillItem::endDateStr() const {
    if( (m_d->itemType == Root) ||
            (m_d->itemType == Comment) ||
            (m_d->itemType == PPU) ){
        return QString();
    }
    if( m_d->parser != nullptr ){
        return m_d->parser->toString( *(m_d->endDate), QLocale::NarrowFormat );
    }
    return m_d->endDate->toString();
}

void AccountingTAMBillItem::setEndDate(const QDate &newEndDate) {
    if( newEndDate.isValid() ){
        if( m_d->itemType == Payment ){
            if( *(m_d->endDate) != newEndDate ){
                *(m_d->endDate) = newEndDate;
                emit endDateChanged( endDateStr() );
                updateDaysCount();
            }
        } else if( m_d->parentItem != nullptr ){
            m_d->parentItem->setEndDate(newEndDate);
        }
    }
}

void AccountingTAMBillItem::setEndDate(const QString &newEndDate) {
    if( m_d->parser != nullptr ){
        setEndDate( m_d->parser->evaluateDate(newEndDate, QLocale::NarrowFormat) );
    } else {
        setEndDate( QDate::fromString(newEndDate, Qt::DefaultLocaleShortDate) );
    }
}

int AccountingTAMBillItem::daysCount() const {
    if( m_d->itemType == Payment ){
        return m_d->daysCount;
    } else if( m_d->parentItem != nullptr ){
        return m_d->parentItem->daysCount();
    }
    return 0;
}

QDate AccountingTAMBillItem::day(int i) const {
    if( m_d->itemType == Payment ){
        if ( i >= 0 && i < m_d->daysCount) {
            return m_d->startDate->addDays( i );
        }
    } else if( m_d->parentItem != nullptr ){
        return m_d->parentItem->day(i);
    }
    return QDate();
}

QString AccountingTAMBillItem::dayStr(int i) const {
    return day(i).toString( "dd/MM" );
}
