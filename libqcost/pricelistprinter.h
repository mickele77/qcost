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
#ifndef PRICELISTPRINTER_H
#define PRICELISTPRINTER_H

#include "qcost_export.h"

class PriceList;
class PriceFieldModel;
class QString;
template <typename T> class QList;

#include <Qt>

class PriceListPrinterPrivate;

class EXPORT_QCOST_LIB_OPT PriceListPrinter {
public:
    enum PrintPriceItemsOption{
        PrintShortDesc,
        PrintLongDesc,
        PrintShortLongDesc,
        PrintShortLongDescOpt
    };

    PriceListPrinter(PriceList *b, PriceFieldModel *pfm);
    ~PriceListPrinter();

    void setPriceList(PriceList * b);

    bool printODT( PrintPriceItemsOption printOption,
                   const QList<int> &fieldsToPrint,
                   int priceDataSetToPrintInput,
                   bool printNumLetters,
                   bool printPriceList,
                   bool printPriceAP, bool APgroupPrAm,
                   const QString &fileName,
                   double printerWidth = 210.0,
                   double printerHeight = 297.0,
                   Qt::Orientation paperOrientation = Qt::Vertical );

private:
    PriceListPrinterPrivate * m_d;
};

#endif // PRICELISTPRINTER_H
