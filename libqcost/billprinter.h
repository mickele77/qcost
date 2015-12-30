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
#ifndef BILLPRINTER_H
#define BILLPRINTER_H

#include "qcost_export.h"

class Bill;
class PriceFieldModel;
class MathParser;
class QString;
class Attribute;
template <typename T> class QList;

#include <Qt>

class BillPrinterPrivate;

class EXPORT_QCOST_LIB_OPT BillPrinter {
public:
    enum PrintBillItemsOption{
        PrintShortDesc,
        PrintLongDesc,
        PrintShortLongDesc,
        PrintShortLongDescOpt
    };
    enum PrintOption{
        PrintBill,
        PrintSummary,
        PrintSummaryWithDetails
    };
    enum AttributePrintOption{
        AttributePrintSimple,
        AttributePrintUnion,
        AttributePrintIntersection
    };

    BillPrinter(Bill *b, PriceFieldModel *pfm, MathParser *prs);
    ~BillPrinter();

    void setBill(Bill * b);

    bool printODT( PrintBillItemsOption prBillItemsOption,
                   PrintOption prOption,
                   const QList<int> & fieldsToPrint,
                   const QString &fileName,
                   double paperWidth = 210.0, double paperHeight = 297.0,
                   Qt::Orientation paperOrientation = Qt::Vertical,
                   bool groupPrAm = false ) const;

    bool printAttributeODT( PrintBillItemsOption prBillItemsOption,
                            BillPrinter::AttributePrintOption prOption,
                            const QList<int> & fieldsToPrint,
                            const QList<Attribute *> & attrsToPrint,
                            const QString &fileName,
                            double paperWidth = 210.0, double paperHeight = 297.0,
                            Qt::Orientation paperOrientation = Qt::Vertical,
                            bool groupPrAm = false ) const;

private:
    BillPrinterPrivate * m_d;

    bool printBillODT(PrintBillItemsOption prItemsOption,
                       const QList<int> &fieldsToPrint,
                       const QString &fileName,
                       double paperWidth = 210.0, double paperHeight = 297.0,
                       Qt::Orientation paperOrientation = Qt::Vertical,
                       bool groupPrAm = false ) const;

    bool printSummaryODT( PrintBillItemsOption prBillItemsOption,
                          const QList<int> &fieldsToPrint,
                          const QString &fileName,
                          double paperWidth = 210.0, double paperHeight = 297.0,
                          Qt::Orientation paperOrientation = Qt::Vertical,
                          bool groupPrAm = false,
                          bool writeDetails = true) const;
};

#endif // BILLPRINTER_H
