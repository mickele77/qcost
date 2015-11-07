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
#include "accountingprinter.h"

#include "paymentdatamodel.h"
#include "paymentdata.h"
#include "accountingbill.h"
#include "accountingtambill.h"
#include "accountinglsbills.h"
#include "accountinglsbill.h"

#include "odtwriter.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextLength>
#include <QTextCodec>
#include <QFileInfo>
#include <QFile>
#include <QDate>

class AccountingPrinterPrivate{
public:
    AccountingPrinterPrivate(AccountingBill * b, MathParser * prs):
        accountingBill(b),
        accountingTAMBill(NULL),
        accountingLSBills(NULL),
        accountingLSBill(NULL),
        parser(prs){
    }
    AccountingPrinterPrivate(AccountingTAMBill * b, MathParser * prs):
        accountingBill(NULL),
        accountingTAMBill(b),
        accountingLSBills(NULL),
        accountingLSBill(NULL),
        parser(prs){
    }
    AccountingPrinterPrivate(AccountingLSBills * b, MathParser * prs):
        accountingBill(NULL),
        accountingTAMBill(NULL),
        accountingLSBills(b),
        accountingLSBill(NULL),
        parser(prs){
    }
    AccountingPrinterPrivate(AccountingLSBill * b, MathParser * prs):
        accountingBill(NULL),
        accountingTAMBill(NULL),
        accountingLSBills(NULL),
        accountingLSBill(b),
        parser(prs){
    }
    ~AccountingPrinterPrivate(){
    }

    AccountingBill * accountingBill;
    AccountingTAMBill * accountingTAMBill;
    AccountingLSBills * accountingLSBills;
    AccountingLSBill * accountingLSBill;
    MathParser * parser;

    static double margin;
};

double AccountingPrinterPrivate::margin = 10.0;

AccountingPrinter::AccountingPrinter(AccountingBill * b, MathParser * prs ):
    m_d( new AccountingPrinterPrivate(b, prs) ) {
}

AccountingPrinter::AccountingPrinter(AccountingTAMBill *b, MathParser *prs):
    m_d( new AccountingPrinterPrivate(b, prs) ) {
}

AccountingPrinter::AccountingPrinter(AccountingLSBills *b, MathParser *prs):
    m_d( new AccountingPrinterPrivate(b, prs) ){
}

AccountingPrinter::AccountingPrinter(AccountingLSBill *b, MathParser *prs):
    m_d( new AccountingPrinterPrivate(b, prs) ) {
}

AccountingPrinter::~AccountingPrinter() {
    delete m_d;
}

bool AccountingPrinter::printODT( int payToPrint, AccountingPrinter::PrintOption prOption,
                                  PrintAmountsOption prAmountsOption,
                                  AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                  const QString &fileName,
                                  double paperWidth,
                                  double paperHeight,
                                  Qt::Orientation paperOrientation ) const {
    if( paperOrientation == Qt::Horizontal ){
        if( paperHeight > paperWidth ){
            double com = paperHeight;
            paperHeight = paperWidth;
            paperWidth = com;
        }
    } else {
        if( paperHeight < paperWidth ){
            double com = paperHeight;
            paperHeight = paperWidth;
            paperWidth = com;
        }
    }

    if( (prOption == PrintMeasures) ||
            (prOption == PrintRawMeasures) ){
        if( m_d->accountingBill != NULL ){
            return printMeasuresODT( payToPrint, prOption, prAmountsOption, prPPUDescOption, fileName, paperWidth, paperHeight, paperOrientation );
        } else if( m_d->accountingTAMBill != NULL ){
            return printTAMMeasuresODT( payToPrint, prOption, prAmountsOption, prPPUDescOption, fileName, paperWidth, paperHeight, paperOrientation );
        }
    } else if( prOption == PrintAccounting ){
        return printAccountingODT( payToPrint, prPPUDescOption, fileName, paperWidth, paperHeight, paperOrientation );
    } else if( prOption == PrintPayment ){
        return printPaymentODT( payToPrint, prPPUDescOption, fileName, paperWidth, paperHeight, paperOrientation );
    } else if( prOption == PrintAccountingSummary ){
        return printSummaryODT( payToPrint, prAmountsOption, prPPUDescOption, fileName, paperWidth, paperHeight, paperOrientation, false );
    }
    return false;
}

