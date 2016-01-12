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
#include "accountinglsbillprintergui.h"
#include "ui_accountinglsbillprintergui.h"

#include "accountingbill.h"
#include "accountingbillitem.h"

#include <QPageSize>
#include <QComboBox>

class AccountingLSBillPrinterGUIPrivate{
public:
    AccountingLSBillPrinterGUIPrivate( AccountingPrinter::PrintPPUDescOption * prPPUDescOption,
                                       AccountingPrinter::PrintOption *prOption,
                                       AccountingPrinter::PrintLSOption *prLSOption,
                                       bool * prAmounts,
                                       int * payToPrint,
                                       double *pWidth,
                                       double *pHeight,
                                       Qt::Orientation * pOrient ):
        ui(new Ui::AccountingLSBillPrinterGUI),
        printPPUDescOption(prPPUDescOption),
        printOption(prOption),
        printLSOption(prLSOption),
        printAmounts(prAmounts),
        paymentToPrint(payToPrint),
        paperWidth(pWidth),
        paperHeight(pHeight),
        paperOrientation( pOrient ) {
        pageSizeList << QPageSize( QPageSize::A4 );
    }
    ~AccountingLSBillPrinterGUIPrivate(){
        delete ui;
    }

    Ui::AccountingLSBillPrinterGUI *ui;
    AccountingPrinter::PrintPPUDescOption * printPPUDescOption;
    AccountingPrinter::PrintOption *printOption;
    AccountingPrinter::PrintLSOption *printLSOption;
    bool * printAmounts;
    int * paymentToPrint;
    double *paperWidth;
    double *paperHeight;
    Qt::Orientation * paperOrientation;
    QList<QPageSize> pageSizeList;
};

AccountingLSBillPrinterGUI::AccountingLSBillPrinterGUI( AccountingBill * measuresBill,
                                                        AccountingPrinter::PrintPPUDescOption * prPPUDescOption,
                                                        AccountingPrinter::PrintOption *prOption, AccountingPrinter::PrintLSOption *prLSOption,
                                                        bool * printAmounts,
                                                        int * payToPrint,
                                                        double *pWidth,
                                                        double *pHeight,
                                                        Qt::Orientation * pOrient,
                                                        QWidget *parent ) :
    QDialog(parent),
    m_d( new AccountingLSBillPrinterGUIPrivate( prPPUDescOption, prOption, prLSOption, printAmounts, payToPrint, pWidth, pHeight, pOrient ) ) {
    m_d->ui->setupUi(this);

    connect( this, &AccountingLSBillPrinterGUI::accepted, this, &AccountingLSBillPrinterGUI::setPrintData );

    for( int i=0; i < m_d->pageSizeList.size(); ++i ){
        m_d->ui->paperDimensionsComboBox->addItem( m_d->pageSizeList.at(i).name() );
    }

    if( *(m_d->printPPUDescOption) == AccountingPrinter::PrintShortDesc ){
        m_d->ui->printShortDescRadioButton->setChecked( true );
    } else if( *(m_d->printPPUDescOption) == AccountingPrinter::PrintLongDesc ){
        m_d->ui->printLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printPPUDescOption) == AccountingPrinter::PrintShortLongDesc ){
        m_d->ui->printShortLongDescRadioButton->setChecked( true );
    } else if( *(m_d->printPPUDescOption) == AccountingPrinter::PrintShortLongDescOpt ){
        m_d->ui->printShortLongDescOptRadioButton->setChecked( true );
    }

    if( *(m_d->printOption) == AccountingPrinter::PrintRawMeasures ){
        m_d->ui->printRawMeasuresButton->setChecked( true );
    } else if( *(m_d->printOption) == AccountingPrinter::PrintMeasures ){
        m_d->ui->printMeasuresButton->setChecked( true );
    } else if( *(m_d->printOption) == AccountingPrinter::PrintAccountingSummary ){
        m_d->ui->printAccountingSummaryButton->setChecked( true );
    }

    if( *(m_d->printLSOption) == AccountingPrinter::PrintLSAcc ){
        m_d->ui->printAccRadioButton->setChecked( true );
    } else if( *(m_d->printLSOption) == AccountingPrinter::PrintLSProj ){
        m_d->ui->printProjRadioButton->setChecked( true );
    } else if( *(m_d->printLSOption) == AccountingPrinter::PrintLSProjAcc ){
        m_d->ui->printProjAccRadioButton->setChecked( true );
    }

    m_d->ui->printAmountsCheckBox->setChecked( *(m_d->printAmounts) );

    connect( m_d->ui->printMeasuresButton, &QRadioButton::toggled, this, &AccountingLSBillPrinterGUI::updateOptionsAvailable );
    connect( m_d->ui->printAccountingSummaryButton, &QRadioButton::toggled, this, &AccountingLSBillPrinterGUI::updateOptionsAvailable );
    connect( m_d->ui->printRawMeasuresButton, &QRadioButton::toggled, this, &AccountingLSBillPrinterGUI::updateOptionsAvailable );

    m_d->ui->billToPrintComboBox->insertItem(0, trUtf8("Tutti"));
    for( int i=0; i < measuresBill->paymentsCount(); ++i ){
        m_d->ui->billToPrintComboBox->insertItem((i+1), measuresBill->payment(i)->name() );
    }
    m_d->ui->billToPrintComboBox->setCurrentIndex(*(payToPrint)+1);

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

