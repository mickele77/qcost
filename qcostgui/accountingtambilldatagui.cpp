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
#include "accountingtambilldatagui.h"
#include "ui_accountingtambilldatagui.h"

#include "accountingattributeprintergui.h"
#include "accountingbillsetpricelistmodegui.h"

#include "project.h"
#include "accountingtambill.h"
#include "attributesmodel.h"
#include "accountingpricefieldmodel.h"
#include "pricelist.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QShowEvent>
#include <QProcess>
#include <QFileDialog>
#include <QMenu>

class AccountingTAMBillDataGUIPrivate{
public:
    AccountingTAMBillDataGUIPrivate(PriceFieldModel * pfm, MathParser * prs, Project * prj, QString * wpf):
        ui(new Ui::AccountingTAMBillDataGUI() ),
        project(prj),
        accounting(NULL),
        parser(prs),
        wordProcessorFile(wpf),
        priceFieldModel(pfm),
        amountSpacer(NULL){
    }
    Ui::AccountingTAMBillDataGUI * ui;
    Project * project;
    AccountingTAMBill * accounting;
    MathParser * parser;
    QString * wordProcessorFile;
    PriceFieldModel * priceFieldModel;
    QList<QLabel *> amountLabelList;
    QList<QLineEdit *> amountLEditList;
    QSpacerItem * amountSpacer;
};

AccountingTAMBillDataGUI::AccountingTAMBillDataGUI(PriceFieldModel * pfm, MathParser * prs, AccountingTAMBill * b, Project * prj, QString * wordProcessorFile, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingTAMBillDataGUIPrivate( pfm, prs, prj, wordProcessorFile ) ) {
    m_d->ui->setupUi( this );
    m_d->ui->totalPriceFieldTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_d->ui->noDiscountPriceFieldTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    populatePriceListComboBox();

    setAccountingTAMBill( b );
    connect( m_d->ui->addAttributePushButton, &QPushButton::clicked, this, &AccountingTAMBillDataGUI::addAttribute );
    connect( m_d->ui->removeAttributePushButton, &QPushButton::clicked, this, &AccountingTAMBillDataGUI::removeAttribute );
    connect( m_d->ui->printAttributePushButton, &QPushButton::clicked, this, &AccountingTAMBillDataGUI::printAttributeAccountingODT );

    m_d->ui->attributesTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_d->ui->attributesTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(m_d->ui->attributesTableView, &QTableView::customContextMenuRequested, this, &AccountingTAMBillDataGUI::attributesTableViewCustomMenuRequested );

    connect( m_d->ui->discountLineEdit, &QLineEdit::editingFinished, this, &AccountingTAMBillDataGUI::setDiscount );
}

AccountingTAMBillDataGUI::~AccountingTAMBillDataGUI(){
    delete m_d;
}

void AccountingTAMBillDataGUI::setAccountingTAMBill(AccountingTAMBill *b) {
    if( m_d->accounting != b ){
        if( m_d->accounting != NULL ){
            disconnect( m_d->accounting, &AccountingTAMBill::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::amountToDiscountChanged, m_d->ui->amountToDiscountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::amountDiscountedChanged, m_d->ui->amountDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accounting, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTAMBillDataGUI::setAccountingNULL );
            disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTAMBillDataGUI::setPriceList );
            disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTAMBillDataGUI::setPriceDataSet );
        }
        m_d->ui->priceListComboBox->setCurrentIndex( 0 );
        m_d->ui->currentPriceDataSetSpinBox->setMaximum(1);
        m_d->ui->currentPriceDataSetSpinBox->setValue(1);

        for( int i=0; i < m_d->amountLEditList.size(); ++i){
            if( i < m_d->priceFieldModel->fieldCount() ){
                m_d->amountLEditList.at(i)->clear();
            }
        }
        m_d->ui->discountLineEdit->clear();
        m_d->ui->totalPriceFieldTableView->setModel( NULL );
        m_d->ui->noDiscountPriceFieldTableView->setModel( NULL );
        m_d->ui->attributesTableView->setModel( NULL );

        m_d->accounting = b;

        if( m_d->accounting != NULL ){
            m_d->ui->totalAmountToDiscountLineEdit->setText( m_d->accounting->totalAmountToDiscountStr() );
            connect( m_d->accounting, &AccountingTAMBill::totalAmountToDiscountChanged, m_d->ui->totalAmountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountNotToDiscountLineEdit->setText( m_d->accounting->amountNotToDiscountStr() );
            connect( m_d->accounting, &AccountingTAMBill::amountNotToDiscountChanged, m_d->ui->amountNotToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountToDiscountLineEdit->setText( m_d->accounting->amountToDiscountStr() );
            connect( m_d->accounting, &AccountingTAMBill::amountToDiscountChanged, m_d->ui->amountToDiscountLineEdit, &QLineEdit::setText );
            m_d->ui->amountDiscountedLineEdit->setText( m_d->accounting->amountDiscountedStr() );
            connect( m_d->accounting, &AccountingTAMBill::amountDiscountedChanged, m_d->ui->amountDiscountedLineEdit, &QLineEdit::setText );
            m_d->ui->totalAmountLineEdit->setText( m_d->accounting->totalAmountStr() );
            connect( m_d->accounting, &AccountingTAMBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            m_d->ui->discountLineEdit->setText( m_d->accounting->discountStr());
            connect( m_d->accounting, &AccountingTAMBill::discountChanged, m_d->ui->discountLineEdit, &QLineEdit::setText );

            setPriceListComboBox();
            connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTAMBillDataGUI::setPriceList );
            setPriceDataSetSpinBox();
            connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTAMBillDataGUI::setPriceDataSet );

            m_d->ui->totalPriceFieldTableView->setModel( m_d->accounting->totalAmountPriceFieldModel() );
            m_d->ui->noDiscountPriceFieldTableView->setModel( m_d->accounting->noDiscountAmountPriceFieldModel() );
            m_d->ui->attributesTableView->setModel( m_d->accounting->attributeModel() );

            connect( m_d->accounting, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTAMBillDataGUI::setAccountingNULL );
        }
    }
}

