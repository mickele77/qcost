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
#include "accountingtambillitemgui.h"
#include "ui_accountingtambillitemgui.h"

#include "priceitemgui.h"
#include "qcalendardialog.h"
#include "importbillitemmeasurestxt.h"

#include "project.h"
#include "accountingtambill.h"
#include "accountingtambillitem.h"

#include "accountingtammeasuresmodel.h"
#include "measuresmodel.h"

#include "accountingitemattributemodel.h"
#include "attributesmodel.h"
#include "priceitem.h"
#include "unitmeasure.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QDate>
#include <QSpacerItem>

class AccountingTAMBillItemGUIPrivate{
public:
    AccountingTAMBillItemGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions, QString * EPAFileName, MathParser * prs, Project * prj ):
        ui(new Ui::AccountingTAMBillItemGUI),
        parser(prs),
        TAMBillItem(nullptr),
        connectedUnitMeasure(nullptr),
        itemAttributeModel( new AccountingItemAttributeModel(nullptr, nullptr ) ),
        priceItemGUI( new PriceItemGUI( EPAImpOptions, EPAFileName, nullptr, 0, prs, prj, nullptr )),
        vSpacer(nullptr) {
    }
    ~AccountingTAMBillItemGUIPrivate(){
        delete ui;
        delete itemAttributeModel;
    }
    Ui::AccountingTAMBillItemGUI * ui;
    MathParser * parser;
    AccountingTAMBillItem * TAMBillItem;
    UnitMeasure * connectedUnitMeasure;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceItemGUI * priceItemGUI;
    QSpacerItem * vSpacer;
};

AccountingTAMBillItemGUI::AccountingTAMBillItemGUI( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                                            QString * EPAFileName,
                                            MathParser * prs,
                                            Project * prj,
                                            QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingTAMBillItemGUIPrivate( EPAImpOptions, EPAFileName, prs, prj ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->progNumberGroupBox->setHidden( true );
    m_d->ui->priceGroupBox->setHidden( true );
    m_d->ui->amountsGroupBox->setHidden( true );
    m_d->ui->priceTab->layout()->addWidget( m_d->priceItemGUI );
    m_d->ui->attributeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    m_d->ui->priceGBCheckBox->setChecked(true);

    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingTAMBillItemGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingTAMBillItemGUI::removeAttribute );
}

AccountingTAMBillItemGUI::~AccountingTAMBillItemGUI() {
    delete m_d;
}

void AccountingTAMBillItemGUI::setItem(AccountingTAMBillItem *b) {
    if( m_d->TAMBillItem != b ){
        if( m_d->TAMBillItem != nullptr ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToBDiscountedLineEdit, &QLineEdit::setText );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::priceItemChanged, this, &AccountingTAMBillItemGUI::connectPriceItem );
            disconnectPriceItem( m_d->TAMBillItem->priceItem() );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingTAMBillItemGUI::setAccountingItemnullptr );
        }

        m_d->ui->totalAmountToDiscountLineEdit->clear();
        m_d->ui->amountNotToDiscountLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->PPUNotToBDiscountedLineEdit->clear();
        m_d->ui->PPUTotalToDiscountLineEdit->clear();
        m_d->ui->PPUNotToBDiscountedLineEdit->clear();

        m_d->TAMBillItem = b;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->TAMBillItem != nullptr ){
            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->TAMBillItem->totalAmountToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->TAMBillItem->amountNotToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->TAMBillItem->totalAmountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            connectPriceItem( nullptr, m_d->TAMBillItem->priceItem());
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::priceItemChanged, this, &AccountingTAMBillItemGUI::connectPriceItem );

            m_d->ui->PPUTotalToDiscountLineEdit->setText( m_d->TAMBillItem->PPUTotalToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->PPUNotToBDiscountedLineEdit->setText( m_d->TAMBillItem->PPUNotToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToBDiscountedLineEdit, &QLineEdit::setText );

            m_d->priceItemGUI->setCurrentPriceDataSet( m_d->TAMBillItem->currentPriceDataSet() );

            m_d->ui->itemMeasuresTableView->setModel( m_d->TAMBillItem->measuresModel() );

            connect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingTAMBillItemGUI::setAccountingItemnullptr );
        } else {
            m_d->ui->priceCodeLineEdit->clear();
            m_d->ui->priceShortDescLineEdit->clear();

            m_d->ui->PPUTotalToDiscountLineEdit->clear();
            m_d->ui->PPUNotToBDiscountedLineEdit->clear();

            m_d->ui->totalAmountToDiscountLineEdit->clear();
            m_d->ui->amountNotToDiscountLineEdit->clear();
            m_d->ui->totalAmountLineEdit->clear();

            m_d->priceItemGUI->setPriceItemnullptr();
            m_d->ui->itemMeasuresTableView->setModel( nullptr );
        }
    }
}

void AccountingTAMBillItemGUI::setAccountingTAMBill(AccountingTAMBill *b) {
    if( b != nullptr ){
        m_d->itemAttributeModel->setAttributeModel( b->attributesModel() );
    } else {
        m_d->itemAttributeModel->setAttributeModel( nullptr );
    }
    // quando si cambia computo corrente la scheda della riga si azzera
    setItem( (AccountingTAMBillItem *)(nullptr) );
}

