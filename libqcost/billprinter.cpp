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
#include "billprinter.h"

#include "bill.h"
#include "pricefieldmodel.h"

#include "odtwriter.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextCodec>
#include <QFileInfo>
#include <QFile>

class BillPrinterPrivate{
public:
    BillPrinterPrivate(Bill * b, PriceFieldModel * pfm, MathParser * prs):
        bill(b),
        priceFieldModel(pfm),
        parser(prs){
    }
    ~BillPrinterPrivate(){
    }

    Bill * bill;
    PriceFieldModel * priceFieldModel;
    MathParser * parser;
};

BillPrinter::BillPrinter( Bill * b, PriceFieldModel * pfm, MathParser * prs ):
    m_d( new BillPrinterPrivate(b, pfm, prs) ) {
}

BillPrinter::~BillPrinter() {
    delete m_d;
}

void BillPrinter::setBill(Bill *b) {
    m_d->bill = b;
}

bool BillPrinter::printODT( BillPrinter::PrintBillItemsOption prBillItemsOption,
                            BillPrinter::PrintOption prOption,
                            const QList<int> & fieldsToPrint,
                            const QString &fileName,
                            double paperWidth,
                            double paperHeight,
                            Qt::Orientation paperOrientation,
                            bool groupPrAm ) const {
    if( prOption == PrintBill ){
        return printBillODT( prBillItemsOption, fieldsToPrint, fileName, paperWidth, paperHeight, paperOrientation, groupPrAm );
    } else if( prOption == PrintSummary ){
        return printSummaryODT( prBillItemsOption, fieldsToPrint, fileName, paperWidth, paperHeight, paperOrientation, groupPrAm, false );
    } else if( prOption == PrintSummaryWithDetails ){
        return printSummaryODT( prBillItemsOption, fieldsToPrint, fileName, paperWidth, paperHeight, paperOrientation, groupPrAm, true );
    }
    return false;
}

