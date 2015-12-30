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
#ifndef BILLPRINTERGUI_H
#define BILLPRINTERGUI_H

#include "billprinter.h"

class PriceFieldModel;

#include <QDialog>

class BillPrinterGUIPrivate;

class BillPrinterGUI : public QDialog {
    Q_OBJECT
public:
    explicit BillPrinterGUI( BillPrinter::PrintBillItemsOption *prItemsOption,
                             BillPrinter::PrintOption * prOption,
                             QList<int> * prFlds,
                             double *pWidth,
                             double *pHeight,
                             Qt::Orientation *pOrient,
                             bool * groupPrAm,
                             PriceFieldModel * pfm,
                             QWidget *parent = 0);
    ~BillPrinterGUI();
private slots:
    void setPrintData();
    void insertPriceFieldComboBox();
    void removePriceFieldComboBox();
private:
    BillPrinterGUIPrivate * m_d;
};

#endif // BILLPRINTERGUI_H
