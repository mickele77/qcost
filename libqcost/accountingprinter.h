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
#ifndef ACCOUNTINGPRINTER_H
#define ACCOUNTINGPRINTER_H

#include "qcost_export.h"

class PaymentDataModel;
class AccountingBill;
class AccountingTAMBill;
class AccountingLSBills;
class AccountingLSBill;
class PriceFieldModel;
class MathParser;
class QString;
class Attribute;
template <typename T> class QList;

#include <QTextLength>
#include <Qt>

class AccountingPrinterPrivate;

class EXPORT_QCOST_LIB_OPT AccountingPrinter {
public:
    enum PrintAmountsOption{
        PrintNoAmount,
        PrintTotalAmountsToDiscount,
        PrintAmountsNotToDiscount,
        PrintAllAmounts
    };
    enum PrintPPUDescOption{
        PrintShortDesc,
        PrintLongDesc,
        PrintShortLongDesc,
        PrintShortLongDescOpt
    };
    enum PrintOption{
        PrintRawMeasures,
        PrintMeasures,
        PrintPayment,
        PrintAccounting,
        PrintAccountingSummary
    };
    enum PrintLSOption{
        PrintLSProj,
        PrintLSAcc,
        PrintLSProjAcc
    };
    enum AttributePrintOption{
        AttributePrintSimple,
        AttributePrintUnion,
        AttributePrintIntersection
    };

    AccountingPrinter(AccountingBill *b, MathParser *prs);
    AccountingPrinter(AccountingTAMBill *b, MathParser *prs);
    AccountingPrinter(AccountingLSBills *b, MathParser *prs);
    AccountingPrinter(AccountingLSBill *b, MathParser *prs);
    ~AccountingPrinter();

    /** Stampa:
     * - libretto delle misure
     * - brogliaccio del libretto delle misure
     * - S.A.L. */
    bool printODT( int payToPrint,
                   PrintOption prOption,
                   PrintAmountsOption prAmountsOption,
                   PrintPPUDescOption prPPUDescOption,
                   const QString &fileName,
                   double paperWidth = 210.0, double paperHeight = 297.0,
                   Qt::Orientation paperOrientation = Qt::Vertical) const;

    /** Stampa brogliaccio/libretto opere a corpo */
    bool printLSODT( int payToPrint,
                     PaymentDataModel *payDataModel,
                     AccountingPrinter::PrintOption prOption,
                     AccountingPrinter::PrintLSOption prLSOption,
                     AccountingPrinter::PrintPPUDescOption prPPDescOption,
                     bool printAmounts,
                     const QString &fileName,
                     double paperWidth, double paperHeight,
                     Qt::Orientation paperOrientation ) const;

    bool printAttributeODT( AttributePrintOption prOption,
                            PrintAmountsOption prAmountsOption,
                            PrintPPUDescOption prPPUDescOption,
                            const QList<Attribute *> &attrsToPrint,
                            const QString &fileName,
                            double paperWidth = 210.0, double paperHeight = 297.0,
                            Qt::Orientation paperOrientation = Qt::Vertical ) const;

private:
    AccountingPrinterPrivate * m_d;

    /** Interfaccia privata per stampa sommario contabilita' */
    bool printSummaryODT( int payToPrint,
                          PrintAmountsOption prAmountsOption,
                          PrintPPUDescOption prPPUDescOption,
                          const QString &fileName,
                          double paperWidth = 210.0, double paperHeight = 297.0,
                          Qt::Orientation paperOrientation = Qt::Vertical,
                          bool writeDetails = true ) const;

    /** Interfaccia privata per stampa registro di contabilità */
    bool printAccountingODT( int payToPrint,
                             AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                             const QString &fileName,
                             double paperWidth = 210.0, double paperHeight = 297.0,
                             Qt::Orientation paperOrientation = Qt::Vertical ) const;

    /** Interfaccia privata per stampa S.A.L. */
    bool printPaymentODT( int payToPrint,
                          AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                          const QString &fileName,
                          int paperWidth = 210.0, int paperHeight=297.0,
                          Qt::Orientation paperOrientation = Qt::Vertical ) const;

    /** Interfaccia privata per stampa libretto delle misure/brogliaccio */
    bool printMeasuresODT( int payToPrint,
                           AccountingPrinter::PrintOption prOption,
                           AccountingPrinter::PrintAmountsOption prAmountsOption,
                           AccountingPrinter::PrintPPUDescOption prPPDescOption,
                           const QString &fileName,
                           double paperWidth = 210.0, double paperHeight = 297.0,
                           Qt::Orientation paperOrientation = Qt::Vertical ) const;

    /** Interfaccia privata per stampa lista in economia */
    bool printTAMMeasuresODT( int billToPrint,
                              AccountingPrinter::PrintOption prOption,
                              AccountingPrinter::PrintAmountsOption prAmountsOption,
                              AccountingPrinter::PrintPPUDescOption prPPDescOption,
                              const QString &fileName,
                              double paperWidth = 210.0, double paperHeight = 297.0,
                              Qt::Orientation paperOrientation = Qt::Vertical ) const;

    /** Interfaccia privata per stampa libretto/brogliaccio di una sola categoria a corpo */
    bool printLSMeasuresODT( int payToPrint,
                             PaymentDataModel *payDataModel,
                             AccountingPrinter::PrintOption prOption,
                             AccountingPrinter::PrintLSOption prLSOption,
                             AccountingPrinter::PrintPPUDescOption prPPDescOption,
                             bool printAmounts,
                             const QString &fileName,
                             double paperWidth, double paperHeight,
                             Qt::Orientation paperOrientation) const;

    /** Interfaccia privata per stampa libretto/brogliaccio di tutte le categorie a corpo */
    bool printLSBillsMeasuresODT( int payToPrint, PaymentDataModel *payDataModel,
                                  AccountingPrinter::PrintOption prOption,
                                  AccountingPrinter::PrintLSOption prLSOption,
                                  AccountingPrinter::PrintPPUDescOption prPPDescOption,
                                  bool printAmounts,
                                  const QString &fileName,
                                  double paperWidth, double paperHeight,
                                  Qt::Orientation paperOrientation) const;

    /** Larghezza delle colonne della tabella nel caso di stampa di libretto opere a corpo */
    QVector<QTextLength> printAccountingLSColWidth(double tableWidth,
                                                   AccountingPrinter::PrintLSOption prLSOption,
                                                   Qt::Orientation paperOrientation,
                                                   bool printAmounts) const;
};

#endif // ACCOUNTINGPRINTER_H
