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
#include "accountinglstreegui.h"

#include "ui_accountinglstreegui.h"

#include "accountingsetpricelistmodegui.h"
#include "attributechangedialog.h"
#include "qcalendardialog.h"

#include "qcostclipboarddata.h"
#include "project.h"
#include "accountinglsbill.h"
#include "accountinglsbillitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QLineEdit>
#include <QMenu>
#include <QClipboard>
#include <QMessageBox>
#include <QDate>
#include <QShowEvent>

class AccountingLSTreeGUIPrivate{
public:
    AccountingLSTreeGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                QString * fileName, MathParser * prs, Project * p ):
        ui(new Ui::AccountingLSTreeGUI),
        parser(prs),
        priceFieldModel( p->priceFieldModel() ),
        bill(NULL),
        project( p ),
        EPAImportOptions(EPAImpOptions),
        EPAFileName(fileName){
    }

    Ui::AccountingLSTreeGUI *ui;
    MathParser * parser;
    PriceFieldModel * priceFieldModel;
    AccountingLSBill * bill;
    Project * project;
    QList<QLabel *> amountLabelList;
    QMap<PriceListDBWidget::ImportOptions, bool> *EPAImportOptions;
    QString * EPAFileName;

    QAction * addBillAction;
    QAction * addCommentAction;
    QAction * addPPUAction;
    QAction * addLSAction;
    QAction * addTAMAction;

    QAction * addTAMBillAction;
    QAction * addTAMCommentAction;
    QAction * addTAMPPUAction;
};

AccountingLSTreeGUI::AccountingLSTreeGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                          QString * EPAFileName, MathParser * prs, Project * prj, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingLSTreeGUIPrivate(EPAImpOptions, EPAFileName, prs, prj) ){
    m_d->ui->setupUi(this);

    connect( m_d->ui->addPushButton, &QPushButton::clicked, this, &AccountingLSTreeGUI::addItems );
    connect( m_d->ui->addChildPushButton, &QPushButton::clicked, this, &AccountingLSTreeGUI::addChildItems );
    connect( m_d->ui->delPushButton, &QPushButton::clicked, this, &AccountingLSTreeGUI::removeItems );
    connect( m_d->ui->treeView, &QTreeView::doubleClicked, this, static_cast<void(AccountingLSTreeGUI::*)(const QModelIndex &)>(&AccountingLSTreeGUI::editAccountingData) );

    m_d->ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->treeView, &QTreeView::customContextMenuRequested, this, &AccountingLSTreeGUI::accountingTreeViewCustomMenuRequested );
}

AccountingLSTreeGUI::~AccountingLSTreeGUI() {
    delete m_d;
}

void AccountingLSTreeGUI::accountingTreeViewCustomMenuRequested(QPoint pos){
    QMenu *menu=new QMenu(this);
    QAction * addAction = new QAction( trUtf8("Aggiungi"), this);
    connect( addAction, SIGNAL(triggered()), this, SLOT(addItems()) );
    menu->addAction( addAction );
    QAction * addChildAction = new QAction( trUtf8("Aggiungi ▼"), this);
    connect( addChildAction, SIGNAL(triggered()), this, SLOT(addChildItems()) );
    menu->addAction( addChildAction );
    QAction * delAction = new QAction( trUtf8("Elimina"), this);
    connect( delAction, SIGNAL(triggered()), this, SLOT(removeItems()) );
    menu->addAction( delAction );
    menu->addSeparator();
    QAction * attributesAction = new QAction( trUtf8("Etichette"), this);
    connect( attributesAction, SIGNAL(triggered()), this, SLOT(editAttributes()) );
    menu->addAction( attributesAction );
    menu->addSeparator();
    QAction * cutAction = new QAction( trUtf8("Taglia"), this);
    connect( cutAction, SIGNAL(triggered()), this, SLOT(cutToClipboard()) );
    menu->addAction( cutAction );
    QAction * copyAction = new QAction( trUtf8("Copia"), this);
    connect( copyAction, SIGNAL(triggered()), this, SLOT(copyToClipboard()) );
    menu->addAction( copyAction );
    QAction * pasteAction = new QAction( trUtf8("Incolla"), this);
    connect( pasteAction, SIGNAL(triggered()), this, SLOT(pasteFromClipboard()) );
    menu->addAction( pasteAction );

    menu->addSeparator();
    QAction * resizeColToContent = new QAction( trUtf8("Ottimizza colonne"), this);
    connect( resizeColToContent, SIGNAL(triggered()), SLOT(resizeColumnsToContents()) );
    menu->addAction( resizeColToContent );

    menu->popup( m_d->ui->treeView->viewport()->mapToGlobal(pos) );

    menu->popup( m_d->ui->treeView->viewport()->mapToGlobal(pos) );
}

