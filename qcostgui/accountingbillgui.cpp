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
#include "accountingbillgui.h"

#include "accountingbilldatagui.h"
#include "accountingtreegui.h"
#include "accountingitemppugui.h"
#include "accountingitempaymentgui.h"
#include "accountingitemlsgui.h"
#include "accountingitemtamgui.h"
#include "accountingitemcommentgui.h"

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
    AccountingItemPaymentGUI * billGUI;
    AccountingItemPPUGUI * ppuGUI;
    AccountingItemLSGUI * lsGUI;
    AccountingItemTAMGUI * tamGUI;
    AccountingItemCommentGUI * commentGUI;
public:
    AccountingItemWidget( AccountingItemPaymentGUI * _wipGUI,
                          AccountingItemPPUGUI * _ppuGUI,
                          AccountingItemLSGUI * _lsGUI,
                          AccountingItemTAMGUI * _tamGUI,
                          AccountingItemCommentGUI * _commentGUI,
                          QWidget * parent = 0 ):
        QWidget(parent),
        billGUI(_wipGUI),
        ppuGUI(_ppuGUI),
        lsGUI(_lsGUI),
        tamGUI(_tamGUI),
        commentGUI(_commentGUI){
        QGridLayout * layout = new QGridLayout( this );
        ppuGUI->setParent( this );
        billGUI->setParent( this );
        layout->addWidget( billGUI, 0, 0);
        layout->addWidget( ppuGUI, 0, 1);
        layout->addWidget( lsGUI, 0, 2);
        layout->addWidget( tamGUI, 0, 3);
        layout->addWidget( commentGUI, 0, 4);
    }
    QSize sizeHint() const{
        QSize s = ppuGUI->sizeHint();
        s = s.expandedTo( billGUI->sizeHint() );
        s = s.expandedTo( lsGUI->sizeHint() );
        s = s.expandedTo( tamGUI->sizeHint() );
        return s.expandedTo( commentGUI->sizeHint() );
    }
};

class AccountingBillGUIPrivate{
public:
    AccountingBillGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                              QString * EPAFileName,
                              MathParser * prs, AccountingBill * b, Project * prj,
                              QString * wpf, QWidget *parent ):
        accounting ( b ),
        currentItem( NULL ),
        project(prj),
        accountingItemEditingPrice(NULL),
        accountingDataGUI( new AccountingBillDataGUI( prj->priceFieldModel(), prs, NULL, prj, wpf, parent ) ),
        mainSplitter( new QSplitter(Qt::Horizontal, parent ) ),
        accountingTreeGUI( new AccountingTreeGUI( EPAImpOptions, EPAFileName, b, prs, prj, mainSplitter ) ),
        accountingItemBillGUI( new AccountingItemPaymentGUI( prj->priceFieldModel(), parent ) ),
        accountingItemPPUGUI( new AccountingItemPPUGUI( EPAImpOptions, EPAFileName, prs, prj, parent ) ),
        accountingItemLSGUI( new AccountingItemLSGUI( prj->accountingLSBills(), prj->priceFieldModel(), parent ) ),
        accountingItemTAMGUI( new AccountingItemTAMGUI( prj->accountingTAMBill(), prj->priceFieldModel(), parent ) ),
        accountingItemCommentGUI( new AccountingItemCommentGUI( parent ) ),
        accountingItemWidget( new AccountingItemWidget(accountingItemBillGUI, accountingItemPPUGUI, accountingItemLSGUI, accountingItemTAMGUI, accountingItemCommentGUI, mainSplitter ) ) {
        accountingItemBillGUI->hide();
        accountingItemPPUGUI->hide();
        accountingItemLSGUI->hide();
        accountingItemTAMGUI->hide();
        accountingItemCommentGUI->hide();
    }

    AccountingBill * accounting;
    AccountingBillItem * currentItem;
    Project * project;
    AccountingBillItem * accountingItemEditingPrice;
    PriceItem * importingDataPriceItem;

    AccountingBillDataGUI * accountingDataGUI;
    QSplitter * mainSplitter;
    AccountingTreeGUI * accountingTreeGUI;
    AccountingItemPaymentGUI * accountingItemBillGUI;
    AccountingItemPPUGUI * accountingItemPPUGUI;
    AccountingItemLSGUI * accountingItemLSGUI;
    AccountingItemTAMGUI * accountingItemTAMGUI;
    AccountingItemCommentGUI * accountingItemCommentGUI;
    AccountingItemWidget * accountingItemWidget;
};

AccountingBillGUI::AccountingBillGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                      QString * EPAFileName,
                                      MathParser * prs, AccountingBill * b, Project *p,
                                      QString * wordProcessorFile, QWidget *parent) :
    QTabWidget(parent),
    m_d( new AccountingBillGUIPrivate( EPAImpOptions, EPAFileName, prs, b, p, wordProcessorFile, this ) ){

    addTab( m_d->accountingDataGUI, trUtf8("Libretto delle Misure - Dati generali"));
    addTab( m_d->mainSplitter, trUtf8("Libretto delle Misure - Misure"));

    setCurrentIndex( 1 );
    setAccountingBill(b);

    connect( m_d->accountingTreeGUI, &AccountingTreeGUI::currentBillItemChanged, this, &AccountingBillGUI::setAccountingItem );
}

