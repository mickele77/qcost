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
#ifndef ACCOUNTINGPRINTERGUI_H
#define ACCOUNTINGPRINTERGUI_H

class PaymentDataModel;

#include "accountingprinter.h"

#include <QDialog>

class AccountingPrinterGUIPrivate;

class AccountingPrinterGUI : public QDialog {
    Q_OBJECT
public:
    explicit AccountingPrinterGUI(PaymentDataModel *dataModel,
                                   AccountingPrinter::PrintPPUDescOption *prItemsOption,
                                   AccountingPrinter::PrintOption * prOption,
                                   AccountingPrinter::PrintAmountsOption * prAmountsOption,
                                   int * payToPrint,
                                   double *pWidth,
                                   double *pHeight,
                                   Qt::Orientation *pOrient,
                                   QWidget *parent = 0);
    ~AccountingPrinterGUI();
private slots:
    void setPrintData();
    void updateOptionsAvailable();
private:
    AccountingPrinterGUIPrivate * m_d;
};

#endif // ACCOUNTINGPRINTERGUI_H
