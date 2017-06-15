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

#include "accountingbillitemprivate.h"

class AccountingTAMBillItemPrivate {
public:
    AccountingTAMBillItemPrivate():
        startDate( NULL ),
        endDate(NULL){
    }
    ~AccountingTAMBillItemPrivate() {
        if( startDate != NULL ) {
            delete startDate;
        }
        if( endDate != NULL ){
            delete endDate;
        }
    }
    QDate * startDate;
    QDate * endDate;
};

AccountingTAMBillItem::AccountingTAMBillItem(AccountingTAMBillItem *parentItem, AccountingBillItem::ItemType iType,
                                             PriceFieldModel * pfm, MathParser * parser ):
    AccountingBillItem( parentItem, iType, pfm, parser ),
    m_dd( new AccountingTAMBillItemPrivate() ){
    if( parentItem == NULL ) {
        m_dd->startDate = new QDate();
        m_dd->endDate = new QDate();
    }
}

AccountingTAMBillItem::~AccountingTAMBillItem(){
    delete m_dd;
}

QString AccountingTAMBillItem::title() const{
    if( m_d->itemType == Payment ){
        return trUtf8("Lista N.%1 (%2-%3)").arg(QString::number(childNumber()+1), dateBeginStr(), dateEndStr() );
    }
    return QString();
}

bool AccountingTAMBillItem::insertChildren(AccountingBillItem::ItemType iType, int position, int count ){
    if (position < 0 || position > m_d->childrenContainer.size() )
        return false;

    if( (m_d->itemType == Root && (iType == Payment) ) ||
            (m_d->itemType == Payment && (iType == PPU || iType == Comment)) ){

        bool hadChildren = m_d->childrenContainer.size() > 0;

        for (int row = 0; row < count; ++row) {
            AccountingTAMBillItem * item = new AccountingTAMBillItem( this, iType,  m_d->priceFieldModel, m_d->parser );
            while( findItemFromId( item->id() ) != NULL ){
                item->setId( item->id() + 1 );
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

        emit itemChanged();

        return true;
    }
    return false;
}

QDate AccountingTAMBillItem::startDate() const {
    AccountingTAMBillItem * TAMParent = dynamic_cast<AccountingTAMBillItem *>( m_d->parentItem );
    if( TAMParent != NULL ){
        return TAMParent->startDate();
    } else {
        return *(m_dd->startDate);
    }
}

void AccountingTAMBillItem::setStartDate(const QDate &newStDate) {
    AccountingTAMBillItem * TAMParent = dynamic_cast<AccountingTAMBillItem *>( m_d->parentItem );
    if( TAMParent != NULL ){
        TAMParent->setStartDate(newStDate);
    } else {
        if( *(m_dd->startDate) != newStDate ){
            *(m_dd->startDate) = newStDate;
        }
    }
}

QDate AccountingTAMBillItem::endDate() const {
    AccountingTAMBillItem * TAMParent = dynamic_cast<AccountingTAMBillItem *>( m_d->parentItem );
    if( TAMParent != NULL ){
        return TAMParent->endDate();
    } else {
        return *(m_dd->endDate);
    }
}

void AccountingTAMBillItem::setEndDate(const QDate &newEndDate) {
    AccountingTAMBillItem * TAMParent = dynamic_cast<AccountingTAMBillItem *>( m_d->parentItem );
    if( TAMParent != NULL ){
        TAMParent->setEndDate(newEndDate);
    } else {
        if( *(m_dd->endDate) != newEndDate ){
            *(m_dd->endDate) = newEndDate;
        }
    }
}
