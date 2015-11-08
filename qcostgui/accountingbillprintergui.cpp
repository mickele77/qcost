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
#include "accountingbillprintergui.h"
#include "ui_accountingbillprintergui.h"

#include "paymentdatamodel.h"
#include "paymentdata.h"

#include <QPageSize>
#include <QComboBox>
#include <QVariant>

class AccountingBillPrinterGUIPrivate{
public:
    AccountingBillPrinterGUIPrivate( AccountingPrinter::PrintPPUDescOption * prPPUDescOption,
                                     AccountingPrinter::PrintOption *prOption,
                                     AccountingPrinter::PrintAmountsOption * prAmountsOption,
                                     int * payToPrint,
                                     int payCount,
                                     double *pWidth,
                                     double *pHeight,
                                     Qt::Orientation * pOrient ):
        ui(new Ui::AccountingBillPrinterGUI),
        printPPUDescOption(prPPUDescOption),
        printOption(prOption),
        printAmountsOption(prAmountsOption),
        paymentToPrint(payToPrint),
        paymentsCount(payCount),
        paperWidth(pWidth),
        paperHeight(pHeight),
        paperOrientation( pOrient ) {
        pageSizeList << QPageSize( QPageSize::A4 );
    }
    ~AccountingBillPrinterGUIPrivate(){
        delete ui;
    }

    Ui::AccountingBillPrinterGUI *ui;
    AccountingPrinter::PrintPPUDescOption * printPPUDescOption;
    AccountingPrinter::PrintOption *printOption;
    AccountingPrinter::PrintAmountsOption * printAmountsOption;
    int * paymentToPrint;
    int paymentsCount;
    double *paperWidth;
    double *paperHeight;
    Qt::Orientation * paperOrientation;
    QList<QPageSize> pageSizeList;
};

AccountingBillPrinterGUI::AccountingBillPrinterGUI(PaymentDataModel * dataModel,
                                                   AccountingPrinter::PrintPPUDescOption * prPPUDescOption,
                                                   AccountingPrinter::PrintOption *prOption,
                                                   AccountingPrinter::PrintAmountsOption *prAmountsOption,
                                                   int * payToPrint,
                                                   double *pWidth,
                                                   double *pHeight,
                                                   Qt::Orientation * pOrient,
                                                   QWidget *parent ) :
    QDialog(parent),
    m_d( new AccountingBillPrinterGUIPrivate( prPPUDescOption, prOption, prAmountsOption, payToPrint, dataModel->paymentsCount(), pWidth, pHeight, pOrient ) ) {
    m_d->ui->setupUi(this);

    connect( this, &AccountingBillPrinterGUI::accepted, this, &AccountingBillPrinterGUI::setPrintData );

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
    } else if( *(m_d->printOption) == AccountingPrinter::PrintMeasures ){
        m_d->ui->printBillButton->setChecked( true );
    } else if( *(m_d->printOption) == AccountingPrinter::PrintRawMeasures ){
        m_d->ui->printRawBillButton->setChecked( true );
    }

    connect( m_d->ui->printBillButton, &QRadioButton::toggled, this, &AccountingBillPrinterGUI::updateOptionsAvailable );
    connect( m_d->ui->printPaymentButton, &QRadioButton::toggled, this, &AccountingBillPrinterGUI::updateOptionsAvailable );
    connect( m_d->ui->printAccountingButton, &QRadioButton::toggled, this, &AccountingBillPrinterGUI::updateOptionsAvailable );
    connect( m_d->ui->printAccountingSummaryButton, &QRadioButton::toggled, this, &AccountingBillPrinterGUI::updateOptionsAvailable );
    connect( m_d->ui->printRawBillButton, &QRadioButton::toggled, this, &AccountingBillPrinterGUI::updateOptionsAvailable );

    if( *(m_d->printAmountsOption) == AccountingPrinter::PrintTotalAmountsToDiscount ){
        m_d->ui->printTotalAmountsToDiscountRadioButton->setChecked( true );
    } else if( *(m_d->printAmountsOption) == AccountingPrinter::PrintAmountsNotToDiscount ){
        m_d->ui->printAmountsNotToDiscountRadioButton->setChecked( true );
    } else if( *(m_d->printAmountsOption) == AccountingPrinter::PrintAllAmounts ){
        m_d->ui->printAllAmountsRadioButton->setChecked( true );
    } else if( *(m_d->printAmountsOption) == AccountingPrinter::PrintNoAmount ){
        m_d->ui->printNoAmountsRadioButton->setChecked( true );
    }

    for( int i=0; i < dataModel->paymentsCount(); ++i ){
        m_d->ui->payToPrintComboBox->addItem( dataModel->paymentData(i)->name(), QVariant(i) );
    }
    m_d->ui->payToPrintComboBox->addItem(trUtf8("Tutti"), QVariant(-1) );
    m_d->ui->payToPrintComboBox->setCurrentIndex(*(payToPrint)+1);

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::WindowMaximizeButtonHint;
    setWindowFlags( flags );
}

