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
#include "varsgui.h"

#include "ui_varsgui.h"

#include "bill.h"
#include "accountingbill.h"
#include "varsmodel.h"

#include <QMenu>

class VarsGUIPrivate{
public:
    VarsGUIPrivate():
        ui(new Ui::VarsGUI() ),
        bill(nullptr),
        accountingBill(nullptr){
    }
    Ui::VarsGUI * ui;
    Bill * bill;
    AccountingBill * accountingBill;
};

VarsGUI::VarsGUI( Bill * b, QWidget *parent) :
    QWidget(parent),
    m_d( new VarsGUIPrivate() ) {
    init();
    setBill(b);
}

VarsGUI::VarsGUI( AccountingBill * b, QWidget *parent) :
    QWidget(parent),
    m_d( new VarsGUIPrivate() ) {
    init();
    setBill(b);
}

void VarsGUI::init() {
    m_d->ui->setupUi( this );

    connect( m_d->ui->addVarPushButton, &QPushButton::clicked, this, &VarsGUI::addVar );
    connect( m_d->ui->removeVarPushButton, &QPushButton::clicked, this, &VarsGUI::removeVar );

    m_d->ui->varsTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->varsTableView, &QTableView::customContextMenuRequested, this, &VarsGUI::varsTableViewCustomMenuRequested );
}

VarsGUI::~VarsGUI(){
    delete m_d;
}

void VarsGUI::setBill(Bill *b) {
    if( m_d->bill != b || m_d->accountingBill != nullptr ){
        if( m_d->bill != nullptr ){
            disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &VarsGUI::setBillnullptr );
            m_d->bill = nullptr;
        }
        if( m_d->accountingBill != nullptr ){
            disconnect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &VarsGUI::setBillnullptr );
            m_d->accountingBill = nullptr;
        }
        m_d->ui->varsTableView->setModel( nullptr );

        m_d->bill = b;

        if( m_d->bill != nullptr ){
            m_d->ui->varsTableView->setModel( m_d->bill->varsModel() );
            connect( m_d->bill, &Bill::aboutToBeDeleted, this, &VarsGUI::setBillnullptr );
        }
    }
}

void VarsGUI::setBill(AccountingBill *b) {
    if( m_d->accountingBill != b || m_d->bill != nullptr ){
        if( m_d->bill != nullptr ){
            disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &VarsGUI::setBillnullptr );
            m_d->bill = nullptr;
        }
        if( m_d->accountingBill != nullptr ){
            disconnect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &VarsGUI::setBillnullptr );
            m_d->accountingBill = nullptr;
        }
        m_d->ui->varsTableView->setModel( nullptr );

        m_d->accountingBill = b;

        if( m_d->accountingBill != nullptr ){
            m_d->ui->varsTableView->setModel( m_d->accountingBill->varsModel() );
            connect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &VarsGUI::setBillnullptr );
        }
    }
}

void VarsGUI::setBillnullptr(){
    if( m_d->bill != nullptr ){
        disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &VarsGUI::setBillnullptr );
        m_d->bill = nullptr;
    }
    if( m_d->accountingBill != nullptr ){
        disconnect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &VarsGUI::setBillnullptr );
        m_d->accountingBill = nullptr;
    }
}

void VarsGUI::addVar(){
    if( m_d->bill != nullptr ){
        if( m_d->ui->varsTableView->selectionModel() ){
            QModelIndexList selectedIndexes = m_d->ui->varsTableView->selectionModel()->selectedIndexes();
            int count = 1;
            int row = 0;
            if( selectedIndexes.size() > 0 ){
                QList<int> selectedRows;
                for( QModelIndexList::iterator i = selectedIndexes.begin(); i != selectedIndexes.end(); ++ i ){
                    if( !selectedRows.contains((*i).row()) ){
                        selectedRows.append( (*i).row() );
                    }
                }
                qSort( selectedRows );
                row = selectedRows.last() + 1;
                count = selectedRows.size();
            }
            m_d->bill->varsModel()->insertRows( row, count );
        }
    }
}

void VarsGUI::removeVar(){
    if( m_d->bill != nullptr ){
        if( m_d->ui->varsTableView->selectionModel() ){
            QModelIndexList selectedRows = m_d->ui->varsTableView->selectionModel()->selectedRows();
            int count = selectedRows.size();
            if( count > 0 ){
                int row = selectedRows.at(0).row();
                for( int i=1; i<selectedRows.size(); ++i){
                    if( selectedRows.at(i).row() < row ){
                        row = selectedRows.at(i).row();
                    }
                }
                m_d->bill->varsModel()->removeRows( row, count );
            }
        }
    }
}

void VarsGUI::resizeVarColsToContents(){
    m_d->ui->varsTableView->resizeColumnsToContents();
}

void VarsGUI::varsTableViewCustomMenuRequested(QPoint pos){
    QMenu *menu=new QMenu(this);
    QAction * addAction = new QAction( tr("Aggiungi"), this);
    connect( addAction, &QAction::triggered, this, &VarsGUI::addVar );
    menu->addAction( addAction );
    QAction * delAction = new QAction( tr("Elimina"), this);
    connect( delAction, &QAction::triggered, this, &VarsGUI::removeVar );
    menu->addAction( delAction );
    menu->addSeparator();

    menu->addSeparator();
    QAction * resizeColToContentAction = new QAction( tr("Ottimizza colonne"), this);
    connect( resizeColToContentAction, &QAction::triggered, this, &VarsGUI::resizeVarsColToContents );
    menu->addAction( resizeColToContentAction );

    menu->popup( m_d->ui->varsTableView->viewport()->mapToGlobal(pos) );
}

void VarsGUI::resizeVarsColToContents(){
    if( m_d->ui->varsTableView->model() ){
        for( int i=0; i < m_d->ui->varsTableView->model()->columnCount(); ++i ){
            m_d->ui->varsTableView->resizeColumnToContents(i);
        }
    }
}
