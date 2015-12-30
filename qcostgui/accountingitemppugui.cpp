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
#include "accountingitemppugui.h"
#include "ui_accountingitemppugui.h"

#include "priceitemgui.h"
#include "qcalendardialog.h"
#include "importbillitemmeasurestxt.h"

#include "project.h"
#include "bill.h"
#include "accountingbill.h"
#include "accountingbillitem.h"
#include "accountingtambill.h"
#include "accountingtambillitem.h"

#include "measuresmodel.h"

#include "accountingitemattributemodel.h"
#include "attributesmodel.h"
#include "priceitem.h"
#include "unitmeasure.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QDate>

class AccountingItemPPUGUIPrivate{
public:
    AccountingItemPPUGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions, QString * EPAFileName, MathParser * prs, Project * prj ):
        ui(new Ui::AccountingItemPPUGUI),
        parser(prs),
        billItem(NULL),
        TAMBillItem(NULL),
        connectedUnitMeasure(NULL),
        itemAttributeModel( new AccountingItemAttributeModel(NULL, NULL ) ),
        priceItemGUI( new PriceItemGUI( EPAImpOptions, EPAFileName, NULL, 0, prs, prj, NULL )),
        vSpacer(NULL) {
    }
    ~AccountingItemPPUGUIPrivate(){
        delete ui;
        delete itemAttributeModel;
    }
    Ui::AccountingItemPPUGUI * ui;
    MathParser * parser;
    AccountingBillItem * billItem;
    AccountingTAMBillItem * TAMBillItem;
    UnitMeasure * connectedUnitMeasure;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceItemGUI * priceItemGUI;
    QSpacerItem * vSpacer;
};

AccountingItemPPUGUI::AccountingItemPPUGUI( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                                            QString * EPAFileName,
                                            MathParser * prs,
                                            Project * prj,
                                            QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingItemPPUGUIPrivate( EPAImpOptions, EPAFileName, prs, prj ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->priceGroupBox->setHidden( true );
    m_d->ui->amountsGroupBox->setHidden( true );
    m_d->ui->priceTab->layout()->addWidget( m_d->priceItemGUI );
    m_d->ui->attributeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    associateLinesModel( false );
    m_d->ui->priceGBCheckBox->setChecked(true);

    connect( m_d->ui->addItemLinePushButton, &QPushButton::clicked, this, &AccountingItemPPUGUI::addMeasureLines );
    connect( m_d->ui->delItemLinePushButton, &QPushButton::clicked, this, &AccountingItemPPUGUI::delMeasureLines );
    connect( m_d->ui->importItemMeasuresTXTPushButton, &QPushButton::clicked, this, &AccountingItemPPUGUI::importAccountingMeasureMeasuresTXT );

    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingItemPPUGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingItemPPUGUI::removeAttribute );

    connect( m_d->priceItemGUI, static_cast<void(PriceItemGUI::*)(PriceItem *, Bill *)>(&PriceItemGUI::editPriceItemAP), this, &AccountingItemPPUGUI::editPriceItemAP );
    connect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &AccountingItemPPUGUI::associateLinesModel );

    m_d->ui->dateLineEdit->installEventFilter(this);
}

AccountingItemPPUGUI::~AccountingItemPPUGUI() {
    delete m_d;
}