AccountingBillPrinterGUI::~AccountingBillPrinterGUI() {
    delete m_d;
}

void AccountingBillPrinterGUI::updateOptionsAvailable(){
    if( m_d->ui->printBillButton->isChecked() ){
        m_d->ui->printNoAmountsRadioButton->setEnabled( true );
        m_d->ui->printNoAmountsRadioButton->setChecked(true);
        m_d->ui->printTotalAmountsToDiscountRadioButton->setDisabled( true );
        m_d->ui->printAmountsNotToDiscountRadioButton->setDisabled( true );
        m_d->ui->printAllAmountsRadioButton->setDisabled( true );
        if( m_d->ui->payToPrintComboBox->count() == m_d->paymentsCount ){
            m_d->ui->payToPrintComboBox->addItem( trUtf8("Tutti"), QVariant(-1) );
        }
    } else if( m_d->ui->printPaymentButton->isChecked() ||
               m_d->ui->printAccountingButton->isChecked() ){
        m_d->ui->printNoAmountsRadioButton->setDisabled( true );
        m_d->ui->printTotalAmountsToDiscountRadioButton->setDisabled( true );
        m_d->ui->printAmountsNotToDiscountRadioButton->setDisabled( true );
        m_d->ui->printAllAmountsRadioButton->setEnabled( true );
        m_d->ui->printAllAmountsRadioButton->setChecked(true);
        if( m_d->ui->payToPrintComboBox->count() > m_d->paymentsCount ){
            m_d->ui->payToPrintComboBox->removeItem( m_d->paymentsCount );
        }
    } else {
        m_d->ui->printNoAmountsRadioButton->setEnabled( true );
        m_d->ui->printTotalAmountsToDiscountRadioButton->setEnabled( true );
        m_d->ui->printAmountsNotToDiscountRadioButton->setEnabled( true );
        m_d->ui->printAllAmountsRadioButton->setEnabled( true );
        m_d->ui->printAllAmountsRadioButton->setEnabled( true );
        if( m_d->ui->payToPrintComboBox->count() == m_d->paymentsCount ){
            m_d->ui->payToPrintComboBox->addItem( trUtf8("Tutti"), QVariant(-1) );
        }
    }
}

void AccountingBillPrinterGUI::setPrintData(){
    if( m_d->ui->printTotalAmountsToDiscountRadioButton->isChecked() ){
        *(m_d->printAmountsOption) = AccountingPrinter::PrintTotalAmountsToDiscount;
    } else if( m_d->ui->printAmountsNotToDiscountRadioButton->isChecked() ){
        *(m_d->printAmountsOption) = AccountingPrinter::PrintAmountsNotToDiscount;
    } else if( m_d->ui->printAllAmountsRadioButton->isChecked() ){
        *(m_d->printAmountsOption) = AccountingPrinter::PrintAllAmounts;
    } else if( m_d->ui->printNoAmountsRadioButton->isChecked() ){
        *(m_d->printAmountsOption) = AccountingPrinter::PrintNoAmount;
    }

    if( m_d->ui->printAccountingButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintAccounting;
    } else if( m_d->ui->printAccountingSummaryButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintAccountingSummary;
    } else if( m_d->ui->printPaymentButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintPayment;
    } else if( m_d->ui->printBillButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintMeasures;
    } else if( m_d->ui->printRawBillButton->isChecked() ){
        *(m_d->printOption) = AccountingPrinter::PrintRawMeasures;
    }

    *(m_d->paymentToPrint) = m_d->ui->payToPrintComboBox->currentData().toInt();

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
