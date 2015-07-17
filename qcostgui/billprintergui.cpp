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
#include "billprintergui.h"
#include "ui_billprintergui.h"

#include "pricefieldmodel.h"

#include <QPageSize>
#include <QComboBox>

class BillPrinterGUIPrivate{
public:
    BillPrinterGUIPrivate( BillPrinter::PrintBillItemsOption * prItemsOption,
                           BillPrinter::PrintOption *prOption,
                           QList<int> * pFlds,
                           double *pWidth,
                           double *pHeight,
                           Qt::Orientation * pOrient,
                           bool *groupPrAm,
                           PriceFieldModel * pfm):
        ui(new Ui::BillPrinterGUI),
        printItemsOption(prItemsOption),
        printOption(prOption),
        printFields(pFlds),
        paperWidth(pWidth),
        paperHeight(pHeight),
        paperOrientation( pOrient ),
        groupPriceAmount( groupPrAm ),
        priceFieldsNames( pfm->fieldNames() ){
        pageSizeList << QPageSize( QPageSize::A4 );
    };
    ~BillPrinterGUIPrivate(){
        delete ui;
    };

    Ui::BillPrinterGUI *ui;
    BillPrinter::PrintBillItemsOption * printItemsOption;
    QList<QComboBox *> priceFieldComboBoxList;
    BillPrinter::PrintOption *printOption;
    QList<int> * printFields;
    double *paperWidth;
    double *paperHeight;
    Qt::Orientation * paperOrientation;
    bool * groupPriceAmount;
    QList<QString> priceFieldsNames;
    QList<QPageSize> pageSizeList;
};

BillPrinterGUI::BillPrinterGUI( BillPrinter::PrintBillItemsOption * prItemsOption,
                                BillPrinter::PrintOption *prOption,
                                QList<int> * prFlds,
                                double *pWidth,
                                double *pHeight,
                                Qt::Orientation * pOrient,
                                bool *groupPrAm,
                                PriceFieldModel * pfm,
                                QWidget *parent ) :
    QDialog(parent),
    m_d( new BillPrinterGUIPrivate( prItemsOption, prOption, prFlds, pWidth, pHeight, pOrient, groupPrAm, pfm) ) {
    m_d->ui->setupUi(this);

    connect( this, &BillPrinterGUI::accepted, this, &BillPrinterGUI::setPrintData );
    connect( m_d->ui->insertPriceFieldPushButton, &QPushButton::clicked, this, &BillPrinterGUI::insertPriceFieldComboBox );
    connect( m_d->ui->removePriceFieldPushButton, &QPushButton::clicked, this, &BillPrinterGUI::removePriceFieldComboBox );

    m_d->ui->groupPriceAmountCheckBox->setChecked( *groupPrAm );

    for( int i=0; i < m_d->pageSizeList.size(); ++i ){
        m_d->ui->paperDimensionsComboBox->addItem( m_d->pageSizeList.at(i).name() );
    }

    if( *(m_d->printItemsOption) == BillPrinter::PrintShortDesc ){
        m_d->ui->printShortDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == BillPrinter::PrintLongDesc ){
        m_d->ui->printLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == BillPrinter::PrintShortLongDesc ){
        m_d->ui->printShortLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == BillPrinter::PrintShortLongDescOpt ){
        m_d->ui->printShortLongDescOptRadioButton->setChecked( true );
    }

    if( *(m_d->printOption) == BillPrinter::PrintBill ){
        m_d->ui->printBillRadioButton->setChecked( true );
    } else if( *(m_d->printOption) == BillPrinter::PrintSummary ){
        m_d->ui->printSummaryRadioButton->setChecked( true );
    } else if( *(m_d->printOption) == BillPrinter::PrintSummaryWithDetails ){
        m_d->ui->printSummaryWithDetailsRadioButton->setChecked( true );
    }

    insertPriceFieldComboBox();

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

BillPrinterGUI::~BillPrinterGUI() {
    delete m_d;
}

void BillPrinterGUI::setPrintData(){
    if( m_d->ui->printBillRadioButton->isChecked() || m_d->ui->printEmptyBillRadioButton->isChecked() ){
        *(m_d->printOption) = BillPrinter::PrintBill;
    } else if( m_d->ui->printSummaryRadioButton->isChecked() ){
        *(m_d->printOption) = BillPrinter::PrintSummary;
    } else if( m_d->ui->printSummaryWithDetailsRadioButton->isChecked() ){
        *(m_d->printOption) = BillPrinter::PrintSummaryWithDetails;
    }

    m_d->printFields->clear();
    if( !m_d->ui->printEmptyBillRadioButton->isChecked() ){
        for( int i=0; i < m_d->priceFieldComboBoxList.size(); ++i){
            m_d->printFields->append( m_d->priceFieldComboBoxList.at(i)->currentIndex() );
        }
    }

    *(m_d->paperWidth) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).width();
    *(m_d->paperHeight) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).height();

    if( m_d->ui->paperHorizzontalRadioButton->isChecked() ){
        *(m_d->paperOrientation) = Qt::Horizontal;
    } else {
        *(m_d->paperOrientation) = Qt::Vertical;
    }

    if( m_d->ui->printShortDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = BillPrinter::PrintShortDesc;
    } else if( m_d->ui->printLongDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = BillPrinter::PrintLongDesc;
    } else if( m_d->ui->printShortLongDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = BillPrinter::PrintShortLongDesc;
    } else if( m_d->ui->printShortLongDescOptRadioButton->isChecked() ){
        *(m_d->printItemsOption) = BillPrinter::PrintShortLongDescOpt;
    }

    *(m_d->groupPriceAmount) = m_d->ui->groupPriceAmountCheckBox->isChecked();
}

void BillPrinterGUI::insertPriceFieldComboBox() {
    QComboBox * box = new QComboBox( this );

    // box populate
    for( QList<QString>::iterator i=m_d->priceFieldsNames.begin(); i != m_d->priceFieldsNames.end(); ++i ){
        box->addItem( *i );
    }
    m_d->priceFieldComboBoxList.append( box );
    m_d->ui->priceFieldComboBoxLayout->insertWidget( m_d->priceFieldComboBoxList.size()-1, box );
}

void BillPrinterGUI::removePriceFieldComboBox() {
    if( !m_d->priceFieldComboBoxList.isEmpty() ){
        m_d->ui->priceFieldComboBoxLayout->removeWidget( m_d->priceFieldComboBoxList.last() );
        delete m_d->priceFieldComboBoxList.takeLast();
    }
}
