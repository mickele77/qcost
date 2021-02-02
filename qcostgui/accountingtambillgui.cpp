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
#include "accountingtambillgui.h"

#include "accountingtambilldatagui.h"
#include "accountingtambillpricedatagui.h"
#include "attributesgui.h"
#include "accountingtreegui.h"
#include "accountingitemppugui.h"
#include "accountingitemcommentgui.h"
#include "accountingitempaymentgui.h"

#include "project.h"
#include "accountingtambill.h"
#include "accountingtambillitem.h"
#include "priceitem.h"
#include "unitmeasuremodel.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSplitter>
#include <QResizeEvent>

class AccountingItemWidget : public QWidget {
private:
    AccountingItemPPUGUI * ppuGUI;
    AccountingItemCommentGUI * commentGUI;
    AccountingItemPaymentGUI * billGUI;
public:
    AccountingItemWidget( AccountingItemPPUGUI * _ppuGUI,
                          AccountingItemCommentGUI * _commentGUI,
                          AccountingItemPaymentGUI * _billGUI,
                          QWidget * parent = nullptr ):
        QWidget(parent),
        ppuGUI(_ppuGUI),
        commentGUI(_commentGUI),
        billGUI(_billGUI){
        QGridLayout * layout = new QGridLayout( this );
        ppuGUI->setParent( this );
        layout->addWidget( ppuGUI, 0, 1);
        layout->addWidget( commentGUI, 0, 2);
        layout->addWidget( billGUI, 0, 3);
    }
    QSize sizeHint() const{
        QSize s = ppuGUI->sizeHint();
        return s.expandedTo( ppuGUI->sizeHint() );
    }
};

class AccountingTAMBillGUIPrivate{
public:
    AccountingTAMBillGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                 QString * EPAFileName,
                                 MathParser * prs,
                                 AccountingTAMBill * b,
                                 Project * prj,
                                 QString * wpf, QWidget *parent ):
        accountingTAMBill ( b ),
        currentAccountingTAMBillItem( nullptr ),
        project(prj),
        accountingItemEditingPrice( nullptr ),
        dataGUI( new AccountingTAMBillDataGUI( parent ) ),
        priceDataGUI( new AccountingTAMBillPriceDataGUI( prj->priceFieldModel(), nullptr, prj, parent ) ),
        attributesGUI( new AttributesGUI( prj->priceFieldModel(), nullptr, wpf, parent ) ),
        mainSplitter( new QSplitter(Qt::Horizontal, parent ) ),
        accountingTreeGUI( new AccountingTreeGUI( EPAImpOptions, EPAFileName, b, prs, prj, mainSplitter ) ),
        accountingTAMBillItemPPUGUI( new AccountingItemPPUGUI( EPAImpOptions, EPAFileName, prs, prj, parent ) ),
        accountingTAMBillItemCommentGUI( new AccountingItemCommentGUI( parent ) ),
        accountingTAMBillItemBillGUI( new AccountingItemPaymentGUI( prj->priceFieldModel(), parent ) ),
        accountingItemWidget( new AccountingItemWidget( accountingTAMBillItemPPUGUI, accountingTAMBillItemCommentGUI, accountingTAMBillItemBillGUI, mainSplitter ) ) {
        accountingTAMBillItemPPUGUI->hide();
        accountingTAMBillItemCommentGUI->hide();
        accountingTAMBillItemBillGUI->hide();
    }

    AccountingTAMBill * accountingTAMBill;
    AccountingTAMBillItem * currentAccountingTAMBillItem;
    Project * project;
    AccountingTAMBillItem * accountingItemEditingPrice;
    PriceItem * importingDataPriceItem;

    AccountingTAMBillDataGUI * dataGUI;
    AccountingTAMBillPriceDataGUI * priceDataGUI;
    AttributesGUI * attributesGUI;
    QSplitter * mainSplitter;
    AccountingTreeGUI * accountingTreeGUI;
    AccountingItemPPUGUI * accountingTAMBillItemPPUGUI;
    AccountingItemCommentGUI * accountingTAMBillItemCommentGUI;
    AccountingItemPaymentGUI * accountingTAMBillItemBillGUI;
    AccountingItemWidget * accountingItemWidget;
};

AccountingTAMBillGUI::AccountingTAMBillGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                            QString * EPAFileName,
                                            MathParser * prs, AccountingTAMBill * b, Project *p,
                                            QString * wordProcessorFile, QWidget *parent) :
    QTabWidget(parent),
    m_d( new AccountingTAMBillGUIPrivate( EPAImpOptions, EPAFileName, prs, b, p, wordProcessorFile, this ) ){

    addTab( m_d->dataGUI, tr("Importi complessivi"));
    addTab( m_d->priceDataGUI, tr("Prezzi"));
    addTab( m_d->attributesGUI, tr("Etichette"));
    addTab( m_d->mainSplitter, tr("Liste"));

    setCurrentIndex( count() - 1 );
    setBill(b);

    connect( m_d->accountingTreeGUI, &AccountingTreeGUI::currentTAMBillItemChanged, this, &AccountingTAMBillGUI::setBillItem );
}

