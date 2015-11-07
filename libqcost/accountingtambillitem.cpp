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

#include "accountingtambillitem.h"

#include "accountingbillitemprivate.h"

#include "measuresmodel.h"

AccountingTAMBillItem::AccountingTAMBillItem(AccountingTAMBillItem *parentItem, AccountingBillItem::ItemType iType,
                                             PriceFieldModel * pfm, MathParser * parser ):
    AccountingBillItem( parentItem, iType, pfm, parser ){
}

AccountingTAMBillItem::~AccountingTAMBillItem(){
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
            while( findAccountingItemId( item->id() ) != NULL ){
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

void AccountingTAMBillItem::readXml( QXmlStreamReader *reader, PriceList * priceList, AttributeModel * attrModel ) {
    if( m_d->itemType != Root ){
        if(reader->isStartElement() && reader->name().toString().toUpper() == "ACCOUNTINGBILLITEM"){
            loadFromXmlTmp( reader->attributes() );
        }
        reader->readNext();
    }

    while( (!reader->atEnd()) &&
           (!reader->hasError()) &&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGBILLITEM")&&
           !(reader->isEndElement() && reader->name().toString().toUpper() == "ACCOUNTINGBILL")  ){
        if( m_d->itemType == Root ){
            if( reader->name().toString().toUpper() == "ACCOUNTINGBILLITEM" && reader->isStartElement()) {
                if( reader->attributes().hasAttribute( "itemType" ) ){
                    if( reader->attributes().value( "itemType" ).toString().toUpper() == "PAYMENT" ){
                        appendChildren( Payment );
                        m_d->childrenContainer.last()->readXml( reader, priceList, attrModel );
                    }
                }
            }
        } else if( m_d->itemType == Payment ){
            if( reader->name().toString().toUpper() == "ACCOUNTINGBILLITEM" && reader->isStartElement()) {
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
                    m_d->childrenContainer.last()->readXml( reader, priceList, attrModel );
                }
            }
        }  else if( m_d->itemType == PPU ){
            if( reader->name().toString().toUpper() == "MEASURESMODEL" && reader->isStartElement() ) {
                generateMeasuresModel()->readXml( reader );
            }
        }
        reader->readNext();
    }
}

void AccountingTAMBillItem::writeXml(QXmlStreamWriter *writer) {
    if( m_d->itemType == Payment ){
        writer->writeStartElement( "AccountingBillItem" );
        writer->writeAttribute( "itemType", QString("Payment") );
        writer->writeAttribute( "dateBegin", QString::number( m_d->date.toJulianDay() ) );
        writer->writeAttribute( "dateEnd", QString::number( m_d->dateEnd.toJulianDay() ) );

        QString attrs = m_d->attributesString();
        if( !attrs.isEmpty() ){
            writer->writeAttribute( "attributes", attrs );
        }
        for( QList<AccountingBillItem *>::iterator i = m_d->childrenContainer.begin(); i != m_d->childrenContainer.end(); ++i){
            (*i)->writeXml( writer );
        }
        writer->writeEndElement();
    } else {
        AccountingBillItem::writeXml( writer );
    }
}
