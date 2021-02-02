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
#include "attributechangedialog.h"
#include "ui_attributechangedialog.h"

#include "accountingtambillitem.h"
#include "accountingbillitem.h"
#include "billitem.h"
#include "attributeselectmodel.h"
#include "attributesmodel.h"
#include "attribute.h"

class AttributeChangeDialogPrivate{
public:
    AttributeChangeDialogPrivate( QList<BillItem *> * il,
                                  AttributesModel * bam ):
        ui( new Ui::AttributeChangeDialog ),
        billItemsList(il),
        accountingBillItemsList( nullptr ),
        accountingTAMBillItemsList( nullptr ),
        accountingAttModel(bam),
        selectModel( new AttributeSelectModel( bam ) ){
    }
    AttributeChangeDialogPrivate( QList<AccountingBillItem *> * il,
                                  AttributesModel * bam ):
        ui( new Ui::AttributeChangeDialog ),
        billItemsList(nullptr),
        accountingBillItemsList( il ),
        accountingTAMBillItemsList( nullptr ),
        accountingAttModel(bam),
        selectModel( new AttributeSelectModel( bam ) ){
    }
    AttributeChangeDialogPrivate( QList<AccountingTAMBillItem *> * il,
                                  AttributesModel * bam ):
        ui( new Ui::AttributeChangeDialog ),
        billItemsList(nullptr),
        accountingBillItemsList( nullptr ),
        accountingTAMBillItemsList( il ),
        accountingAttModel(bam),
        selectModel( new AttributeSelectModel( bam ) ){
    }
    ~AttributeChangeDialogPrivate(){
        delete ui;
        delete selectModel;
    }
    Ui::AttributeChangeDialog *ui;
    QList<BillItem *> * billItemsList;
    QList<AccountingBillItem *> * accountingBillItemsList;
    QList<AccountingTAMBillItem *> * accountingTAMBillItemsList;
    AttributesModel * accountingAttModel;
    AttributeSelectModel * selectModel;
};

AttributeChangeDialog::AttributeChangeDialog(QList<BillItem *> * itemsList,
                                             AttributesModel *accountingAttModel,
                                             QWidget *parent) :
    QDialog(parent),
    m_d( new AttributeChangeDialogPrivate( itemsList, accountingAttModel ) ) {
    setup();
}

AttributeChangeDialog::AttributeChangeDialog(QList<AccountingBillItem *> * itemsList,
                                             AttributesModel *accountingAttModel,
                                             QWidget *parent) :
    QDialog(parent),
    m_d( new AttributeChangeDialogPrivate( itemsList, accountingAttModel ) ) {
    setup();
}

AttributeChangeDialog::AttributeChangeDialog(QList<AccountingTAMBillItem *> * itemsList,
                                             AttributesModel *accountingAttModel,
                                             QWidget *parent) :
    QDialog(parent),
    m_d( new AttributeChangeDialogPrivate( itemsList, accountingAttModel ) ) {
    setup();
}

void AttributeChangeDialog::setup(){
    m_d->ui->setupUi(this);
    m_d->ui->attributesTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    m_d->ui->attributesTableView->setModel( m_d->selectModel );

    connect( m_d->ui->cancelPushButton, &QPushButton::clicked, this, &AttributeChangeDialog::reject );
    connect( m_d->ui->addAttributesPushButton, &QPushButton::clicked, this, &AttributeChangeDialog::addAttributes );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AttributeChangeDialog::removeAttributes );
    connect( m_d->ui->setAttributesPushButton, &QPushButton::clicked, this, &AttributeChangeDialog::setAttributes );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

AttributeChangeDialog::~AttributeChangeDialog() {
    delete m_d;
}

void AttributeChangeDialog::addAttributes(){
    if( m_d->ui->attributesTableView->selectionModel() ){
        QList<Attribute *> selectedAttributes = m_d->selectModel->selectedAttributes();
        if( m_d->billItemsList != nullptr ){
            for( QList<BillItem *>::iterator i = m_d->billItemsList->begin(); i != m_d->billItemsList->end(); ++i){
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->addAttribute( *j );
                }
            }
        } else if( m_d->accountingBillItemsList != nullptr ){
            for( QList<AccountingBillItem *>::iterator i = m_d->accountingBillItemsList->begin(); i != m_d->accountingBillItemsList->end(); ++i){
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->addAttribute( *j );
                }
            }
        } else if( m_d->accountingTAMBillItemsList != nullptr ){
            for( QList<AccountingTAMBillItem *>::iterator i = m_d->accountingTAMBillItemsList->begin(); i != m_d->accountingTAMBillItemsList->end(); ++i){
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->addAttribute( *j );
                }
            }
        }
    }
    accept();
}

void AttributeChangeDialog::removeAttributes(){
    if( m_d->ui->attributesTableView->selectionModel() ){
        QList<Attribute *> selectedAttributes = m_d->selectModel->selectedAttributes();
        if( m_d->billItemsList != nullptr ){
            for( QList<BillItem *>::iterator i = m_d->billItemsList->begin(); i != m_d->billItemsList->end(); ++i){
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->removeAttribute( *j );
                }
            }
        } else if( m_d->accountingBillItemsList != nullptr ){
            for( QList<AccountingBillItem *>::iterator i = m_d->accountingBillItemsList->begin(); i != m_d->accountingBillItemsList->end(); ++i){
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->removeAttribute( *j );
                }
            }
        } else if( m_d->accountingTAMBillItemsList != nullptr ){
            for( QList<AccountingTAMBillItem *>::iterator i = m_d->accountingTAMBillItemsList->begin(); i != m_d->accountingTAMBillItemsList->end(); ++i){
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->removeAttribute( *j );
                }
            }
        }
    }
    accept();
}

void AttributeChangeDialog::setAttributes(){
    if( m_d->ui->attributesTableView->selectionModel() ){
        QList<Attribute *> selectedAttributes = m_d->selectModel->selectedAttributes();
        if( m_d->billItemsList != nullptr ){
            for( QList<BillItem *>::iterator i = m_d->billItemsList->begin(); i != m_d->billItemsList->end(); ++i){
                (*i)->removeAllAttributes();
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->addAttribute( *j );
                }
            }
        } else if( m_d->accountingBillItemsList != nullptr ){
            for( QList<AccountingBillItem *>::iterator i = m_d->accountingBillItemsList->begin(); i != m_d->accountingBillItemsList->end(); ++i){
                (*i)->removeAllAttributes();
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->addAttribute( *j );
                }
            }
        } else if( m_d->accountingTAMBillItemsList != nullptr ){
            for( QList<AccountingTAMBillItem *>::iterator i = m_d->accountingTAMBillItemsList->begin(); i != m_d->accountingTAMBillItemsList->end(); ++i){
                (*i)->removeAllAttributes();
                for( QList<Attribute *>::iterator j = selectedAttributes.begin(); j != selectedAttributes.end(); ++j ){
                    (*i)->addAttribute( *j );
                }
            }
        }
    }
    accept();
}
