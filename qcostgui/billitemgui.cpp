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
#include "billitemgui.h"
#include "ui_billitemgui.h"

#include "priceitemgui.h"
#include "importbillitemmeasurestxt.h"

#include "project.h"
#include "bill.h"
#include "measuresmodel.h"
#include "billitemattributemodel.h"
#include "billitem.h"
#include "priceitem.h"
#include "unitmeasure.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

class BillItemGUIPrivate{
public:
    BillItemGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions, QString * EPAFileName, MathParser * prs, Project * prj ):
        ui(new Ui::BillItemGUI),
        parser(prs),
        item(nullptr),
        bill(nullptr),
        connectedUnitMeasure(nullptr),
        itemAttributeModel( new BillItemAttributeModel(nullptr, nullptr) ),
        priceItemGUI( new PriceItemGUI( EPAImpOptions, EPAFileName, nullptr, 0, prs, prj, nullptr )),
        vSpacer(nullptr),
        priceFieldModel( prj->priceFieldModel() ){
    }
    ~BillItemGUIPrivate(){
        delete ui;
        delete itemAttributeModel;
    }
    Ui::BillItemGUI * ui;
    MathParser * parser;
    BillItem * item;
    Bill * bill;
    UnitMeasure * connectedUnitMeasure;
    BillItemAttributeModel * itemAttributeModel;
    PriceItemGUI * priceItemGUI;
    QSpacerItem * vSpacer;

    PriceFieldModel * priceFieldModel;
    QList<QLabel* > priceDataFieldLabel;
    QList<QLineEdit *> priceDataFieldLEdit;
    QList<QLabel* > amountDataFieldLabel;
    QList<QLineEdit *> amountDataFieldLEdit;
};

BillItemGUI::BillItemGUI( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                          QString * EPAFileName,
                          BillItem *b,
                          MathParser * prs,
                          Project * prj,
                          QWidget *parent) :
    QWidget(parent),
    m_d(new BillItemGUIPrivate( EPAImpOptions, EPAFileName, prs, prj ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->priceGroupBox->setHidden( true );
    m_d->ui->amountsGroupBox->setHidden( true );
    m_d->ui->priceTab->layout()->addWidget( m_d->priceItemGUI );
    m_d->ui->attributeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_d->ui->attributeTableView->setModel( m_d->itemAttributeModel );
    associateLinesModel( false );
    setBillItem( b );

    connect( m_d->ui->addBillItemLinePushButton, &QPushButton::clicked, this, &BillItemGUI::addMeasureLines );
    connect( m_d->ui->delBillItemLinePushButton, &QPushButton::clicked, this, &BillItemGUI::delMeasureLines );
    connect( m_d->ui->importBillItemMeasuresTXTPushButton, &QPushButton::clicked, this, &BillItemGUI::importBillItemMeasuresTXT );

    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &BillItemGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &BillItemGUI::removeAttribute );

    connect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &BillItemGUI::updatePriceAmountNamesValues );
    connect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &BillItemGUI::updatePriceAmountNamesValues );
    updatePriceAmountNamesValues();
    connect( m_d->priceFieldModel, &PriceFieldModel::priceNameChanged, this, &BillItemGUI::updatePriceName );
    connect( m_d->priceFieldModel, &PriceFieldModel::amountNameChanged, this, &BillItemGUI::updateAmountName );

    connect( m_d->priceItemGUI, static_cast<void(PriceItemGUI::*)(PriceItem *, Bill *)>(&PriceItemGUI::editPriceItemAP), this, &BillItemGUI::editPriceItemAP );
}

BillItemGUI::~BillItemGUI() {
    delete m_d;
}