AccountingBillGUI::~AccountingBillGUI(){
    delete m_d;
}

void AccountingBillGUI::setAccountingBill( AccountingBill * b ){
    m_d->accounting = b;
    m_d->accountingDataGUI->setAccountingBill( b );
    m_d->accountingTreeGUI->setAccountingBill( b );
    m_d->accountingItemBillGUI->setAccountingBill( b );
    m_d->accountingItemPPUGUI->setAccountingBill( b );
    m_d->accountingItemLSGUI->setAccountingBill( b );
    m_d->accountingItemTAMGUI->setAccountingBill( b );
    setAccountingItem( m_d->accountingTreeGUI->currentAccountingBill() );
}

void AccountingBillGUI::setAccountingItem(AccountingBillItem * newItem ) {
    if( m_d->currentItem != NULL ){
        disconnect( m_d->currentItem, static_cast<void(AccountingBillItem::*)(bool)>(&AccountingBillItem::hasChildrenChanged), this, &AccountingBillGUI::updateAccountingMeasureGUI );
        disconnect( m_d->currentItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingBillGUI::setAccountingMeasureNULL );
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
    m_d->currentItem = newItem;
    if( m_d->currentItem != NULL ){
        updateAccountingMeasureGUI();
        connect( m_d->currentItem, static_cast<void(AccountingBillItem::*)(bool)>(&AccountingBillItem::hasChildrenChanged), this, &AccountingBillGUI::updateAccountingMeasureGUI );
        connect( m_d->currentItem, &AccountingBillItem::aboutToBeDeleted, this, &AccountingBillGUI::setAccountingMeasureNULL );
    }
}

void AccountingBillGUI::setAccountingMeasureNULL() {
    setAccountingItem( NULL );
}

void AccountingBillGUI::updateAccountingMeasureGUI() {
    if( m_d->currentItem != NULL ){
        if( m_d->currentItem->itemType() == AccountingBillItem::Payment ){
            m_d->accountingItemBillGUI->show();
            m_d->accountingItemBillGUI->setAccountingItem( m_d->currentItem );

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
        if( m_d->currentItem->itemType() == AccountingBillItem::PPU ){
            m_d->accountingItemBillGUI->hide();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->show();
            m_d->accountingItemPPUGUI->setItem( m_d->currentItem );

            m_d->accountingItemLSGUI->hide();
            m_d->accountingItemLSGUI->setAccountingItemNULL();

            m_d->accountingItemTAMGUI->hide();
            m_d->accountingItemTAMGUI->setAccountingItemNULL();

            m_d->accountingItemCommentGUI->hide();
            m_d->accountingItemCommentGUI->setAccountingItemNULL();

            return;
        }
        if(  m_d->currentItem->itemType() == AccountingBillItem::LumpSum ){
            m_d->accountingItemBillGUI->hide();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->hide();
            m_d->accountingItemPPUGUI->setAccountingItemNULL();

            m_d->accountingItemLSGUI->show();
            m_d->accountingItemLSGUI->setItem( m_d->currentItem );

            m_d->accountingItemTAMGUI->hide();
            m_d->accountingItemTAMGUI->setAccountingItemNULL();

            m_d->accountingItemCommentGUI->hide();
            m_d->accountingItemCommentGUI->setAccountingItemNULL();

            return;
        }
        if(  m_d->currentItem->itemType() == AccountingBillItem::TimeAndMaterials ){
            m_d->accountingItemBillGUI->hide();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->hide();
            m_d->accountingItemPPUGUI->setAccountingItemNULL();

            m_d->accountingItemLSGUI->hide();
            m_d->accountingItemLSGUI->setAccountingItemNULL();

            m_d->accountingItemTAMGUI->show();
            m_d->accountingItemTAMGUI->setAccountingItem( m_d->currentItem );

            m_d->accountingItemCommentGUI->hide();
            m_d->accountingItemCommentGUI->setAccountingItemNULL();

            return;
        }
        if(  m_d->currentItem->itemType() == AccountingBillItem::Comment ){
            m_d->accountingItemBillGUI->hide();
            m_d->accountingItemBillGUI->setAccountingItemNULL();

            m_d->accountingItemPPUGUI->hide();
            m_d->accountingItemPPUGUI->setAccountingItemNULL();

            m_d->accountingItemLSGUI->hide();
            m_d->accountingItemLSGUI->setAccountingItemNULL();

            m_d->accountingItemTAMGUI->hide();
            m_d->accountingItemTAMGUI->setAccountingItemNULL();

            m_d->accountingItemCommentGUI->show();
            m_d->accountingItemCommentGUI->setAccountingItem( m_d->currentItem );

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
