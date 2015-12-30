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
#include "attributesgui.h"

#include "ui_billattributesgui.h"

#include "accountingattributeprintergui.h"
#include "billattributeprintergui.h"
#include "bill.h"
#include "accountingbill.h"
#include "attributesmodel.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class AttributesGUIPrivate{
public:
    AttributesGUIPrivate(PriceFieldModel * pfm, MathParser * prs, QString * wpf):
        ui(new Ui::BillAttributesGUI() ),
        bill(NULL),
        accountingBill(NULL),
        parser(prs),
        wordProcessorFile(wpf),
        priceFieldModel(pfm){
    }
    Ui::BillAttributesGUI * ui;
    Bill * bill;
    AccountingBill * accountingBill;
    MathParser * parser;
    QString * wordProcessorFile;
    PriceFieldModel * priceFieldModel;
};

AttributesGUI::AttributesGUI(PriceFieldModel * pfm, MathParser * prs, Bill * b, QString * wordProcessorFile, QWidget *parent) :
    QWidget(parent),
    m_d( new AttributesGUIPrivate( pfm, prs, wordProcessorFile ) ) {
    init();
    setBill( b );
}

AttributesGUI::AttributesGUI(PriceFieldModel * pfm, MathParser * prs, AccountingBill * b, QString * wordProcessorFile, QWidget *parent) :
    QWidget(parent),
    m_d( new AttributesGUIPrivate( pfm, prs, wordProcessorFile ) ) {
    init();
    setBill( b );
}

void AttributesGUI::init() {
    m_d->ui->setupUi( this );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AttributesGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AttributesGUI::removeAttribute );
    connect( m_d->ui->printAttributeBillPushButton, &QPushButton::clicked, this, &AttributesGUI::printAttributeBillODT );

    m_d->ui->attributesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->attributesTableView, &QTableView::customContextMenuRequested, this, &AttributesGUI::attributesTableViewCustomMenuRequested );
}

AttributesGUI::~AttributesGUI(){
    delete m_d;
}

void AttributesGUI::setBill(Bill *b) {
    if( m_d->bill != b ){
        if( m_d->bill != NULL ){
            disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &AttributesGUI::setBillNULL );
            m_d->bill = NULL;
        }
        if( m_d->accountingBill != NULL ){
            disconnect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &AttributesGUI::setBillNULL );
            m_d->accountingBill = NULL;
        }
        m_d->ui->attributesTableView->setModel( NULL );

        m_d->bill = b;

        if( m_d->bill != NULL ){
            m_d->ui->attributesTableView->setModel( m_d->bill->attributesModel() );
            connect( m_d->bill, &Bill::aboutToBeDeleted, this, &AttributesGUI::setBillNULL );
        }
    }
}

void AttributesGUI::setBill( AccountingBill *b ) {
    if( m_d->accountingBill != b || m_d->bill != NULL ){
        if( m_d->bill != NULL ){
            disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &AttributesGUI::setBillNULL );
            m_d->bill = NULL;
        }
        if( m_d->accountingBill != NULL ){
            disconnect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &AttributesGUI::setBillNULL );
            m_d->accountingBill = NULL;
        }
        m_d->ui->attributesTableView->setModel( NULL );

        m_d->accountingBill = b;

        if( m_d->accountingBill != NULL ){
            m_d->ui->attributesTableView->setModel( m_d->accountingBill->attributesModel() );
            connect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &AttributesGUI::setBillNULL );
        }
    }
}

void AttributesGUI::setBillNULL(){
    if( m_d->bill != NULL ){
        disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &AttributesGUI::setBillNULL );
        m_d->bill = NULL;
    }
    if( m_d->accountingBill != NULL ){
        disconnect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &AttributesGUI::setBillNULL );
        m_d->accountingBill = NULL;
    }
}