bool AccountingPrinter::printAttributeODT( AccountingPrinter::AttributePrintOption prOption,
                                           AccountingPrinter::PrintAmountsOption prAmountsOption,
                                           AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                           const QList<Attribute *> &attrsToPrint,
                                           const QString &fileName,
                                           double paperWidth,
                                           double paperHeight,
                                           Qt::Orientation paperOrientation) const {
    if( m_d->accountingBill != NULL ){
        if( paperOrientation == Qt::Horizontal ){
            if( paperHeight > paperWidth ){
                double com = paperHeight;
                paperHeight = paperWidth;
                paperWidth = com;
            }
        } else {
            if( paperHeight < paperWidth ){
                double com = paperHeight;
                paperHeight = paperWidth;
                paperWidth = com;
            }
        }
        double margin = 10.0;
        double tableWidth = paperWidth - 2.0 * margin;

        QTextDocument doc;
        QTextCursor cursor(&doc);

        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat headerWithPBBlockFormat = headerBlockFormat;
        headerWithPBBlockFormat.setPageBreakPolicy( QTextFormat::PageBreak_AlwaysBefore );

        QTextBlockFormat parBlockFormat;

        cursor.setBlockFormat( headerWithPBBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        cursor.insertText( m_d->accountingBill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        cursor.insertText(QObject::trUtf8("Libretto delle Misure") );

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*margin ) );
        // numero progressivo + codice + descrizione + unità di misura + quantità + [ prezzo campo, importo campo ]
        QVector<QTextLength> colWidths;
        int dataCols = 1;
        if( prAmountsOption != PrintNoAmount ){
            dataCols = 3;
        }
        if( paperOrientation == Qt::Horizontal ){
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 70.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double usedWidth =  10.0 + 30.0 + 70.0 + 20.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        } else { // pageOrientation == Qt::Vertical
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                double usedWidth = 0.0;
                colWidths << QTextLength( QTextLength::FixedLength, 25.0 )
                          << QTextLength( QTextLength::FixedLength, 65.0 )
                          << QTextLength( QTextLength::FixedLength, 15.0 );
                usedWidth =  10.0 + 25.0 + 65.0 + 15.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        }
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->accountingBill->writeODTAttributeAccountingOnTable( &cursor, prOption, prAmountsOption, prPPUDescOption, attrsToPrint );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( margin, margin, margin, margin );
            writer.setPageOrientation( paperOrientation );
            return writer.writeAll();
        }
    } else if( m_d->accountingTAMBill != NULL ){
        if( paperOrientation == Qt::Horizontal ){
            if( paperHeight > paperWidth ){
                double com = paperHeight;
                paperHeight = paperWidth;
                paperWidth = com;
            }
        } else {
            if( paperHeight < paperWidth ){
                double com = paperHeight;
                paperHeight = paperWidth;
                paperWidth = com;
            }
        }
        double margin = 10.0;
        double tableWidth = paperWidth - 2.0 * margin;

        QTextDocument doc;
        QTextCursor cursor(&doc);

        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat headerWithPBBlockFormat = headerBlockFormat;
        headerWithPBBlockFormat.setPageBreakPolicy( QTextFormat::PageBreak_AlwaysBefore );

        QTextBlockFormat parBlockFormat;

        cursor.setBlockFormat( headerWithPBBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        cursor.insertText( m_d->accountingBill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        cursor.insertText(QObject::trUtf8("Lista Opere in economia") );

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*margin ) );
        // numero progressivo + codice + descrizione + unità di misura + quantità + [ prezzo campo, importo campo ]
        QVector<QTextLength> colWidths;
        int dataCols = 1;
        if( prAmountsOption != PrintNoAmount ){
            dataCols = 3;
        }
        if( paperOrientation == Qt::Horizontal ){
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 70.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double usedWidth =  10.0 + 30.0 + 70.0 + 20.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        } else { // pageOrientation == Qt::Vertical
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                double usedWidth = 0.0;
                colWidths << QTextLength( QTextLength::FixedLength, 25.0 )
                          << QTextLength( QTextLength::FixedLength, 65.0 )
                          << QTextLength( QTextLength::FixedLength, 15.0 );
                usedWidth =  10.0 + 25.0 + 65.0 + 15.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        }
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->accountingTAMBill->writeODTAttributeAccountingOnTable( &cursor, prOption, prAmountsOption, prPPUDescOption, attrsToPrint );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( margin, margin, margin, margin );
            writer.setPageOrientation( paperOrientation );
            return writer.writeAll();
        }
    }
    return false;
}

