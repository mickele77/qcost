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
#include "accountingprintergui.h"
#include "ui_accountingprintergui.h"

#include "paymentadatamodel.h"
#include "paymentdata.h"

#include <QPageSize>
#include <QComboBox>

class AccountingPrinterGUIPrivate{
public:
    AccountingPrinterGUIPrivate( PaymentDataModel * dModel,
                                 AccountingPrinter::PrintPPUDescOption * prPPUDescOption,
                                 AccountingPrinter::PrintOption *prOption,
                                 int * bToPrint,
                                 double *pWidth,
                                 double *pHeight,
                                 Qt::Orientation * pOrient ):
        ui(new Ui::AccountingPrinterGUI),
        dataModel(dModel),
        printPPUDescOption(prPPUDescOption),
        printOption(prOption),
        billToPrint(bToPrint),
        paperWidth(pWidth),
        paperHeight(pHeight),
        paperOrientation( pOrient ) {
        pageSizeList << QPageSize( QPageSize::A4 );
    }
    ~AccountingPrinterGUIPrivate(){
        delete ui;
    }

    Ui::AccountingPrinterGUI *ui;
    PaymentDataModel * dataModel;
    AccountingPrinter::PrintPPUDescOption * printPPUDescOption;
    AccountingPrinter::PrintOption *printOption;
    int * billToPrint;
    double *paperWidth;
    double *paperHeight;
    Qt::Orientation * paperOrientation;
    QList<QPageSize> pageSizeList;
};

AccountingPrinterGUI::AccountingPrinterGUI( PaymentDataModel * dataModel,
                                            AccountingPrinter::PrintPPUDescOption * prPPUDescOption,
                                            AccountingPrinter::PrintOption *prOption,
                                            int * billToPrint,
                                            double *pWidth,
                                            double *pHeight,
                                            Qt::Orientation * pOrient,
                                            QWidget *parent ) :
    QDialog(parent),
    m_d( new AccountingPrinterGUIPrivate( dataModel, prPPUDescOption, prOption, billToPrint, pWidth, pHeight, pOrient ) ) {
    m_d->ui->setupUi(this);

    connect( this, &AccountingPrinterGUI::accepted, this, &AccountingPrinterGUI::setPrintData );

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

    if( *(m_d->printOption) == AccountingPrinter::PrintAccounting ){
        m_d->ui->printAccountingButton->setChecked( true );
    } else if( *(m_d->printOption) == AccountingPrinter::PrintAccountingSummary ){
        m_d->ui->printAccountingSummaryButton->setChecked( true );
    } else if( *(m_d->printOption) == AccountingPrinter::PrintPayment ){
        m_d->ui->printPaymentButton->setChecked( true );
    } else if( *(m_d->printOption) == AccountingPrinter::PrintBill ){
        m_d->ui->printBillButton->setChecked( true );
    } else if( *(m_d->printOption) == AccountingPrinter::PrintRawBill ){
        m_d->ui->printRawBillButton->setChecked( true );
    }

    m_d->ui->billToPrintComboBox->insertItem(0, trUtf8("Tutti"));
    for( int i=0; i < m_d->dataModel->paymentsCount(); ++i ){
        m_d->ui->billToPrintComboBox->insertItem((i+1), m_d->dataModel->billData(i)->name() );
    }
    m_d->ui->billToPrintComboBox->setCurrentIndex(0);


    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

AccountingPrinterGUI::~AccountingPrinterGUI() {
    delete m_d;
}

void AccountingPrinterGUI::setPrintData(){
    if( m_d->ui->printAccountingButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintAccounting;
    } else if( m_d->ui->printAccountingSummaryButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintAccountingSummary;
    } else if( m_d->ui->printPaymentButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintPayment;
    } else if( m_d->ui->printBillButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintBill;
    } else if( m_d->ui->printRawBillButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintRawBill;
    }

    *(m_d->billToPrint) = m_d->ui->billToPrintComboBox->currentIndex() - 1;

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