bool BillPrinter::printAttributeODT( BillPrinter::PrintBillItemsOption prItemsOption,
                                     BillPrinter::AttributePrintOption prOption,
                                     const QList<int> &fieldsToPrint,
                                     const QList<Attribute *> &attrsToPrint,
                                     const QString &fileName,
                                     double paperWidth,
                                     double paperHeight,
                                     Qt::Orientation paperOrientation,
                                     bool groupPrAm) const {
    if( m_d->bill != nullptr ){
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
        cursor.insertText( m_d->bill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        if( fieldsToPrint.size() > 0 ){
            cursor.insertText(QObject::tr("Computo Metrico Estimativo") );
        } else {
            cursor.insertText(QObject::tr("Computo Metrico") );
        }

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*margin ) );
        // numero progressivo + codice + descrizione + unità di misura + quantità + [ prezzo campo, importo campo ]
        QVector<QTextLength> colWidths;
        if( paperOrientation == Qt::Horizontal ){
            if( fieldsToPrint.size() > 0 ){
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 70.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double usedWidth = 0.0;
                for( QVector<QTextLength>::iterator iter = colWidths.begin(); iter != colWidths.end(); ++iter ){
                    usedWidth += iter->rawValue();
                }
                double colEqualWidth = (tableWidth - usedWidth ) / (1 + 2*fieldsToPrint.size() );
                for( int i=0; i < (1 + 2*fieldsToPrint.size() ); ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            } else { // fieldsToPrint.size() == 0
                double descWidth = tableWidth - (30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            }
        } else { // pageOrientation == Qt::Vertical
            if( fieldsToPrint.size() > 0 ){
                double usedWidth = 0.0;
                if( fieldsToPrint.size() > 1  ){
                    colWidths << QTextLength( QTextLength::FixedLength, 20.0 )
                              << QTextLength( QTextLength::FixedLength, 45.0 )
                              << QTextLength( QTextLength::FixedLength, 15.0 );
                    for( QVector<QTextLength>::iterator iter = colWidths.begin(); iter != colWidths.end(); ++iter ){
                        usedWidth += iter->rawValue();
                    }
                } else { // fieldsToPrint.size() == 1
                    colWidths << QTextLength( QTextLength::FixedLength, 25.0 )
                              << QTextLength( QTextLength::FixedLength, 65.0 )
                              << QTextLength( QTextLength::FixedLength, 15.0 );
                    for( QVector<QTextLength>::iterator iter = colWidths.begin(); iter != colWidths.end(); ++iter ){
                        usedWidth += iter->rawValue();
                    }
                }
                double colEqualWidth = (tableWidth - usedWidth ) / (1 + 2*fieldsToPrint.size() );
                for( int i=0; i < (1 + 2*fieldsToPrint.size() ); ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            } else { // fieldsToPrint.size() == 0
                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            }
        }
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->bill->writeODTAttributeBillOnTable( &cursor, prOption, prItemsOption, fieldsToPrint, attrsToPrint, groupPrAm );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( margin, margin, margin, margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}

bool BillPrinter::printBillODT( PrintBillItemsOption prItemsOption,
                                const QList<int> &fieldsToPrint,
                                const QString &fileName,
                                double paperWidth, double paperHeight,
                                Qt::Orientation paperOrientation,
                                bool groupPrAm ) const {
    if( m_d->bill != nullptr ){
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
        cursor.insertText( m_d->bill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        if( fieldsToPrint.size() > 0 ){
            cursor.insertText(QObject::tr("Computo Metrico Estimativo") );
        } else {
            cursor.insertText(QObject::tr("Computo Metrico") );
        }

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*margin ) );
        // numero progressivo + codice + descrizione + unità di misura + quantità + [ prezzo campo, importo campo ]
        QVector<QTextLength> colWidths;

        // numero complessivo colonne
        int colCount = 5;
        for( int i=0; i < fieldsToPrint.size(); ++i ) {
            if( m_d->priceFieldModel->applyFormula(fieldsToPrint.at(i)) == PriceFieldModel::ToBillItems ) {
                colCount += 1;
            } else {
                colCount += 2;
            }
        }

        if( paperOrientation == Qt::Horizontal ){
            if( fieldsToPrint.size() > 0 ){
                colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 70.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double usedWidth = 0.0;
                for( QVector<QTextLength>::iterator iter = colWidths.begin(); iter != colWidths.end(); ++iter ){
                    usedWidth += iter->rawValue();
                }
                int colEqualWidthCount = colCount - colWidths.size();
                double colEqualWidth = (tableWidth - usedWidth ) / colEqualWidthCount;
                for( int i=0; i < colEqualWidthCount; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            } else { // fieldsToPrint.size() == 0
                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            }
        } else { // pageOrientation == Qt::Vertical
            if( fieldsToPrint.size() > 0 ){
                if( fieldsToPrint.size() > 1  ){
                    colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                              << QTextLength( QTextLength::FixedLength, 20.0 )
                              << QTextLength( QTextLength::FixedLength, 45.0 )
                              << QTextLength( QTextLength::FixedLength, 15.0 );
                } else { // fieldsToPrint.size() == 1
                    colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                              << QTextLength( QTextLength::FixedLength, 25.0 )
                              << QTextLength( QTextLength::FixedLength, 65.0 )
                              << QTextLength( QTextLength::FixedLength, 15.0 );
                }
                double usedWidth = 0.0;
                for( QVector<QTextLength>::iterator iter = colWidths.begin(); iter != colWidths.end(); ++iter ){
                    usedWidth += iter->rawValue();
                }
                int colEqualWidthCount = colCount - colWidths.size();
                double colEqualWidth = (tableWidth - usedWidth ) / colEqualWidthCount;
                for( int i=0; i < colEqualWidthCount; ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            } else { // fieldsToPrint.size() == 0
                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            }
        }
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->bill->writeODTBillOnTable( &cursor, prItemsOption, fieldsToPrint, groupPrAm );

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( paperWidth, paperHeight );
            writer.setMarginsMM( margin, margin, margin, margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}

bool BillPrinter::printSummaryODT( PrintBillItemsOption prBillItemsOption,
                                   const QList<int> &fieldsToPrint,
                                   const QString &fileName,
                                   double paperWidth, double paperHeight,
                                   Qt::Orientation paperOrientation,
                                   bool groupPrAm,
                                   bool writeDetails ) const {
    if( m_d->bill != nullptr ){
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
        cursor.insertText( m_d->bill->name() );

        cursor.insertBlock( headerBlockFormat );
        cursor.setBlockCharFormat( headerBlockCharFormat );

        if( fieldsToPrint.size() > 0 ){
            cursor.insertText(QObject::tr("Sommario Computo Metrico Estimativo") );
        } else {
            cursor.insertText(QObject::tr("Sommario Computo Metrico") );
        }

        cursor.insertBlock( parBlockFormat );

        QTextTableFormat tableFormat;
        tableFormat.setCellPadding(5);
        tableFormat.setHeaderRowCount(2);
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
        // tableFormat.setBorder(borderWidth);
        tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth-2.0*margin ) );
        // codice + descrizione + unità di misura + quantità + { [prezzo campo, importo campo] - [prezzo campo], [importo campo]}
        QVector<QTextLength> colWidths;
        if( paperOrientation == Qt::Horizontal ){
            if( fieldsToPrint.size() > 0 ){
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, 70.0 )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                double usedWidth =  0.0;
                for( QVector<QTextLength>::iterator iter = colWidths.begin(); iter != colWidths.end(); ++iter ){
                    usedWidth += iter->rawValue();
                }
                double colEqualWidth = (tableWidth - usedWidth ) / (1 + 2*fieldsToPrint.size() );
                for( int i=0; i < (1 + 2*fieldsToPrint.size() ); ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            } else { // fieldsToPrint.size() == 0
                if( fieldsToPrint.size() > 1  ){
                    double descWidth = tableWidth - (30.0 + 20.0 + 30.0);
                    colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                              << QTextLength( QTextLength::FixedLength, descWidth )
                              << QTextLength( QTextLength::FixedLength, 20.0 )
                              << QTextLength( QTextLength::FixedLength, 30.0 );
                }
            }
        } else { // pageOrientation == Qt::Vertical
            if( fieldsToPrint.size() > 0 ){
                double usedWidth = 0.0;
                if( fieldsToPrint.size() > 1  ){
                    colWidths << QTextLength( QTextLength::FixedLength, 20.0 )
                              << QTextLength( QTextLength::FixedLength, 45.0 )
                              << QTextLength( QTextLength::FixedLength, 15.0 );
                } else { // fieldsToPrint.size() == 1
                    colWidths << QTextLength( QTextLength::FixedLength, 25.0 )
                              << QTextLength( QTextLength::FixedLength, 65.0 )
                              << QTextLength( QTextLength::FixedLength, 15.0 );
                }
                for( QVector<QTextLength>::iterator iter = colWidths.begin(); iter != colWidths.end(); ++iter ){
                    usedWidth += iter->rawValue();
                }
                double colEqualWidth = (tableWidth - usedWidth ) / (1 + 2*fieldsToPrint.size() );
                for( int i=0; i < (1 + 2*fieldsToPrint.size() ); ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                }
            } else { // fieldsToPrint.size() == 0
                double descWidth = tableWidth - (30.0 + 20.0 + 30.0);
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 )
                          << QTextLength( QTextLength::FixedLength, 30.0 );
            }
        }
        tableFormat.setColumnWidthConstraints( colWidths );
        cursor.insertTable(1, colWidths.size(), tableFormat );

        m_d->bill->writeODTSummaryOnTable( &cursor, prBillItemsOption, fieldsToPrint, groupPrAm, writeDetails );

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