AccountingTAMBillGUI::~AccountingTAMBillGUI(){
    delete m_d;
}

void AccountingTAMBillGUI::setBill( AccountingTAMBill * b ){
    if( m_d->accountingTAMBill != b ){
        m_d->accountingTAMBill = b;
        m_d->dataGUI->setAccountingTAMBill( b );
        m_d->priceDataGUI->setAccountingTAMBill( b );
        m_d->attributesGUI->setBill( b );
        m_d->accountingTreeGUI->setAccountingTAMBill( b );
        m_d->accountingTAMBillItemPPUGUI->setAccountingTAMBill( b );
        m_d->accountingTAMBillItemBillGUI->setAccountingTAMBill( b );
    }
}

void AccountingTAMBillGUI::setBillItem(AccountingTAMBillItem * newItem ) {
    if( m_d->currentAccountingTAMBillItem != nullptr ){
        disconnect( m_d->currentAccountingTAMBillItem, static_cast<void(AccountingTAMBillItem::*)(bool)>(&AccountingTAMBillItem::hasChildrenChanged), this, &AccountingTAMBillGUI::updateGUI );
        disconnect( m_d->currentAccountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingTAMBillGUI::setBillItemnullptr );
        m_d->accountingTAMBillItemPPUGUI->setItem( (AccountingTAMBillItem *)(nullptr) );
        m_d->accountingTAMBillItemPPUGUI->hide();
        m_d->accountingTAMBillItemCommentGUI->setAccountingItemnullptr();
        m_d->accountingTAMBillItemCommentGUI->hide();
        m_d->accountingTAMBillItemBillGUI->setAccountingItemnullptr();
        m_d->accountingTAMBillItemBillGUI->hide();
    }
    m_d->currentAccountingTAMBillItem = newItem;
    if( m_d->currentAccountingTAMBillItem != nullptr ){
        updateGUI();
        connect( m_d->currentAccountingTAMBillItem, static_cast<void(AccountingTAMBillItem::*)(bool)>(&AccountingTAMBillItem::hasChildrenChanged), this, &AccountingTAMBillGUI::updateGUI );
        connect( m_d->currentAccountingTAMBillItem, &AccountingTAMBillItem::aboutToBeDeleted, this, &AccountingTAMBillGUI::setBillItemnullptr );
    }
}

void AccountingTAMBillGUI::setBillItemnullptr() {
    setBillItem( nullptr );
}

void AccountingTAMBillGUI::updateGUI() {
    if( m_d->currentAccountingTAMBillItem != nullptr ){

        if( m_d->currentAccountingTAMBillItem->itemType() == AccountingTAMBillItem::PPU ){
            m_d->accountingTAMBillItemPPUGUI->show();
            m_d->accountingTAMBillItemPPUGUI->setItem( m_d->currentAccountingTAMBillItem );

            m_d->accountingTAMBillItemCommentGUI->hide();
            m_d->accountingTAMBillItemCommentGUI->setAccountingItemnullptr();

            m_d->accountingTAMBillItemBillGUI->hide();
            m_d->accountingTAMBillItemBillGUI->setAccountingItemnullptr();

            return;
        }

        if( m_d->currentAccountingTAMBillItem->itemType() == AccountingTAMBillItem::Comment ){
            m_d->accountingTAMBillItemPPUGUI->hide();
            m_d->accountingTAMBillItemPPUGUI->setAccountingItemnullptr();

            m_d->accountingTAMBillItemCommentGUI->show();
            m_d->accountingTAMBillItemCommentGUI->setAccountingItem( m_d->currentAccountingTAMBillItem );

            m_d->accountingTAMBillItemBillGUI->hide();
            m_d->accountingTAMBillItemBillGUI->setAccountingItemnullptr();

            return;
        }

        if( m_d->currentAccountingTAMBillItem->itemType() == AccountingTAMBillItem::Payment ){
            m_d->accountingTAMBillItemPPUGUI->hide();
            m_d->accountingTAMBillItemPPUGUI->setAccountingItemnullptr();

            m_d->accountingTAMBillItemCommentGUI->hide();
            m_d->accountingTAMBillItemCommentGUI->setAccountingItemnullptr();

            m_d->accountingTAMBillItemBillGUI->show();
            m_d->accountingTAMBillItemBillGUI->setAccountingItem( m_d->currentAccountingTAMBillItem );

            return;
        }
    }

    m_d->accountingTAMBillItemPPUGUI->hide();
    m_d->accountingTAMBillItemPPUGUI->setAccountingItemnullptr();

    m_d->accountingTAMBillItemCommentGUI->hide();
    m_d->accountingTAMBillItemCommentGUI->setAccountingItemnullptr();

    m_d->accountingTAMBillItemBillGUI->hide();
    m_d->accountingTAMBillItemBillGUI->setAccountingItemnullptr();
}
