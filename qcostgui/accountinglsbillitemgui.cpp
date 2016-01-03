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

#include "accountinglsbillitemgui.h"
#include "ui_accountinglsbillitemgui.h"

#include "priceitemgui.h"
#include "importlsitemmeasurestxt.h"
#include "qcalendardialog.h"

#include "project.h"
#include "accountinglsbill.h"
#include "measureslsmodel.h"
#include "accountinglsitemmeasure.h"
#include "accountingitemattributemodel.h"
#include "accountinglsbillitem.h"
#include "bill.h"
#include "priceitem.h"
#include "unitmeasure.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QDate>

class AccountingLSBillItemGUIPrivate{
public:
    AccountingLSBillItemGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions, QString * EPAFileName, MathParser * prs, Project * prj ):
        ui(new Ui::AccountingLSBillItemGUI),
        parser(prs),
        item(NULL),
        bill(NULL),
        connectedUnitMeasure(NULL),
        itemAttributeModel( new AccountingItemAttributeModel(NULL, NULL) ),
        priceItemGUI( new PriceItemGUI( EPAImpOptions, EPAFileName, NULL, 0, prs, prj, NULL )),
        vSpacer(NULL),
        priceFieldModel( prj->priceFieldModel() ){
    }
    ~AccountingLSBillItemGUIPrivate(){
        delete ui;
        delete itemAttributeModel;
    }
    Ui::AccountingLSBillItemGUI * ui;
    MathParser * parser;
    AccountingLSBillItem * item;
    AccountingLSBill * bill;
    UnitMeasure * connectedUnitMeasure;
    AccountingItemAttributeModel * itemAttributeModel;
    PriceItemGUI * priceItemGUI;
    QSpacerItem * vSpacer;

    PriceFieldModel * priceFieldModel;
};

AccountingLSBillItemGUI::AccountingLSBillItemGUI(QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                                                 QString * EPAFileName,
                                                 MathParser * prs,
                                                 Project * prj,
                                                 QWidget *parent) :
    QWidget(parent),
    m_d(new AccountingLSBillItemGUIPrivate( EPAImpOptions, EPAFileName, prs, prj ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->priceGroupBox->setHidden( true );
    m_d->ui->amountsGroupBox->setHidden( true );
    m_d->ui->priceTab->layout()->addWidget( m_d->priceItemGUI );
    m_d->ui->attributeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );

    connect( m_d->ui->addMeasurePushButton, &QPushButton::clicked, this, &AccountingLSBillItemGUI::addMeasureLines );
    connect( m_d->ui->delMeasurePushButton, &QPushButton::clicked, this, &AccountingLSBillItemGUI::delMeasureLines );
    connect( m_d->ui->importMeasuresTXTPushButton, &QPushButton::clicked, this, &AccountingLSBillItemGUI::importMeasuresTXT );

    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillItemGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillItemGUI::removeAttribute );

    connect( m_d->priceItemGUI, static_cast<void(PriceItemGUI::*)(PriceItem *, Bill *)>(&PriceItemGUI::editPriceItemAP), this, &AccountingLSBillItemGUI::editPriceItemAP );

    connect( m_d->ui->measuresTableView, &QTableView::doubleClicked, this, &AccountingLSBillItemGUI::editMeasureDate );
}

AccountingLSBillItemGUI::~AccountingLSBillItemGUI() {
    delete m_d;
}

