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
#include "accountinglsbillgui.h"

#include "accountinglsbilldatagui.h"
#include "accountinglstreegui.h"
#include "accountinglsbillitemgui.h"
#include "accountinglsbillitemtitlegui.h"

#include "project.h"
#include "accountinglsbill.h"
#include "accountinglsbillitem.h"
#include "priceitem.h"
#include "unitmeasuremodel.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSplitter>
#include <QResizeEvent>

class AccountingItemWidget : public QWidget {
private:
    AccountingLSBillItemGUI * itemGUI;
    AccountingLSBillItemTitleGUI * itemTitleGUI;
public:
    AccountingItemWidget( AccountingLSBillItemGUI * _itemGUI,
                          AccountingLSBillItemTitleGUI * _titleGUI,
                          QWidget * parent = 0 ):
        QWidget(parent),
        itemGUI(_itemGUI),
        itemTitleGUI(_titleGUI){
        QGridLayout * layout = new QGridLayout( this );
        itemGUI->setParent( this );
        itemTitleGUI->setParent( this );
        layout->addWidget( itemGUI, 0, 0);
        layout->addWidget( itemTitleGUI, 0, 1);
    }
    QSize sizeHint() const{
        QSize s = itemGUI->sizeHint();
        return s.expandedTo( itemTitleGUI->sizeHint() );
    }
};

class AccountingLSBillGUIPrivate{
public:
    AccountingLSBillGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                QString * EPAFileName,
                                MathParser * prs, Project * prj,
                                QString * wpf, QWidget *parent ):
        item( NULL ),
        bill ( NULL ),
        project(prj),
        itemEditingPrice(NULL),
        accountingDataGUI( new AccountingLSBillDataGUI( prj->priceFieldModel(), prs, NULL, prj, wpf, parent ) ),
        mainSplitter( new QSplitter(Qt::Horizontal, parent ) ),
        treeGUI( new AccountingLSTreeGUI( EPAImpOptions, EPAFileName, prs, prj, mainSplitter ) ),
        itemGUI( new AccountingLSBillItemGUI( EPAImpOptions, EPAFileName, prs, prj, parent ) ),
        itemTitleGUI( new AccountingLSBillItemTitleGUI( prj->priceFieldModel(), parent ) ),
        itemWidget( new AccountingItemWidget(itemGUI, itemTitleGUI, mainSplitter ) ) {
        itemGUI->hide();
        itemTitleGUI->hide();
    }

    AccountingLSBillItem * item;
    AccountingLSBill * bill;
    Project * project;
    AccountingLSBillItem * itemEditingPrice;
    PriceItem * importingDataPriceItem;

    AccountingLSBillDataGUI * accountingDataGUI;
    QSplitter * mainSplitter;
    AccountingLSTreeGUI * treeGUI;
    AccountingLSBillItemGUI * itemGUI;
    AccountingLSBillItemTitleGUI * itemTitleGUI;
    AccountingItemWidget * itemWidget;
};

AccountingLSBillGUI::AccountingLSBillGUI(QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                         QString * EPAFileName,
                                         MathParser * prs, Project *p,
                                         QString * wordProcessorFile, QWidget *parent) :
    QTabWidget(parent),
    m_d( new AccountingLSBillGUIPrivate( EPAImpOptions, EPAFileName, prs, p, wordProcessorFile, this ) ){

    addTab( m_d->accountingDataGUI, trUtf8("Libretto Opere a Corpo - Dati generali"));
    addTab( m_d->mainSplitter, trUtf8("Libretto Opere a Corpo - Misure"));

    setCurrentIndex( 1 );

    connect( m_d->treeGUI, &AccountingLSTreeGUI::currentItemChanged, this, &AccountingLSBillGUI::setBillItem );
}

AccountingLSBillGUI::~AccountingLSBillGUI(){
    delete m_d;
}

void AccountingLSBillGUI::setBill( AccountingLSBill * b ){
    m_d->bill = b;
    m_d->accountingDataGUI->setAccountingBill( b );
    m_d->treeGUI->setBill( b );
    m_d->itemGUI->setBill( b );
    m_d->itemTitleGUI->setBill( b );
    setBillItem( m_d->treeGUI->currentItem() );
}

void AccountingLSBillGUI::setBillItem( AccountingLSBillItem *newItem ) {
    if( m_d->item != NULL ){
        disconnect( m_d->item, static_cast<void(AccountingLSBillItem::*)(bool)>(&AccountingLSBillItem::hasChildrenChanged), this, &AccountingLSBillGUI::updateItemGUI );
        disconnect( m_d->item, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingLSBillGUI::setBillItemNULL );
    }
    m_d->item = newItem;
    if( m_d->item != NULL ){
        connect( m_d->item, static_cast<void(AccountingLSBillItem::*)(bool)>(&AccountingLSBillItem::hasChildrenChanged), this, &AccountingLSBillGUI::updateItemGUI );
        connect( m_d->item, &AccountingLSBillItem::aboutToBeDeleted, this, &AccountingLSBillGUI::setBillItemNULL );
    }
    updateItemGUI();
}

void AccountingLSBillGUI::setBillItemNULL() {
    setBillItem( NULL );
}

void AccountingLSBillGUI::updateItemGUI() {
    if( m_d->item != NULL ){
        if( m_d->item->hasChildren() ){
            m_d->itemTitleGUI->setBillItem( m_d->item );
            m_d->itemTitleGUI->show();

            m_d->itemGUI->hide();
            m_d->itemGUI->clear();
        } else {
            m_d->itemGUI->setBillItem( m_d->item );
            m_d->itemGUI->show();

            m_d->itemTitleGUI->hide();
            m_d->itemTitleGUI->setBillItemNULL();
        }
        return;
    }

    m_d->itemGUI->hide();
    m_d->itemGUI->clear();
    m_d->itemGUI->setBillNULL();

    m_d->itemTitleGUI->hide();
    m_d->itemTitleGUI->setBillItemNULL();
    m_d->itemTitleGUI->setBillNULL();
}
