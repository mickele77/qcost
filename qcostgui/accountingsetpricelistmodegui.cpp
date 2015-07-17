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
#include "accountingsetpricelistmodegui.h"
#include "ui_setpricelistmodegui.h"

class AccountingSetPriceListModeGUIPrivate{
public:
    AccountingSetPriceListModeGUIPrivate():
        ui( new Ui::SetPriceListModeGUI() ) {
    }
    ~AccountingSetPriceListModeGUIPrivate(){
        delete ui;
    }

    Ui::SetPriceListModeGUI * ui;
};

AccountingSetPriceListModeGUI::AccountingSetPriceListModeGUI(QWidget *parent) :
    QDialog(parent),
    m_d(new AccountingSetPriceListModeGUIPrivate()) {
    m_d->ui->setupUi(this);

    m_d->ui->searchAndAddRadioButton->setToolTip( trUtf8("Cerca i prezzi nel nuovo elenco prezzi in base al codice; aggiunge all'ellenco prezzi i prezzi mancanti"));
    m_d->ui->addRadioButton->setToolTip( trUtf8("Aggiunge i prezzo del vecchio EP al nuovo EP"));
    m_d->ui->searchRadioButton->setToolTip( trUtf8("Cerca i prezzi nel nuovo elenco prezzi in base al codice; imposta un prezzo nullo per le voci mancanti"));
    m_d->ui->nullPriceItemRadioButton->setToolTip( "Imposta un prezzo nullo per le voci di computo conservandone le quantità" );
    m_d->ui->resetRadioButton->setToolTip( "Cancella tutte le righe del computo" );

    m_d->ui->searchAndAddRadioButton->setChecked( true );

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

AccountingSetPriceListModeGUI::~AccountingSetPriceListModeGUI() {
    delete m_d;
}

AccountingBill::SetPriceListMode AccountingSetPriceListModeGUI::returnValue() {
    if( exec() == QDialog::Accepted ){
        if( m_d->ui->searchAndAddRadioButton->isChecked() ){
            return AccountingBill::SearchAndAdd;
        } else if( m_d->ui->addRadioButton->isChecked() ){
            return AccountingBill::Add;
        } else if( m_d->ui->searchRadioButton->isChecked() ){
            return AccountingBill::Search;
        } else if( m_d->ui->nullPriceItemRadioButton->isChecked() ){
            return AccountingBill::NULLPriceItem;
        } else if( m_d->ui->resetRadioButton->isChecked() ){
            return AccountingBill::ResetBill;
        }
    }
    return AccountingBill::None;
}

AccountingTAMBill::SetPriceListMode AccountingSetPriceListModeGUI::returnTAMValue() {
    if( exec() == QDialog::Accepted ){
        if( m_d->ui->searchAndAddRadioButton->isChecked() ){
            return AccountingTAMBill::SearchAndAdd;
        } else if( m_d->ui->addRadioButton->isChecked() ){
            return AccountingTAMBill::Add;
        } else if( m_d->ui->searchRadioButton->isChecked() ){
            return AccountingTAMBill::Search;
        } else if( m_d->ui->nullPriceItemRadioButton->isChecked() ){
            return AccountingTAMBill::NULLPriceItem;
        } else if( m_d->ui->resetRadioButton->isChecked() ){
            return AccountingTAMBill::ResetBill;
        }
    }
    return AccountingTAMBill::None;
}
