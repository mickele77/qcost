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
#include "billsetpricelistmodegui.h"
#include "ui_setpricelistmodegui.h"

class BillSetPriceListModeGUIPrivate{
public:
    BillSetPriceListModeGUIPrivate():
        ui( new Ui::SetPriceListModeGUI() ) {
    };
    ~BillSetPriceListModeGUIPrivate(){
        delete ui;
    };

    Ui::SetPriceListModeGUI * ui;
};

BillSetPriceListModeGUI::BillSetPriceListModeGUI(QWidget *parent) :
    QDialog(parent),
    m_d(new BillSetPriceListModeGUIPrivate()) {
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

BillSetPriceListModeGUI::~BillSetPriceListModeGUI() {
    delete m_d;
}

Bill::SetPriceListMode BillSetPriceListModeGUI::returnValue() {
    if( exec() == QDialog::Accepted ){
        if( m_d->ui->searchAndAddRadioButton->isChecked() ){
            return Bill::SearchAndAdd;
        } else if( m_d->ui->addRadioButton->isChecked() ){
            return Bill::Add;
        } else if( m_d->ui->searchRadioButton->isChecked() ){
            return Bill::Search;
        } else if( m_d->ui->nullPriceItemRadioButton->isChecked() ){
            return Bill::NULLPriceItem;
        } else if( m_d->ui->resetRadioButton->isChecked() ){
            return Bill::ResetBill;
        }
    }
    return Bill::None;
}
