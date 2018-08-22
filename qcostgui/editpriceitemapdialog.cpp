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
#include "editpriceitemapdialog.h"

#include "billtreegui.h"
#include "billitemgui.h"
#include "billitemtitlegui.h"

#include "project.h"
#include "bill.h"
#include "billitem.h"
#include "priceitem.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QSplitter>

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

class EditPriceItemAPDialogPrivate{
public:
    EditPriceItemAPDialogPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                  QString * EPAFileName,
                                  PriceItem * pItem,
                                  int pCol,
                                  Project * prj,
                                  MathParser * prs,
                                  QWidget *parent ):
        priceItem( pItem ),
        priceCol( pCol ),
        editingBill( * (pItem->associatedAP(pCol)) ),
        currentBillItem(NULL),
        mainLayout( new QGridLayout( parent ) ),
        mainSplitter( new QSplitter(Qt::Horizontal, parent ) ),
        billTreeGUI( new BillTreeGUI( true, EPAImpOptions, EPAFileName, NULL, prs, prj, mainSplitter ) ),
        billItemGUI( new BillItemGUI( EPAImpOptions, EPAFileName, NULL, prs, prj, mainSplitter ) ),
        billItemTitleGUI( new BillItemTitleGUI( NULL, prj->priceFieldModel(), parent ) ),
        billItemWidget( new BillItemWidget(billItemGUI, billItemTitleGUI, mainSplitter ) ),
        buttonBox( new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) ) {
        billItemGUI->hide();
        billItemTitleGUI->hide();
        mainLayout->addWidget( mainSplitter, 0, 0);
        mainLayout->addWidget( buttonBox, 1, 0);
    }

    PriceItem * priceItem;
    int priceCol;
    Bill editingBill;
    BillItem * currentBillItem;

    QGridLayout * mainLayout;
    QSplitter * mainSplitter;
    BillTreeGUI * billTreeGUI;
    BillItemGUI * billItemGUI;
    BillItemTitleGUI * billItemTitleGUI;
    BillItemWidget * billItemWidget;
    QDialogButtonBox * buttonBox;
};

EditPriceItemAPDialog::EditPriceItemAPDialog( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                              QString * EPAFileName,
                                              PriceItem * pItem,
                                              int priceCol,
                                              Project * prj,
                                              MathParser * prs,
                                              QWidget *parent) :
    QDialog(parent),
    m_d( new EditPriceItemAPDialogPrivate( EPAImpOptions, EPAFileName, pItem, priceCol, prj, prs, this ) ){

    m_d->billTreeGUI->setBill( &(m_d->editingBill) );

    connect( m_d->billTreeGUI, &BillTreeGUI::currentItemChanged, this, &EditPriceItemAPDialog::setBillItem );

    connect( m_d->buttonBox, &QDialogButtonBox::accepted, this, &EditPriceItemAPDialog::setPriceItemAP );
    connect( m_d->buttonBox, &QDialogButtonBox::rejected, this, &EditPriceItemAPDialog::reject );

    setWindowTitle( trUtf8("Analisi prezzi - %1").arg(pItem->codeFull()) );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

EditPriceItemAPDialog::~EditPriceItemAPDialog(){
    delete m_d;
}

void EditPriceItemAPDialog::setPriceItemAP(){
    *(m_d->priceItem->associatedAP( m_d->priceCol )) = m_d->editingBill;
    accept();
}

void EditPriceItemAPDialog::setBillItem(BillItem * newItem ) {
    if( m_d->currentBillItem != NULL ){
        disconnect( m_d->currentBillItem, static_cast<void(BillItem::*)(bool)>(&BillItem::hasChildrenChanged), this, &EditPriceItemAPDialog::updateBillItemGUI );
        disconnect( m_d->currentBillItem, &BillItem::aboutToBeDeleted, this, &EditPriceItemAPDialog::setBillItemNULL );
        m_d->billItemGUI->setBillItem( NULL );
        m_d->billItemGUI->hide();
        m_d->billItemTitleGUI->setBillItem( NULL);
        m_d->billItemTitleGUI->hide();
    }
    m_d->currentBillItem = newItem;
    if( m_d->currentBillItem != NULL ){
        updateBillItemGUI();
        connect( m_d->currentBillItem, static_cast<void(BillItem::*)(bool)>(&BillItem::hasChildrenChanged), this, &EditPriceItemAPDialog::updateBillItemGUI );
        connect( m_d->currentBillItem, &BillItem::aboutToBeDeleted, this, &EditPriceItemAPDialog::setBillItemNULL );
    }
}

void EditPriceItemAPDialog::setBillItemNULL() {
    setBillItem( NULL );
}

void EditPriceItemAPDialog::updateBillItemGUI() {
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
