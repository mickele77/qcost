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
#include "accountingtambillpricedatagui.h"
#include "ui_accountingtambillpricedatagui.h"

#include "accountingattributeprintergui.h"
#include "accountingbillsetpricelistmodegui.h"

#include "project.h"
#include "accountingtambill.h"
#include "attributesmodel.h"
#include "accountingpricefieldmodel.h"
#include "pricelist.h"
#include "pricefieldmodel.h"

#include <QShowEvent>
#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class AccountingTAMBillPriceDataGUIPrivate{
public:
    AccountingTAMBillPriceDataGUIPrivate(PriceFieldModel * pfm, Project * prj):
        ui(new Ui::AccountingTAMBillPriceDataGUI() ),
        project(prj),
        accounting(NULL),
        priceFieldModel(pfm),
        amountSpacer(NULL){
    }
    Ui::AccountingTAMBillPriceDataGUI * ui;
    Project * project;
    AccountingTAMBill * accounting;
    PriceFieldModel * priceFieldModel;
    QList<QLabel *> amountLabelList;
    QSpacerItem * amountSpacer;
};

AccountingTAMBillPriceDataGUI::AccountingTAMBillPriceDataGUI( PriceFieldModel * pfm,
                                                              AccountingTAMBill * b,
                                                              Project * prj,
                                                              QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingTAMBillPriceDataGUIPrivate( pfm, prj ) ) {
    m_d->ui->setupUi( this );
    m_d->ui->totalPriceFieldTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_d->ui->noDiscountPriceFieldTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    populatePriceListComboBox();

    setAccountingTAMBill( b );

    connect( m_d->ui->discountLineEdit, &QLineEdit::editingFinished, this, &AccountingTAMBillPriceDataGUI::setDiscount );
}

AccountingTAMBillPriceDataGUI::~AccountingTAMBillPriceDataGUI(){
    delete m_d;
}

void AccountingTAMBillPriceDataGUI::setAccountingTAMBill(AccountingTAMBill *b) {
    if( m_d->accounting != b ){
        if( m_d->accounting != NULL ){
            disconnect( m_d->accounting, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTAMBillPriceDataGUI::setAccountingNULL );
            disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTAMBillPriceDataGUI::setPriceList );
            disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTAMBillPriceDataGUI::setPriceDataSet );
        }
        m_d->ui->priceListComboBox->setCurrentIndex( 0 );
        m_d->ui->currentPriceDataSetSpinBox->setMaximum(1);
        m_d->ui->currentPriceDataSetSpinBox->setValue(1);

        m_d->ui->discountLineEdit->clear();
        m_d->ui->totalPriceFieldTableView->setModel( NULL );
        m_d->ui->noDiscountPriceFieldTableView->setModel( NULL );

        m_d->accounting = b;

        if( m_d->accounting != NULL ){
            m_d->ui->discountLineEdit->setText( m_d->accounting->discountStr());
            connect( m_d->accounting, &AccountingTAMBill::discountChanged, m_d->ui->discountLineEdit, &QLineEdit::setText );

            setPriceListComboBox();
            connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTAMBillPriceDataGUI::setPriceList );
            setPriceDataSetSpinBox();
            connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTAMBillPriceDataGUI::setPriceDataSet );

            m_d->ui->totalPriceFieldTableView->setModel( m_d->accounting->totalAmountPriceFieldModel() );
            m_d->ui->noDiscountPriceFieldTableView->setModel( m_d->accounting->noDiscountAmountPriceFieldModel() );

            connect( m_d->accounting, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTAMBillPriceDataGUI::setAccountingNULL );
        }
    }
}

void AccountingTAMBillPriceDataGUI::setDiscount() {
    if( m_d->accounting != NULL ){
        m_d->accounting->setDiscount( m_d->ui->discountLineEdit->text() );
    }
}

void AccountingTAMBillPriceDataGUI::setAccountingNULL(){
    setAccountingTAMBill( NULL );
}

void AccountingTAMBillPriceDataGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTAMBillPriceDataGUI::setPriceList );
        populatePriceListComboBox();
        setPriceListComboBox();
        connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTAMBillPriceDataGUI::setPriceList );

        disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTAMBillPriceDataGUI::setPriceDataSet );
        setPriceDataSetSpinBox();
        connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTAMBillPriceDataGUI::setPriceDataSet );
    }
    QWidget::showEvent( event );
}

void AccountingTAMBillPriceDataGUI::populatePriceListComboBox(){
    m_d->ui->priceListComboBox->clear();
    m_d->ui->priceListComboBox->addItem( QString("---"), qVariantFromValue((void *) NULL ));
    for( int i=0; i < m_d->project->priceListCount(); ++i){
        QString n;
        if( m_d->project->priceList(i) ){
            n =  m_d->project->priceList(i)->name();
        }
        m_d->ui->priceListComboBox->addItem( n, qVariantFromValue((void *) m_d->project->priceList(i) ));
    }
}

void AccountingTAMBillPriceDataGUI::setPriceListComboBox() {
    if( m_d->accounting ){
        int i = m_d->ui->priceListComboBox->findData( qVariantFromValue((void *) m_d->accounting->priceList() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->priceListComboBox->setCurrentIndex( i );
    }
}

void AccountingTAMBillPriceDataGUI::setPriceDataSetSpinBox() {
    if( m_d->accounting ){
        if( m_d->accounting->priceList() ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->accounting->priceList()->priceDataSetCount() );
        }
        m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->accounting->priceDataSet()+1 );
    } else {
        m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
    }
}

void AccountingTAMBillPriceDataGUI::setPriceList() {
    QVariant v =  m_d->ui->priceListComboBox->itemData( m_d->ui->priceListComboBox->currentIndex() );
    PriceList * currentPriceList = (PriceList *) v.value<void *>();
    if( m_d->accounting ){
        if( m_d->accounting->priceList() && m_d->accounting->rowCount() > 0 ){
            AccountingBillSetPriceListModeGUI modeGUI( this );
            AccountingTAMBill::SetPriceListMode plMode = modeGUI.returnValueTAMBill();
            m_d->accounting->setPriceList( currentPriceList, plMode );
        } else {
            m_d->accounting->setPriceList( currentPriceList );
        }
        if( m_d->accounting->priceList() ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->accounting->priceList()->priceDataSetCount() );
        } else {
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        }
    }
}

void AccountingTAMBillPriceDataGUI::setPriceDataSet() {
    if( m_d->accounting ){
        if( m_d->accounting->priceList() ){
            if( m_d->ui->currentPriceDataSetSpinBox->value() < 1 ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
            } else if(  m_d->ui->currentPriceDataSetSpinBox->value() > m_d->accounting->priceList()->priceDataSetCount() ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->accounting->priceList()->priceDataSetCount() );
            } else {
                m_d->accounting->setPriceDataSet( m_d->ui->currentPriceDataSetSpinBox->value()-1 );
            }
        }
    }
}
