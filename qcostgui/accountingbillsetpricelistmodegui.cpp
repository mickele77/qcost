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
#include "accountingbillsetpricelistmodegui.h"
#include "ui_setpricelistmodegui.h"

class AccountingBillSetPriceListModeGUIPrivate{
public:
    AccountingBillSetPriceListModeGUIPrivate():
        ui( new Ui::SetPriceListModeGUI() ) {
    }
    ~AccountingBillSetPriceListModeGUIPrivate(){
        delete ui;
    }

    Ui::SetPriceListModeGUI * ui;
};

AccountingBillSetPriceListModeGUI::AccountingBillSetPriceListModeGUI(QWidget *parent) :
    QDialog(parent),
    m_d(new AccountingBillSetPriceListModeGUIPrivate()) {
    m_d->ui->setupUi(this);

    m_d->ui->searchAndAddRadioButton->setToolTip( tr("Cerca i prezzi nel nuovo elenco prezzi in base al codice; aggiunge all'ellenco prezzi i prezzi mancanti"));
    m_d->ui->addRadioButton->setToolTip( tr("Aggiunge i prezzo del vecchio EP al nuovo EP"));
    m_d->ui->searchRadioButton->setToolTip( tr("Cerca i prezzi nel nuovo elenco prezzi in base al codice; imposta un prezzo nullptro per le voci mancanti"));
    m_d->ui->nullptrPriceItemRadioButton->setToolTip( "Imposta un prezzo nullptro per le voci di computo conservandone le quantità" );
    m_d->ui->resetRadioButton->setToolTip( "Cancella tutte le righe del computo" );

    m_d->ui->searchAndAddRadioButton->setChecked( true );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

AccountingBillSetPriceListModeGUI::~AccountingBillSetPriceListModeGUI() {
    delete m_d;
}

AccountingBill::SetPriceListMode AccountingBillSetPriceListModeGUI::returnValue() {
    if( exec() == QDialog::Accepted ){
        if( m_d->ui->searchAndAddRadioButton->isChecked() ){
            return AccountingBill::SearchAndAdd;
        } else if( m_d->ui->addRadioButton->isChecked() ){
            return AccountingBill::Add;
        } else if( m_d->ui->searchRadioButton->isChecked() ){
            return AccountingBill::Search;
        } else if( m_d->ui->nullptrPriceItemRadioButton->isChecked() ){
            return AccountingBill::nullptrPriceItem;
        } else if( m_d->ui->resetRadioButton->isChecked() ){
            return AccountingBill::ResetBill;
        }
    }
    return AccountingBill::None;
}

AccountingTAMBill::SetPriceListMode AccountingBillSetPriceListModeGUI::returnValueTAMBill() {
    if( exec() == QDialog::Accepted ){
        if( m_d->ui->searchAndAddRadioButton->isChecked() ){
            return AccountingTAMBill::SearchAndAdd;
        } else if( m_d->ui->addRadioButton->isChecked() ){
            return AccountingTAMBill::Add;
        } else if( m_d->ui->searchRadioButton->isChecked() ){
            return AccountingTAMBill::Search;
        } else if( m_d->ui->nullptrPriceItemRadioButton->isChecked() ){
            return AccountingTAMBill::nullptrPriceItem;
        } else if( m_d->ui->resetRadioButton->isChecked() ){
            return AccountingTAMBill::ResetBill;
        }
    }
    return AccountingTAMBill::None;
}

AccountingLSBill::SetPriceListMode AccountingBillSetPriceListModeGUI::returnValueLSBill() {
    if( exec() == QDialog::Accepted ){
        if( m_d->ui->searchAndAddRadioButton->isChecked() ){
            return AccountingLSBill::SearchAndAdd;
        } else if( m_d->ui->addRadioButton->isChecked() ){
            return AccountingLSBill::Add;
        } else if( m_d->ui->searchRadioButton->isChecked() ){
            return AccountingLSBill::Search;
        } else if( m_d->ui->nullptrPriceItemRadioButton->isChecked() ){
            return AccountingLSBill::nullptrPriceItem;
        } else if( m_d->ui->resetRadioButton->isChecked() ){
            return AccountingLSBill::ResetBill;
        }
    }
    return AccountingLSBill::None;
}
