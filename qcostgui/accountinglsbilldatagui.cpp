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
#include "accountinglsbilldatagui.h"
#include "ui_accountinglsbilldatagui.h"

#include "accountingattributeprintergui.h"
#include "accountingbillsetpricelistmodegui.h"

#include "project.h"
#include "accountinglsbill.h"
#include "attributesmodel.h"
#include "accountingpricefieldmodel.h"
#include "pricelist.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class AccountingLSBillDataGUIPrivate{
public:
    AccountingLSBillDataGUIPrivate(PriceFieldModel * pfm, MathParser * prs, Project * prj, QString * wpf):
        ui(new Ui::AccountingLSBillDataGUI() ),
        project(prj),
        accounting(NULL),
        parser(prs),
        wordProcessorFile(wpf),
        priceFieldModel(pfm),
        amountSpacer(NULL){
    }
    Ui::AccountingLSBillDataGUI * ui;
    Project * project;
    AccountingLSBill * accounting;
    MathParser * parser;
    QString * wordProcessorFile;
    PriceFieldModel * priceFieldModel;
    QList<QLabel *> amountLabelList;
    QList<QLineEdit *> amountLEditList;
    QSpacerItem * amountSpacer;
};

AccountingLSBillDataGUI::AccountingLSBillDataGUI(PriceFieldModel * pfm, MathParser * prs, AccountingLSBill * b, Project * prj, QString * wordProcessorFile, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingLSBillDataGUIPrivate( pfm, prs, prj, wordProcessorFile ) ) {
    m_d->ui->setupUi( this );
    m_d->ui->totalPriceFieldTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    populatePriceListComboBox();

    setAccountingBill( b );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillDataGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillDataGUI::removeAttribute );
    connect( m_d->ui->printAttributePushButton, &QPushButton::clicked, this, &AccountingLSBillDataGUI::printAttributeAccountingODT );

    m_d->ui->attributesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_d->ui->attributesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(m_d->ui->attributesTableView, &QTableView::customContextMenuRequested, this, &AccountingLSBillDataGUI::attributesTableViewCustomMenuRequested );

    connect( m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::editingFinished, this, &AccountingLSBillDataGUI::setPPUTotalToDiscount );
    connect( m_d->ui->PPUNotToDiscountLineEdit, &QLineEdit::editingFinished, this, &AccountingLSBillDataGUI::setPPUNotToDiscount );
}

AccountingLSBillDataGUI::~AccountingLSBillDataGUI(){
    delete m_d;
}