bool AccountingPrinter::printLSODT( int payToPrint,
                                    PaymentDataModel * payDataModel,
                                    AccountingPrinter::PrintOption prOption,
                                    AccountingPrinter::PrintLSOption prLSOption,
                                    AccountingPrinter::PrintPPUDescOption prPPDescOption,
                                    bool printAmounts,
                                    const QString &fileName,
                                    double paperWidth, double paperHeight,
                                    Qt::Orientation paperOrientation) const {
    if( paperOrientation == Qt::Horizontal ){
        if( paperHeight > paperWidth ){
            double com = paperHeight;
            paperHeight = paperWidth;
            paperWidth = com;
        }
    } else {
        if( paperHeight < paperWidth ){
            double com = paperHeight;
            paperHeight = paperWidth;
            paperWidth = com;
        }
    }

    if( m_d->accountingLSBill != NULL ){
        return printLSMeasuresODT( payToPrint, payDataModel,
                                   prOption, prLSOption, prPPDescOption, printAmounts,
                                   fileName, paperWidth, paperHeight, paperOrientation );
    }

    if( m_d->accountingLSBills != NULL ){
        return printLSBillsMeasuresODT( payToPrint, payDataModel,
                                        prOption, prLSOption, prPPDescOption, printAmounts,
                                        fileName, paperWidth, paperHeight, paperOrientation );
    }

    return false;
}