void AccountingTAMBillDataGUI::setDiscount() {
    if( m_d->accounting != NULL ){
        m_d->accounting->setDiscount( m_d->ui->discountLineEdit->text() );
    }
}

void AccountingTAMBillDataGUI::setAccountingNULL(){
    setAccountingTAMBill( NULL );
}

void AccountingTAMBillDataGUI::updateAmountValue(int priceField, const QString & newVal ){
    if( priceField > -1 && priceField < m_d->amountLEditList.size() ){
        m_d->amountLEditList.at(priceField)->setText( newVal );
    }
}

void AccountingTAMBillDataGUI::addAttribute(){
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

void AccountingTAMBillDataGUI::removeAttribute(){
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

void AccountingTAMBillDataGUI::updateAmountsValue()
{

}

bool AccountingTAMBillDataGUI::printAttributeAccountingODT(){
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
                                                             trUtf8("Stampa riepilogo Lista Opere in Economia"), ".",
                                                             trUtf8("Documento di testo (*.odt)"));

            if (!fileName.isEmpty()){
                QString suf = fileName.split(".").last().toLower();
                if( suf != "odt"){
                    fileName.append( ".odt" );
                }

                /*AccountingPrinter printer( m_d->accounting, m_d->priceFieldModel, m_d->parser );
                bool ret = printer.printAttributeODT( prAccountingMeasureOption, prOption, prFields, prAttrs, fileName, paperWidth, paperHeight, paperOrientation, groupPrAm );
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
                return ret;*/
            }
        }
    }
    return false;
}

void AccountingTAMBillDataGUI::resizeAttributeColsToContents(){
    m_d->ui->attributesTableView->resizeColumnsToContents();
}

void AccountingTAMBillDataGUI::attributesTableViewCustomMenuRequested(QPoint pos){
    QMenu *menu=new QMenu(this);
    QAction * addAction = new QAction( trUtf8("Aggiungi"), this);
    connect( addAction, &QAction::triggered, this, &AccountingTAMBillDataGUI::addAttribute );
    menu->addAction( addAction );
    QAction * delAction = new QAction( trUtf8("Elimina"), this);
    connect( delAction, &QAction::triggered, this, &AccountingTAMBillDataGUI::removeAttribute );
    menu->addAction( delAction );
    menu->addSeparator();

    menu->addSeparator();
    QAction * printAttributeAccounting = new QAction( trUtf8("Stampa"), this);
    connect( printAttributeAccounting, &QAction::triggered, this, &AccountingTAMBillDataGUI::printAttributeAccountingODT );
    menu->addAction( printAttributeAccounting );

    menu->addSeparator();
    QAction * resizeColToContentAction = new QAction( trUtf8("Ottimizza colonne"), this);
    connect( resizeColToContentAction, &QAction::triggered, this, &AccountingTAMBillDataGUI::resizeAttributesColToContents );
    menu->addAction( resizeColToContentAction );

    menu->popup( m_d->ui->attributesTableView->viewport()->mapToGlobal(pos) );
}

void AccountingTAMBillDataGUI::resizeAttributesColToContents(){
    if( m_d->ui->attributesTableView->model() ){
        for( int i=0; i < m_d->ui->attributesTableView->model()->columnCount(); ++i ){
            m_d->ui->attributesTableView->resizeColumnToContents(i);
        }
    }
}

void AccountingTAMBillDataGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTAMBillDataGUI::setPriceList );
        populatePriceListComboBox();
        setPriceListComboBox();
        connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTAMBillDataGUI::setPriceList );

        disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTAMBillDataGUI::setPriceDataSet );
        setPriceDataSetSpinBox();
        connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTAMBillDataGUI::setPriceDataSet );
    }
    QWidget::showEvent( event );
}

void AccountingTAMBillDataGUI::populatePriceListComboBox(){
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

void AccountingTAMBillDataGUI::setPriceListComboBox() {
    if( m_d->accounting ){
        int i = m_d->ui->priceListComboBox->findData( qVariantFromValue((void *) m_d->accounting->priceList() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->priceListComboBox->setCurrentIndex( i );
    }
}

void AccountingTAMBillDataGUI::setPriceDataSetSpinBox() {
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

void AccountingTAMBillDataGUI::setPriceList() {
    QVariant v =  m_d->ui->priceListComboBox->itemData( m_d->ui->priceListComboBox->currentIndex() );
    PriceList * currentPriceList = (PriceList *) v.value<void *>();
    if( m_d->accounting ){
        if( m_d->accounting->priceList() && m_d->accounting->rowCount() > 0 ){
            AccountingBillSetPriceListModeGUI modeGUI( this );
            AccountingTAMBill::SetPriceListMode plMode = modeGUI.returnValueTAMBill();
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

void AccountingTAMBillDataGUI::setPriceDataSet() {
    if( m_d->accounting ){
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