void BillItemGUI::setBillItem(BillItem *b) {
    if( m_d->item != b ){
        if( m_d->item != nullptr ){
            disconnect( m_d->item, &BillItem::currentPriceDataSetChanged, this, &BillItemGUI::updatePriceValues );
            disconnect( m_d->item, &BillItem::quantityChanged, m_d->ui->quantityLineEdit, &QLineEdit::setText );
            disconnect( m_d->ui->quantityLineEdit, &QLineEdit::editingFinished, this, &BillItemGUI::setQuantityLE );
            disconnect( m_d->item, static_cast<void(BillItem::*)(int,const QString &)>(&BillItem::amountChanged), this, &BillItemGUI::updateAmountValue );
            disconnect( m_d->item, &BillItem::priceItemChanged, this, &BillItemGUI::connectPriceItem );
            disconnectPriceItem( m_d->item->priceItem() );
            disconnect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &BillItemGUI::associateLinesModel );
            disconnect( m_d->item, &BillItem::aboutToBeDeleted, this, &BillItemGUI::setBillItemnullptr );
        }

        m_d->item = b;
        m_d->itemAttributeModel->setBillItem( b );

        if( m_d->item != nullptr ){
            connect( m_d->item, &BillItem::currentPriceDataSetChanged, this, &BillItemGUI::updatePriceValues );

            m_d->ui->quantityLineEdit->setText( m_d->item->quantityStr() );
            connect( m_d->item, &BillItem::quantityChanged, m_d->ui->quantityLineEdit, &QLineEdit::setText );
            connect( m_d->ui->quantityLineEdit, &QLineEdit::editingFinished, this, &BillItemGUI::setQuantityLE );
            connect( m_d->item, static_cast<void(BillItem::*)(int,const QString &)>(&BillItem::amountChanged), this, &BillItemGUI::updateAmountValue );
            updateAmountValues();

            connect( m_d->item, &BillItem::priceItemChanged, this, &BillItemGUI::connectPriceItem );
            connectPriceItem( nullptr, m_d->item->priceItem());

            m_d->priceItemGUI->setCurrentPriceDataSet( m_d->item->currentPriceDataSet() );

            m_d->ui->billItemLinesTableView->setModel( m_d->item->measuresModel() );

            associateLinesModel( m_d->item->measuresModel() != nullptr );

            m_d->ui->associateLinesCheckBox->setChecked(  m_d->item->measuresModel() != nullptr  );
            connect( m_d->ui->associateLinesCheckBox, &QCheckBox::toggled, this, &BillItemGUI::associateLinesModel );

            connect( m_d->item, &BillItem::aboutToBeDeleted, this, &BillItemGUI::setBillItemnullptr );
        } else {
            m_d->ui->quantityLineEdit->clear();
            m_d->ui->priceCodeLineEdit->clear();
            m_d->ui->priceShortDescLineEdit->clear();
            for( int i=0; i < m_d->priceDataFieldLEdit.size(); ++i ){
                m_d->priceDataFieldLEdit.at(i)->clear();
            }
            for( int i=0; i < m_d->amountDataFieldLEdit.size(); ++i ){
                m_d->amountDataFieldLEdit.at(i)->clear();
            }
            m_d->priceItemGUI->setPriceItemnullptr();
            m_d->ui->billItemLinesTableView->setModel( nullptr );
            associateLinesModel( false );
        }
    }
}

void BillItemGUI::setBill(Bill *b) {
    if( b != m_d->bill ){
        if( m_d->bill != nullptr ){
            m_d->itemAttributeModel->setAttributeModel( nullptr );
            disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &BillItemGUI::setBillnullptr );
        }

        m_d->bill = b;
        if( m_d->bill != nullptr ){
            m_d->itemAttributeModel->setAttributeModel( b->attributesModel() );
            connect( m_d->bill, &Bill::aboutToBeDeleted, this, &BillItemGUI::setBillnullptr );
        }

        // quando si cambia computo corrente la scheda della riga si azzera
        setBillItemnullptr();
    }
}

void BillItemGUI::setBillnullptr() {
    setBill(nullptr);
}

void BillItemGUI::setQuantityLE(){
    if( m_d->item != nullptr ){
        m_d->item->setQuantity( m_d->ui->quantityLineEdit->text() );
    }
}