void AccountingItemPPUGUI::setItem(AccountingTAMBillItem *b) {
    if( m_d->TAMBillItem != b || m_d->billItem != NULL ){
        if( m_d->billItem != NULL ){
            disconnect( m_d->billItem, &AccountingBillItem::dateChanged, m_d->ui->dateLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->dateLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setDateLE );
            disconnect( m_d->billItem, &AccountingBillItem::quantityChanged, m_d->ui->quantityLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->quantityLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setQuantityLE );

            disconnect( m_d->billItem, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            disconnect( m_d->billItem, &AccountingBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToBDiscountedLineEdit, &QLineEdit::setText );

            disconnect( m_d->billItem, &AccountingBillItem::priceItemChanged, this, &AccountingItemPPUGUI::connectPriceItem );
            disconnectPriceItem( m_d->billItem->priceItem() );

            disconnect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &AccountingItemPPUGUI::associateLinesModel );
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemPPUGUI::setAccountingItemNULL );
        }
        if( m_d->TAMBillItem != NULL ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::dateChanged, m_d->ui->dateLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->dateLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setDateLE );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::quantityChanged, m_d->ui->quantityLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->quantityLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setQuantityLE );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToBDiscountedLineEdit, &QLineEdit::setText );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::priceItemChanged, this, &AccountingItemPPUGUI::connectPriceItem );
            disconnectPriceItem( m_d->TAMBillItem->priceItem() );

            disconnect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &AccountingItemPPUGUI::associateLinesModel );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemPPUGUI::setAccountingItemNULL );
        }

        m_d->ui->totalAmountToDiscountLineEdit->clear();
        m_d->ui->amountNotToDiscountLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->PPUNotToBDiscountedLineEdit->clear();
        m_d->ui->PPUTotalToDiscountLineEdit->clear();
        m_d->ui->PPUNotToBDiscountedLineEdit->clear();

        m_d->billItem = NULL;
        m_d->TAMBillItem = b;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->TAMBillItem != NULL ){
            m_d->ui->dateLineEdit->setText( m_d->TAMBillItem->dateStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::dateChanged, m_d->ui->dateLineEdit, &QLineEdit::setText );
            connect( m_d->ui->dateLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setDateLE );
            m_d->ui->quantityLineEdit->setText( m_d->TAMBillItem->quantityStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::quantityChanged, m_d->ui->quantityLineEdit, &QLineEdit::setText );
            connect( m_d->ui->quantityLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setQuantityLE );

            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->TAMBillItem->totalAmountToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->TAMBillItem->amountNotToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->TAMBillItem->totalAmountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            connectPriceItem( NULL, m_d->TAMBillItem->priceItem());
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::priceItemChanged, this, &AccountingItemPPUGUI::connectPriceItem );

            m_d->ui->PPUTotalToDiscountLineEdit->setText( m_d->TAMBillItem->PPUTotalToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->PPUNotToBDiscountedLineEdit->setText( m_d->TAMBillItem->PPUNotToDiscountStr() );
            connect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToBDiscountedLineEdit, &QLineEdit::setText );

            m_d->priceItemGUI->setCurrentPriceDataSet( m_d->TAMBillItem->currentPriceDataSet() );

            m_d->ui->itemMeasuresTableView->setModel( m_d->TAMBillItem->measuresModel() );

            associateLinesModel( m_d->TAMBillItem->measuresModel() != NULL );

            m_d->ui->associateLinesCheckBox->setChecked(  m_d->TAMBillItem->measuresModel() != NULL  );
            connect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &AccountingItemPPUGUI::associateLinesModel );

            connect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemPPUGUI::setAccountingItemNULL );
        } else {
            m_d->ui->quantityLineEdit->clear();
            m_d->ui->priceCodeLineEdit->clear();
            m_d->ui->priceShortDescLineEdit->clear();

            m_d->ui->PPUTotalToDiscountLineEdit->clear();
            m_d->ui->PPUNotToBDiscountedLineEdit->clear();

            m_d->ui->totalAmountToDiscountLineEdit->clear();
            m_d->ui->amountNotToDiscountLineEdit->clear();
            m_d->ui->totalAmountLineEdit->clear();

            m_d->priceItemGUI->setPriceItemNULL();
            m_d->ui->itemMeasuresTableView->setModel( NULL );
            associateLinesModel( false );
        }
    }
}