void AttributesGUI::addAttribute(){
    if( m_d->bill != NULL ){
        if( m_d->ui->attributesTableView->selectionModel() ){
            QModelIndexList selectedIndexes = m_d->ui->attributesTableView->selectionModel()->selectedIndexes();
            int count = 1;
            int row = 0;
            if( selectedIndexes.size() > 0 ){
                QList<int> selectedRows;
                for( QModelIndexList::iterator i = selectedIndexes.begin(); i != selectedIndexes.end(); ++ i ){
                    if( !selectedRows.contains((*i).row()) ){
                        selectedRows.append( (*i).row() );
                    }
                }
                qSort( selectedRows );
                row = selectedRows.last() + 1;
                count = selectedRows.size();
            }
            m_d->bill->attributesModel()->insertRows( row, count );
        }
    } else if( m_d->accountingBill != NULL ){
        if( m_d->ui->attributesTableView->selectionModel() ){
            QModelIndexList selectedIndexes = m_d->ui->attributesTableView->selectionModel()->selectedIndexes();
            int count = 1;
            int row = 0;
            if( selectedIndexes.size() > 0 ){
                QList<int> selectedRows;
                for( QModelIndexList::iterator i = selectedIndexes.begin(); i != selectedIndexes.end(); ++ i ){
                    if( !selectedRows.contains((*i).row()) ){
                        selectedRows.append( (*i).row() );
                    }
                }
                qSort( selectedRows );
                row = selectedRows.last() + 1;
                count = selectedRows.size();
            }
            m_d->accountingBill->attributesModel()->insertRows( row, count );
        }
    }
}

void AttributesGUI::removeAttribute(){
    if( m_d->bill != NULL ){
        if( m_d->ui->attributesTableView->selectionModel() ){
            QModelIndexList selectedRows = m_d->ui->attributesTableView->selectionModel()->selectedRows();
            int count = selectedRows.size();
            if( count > 0 ){
                int row = selectedRows.at(0).row();
                for( int i=1; i<selectedRows.size(); ++i){
                    if( selectedRows.at(i).row() < row ){
                        row = selectedRows.at(i).row();
                    }
                }
                m_d->bill->attributesModel()->removeRows( row, count );
            }
        }
    } else if( m_d->accountingBill != NULL ){
        if( m_d->ui->attributesTableView->selectionModel() ){
            QModelIndexList selectedRows = m_d->ui->attributesTableView->selectionModel()->selectedRows();
            int count = selectedRows.size();
            if( count > 0 ){
                int row = selectedRows.at(0).row();
                for( int i=1; i<selectedRows.size(); ++i){
                    if( selectedRows.at(i).row() < row ){
                        row = selectedRows.at(i).row();
                    }
                }
                m_d->accountingBill->attributesModel()->removeRows( row, count );
            }
        }
    }
}

