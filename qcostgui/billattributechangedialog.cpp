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
#include "billattributechangedialog.h"
#include "ui_billattributechangedialog.h"

#include "billitem.h"
#include "billattributeselectmodel.h"
#include "billattributemodel.h"
#include "billattribute.h"

class BillAttributeChangeDialogPrivate{
public:
    BillAttributeChangeDialogPrivate( QList<BillItem *> * il,
                                      BillAttributeModel * bam ):
        ui( new Ui::BillAttributeChangeDialog ),
        itemsList( il ),
        billAttModel(bam),
        billSelectModel( new BillAttributeSelectModel( bam ) ){
    };
    ~BillAttributeChangeDialogPrivate(){
        delete ui;
        delete billSelectModel;
    };
    Ui::BillAttributeChangeDialog *ui;
    QList<BillItem *> * itemsList;
    BillAttributeModel * billAttModel;
    BillAttributeSelectModel * billSelectModel;
};

BillAttributeChangeDialog::BillAttributeChangeDialog( QList<BillItem *> * itemsList,
                                                      BillAttributeModel * billAttModel,
                                                      QWidget *parent) :
    QDialog(parent),
    m_d( new BillAttributeChangeDialogPrivate( itemsList, billAttModel ) ) {
    m_d->ui->setupUi(this);
    m_d->ui->billAttributeTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    m_d->ui->billAttributeTableView->setModel( m_d->billSelectModel );

    connect( m_d->ui->cancelPushButton, &QPushButton::clicked, this, &BillAttributeChangeDialog::reject );
    connect( m_d->ui->addAttributesPushButton, &QPushButton::clicked, this, &BillAttributeChangeDialog::addAttributes );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &BillAttributeChangeDialog::removeAttributes );
    connect( m_d->ui->setAttributesPushButton, &QPushButton::clicked, this, &BillAttributeChangeDialog::setAttributes );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

BillAttributeChangeDialog::~BillAttributeChangeDialog() {
    delete m_d;
}

void BillAttributeChangeDialog::addAttributes(){
    if( m_d->ui->billAttributeTableView->selectionModel() ){
        QList<BillAttribute *> selectedAttributes = m_d->billSelectModel->selectedAttributes();
        for( QList<BillItem *>::iterator i = m_d->itemsList->begin(); i != m_d->itemsList->end(); ++i){
            for( QList<BillAttribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                (*i)->addAttribute( *j );
            }
        }
    }
    accept();
}

void BillAttributeChangeDialog::removeAttributes(){
    if( m_d->ui->billAttributeTableView->selectionModel() ){
        QList<BillAttribute *> selectedAttributes = m_d->billSelectModel->selectedAttributes();
        for( QList<BillItem *>::iterator i = m_d->itemsList->begin(); i != m_d->itemsList->end(); ++i){
            for( QList<BillAttribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                (*i)->removeAttribute( *j );
            }
        }
    }
    accept();
}

void BillAttributeChangeDialog::setAttributes(){
    if( m_d->ui->billAttributeTableView->selectionModel() ){
        QList<BillAttribute *> selectedAttributes = m_d->billSelectModel->selectedAttributes();
        for( QList<BillItem *>::iterator i = m_d->itemsList->begin(); i != m_d->itemsList->end(); ++i){
            (*i)->removeAllAttributes();
            for( QList<BillAttribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                (*i)->addAttribute( *j );
            }
        }
    }
    accept();
}
