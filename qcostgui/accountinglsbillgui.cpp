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
#include "accountinglsbillgui.h"

#include "accountinglsbilldatagui.h"
#include "accountinglstreegui.h"
#include "accountinglsbillitemgui.h"
#include "accountinglsbillitemtitlegui.h"

#include "project.h"
#include "accountingbill.h"
#include "accountingbillitem.h"
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
                              MathParser * prs, AccountingLSBill * b, Project * prj,
                              QString * wpf, QWidget *parent ):
        accounting ( b ),
        currentAccountingMeasure( NULL ),
        project(prj),
        accountingItemEditingPrice(NULL),
        accountingDataGUI( new AccountingLSBillDataGUI( prj->priceFieldModel(), prs, NULL, prj, wpf, parent ) ),
        mainSplitter( new QSplitter(Qt::Horizontal, parent ) ),
        accountingTreeGUI( new AccountingLSTreeGUI( EPAImpOptions, EPAFileName, b, prs, prj, mainSplitter ) ),
        accountingLSBillItemGUI( new AccountingLSBillItemGUI( EPAImpOptions, EPAFileName, prs, prj, parent ) ),
        AccountingLSBillItemTitleGUI( new AccountingLSBillItemTitleGUI( EPAImpOptions, EPAFileName, prs, prj, parent ) ),
        accountingLSItemWidget( new AccountingItemWidget(accountingItemBillGUI, accountingItemPPUGUI, accountingItemLSGUI, accountingItemTAMGUI, accountingItemCommentGUI, mainSplitter ) ) {
        accountingItemBillGUI->hide();
        accountingItemPPUGUI->hide();
        accountingItemLSGUI->hide();
        accountingItemTAMGUI->hide();
        accountingItemCommentGUI->hide();
    }

    AccountingLSBill * accounting;
    AccountingLSBillItem * currentAccountingMeasure;
    Project * project;
    AccountingBillItem * accountingItemEditingPrice;
    PriceItem * importingDataPriceItem;

    AccountingLSBillDataGUI * accountingDataGUI;
    QSplitter * mainSplitter;
    AccountingLSTreeGUI * accountingTreeGUI;
    AccountingLSBillItemGUI * accountingBillItemGUI;
    AccountingLSBillItemTitleGUI * accountingBillItemTitleGUI;
    AccountingItemWidget * accountingLSItemWidget;
};

AccountingLSBillGUI::AccountingLSBillGUI(QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                      QString * EPAFileName,
                                      MathParser * prs, AccountingLSBill *b, Project *p,
                                      QString * wordProcessorFile, QWidget *parent) :
    QTabWidget(parent),
    m_d( new AccountingLSBillGUIPrivate( EPAImpOptions, EPAFileName, prs, b, p, wordProcessorFile, this ) ){

    addTab( m_d->accountingDataGUI, trUtf8("Libretto delle Misure - Dati generali"));
    addTab( m_d->mainSplitter, trUtf8("Libretto delle Misure - Misure"));

    setCurrentIndex( 1 );
    setAccountingBill(b);

    connect( m_d->accountingTreeGUI, &AccountingTreeGUI::currentBillItemChanged, this, &AccountingLSBillGUI::setAccountingItem );
}

AccountingLSBillGUI::~AccountingLSBillGUI(){
    delete m_d;
}

void AccountingLSBillGUI::setAccountingBill( AccountingLSBill * b ){
    m_d->accounting = b;
    m_d->accountingDataGUI->setAccountingBill( b );
    m_d->accountingTreeGUI->setAccountingBill( b );
    m_d->accountingItemBillGUI->setAccountingBill( b );
    m_d->accountingItemPPUGUI->setAccountingBill( b );
    m_d->accountingItemLSGUI->setAccountingBill( b );
    m_d->accountingItemTAMGUI->setAccountingBill( b );
    setAccountingItem( m_d->accountingTreeGUI->currentAccountingBill() );
}

void AccountingLSBillGUI::setAccountingItem(AccountingLSBillItem *newItem ) {
    if( m_d->currentAccountingMeasure != NULL ){
        disconnect( m_d->currentAccountingMeasure, static_cast<void(AccountingBillItem::*)(bool)>(&AccountingBillItem::hasChildrenChanged), this, &AccountingLSBillGUI::updateAccountingMeasureGUI );
        disconnect( m_d->currentAccountingMeasure, &AccountingBillItem::aboutToBeDeleted, this, &AccountingLSBillGUI::setAccountingMeasureNULL );
        m_d->accountingItemBillGUI->setAccountingItemNULL();
        m_d->accountingItemBillGUI->hide();
        m_d->accountingItemPPUGUI->setAccountingItemNULL();
        m_d->accountingItemPPUGUI->hide();
        m_d->accountingItemLSGUI->setAccountingItemNULL();
        m_d->accountingItemLSGUI->hide();
        m_d->accountingItemTAMGUI->setAccountingItemNULL();
        m_d->accountingItemTAMGUI->hide();
        m_d->accountingItemCommentGUI->setAccountingItemNULL();
        m_d->accountingItemCommentGUI->hide();
    }
    m_d->currentAccountingMeasure = newItem;
    if( m_d->currentAccountingMeasure != NULL ){
        updateAccountingMeasureGUI();
        connect( m_d->currentAccountingMeasure, static_cast<void(AccountingBillItem::*)(bool)>(&AccountingBillItem::hasChildrenChanged), this, &AccountingLSBillGUI::updateAccountingMeasureGUI );
        connect( m_d->currentAccountingMeasure, &AccountingBillItem::aboutToBeDeleted, this, &AccountingLSBillGUI::setAccountingMeasureNULL );
    }
}