AccountingLSBillPrinterGUI::~AccountingLSBillPrinterGUI() {
    delete m_d;
}

void AccountingLSBillPrinterGUI::updateOptionsAvailable(){
    if( m_d->ui->printMeasuresButton->isChecked() ){
        m_d->ui->printAmountsCheckBox->setChecked( false );
        m_d->ui->printAmountsCheckBox->setEnabled( false );
    } else {
        m_d->ui->printAmountsCheckBox->setEnabled( true );
    }
}

void AccountingLSBillPrinterGUI::setPrintData(){
    *(m_d->printAmounts) = m_d->ui->printAmountsCheckBox->isChecked();

    if( m_d->ui->printProjAccRadioButton->isChecked() ){
        *(m_d->printLSOption) = AccountingPrinter::PrintLSProjAcc;
    } else if( m_d->ui->printProjRadioButton->isChecked() ){
        *(m_d->printLSOption) = AccountingPrinter::PrintLSProj;
    } else if( m_d->ui->printAccRadioButton->isChecked() ){
        *(m_d->printLSOption) = AccountingPrinter::PrintLSAcc;
    }

    if( m_d->ui->printAccountingSummaryButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintAccountingSummary;
    } else if( m_d->ui->printMeasuresButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintMeasures;
    } else if( m_d->ui->printRawMeasuresButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintRawMeasures;
    }

    *(m_d->paymentToPrint) = m_d->ui->billToPrintComboBox->currentIndex() - 1;

    *(m_d->paperWidth) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).width();
    *(m_d->paperHeight) = m_d->pageSizeList.at( m_d->ui->paperDimensionsComboBox->currentIndex() ).size( QPageSize::Millimeter ).height();

    if( m_d->ui->paperHorizzontalRadioButton->isChecked() ){
        *(m_d->paperOrientation) = Qt::Horizontal;
    } else {
        *(m_d->paperOrientation) = Qt::Vertical;
    }

    if( m_d->ui->printShortDescRadioButton->isChecked() ){
        *(m_d->printPPUDescOption) = AccountingPrinter::PrintShortDesc;
    } else if( m_d->ui->printLongDescRadioButton->isChecked() ){
        *(m_d->printPPUDescOption) = AccountingPrinter::PrintLongDesc;
    } else if( m_d->ui->printShortLongDescRadioButton->isChecked() ){
        *(m_d->printPPUDescOption) = AccountingPrinter::PrintShortLongDesc;
    } else if( m_d->ui->printShortLongDescOptRadioButton->isChecked() ){
        *(m_d->printPPUDescOption) = AccountingPrinter::PrintShortLongDescOpt;
    }
}