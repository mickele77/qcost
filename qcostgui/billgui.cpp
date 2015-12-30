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
#include "billgui.h"

#include "billdatagui.h"
#include "attributesgui.h"
#include "varsgui.h"
#include "billtreegui.h"
#include "billitemgui.h"
#include "billitemtitlegui.h"

#include "project.h"
#include "bill.h"
#include "billitem.h"
#include "priceitem.h"
#include "unitmeasuremodel.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSplitter>
#include <QResizeEvent>

class BillItemWidget : public QWidget {
private:
    BillItemGUI * itemGUI;
    BillItemTitleGUI * itemTitleGUI;
public:
    BillItemWidget( BillItemGUI * itGUI, BillItemTitleGUI * ittGUI, QWidget * parent = 0 ):
        QWidget(parent),
        itemGUI(itGUI),
        itemTitleGUI(ittGUI){
        QGridLayout * layout = new QGridLayout( this );
        itemGUI->setParent( this );
        itemTitleGUI->setParent( this );
        layout->addWidget( itemGUI, 0,0 );
        layout->addWidget( itemTitleGUI, 0, 1);
    }
    QSize sizeHint() const{
        QSize s = itemGUI->sizeHint();
        return s.expandedTo( itemTitleGUI->sizeHint() );
    }
};

class BillGUIPrivate{
public:
    BillGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                    QString * EPAFileName,
                    MathParser * prs, Bill * b, Project * prj,
                    QString * wpf, QWidget *parent ):
        bill ( b ),
        currentBillItem( NULL ),
        project(prj),
        billItemEditingPrice(NULL),
        billDataGUI( new BillDataGUI( prj->priceFieldModel(), NULL, parent ) ),
        billAttributesGUI( new AttributesGUI( prj->priceFieldModel(), prs, (Bill *)(NULL), wpf, parent ) ),
        billVarsGUI( new VarsGUI( (Bill *) NULL, parent ) ),
        mainSplitter( new QSplitter(Qt::Horizontal, parent ) ),
        billTreeGUI( new BillTreeGUI( EPAImpOptions, EPAFileName, NULL, prs, prj, mainSplitter ) ),
        billItemGUI( new BillItemGUI( EPAImpOptions, EPAFileName, NULL, prs, prj, parent ) ),
        billItemTitleGUI( new BillItemTitleGUI( NULL, prj->priceFieldModel(), parent ) ),
        billItemWidget( new BillItemWidget(billItemGUI, billItemTitleGUI, mainSplitter ) ) {
        billItemGUI->hide();
        billItemTitleGUI->hide();
    }

    Bill * bill;
    BillItem * currentBillItem;
    Project * project;
    BillItem * billItemEditingPrice;
    PriceItem * importingDataPriceItem;

    BillDataGUI * billDataGUI;
    AttributesGUI * billAttributesGUI;
    VarsGUI * billVarsGUI;
    QSplitter * mainSplitter;
    BillTreeGUI * billTreeGUI;
    BillItemGUI * billItemGUI;
    BillItemTitleGUI * billItemTitleGUI;
    BillItemWidget * billItemWidget;
};

BillGUI::BillGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                  QString * EPAFileName,
                  MathParser * prs, Bill * b, Project *p,
                  QString * wordProcessorFile, QWidget *parent) :
    QTabWidget(parent),
    m_d( new BillGUIPrivate( EPAImpOptions, EPAFileName, prs, b, p, wordProcessorFile, this ) ){

    addTab( m_d->billDataGUI, trUtf8("Dati generali"));
    addTab( m_d->billAttributesGUI, trUtf8("Etichette"));
    addTab( m_d->billVarsGUI, trUtf8("Variabili"));
    addTab( m_d->mainSplitter, trUtf8("Misure"));

    setCurrentIndex( count() - 1 );
    setBill(b);

    connect( m_d->billTreeGUI, &BillTreeGUI::currentItemChanged, this, &BillGUI::setBillItem );
}

BillGUI::~BillGUI(){
    delete m_d;
}

void BillGUI::setBill( Bill * b ){
    m_d->bill = b;
    m_d->billDataGUI->setBill( b );
    m_d->billAttributesGUI->setBill( b );
    m_d->billVarsGUI->setBill( b );
    m_d->billTreeGUI->setBill( b );
    m_d->billItemGUI->setBill( b );
    m_d->billItemTitleGUI->setBill( b );
    setBillItem( m_d->billTreeGUI->currentBillItem() );
}

void BillGUI::setBillItem(BillItem * newItem ) {
    if( m_d->currentBillItem != NULL ){
        disconnect( m_d->currentBillItem, static_cast<void(BillItem::*)(bool)>(&BillItem::hasChildrenChanged), this, &BillGUI::updateBillItemGUI );
        disconnect( m_d->currentBillItem, &BillItem::aboutToBeDeleted, this, &BillGUI::setBillItemNULL );
        m_d->billItemGUI->setBillItem( NULL );
        m_d->billItemGUI->hide();
        m_d->billItemTitleGUI->setBillItem( NULL);
        m_d->billItemTitleGUI->hide();
    }
    m_d->currentBillItem = newItem;
    if( m_d->currentBillItem != NULL ){
        updateBillItemGUI();
        connect( m_d->currentBillItem, static_cast<void(BillItem::*)(bool)>(&BillItem::hasChildrenChanged), this, &BillGUI::updateBillItemGUI );
        connect( m_d->currentBillItem, &BillItem::aboutToBeDeleted, this, &BillGUI::setBillItemNULL );
    }
}

void BillGUI::setBillItemNULL() {
    setBillItem( NULL );
}

void BillGUI::updateBillItemGUI() {
    if( m_d->currentBillItem != NULL ){
        if( m_d->currentBillItem->hasChildren() ){
            m_d->billItemGUI->hide();
            m_d->billItemGUI->setBillItem( NULL );

            m_d->billItemTitleGUI->setBillItem( m_d->currentBillItem );
            m_d->billItemTitleGUI->show();
        } else {
            m_d->billItemGUI->setBillItem( m_d->currentBillItem );
            m_d->billItemGUI->show();

            m_d->billItemTitleGUI->hide();
            m_d->billItemTitleGUI->setBillItem( NULL );
        }
    } else {
        m_d->billItemGUI->hide();
        m_d->billItemGUI->setBillItem( NULL );

        m_d->billItemTitleGUI->hide();
        m_d->billItemTitleGUI->setBillItem( NULL );
    }
}