void AccountingLSBillItemGUI::setBillItem(AccountingLSBillItem *b) {
    if( m_d->item != b ){
        if( m_d->item != NULL ){
            disconnect( m_d->item, &AccountingLSBillItem::projQuantityChanged, m_d->ui->projQuantityLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::projAmountChanged, m_d->ui->projAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::accQuantityChanged, m_d->ui->accQuantityLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::accAmountChanged, m_d->ui->accAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::PPUChanged, m_d->ui->PPULineEdit, &QLineEdit::setText );
            disconnect( m_d->item, &AccountingLSBillItem::priceItemChanged, this, &AccountingLSBillItemGUI::connectPriceItem );
            disconnectPriceItem( m_d->item->priceItem() );
            disconnect( m_d->item, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingLSBillItemGUI::clear );
        }

        m_d->ui->projQuantityLineEdit->clear();
        m_d->ui->accQuantityLineEdit->clear();
        m_d->ui->PPULineEdit->clear();
        m_d->ui->projAmountLineEdit->clear();
        m_d->ui->accAmountLineEdit->clear();

        m_d->item = b;
        m_d->itemAttributeModel->setItem( b );

        if( m_d->item != NULL ){
            m_d->ui->projQuantityLineEdit->setText( m_d->item->projQuantityStr() );
            connect( m_d->item, &AccountingLSBillItem::projQuantityChanged, m_d->ui->projQuantityLineEdit, &QLineEdit::setText );

            m_d->ui->projAmountLineEdit->setText( m_d->item->projAmountStr() );
            connect( m_d->item, &AccountingLSBillItem::projAmountChanged, m_d->ui->projAmountLineEdit, &QLineEdit::setText );

            m_d->ui->accQuantityLineEdit->setText( m_d->item->accQuantityStr() );
            connect( m_d->item, &AccountingLSBillItem::accQuantityChanged, m_d->ui->accQuantityLineEdit, &QLineEdit::setText );

            m_d->ui->accAmountLineEdit->setText( m_d->item->accAmountStr() );
            connect( m_d->item, &AccountingLSBillItem::accAmountChanged, m_d->ui->accAmountLineEdit, &QLineEdit::setText );

            m_d->ui->percentageAccountedLineEdit->setText( m_d->item->percentageAccountedStr() );
            connect( m_d->item, &AccountingLSBillItem::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );

            m_d->ui->PPULineEdit->setText( m_d->item->PPUStr() );
            connect( m_d->item, &AccountingLSBillItem::PPUChanged, m_d->ui->PPULineEdit, &QLineEdit::setText );

            connect( m_d->item, &AccountingLSBillItem::priceItemChanged, this, &AccountingLSBillItemGUI::connectPriceItem );
            connectPriceItem( NULL, m_d->item->priceItem());

            m_d->priceItemGUI->setCurrentPriceDataSet( m_d->item->currentPriceDataSet() );

            m_d->ui->measuresTableView->setModel( m_d->item->measuresModel() );

            connect( m_d->item, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingLSBillItemGUI::clear );
        }
    }
}

void AccountingLSBillItemGUI::setBill(AccountingLSBill *b) {
    if( m_d->bill != b ){
        if( m_d->bill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( NULL );
        }

        m_d->bill = b;

        if( m_d->bill != NULL ){
            m_d->itemAttributeModel->setAttributeModel( b->attributesModel() );
            connect( m_d->bill, &AccountingLSBill::aboutToBeDeleted, this, &AccountingLSBillItemGUI::setBillNULL );
        }

        // quando si cambia computo corrente la scheda della riga si azzera
        clear();
    }
}

void AccountingLSBillItemGUI::setBillNULL() {
    setBill(NULL);
}