void AccountingLSBillDataGUI::setAccountingBill(AccountingLSBill *b) {
    if( m_d->accounting != b ){
        if( m_d->accounting != NULL ){
            disconnect( m_d->ui->codeLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingLSBill::setCode );
            disconnect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingLSBill::setName );
            disconnect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &AccountingLSBillDataGUI::setDescription );

            disconnect( m_d->accounting, &AccountingLSBill::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingLSBill::PPUNotToDiscountChanged, m_d->ui->PPUNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingLSBill::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );

            disconnect( m_d->accounting, &AccountingLSBill::aboutToBeDeleted, this, &AccountingLSBillDataGUI::clear );
            disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingLSBillDataGUI::setPriceList );
            disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingLSBillDataGUI::setPriceDataSet );
        }
        m_d->ui->priceListComboBox->setCurrentIndex( 0 );
        m_d->ui->currentPriceDataSetSpinBox->setMaximum(1);
        m_d->ui->currentPriceDataSetSpinBox->setValue(1);

        m_d->ui->codeLineEdit->clear();
        m_d->ui->nameLineEdit->clear();
        m_d->ui->descriptionTextEdit->clear();
        m_d->ui->PPUTotalToDiscountLineEdit->clear();
        m_d->ui->PPUNotToDiscountLineEdit->clear();
        m_d->ui->percentageAccountedLineEdit->clear();
        m_d->ui->totalPriceFieldTableView->setModel( NULL );
        m_d->ui->attributesTableView->setModel( NULL );

        m_d->accounting = b;

        if( m_d->accounting != NULL ){
            m_d->ui->codeLineEdit->setText( m_d->accounting->code() );
            connect( m_d->ui->codeLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingLSBill::setCode );
            m_d->ui->nameLineEdit->setText( m_d->accounting->name() );
            connect( m_d->ui->nameLineEdit, &QLineEdit::textEdited, m_d->accounting, &AccountingLSBill::setName );
            m_d->ui->descriptionTextEdit->setPlainText( m_d->accounting->description() );
            connect( m_d->ui->descriptionTextEdit, &QPlainTextEdit::textChanged, this, &AccountingLSBillDataGUI::setDescription );

            m_d->ui->PPUTotalToDiscountLineEdit->setText( m_d->accounting->PPUTotalToDiscountStr() );
            connect( m_d->accounting, &AccountingLSBill::PPUTotalToDiscountChanged, m_d->ui->PPUTotalToDiscountLineEdit, &QLineEdit::setText );
            QString s = m_d->accounting->PPUNotToDiscountStr();
            m_d->ui->PPUNotToDiscountLineEdit->setText( m_d->accounting->PPUNotToDiscountStr() );
            connect( m_d->accounting, &AccountingLSBill::PPUNotToDiscountChanged, m_d->ui->PPUNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->percentageAccountedLineEdit->setText( m_d->accounting->percentageAccountedStr() );
            connect( m_d->accounting, &AccountingLSBill::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );

            setPriceListComboBox();
            connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingLSBillDataGUI::setPriceList );
            setPriceDataSetSpinBox();
            connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingLSBillDataGUI::setPriceDataSet );

            m_d->ui->totalPriceFieldTableView->setModel( m_d->accounting->totalAmountPriceFieldModel() );
            m_d->ui->attributesTableView->setModel( m_d->accounting->attributeModel() );

            connect( m_d->accounting, &AccountingLSBill::aboutToBeDeleted, this, &AccountingLSBillDataGUI::clear );
        }
    }
}

void AccountingLSBillDataGUI::setDescription(){
    if( m_d->accounting ){
        m_d->accounting->setDescription( m_d->ui->descriptionTextEdit->toPlainText() );
    }
}

void AccountingLSBillDataGUI::clear(){
    setAccountingBill( NULL );
}

void AccountingLSBillDataGUI::addAttribute(){
    if( m_d->accounting != NULL ){
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
            m_d->accounting->attributeModel()->insertRows( row, count );
        }
    }
}

void AccountingLSBillDataGUI::removeAttribute(){
    if( m_d->accounting != NULL ){
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
                m_d->accounting->attributeModel()->removeRows( row, count );
            }
        }
    }
}

bool AccountingLSBillDataGUI::printAttributeAccountingODT(){
    if( m_d->accounting != NULL ){
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
                                                m_d->accounting->attributeModel(),
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
                AccountingPrinter printer( m_d->accounting, m_d->parser );
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

void AccountingLSBillDataGUI::resizeAttributeColsToContents(){
    m_d->ui->attributesTableView->resizeColumnsToContents();
}

void AccountingLSBillDataGUI::attributesTableViewCustomMenuRequested(QPoint pos){
    QMenu *menu=new QMenu(this);
    QAction * addAction = new QAction( trUtf8("Aggiungi"), this);
    connect( addAction, &QAction::triggered, this, &AccountingLSBillDataGUI::addAttribute );
    menu->addAction( addAction );
    QAction * delAction = new QAction( trUtf8("Elimina"), this);
    connect( delAction, &QAction::triggered, this, &AccountingLSBillDataGUI::removeAttribute );
    menu->addAction( delAction );
    menu->addSeparator();

    menu->addSeparator();
    QAction * printAttributeAccounting = new QAction( trUtf8("Stampa"), this);
    connect( printAttributeAccounting, &QAction::triggered, this, &AccountingLSBillDataGUI::printAttributeAccountingODT );
    menu->addAction( printAttributeAccounting );

    menu->addSeparator();
    QAction * resizeColToContentAction = new QAction( trUtf8("Ottimizza colonne"), this);
    connect( resizeColToContentAction, &QAction::triggered, this, &AccountingLSBillDataGUI::resizeAttributesColToContents );
    menu->addAction( resizeColToContentAction );

    menu->popup( m_d->ui->attributesTableView->viewport()->mapToGlobal(pos) );
}

void AccountingLSBillDataGUI::resizeAttributesColToContents(){
    if( m_d->ui->attributesTableView->model() ){
        for( int i=0; i < m_d->ui->attributesTableView->model()->columnCount(); ++i ){
            m_d->ui->attributesTableView->resizeColumnToContents(i);
        }
    }
}

void AccountingLSBillDataGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingLSBillDataGUI::setPriceList );
        populatePriceListComboBox();
        setPriceListComboBox();
        connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingLSBillDataGUI::setPriceList );

        disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingLSBillDataGUI::setPriceDataSet );
        setPriceDataSetSpinBox();
        connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingLSBillDataGUI::setPriceDataSet );
    }
    QWidget::showEvent( event );
}

