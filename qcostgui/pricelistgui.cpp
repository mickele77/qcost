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
#include "pricelistgui.h"

#include "pricelisttreegui.h"
#include "priceitemgui.h"
#include "pricelistdatagui.h"

#include "project.h"
#include "bill.h"
#include "priceitem.h"
#include "pricelist.h"
#include "unitmeasuremodel.h"

#include <QSplitter>
#include <QGridLayout>

class PriceItemWidget : public QWidget {
private:
    PriceItemGUI * itemGUI;
public:
    PriceItemWidget( PriceItemGUI * itGUI, QWidget * parent = 0 ):
        QWidget(parent),
        itemGUI(itGUI) {
        QGridLayout * layout = new QGridLayout( this );
        itemGUI->setParent( this );
        layout->addWidget( itemGUI, 0,0 );
        itemGUI->hide();
    }
    QSize sizeHint() const{
        return itemGUI->sizeHint();
    }
};

class PriceListGUIPrivate{
public:
    PriceListGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions, QString * fileName, PriceList * pl, MathParser * prs, Project * prj, QWidget *parent):
        project( prj ),
        priceList( pl ),
        currentPriceItem(NULL),
        dataGUI( new PriceListDataGUI( NULL, parent ) ),
        priceItemsSplitter( new QSplitter( parent ) ),
        treeGUI( new PriceListTreeGUI( EPAImpOptions, fileName, NULL, 0, prs, prj->priceFieldModel(), prj->unitMeasureModel(), priceItemsSplitter )),
        itemGUI( new PriceItemGUI( EPAImpOptions, fileName, NULL, 0, prs, prj, parent )),
        itemWidget( new PriceItemWidget(itemGUI, priceItemsSplitter ) ),
        editingPriceListAP(NULL),
        importingDataPriceItem(NULL){
        priceItemsSplitter->addWidget( treeGUI );
        priceItemsSplitter->addWidget( itemWidget );
        priceItemsSplitter->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding);
    };
    Project * project;
    PriceList * priceList;
    PriceItem * currentPriceItem;
    PriceListDataGUI * dataGUI;
    QSplitter * priceItemsSplitter;
    PriceListTreeGUI * treeGUI;
    PriceItemGUI * itemGUI;
    PriceItemWidget * itemWidget;
    Bill * editingPriceListAP;
    PriceItem * importingDataPriceItem;
};

PriceListGUI::PriceListGUI( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions, QString * EPAFileName,
                            PriceList * pl, MathParser * prs, Project * prj,
                            QWidget *parent ) :
    QTabWidget(parent),
    m_d(new PriceListGUIPrivate( EPAImpOptions, EPAFileName, pl, prs, prj, this ) ){

    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding);

    addTab( m_d->dataGUI, trUtf8("Prezzi - Dati generali") );
    addTab( m_d->priceItemsSplitter, trUtf8("Prezzi - Elenco") );

    setCurrentIndex( 1 );

    setPriceList( pl );

    connect( m_d->treeGUI, &PriceListTreeGUI::currentItemChanged, this, &PriceListGUI::setPriceItem );
    connect( m_d->treeGUI, &PriceListTreeGUI::currentPriceDataSetChanged, m_d->itemGUI, static_cast< void (PriceItemGUI::*)(int) > (&PriceItemGUI::setCurrentPriceDataSet) );
    connect( m_d->itemGUI, &PriceItemGUI::currentPriceDataSetChanged, m_d->treeGUI, static_cast< void (PriceListTreeGUI::*)(int) > (&PriceListTreeGUI::setCurrentPriceDataSet) );
}

PriceListGUI::~PriceListGUI(){
    delete m_d;
}

void PriceListGUI::setPriceList(PriceList *pl) {
    m_d->priceList = pl;
    m_d->dataGUI->setPriceList( pl );
    m_d->treeGUI->setPriceList( pl );
    // quando si cambia l'elenco prezzi la scheda della voce si azzera
    setPriceItem( m_d->treeGUI->currentPriceItem() );
}

void PriceListGUI::setPriceItem(PriceItem * newItem ) {
    if( m_d->currentPriceItem != NULL ){
        disconnect( m_d->currentPriceItem, &PriceItem::aboutToBeDeleted, this, &PriceListGUI::setPriceItemNULL );
        m_d->itemGUI->setPriceItem( NULL );
        m_d->itemGUI->hide();
    }
    m_d->currentPriceItem = newItem;
    if( m_d->currentPriceItem != NULL ){
        updatePriceItemGUI();
        connect( m_d->currentPriceItem, &PriceItem::aboutToBeDeleted, this, &PriceListGUI::setPriceItemNULL );
    }
}

void PriceListGUI::setPriceItemNULL() {
    setPriceItem( NULL );
}

void PriceListGUI::updatePriceItemGUI() {
    if( m_d->currentPriceItem != NULL ){
        m_d->itemGUI->show();
        m_d->itemGUI->setPriceItem( m_d->currentPriceItem );
    } else {
        m_d->itemGUI->hide();
    }
    m_d->itemGUI->setPriceItem( m_d->currentPriceItem );
}