void BillItemGUI::disconnectPriceItem( PriceItem * priceItem ) {
    if( priceItem != nullptr ){
        disconnect( priceItem, &PriceItem::codeFullChanged, m_d->ui->priceCodeLineEdit, &QLineEdit::setText );
        disconnect( priceItem, &PriceItem::shortDescriptionFullChanged, m_d->ui->priceShortDescLineEdit, &QLineEdit::setText );
        disconnect( priceItem, &PriceItem::unitMeasureChanged, this, &BillItemGUI::connectPriceUnitMeasure );
        disconnect( priceItem, &PriceItem::valueChanged, this, &BillItemGUI::updatePriceValue );
        if( priceItem->unitMeasure() ){
            disconnect( priceItem->unitMeasure(), &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
        }
        m_d->priceItemGUI->setPriceItem( nullptr );
    }
}

void BillItemGUI::connectPriceItem( PriceItem * oldPriceItem, PriceItem * newPriceItem ) {
    if( m_d->item ){

        disconnectPriceItem( oldPriceItem );

        if( newPriceItem != nullptr ){
            m_d->ui->priceCodeLineEdit->setText( newPriceItem->codeFull() );
            m_d->ui->priceShortDescLineEdit->setText( newPriceItem->shortDescriptionFull() );
            if( newPriceItem->unitMeasure() ){
                m_d->ui->priceUnitMeasureLineEdit->setText( newPriceItem->unitMeasure()->tag() );
            }
            for( int i=0; (i < m_d->priceFieldModel->fieldCount()) && (i < m_d->priceDataFieldLEdit.size()); ++i ){
                m_d->priceDataFieldLEdit.at( i )->setText( newPriceItem->valueStr(i, m_d->item->currentPriceDataSet() ) );
            }

            connect( newPriceItem, &PriceItem::codeFullChanged, m_d->ui->priceCodeLineEdit, &QLineEdit::setText );
            connect( newPriceItem, &PriceItem::shortDescriptionFullChanged, m_d->ui->priceShortDescLineEdit, &QLineEdit::setText );
            connect( newPriceItem, &PriceItem::unitMeasureChanged, this, &BillItemGUI::connectPriceUnitMeasure );
            connect( newPriceItem, &PriceItem::valueChanged, this, &BillItemGUI::updatePriceValue );
            if( newPriceItem->unitMeasure() ){
                connect( newPriceItem->unitMeasure(), &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }
            m_d->priceItemGUI->setPriceItem( newPriceItem );
        } else {
            m_d->ui->priceCodeLineEdit->clear();
            m_d->ui->priceShortDescLineEdit->clear();
            m_d->ui->priceUnitMeasureLineEdit->clear();
            for( int i=0; i < m_d->priceDataFieldLEdit.size(); ++i ){
                m_d->priceDataFieldLEdit.at(i)->clear();
            }
            m_d->priceItemGUI->setPriceItemnullptr();
        }
    }
}

void BillItemGUI::connectPriceUnitMeasure(){
    if( m_d->item ){
        if( m_d->item->priceItem()){
            if( m_d->connectedUnitMeasure != nullptr ){
                disconnect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }

            m_d->connectedUnitMeasure = m_d->item->priceItem()->unitMeasure();

            if( m_d->connectedUnitMeasure != nullptr ){
                m_d->ui->priceUnitMeasureLineEdit->setText( m_d->connectedUnitMeasure->tag()  );
                connect(  m_d->connectedUnitMeasure, &UnitMeasure::tagChanged, m_d->ui->priceUnitMeasureLineEdit, &QLineEdit::setText );
            }
        }
    }
}

void BillItemGUI::associateLinesModel(bool ass) {
    if( ass ){
        if( m_d->item != nullptr ){
            m_d->ui->billItemLinesGroupBox->setVisible( true );
            m_d->ui->billItemLinesTableView->setModel( m_d->item->generateMeasuresModel() );
            m_d->ui->quantityLineEdit->setReadOnly( true );
            if( m_d->vSpacer != nullptr ){
                m_d->ui->dataTabLayout->removeItem( m_d->vSpacer);
                delete m_d->vSpacer;
                m_d->vSpacer = nullptr;
            }
            return;
        }
    }
    if( m_d->item != nullptr ){
        m_d->item->removeMeasuresModel();
    }
    m_d->ui->billItemLinesGroupBox->setVisible( false );
    m_d->ui->billItemLinesTableView->setModel( nullptr );
    if( m_d->vSpacer == nullptr ){
        m_d->vSpacer = new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_d->ui->dataTabLayout->addItem( m_d->vSpacer, 6, 0, 1, 3 );
    }
    m_d->ui->quantityLineEdit->setReadOnly( false );
}

void BillItemGUI::addMeasureLines() {
    if( m_d->item != nullptr ){
        if( m_d->item->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->billItemLinesTableView->selectionModel()->selectedIndexes();
            QList<int> rowList;
            for( int i=0; i < rowListSelected.size(); i++ ){
                if( !rowList.contains(rowListSelected.at(i).row()) ){
                    rowList.append( rowListSelected.at(i).row() );
                }
            }
            std::sort( rowList.begin(), rowList.end() );

            if( rowList.size() > 0 ){
                m_d->item->measuresModel()->insertRows( rowList.last()+1, rowList.size());
            } else {
                m_d->item->measuresModel()->insertRows( 0 );
            }
        }
    }
}

void BillItemGUI::delMeasureLines() {
    if( m_d->item != nullptr ){
        if( m_d->item->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->billItemLinesTableView->selectionModel()->selectedRows();
            if( !rowListSelected.isEmpty() ){
                QList<int> rowList;
                for( int i=0; i < rowListSelected.size(); i++ ){
                    if( !rowList.contains(rowListSelected.at(i).row()) ){
                        rowList.append( rowListSelected.at(i).row() );
                    }
                }
                std::sort( rowList.begin(), rowList.end() );
                m_d->item->measuresModel()->removeRows( rowList.first(), rowList.size() );
            }
        }
    }
}

void BillItemGUI::importBillItemMeasuresTXT() {
    if( m_d->item != nullptr ){
        if( m_d->item->measuresModel() ){
            QModelIndexList rowListSelected = m_d->ui->billItemLinesTableView->selectionModel()->selectedRows();
            QList<int> rowList;
            for( int i=0; i < rowListSelected.size(); i++ ){
                if( !rowList.contains(rowListSelected.at(i).row()) ){
                    rowList.append( rowListSelected.at(i).row() );
                }
            }
            std::sort( rowList.begin(), rowList.end() );

            int position = m_d->item->measuresModel()->rowCount();
            if( rowList.size() > 0 ){
                position = rowList.last()+1;
            }

            ImportBillItemMeasuresTXT dialog( m_d->item->measuresModel(), position, m_d->parser, this );
            dialog.exec();
        }
    }
}

void BillItemGUI::setBillItemnullptr() {
    setBillItem( nullptr );
}

void BillItemGUI::addAttribute(){
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

void BillItemGUI::removeAttribute(){
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

void BillItemGUI::updatePriceAmountNamesValues(){
    for( QList<QLabel* >::iterator i = m_d->priceDataFieldLabel.begin(); i != m_d->priceDataFieldLabel.end(); ++i ){
        m_d->ui->priceDataLayout->removeWidget( *i );
        delete *i;
    }
    m_d->priceDataFieldLabel.clear();
    for( QList<QLineEdit* >::iterator i = m_d->priceDataFieldLEdit.begin(); i != m_d->priceDataFieldLEdit.end(); ++i ){
        m_d->ui->priceDataLayout->removeWidget( *i );
        delete *i;
    }
    m_d->priceDataFieldLEdit.clear();
    for( QList<QLabel* >::iterator i = m_d->amountDataFieldLabel.begin(); i != m_d->amountDataFieldLabel.end(); ++i ){
        m_d->ui->amountsDataLayout->removeWidget( *i );
        delete *i;
    }
    m_d->amountDataFieldLabel.clear();
    for( QList<QLineEdit* >::iterator i = m_d->amountDataFieldLEdit.begin(); i != m_d->amountDataFieldLEdit.end(); ++i ){
        m_d->ui->amountsDataLayout->removeWidget( *i );
        delete *i;
    }
    m_d->amountDataFieldLEdit.clear();

    for( int i=0; i<m_d->priceFieldModel->fieldCount(); ++i){
        QLabel * label = new QLabel( m_d->priceFieldModel->priceName( i ) );
        QLineEdit * lEdit = new QLineEdit();
        lEdit->setReadOnly( true );
        m_d->ui->priceDataLayout->addWidget( label, 3+i, 0 );
        m_d->ui->priceDataLayout->addWidget( lEdit, 3+i, 1 );
        lEdit->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
        m_d->priceDataFieldLabel.append( label );
        m_d->priceDataFieldLEdit.append( lEdit );

        label = new QLabel( m_d->priceFieldModel->amountName( i ) );
        lEdit = new QLineEdit();
        lEdit->setReadOnly( true );
        lEdit->setAlignment( Qt::AlignRight );
        if( m_d->item != nullptr ){
            lEdit->setText( m_d->item->amountStr(i) );
        }
        m_d->ui->amountsDataLayout->addWidget( label, i, 0 );
        m_d->ui->amountsDataLayout->addWidget( lEdit, i, 1 );
        m_d->amountDataFieldLabel.append( label );
        m_d->amountDataFieldLEdit.append( lEdit );
    }
}

void BillItemGUI::updateAmountValue(int priceField, const QString & newVal){
    if( priceField > -1 && priceField < m_d->amountDataFieldLEdit.size() ){
        m_d->amountDataFieldLEdit.at(priceField)->setText( newVal );
    }
}

void BillItemGUI::updateAmountValues(){
    for( int i = 0; i < m_d->amountDataFieldLEdit.size(); ++i){
        if( (i < m_d->priceFieldModel->fieldCount()) && ( m_d->item != nullptr ) ){
            m_d->amountDataFieldLEdit.at(i)->setText( m_d->item->amountStr(i) );
        } else {
            m_d->amountDataFieldLEdit.at(i)->clear();
        }
    }
}

void BillItemGUI::updatePriceValue( int priceField, int priceCol, const QString & newValue){
    if( m_d->item ){
        if( m_d->item->currentPriceDataSet() == priceCol ){
            if( (priceField > -1) && priceField < m_d->priceDataFieldLEdit.size()){
                m_d->priceDataFieldLEdit.at(priceField)->setText( newValue);
            }
        }
    }
}

void BillItemGUI::updatePriceValues( int priceCol ){
    for( int i = 0; i < m_d->priceDataFieldLEdit.size(); ++i){
        if( (i < m_d->priceFieldModel->fieldCount()) && ( m_d->item != nullptr ) ){
            if( m_d->item->priceItem() != nullptr ){
                m_d->priceDataFieldLEdit.at(i)->setText( m_d->item->priceItem()->valueStr(i, priceCol) );
                continue;
            }
        }
        m_d->priceDataFieldLEdit.at(i)->clear();
    }
}

void BillItemGUI::updatePriceName( int priceField, const QString & newName ){
    if( priceField > -1 && priceField < m_d->priceDataFieldLabel.size() ){
        m_d->priceDataFieldLabel.at(priceField)->setText( newName );
    }
}

void BillItemGUI::updateAmountName( int priceField, const QString & newName ){
    if( priceField > -1 && priceField < m_d->amountDataFieldLabel.size() ){
        m_d->amountDataFieldLabel.at(priceField)->setText( newName );
    }
}