void AccountingLSBillDataGUI::populatePriceListComboBox(){
    m_d->ui->priceListComboBox->clear();
    m_d->ui->priceListComboBox->addItem( QString("---"), qVariantFromValue((void *) NULL ));
    for( int i=0; i < m_d->project->priceListCount(); ++i){
        QString n;
        if( m_d->project->priceList(i) ){
            n =  m_d->project->priceList(i)->name();
        }
        m_d->ui->priceListComboBox->addItem( n, qVariantFromValue((void *) m_d->project->priceList(i) ));
    }
}

void AccountingLSBillDataGUI::setPriceListComboBox() {
    if( m_d->accounting ){
        int i = m_d->ui->priceListComboBox->findData( qVariantFromValue((void *) m_d->accounting->priceList() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->priceListComboBox->setCurrentIndex( i );
    }
}

void AccountingLSBillDataGUI::setPriceDataSetSpinBox() {
    if( m_d->accounting ){
        if( m_d->accounting->priceList() ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->accounting->priceList()->priceDataSetCount() );
        }
        m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->accounting->priceDataSet()+1 );
    } else {
        m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
    }
}

void AccountingLSBillDataGUI::setPriceList() {
    QVariant v =  m_d->ui->priceListComboBox->itemData( m_d->ui->priceListComboBox->currentIndex() );
    PriceList * currentPriceList = (PriceList *) v.value<void *>();
    if( m_d->accounting != NULL ){
        if( m_d->accounting->priceList() && m_d->accounting->rowCount() > 0 ){
            AccountingBillSetPriceListModeGUI modeGUI( this );
            AccountingLSBill::SetPriceListMode plMode = modeGUI.returnValueLSBill();
            m_d->accounting->setPriceList( currentPriceList, plMode );
        } else {
            m_d->accounting->setPriceList( currentPriceList );
        }
        if( m_d->accounting->priceList() ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->accounting->priceList()->priceDataSetCount() );
        } else {
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        }
    }
}

void AccountingLSBillDataGUI::setPriceDataSet() {
    if( m_d->accounting != NULL ){
        if( m_d->accounting->priceList() ){
            if( m_d->ui->currentPriceDataSetSpinBox->value() < 1 ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
            } else if(  m_d->ui->currentPriceDataSetSpinBox->value() > m_d->accounting->priceList()->priceDataSetCount() ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->accounting->priceList()->priceDataSetCount() );
            } else {
                m_d->accounting->setPriceDataSet( m_d->ui->currentPriceDataSetSpinBox->value()-1 );
            }
        }
    }
}

void AccountingLSBillDataGUI::setPPUTotalToDiscount(){
    if( m_d->accounting != NULL ){
        m_d->accounting->setPPUTotalToDiscount( m_d->ui->PPUTotalToDiscountLineEdit->text() );
    }
}

void AccountingLSBillDataGUI::setPPUNotToDiscount(){
    if( m_d->accounting != NULL ){
        m_d->accounting->setPPUNotToDiscount( m_d->ui->PPUNotToDiscountLineEdit->text() );
    }
}