bool AccountingPrinter::printMeasuresODT( int payToPrint,
                                          AccountingPrinter::PrintOption prOption,
                                          AccountingPrinter::PrintAmountsOption prAmountsOption,
                                          AccountingPrinter::PrintPPUDescOption prPPDescOption,
                                          const QString &fileName,
                                          double paperWidth, double paperHeight,
                                          Qt::Orientation paperOrientation) const {

    if( m_d->accountingBill != NULL ){
        double tableWidth = paperWidth - 2.0 * AccountingPrinterPrivate::margin;

        // numero progressivo + data + codice + descrizione + unità di misura + quantità + [prezzo + importo]
        QVector<QTextLength> colWidths;
        int dataCols = 1;
        if( prAmountsOption != PrintNoAmount ){
            dataCols += 2;
        }
        if( paperOrientation == Qt::Horizontal ){
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (8.0 + 18.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                          << QTextLength( QTextLength::FixedLength, 18.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                double usedWidth =  8.0 + 18.0 + 30.0 + 60.0 + 20.0;
                colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                          << QTextLength( QTextLength::FixedLength, 18.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 60.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        } else { // pageOrientation == Qt::Vertical
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (8.0 + 18.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                          << QTextLength( QTextLength::FixedLength, 18.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                double usedWidth = 0.0;
                colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                          << QTextLength( QTextLength::FixedLength, 18.0 )
                          << QTextLength( QTextLength::FixedLength, 25.0 )
                          << QTextLength( QTextLength::FixedLength, 60.0 )
                          << QTextLength( QTextLength::FixedLength, 15.0 );
                usedWidth =  8.0 + 18.0 + 25.0 + 60.0 + 15.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i<dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        }

        QTextDocument doc;
        QTextCursor cursor(&doc);

        // creiamo i vari stili necessari
        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat headerWithPBBlockFormat = headerBlockFormat;
        headerWithPBBlockFormat.setPageBreakPolicy( QTextFormat::PageBreak_AlwaysBefore );

        QTextBlockFormat parBlockFormat;

        cursor.setBlockFormat( headerWithPBBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        cursor.insertText( m_d->accountingBill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        if( prOption == PrintRawMeasures ){
            cursor.insertText(QObject::trUtf8("Brogliaccio del Libretto delle misure") );
            cursor.insertBlock( parBlockFormat );
        } else if( prOption == PrintMeasures ){
            cursor.insertText(QObject::trUtf8("Libretto delle Misure") );
            cursor.insertBlock( parBlockFormat );
        }

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*AccountingPrinterPrivate::margin ) );
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->accountingBill->writeODTAccountingOnTable( &cursor, payToPrint, prAmountsOption, prPPDescOption,
                                                        false );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin,
                                 AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}

bool AccountingPrinter::printTAMMeasuresODT( int billToPrint,
                                             AccountingPrinter::PrintOption prOption,
                                             AccountingPrinter::PrintAmountsOption prAmountsOption,
                                             AccountingPrinter::PrintPPUDescOption prPPDescOption,
                                             const QString &fileName,
                                             double paperWidth, double paperHeight,
                                             Qt::Orientation paperOrientation ) const {
    if( m_d->accountingTAMBill != NULL ){
        double tableWidth = paperWidth - 2.0 * AccountingPrinterPrivate::margin;

        // numero progressivo + codice + descrizione + unità di misura + quantità + [ prezzo campo, importo campo ]
        QVector<QTextLength> colWidths;
        int dataCols = 1;
        if( prAmountsOption != PrintNoAmount ){
            dataCols += 2;
        }
        if( paperOrientation == Qt::Horizontal ){
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (8.0 + 18.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                          << QTextLength( QTextLength::FixedLength, 18.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                double usedWidth =  8.0 + 18.0 + 30.0 + 60.0 + 20.0;
                colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                          << QTextLength( QTextLength::FixedLength, 18.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 60.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        } else { // pageOrientation == Qt::Vertical
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (8.0 + 18.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                          << QTextLength( QTextLength::FixedLength, 18.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                double usedWidth = 0.0;
                colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                          << QTextLength( QTextLength::FixedLength, 18.0 )
                          << QTextLength( QTextLength::FixedLength, 25.0 )
                          << QTextLength( QTextLength::FixedLength, 60.0 )
                          << QTextLength( QTextLength::FixedLength, 15.0 );
                usedWidth =  8.0 + 18.0 + 25.0 + 60.0 + 15.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i<dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        }

        QTextDocument doc;
        QTextCursor cursor(&doc);

        // creiamo i vari stili necessari
        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat headerWithPBBlockFormat = headerBlockFormat;
        headerWithPBBlockFormat.setPageBreakPolicy( QTextFormat::PageBreak_AlwaysBefore );

        QTextBlockFormat parBlockFormat;


        cursor.setBlockFormat( headerWithPBBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        cursor.insertText( m_d->accountingTAMBill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        if( billToPrint < 0 ){
            if( prOption == PrintRawMeasures ){
                cursor.insertText(QObject::trUtf8("Brogliaccio delle liste in economia") );
            } else if( prOption == PrintAccounting ){
                cursor.insertText(QObject::trUtf8("Liste in Economia") );
            }
        }

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*AccountingPrinterPrivate::margin ) );
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->accountingTAMBill->writeODTAccountingOnTable( &cursor, billToPrint, prAmountsOption, prPPDescOption );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}

bool AccountingPrinter::printAccountingODT( int payToPrint,
                                            AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                            const QString &fileName,
                                            double paperWidth, double paperHeight,
                                            Qt::Orientation paperOrientation) const {
    // prAmountsOption != PrintNoAmount
    if( m_d->accountingBill != NULL ){
        double tableWidth = paperWidth - 2.0 * AccountingPrinterPrivate::margin;

        // numero progressivo + data + codice + descrizione + unità di misura + quantità + [prezzo + importo]
        QVector<QTextLength> colWidths;
        int dataCols = 3;
        if( paperOrientation == Qt::Horizontal ){
            double usedWidth =  8.0 + 18.0 + 30.0 + 60.0 + 20.0;
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                      << QTextLength( QTextLength::FixedLength, 18.0 )
                      << QTextLength( QTextLength::FixedLength, 30.0 )
                      << QTextLength( QTextLength::FixedLength, 60.0 )
                      << QTextLength( QTextLength::FixedLength, 20.0 );
            double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
            for( int i=0; i < dataCols; ++i ){
                colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
            }
        } else { // pageOrientation == Qt::Vertical
            double usedWidth = 0.0;
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                      << QTextLength( QTextLength::FixedLength, 18.0 )
                      << QTextLength( QTextLength::FixedLength, 25.0 )
                      << QTextLength( QTextLength::FixedLength, 60.0 )
                      << QTextLength( QTextLength::FixedLength, 15.0 );
            usedWidth =  8.0 + 18.0 + 25.0 + 60.0 + 15.0;
            double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
            for( int i=0; i<dataCols; ++i ){
                colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
            }
        }

        QTextDocument doc;
        QTextCursor cursor(&doc);

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*AccountingPrinterPrivate::margin ) );
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->accountingBill->writeODTAccountingOnTable( &cursor, payToPrint, AccountingPrinter::PrintAllAmounts, prPPUDescOption,
                                                        true );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin,
                                 AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}

QVector<QTextLength> AccountingPrinter::printAccountingLSColWidth( double tableWidth,
                                                                   AccountingPrinter::PrintLSOption prLSOption,
                                                                   Qt::Orientation paperOrientation,
                                                                   bool printAmounts) const {
    QVector<QTextLength> colWidths;
    int dataCols = 0;
    if( prLSOption == PrintLSProj ){
        // numero progressivo + codice + descrizione + unità di misura + quantità [+ prezzo + importo ]
        if( paperOrientation == Qt::Horizontal ){
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )   // num prog
                      << QTextLength( QTextLength::FixedLength, 30.0 )  // codice prezzo
                      << QTextLength( QTextLength::FixedLength, 20.0 ); // udm
        } else { // paperOrientation == Qt::Vertical
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )   // num prog
                      << QTextLength( QTextLength::FixedLength, 25.0 )  // codice prezzo
                      << QTextLength( QTextLength::FixedLength, 15.0 ); // udm
        }

        // calcoliamo il numero di colonne con dati
        dataCols += 1;   // quantita'
        if( printAmounts ){
            dataCols += 1; // prezzo
            dataCols += 1; // importo
        }
        double dataColWidth = 20.0; // larghezza della colonna con dati
        if( paperOrientation == Qt::Horizontal ){
            dataColWidth = 30.0;
        }

        // inseriamo le colonne con dati
        for( int i=0; i < dataCols; ++i ){
            colWidths << QTextLength( QTextLength::FixedLength, dataColWidth );
        }

        // inseriamo per differeza la colonna con la descrizione
        double usedWidth =  0.0;
        for( QVector<QTextLength>::iterator i=colWidths.begin(); i != colWidths.end(); ++i ){
            usedWidth += i->rawValue();
        }
        colWidths.insert( 2, QTextLength( QTextLength::FixedLength, tableWidth - usedWidth) );
    } else if( prLSOption == PrintLSAcc ){
        // numero progressivo + codice + descrizione + unità di misura + data cont. + quantità cont. [+ prezzo + importo cont.]
        if( paperOrientation == Qt::Horizontal ){
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )   // num prog
                      << QTextLength( QTextLength::FixedLength, 30.0 )  // codice prezzo
                      << QTextLength( QTextLength::FixedLength, 20.0 ); // udm
        } else { // paperOrientation == Qt::Vertical
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )   // num prog
                      << QTextLength( QTextLength::FixedLength, 25.0 )  // codice prezzo
                      << QTextLength( QTextLength::FixedLength, 15.0 ); // udm
        }

        // calcoliamo il numero di colonne con dati
        dataCols += 1;   // data
        dataCols += 1;   // quantita'
        if( printAmounts ){
            dataCols += 1; // prezzo
            dataCols += 1; // importo
        }
        double dataColWidth = 20.0; // larghezza della colonna con dati
        if( paperOrientation == Qt::Horizontal ){
            dataColWidth = 30.0;
        }

        // inseriamo le colonne con dati
        for( int i=0; i < dataCols; ++i ){
            colWidths << QTextLength( QTextLength::FixedLength, dataColWidth );
        }

        // inseriamo per differeza la colonna con la descrizione
        double usedWidth =  0.0;
        for( QVector<QTextLength>::iterator i=colWidths.begin(); i != colWidths.end(); ++i ){
            usedWidth += i->rawValue();
        }
        colWidths.insert( 2, QTextLength( QTextLength::FixedLength, tableWidth - usedWidth) );
    } else if( prLSOption == PrintLSProjAcc ){
        // numero progressivo + codice + descrizione + unità di misura [ + prezzo ] +
        // quantita' prog [+importo prog.] + data cont. + quantità cont. [+ importo cont.]
        if( paperOrientation == Qt::Horizontal ){
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )   // num prog
                      << QTextLength( QTextLength::FixedLength, 30.0 )  // codice prezzo
                      << QTextLength( QTextLength::FixedLength, 20.0 ); // udm
        } else { // paperOrientation == Qt::Vertical
            colWidths << QTextLength( QTextLength::FixedLength, 5.0 )   // num prog
                      << QTextLength( QTextLength::FixedLength, 20.0 )  // codice prezzo
                      << QTextLength( QTextLength::FixedLength, 12.0 ); // udm
        }


        // calcoliamo il numero di colonne con dati
        if( printAmounts ){
            dataCols += 1;  // prezzo
        }
        dataCols += 1;      // quantita' prog.
        if( printAmounts ){
            dataCols += 1;  // importo prog.
        }
        dataCols += 1;   // data cont.
        dataCols += 1;   // quantita' cont.
        if( printAmounts ){
            dataCols += 1; // importo cont.
        }
        double dataColWidth = 18.0;   // larghezza della colonna con dati per orientamento verticale
        if( paperOrientation == Qt::Horizontal ){
            dataColWidth = 30.0;      // larghezza della colonna con dati per orientamento orizzontale
        }

        // inseriamo le colonne con dati
        for( int i=0; i < dataCols; ++i ){
            colWidths << QTextLength( QTextLength::FixedLength, dataColWidth );
        }

        // inseriamo per differeza la colonna con la descrizione
        double usedWidth =  0.0;
        for( QVector<QTextLength>::iterator i=colWidths.begin(); i != colWidths.end(); ++i ){
            usedWidth += i->rawValue();
        }
        colWidths.insert( 2, QTextLength( QTextLength::FixedLength, tableWidth - usedWidth) );
    }

    return colWidths;
}

bool AccountingPrinter::printLSMeasuresODT( int payToPrint,
                                            PaymentDataModel * payDataModel,
                                            AccountingPrinter::PrintOption prOption,
                                            AccountingPrinter::PrintLSOption prLSOption,
                                            AccountingPrinter::PrintPPUDescOption prPPDescOption,
                                            bool printAmounts,
                                            const QString &fileName,
                                            double paperWidth, double paperHeight,
                                            Qt::Orientation paperOrientation) const {
    if( m_d->accountingLSBill != NULL ){
        double tableWidth = paperWidth - 2.0 * AccountingPrinterPrivate::margin;
        QVector<QTextLength> colWidths = printAccountingLSColWidth( tableWidth, prLSOption, paperOrientation, printAmounts );

        QTextDocument doc;
        QTextCursor cursor(&doc);

        // creiamo i vari stili necessari

        // stile del tiolo
        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        // stile del nome del s.a.l.
        QTextCharFormat subHeaderBlockCharFormat;
        subHeaderBlockCharFormat.setFontWeight( QFont::Bold );
        subHeaderBlockCharFormat.setFontItalic(true);
        QTextBlockFormat subHeaderBlockFormat;
        subHeaderBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat parBlockFormat;

        cursor.setBlockFormat( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        if( prOption == PrintRawMeasures ){
            cursor.insertText(QObject::trUtf8("Brogliaccio delle libretto delle misure - Opere a Corpo") );
        } else if( prOption == PrintMeasures ){
            cursor.insertText(QObject::trUtf8("Libretto delle misure - Opere a Corpo") );
        }

        QList<int> payToPrintList;
        if( payToPrint < 0 ){
            for( int i=0; i < payDataModel->paymentsCount(); ++i ){
                payToPrintList << i;
            }
        } else if( payToPrint < payDataModel->paymentsCount() ){
            payToPrintList << payToPrint;
        }

        for( int i=0; i < payToPrintList.size(); ++i ){
            cursor.insertBlock( subHeaderBlockFormat );
            cursor.setBlockCharFormat( subHeaderBlockCharFormat );
            cursor.insertText( payDataModel->paymentData(payToPrintList.at(i))->name() );

            cursor.insertBlock( parBlockFormat );

            QTextTableFormat tableFormat;
            tableFormat.setCellPadding(5);
            tableFormat.setHeaderRowCount(2);
            tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
            // tableFormat.setBorder(borderWidth);
            tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*AccountingPrinterPrivate::margin ) );
            tableFormat.setColumnWidthConstraints( colWidths );
            cursor.insertTable(1, colWidths.size(), tableFormat );

            m_d->accountingLSBill->writeODTAccountingOnTable( &cursor,
                                                              payDataModel->paymentData(payToPrintList.at(i))->dateBegin(), payDataModel->paymentData(payToPrintList.at(i))->dateEnd(),
                                                              prLSOption, prPPDescOption, printAmounts );

            cursor.movePosition( QTextCursor::End );
        }

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}

bool AccountingPrinter::printLSBillsMeasuresODT(int payToPrint, PaymentDataModel *payDataModel,
                                                AccountingPrinter::PrintOption prOption, AccountingPrinter::PrintLSOption prLSOption, AccountingPrinter::PrintPPUDescOption prPPDescOption, bool printAmounts,
                                                const QString &fileName, double paperWidth, double paperHeight, Qt::Orientation paperOrientation) const {
    if( m_d->accountingLSBills != NULL ){
        double tableWidth = paperWidth - 2.0 * AccountingPrinterPrivate::margin;
        QVector<QTextLength> colWidths = printAccountingLSColWidth( tableWidth, prLSOption, paperOrientation, printAmounts );

        QTextDocument doc;
        QTextCursor cursor(&doc);

        // creiamo i vari stili necessari

        // titolo del documento
        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        // stile del nome del s.a.l.
        QTextCharFormat subHeaderBlockCharFormat;
        subHeaderBlockCharFormat.setFontWeight( QFont::Bold );
        subHeaderBlockCharFormat.setFontItalic(true);
        QTextBlockFormat subHeaderBlockFormat;
        subHeaderBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat parBlockFormat;

        cursor.setBlockFormat( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        if( prOption == PrintRawMeasures ){
            cursor.insertText(QObject::trUtf8("Brogliaccio delle libretto delle misure - Opere a Corpo") );
        } else if( prOption == PrintAccounting ){
            cursor.insertText(QObject::trUtf8("Libretto delle misure - Opere a Corpo") );
        }

        cursor.insertBlock( parBlockFormat );

        QList<int> payToPrintList;
        if( payToPrint < 0 ){
            for( int i=0; i < payDataModel->paymentsCount(); ++i ){
                payToPrintList << i;
            }
        } else if( payToPrint < payDataModel->paymentsCount() ){
            payToPrintList << payToPrint;
        }

        for( int i=0; i < payToPrintList.size(); ++i ){
            cursor.insertBlock( subHeaderBlockFormat );
            cursor.setBlockCharFormat( subHeaderBlockCharFormat );
            cursor.insertText( payDataModel->paymentData(payToPrintList.at(i))->name() );

            cursor.insertBlock( parBlockFormat );

            QTextTableFormat tableFormat;
            tableFormat.setCellPadding(5);
            tableFormat.setHeaderRowCount(2);
            tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
            // tableFormat.setBorder(borderWidth);
            tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*AccountingPrinterPrivate::margin ) );
            tableFormat.setColumnWidthConstraints( colWidths );
            cursor.insertTable(1, colWidths.size(), tableFormat );

            for( int j = 0; j < m_d->accountingLSBills->billCount(); ++j ){
                m_d->accountingLSBills->bill(j)->writeODTAccountingOnTable( &cursor,
                                                                            payDataModel->paymentData(payToPrintList.at(i))->dateBegin(), payDataModel->paymentData(payToPrintList.at(i))->dateEnd(),
                                                                            prLSOption, prPPDescOption, printAmounts );
            }

            cursor.movePosition( QTextCursor::End );
        }

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}

bool AccountingPrinter::printSummaryODT( int payToPrint,
                                         PrintAmountsOption prAmountsOption,
                                         PrintPPUDescOption prPPUDescOption,
                                         const QString &fileName,
                                         double paperWidth, double paperHeight,
                                         Qt::Orientation paperOrientation,
                                         bool writeDetails ) const {
    if( m_d->accountingBill != NULL ){
        if( paperOrientation == Qt::Horizontal ){
            if( paperHeight > paperWidth ){
                double com = paperHeight;
                paperHeight = paperWidth;
                paperWidth = com;
            }
        } else {
            if( paperHeight < paperWidth ){
                double com = paperHeight;
                paperHeight = paperWidth;
                paperWidth = com;
            }
        }
        double margin = 10.0;
        double tableWidth = paperWidth - 2.0 * margin;

        QTextDocument doc;
        QTextCursor cursor(&doc);

        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat headerWithPBBlockFormat = headerBlockFormat;
        headerWithPBBlockFormat.setPageBreakPolicy( QTextFormat::PageBreak_AlwaysBefore );

        QTextBlockFormat parBlockFormat;

        cursor.setBlockFormat( headerWithPBBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        cursor.insertText( m_d->accountingBill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        cursor.insertText(QObject::trUtf8("Sommario Registro di contabilità") );

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*margin ) );
        // codice + descrizione + unità di misura + quantità + { [prezzo campo, importo campo] - [prezzo campo], [importo campo]}
        QVector<QTextLength> colWidths;
        int dataCols = 1;
        if( prAmountsOption != PrintNoAmount ){
            dataCols += 2;
        }
        if( paperOrientation == Qt::Horizontal ){
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - ( 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 70.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double usedWidth =  30.0 + 70.0 + 20.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        } else { // pageOrientation == Qt::Vertical
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                double usedWidth = 0.0;
                colWidths << QTextLength( QTextLength::FixedLength, 25.0 )
                          << QTextLength( QTextLength::FixedLength, 65.0 )
                          << QTextLength( QTextLength::FixedLength, 15.0 );
                usedWidth = 25.0 + 65.0 + 15.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        }
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->accountingBill->writeODTSummaryOnTable( &cursor, payToPrint, prAmountsOption, prPPUDescOption, writeDetails );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( margin, margin, margin, margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec( QTextCodec::codecForName("UTF-8") );
            return writer.writeAll();
        }
    } else if( m_d->accountingTAMBill != NULL ){
        if( paperOrientation == Qt::Horizontal ){
            if( paperHeight > paperWidth ){
                double com = paperHeight;
                paperHeight = paperWidth;
                paperWidth = com;
            }
        } else {
            if( paperHeight < paperWidth ){
                double com = paperHeight;
                paperHeight = paperWidth;
                paperWidth = com;
            }
        }
        double margin = 10.0;
        double tableWidth = paperWidth - 2.0 * margin;

        QTextDocument doc;
        QTextCursor cursor(&doc);

        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat headerWithPBBlockFormat = headerBlockFormat;
        headerWithPBBlockFormat.setPageBreakPolicy( QTextFormat::PageBreak_AlwaysBefore );

        QTextBlockFormat parBlockFormat;

        cursor.setBlockFormat( headerWithPBBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        cursor.insertText( m_d->accountingBill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        cursor.insertText(QObject::trUtf8("Sommario Registro di contabilità") );

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*margin ) );
        // codice + descrizione + unità di misura + quantità + { [prezzo campo, importo campo] - [prezzo campo], [importo campo]}
        QVector<QTextLength> colWidths;
        int dataCols = 1;
        if( prAmountsOption != PrintNoAmount ){
            dataCols += 2;
        }
        if( paperOrientation == Qt::Horizontal ){
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - ( 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            } else {
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 70.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double usedWidth =  30.0 + 70.0 + 20.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        } else { // pageOrientation == Qt::Vertical
            if( prAmountsOption == PrintNoAmount ){
                double descWidth = tableWidth - (30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );

            } else {
                double usedWidth = 0.0;
                colWidths << QTextLength( QTextLength::FixedLength, 25.0 )
                          << QTextLength( QTextLength::FixedLength, 65.0 )
                          << QTextLength( QTextLength::FixedLength, 15.0 );
                usedWidth = 25.0 + 65.0 + 15.0;
                double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
                for( int i=0; i < dataCols; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            }
        }
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->accountingTAMBill->writeODTSummaryOnTable( &cursor, payToPrint, prAmountsOption, prPPUDescOption, writeDetails );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( margin, margin, margin, margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec( QTextCodec::codecForName("UTF-8") );
            return writer.writeAll();
        }
    }
    return false;
}

bool AccountingPrinter::printPaymentODT( int payToPrint,
                                         AccountingPrinter::PrintPPUDescOption prPPUDescOption,
                                         const QString &fileName,
                                         int paperWidth, int paperHeight,
                                         Qt::Orientation paperOrientation ) const{
    if( m_d->accountingBill != NULL ){
        // se payToPrint è negativo, stampa l'ultimo SAL
        if( payToPrint < 0 || payToPrint > (m_d->accountingBill->paymentCount()-1) ){
            payToPrint = m_d->accountingBill->paymentCount() - 1;
        }

        double tableWidth = paperWidth - 2.0 * AccountingPrinterPrivate::margin;

        // numero progressivo + codice + descrizione + unità di misura + quantità + prezzo + importo
        QVector<QTextLength> colWidths;
        int dataCols = 3;

        if( paperOrientation == Qt::Horizontal ){
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                      << QTextLength( QTextLength::FixedLength, 30.0 )
                      << QTextLength( QTextLength::FixedLength, 60.0 )
                      << QTextLength( QTextLength::FixedLength, 20.0 );
            double usedWidth =  0.0;
            for( QVector<QTextLength>::iterator i = colWidths.begin(); i!= colWidths.end(); ++i ){
                usedWidth += (*i).rawValue();
            }
            double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
            for( int i=0; i < dataCols; ++i ){
                colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
            }
        } else { // pageOrientation == Qt::Vertical
            colWidths << QTextLength( QTextLength::FixedLength, 8.0 )
                      << QTextLength( QTextLength::FixedLength, 25.0 )
                      << QTextLength( QTextLength::FixedLength, 60.0 )
                      << QTextLength( QTextLength::FixedLength, 15.0 );
            double usedWidth =  0.0;
            for( QVector<QTextLength>::iterator i = colWidths.begin(); i!= colWidths.end(); ++i ){
                usedWidth += (*i).rawValue();
            }
            double colEqualWidth = (tableWidth - usedWidth ) / dataCols;
            for( int i=0; i<dataCols; ++i ){
                colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
            }
        }

        QTextDocument doc;
        QTextCursor cursor(&doc);

        // creiamo i vari stili necessari
        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextCharFormat subHeaderBlockCharFormat;
        subHeaderBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat subHeaderBlockFormat;
        subHeaderBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat headerWithPBBlockFormat = headerBlockFormat;
        headerWithPBBlockFormat.setPageBreakPolicy( QTextFormat::PageBreak_AlwaysBefore );

        QTextBlockFormat parBlockFormat;

        cursor.setBlockFormat( headerWithPBBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );
        cursor.insertText( m_d->accountingBill->payment(payToPrint)->title() );
        if( m_d->accountingBill->payment(payToPrint) != NULL ){
            cursor.insertBlock( subHeaderBlockFormat );
            cursor.setBlockCharFormat( subHeaderBlockCharFormat );
            cursor.insertText( QObject::trUtf8("Opere eseguite a tutto il %1").arg(m_d->accountingBill->payment(payToPrint)->dateEndStr() ) );
        }

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*AccountingPrinterPrivate::margin ) );
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->accountingBill->writeODTPaymentOnTable( &cursor, payToPrint, prPPUDescOption );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin,
                                 AccountingPrinterPrivate::margin, AccountingPrinterPrivate::margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}