void AccountingItemPPUGUI::setItem(AccountingBillItem *b) {
    if( m_d->billItem != b || m_d->TAMBillItem != NULL ){
        if( m_d->billItem != NULL ){
            disconnect( m_d->billItem, &AccountingBillItem::dateChanged, m_d->ui->dateLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->dateLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setDateLE );
            disconnect( m_d->billItem, &AccountingBillItem::quantityChanged, m_d->ui->quantityLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->quantityLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setQuantityLE );

            disconnect( m_d->billItem, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            disconnect( m_d->billItem, &AccountingBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->billItem, &AccountingBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToBDiscountedLineEdit, &QLineEdit::setText );

            disconnect( m_d->billItem, &AccountingBillItem::priceItemChanged, this, &AccountingItemPPUGUI::connectPriceItem );
            disconnectPriceItem( m_d->billItem->priceItem() );

            disconnect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &AccountingItemPPUGUI::associateLinesModel );
            disconnect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemPPUGUI::setAccountingItemNULL );
        }
        if( m_d->TAMBillItem != NULL ){
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::dateChanged, m_d->ui->dateLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->dateLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setDateLE );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::quantityChanged, m_d->ui->quantityLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->quantityLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setQuantityLE );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToBDiscountedLineEdit, &QLineEdit::setText );

            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::priceItemChanged, this, &AccountingItemPPUGUI::connectPriceItem );
            disconnectPriceItem( m_d->TAMBillItem->priceItem() );

            disconnect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &AccountingItemPPUGUI::associateLinesModel );
            disconnect( m_d->TAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingItemPPUGUI::setAccountingItemNULL );
        }

        m_d->ui->PPUTotalToDiscountLineEdit->clear();
        m_d->ui->PPUNotToBDiscountedLineEdit->clear();

        m_d->billItem = b;
        m_d->TAMBillItem = NULL;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->billItem != NULL ){
            m_d->ui->dateLineEdit->setText( m_d->billItem->dateStr() );
            connect( m_d->billItem, &AccountingBillItem::dateChanged, m_d->ui->dateLineEdit, &QLineEdit::setText );
            connect( m_d->ui->dateLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setDateLE );
            m_d->ui->quantityLineEdit->setText( m_d->billItem->quantityStr() );
            connect( m_d->billItem, &AccountingBillItem::quantityChanged, m_d->ui->quantityLineEdit, &QLineEdit::setText );
            connect( m_d->ui->quantityLineEdit, &QLineEdit::editingFinished, this, &AccountingItemPPUGUI::setQuantityLE );

            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->billItem->totalAmountToDiscountStr() );
            connect( m_d->billItem, &AccountingBillItem::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->billItem->amountNotToDiscountStr() );
            connect( m_d->billItem, &AccountingBillItem::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->billItem->totalAmountStr() );
            connect( m_d->billItem, &AccountingBillItem::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            connectPriceItem( NULL, m_d->billItem->priceItem());
            connect( m_d->billItem, &AccountingBillItem::priceItemChanged, this, &AccountingItemPPUGUI::connectPriceItem );

            m_d->ui->PPUTotalToDiscountLineEdit->setText( m_d->billItem->PPUTotalToDiscountStr() );
            connect( m_d->billItem, &AccountingBillItem::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->PPUNotToBDiscountedLineEdit->setText( m_d->billItem->PPUNotToDiscountStr() );
            connect( m_d->billItem, &AccountingBillItem::PPUNotToDiscountChanged, m_d->ui->PPUNotToBDiscountedLineEdit, &QLineEdit::setText );

            m_d->priceItemGUI->setCurrentPriceDataSet( m_d->billItem->currentPriceDataSet() );

            m_d->ui->itemMeasuresTableView->setModel( m_d->billItem->measuresModel() );

            associateLinesModel( m_d->billItem->measuresModel() != NULL );

            m_d->ui->associateLinesCheckBox->setChecked(  m_d->billItem->measuresModel() != NULL  );
            connect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &AccountingItemPPUGUI::associateLinesModel );

            connect( m_d->billItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingItemPPUGUI::setAccountingItemNULL );
        } else {
            m_d->ui->quantityLineEdit->clear();
            m_d->ui->priceCodeLineEdit->clear();
            m_d->ui->priceShortDescLineEdit->clear();

            m_d->ui->PPUTotalToDiscountLineEdit->clear();
            m_d->ui->PPUNotToBDiscountedLineEdit->clear();

            m_d->ui->totalAmountToDiscountLineEdit->clear();
            m_d->ui->amountNotToDiscountLineEdit->clear();
            m_d->ui->totalAmountLineEdit->clear();

            m_d->priceItemGUI->setPriceItemNULL();
            m_d->ui->itemMeasuresTableView->setModel( NULL );
            associateLinesModel( false );
        }
    }
}

