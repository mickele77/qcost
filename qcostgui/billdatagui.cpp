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
#include "billdatagui.h"

#include "ui_billdatagui.h"

#include "billattributeprintergui.h"
#include "bill.h"
#include "billattributemodel.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class BillDataGUIPrivate{
public:
    BillDataGUIPrivate(PriceFieldModel * pfm, MathParser * prs, QString * wpf):
        ui(new Ui::BillDataGUI() ),
        bill(NULL),
        parser(prs),
        wordProcessorFile(wpf),
        priceFieldModel(pfm),
        amountSpacer(NULL){
    };
    Ui::BillDataGUI * ui;
    Bill * bill;
    MathParser * parser;
    QString * wordProcessorFile;
    PriceFieldModel * priceFieldModel;
    QList<QLabel *> amountLabelList;
    QList<QLineEdit *> amountLEditList;
    QSpacerItem * amountSpacer;
};

BillDataGUI::BillDataGUI(PriceFieldModel * pfm, MathParser * prs, Bill * b, QString * wordProcessorFile, QWidget *parent) :
    QWidget(parent),
    m_d( new BillDataGUIPrivate( pfm, prs, wordProcessorFile ) ) {
    m_d->ui->setupUi( this );
    setBill( b );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &BillDataGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &BillDataGUI::removeAttribute );
    connect( m_d->ui->printAttributeBillPushButton, &QPushButton::clicked, this, &BillDataGUI::printAttributeBillODT );

    connect( pfm, &PriceFieldModel::endInsertPriceField, this, &BillDataGUI::updateAmountsNameValue );
    connect( pfm, &PriceFieldModel::endRemovePriceField, this, &BillDataGUI::updateAmountsNameValue );
    updateAmountsNameValue();
    connect( pfm, &PriceFieldModel::amountNameChanged, this, &BillDataGUI::updateAmountName );

    m_d->ui->attributesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->attributesTableView, &QTableView::customContextMenuRequested, this, &BillDataGUI::attributesTableViewCustomMenuRequested );
}

BillDataGUI::~BillDataGUI(){
    delete m_d;
}

void BillDataGUI::setBill(Bill *b) {
    if( m_d->bill != b ){
        if( m_d->bill != NULL ){
            disconnect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->bill, &Bill::setName );
            disconnect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &BillDataGUI::setDescription );
            disconnect( m_d->bill, static_cast<void(Bill::*)(int,const QString &)> (&Bill::amountChanged), this, &BillDataGUI::updateAmountValue );
            disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &BillDataGUI::setBillNULL );
        }
        m_d->ui->nameLineEdit->clear();
        m_d->ui->descriptionTextEdit->clear();
        for( int i=0; i < m_d->amountLEditList.size(); ++i){
            if( i < m_d->priceFieldModel->fieldCount() ){
                m_d->amountLEditList.at(i)->clear();
            }
        }
        m_d->ui->attributesTableView->setModel( NULL );

        m_d->bill = b;

        if( m_d->bill != NULL ){
            m_d->ui->nameLineEdit->setText( m_d->bill->name() );
            connect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->bill, &Bill::setName );
            m_d->ui->descriptionTextEdit->setPlainText( m_d->bill->description() );
            connect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &BillDataGUI::setDescription );

            for( int i=0; i < m_d->amountLEditList.size(); ++i){
                if( i < m_d->priceFieldModel->fieldCount() ){
                    m_d->amountLEditList.at(i)->setText( m_d->bill->amountStr(i) );
                }
            }
            connect( m_d->bill, static_cast<void(Bill::*)(int,const QString &)> (&Bill::amountChanged), this, &BillDataGUI::updateAmountValue );

            m_d->ui->attributesTableView->setModel( m_d->bill->attributeModel() );

            connect( m_d->bill, &Bill::aboutToBeDeleted, this, &BillDataGUI::setBillNULL );
        }
    }
}


void BillDataGUI::setDescription(){
    if( m_d->bill ){
        m_d->bill->setDescription( m_d->ui->descriptionTextEdit->toPlainText() );
    }
}

void BillDataGUI::setBillNULL(){
    setBill( NULL );
}

