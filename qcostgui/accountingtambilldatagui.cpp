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
#include "accountingtambilldatagui.h"
#include "ui_accountingtambilldatagui.h"

#include "accountingattributeprintergui.h"
#include "accountingbillsetpricelistmodegui.h"

#include "project.h"
#include "accountingtambill.h"
#include "attributesmodel.h"
#include "accountingpricefieldmodel.h"
#include "pricelist.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QShowEvent>
#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class AccountingTAMBillDataGUIPrivate{
public:
    AccountingTAMBillDataGUIPrivate():
        ui(new Ui::AccountingTAMBillDataGUI() ),
        accounting(nullptr),
        amountSpacer(nullptr){
    }
    Ui::AccountingTAMBillDataGUI * ui;
    AccountingTAMBill * accounting;
    QList<QLabel *> amountLabelList;
    QList<QLineEdit *> amountLEditList;
    QSpacerItem * amountSpacer;
};

AccountingTAMBillDataGUI::AccountingTAMBillDataGUI(QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingTAMBillDataGUIPrivate() ) {
    m_d->ui->setupUi( this );
}

AccountingTAMBillDataGUI::~AccountingTAMBillDataGUI(){
    delete m_d;
}

void AccountingTAMBillDataGUI::setAccountingTAMBill(AccountingTAMBill *b) {
    if( m_d->accounting != b ){
        if( m_d->accounting != nullptr ){
            disconnect( m_d->accounting, &AccountingTAMBill::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::amountToDiscountChanged, m_d->ui->amountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::amountDiscountedChanged, m_d->ui->amountDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTAMBillDataGUI::setAccountingnullptr );
        }

        for( int i=0; i < m_d->amountLEditList.size(); ++i){
            m_d->amountLEditList.at(i)->clear();
        }

        m_d->accounting = b;

        if( m_d->accounting != nullptr ){
            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->accounting->totalAmountToDiscountStr() );
            connect( m_d->accounting, &AccountingTAMBill::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->accounting->amountNotToDiscountStr() );
            connect( m_d->accounting, &AccountingTAMBill::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountToDiscountLineEdit->setText( m_d->accounting->amountToDiscountStr() );
            connect( m_d->accounting, &AccountingTAMBill::amountToDiscountChanged, m_d->ui->amountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountDiscountedLineEdit->setText( m_d->accounting->amountDiscountedStr() );
            connect( m_d->accounting, &AccountingTAMBill::amountDiscountedChanged, m_d->ui->amountDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->accounting->totalAmountStr() );
            connect( m_d->accounting, &AccountingTAMBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            connect( m_d->accounting, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTAMBillDataGUI::setAccountingnullptr );
        }
    }
}

void AccountingTAMBillDataGUI::setAccountingnullptr(){
    setAccountingTAMBill( nullptr );
}

void AccountingTAMBillDataGUI::updateAmountValue(int priceField, const QString & newVal ){
    if( priceField > -1 && priceField < m_d->amountLEditList.size() ){
        m_d->amountLEditList.at(priceField)->setText( newVal );
    }
}
