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
#include "accountingtreegui.h"

#include "ui_accountingtreegui.h"

#include "accountingsetpricelistmodegui.h"
#include "attributechangedialog.h"
#include "qcalendardialog.h"

#include "qcostclipboarddata.h"
#include "project.h"
#include "accountingbill.h"
#include "accountingbillitem.h"
#include "accountingtambill.h"
#include "accountingtambillitem.h"
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

class AccountingTreeGUIPrivate{
public:
    AccountingTreeGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString * fileName, MathParser * prs, Project * p ):
        ui(new Ui::AccountingTreeGUI),
        parser(prs),
        priceFieldModel( p->priceFieldModel() ),
        accountingBill(NULL),
        accountingTAMBill(NULL),
        project( p ),
        EPAImportOptions(EPAImpOptions),
        EPAFileName(fileName){
    }

    Ui::AccountingTreeGUI *ui;
    MathParser * parser;
    PriceFieldModel * priceFieldModel;
    AccountingBill * accountingBill;
    AccountingTAMBill * accountingTAMBill;
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

AccountingTreeGUI::AccountingTreeGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString * EPAFileName, AccountingBill * b, MathParser * prs, Project * prj, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingTreeGUIPrivate(EPAImpOptions, EPAFileName, prs, prj) ){
    m_d->ui->setupUi(this);

    populatePriceListComboBox();

    setAccountingBill( b );

    m_d->addBillAction = new QAction(trUtf8("S.A.L."), this);
    connect( m_d->addBillAction, &QAction::triggered, this, &AccountingTreeGUI::addItems );
    m_d->addCommentAction = new QAction(trUtf8("Riga di commento"), this);
    connect( m_d->addCommentAction, &QAction::triggered, this, &AccountingTreeGUI::addItems );
    m_d->addPPUAction = new QAction(trUtf8("Opera a misura"), this);
    connect( m_d->addPPUAction, &QAction::triggered, this, &AccountingTreeGUI::addItems );
    m_d->addLSAction = new QAction(trUtf8("Opera a corpo"), this);
    connect( m_d->addLSAction, &QAction::triggered, this, &AccountingTreeGUI::addItems );
    m_d->addTAMAction = new QAction(trUtf8("Lista economie"), this);
    connect( m_d->addTAMAction, &QAction::triggered, this, &AccountingTreeGUI::addItems );
    QMenu * addPushButtonMenu = new QMenu( m_d->ui->addPushButton );
    addPushButtonMenu->addAction(m_d->addBillAction);
    addPushButtonMenu->addAction(m_d->addCommentAction);
    addPushButtonMenu->addAction(m_d->addPPUAction);
    addPushButtonMenu->addAction(m_d->addLSAction);
    addPushButtonMenu->addAction(m_d->addTAMAction);
    m_d->ui->addPushButton->setMenu( addPushButtonMenu );
    connect( m_d->ui->delPushButton, &QPushButton::clicked, this, &AccountingTreeGUI::removeItems );
    connect( m_d->ui->treeView, &QTreeView::doubleClicked, this, static_cast<void(AccountingTreeGUI::*)(const QModelIndex &)>(&AccountingTreeGUI::editAccountingData) );

    m_d->ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->treeView, &QTreeView::customContextMenuRequested, this, &AccountingTreeGUI::accountingTreeViewCustomMenuRequested );
}

AccountingTreeGUI::AccountingTreeGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString * EPAFileName, AccountingTAMBill * b, MathParser * prs, Project * prj, QWidget *parent) :
    QWidget(parent),
    m_d( new AccountingTreeGUIPrivate(EPAImpOptions, EPAFileName, prs, prj) ){
    m_d->ui->setupUi(this);

    populatePriceListComboBox();

    setAccountingTAMBill( b );

    m_d->addTAMBillAction = new QAction(trUtf8("Lista"), this);
    connect( m_d->addTAMBillAction, &QAction::triggered, this, &AccountingTreeGUI::addItems );
    m_d->addTAMCommentAction = new QAction(trUtf8("Riga di commento"), this);
    connect( m_d->addTAMCommentAction, &QAction::triggered, this, &AccountingTreeGUI::addItems );
    m_d->addTAMPPUAction = new QAction(trUtf8("Misura"), this);
    connect( m_d->addTAMPPUAction, &QAction::triggered, this, &AccountingTreeGUI::addItems );

    QMenu * addPushButtonMenu = new QMenu( m_d->ui->addPushButton );
    addPushButtonMenu->addAction(m_d->addTAMBillAction);
    addPushButtonMenu->addAction(m_d->addTAMCommentAction);
    addPushButtonMenu->addAction(m_d->addTAMPPUAction);
    m_d->ui->addPushButton->setMenu( addPushButtonMenu );
    connect( m_d->ui->delPushButton, &QPushButton::clicked, this, &AccountingTreeGUI::removeItems );
    connect( m_d->ui->treeView, &QTreeView::doubleClicked, this, static_cast<void(AccountingTreeGUI::*)(const QModelIndex &)>(&AccountingTreeGUI::editAccountingData) );

    m_d->ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->treeView, &QTreeView::customContextMenuRequested, this, &AccountingTreeGUI::accountingTreeViewCustomMenuRequested );
}

