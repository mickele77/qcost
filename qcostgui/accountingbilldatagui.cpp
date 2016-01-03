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
#include "accountingbilldatagui.h"
#include "ui_accountingbilldatagui.h"

#include "accountingattributeprintergui.h"
#include "accountingbillsetpricelistmodegui.h"

#include "project.h"
#include "accountingbill.h"
#include "attributesmodel.h"
#include "accountingpricefieldmodel.h"
#include "pricelist.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class AccountingBillDataGUIPrivate{
public:
    AccountingBillDataGUIPrivate(PriceFieldModel * pfm, MathParser * prs, Project * prj, QString * wpf):
        ui(new Ui::AccountingBillDataGUI() ),
        project(prj),
        accounting(NULL),
        parser(prs),
        wordProcessorFile(wpf),
        priceFieldModel(pfm),
        amountSpacer(NULL){
    }
    Ui::AccountingBillDataGUI * ui;
    Project * project;
    AccountingBill * accounting;
    MathParser * parser;
    QString * wordProcessorFile;
    PriceFieldModel * priceFieldModel;
    QList<QLabel *> amountLabelList;
    QList<QLineEdit *> amountLEditList;
    QSpacerItem * amountSpacer;
};

AccountingBillDataGUI::AccountingBillDataGUI(PriceFieldModel * pfm, MathParser * prs, AccountingBill * b, Project * prj, QString * wordProcessorFile, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingBillDataGUIPrivate( pfm, prs, prj, wordProcessorFile ) ) {
    m_d->ui->setupUi( this );

    setAccountingBill( b );
}

AccountingBillDataGUI::~AccountingBillDataGUI(){
    delete m_d;
}

void AccountingBillDataGUI::setAccountingBill(AccountingBill *b) {
    if( m_d->accounting != b ){
        if( m_d->accounting != NULL ){
            disconnect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingBill::setName );
            disconnect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &AccountingBillDataGUI::setDescription );

            disconnect( m_d->accounting, &AccountingBill::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingBill::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingBill::amountToDiscountChanged, m_d->ui->amountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingBill::amountDiscountedChanged, m_d->ui->amountDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            disconnect( m_d->accounting, &AccountingBill::aboutToBeDeleted, this, &AccountingBillDataGUI::setAccountingNULL );
        }

        m_d->ui->totalAmountToDiscountLineEdit->clear();
        m_d->ui->amountNotToDiscountLineEdit->clear();
        m_d->ui->amountToDiscountLineEdit->clear();
        m_d->ui->amountDiscountedLineEdit->clear();
        m_d->ui->totalAmountLineEdit->clear();

        m_d->ui->nameLineEdit->clear();
        m_d->ui->descriptionTextEdit->clear();
        for( int i=0; i < m_d->amountLEditList.size(); ++i){
            if( i < m_d->priceFieldModel->fieldCount() ){
                m_d->amountLEditList.at(i)->clear();
            }
        }

        m_d->accounting = b;

        if( m_d->accounting != NULL ){
            m_d->ui->nameLineEdit->setText( m_d->accounting->name() );
            connect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingBill::setName );
            m_d->ui->descriptionTextEdit->setPlainText( m_d->accounting->description() );
            connect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &AccountingBillDataGUI::setDescription );

            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->accounting->totalAmountToDiscountStr() );
            connect( m_d->accounting, &AccountingBill::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->accounting->amountNotToDiscountStr() );
            connect( m_d->accounting, &AccountingBill::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountToDiscountLineEdit->setText( m_d->accounting->amountToDiscountStr() );
            connect( m_d->accounting, &AccountingBill::amountToDiscountChanged, m_d->ui->amountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountDiscountedLineEdit->setText( m_d->accounting->amountDiscountedStr() );
            connect( m_d->accounting, &AccountingBill::amountDiscountedChanged, m_d->ui->amountDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->accounting->totalAmountStr() );
            connect( m_d->accounting, &AccountingBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            connect( m_d->accounting, &AccountingBill::aboutToBeDeleted, this, &AccountingBillDataGUI::setAccountingNULL );
        }
    }
}

void AccountingBillDataGUI::setDescription(){
    if( m_d->accounting != NULL ){
        m_d->accounting->setDescription( m_d->ui->descriptionTextEdit->toPlainText() );
    }
}

void AccountingBillDataGUI::setAccountingNULL(){
    setAccountingBill( NULL );
}
