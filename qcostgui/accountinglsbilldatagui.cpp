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
#include "accountinglsbilldatagui.h"
#include "ui_accountinglsbilldatagui.h"

#include "accountingattributeprintergui.h"
#include "accountingbillsetpricelistmodegui.h"

#include "project.h"
#include "accountinglsbill.h"
#include "attributesmodel.h"
#include "accountingpricefieldmodel.h"
#include "pricelist.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class AccountingLSBillDataGUIPrivate{
public:
    AccountingLSBillDataGUIPrivate():
        ui(new Ui::AccountingLSBillDataGUI() ),
        accounting(nullptr){
    }
    Ui::AccountingLSBillDataGUI * ui;
    AccountingLSBill * accounting;
};

AccountingLSBillDataGUI::AccountingLSBillDataGUI(QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingLSBillDataGUIPrivate() ) {
    m_d->ui->setupUi( this );

    connect( m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::editingFinished, this, &AccountingLSBillDataGUI::setPPUTotalToDiscount );
    connect( m_d->ui->PPUNotToDiscountLineEdit, &QLineEdit::editingFinished, this, &AccountingLSBillDataGUI::setPPUNotToDiscount );
}

AccountingLSBillDataGUI::~AccountingLSBillDataGUI(){
    delete m_d;
}

void AccountingLSBillDataGUI::setAccountingBill(AccountingLSBill *b) {
    if( m_d->accounting != b ){
        if( m_d->accounting != nullptr ){
            disconnect( m_d->ui->codeLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingLSBill::setCode );
            disconnect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingLSBill::setName );
            disconnect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &AccountingLSBillDataGUI::setDescription );

            disconnect( m_d->accounting, &AccountingLSBill::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingLSBill::PPUNotToDiscountChanged, m_d->ui->PPUNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingLSBill::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );
        }
        m_d->ui->codeLineEdit->clear();
        m_d->ui->nameLineEdit->clear();
        m_d->ui->descriptionTextEdit->clear();
        m_d->ui->PPUTotalToDiscountLineEdit->clear();
        m_d->ui->PPUNotToDiscountLineEdit->clear();
        m_d->ui->percentageAccountedLineEdit->clear();

        m_d->accounting = b;

        if( m_d->accounting != nullptr ){
            m_d->ui->codeLineEdit->setText( m_d->accounting->code() );
            connect( m_d->ui->codeLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingLSBill::setCode );
            m_d->ui->nameLineEdit->setText( m_d->accounting->name() );
            connect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingLSBill::setName );
            m_d->ui->descriptionTextEdit->setPlainText( m_d->accounting->description() );
            connect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &AccountingLSBillDataGUI::setDescription );

            m_d->ui->PPUTotalToDiscountLineEdit->setText( m_d->accounting->PPUTotalToDiscountStr() );
            connect( m_d->accounting, &AccountingLSBill::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            QString s = m_d->accounting->PPUNotToDiscountStr();
            m_d->ui->PPUNotToDiscountLineEdit->setText( m_d->accounting->PPUNotToDiscountStr() );
            connect( m_d->accounting, &AccountingLSBill::PPUNotToDiscountChanged, m_d->ui->PPUNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->percentageAccountedLineEdit->setText( m_d->accounting->percentageAccountedStr() );
            connect( m_d->accounting, &AccountingLSBill::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );

            connect( m_d->accounting, &AccountingLSBill::aboutToBeDeleted, this, &AccountingLSBillDataGUI::setAccountingBillnullptr );
        }
    }
}

void AccountingLSBillDataGUI::setDescription(){
    if( m_d->accounting ){
        m_d->accounting->setDescription( m_d->ui->descriptionTextEdit->toPlainText() );
    }
}

void AccountingLSBillDataGUI::setAccountingBillnullptr(){
    setAccountingBill( nullptr );
}

void AccountingLSBillDataGUI::setPPUTotalToDiscount(){
    if( m_d->accounting != nullptr ){
        m_d->accounting->setPPUTotalToDiscount( m_d->ui->PPUTotalToDiscountLineEdit->text() );
    }
}

void AccountingLSBillDataGUI::setPPUNotToDiscount(){
    if( m_d->accounting != nullptr ){
        m_d->accounting->setPPUNotToDiscount( m_d->ui->PPUNotToDiscountLineEdit->text() );
    }
}