void AccountingLSBillItemGUI::disconnectPriceItem( PriceItem * priceItem ) {
    if( priceItem != NULL ){
        disconnect( priceItem, &PriceItem::codeFullChanged, m_d->ui->priceCodeLineEdit, &QLineEdit::setText );
        disconnect( priceItem, &PriceItem::shortDescriptionFullChanged, m_d->ui->priceShortDescLineEdit, &QLineEdit::setText );
        disconnect( priceItem, &PriceItem::unitMeasureChanged, this, &AccountingLSBillItemGUI::connectPriceUnitMeasure );
        if( priceItem->unitMeasure() ){
            disconnect( priceItem->unitMeasure(), &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
        }
        m_d->priceItemGUI->setPriceItem( NULL );
    }
}

void AccountingLSBillItemGUI::connectPriceItem( PriceItem * oldPriceItem, PriceItem * newPriceItem ) {
    if( m_d->item ){

        disconnectPriceItem( oldPriceItem );

        if( newPriceItem != NULL ){
            m_d->ui->priceCodeLineEdit->setText( newPriceItem->codeFull() );
            m_d->ui->priceShortDescLineEdit->setText( newPriceItem->shortDescriptionFull() );
            if( newPriceItem->unitMeasure() ){
                m_d->ui->priceUnitMeasureLineEdit->setText( newPriceItem->unitMeasure()->tag() );
            }

            connect( newPriceItem, &PriceItem::codeFullChanged, m_d->ui->priceCodeLineEdit, &QLineEdit::setText );
            connect( newPriceItem, &PriceItem::shortDescriptionFullChanged, m_d->ui->priceShortDescLineEdit, &QLineEdit::setText );
            connect( newPriceItem, &PriceItem::unitMeasureChanged, this, &AccountingLSBillItemGUI::connectPriceUnitMeasure );
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
}

void AccountingLSBillItemGUI::connectPriceUnitMeasure(){
    if( m_d->item ){
        if( m_d->item->priceItem()){
            if( m_d->connectedUnitMeasure != NULL ){
                disconnect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }

            m_d->connectedUnitMeasure = m_d->item->priceItem()->unitMeasure();

            if( m_d->connectedUnitMeasure != NULL ){
                m_d->ui->priceUnitMeasureLineEdit->setText( m_d->connectedUnitMeasure->tag()  );
                connect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }
        }
    }
}

void AccountingLSBillItemGUI::addMeasureLines() {
    if( m_d->item != NULL ){
        if( m_d->item->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->measuresTableView->selectionModel()->selectedIndexes();
            QList<int> rowList;
            for( int i=0; i < rowListSelected.size(); i++ ){
                if( !rowList.contains(rowListSelected.at(i).row()) ){
                    rowList.append( rowListSelected.at(i).row() );
                }
            }
            qSort( rowList.begin(), rowList.end() );

            if( rowList.size() > 0 ){
                m_d->item->measuresModel()->insertRows( rowList.last()+1, rowList.size());
            } else {
                m_d->item->measuresModel()->insertRows( 0 );
            }
        }
    }
}

void AccountingLSBillItemGUI::delMeasureLines() {
    if( m_d->item != NULL ){
        if( m_d->item->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->measuresTableView->selectionModel()->selectedRows();
            if( !rowListSelected.isEmpty() ){
                QList<int> rowList;
                for( int i=0; i < rowListSelected.size(); i++ ){
                    if( !rowList.contains(rowListSelected.at(i).row()) ){
                        rowList.append( rowListSelected.at(i).row() );
                    }
                }
                qSort( rowList.begin(), rowList.end() );
                m_d->item->measuresModel()->removeRows( rowList.first(), rowList.size() );
            }
        }
    }
}

void AccountingLSBillItemGUI::importMeasuresTXT() {
    if( m_d->item != NULL ){
        if( m_d->item->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->measuresTableView->selectionModel()->selectedRows();
            QList<int> rowList;
            for( int i=0; i < rowListSelected.size(); i++ ){
                if( !rowList.contains(rowListSelected.at(i).row()) ){
                    rowList.append( rowListSelected.at(i).row() );
                }
            }
            qSort( rowList.begin(), rowList.end() );

            int position = m_d->item->measuresModel()->rowCount();
            if( rowList.size() > 0 ){
                position = rowList.last()+1;
            }

            ImportLSItemMeasuresTXT dialog( m_d->item->measuresModel(), position, m_d->parser, this );
            dialog.exec();
        }
    }
}

void AccountingLSBillItemGUI::clear() {
    setBillItem( NULL );
}

void AccountingLSBillItemGUI::addAttribute(){
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

void AccountingLSBillItemGUI::removeAttribute(){
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

void AccountingLSBillItemGUI::editMeasureDate( const QModelIndex & index ) {
    if( m_d->item != NULL ){
        if( index.column() == m_d->item->measuresModel()->accDateCol() ){
            AccountingLSItemMeasure * m = m_d->item->measuresModel()->measure(index.row() );
            QDate d = m->accDate();
            QCalendarDialog dialog( &d, this );
            if( dialog.exec() == QDialog::Accepted ){
                m->setAccDate( d );
            }
        }
    }
}