bool AttributesGUI::printAttributeBillODT(){
    if( m_d->bill != NULL ){
        BillPrinter::PrintBillItemsOption prBillItemOption;
        BillPrinter::AttributePrintOption prOption;
        QList<int> prFields;
        QList<Attribute *> prAttrs;
        double paperWidth = 210.0, paperHeight = 297.0;
        Qt::Orientation paperOrientation;
        bool groupPrAm = false;
        BillAttributePrinterGUI printGUI( &prBillItemOption,
                                          &prOption,
                                          &prFields,
                                          &prAttrs,
                                          &paperWidth, &paperHeight,
                                          &paperOrientation,
                                          &groupPrAm,
                                          m_d->priceFieldModel,
                                          m_d->bill->attributesModel(),
                                          this );
        if( printGUI.exec() == QDialog::Accepted ){
            QString fileName = QFileDialog::getSaveFileName( this,
                                                             trUtf8("Stampa Computo Metrico Estimativo"), ".",
                                                             trUtf8("Documento di testo (*.odt)"));

            if (!fileName.isEmpty()){
                QString suf = fileName.split(".").last().toLower();
                if( suf != "odt"){
                    fileName.append( ".odt" );
                }
                BillPrinter printer( m_d->bill, m_d->priceFieldModel, m_d->parser );
                bool ret = printer.printAttributeODT( prBillItemOption, prOption, prFields, prAttrs, fileName, paperWidth, paperHeight, paperOrientation, groupPrAm );
                if( m_d->wordProcessorFile != NULL ){
                    if( !m_d->wordProcessorFile->isEmpty() ){
                        if( QFileInfo(*(m_d->wordProcessorFile)).exists() ){
                            QStringList args;
                            args << fileName;
                            QProcess *proc = new QProcess(this);
                            proc->start(*(m_d->wordProcessorFile), args);
                        }
                    }
                }
                return ret;
            }
        }
    } else if( m_d->accountingBill != NULL ){
        AccountingPrinter::PrintPPUDescOption prAccountingMeasureOption;
        AccountingPrinter::PrintAmountsOption prAmountsOption;
        AccountingPrinter::AttributePrintOption prOption;
        QList<Attribute *> prAttrs;
        double paperWidth = 210.0, paperHeight = 297.0;
        Qt::Orientation paperOrientation;
        AccountingAttributePrinterGUI printGUI( &prAccountingMeasureOption,
                                                &prAmountsOption,
                                                &prOption,
                                                &prAttrs,
                                                &paperWidth, &paperHeight,
                                                &paperOrientation,
                                                m_d->accountingBill->attributesModel(),
                                                this );
        if( printGUI.exec() == QDialog::Accepted ){
            QString fileName = QFileDialog::getSaveFileName( this,
                                                             trUtf8("Stampa Computo Metrico Estimativo"), ".",
                                                             trUtf8("Documento di testo (*.odt)"));

            if (!fileName.isEmpty()){
                QString suf = fileName.split(".").last().toLower();
                if( suf != "odt"){
                    fileName.append( ".odt" );
                }
                AccountingPrinter printer( m_d->accountingBill, m_d->parser );
                bool ret = printer.printAttributeODT( prOption, prAmountsOption, prAccountingMeasureOption, prAttrs, fileName, paperWidth, paperHeight, paperOrientation );
                if( m_d->wordProcessorFile != NULL ){
                    if( !m_d->wordProcessorFile->isEmpty() ){
                        if( QFileInfo(*(m_d->wordProcessorFile)).exists() ){
                            QStringList args;
                            args << fileName;
                            QProcess *proc = new QProcess(this);
                            proc->start(*(m_d->wordProcessorFile), args);
                        }
                    }
                }
                return ret;
            }
        }
    }

    return false;
}

void AttributesGUI::resizeAttributeColsToContents(){
    m_d->ui->attributesTableView->resizeColumnsToContents();
}

void AttributesGUI::attributesTableViewCustomMenuRequested(QPoint pos){
    QMenu *menu=new QMenu(this);
    QAction * addAction = new QAction( trUtf8("Aggiungi"), this);
    connect( addAction, &QAction::triggered, this, &AttributesGUI::addAttribute );
    menu->addAction( addAction );
    QAction * delAction = new QAction( trUtf8("Elimina"), this);
    connect( delAction, &QAction::triggered, this, &AttributesGUI::removeAttribute );
    menu->addAction( delAction );
    menu->addSeparator();

    menu->addSeparator();
    QAction * printAttributeBill = new QAction( trUtf8("Stampa"), this);
    connect( printAttributeBill, &QAction::triggered, this, &AttributesGUI::printAttributeBillODT );
    menu->addAction( printAttributeBill );

    menu->addSeparator();
    QAction * resizeColToContentAction = new QAction( trUtf8("Ottimizza colonne"), this);
    connect( resizeColToContentAction, &QAction::triggered, this, &AttributesGUI::resizeAttributesColToContents );
    menu->addAction( resizeColToContentAction );

    menu->popup( m_d->ui->attributesTableView->viewport()->mapToGlobal(pos) );
}

void AttributesGUI::resizeAttributesColToContents(){
    if( m_d->ui->attributesTableView->model() ){
        for( int i=0; i < m_d->ui->attributesTableView->model()->columnCount(); ++i ){
            m_d->ui->attributesTableView->resizeColumnToContents(i);
        }
    }
}
