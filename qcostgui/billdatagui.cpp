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
#include "billdatagui.h"

#include "ui_billdatagui.h"

#include "billattributeprintergui.h"
#include "bill.h"
#include "attributesmodel.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

class BillDataGUIPrivate{
public:
    BillDataGUIPrivate(PriceFieldModel * pfm ):
        ui(new Ui::BillDataGUI() ),
        bill(NULL),
        priceFieldModel(pfm),
        amountVertSpacer(NULL),
        amountHorSpacer(NULL){
    };
    Ui::BillDataGUI * ui;
    Bill * bill;
    PriceFieldModel * priceFieldModel;
    QList<QLabel *> amountLabelList;
    QList<QLineEdit *> amountLEditList;
    QSpacerItem * amountVertSpacer;
    QSpacerItem * amountHorSpacer;
};

BillDataGUI::BillDataGUI(PriceFieldModel * pfm, Bill * b, QWidget *parent) :
    QWidget(parent),
    m_d( new BillDataGUIPrivate( pfm ) ) {
    m_d->ui->setupUi( this );
    setBill( b );
    connect( pfm, &PriceFieldModel::endInsertPriceField, this, &BillDataGUI::updateAmountsNameValue );
    connect( pfm, &PriceFieldModel::endRemovePriceField, this, &BillDataGUI::updateAmountsNameValue );
    updateAmountsNameValue();
    connect( pfm, &PriceFieldModel::amountNameChanged, this, &BillDataGUI::updateAmountName );

}

BillDataGUI::~BillDataGUI(){
    delete m_d;
}

void BillDataGUI::setBill(Bill *b) {
    if( m_d->bill != b ){
        if( m_d->bill != NULL ){
            disconnect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->bill, &Bill::setName );
            disconnect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &BillDataGUI::setDescription );
            disconnect( m_d->bill, static_cast<void(Bill::*)(int,const QString &)> (&Bill::amountChanged), this, &BillDataGUI::updateAmountValue );
            disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &BillDataGUI::setBillNULL );
        }
        m_d->ui->nameLineEdit->clear();
        m_d->ui->descriptionTextEdit->clear();
        for( int i=0; i < m_d->amountLEditList.size(); ++i){
            if( i < m_d->priceFieldModel->fieldCount() ){
                m_d->amountLEditList.at(i)->clear();
            }
        }

        m_d->bill = b;

        if( m_d->bill != NULL ){
            m_d->ui->nameLineEdit->setText( m_d->bill->name() );
            connect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->bill, &Bill::setName );
            m_d->ui->descriptionTextEdit->setPlainText( m_d->bill->description() );
            connect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &BillDataGUI::setDescription );

            for( int i=0; i < m_d->amountLEditList.size(); ++i){
                if( i < m_d->priceFieldModel->fieldCount() ){
                    m_d->amountLEditList.at(i)->setText( m_d->bill->amountStr(i) );
                }
            }
            connect( m_d->bill, static_cast<void(Bill::*)(int,const QString &)> (&Bill::amountChanged), this, &BillDataGUI::updateAmountValue );

            connect( m_d->bill, &Bill::aboutToBeDeleted, this, &BillDataGUI::setBillNULL );
        }
    }
}


void BillDataGUI::setDescription(){
    if( m_d->bill ){
        m_d->bill->setDescription( m_d->ui->descriptionTextEdit->toPlainText() );
    }
}

void BillDataGUI::setBillNULL(){
    setBill( NULL );
}

void BillDataGUI::updateAmountsNameValue(){
    for( QList<QLabel * >::iterator i = m_d->amountLabelList.begin(); i != m_d->amountLabelList.end(); ++i ){
        m_d->ui->amountsLayout->removeWidget( *i );
        delete *i;
    }
    m_d->amountLabelList.clear();
    for( QList<QLineEdit * >::iterator i = m_d->amountLEditList.begin(); i != m_d->amountLEditList.end(); ++i ){
        m_d->ui->amountsLayout->removeWidget( *i );
        delete *i;
    }
    if( m_d->amountVertSpacer != NULL ){
        m_d->ui->amountsLayout->removeItem( m_d->amountVertSpacer );
        delete m_d->amountVertSpacer;
        m_d->amountVertSpacer = NULL;
    }
    if( m_d->amountHorSpacer != NULL ){
        m_d->ui->amountsLayout->removeItem( m_d->amountHorSpacer );
        delete m_d->amountHorSpacer;
        m_d->amountHorSpacer = NULL;
    }
    m_d->amountLEditList.clear();

    for( int i=0; i<m_d->priceFieldModel->fieldCount(); ++i){
        QLabel * label = new QLabel( m_d->priceFieldModel->amountName( i ) );
        QLineEdit * lEdit = new QLineEdit();
        lEdit->setReadOnly( true );
        lEdit->setAlignment( Qt::AlignRight);
        if( m_d->bill != NULL ){
            lEdit->setText( m_d->bill->amountStr(i));
        }
        m_d->ui->amountsLayout->addWidget( label, i, 0 );
        m_d->ui->amountsLayout->addWidget( lEdit, i, 1 );
        if( i == 0 ){
            m_d->amountVertSpacer = new QSpacerItem( 20, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
            m_d->ui->amountsLayout->addItem( m_d->amountVertSpacer, i, 2 );
        }
        m_d->amountLabelList.append( label );
        m_d->amountLEditList.append( lEdit );
    }
    m_d->amountHorSpacer = new QSpacerItem( 20, 25, QSizePolicy::Minimum, QSizePolicy::Expanding );
    m_d->ui->amountsLayout->addItem( m_d->amountHorSpacer, m_d->priceFieldModel->fieldCount(), 0 );
}

void BillDataGUI::updateAmountValue(int priceField, const QString & newVal ){
    if( priceField > -1 && priceField < m_d->amountLEditList.size() ){
        m_d->amountLEditList.at(priceField)->setText( newVal );
    }
}

void BillDataGUI::updateAmountName( int priceField, const QString & newName ){
    if( priceField > -1 && priceField < m_d->amountLEditList.size() ){
        m_d->amountLabelList.at(priceField)->setText( newName );
    }
}