void BillDataGUI::updateAmountsNameValue(){
    for( QList<QLabel * >::iterator i = m_d->amountLabelList.begin(); i != m_d->amountLabelList.end(); ++i ){
        m_d->ui->amountsLayout->removeWidget( *i );
        delete *i;
    }
    m_d->amountLabelList.clear();
    for( QList<QLineEdit * >::iterator i = m_d->amountLEditList.begin(); i != m_d->amountLEditList.end(); ++i ){
        m_d->ui->amountsLayout->removeWidget( *i );
        delete *i;
    }
    if( m_d->amountSpacer != NULL ){
        m_d->ui->amountsLayout->removeItem( m_d->amountSpacer );
        delete m_d->amountSpacer;
        m_d->amountSpacer = NULL;
    }
    m_d->amountLEditList.clear();

    for( int i=0; i<m_d->priceFieldModel->fieldCount(); ++i){
        QLabel * label = new QLabel( m_d->priceFieldModel->amountName( i ) );
        QLineEdit * lEdit = new QLineEdit();
        lEdit->setReadOnly( true );
        lEdit->setAlignment( Qt::AlignRight);
        if( m_d->bill != NULL ){
            lEdit->setText( m_d->bill->amountStr(i));
        }
        m_d->ui->amountsLayout->addWidget( label, i, 0 );
        m_d->ui->amountsLayout->addWidget( lEdit, i, 1 );
        if( i == 0 ){
            m_d->amountSpacer = new QSpacerItem(20, 25, QSizePolicy::Expanding, QSizePolicy::Minimum );
            m_d->ui->amountsLayout->addItem( m_d->amountSpacer, i, 2 );
        }
        m_d->amountLabelList.append( label );
        m_d->amountLEditList.append( lEdit );
    }
}

void BillDataGUI::updateAmountValue(int priceField, const QString & newVal ){
    if( priceField > -1 && priceField < m_d->amountLEditList.size() ){
        m_d->amountLEditList.at(priceField)->setText( newVal );
    }
}

void BillDataGUI::updateAmountName( int priceField, const QString & newName ){
    if( priceField > -1 && priceField < m_d->amountLEditList.size() ){
        m_d->amountLabelList.at(priceField)->setText( newName );
    }
}

void BillDataGUI::addAttribute(){
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
            m_d->bill->attributeModel()->insertRows( row, count );
        }
    }
}

void BillDataGUI::removeAttribute(){
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
                m_d->bill->attributeModel()->removeRows( row, count );
            }
        }
    }
}

bool BillDataGUI::printAttributeBillODT(){
    if( m_d->bill != NULL ){
        BillPrinter::PrintBillItemsOption prBillItemOption;
        BillPrinter::AttributePrintOption prOption;
        QList<int> prFields;
        QList<BillAttribute *> prAttrs;
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
                                          m_d->bill->attributeModel(),
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
    }
    return false;
}

void BillDataGUI::resizeAttributeColsToContents(){
    m_d->ui->attributesTableView->resizeColumnsToContents();
}

void BillDataGUI::attributesTableViewCustomMenuRequested(QPoint pos){
    QMenu *menu=new QMenu(this);
    QAction * addAction = new QAction( trUtf8("Aggiungi"), this);
    connect( addAction, &QAction::triggered, this, &BillDataGUI::addAttribute );
    menu->addAction( addAction );
    QAction * delAction = new QAction( trUtf8("Elimina"), this);
    connect( delAction, &QAction::triggered, this, &BillDataGUI::removeAttribute );
    menu->addAction( delAction );
    menu->addSeparator();

    menu->addSeparator();
    QAction * printAttributeBill = new QAction( trUtf8("Stampa"), this);
    connect( printAttributeBill, &QAction::triggered, this, &BillDataGUI::printAttributeBillODT );
    menu->addAction( printAttributeBill );

    menu->addSeparator();
    QAction * resizeColToContentAction = new QAction( trUtf8("Ottimizza colonne"), this);
    connect( resizeColToContentAction, &QAction::triggered, this, &BillDataGUI::resizeAttributesColToContents );
    menu->addAction( resizeColToContentAction );

    menu->popup( m_d->ui->attributesTableView->viewport()->mapToGlobal(pos) );
}

void BillDataGUI::resizeAttributesColToContents(){
    if( m_d->ui->attributesTableView->model() ){
        for( int i=0; i < m_d->ui->attributesTableView->model()->columnCount(); ++i ){
            m_d->ui->attributesTableView->resizeColumnToContents(i);
        }
    }
}