void AccountingItemPPUGUI::setAccountingTAMBill(AccountingTAMBill *b) {
    if( b != NULL ){
        m_d->itemAttributeModel->setAttributeModel( b->attributeModel() );
    } else {
        m_d->itemAttributeModel->setAttributeModel( NULL );
    }
    // quando si cambia computo corrente la scheda della riga si azzera
    setItem( (AccountingTAMBillItem *)(NULL) );
}

void AccountingItemPPUGUI::setAccountingBill(AccountingBill *b) {
    if( b != NULL ){
        m_d->itemAttributeModel->setAttributeModel( b->attributesModel() );
    } else {
        m_d->itemAttributeModel->setAttributeModel( NULL );
    }
    // quando si cambia computo corrente la scheda della riga si azzera
    setItem( (AccountingBillItem *)(NULL) );
}

void AccountingItemPPUGUI::changeItemDateGUI(){
    if( m_d->TAMBillItem != NULL ){
        QDate d = m_d->TAMBillItem->date();
        QCalendarDialog dialog( &d, this );
        if( dialog.exec() == QDialog::Accepted ){
            m_d->TAMBillItem->setDate( d );
        }
    }
}

void AccountingItemPPUGUI::setDateLE(){
    if( m_d->TAMBillItem != NULL ){
        m_d->TAMBillItem->setDate( m_d->ui->quantityLineEdit->text() );
    }
}

void AccountingItemPPUGUI::setQuantityLE(){
    if( m_d->TAMBillItem != NULL ){
        m_d->TAMBillItem->setQuantity( m_d->ui->quantityLineEdit->text() );
    }
}