void AccountingTAMBillItemGUI::changeItemDateGUI(){
    if( m_d->TAMBillItem != nullptr ){
        QDate d = m_d->TAMBillItem->startDate();
        QCalendarDialog dialog( &d, this );
        if( dialog.exec() == QDialog::Accepted ){
            m_d->TAMBillItem->setStartDate( d );
        }
    }
}

void AccountingTAMBillItemGUI::disconnectPriceItem( PriceItem * priceItem ) {
    if( priceItem != nullptr ){
        disconnect( priceItem, &PriceItem::codeFullChanged, m_d->ui->priceCodeLineEdit, &QLineEdit::setText );
        disconnect( priceItem, &PriceItem::shortDescriptionFullChanged, m_d->ui->priceShortDescLineEdit, &QLineEdit::setText );
        disconnect( priceItem, &PriceItem::unitMeasureChanged, this, &AccountingTAMBillItemGUI::connectPriceUnitMeasure );
        if( priceItem->unitMeasure() ){
            disconnect( priceItem->unitMeasure(), &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
        }
        m_d->ui->priceCodeLineEdit->clear();
        m_d->ui->priceShortDescLineEdit->clear();
        m_d->ui->priceUnitMeasureLineEdit->clear();
        m_d->priceItemGUI->setPriceItem( nullptr );
    }
}

void AccountingTAMBillItemGUI::connectPriceItem( PriceItem * oldPriceItem, PriceItem * newPriceItem ) {
    disconnectPriceItem( oldPriceItem );

    if( newPriceItem != nullptr ){
        m_d->ui->priceCodeLineEdit->setText( newPriceItem->codeFull() );
        m_d->ui->priceShortDescLineEdit->setText( newPriceItem->shortDescriptionFull() );
        if( newPriceItem->unitMeasure() ){
            m_d->ui->priceUnitMeasureLineEdit->setText( newPriceItem->unitMeasure()->tag() );
        } else {
            m_d->ui->priceUnitMeasureLineEdit->setText( QString("---") );
        }

        connect( newPriceItem, &PriceItem::codeFullChanged, m_d->ui->priceCodeLineEdit, &QLineEdit::setText );
        connect( newPriceItem, &PriceItem::shortDescriptionFullChanged, m_d->ui->priceShortDescLineEdit, &QLineEdit::setText );
        connect( newPriceItem, &PriceItem::unitMeasureChanged, this, &AccountingTAMBillItemGUI::connectPriceUnitMeasure );
        if( newPriceItem->unitMeasure() ){
            connect( newPriceItem->unitMeasure(), &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
        }
        m_d->priceItemGUI->setPriceItem( newPriceItem );
    } else {
        m_d->ui->priceCodeLineEdit->clear();
        m_d->ui->priceShortDescLineEdit->clear();
        m_d->ui->priceUnitMeasureLineEdit->clear();
        m_d->priceItemGUI->setPriceItemnullptr();
    }
}

void AccountingTAMBillItemGUI::connectPriceUnitMeasure(){
    if( m_d->TAMBillItem != nullptr ){
        if( m_d->TAMBillItem->priceItem() != nullptr ) {
            if( m_d->connectedUnitMeasure != nullptr ){
                disconnect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }

            m_d->connectedUnitMeasure = m_d->TAMBillItem->priceItem()->unitMeasure();

            if( m_d->connectedUnitMeasure != nullptr ){
                m_d->ui->priceUnitMeasureLineEdit->setText( m_d->connectedUnitMeasure->tag()  );
                connect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }
        }
    }
}

void AccountingTAMBillItemGUI::setAccountingItemnullptr() {
    setItem( (AccountingTAMBillItem *)( nullptr ));
}

void AccountingTAMBillItemGUI::addAttribute(){
    if( m_d->itemAttributeModel != nullptr ){
        if( m_d->ui->attributeTableView->selectionModel() ){
            int count = 1;
            QModelIndexList selectedRows = m_d->ui->attributeTableView->selectionModel()->selectedRows();
            if( selectedRows.size() > 1 ){
                count = selectedRows.size();
            }
            int row = m_d->itemAttributeModel->rowCount();
            QModelIndex currentIndex = m_d->ui->attributeTableView->selectionModel()->currentIndex();
            if( currentIndex.isValid() ){
                row = currentIndex.row() + 1;
            }
            m_d->itemAttributeModel->insertRows( row, count );
        }
    }
}

void AccountingTAMBillItemGUI::removeAttribute(){
    if( m_d->itemAttributeModel != nullptr ){
        if( m_d->ui->attributeTableView->selectionModel() ){
            QModelIndexList selectedRows = m_d->ui->attributeTableView->selectionModel()->selectedRows();
            int count = selectedRows.size();
            if( count > 0 ){
                int row = selectedRows.at(0).row();
                for( int i=1; i<selectedRows.size(); ++i){
                    if( selectedRows.at(i).row() < row ){
                        row = selectedRows.at(i).row();
                    }
                }
                m_d->itemAttributeModel->removeRows( row, count );
            }
        }
    }
}