void AccountingLSTreeGUI::copyToClipboard(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QCostClipboardData *data = new QCostClipboardData();
            const QCostClipboardData *clipData = qobject_cast<const QCostClipboardData *>(QApplication::clipboard()->mimeData());
            if( clipData != NULL ){
                *data = *clipData;
            }
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QList<AccountingLSBillItem *> copiedBillItems;
            for( int i=0; i < selRows.size(); ++i ){
                copiedBillItems << m_d->bill->item( selRows.at(i)) ;
            }
            /*data->setCopiedBillItems( copiedBillItems, m_d->bill, QCostClipboardData::Copy );
            QApplication::clipboard()->setMimeData( data );*/
        }
    }
}

void AccountingLSTreeGUI::cutToClipboard(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QCostClipboardData *data = new QCostClipboardData();
            const QCostClipboardData *clipData = qobject_cast<const QCostClipboardData *>(QApplication::clipboard()->mimeData());
            if( clipData != NULL ){
                *data = *clipData;
            }
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QList<AccountingLSBillItem *> copiedBillItems;
            for( int i=0; i < selRows.size(); ++i ){
                copiedBillItems << m_d->bill->item( selRows.at(i)) ;
            }
            /*data->setCopiedBillItems( copiedBillItems, m_d->bill, QCostClipboardData::Cut );
            QApplication::clipboard()->setMimeData( data );*/
        }
    }
}