void AccountingItemPPUGUI::disconnectPriceItem( PriceItem * priceItem ) {
    if( priceItem != NULL ){
        disconnect( priceItem, &PriceItem::codeFullChanged, m_d->ui->priceCodeLineEdit, &QLineEdit::setText );
        disconnect( priceItem, &PriceItem::shortDescriptionFullChanged, m_d->ui->priceShortDescLineEdit, &QLineEdit::setText );
        disconnect( priceItem, &PriceItem::unitMeasureChanged, this, &AccountingItemPPUGUI::connectPriceUnitMeasure );
        if( priceItem->unitMeasure() ){
            disconnect( priceItem->unitMeasure(), &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
        }
        m_d->ui->priceCodeLineEdit->clear();
        m_d->ui->priceShortDescLineEdit->clear();
        m_d->ui->priceUnitMeasureLineEdit->clear();
        m_d->priceItemGUI->setPriceItem( NULL );
    }
}

void AccountingItemPPUGUI::connectPriceItem( PriceItem * oldPriceItem, PriceItem * newPriceItem ) {
    disconnectPriceItem( oldPriceItem );

    if( newPriceItem != NULL ){
        m_d->ui->priceCodeLineEdit->setText( newPriceItem->codeFull() );
        m_d->ui->priceShortDescLineEdit->setText( newPriceItem->shortDescriptionFull() );
        if( newPriceItem->unitMeasure() ){
            m_d->ui->priceUnitMeasureLineEdit->setText( newPriceItem->unitMeasure()->tag() );
        } else {
            m_d->ui->priceUnitMeasureLineEdit->setText( QString("---") );
        }

        connect( newPriceItem, &PriceItem::codeFullChanged, m_d->ui->priceCodeLineEdit, &QLineEdit::setText );
        connect( newPriceItem, &PriceItem::shortDescriptionFullChanged, m_d->ui->priceShortDescLineEdit, &QLineEdit::setText );
        connect( newPriceItem, &PriceItem::unitMeasureChanged, this, &AccountingItemPPUGUI::connectPriceUnitMeasure );
        if( newPriceItem->unitMeasure() ){
            connect( newPriceItem->unitMeasure(), &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
        }
        m_d->priceItemGUI->setPriceItem( newPriceItem );
    } else {
        m_d->ui->priceCodeLineEdit->clear();
        m_d->ui->priceShortDescLineEdit->clear();
        m_d->ui->priceUnitMeasureLineEdit->clear();
        m_d->priceItemGUI->setPriceItemNULL();
    }
}

void AccountingItemPPUGUI::connectPriceUnitMeasure(){
    if( m_d->TAMBillItem != NULL ){
        if( m_d->TAMBillItem->priceItem() != NULL ) {
            if( m_d->connectedUnitMeasure != NULL ){
                disconnect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }

            m_d->connectedUnitMeasure = m_d->TAMBillItem->priceItem()->unitMeasure();

            if( m_d->connectedUnitMeasure != NULL ){
                m_d->ui->priceUnitMeasureLineEdit->setText( m_d->connectedUnitMeasure->tag()  );
                connect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }
        }
    } else if( m_d->billItem != NULL ){
        if( m_d->billItem->priceItem() != NULL ) {
            if( m_d->connectedUnitMeasure != NULL ){
                disconnect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }

            m_d->connectedUnitMeasure = m_d->billItem->priceItem()->unitMeasure();

            if( m_d->connectedUnitMeasure != NULL ){
                m_d->ui->priceUnitMeasureLineEdit->setText( m_d->connectedUnitMeasure->tag()  );
                connect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }
        }
    }

}

void AccountingItemPPUGUI::associateLinesModel(bool ass) {
    m_d->ui->itemMeasuresGroupBox->setVisible( ass );
    m_d->ui->quantityLineEdit->setReadOnly( ass );
    if( ass ){
        if( m_d->TAMBillItem != NULL ){
            m_d->ui->itemMeasuresTableView->setModel( m_d->TAMBillItem->generateMeasuresModel() );
        }
        if( m_d->billItem != NULL ){
            m_d->ui->itemMeasuresTableView->setModel( m_d->billItem->generateMeasuresModel() );
        }
        if( m_d->vSpacer != NULL ){
            m_d->ui->dataTabLayout->removeItem( m_d->vSpacer);
            delete m_d->vSpacer;
            m_d->vSpacer = NULL;
        }
    } else {
        if( m_d->TAMBillItem != NULL ){
            m_d->TAMBillItem->removeMeasuresModel();
        }
        if( m_d->billItem != NULL ){
            m_d->billItem->removeMeasuresModel();
        }
        m_d->ui->itemMeasuresTableView->setModel( NULL );
        if( m_d->vSpacer == NULL ){
            m_d->vSpacer = new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding);
            m_d->ui->dataTabLayout->addItem( m_d->vSpacer, 6, 0, 1, 3 );
        }
    }
}

void AccountingItemPPUGUI::addMeasureLines() {
    if( m_d->TAMBillItem != NULL ){
        if( m_d->TAMBillItem->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->itemMeasuresTableView->selectionModel()->selectedIndexes();
            QList<int> rowList;
            for( int i=0; i < rowListSelected.size(); i++ ){
                if( !rowList.contains(rowListSelected.at(i).row()) ){
                    rowList.append( rowListSelected.at(i).row() );
                }
            }
            qSort( rowList.begin(), rowList.end() );

            if( rowList.size() > 0 ){
                m_d->TAMBillItem->measuresModel()->insertRows( rowList.last()+1, rowList.size());
            } else {
                m_d->TAMBillItem->measuresModel()->insertRows( 0 );
            }
        }
    } else if( m_d->billItem != NULL ){
        if( m_d->billItem->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->itemMeasuresTableView->selectionModel()->selectedIndexes();
            QList<int> rowList;
            for( int i=0; i < rowListSelected.size(); i++ ){
                if( !rowList.contains(rowListSelected.at(i).row()) ){
                    rowList.append( rowListSelected.at(i).row() );
                }
            }
            qSort( rowList.begin(), rowList.end() );

            if( rowList.size() > 0 ){
                m_d->billItem->measuresModel()->insertRows( rowList.last()+1, rowList.size());
            } else {
                m_d->billItem->measuresModel()->insertRows( 0 );
            }
        }
    }
}

void AccountingItemPPUGUI::delMeasureLines() {
    if( m_d->TAMBillItem != NULL ){
        if( m_d->TAMBillItem->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->itemMeasuresTableView->selectionModel()->selectedRows();
            if( !rowListSelected.isEmpty() ){
                QList<int> rowList;
                for( int i=0; i < rowListSelected.size(); i++ ){
                    if( !rowList.contains(rowListSelected.at(i).row()) ){
                        rowList.append( rowListSelected.at(i).row() );
                    }
                }
                qSort( rowList.begin(), rowList.end() );
                m_d->TAMBillItem->measuresModel()->removeRows( rowList.first(), rowList.size() );
            }
        }
    } else if( m_d->billItem != NULL ){
        if( m_d->billItem->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->itemMeasuresTableView->selectionModel()->selectedRows();
            if( !rowListSelected.isEmpty() ){
                QList<int> rowList;
                for( int i=0; i < rowListSelected.size(); i++ ){
                    if( !rowList.contains(rowListSelected.at(i).row()) ){
                        rowList.append( rowListSelected.at(i).row() );
                    }
                }
                qSort( rowList.begin(), rowList.end() );
                m_d->billItem->measuresModel()->removeRows( rowList.first(), rowList.size() );
            }
        }
    }
}

void AccountingItemPPUGUI::importAccountingMeasureMeasuresTXT() {
    if( m_d->TAMBillItem != NULL ){
        if( m_d->TAMBillItem->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->itemMeasuresTableView->selectionModel()->selectedRows();
            QList<int> rowList;
            for( int i=0; i < rowListSelected.size(); i++ ){
                if( !rowList.contains(rowListSelected.at(i).row()) ){
                    rowList.append( rowListSelected.at(i).row() );
                }
            }
            qSort( rowList.begin(), rowList.end() );

            int position = m_d->TAMBillItem->measuresModel()->rowCount();
            if( rowList.size() > 0 ){
                position = rowList.last()+1;
            }

            ImportBillItemMeasuresTXT dialog( m_d->TAMBillItem->measuresModel(), position, m_d->parser, this );
            dialog.exec();
        }
    } else if( m_d->billItem != NULL ){
        if( m_d->billItem->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->itemMeasuresTableView->selectionModel()->selectedRows();
            QList<int> rowList;
            for( int i=0; i < rowListSelected.size(); i++ ){
                if( !rowList.contains(rowListSelected.at(i).row()) ){
                    rowList.append( rowListSelected.at(i).row() );
                }
            }
            qSort( rowList.begin(), rowList.end() );

            int position = m_d->billItem->measuresModel()->rowCount();
            if( rowList.size() > 0 ){
                position = rowList.last()+1;
            }

            ImportBillItemMeasuresTXT dialog( m_d->billItem->measuresModel(), position, m_d->parser, this );
            dialog.exec();
        }
    }
}

void AccountingItemPPUGUI::setAccountingItemNULL() {
    setItem( (AccountingTAMBillItem *)( NULL ));
    setItem( (AccountingBillItem *)( NULL ));
}

void AccountingItemPPUGUI::addAttribute(){
    if( m_d->itemAttributeModel != NULL ){
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

void AccountingItemPPUGUI::removeAttribute(){
    if( m_d->itemAttributeModel != NULL ){
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

bool AccountingItemPPUGUI::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseButtonDblClick )     {
        if( m_d->TAMBillItem != NULL ){
            if( object == m_d->ui->dateLineEdit ){
                QDate d = m_d->TAMBillItem->date();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->TAMBillItem->setDate( d );
                }
            }
        }
        if( m_d->billItem != NULL ){
            if( object == m_d->ui->dateLineEdit ){
                QDate d = m_d->billItem->date();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->billItem->setDate( d );
                }
            }
        }
    }
    return false;
}
