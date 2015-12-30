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
#include "pricelistdatagui.h"

#include "ui_pricelistdatagui.h"

#include "pricefieldmodel.h"
#include "pricelist.h"

class PriceListDataGUIPrivate{
public:
    PriceListDataGUIPrivate():
        ui( new Ui::PriceListDataGUI() ),
        priceList(NULL) {
    };

    Ui::PriceListDataGUI * ui;
    PriceList * priceList;
};

PriceListDataGUI::PriceListDataGUI(PriceList *p, QWidget *parent) :
    QWidget(parent),
    m_d(new PriceListDataGUIPrivate() ){

    m_d->ui->setupUi(this);

    setPriceList( p );
}

PriceListDataGUI::~PriceListDataGUI(){
    delete m_d;
}

void PriceListDataGUI::setPriceList(PriceList *p) {
    if( m_d->priceList != p ){
        if( m_d->priceList != NULL ){
            disconnect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->priceList, &PriceList::setName );
            disconnect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &PriceListDataGUI::setDescription );
            disconnect( m_d->priceList, &PriceList::aboutToBeDeleted, this, &PriceListDataGUI::setPriceListNULL );
        }
        m_d->ui->nameLineEdit->clear();
        m_d->ui->descriptionTextEdit->clear();
        m_d->priceList = p;
        if( m_d->priceList != NULL ){
            m_d->ui->nameLineEdit->setText( m_d->priceList->name() );
            connect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->priceList, &PriceList::setName );
            m_d->ui->descriptionTextEdit->setPlainText( m_d->priceList->description() );
            connect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &PriceListDataGUI::setDescription );
            connect( m_d->priceList, &PriceList::aboutToBeDeleted, this, &PriceListDataGUI::setPriceListNULL );
        }
    }
}

void PriceListDataGUI::setPriceListNULL(){
    setPriceList( NULL );
}

void PriceListDataGUI::setDescription(){
    if( m_d->priceList ){
        m_d->priceList->setDescription( m_d->ui->descriptionTextEdit->toPlainText() );
    }
}