void AccountingLSTreeGUI::pasteFromClipboard(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QClipboard * clp = QApplication::clipboard();
            const QMimeData * mimeData = clp->mimeData();
            const QCostClipboardData *data = qobject_cast<const QCostClipboardData *>( mimeData );

            if( data != NULL ){
                QModelIndex currIndex = m_d->ui->treeView->currentIndex();
                int currRow = m_d->bill->rowCount( )-1;
                QModelIndex currParent = QModelIndex();
                if( currIndex.isValid() ){
                    currRow = currIndex.row();
                    currParent = currIndex.parent();
                }
                QList<AccountingLSBillItem *> itemsToCopy;
                AccountingLSBill * itemsToCopyBill = NULL;
                QCostClipboardData::Mode mode;
                data->getCopiedAccountingLSBillItems( &itemsToCopy, itemsToCopyBill, &mode);
                if( itemsToCopyBill != NULL ){
                    if( mode == QCostClipboardData::Copy ){
                        if( itemsToCopyBill->priceList() != m_d->bill->priceList() ){
                            for( QList<AccountingLSBillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                                if( (*i)->hasChildren() ){
                                    (*i)->setPriceItem( NULL );
                                } else {
                                    PriceItem * pItem = m_d->bill->priceList()->priceItemCode( (*i)->priceItem()->codeFull() );
                                    if( pItem == NULL ){
                                        pItem = m_d->bill->priceList()->appendPriceItem();
                                        *pItem = *((*i)->priceItem());
                                    }
                                    (*i)->setPriceItem( pItem );
                                }
                            }
                        }
                        for( QList<AccountingLSBillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                            bool containsParent = false;
                            for( QList<AccountingLSBillItem *>::iterator j=itemsToCopy.begin(); j != itemsToCopy.end(); ++j ){
                                if( i != j ){
                                    if( (*i)->isDescending(*j) ){
                                        containsParent = true;
                                        break;
                                    }
                                }
                            }
                            if( !containsParent ){
                                if(m_d->bill->insertBillItems( NULL, currRow+1, 1, currParent )) {
                                    *(m_d->bill->item( m_d->bill->index(currRow+1, 0, currParent ) ) ) = *(*i);
                                }
                            }
                        }
                    } else if( mode == QCostClipboardData::Cut ){
                        if( itemsToCopyBill == m_d->bill ){
                            for( QList<AccountingLSBillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                                bool containsParent = false;
                                for( QList<AccountingLSBillItem *>::iterator j=itemsToCopy.begin(); j != itemsToCopy.end(); ++j ){
                                    if( i != j ){
                                        if( (*i)->isDescending(*j) ){
                                            containsParent = true;
                                            break;
                                        }
                                    }
                                }
                                if( !containsParent ){
                                    m_d->bill->moveRows( m_d->bill->index((*i)->parent(), 0), (*i)->childNumber(), 1, currParent, currRow+1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void AccountingLSTreeGUI::editAttributes(){
    if( m_d->bill ){
        QList<AccountingLSBillItem *> itemsList;
        if( m_d->ui->treeView->selectionModel() ){
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            for( int i=0; i < selRows.size(); ++i ){
                itemsList << m_d->bill->item( selRows.at(i)) ;
            }
        }
        /*AttributeChangeDialog dialog( &itemsList, m_d->bill->attributeModel(), this );
        dialog.exec();*/
    }
}

#include "editpriceitemdialog.h"

void AccountingLSTreeGUI::editAccountingData( const QModelIndex & index ){
    if( m_d->bill != NULL ){
        if( index.column() == 4 ){
            /*            QDate d = m_d->bill->billItem( index )->date();
            QCalendarDialog dialog( &d, this );
            if( dialog.exec() == QDialog::Accepted ){
                m_d->bill->item( index )->setDate( d );
            }*/
        } else if( index.column() >= 1 || index.column() <= 3 ){
            if( m_d->bill->priceList() == NULL ){
                QMessageBox msgBox;
                msgBox.setText( trUtf8("Al computo non è associato alcun elenco prezzi") );
                msgBox.setInformativeText(trUtf8("Prima di associare un prezzo ad una riga è necessario aver impostato l'elenco prezzi del computo.") );
                msgBox.setStandardButtons(QMessageBox::Ok );
                msgBox.exec();
            } else {
                if( !m_d->bill->item( index )->hasChildren() ){
                    EditPriceItemDialog dialog( m_d->EPAImportOptions,
                                                m_d->EPAFileName,
                                                m_d->bill->priceList(),
                                                m_d->bill->priceDataSet(),
                                                m_d->bill->item( index ),
                                                m_d->parser,
                                                m_d->project,
                                                this );
                    dialog.exec();
                }
            }
        }
    }
}

AccountingLSBillItem *AccountingLSTreeGUI::currentItem() {
    if( m_d->ui->treeView->selectionModel() ){
        if( m_d->bill != NULL ){
            if( m_d->ui->treeView->selectionModel()->currentIndex().isValid() ){
                return m_d->bill->item( m_d->ui->treeView->selectionModel()->currentIndex() );
            }
        }
    }
    return NULL;
}

void AccountingLSTreeGUI::setBill(AccountingLSBill *b ) {
    if( m_d->bill != b ){
        if( m_d->bill != NULL ){
            disconnect( m_d->bill, &AccountingLSBill::projAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->bill, &AccountingLSBill::accAmountChanged, m_d->ui->totalAmountAccountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->bill, &AccountingLSBill::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->bill, &AccountingLSBill::aboutToBeDeleted, this, &AccountingLSTreeGUI::clear );
        }
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->totalAmountAccountedLineEdit->clear();
        m_d->ui->percentageAccountedLineEdit->clear();
        if( m_d->ui->treeView->selectionModel() ){
            disconnect( m_d->ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountingLSTreeGUI::changeCurrentItem  );
        }

        m_d->bill = b;

        m_d->ui->treeView->setModel( b );
        if( m_d->bill != NULL ){
            m_d->ui->totalAmountLineEdit->setText( m_d->bill->projAmountStr() );
            m_d->ui->totalAmountAccountedLineEdit->setText( m_d->bill->accAmountStr() );
            m_d->ui->percentageAccountedLineEdit->setText( m_d->bill->percentageAccountedStr() );
            connect( m_d->bill, &AccountingLSBill::projAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            connect( m_d->bill, &AccountingLSBill::accAmountChanged, m_d->ui->totalAmountAccountedLineEdit, &QLineEdit::setText );
            connect( m_d->bill, &AccountingLSBill::percentageAccountedChanged, m_d->ui->percentageAccountedLineEdit, &QLineEdit::setText );

            connect( m_d->bill, &AccountingLSBill::aboutToBeDeleted, this, &AccountingLSTreeGUI::clear );
        }
        if( m_d->ui->treeView->selectionModel() ){
            connect( m_d->ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountingLSTreeGUI::changeCurrentItem  );
        }
    }
}


void AccountingLSTreeGUI::clear(){
    setBill(NULL);
}

void AccountingLSTreeGUI::changeCurrentItem(const QModelIndex &currentIndex  ) {
    if( currentIndex.isValid() ){
        if( m_d->bill != NULL ){
            emit currentItemChanged( m_d->bill->item( currentIndex ));
            return;
        }
    }
    emit currentItemChanged( NULL );
}

void AccountingLSTreeGUI::resizeColumnsToContents(){
    if( m_d->ui->treeView->model() ){
        for( int i=0; i < m_d->ui->treeView->model()->columnCount(); ++i ){
            m_d->ui->treeView->resizeColumnToContents(i);
        }
    }
}

void AccountingLSTreeGUI::addItems(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QModelIndexList rowListSel = m_d->ui->treeView->selectionModel()->selectedRows();
            if( rowListSel.size() > 0 ){
                // eliminiamo gli indici genitori di altri indici
                QModelIndexList rowList;
                for( int i=0; i < rowListSel.size(); ++i){
                    if( !rowListSel.contains( rowListSel.at(i).parent() ) ){
                        rowList.append( rowListSel.at(i));
                    }
                }
                // Reorder by row
                for( int i=0; i < (rowList.size()-1); ++i){
                    for( int j=i+1; j < rowList.size(); ++j){
                        if( rowList.at(j).row() < rowList.at(i).row() ){
                            rowList.swap( i, j );
                        }
                    }
                }
                m_d->bill->insertBillItems( NULL, rowList.last().row()+1, rowList.size(), rowList.last().parent() );
                if( m_d->ui->treeView->selectionModel() ){
                    m_d->ui->treeView->selectionModel()->clearSelection();
                    m_d->ui->treeView->selectionModel()->setCurrentIndex( m_d->bill->index( rowList.last().row()+rowList.size(), 0, rowList.last().parent() ),
                                                                          QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent );
                }
            } else {
                m_d->bill->insertBillItems( NULL, 0 );
                if( m_d->ui->treeView->selectionModel() ){
                    m_d->ui->treeView->selectionModel()->clearSelection();
                    m_d->ui->treeView->selectionModel()->setCurrentIndex( m_d->bill->index( 0, 0, QModelIndex() ),
                                                                          QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent );
                }
            }
        }
    }
}

void AccountingLSTreeGUI::addChildItems(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QModelIndexList rowListSel = m_d->ui->treeView->selectionModel()->selectedRows();
            for( int i=0; i < rowListSel.size(); ++i){
                m_d->bill->insertBillItems( NULL, 0, 1, rowListSel.at(i) );
            }
            if( m_d->ui->treeView->selectionModel() ){
                m_d->ui->treeView->selectionModel()->clearSelection();
                m_d->ui->treeView->selectionModel()->setCurrentIndex( m_d->bill->index( 0, 0, rowListSel.last() ),
                                                                      QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent );
            }
        }
    }
}

void AccountingLSTreeGUI::removeItems(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            for( int i=selRows.size() - 1; i >= 0; --i){
                // se la lista non contiene il genitore dell'oggetto lo rimuovo
                // altrimenti è sufficiente rimuovere il genitore
                if( !(selRows.contains( selRows.at(i).parent()) ) ){
                    m_d->bill->removeItems( selRows.at(i).row(), 1, selRows.at(i).parent() );
                }
            }
        }
    }
}
