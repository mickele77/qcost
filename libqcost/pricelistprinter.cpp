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
#include "pricelistprinter.h"

#include "billprinter.h"
#include "bill.h"
#include "pricelist.h"
#include "priceitem.h"
#include "unitmeasure.h"
#include "odtwriter.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QFile>
#include <QFileInfo>

class PriceListPrinterPrivate{
public:
    PriceListPrinterPrivate(PriceList * b):
        priceList(b) {
    };
    ~PriceListPrinterPrivate(){
    };

    PriceList * priceList;
};

PriceListPrinter::PriceListPrinter( PriceList * b ):
    m_d( new PriceListPrinterPrivate(b) ) {
}

PriceListPrinter::~PriceListPrinter() {
    delete m_d;
}

void PriceListPrinter::setPriceList(PriceList *b) {
    m_d->priceList = b;
}

#include "billprinter.h"
#include "qtextformatuserdefined.h"

bool PriceListPrinter::printODT( PriceListPrinter::PrintPriceItemsOption printOption,
                                 const QList<int> &fieldsToPrint,
                                 int priceDataSetToPrintInput,
                                 bool printPriceList,
                                 bool printPriceAP,
                                 bool APgroupPrAm,
                                 const QString &fileName,
                                 double pageWidth,
                                 double pageHeight,
                                 Qt::Orientation paperOrientation) {
    double borderWidth = 1.0f;
    if( m_d->priceList ){
        int priceDataSetToPrint = 0;
        // controlliamo se il valore di input è corretto
        if( priceDataSetToPrintInput >= 0 && priceDataSetToPrintInput < m_d->priceList->priceDataSetCount() ){
            priceDataSetToPrint = priceDataSetToPrintInput;
        }

        QTextDocument doc;
        QTextCursor cursor(&doc);

        if( paperOrientation == Qt::Horizontal ){
            if( pageHeight > pageWidth ){
                double com = pageHeight;
                pageHeight = pageWidth;
                pageWidth = com;
            }
        } else {
            if( pageHeight < pageWidth ){
                double com = pageHeight;
                pageHeight = pageWidth;
                pageWidth = com;
            }
        }
        double margin = 10.0;
        double tableWidth =  pageWidth - 2.0 * margin;

        QTextCharFormat headerBlockCharFormat;
        headerBlockCharFormat.setFontCapitalization( QFont::AllUppercase );
        headerBlockCharFormat.setFontWeight( QFont::Bold );

        QTextBlockFormat headerBlockFormat;
        headerBlockFormat.setAlignment( Qt::AlignHCenter );

        QTextBlockFormat headerWithPBBlockFormat = headerBlockFormat;
        headerWithPBBlockFormat.setPageBreakPolicy( QTextFormat::PageBreak_AlwaysBefore );

        QTextBlockFormat parBlockFormat;

        if( printPriceList ){
            cursor.setBlockFormat( headerWithPBBlockFormat );
            cursor.setBlockCharFormat( headerBlockCharFormat );
            cursor.insertText( m_d->priceList->name() );

            cursor.insertBlock( headerBlockFormat );
            cursor.setBlockCharFormat( headerBlockCharFormat );
            cursor.insertText(QObject::trUtf8("Elenco Prezzi") );

            cursor.insertBlock( parBlockFormat );

            QTextTableFormat tableFormat;
            tableFormat.setCellPadding(5);
            tableFormat.setHeaderRowCount(2);
            tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
            tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth ) );
            QVector<QTextLength> colWidths;
            if( paperOrientation == Qt::Horizontal ){
                double descColWidth = tableWidth - ( 30.0 + 20.0 + 35.0 * fieldsToPrint.size() );
                colWidths << QTextLength( QTextLength::FixedLength, 30.0 )
                          << QTextLength( QTextLength::FixedLength, descColWidth )
                          << QTextLength( QTextLength::FixedLength, 20.0 );
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, 35.0 );
                }
            } else {
                double descColWidth = tableWidth - ( 25.0 + 15.0 + 30.0 * fieldsToPrint.size() );
                colWidths << QTextLength( QTextLength::FixedLength, 25.0 )
                          << QTextLength( QTextLength::FixedLength, descColWidth )
                          << QTextLength( QTextLength::FixedLength, 15.0 );
                for( int i=0; i < fieldsToPrint.size(); ++i ){
                    colWidths << QTextLength( QTextLength::FixedLength, 30.0 );
                }
            }
            tableFormat.setColumnWidthConstraints( colWidths );
            tableFormat.setHeaderRowCount( 2 );
            cursor.insertTable(1, colWidths.size(), tableFormat);

            m_d->priceList->writeODTOnTable( &cursor, printOption, fieldsToPrint, priceDataSetToPrint );

            cursor.movePosition( QTextCursor::End );
        }

        if( printPriceAP ){
            bool firstAP=true;

            QList<PriceItem *> priceItemList = m_d->priceList->priceItemList();
            for( int i=0; i < priceItemList.size(); ++i ){
                if( (!priceItemList.at(i)->hasChildren()) && (priceItemList.at(i)->associateAP(priceDataSetToPrint)) ){
                    if( firstAP ){
                        if( printPriceList ){
                            // abbiamo stampato già l'elenco prezzi
                            cursor.insertBlock( headerWithPBBlockFormat );
                        } else { //  printData == DataAP
                            // non abbiamo stampato l'elenco prezzi
                            cursor.setBlockFormat( headerWithPBBlockFormat );
                        }
                        cursor.setBlockCharFormat( headerBlockCharFormat );
                        cursor.insertText( m_d->priceList->name() );

                        cursor.insertBlock( headerBlockFormat );
                        cursor.setBlockCharFormat( headerBlockCharFormat );
                        cursor.insertText(QObject::trUtf8("Analisi Prezzi") );

                        cursor.insertBlock( parBlockFormat );

                        firstAP = false;
                    } else {
                        cursor.insertBlock( headerWithPBBlockFormat );
                        cursor.insertText( QString() );
                        cursor.insertBlock( parBlockFormat );
                    }

                    QTextTableCellFormat topLeftFormat;
                    topLeftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
                    topLeftFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
                    topLeftFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
                    topLeftFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
                    QTextTableCellFormat topRightFormat;
                    topRightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
                    topRightFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );
                    topRightFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
                    topRightFormat.setProperty( QTextFormatUserDefined::TableCellBorderTopWidth, QVariant(borderWidth) );
                    QTextTableCellFormat bottomFormat;
                    bottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
                    bottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderLeftWidth, QVariant(borderWidth) );
                    bottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
                    bottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderRightWidth, QVariant(borderWidth) );
                    bottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomStyle, QVariant(QTextFrameFormat::BorderStyle_Solid) );
                    bottomFormat.setProperty( QTextFormatUserDefined::TableCellBorderBottomWidth, QVariant(borderWidth) );

                    // tabella con informazioni generali sul prezzo
                    // descrizione, codice, etc
                    QTextTableFormat tableFormat;
                    tableFormat.setCellPadding(5);
                    tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
                    tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth ) );
                    QVector<QTextLength> colWidths;
                    colWidths << QTextLength( QTextLength::FixedLength, 30 )
                              << QTextLength( QTextLength::FixedLength, pageWidth-2.0*margin - 30 );
                    tableFormat.setColumnWidthConstraints( colWidths );
                    QTextTable * table = cursor.insertTable(1, colWidths.size(), tableFormat);

                    table->cellAt( cursor ).setFormat( topLeftFormat );
                    cursor.insertText( priceItemList.at(i)->codeFull() );

                    cursor.movePosition(QTextCursor::NextCell);
                    table->cellAt( cursor ).setFormat( topRightFormat );
                    cursor.insertText( priceItemList.at(i)->shortDescriptionFull() );

                    table->appendRows(1);
                    table->mergeCells( 1, 0, 1, 2 );
                    cursor.movePosition(QTextCursor::PreviousRow );
                    cursor.movePosition(QTextCursor::NextCell );
                    table->cellAt( cursor ).setFormat( bottomFormat );
                    cursor.insertText( priceItemList.at(i)->longDescriptionFull() );

                    // cursor.movePosition( QTextCursor::End );

                    table->appendRows(1);
                    cursor.movePosition(QTextCursor::PreviousRow );
                    cursor.movePosition(QTextCursor::NextCell );
                    table->cellAt( cursor ).setFormat( topLeftFormat );
                    cursor.insertText( QObject::trUtf8("Unità di Misura") );

                    cursor.movePosition(QTextCursor::NextCell);
                    table->cellAt( cursor ).setFormat( topRightFormat );
                    cursor.insertText( priceItemList.at(i)->unitMeasure()->tag() );

                    // tabella con l'analisi prezzi vera e propria
                    tableFormat.setCellPadding(5);
                    tableFormat.setHeaderRowCount(2);
                    tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_Solid);
                    tableFormat.setWidth( QTextLength( QTextLength::FixedLength, tableWidth ) );
                    colWidths.clear();
                    if( paperOrientation == Qt::Horizontal ){
                        if( fieldsToPrint.size() > 0 ){
                            colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                                      << QTextLength( QTextLength::FixedLength, 30.0 )
                                      << QTextLength( QTextLength::FixedLength, 70.0 )
                                      << QTextLength( QTextLength::FixedLength, 20.0 );
                            double usedWidth =  10.0 + 30.0 + 70.0 + 20.0;
                            double colEqualWidth = (tableWidth - usedWidth ) / (1 + 2*fieldsToPrint.size() );
                            for( int i=0; i < (1 + 2*fieldsToPrint.size() ); ++i ){
                                colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                            }
                        } else { // fieldsToPrint.size() == 0
                            if( fieldsToPrint.size() > 1  ){
                                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                                colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                                          << QTextLength( QTextLength::FixedLength, 30.0 )
                                          << QTextLength( QTextLength::FixedLength, descWidth )
                                          << QTextLength( QTextLength::FixedLength, 20.0 )
                                          << QTextLength( QTextLength::FixedLength, 30.0 );
                            }
                        }
                    } else {
                        if( fieldsToPrint.size() > 0 ){
                            double usedWidth = 0.0;
                            if( fieldsToPrint.size() > 1  ){
                                colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                                          << QTextLength( QTextLength::FixedLength, 20.0 )
                                          << QTextLength( QTextLength::FixedLength, 45.0 )
                                          << QTextLength( QTextLength::FixedLength, 15.0 );
                                usedWidth =  10.0 + 20.0 + 45.0 + 15.0;
                            } else { // fieldsToPrint.size() == 1
                                colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                                          << QTextLength( QTextLength::FixedLength, 25.0 )
                                          << QTextLength( QTextLength::FixedLength, 65.0 )
                                          << QTextLength( QTextLength::FixedLength, 15.0 );
                                usedWidth =  10.0 + 25.0 + 65.0 + 15.0;
                            }
                            double colEqualWidth = (tableWidth - usedWidth ) / (1 + 2*fieldsToPrint.size() );
                            for( int i=0; i < (1 + 2*fieldsToPrint.size() ); ++i ){
                                colWidths << QTextLength( QTextLength::FixedLength, colEqualWidth );
                            }
                        } else { // fieldsToPrint.size() == 0
                            if( fieldsToPrint.size() > 1  ){
                                double descWidth = tableWidth - (10.0 + 30.0 + 20.0 + 30.0);
                                colWidths << QTextLength( QTextLength::FixedLength, 10.0 )
                                          << QTextLength( QTextLength::FixedLength, 30.0 )
                                          << QTextLength( QTextLength::FixedLength, descWidth )
                                          << QTextLength( QTextLength::FixedLength, 20.0 )
                                          << QTextLength( QTextLength::FixedLength, 30.0 );
                            }
                        }
                    }
                    tableFormat.setColumnWidthConstraints( colWidths );
                    tableFormat.setHeaderRowCount( 2 );
                    cursor.insertTable(1, colWidths.size(), tableFormat);

                    BillPrinter::PrintBillItemsOption billPrItemsOption = BillPrinter::PrintShortDesc;
                    if( printOption == PriceListPrinter::PrintShortDesc ){
                        billPrItemsOption = BillPrinter::PrintShortDesc;
                    } else if( printOption == PriceListPrinter::PrintLongDesc ){
                        billPrItemsOption = BillPrinter::PrintLongDesc;
                    } else if( printOption == PriceListPrinter::PrintShortLongDesc ){
                        billPrItemsOption = BillPrinter::PrintShortLongDesc;
                    } else if( printOption == PriceListPrinter::PrintShortLongDescOpt ){
                        billPrItemsOption = BillPrinter::PrintShortLongDescOpt;
                    }
                    priceItemList.at(i)->associatedAP(priceDataSetToPrint)->writeODTBillOnTable( &cursor, billPrItemsOption, fieldsToPrint, APgroupPrAm, priceItemList.at(i)->unitMeasure()->tag() );

                    cursor.movePosition( QTextCursor::End );
                }

            }
        }

        QFile *file = new QFile(fileName);
        QString suf = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
        if (suf == "odf" || suf == "opendocumentformat" || suf == "odt") {
            OdtWriter writer(doc, file);
            writer.setPageSizeMM( pageWidth, pageHeight );
            writer.setMarginsMM( margin, margin, margin, margin );
            writer.setPageOrientation( paperOrientation );
            // writer.setCodec(codec);
            return writer.writeAll();
        }
    }
    return false;
}