AccountingTreeGUI::~AccountingTreeGUI() {
    delete m_d;
}

void AccountingTreeGUI::accountingTreeViewCustomMenuRequested(QPoint pos){
    QMenu *menu=new QMenu(this);
    if( m_d->accountingBill != NULL ){
        QMenu *addSubMenu=new QMenu(this);
        addSubMenu->setTitle( trUtf8("Aggiungi"));
        addSubMenu->addAction( m_d->addBillAction );
        addSubMenu->addAction( m_d->addPPUAction );
        addSubMenu->addAction( m_d->addCommentAction );
        addSubMenu->addAction( m_d->addLSAction );
        addSubMenu->addAction( m_d->addTAMAction );
        menu->addMenu( addSubMenu );
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
    } else if( m_d->accountingTAMBill != NULL ){
        QMenu *addSubMenu=new QMenu(this);
        addSubMenu->setTitle( trUtf8("Aggiungi"));
        addSubMenu->addAction( m_d->addTAMBillAction );
        addSubMenu->addAction( m_d->addTAMPPUAction );
        addSubMenu->addAction( m_d->addTAMCommentAction );
        menu->addMenu( addSubMenu );
        QAction * delAction = new QAction( trUtf8("Elimina"), this);
        connect( delAction, &QAction::triggered, this, &AccountingTreeGUI::removeItems );
        menu->addAction( delAction );
        menu->addSeparator();
        QAction * attributesAction = new QAction( trUtf8("Etichette"), this);
        connect( attributesAction, &QAction::triggered, this, &AccountingTreeGUI::editAttributes );
        menu->addAction( attributesAction );
        menu->addSeparator();
        QAction * cutAction = new QAction( trUtf8("Taglia"), this);
        connect( cutAction, &QAction::triggered, this, &AccountingTreeGUI::cutToClipboard );
        menu->addAction( cutAction );
        QAction * copyAction = new QAction( trUtf8("Copia"), this);
        connect( copyAction, &QAction::triggered, this, &AccountingTreeGUI::copyToClipboard );
        menu->addAction( copyAction );
        QAction * pasteAction = new QAction( trUtf8("Incolla"), this);
        connect( pasteAction, &QAction::triggered, this, &AccountingTreeGUI::pasteFromClipboard );
        menu->addAction( pasteAction );

        menu->addSeparator();
        QAction * resizeColToContent = new QAction( trUtf8("Ottimizza colonne"), this);
        connect( resizeColToContent, &QAction::triggered, this, &AccountingTreeGUI::resizeColumnsToContents );
        menu->addAction( resizeColToContent );
    }

    menu->popup( m_d->ui->treeView->viewport()->mapToGlobal(pos) );
}

void AccountingTreeGUI::copyToClipboard(){
    if( m_d->accountingBill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QCostClipboardData *data = new QCostClipboardData();
            const QCostClipboardData *clipData = qobject_cast<const QCostClipboardData *>(QApplication::clipboard()->mimeData());
            if( clipData != NULL ){
                *data = *clipData;
            }
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QList<AccountingBillItem *> copiedAccountingMeasures;
            for( int i=0; i < selRows.size(); ++i ){
                copiedAccountingMeasures << m_d->accountingBill->item( selRows.at(i)) ;
            }
            data->setCopiedAccountingMeasures( copiedAccountingMeasures, m_d->accountingBill, QCostClipboardData::Copy );
            QApplication::clipboard()->setMimeData( data );
        }
    }
}

