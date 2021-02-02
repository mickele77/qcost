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
#include "accountingbillpricedatagui.h"
#include "ui_accountingbillpricedatagui.h"

#include "accountingattributeprintergui.h"
#include "accountingbillsetpricelistmodegui.h"

#include "project.h"
#include "accountingbill.h"
#include "attributesmodel.h"
#include "accountingpricefieldmodel.h"
#include "pricelist.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QShowEvent>
#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class AccountingBillPriceDataGUIPrivate{
public:
    AccountingBillPriceDataGUIPrivate(PriceFieldModel * pfm, MathParser * prs, Project * prj, QString * wpf):
        ui(new Ui::AccountingBillPriceDataGUI() ),
        project(prj),
        accounting(nullptr),
        parser(prs),
        wordProcessorFile(wpf),
        priceFieldModel(pfm),
        amountSpacer(nullptr){
    }
    Ui::AccountingBillPriceDataGUI * ui;
    Project * project;
    AccountingBill * accounting;
    MathParser * parser;
    QString * wordProcessorFile;
    PriceFieldModel * priceFieldModel;
    QList<QLabel *> amountLabelList;
    QList<QLineEdit *> amountLEditList;
    QSpacerItem * amountSpacer;
};

AccountingBillPriceDataGUI::AccountingBillPriceDataGUI(PriceFieldModel * pfm, MathParser * prs, AccountingBill * b, Project * prj, QString * wordProcessorFile, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingBillPriceDataGUIPrivate( pfm, prs, prj, wordProcessorFile ) ) {
    m_d->ui->setupUi( this );
    m_d->ui->totalPriceFieldTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_d->ui->noDiscountPriceFieldTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    populatePriceListComboBox();

    setAccountingBill( b );

    connect( m_d->ui->discountLineEdit, &QLineEdit::editingFinished, this, &AccountingBillPriceDataGUI::setDiscountFromLineEdit );
}

AccountingBillPriceDataGUI::~AccountingBillPriceDataGUI(){
    delete m_d;
}

void AccountingBillPriceDataGUI::setAccountingBill(AccountingBill *b) {
    if( m_d->accounting != b ){
        if( m_d->accounting != nullptr ){
            disconnect( m_d->accounting, &AccountingBill::discountChanged, m_d->ui->discountLineEdit, &QLineEdit::setText );

            disconnect( m_d->accounting, &AccountingBill::aboutToBeDeleted, this, &AccountingBillPriceDataGUI::setAccountingnullptr );
            disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingBillPriceDataGUI::setPriceList );
            disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingBillPriceDataGUI::setPriceDataSet );
        }
        m_d->ui->priceListComboBox->setCurrentIndex( 0 );
        m_d->ui->currentPriceDataSetSpinBox->setMaximum(1);
        m_d->ui->currentPriceDataSetSpinBox->setValue(1);

        m_d->ui->discountLineEdit->clear();

        for( int i=0; i < m_d->amountLEditList.size(); ++i){
            if( i < m_d->priceFieldModel->fieldCount() ){
                m_d->amountLEditList.at(i)->clear();
            }
        }
        m_d->ui->totalPriceFieldTableView->setModel( nullptr );
        m_d->ui->noDiscountPriceFieldTableView->setModel( nullptr );

        m_d->accounting = b;

        if( m_d->accounting != nullptr ){
            m_d->ui->discountLineEdit->setText( m_d->accounting->discountStr() );
            connect( m_d->accounting, &AccountingBill::discountChanged, m_d->ui->discountLineEdit, &QLineEdit::setText );

            setPriceListComboBox();
            connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingBillPriceDataGUI::setPriceList );
            setPriceDataSetSpinBox();
            connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingBillPriceDataGUI::setPriceDataSet );

            m_d->ui->totalPriceFieldTableView->setModel( m_d->accounting->totalAmountPriceFieldModel() );
            m_d->ui->noDiscountPriceFieldTableView->setModel( m_d->accounting->noDiscountAmountPriceFieldModel() );

            connect( m_d->accounting, &AccountingBill::aboutToBeDeleted, this, &AccountingBillPriceDataGUI::setAccountingnullptr );
        }
    }
}

void AccountingBillPriceDataGUI::setDiscountFromLineEdit() {
    if( m_d->accounting != nullptr ){
        m_d->accounting->setDiscount( m_d->ui->discountLineEdit->text() );
    }
}

void AccountingBillPriceDataGUI::setAccountingnullptr(){
    setAccountingBill( nullptr );
}

void AccountingBillPriceDataGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingBillPriceDataGUI::setPriceList );
        populatePriceListComboBox();
        setPriceListComboBox();
        connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingBillPriceDataGUI::setPriceList );

        disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingBillPriceDataGUI::setPriceDataSet );
        setPriceDataSetSpinBox();
        connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingBillPriceDataGUI::setPriceDataSet );
    }
    QWidget::showEvent( event );
}

void AccountingBillPriceDataGUI::populatePriceListComboBox(){
    m_d->ui->priceListComboBox->clear();
    m_d->ui->priceListComboBox->addItem( QString("---"), qVariantFromValue((void *) nullptr ));
    for( int i=0; i < m_d->project->priceListCount(); ++i){
        QString n;
        if( m_d->project->priceList(i) ){
            n =  m_d->project->priceList(i)->name();
        }
        m_d->ui->priceListComboBox->addItem( n, qVariantFromValue((void *) m_d->project->priceList(i) ));
    }
}

void AccountingBillPriceDataGUI::setPriceListComboBox() {
    if( m_d->accounting ){
        int i = m_d->ui->priceListComboBox->findData( qVariantFromValue((void *) m_d->accounting->priceList() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->priceListComboBox->setCurrentIndex( i );
    }
}

void AccountingBillPriceDataGUI::setPriceDataSetSpinBox() {
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

void AccountingBillPriceDataGUI::setPriceList() {
    QVariant v =  m_d->ui->priceListComboBox->itemData( m_d->ui->priceListComboBox->currentIndex() );
    PriceList * currentPriceList = (PriceList *) v.value<void *>();
    if( m_d->accounting ){
        if( m_d->accounting->priceList() && m_d->accounting->rowCount() > 0 ){
            AccountingBillSetPriceListModeGUI modeGUI( this );
            AccountingBill::SetPriceListMode plMode = modeGUI.returnValue();
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

void AccountingBillPriceDataGUI::setPriceDataSet() {
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
