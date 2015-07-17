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
#include "loadfromtxtdialog.h"
#include "ui_loadfromtxtdialog.h"

#include <QComboBox>
#include <QTextStream>

#include <QDebug>

class LoadFromTXTDialogPrivate{
public:
    LoadFromTXTDialogPrivate( PriceListDBModel * m,
                              QList<PriceListDBModel::PriceColType> * pC,
                              QString * dSep,
                              QString * tSep,
                              bool * setSDFL,
                              double *ovh,
                              double *prf,
                              QLocale *l):
        model(m),
        pCols(pC),
        decimalSeparator( dSep ),
        thousandSeparator( tSep ),
        setShortDescFromLong(setSDFL),
        ui(new Ui::LoadFromTXTDialog),
        overheads(ovh),
        profits(prf),
        locale(l){
    };
    ~LoadFromTXTDialogPrivate(){
        delete ui;
    }

    PriceListDBModel * model;
    QList<PriceListDBModel::PriceColType> * pCols;
    QString * decimalSeparator;
    QString * thousandSeparator;
    bool * setShortDescFromLong;
    Ui::LoadFromTXTDialog *ui;
    QList <QComboBox *> comboBoxlist;
    double *overheads;
    double *profits;
    QLocale * locale;
};

LoadFromTXTDialog::LoadFromTXTDialog(PriceListDBModel *m,
                                     QList<PriceListDBModel::PriceColType> * pCols,
                                     QTextStream * in,
                                     QString * decimalSeparator,
                                     QString * thousandSeparator,
                                     bool * setShortDescFromLong,
                                     double *overheads,
                                     double *profits,
                                     QLocale *l,
                                     QWidget *parent) :
    QDialog(parent),
    m_d( new LoadFromTXTDialogPrivate(m, pCols, decimalSeparator, thousandSeparator, setShortDescFromLong, overheads, profits, l ) ) {
    m_d->ui->setupUi(this);

    int i=0;
    while( !in->atEnd() && i < 200 ){
        m_d->ui->plainTextEdit->appendPlainText( in->readLine() );
        ++i;
    }
    in->seek(0);
    m_d->ui->plainTextEdit->moveCursor( QTextCursor::Start );

    connect( m_d->ui->addColButton, SIGNAL(clicked()), this, SLOT(addComboBox()) );
    connect( m_d->ui->delColButton, SIGNAL(clicked()), this, SLOT(delComboBox()) );
    connect( this, SIGNAL(accepted()), this, SLOT(setValues()));

    m_d->ui->decimalSepLineEdit->setText( *(m_d->decimalSeparator));
    m_d->ui->thousandSepLineEdit->setText( *(m_d->thousandSeparator));
    m_d->ui->setShortDescFromLongCheckBox->setChecked( *setShortDescFromLong );
    m_d->ui->overheadsLineEdit->setText( m_d->locale->toString( *overheads, 'f', 6) );
    m_d->ui->profitsLineEdit->setText( m_d->locale->toString( *profits, 'f', 6) );

    addComboBox();
    m_d->comboBoxlist.last()->setCurrentIndex( m_d->comboBoxlist.last()->findData( QVariant(PriceListDBModel::nullCol) ) );
    addComboBox();
    m_d->comboBoxlist.last()->setCurrentIndex( m_d->comboBoxlist.last()->findData( QVariant(PriceListDBModel::codeCol) ) );
    addComboBox();
    m_d->comboBoxlist.last()->setCurrentIndex( m_d->comboBoxlist.last()->findData( QVariant(PriceListDBModel::longDescCol) ) );
    addComboBox();
    m_d->comboBoxlist.last()->setCurrentIndex( m_d->comboBoxlist.last()->findData( QVariant(PriceListDBModel::unitMeasureCol) ) );
    addComboBox();
    m_d->comboBoxlist.last()->setCurrentIndex( m_d->comboBoxlist.last()->findData( QVariant(PriceListDBModel::priceTotalCol) ) );
    addComboBox();
    m_d->comboBoxlist.last()->setCurrentIndex( m_d->comboBoxlist.last()->findData( QVariant(PriceListDBModel::priceHumanCol) ) );
}

LoadFromTXTDialog::~LoadFromTXTDialog() {

    delete m_d;
}

void LoadFromTXTDialog::setValues() {
    m_d->pCols->clear();
    for( int i=0; i < m_d->comboBoxlist.size(); ++i ){
        *(m_d->pCols) << (PriceListDBModel::PriceColType)(m_d->comboBoxlist.at(i)->itemData( m_d->comboBoxlist.at(i)->currentIndex() ).toInt());
    }
    *(m_d->decimalSeparator) = m_d->ui->decimalSepLineEdit->text();
    *(m_d->thousandSeparator) = m_d->ui->thousandSepLineEdit->text();
    *(m_d->setShortDescFromLong) = m_d->ui->setShortDescFromLongCheckBox->isChecked();
    *(m_d->overheads) = m_d->locale->toDouble( m_d->ui->overheadsLineEdit->text() );
    *(m_d->profits) = m_d->locale->toDouble( m_d->ui->profitsLineEdit->text() );
}

void LoadFromTXTDialog::addComboBox() {
    QComboBox * box = new QComboBox( this );

    // box populate
    QList< QPair<PriceListDBModel::PriceColType, QString> > cols = m_d->model->inputColsUserName();
    for( int i=0; i < cols.size(); ++i ){
        box->addItem( cols.at(i).second, QVariant( cols.at(i).first ));
    }

    m_d->comboBoxlist.append( box );
    m_d->ui->comboBoxLayout->addWidget( box, 0, m_d->comboBoxlist.size() - 1 );
}

void LoadFromTXTDialog::delComboBox() {
    if( !m_d->comboBoxlist.isEmpty() ){
        m_d->ui->comboBoxLayout->removeWidget( m_d->comboBoxlist.last() );
        delete m_d->comboBoxlist.takeLast();
    }
}