void AccountingTreeGUI::cutToClipboard(){
    if( m_d->accountingBill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QCostClipboardData *data = new QCostClipboardData();
            const QCostClipboardData *clipData = qobject_cast<const QCostClipboardData *>(QApplication::clipboard()->mimeData());
            if( clipData != NULL ){
                *data = *clipData;
            }
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QList<AccountingBillItem *> copiedAccountingMeasures;
            for( int i=0; i < selRows.size(); ++i ){
                copiedAccountingMeasures << m_d->accountingBill->item( selRows.at(i)) ;
            }
            data->setCopiedAccountingMeasures( copiedAccountingMeasures, m_d->accountingBill, QCostClipboardData::Cut );
            QApplication::clipboard()->setMimeData( data );
        }
    }
}

void AccountingTreeGUI::pasteFromClipboard(){
    if( m_d->accountingBill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QClipboard * clp = QApplication::clipboard();
            const QMimeData * mimeData = clp->mimeData();
            const QCostClipboardData *data = qobject_cast<const QCostClipboardData *>( mimeData );

            if( data != NULL ){
                QModelIndex currIndex = m_d->ui->treeView->currentIndex();
                int currRow = m_d->accountingBill->rowCount( )-1;
                QModelIndex currParent = QModelIndex();
                if( currIndex.isValid() ){
                    currRow = currIndex.row();
                    currParent = currIndex.parent();
                }
                QList<AccountingBillItem *> itemsToCopy;
                AccountingBill * itemsToCopyAccounting = NULL;
                QCostClipboardData::Mode mode;
                data->getCopiedAccountingMeasures( &itemsToCopy, itemsToCopyAccounting, &mode);
                if( itemsToCopyAccounting != NULL ){
                    if( mode == QCostClipboardData::Copy ){
                        if( itemsToCopyAccounting->priceList() != m_d->accountingBill->priceList() ){
                            for( QList<AccountingBillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                                if( (*i)->itemType() == AccountingBillItem::PPU ){
                                    PriceItem * pItem = m_d->accountingBill->priceList()->priceItemCode( (*i)->priceItem()->codeFull() );
                                    if( pItem == NULL ){
                                        pItem = m_d->accountingBill->priceList()->appendPriceItem();
                                        *pItem = *((*i)->priceItem());
                                    }
                                    (*i)->setPriceItem( pItem );
                                }
                            }
                        }
                        for( QList<AccountingBillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                            bool containsParent = false;
                            for( QList<AccountingBillItem *>::iterator j=itemsToCopy.begin(); j != itemsToCopy.end(); ++j ){
                                if( i != j ){
                                    if( (*i)->isDescending(*j) ){
                                        containsParent = true;
                                        break;
                                    }
                                }
                            }
                            if( !containsParent ){
                                // TODO
                                /*if(m_d->accounting->insertMeasures( currRow+1, 1, currParent )) {
                                    *(m_d->accounting->accountingItem( m_d->accounting->index(currRow+1, 0, currParent ) ) ) = *(*i);
                                }*/
                            }
                        }
                    } else if( mode == QCostClipboardData::Cut ){
                        if( itemsToCopyAccounting == m_d->accountingBill ){
                            for( QList<AccountingBillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                                bool containsParent = false;
                                for( QList<AccountingBillItem *>::iterator j=itemsToCopy.begin(); j != itemsToCopy.end(); ++j ){
                                    if( i != j ){
                                        if( (*i)->isDescending(*j) ){
                                            containsParent = true;
                                            break;
                                        }
                                    }
                                }
                                if( !containsParent ){
                                    m_d->accountingBill->moveRows( m_d->accountingBill->index((*i)->parent(), 0), (*i)->childNumber(), 1, currParent, currRow+1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void AccountingTreeGUI::editAttributes(){
    if( m_d->accountingBill ){
        QList<AccountingBillItem *> itemsList;
        if( m_d->ui->treeView->selectionModel() ){
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            for( int i=0; i < selRows.size(); ++i ){
                itemsList << m_d->accountingBill->item( selRows.at(i)) ;
            }
        }
        AttributeChangeDialog dialog( &itemsList, m_d->accountingBill->attributeModel(), this );
        dialog.exec();
    }
}

#include "editpriceitemdialog.h"

void AccountingTreeGUI::editAccountingData( const QModelIndex & index ){
    if( m_d->accountingBill != NULL ){
        if( index.column() == 4 ){
            QDate d = m_d->accountingBill->item( index )->date();
            QCalendarDialog dialog( &d, this );
            if( dialog.exec() == QDialog::Accepted ){
                m_d->accountingBill->item( index )->setDate( d );
            }
        } else if( index.column() >= 1 || index.column() <= 3 ){
            if( m_d->accountingBill->priceList() == NULL ){
                QMessageBox msgBox;
                msgBox.setText( trUtf8("Al computo non è associato alcun elenco prezzi") );
                msgBox.setInformativeText(trUtf8("Prima di associare un prezzo ad una riga è necessario aver impostato l'elenco prezzi del computo.") );
                msgBox.setStandardButtons(QMessageBox::Ok );
                msgBox.exec();
            } else {
                if( !m_d->accountingBill->item( index )->hasChildren() ){
                    EditPriceItemDialog dialog( m_d->EPAImportOptions, m_d->EPAFileName, m_d->accountingBill->priceList(), m_d->accountingBill->priceDataSet(), m_d->accountingBill->item( index ), m_d->parser, m_d->project, this );
                    dialog.exec();
                }
            }
        }
    }

    if( m_d->accountingTAMBill != NULL ){
        if( index.column() == 4 ){
            if( m_d->accountingTAMBill->item(index)->itemType() == AccountingTAMBillItem::PPU ){
                QDate d = m_d->accountingTAMBill->item( index )->date();
                QCalendarDialog dialog( &d, this );
                if( dialog.exec() == QDialog::Accepted ){
                    m_d->accountingBill->item( index )->setDate( d );
                }
            }
        } else if( (index.column() == 1) ||
                   (index.column() == 2) ||
                   (index.column() == 3) ){
            if( m_d->accountingTAMBill->item(index)->itemType() == AccountingTAMBillItem::PPU ){
                if( m_d->accountingTAMBill->priceList() == NULL ){
                    QMessageBox msgBox;
                    msgBox.setText( trUtf8("Al computo non è associato alcun elenco prezzi") );
                    msgBox.setInformativeText(trUtf8("Prima di associare un prezzo ad una riga è necessario aver impostato l'elenco prezzi del computo.") );
                    msgBox.setStandardButtons(QMessageBox::Ok );
                    msgBox.exec();
                } else {
                    if( !m_d->accountingTAMBill->item( index )->hasChildren() ){
                        EditPriceItemDialog dialog( m_d->EPAImportOptions, m_d->EPAFileName, m_d->accountingTAMBill->priceList(), m_d->accountingBill->priceDataSet(), m_d->accountingTAMBill->item( index ), m_d->parser, m_d->project, this );
                        dialog.exec();
                    }
                }
            }
        }
    }
}

AccountingBillItem *AccountingTreeGUI::currentAccountingBill() {
    if( m_d->ui->treeView->selectionModel() ){
        if( m_d->accountingBill != NULL ){
            if( m_d->ui->treeView->selectionModel()->currentIndex().isValid() ){
                return m_d->accountingBill->item( m_d->ui->treeView->selectionModel()->currentIndex() );
            }
        }
    }
    return NULL;
}

AccountingTAMBillItem *AccountingTreeGUI::currentAccountingTAMBill() {
    if( m_d->ui->treeView->selectionModel() ){
        if( m_d->accountingBill != NULL ){
            if( m_d->ui->treeView->selectionModel()->currentIndex().isValid() ){
                return m_d->accountingTAMBill->item( m_d->ui->treeView->selectionModel()->currentIndex() );
            }
        }
    }
    return NULL;
}

void AccountingTreeGUI::setAccountingBill(AccountingBill *b ) {
    if( m_d->accountingBill != b ){
        if( m_d->accountingTAMBill != NULL ){
            disconnect( m_d->accountingTAMBill, &AccountingTAMBill::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingTAMBill, &AccountingTAMBill::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingTAMBill, &AccountingTAMBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingTAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTreeGUI::clear );
            disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTreeGUI::setPriceList );
            disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTreeGUI::setPriceDatSet );
        }
        if( m_d->accountingBill != NULL ){
            disconnect( m_d->accountingBill, &AccountingBill::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingBill, &AccountingBill::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingBill, &AccountingBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &AccountingTreeGUI::clear );
            disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTreeGUI::setPriceList );
            disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTreeGUI::setPriceDatSet );
        }
        m_d->ui->priceListComboBox->setCurrentIndex( 0 );
        m_d->ui->currentPriceDataSetSpinBox->setMaximum(1);
        m_d->ui->currentPriceDataSetSpinBox->setValue(1);
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->totalAmountToBeDiscountedLineEdit->clear();
        m_d->ui->amountNotToBeDiscountedLineEdit->clear();
        if( m_d->ui->treeView->selectionModel() ){
            disconnect( m_d->ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountingTreeGUI::changeCurrentItem  );
        }

        m_d->accountingBill = b;
        m_d->accountingTAMBill = NULL;

        m_d->ui->treeView->setModel( b );
        if( m_d->accountingBill != NULL ){
            m_d->ui->totalAmountLineEdit->setText( m_d->accountingBill->totalAmountStr() );
            m_d->ui->totalAmountToBeDiscountedLineEdit->setText( m_d->accountingBill->amountNotToBeDiscountedStr() );
            m_d->ui->amountNotToBeDiscountedLineEdit->setText( m_d->accountingBill->totalAmountToBeDiscountedStr() );
            connect( m_d->accountingBill, &AccountingBill::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            connect( m_d->accountingBill, &AccountingBill::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            connect( m_d->accountingBill, &AccountingBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            setPriceListComboBox();
            connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTreeGUI::setPriceList );
            setCurrentPriceDataSetSpinBox();
            connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTreeGUI::setPriceDatSet );
            connect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &AccountingTreeGUI::clear );
        }
        if( m_d->ui->treeView->selectionModel() ){
            connect( m_d->ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountingTreeGUI::changeCurrentItem  );
        }
    }
}

void AccountingTreeGUI::setAccountingTAMBill(AccountingTAMBill *b ) {
    if( m_d->accountingTAMBill != b ){
        if( m_d->accountingTAMBill != NULL ){
            disconnect( m_d->accountingTAMBill, &AccountingTAMBill::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingTAMBill, &AccountingTAMBill::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingTAMBill, &AccountingTAMBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingTAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTreeGUI::clear );
        }
        if( m_d->accountingBill != NULL ){
            disconnect( m_d->accountingBill, &AccountingBill::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingBill, &AccountingBill::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingBill, &AccountingBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );
            disconnect( m_d->accountingBill, &AccountingBill::aboutToBeDeleted, this, &AccountingTreeGUI::clear );
        }
        disconnect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTreeGUI::setPriceList );
        m_d->ui->priceListComboBox->setCurrentIndex( 0 );
        m_d->ui->currentPriceDataSetSpinBox->setMaximum(1);
        m_d->ui->currentPriceDataSetSpinBox->setValue(1);
        m_d->ui->totalAmountLineEdit->clear();
        m_d->ui->totalAmountToBeDiscountedLineEdit->clear();
        m_d->ui->amountNotToBeDiscountedLineEdit->clear();
        if( m_d->ui->treeView->selectionModel() ){
            disconnect( m_d->ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountingTreeGUI::changeCurrentItem  );
        }

        m_d->accountingTAMBill = b;
        m_d->accountingBill = NULL;
        m_d->ui->treeView->setModel( b );

        if( m_d->accountingTAMBill != NULL ){
            m_d->ui->totalAmountLineEdit->setText( m_d->accountingTAMBill->totalAmountStr() );
            m_d->ui->totalAmountToBeDiscountedLineEdit->setText( m_d->accountingTAMBill->amountNotToBeDiscountedStr() );
            m_d->ui->amountNotToBeDiscountedLineEdit->setText( m_d->accountingTAMBill->totalAmountToBeDiscountedStr() );
            connect( m_d->accountingTAMBill, &AccountingTAMBill::totalAmountToBeDiscountedChanged, m_d->ui->totalAmountToBeDiscountedLineEdit, &QLineEdit::setText );
            connect( m_d->accountingTAMBill, &AccountingTAMBill::amountNotToBeDiscountedChanged, m_d->ui->amountNotToBeDiscountedLineEdit, &QLineEdit::setText );
            connect( m_d->accountingTAMBill, &AccountingTAMBill::totalAmountChanged, m_d->ui->totalAmountLineEdit, &QLineEdit::setText );

            setPriceListComboBox();
            connect( m_d->ui->priceListComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &AccountingTreeGUI::setPriceList );
            setCurrentPriceDataSetSpinBox();
            connect( m_d->accountingTAMBill, &AccountingTAMBill::aboutToBeDeleted, this, &AccountingTreeGUI::clear );
        }
        if( m_d->ui->treeView->selectionModel() ){
            connect( m_d->ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AccountingTreeGUI::changeCurrentItem  );
        }
    }
}

void AccountingTreeGUI::clear(){
    setAccountingBill(NULL);
    setAccountingTAMBill(NULL);
}

void AccountingTreeGUI::removeItems(){
    if( m_d->ui->treeView->selectionModel() ){
        QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
        for( int i=selRows.size() - 1; i >= 0; --i){
            // se la lista non contiene il genitore dell'oggetto lo rimuovo
            // altrimenti è sufficiente rimuovere il genitore
            if( !(selRows.contains( selRows.at(i).parent()) ) ){
                if( m_d->accountingBill != NULL ){
                    m_d->accountingBill->removeItems( selRows.at(i).row(), 1, selRows.at(i).parent() );
                } else if( m_d->accountingTAMBill != NULL ){
                    m_d->accountingTAMBill->removeItems( selRows.at(i).row(), 1, selRows.at(i).parent() );
                }
            }
        }
    }
}

void AccountingTreeGUI::changeCurrentItem(const QModelIndex &currentIndex  ) {
    if( currentIndex.isValid() ){
        if( m_d->accountingBill != NULL ){
            emit currentBillItemChanged( m_d->accountingBill->item( currentIndex ));
            return;
        }
        if( m_d->accountingTAMBill != NULL ){
            emit currentTAMBillItemChanged( m_d->accountingTAMBill->item( currentIndex ));
            return;
        }
    }
    emit currentBillItemChanged( NULL );
}

void AccountingTreeGUI::setPriceList() {
    QVariant v =  m_d->ui->priceListComboBox->itemData( m_d->ui->priceListComboBox->currentIndex() );
    PriceList * currentPriceList = (PriceList *) v.value<void *>();
    if( m_d->accountingBill != NULL ){
        if( m_d->accountingBill->priceList() && m_d->accountingBill->rowCount() > 0 ){
            AccountingSetPriceListModeGUI modeGUI( this );
            AccountingBill::SetPriceListMode plMode = modeGUI.returnValue();
            m_d->accountingBill->setPriceList( currentPriceList, plMode );
        } else {
            m_d->accountingBill->setPriceList( currentPriceList );
        }
        if( m_d->accountingBill->priceList() != NULL ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->accountingBill->priceList()->priceDataSetCount() );
        } else {
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        }
    } else if( m_d->accountingTAMBill != NULL ){
        if( m_d->accountingTAMBill->priceList() && m_d->accountingTAMBill->rowCount() > 0 ){
            AccountingSetPriceListModeGUI modeGUI( this );
            AccountingTAMBill::SetPriceListMode plMode = modeGUI.returnTAMValue();
            m_d->accountingTAMBill->setPriceList( currentPriceList, plMode );
        } else {
            m_d->accountingTAMBill->setPriceList( currentPriceList );
        }
        if( m_d->accountingTAMBill->priceList() != NULL ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->accountingTAMBill->priceList()->priceDataSetCount() );
        } else {
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        }
    }
}

void AccountingTreeGUI::setPriceDatSet() {
    if( m_d->accountingBill != NULL ){
        if( m_d->accountingBill->priceList() ){
            if( m_d->ui->currentPriceDataSetSpinBox->value() < 1 ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
            } else if(  m_d->ui->currentPriceDataSetSpinBox->value() > m_d->accountingBill->priceList()->priceDataSetCount() ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->accountingBill->priceList()->priceDataSetCount() );
            } else {
                m_d->accountingBill->setPriceDataSet( m_d->ui->currentPriceDataSetSpinBox->value()-1 );
            }
        }
    } else if( m_d->accountingTAMBill != NULL ){
        if( m_d->accountingTAMBill->priceList() ){
            if( m_d->ui->currentPriceDataSetSpinBox->value() < 1 ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
            } else if(  m_d->ui->currentPriceDataSetSpinBox->value() > m_d->accountingTAMBill->priceList()->priceDataSetCount() ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->accountingTAMBill->priceList()->priceDataSetCount() );
            } else {
                m_d->accountingTAMBill->setPriceDataSet( m_d->ui->currentPriceDataSetSpinBox->value()-1 );
            }
        }
    }

}

void AccountingTreeGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        disconnect( m_d->ui->priceListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setPriceList()) );
        populatePriceListComboBox();
        setPriceListComboBox();
        connect( m_d->ui->priceListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setPriceList()) );

        disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTreeGUI::setPriceDatSet );
        setCurrentPriceDataSetSpinBox();
        connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTreeGUI::setPriceDatSet );
    }
    QWidget::showEvent( event );
}

void AccountingTreeGUI::populatePriceListComboBox(){
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

void AccountingTreeGUI::setPriceListComboBox() {
    if( m_d->accountingBill != NULL ){
        int i = m_d->ui->priceListComboBox->findData( qVariantFromValue((void *) m_d->accountingBill->priceList() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->priceListComboBox->setCurrentIndex( i );
    } else if( m_d->accountingTAMBill != NULL ){
        int i = m_d->ui->priceListComboBox->findData( qVariantFromValue((void *) m_d->accountingTAMBill->priceList() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->priceListComboBox->setCurrentIndex( i );
    }
}

void AccountingTreeGUI::setCurrentPriceDataSetSpinBox() {
    if( m_d->accountingBill != NULL ){
        if( m_d->accountingBill->priceList() ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->accountingBill->priceList()->priceDataSetCount() );
        }
        m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->accountingBill->priceDataSet()+1 );
    } else if( m_d->accountingTAMBill != NULL ){
        if( m_d->accountingTAMBill->priceList() ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->accountingTAMBill->priceList()->priceDataSetCount() );
        }
        m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->accountingTAMBill->priceDataSet()+1 );
    } else {
        m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
    }
}

void AccountingTreeGUI::resizeColumnsToContents(){
    if( m_d->ui->treeView->model() ){
        for( int i=0; i < m_d->ui->treeView->model()->columnCount(); ++i ){
            m_d->ui->treeView->resizeColumnToContents(i);
        }
    }
}

void AccountingTreeGUI::addItems(){
    if( m_d->accountingBill != NULL ){
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
                bool ret = false;
                AccountingBillItem * item = m_d->accountingBill->item( rowList.last() );

                if( sender() == m_d->addBillAction ) {
                    if( item->itemType() == AccountingBillItem::Bill ){
                        ret = m_d->accountingBill->insertItems(  AccountingBillItem::Bill, item->childNumber(), rowList.size(), rowList.last().parent() );
                    } else {
                        ret = m_d->accountingBill->insertItems( AccountingBillItem::Bill, rowList.last().parent().row()+1, rowList.size(), rowList.last().parent() );
                    }
                } else if( sender() == m_d->addCommentAction ) {
                    if( item->itemType() == AccountingBillItem::Bill ){
                        ret = m_d->accountingBill->insertItems( AccountingBillItem::Comment, -1, rowList.size(), rowList.last() );
                    } else if( (item->itemType() == AccountingBillItem::Comment) ||
                               (item->itemType() == AccountingBillItem::PPU) ||
                               (item->itemType() == AccountingBillItem::LumpSum) ||
                               (item->itemType() == AccountingBillItem::TimeAndMaterials) ){
                        ret = m_d->accountingBill->insertItems( AccountingBillItem::Comment, rowList.last().row()+1, rowList.size(), rowList.last().parent() );
                    }
                } else if( sender() == m_d->addPPUAction ) {
                    if( item->itemType() == AccountingBillItem::Bill ){
                        ret = m_d->accountingBill->insertItems( AccountingBillItem::PPU, -1, rowList.size(), rowList.last() );
                    } else if( (item->itemType() == AccountingBillItem::Comment) ||
                               (item->itemType() == AccountingBillItem::PPU) ||
                               (item->itemType() == AccountingBillItem::LumpSum) ||
                               (item->itemType() == AccountingBillItem::TimeAndMaterials) ){
                        ret = m_d->accountingBill->insertItems( AccountingBillItem::PPU, rowList.last().row()+1, rowList.size(), rowList.last().parent() );
                    }
                } else if( sender() == m_d->addLSAction ) {
                    if( item->itemType() == AccountingBillItem::Bill ){
                        ret = m_d->accountingBill->insertItems( AccountingBillItem::LumpSum, -1, rowList.size(), rowList.last() );
                    } else if( (item->itemType() == AccountingBillItem::Comment) ||
                               (item->itemType() == AccountingBillItem::PPU) ||
                               (item->itemType() == AccountingBillItem::LumpSum) ||
                               (item->itemType() == AccountingBillItem::TimeAndMaterials) ){
                        ret = m_d->accountingBill->insertItems( AccountingBillItem::PPU, rowList.last().row()+1, rowList.size(), rowList.last().parent() );
                    }
                } else {
                    return;
                }

                if( ret ){
                    if( m_d->ui->treeView->selectionModel() ){
                        m_d->ui->treeView->selectionModel()->clearSelection();
                        m_d->ui->treeView->selectionModel()->setCurrentIndex( m_d->accountingBill->index( rowList.last().row()+rowList.size(), 0, rowList.last().parent() ),
                                                                              QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent );
                    }
                }
            } else {
                m_d->accountingBill->insertItems( AccountingBillItem::Bill );
            }
        }
    } else if( m_d->accountingTAMBill != NULL ){
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
                // riordiniamo in base al numero di riga
                for( int i=0; i < (rowList.size()-1); ++i){
                    for( int j=i+1; j < rowList.size(); ++j){
                        if( rowList.at(j).row() < rowList.at(i).row() ){
                            rowList.swap( i, j );
                        }
                    }
                }
                bool ret = false;
                AccountingTAMBillItem * item = dynamic_cast<AccountingTAMBillItem *>( m_d->accountingTAMBill->item( rowList.last() ) );

                if( sender() == m_d->addTAMBillAction ) {
                    if( item->itemType() == AccountingTAMBillItem::Bill ){
                        ret = m_d->accountingTAMBill->insertItems(  AccountingTAMBillItem::Bill, item->childNumber(), rowList.size(), rowList.last().parent() );
                    } else {
                        ret = m_d->accountingTAMBill->insertItems( AccountingTAMBillItem::Bill, rowList.last().parent().row()+1, rowList.size(), rowList.last().parent() );
                    }
                } else if( sender() == m_d->addTAMCommentAction ) {
                    if( item->itemType() == AccountingTAMBillItem::Bill ){
                        ret = m_d->accountingTAMBill->insertItems( AccountingTAMBillItem::Comment, -1, rowList.size(), rowList.last() );
                    } else if( (item->itemType() == AccountingTAMBillItem::Comment) ||
                               (item->itemType() == AccountingTAMBillItem::PPU) ){
                        ret = m_d->accountingTAMBill->insertItems( AccountingTAMBillItem::Comment, rowList.last().row()+1, rowList.size(), rowList.last().parent() );
                    }
                } else if( sender() == m_d->addTAMPPUAction ) {
                    if( item->itemType() == AccountingTAMBillItem::Bill ){
                        ret = m_d->accountingTAMBill->insertItems( AccountingTAMBillItem::PPU, -1, rowList.size(), rowList.last() );
                    } else if( (item->itemType() == AccountingTAMBillItem::Comment) ||
                               (item->itemType() == AccountingTAMBillItem::PPU) ){
                        ret = m_d->accountingTAMBill->insertItems( AccountingTAMBillItem::PPU, rowList.last().row()+1, rowList.size(), rowList.last().parent() );
                    }
                } else {
                    return;
                }
                if( ret ){
                    if( m_d->ui->treeView->selectionModel() ){
                        m_d->ui->treeView->selectionModel()->clearSelection();
                        m_d->ui->treeView->selectionModel()->setCurrentIndex( m_d->accountingTAMBill->index( rowList.last().row()+rowList.size(), 0, rowList.last().parent() ),
                                                                              QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent );
                    }
                }
            } else if( sender() == m_d->addTAMBillAction ) {
                m_d->accountingTAMBill->insertItems( AccountingTAMBillItem::Bill );
            }
        }
    }
}