void AccountingLSBillGUI::setAccountingMeasureNULL() {
    setAccountingItem( NULL );
}

void AccountingLSBillGUI::updateAccountingMeasureGUI() {
    if( m_d->currentAccountingMeasure != NULL ){
        if( m_d->currentAccountingMeasure->itemType() == AccountingBillItem::Bill ){
            m_d->accountingItemBillGUI->show();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->hide();
            m_d->accountingItemPPUGUI->setAccountingItemNULL();

            m_d->accountingItemLSGUI->hide();
            m_d->accountingItemLSGUI->setAccountingItemNULL();

            m_d->accountingItemTAMGUI->hide();
            m_d->accountingItemTAMGUI->setAccountingItemNULL();

            m_d->accountingItemCommentGUI->hide();
            m_d->accountingItemCommentGUI->setAccountingItemNULL();

            return;
        }
        if( m_d->currentAccountingMeasure->itemType() == AccountingBillItem::PPU ){
            m_d->accountingItemBillGUI->hide();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->show();
            m_d->accountingItemPPUGUI->setAccountingItemNULL();

            m_d->accountingItemLSGUI->hide();
            m_d->accountingItemLSGUI->setAccountingItemNULL();

            m_d->accountingItemTAMGUI->hide();
            m_d->accountingItemTAMGUI->setAccountingItemNULL();

            m_d->accountingItemCommentGUI->hide();
            m_d->accountingItemCommentGUI->setAccountingItemNULL();

            return;
        }
        if(  m_d->currentAccountingMeasure->itemType() == AccountingBillItem::LumpSum ){
            m_d->accountingItemBillGUI->hide();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->hide();
            m_d->accountingItemPPUGUI->setAccountingItemNULL();

            m_d->accountingItemLSGUI->show();
            m_d->accountingItemLSGUI->setAccountingItemNULL();

            m_d->accountingItemTAMGUI->hide();
            m_d->accountingItemTAMGUI->setAccountingItemNULL();

            m_d->accountingItemCommentGUI->hide();
            m_d->accountingItemCommentGUI->setAccountingItemNULL();

            return;
        }
        if(  m_d->currentAccountingMeasure->itemType() == AccountingBillItem::TimeAndMaterials ){
            m_d->accountingItemBillGUI->hide();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->hide();
            m_d->accountingItemPPUGUI->setAccountingItemNULL();

            m_d->accountingItemLSGUI->hide();
            m_d->accountingItemLSGUI->setAccountingItemNULL();

            m_d->accountingItemTAMGUI->show();
            m_d->accountingItemTAMGUI->setAccountingItemNULL();

            m_d->accountingItemCommentGUI->hide();
            m_d->accountingItemCommentGUI->setAccountingItemNULL();

            return;
        }
        if(  m_d->currentAccountingMeasure->itemType() == AccountingBillItem::Comment ){
            m_d->accountingItemBillGUI->hide();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->hide();
            m_d->accountingItemPPUGUI->setAccountingItemNULL();

            m_d->accountingItemLSGUI->hide();
            m_d->accountingItemLSGUI->setAccountingItemNULL();

            m_d->accountingItemTAMGUI->hide();
            m_d->accountingItemTAMGUI->setAccountingItemNULL();

            m_d->accountingItemCommentGUI->show();
            m_d->accountingItemCommentGUI->setAccountingItemNULL();

            return;
        }
    }
    m_d->accountingItemBillGUI->hide();
    m_d->accountingItemBillGUI->setAccountingItemNULL();

    m_d->accountingItemPPUGUI->hide();
    m_d->accountingItemPPUGUI->setAccountingItemNULL();

    m_d->accountingItemLSGUI->hide();
    m_d->accountingItemLSGUI->setAccountingItemNULL();

    m_d->accountingItemTAMGUI->hide();
    m_d->accountingItemTAMGUI->setAccountingItemNULL();

    m_d->accountingItemCommentGUI->hide();
    m_d->accountingItemCommentGUI->setAccountingItemNULL();
}
