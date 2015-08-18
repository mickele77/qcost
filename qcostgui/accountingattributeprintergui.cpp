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
#include "accountingattributeprintergui.h"
#include "ui_accountingattributeprintergui.h"

#include "attributemodel.h"
#include "pricefieldmodel.h"

#include <QComboBox>
#include <QPageSize>

class AccountingAttributePrinterGUIPrivate{
public:
    AccountingAttributePrinterGUIPrivate( AccountingPrinter::PrintPPUDescOption * prItemsOption,
                                          AccountingPrinter::AttributePrintOption *prOption,
                                          QList<Attribute *> * pAttrs,
                                          double * pWidth,
                                          double * pHeight,
                                          Qt::Orientation * pOrient,
                                          bool *prAmounts,
                                          AttributeModel * bam ):
        ui(new Ui::AccountingAttributePrinterGUI),
        printItemsOption(prItemsOption),
        printOption(prOption),
        printAttributes(pAttrs),
        accountingAttributeModel(bam),
        paperWidth( pWidth ),
        paperHeight( pHeight ),
        paperOrientation( pOrient ),
        printAmounts( prAmounts ){
        pageSizeList << QPageSize( QPageSize::A4 ) << QPageSize( QPageSize::A3 );
    }
    ~AccountingAttributePrinterGUIPrivate(){
        delete ui;
    }

    Ui::AccountingAttributePrinterGUI *ui;
    AccountingPrinter::PrintPPUDescOption * printItemsOption;
    AccountingPrinter::AttributePrintOption *printOption;
    QList<Attribute *> * printAttributes;
    AttributeModel * accountingAttributeModel;
    double *paperWidth;
    double *paperHeight;
    Qt::Orientation * paperOrientation;
    bool * printAmounts;
    QList<QPageSize> pageSizeList;
};

AccountingAttributePrinterGUI::AccountingAttributePrinterGUI(AccountingPrinter::PrintPPUDescOption *prItemsOption,
                                                             AccountingPrinter::AttributePrintOption *prOption,
                                                             QList<Attribute *> * pAttrs,
                                                             double * pWidth,
                                                             double * pHeight,
                                                             Qt::Orientation * pOrient,
                                                             bool *printAmounts,
                                                             AttributeModel * bam,
                                                             QWidget *parent ) :
    QDialog(parent),
    m_d( new AccountingAttributePrinterGUIPrivate( prItemsOption, prOption, pAttrs, pWidth, pHeight, pOrient, printAmounts, bam) ) {
    m_d->ui->setupUi(this);
    m_d->ui->attributesTableView->setModel( m_d->accountingAttributeModel );

    connect( this, &AccountingAttributePrinterGUI::accepted, this, &AccountingAttributePrinterGUI::setPrintData );

    m_d->ui->printAmountsCheckBox->setChecked( *printAmounts );

    for( int i=0; i < m_d->pageSizeList.size(); ++i ){
        m_d->ui->paperDimensionsComboBox->addItem( m_d->pageSizeList.at(i).name() );
    }

    if( *(m_d->printItemsOption) == AccountingPrinter::PrintShortDesc ){
        m_d->ui->printShortDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == AccountingPrinter::PrintLongDesc ){
        m_d->ui->printLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == AccountingPrinter::PrintShortLongDesc ){
        m_d->ui->printShortLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printItemsOption) == AccountingPrinter::PrintShortLongDescOpt ){
        m_d->ui->printShortLongDescOptRadioButton->setChecked( true );
    }

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

AccountingAttributePrinterGUI::~AccountingAttributePrinterGUI() {
    delete m_d;
}

void AccountingAttributePrinterGUI::setPrintData(){
    if( m_d->ui->attributePrintSimpleRadioButton->isChecked() ){
        *m_d->printOption = AccountingPrinter::AttributePrintSimple;
    } else if( m_d->ui->attributePrintUnionRadioButton->isChecked() ){
        *m_d->printOption = AccountingPrinter::AttributePrintUnion;
    } else if( m_d->ui->attributePrintIntersectionRadioButton->isChecked() ){
        *m_d->printOption = AccountingPrinter::AttributePrintIntersection;
    }

    m_d->printAttributes->clear();
    QModelIndexList	selRows = m_d->ui->attributesTableView->selectionModel()->selectedRows();
    for( QModelIndexList::iterator i = selRows.begin(); i != selRows.end(); ++i ){
        Attribute * attr = m_d->accountingAttributeModel->attribute((*i).row() );
        if( !(m_d->printAttributes->contains(attr)) ){
            m_d->printAttributes->append( attr );
        }
    }

    *(m_d->paperWidth) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).width();
    *(m_d->paperHeight) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).height();

    if( m_d->ui->paperHorizzontalRadioButton->isChecked() ){
        *m_d->paperOrientation = Qt::Horizontal;
    } else {
        *m_d->paperOrientation = Qt::Vertical;
    }

    if( m_d->ui->printShortDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = AccountingPrinter::PrintShortDesc;
    } else if( m_d->ui->printLongDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = AccountingPrinter::PrintLongDesc;
    } else if( m_d->ui->printShortLongDescRadioButton->isChecked() ){
        *(m_d->printItemsOption) = AccountingPrinter::PrintShortLongDesc;
    } else if( m_d->ui->printShortLongDescOptRadioButton->isChecked() ){
        *(m_d->printItemsOption) = AccountingPrinter::PrintShortLongDescOpt;
    }

    *(m_d->printAmounts) = m_d->ui->printAmountsCheckBox->isChecked();
}
