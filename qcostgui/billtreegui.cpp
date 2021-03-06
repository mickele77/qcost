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
#include "billtreegui.h"

#include "ui_billtreegui.h"

#include "billsetpricelistmodegui.h"
#include "billattributechangedialog.h"

#include "qcostclipboarddata.h"
#include "project.h"
#include "bill.h"
#include "billitem.h"
#include "pricelist.h"
#include "priceitem.h"
#include "pricefieldmodel.h"
#include "mathparser.h"

#include <QLineEdit>
#include <QMenu>
#include <QClipboard>
#include <QMessageBox>
#include <QShowEvent>

class BillTreeGUIPrivate{
public:
    BillTreeGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString * fileName, MathParser * prs, Project * p ):
        ui(new Ui::BillTreeGUI),
        parser(prs),
        priceFieldModel( p->priceFieldModel() ),
        bill(NULL),
        project( p ),
        EPAImportOptions(EPAImpOptions),
        EPAFileName(fileName){
    }

    Ui::BillTreeGUI *ui;
    MathParser * parser;
    PriceFieldModel * priceFieldModel;
    Bill * bill;
    Project * project;
    QList<QLabel *> amountLabelList;
    QList<QLineEdit *> amountLEditList;
    QMap<PriceListDBWidget::ImportOptions, bool> *EPAImportOptions;
    QString * EPAFileName;
};

BillTreeGUI::BillTreeGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString * EPAFileName, Bill * b, MathParser * prs, Project * prj, QWidget *parent) :
    QWidget(parent),
    m_d( new BillTreeGUIPrivate(EPAImpOptions, EPAFileName, prs, prj) ){
    m_d->ui->setupUi(this);

    populatePriceListComboBox();

    setBill( b );
    connect( m_d->ui->addPushButton, &QPushButton::clicked, this, &BillTreeGUI::addItems );
    connect( m_d->ui->addChildPushButton, &QPushButton::clicked, this, &BillTreeGUI::addChildItems );
    connect( m_d->ui->delPushButton, &QPushButton::clicked, this, &BillTreeGUI::removeItems );
    connect( m_d->ui->treeView, &QTreeView::doubleClicked, this, static_cast<void(BillTreeGUI::*)(const QModelIndex &)>(&BillTreeGUI::editBillItemPrice) );
    connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &BillTreeGUI::setPriceCol );

    connect( prj->priceFieldModel(), &PriceFieldModel::endInsertPriceField, this, &BillTreeGUI::updateAmountsNameValue );
    connect( prj->priceFieldModel(), &PriceFieldModel::endRemovePriceField, this, &BillTreeGUI::updateAmountsNameValue );
    updateAmountsNameValue();
    connect( prj->priceFieldModel(), &PriceFieldModel::amountNameChanged, this, &BillTreeGUI::updateAmountName );

    m_d->ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->treeView, &QTreeView::customContextMenuRequested, this, &BillTreeGUI::billTreeViewCustomMenuRequested );
}

BillTreeGUI::~BillTreeGUI() {
    delete m_d;
}

void BillTreeGUI::billTreeViewCustomMenuRequested(QPoint pos){
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
}

void BillTreeGUI::copyToClipboard(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QCostClipboardData *data = new QCostClipboardData();
            const QCostClipboardData *clipData = qobject_cast<const QCostClipboardData *>(QApplication::clipboard()->mimeData());
            if( clipData != NULL ){
                *data = *clipData;
            }
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QList<BillItem *> copiedBillItems;
            for( int i=0; i < selRows.size(); ++i ){
                copiedBillItems << m_d->bill->billItem( selRows.at(i)) ;
            }
            data->setCopiedBillItems( copiedBillItems, m_d->bill, QCostClipboardData::Copy );
            QApplication::clipboard()->setMimeData( data );
        }
    }
}

void BillTreeGUI::cutToClipboard(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QCostClipboardData *data = new QCostClipboardData();
            const QCostClipboardData *clipData = qobject_cast<const QCostClipboardData *>(QApplication::clipboard()->mimeData());
            if( clipData != NULL ){
                *data = *clipData;
            }
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QList<BillItem *> copiedBillItems;
            for( int i=0; i < selRows.size(); ++i ){
                copiedBillItems << m_d->bill->billItem( selRows.at(i)) ;
            }
            data->setCopiedBillItems( copiedBillItems, m_d->bill, QCostClipboardData::Cut );
            QApplication::clipboard()->setMimeData( data );
        }
    }
}

void BillTreeGUI::pasteFromClipboard(){
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
                QList<BillItem *> itemsToCopy;
                Bill * itemsToCopyBill = NULL;
                QCostClipboardData::Mode mode;
                data->getCopiedBillItems( &itemsToCopy, itemsToCopyBill, &mode);
                if( itemsToCopyBill != NULL ){
                    if( mode == QCostClipboardData::Copy ){
                        if( itemsToCopyBill->priceList() != m_d->bill->priceList() ){
                            for( QList<BillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
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
                        for( QList<BillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                            bool containsParent = false;
                            for( QList<BillItem *>::iterator j=itemsToCopy.begin(); j != itemsToCopy.end(); ++j ){
                                if( i != j ){
                                    if( (*i)->isDescending(*j) ){
                                        containsParent = true;
                                        break;
                                    }
                                }
                            }
                            if( !containsParent ){
                                if(m_d->bill->insertBillItems( NULL, currRow+1, 1, currParent )) {
                                    *(m_d->bill->billItem( m_d->bill->index(currRow+1, 0, currParent ) ) ) = *(*i);
                                }
                            }
                            currRow++;
                        }
                    } else if( mode == QCostClipboardData::Cut ){
                        if( itemsToCopyBill == m_d->bill ){
                            for( QList<BillItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                                bool containsParent = false;
                                for( QList<BillItem *>::iterator j=itemsToCopy.begin(); j != itemsToCopy.end(); ++j ){
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
                                currRow++;
                            }
                        }
                    }
                }
            }
        }
    }
}

void BillTreeGUI::editAttributes(){
    if( m_d->bill ){
        QList<BillItem *> itemsList;
        if( m_d->ui->treeView->selectionModel() ){
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            for( int i=0; i < selRows.size(); ++i ){
                itemsList << m_d->bill->billItem( selRows.at(i)) ;
            }
        }
        BillAttributeChangeDialog dialog( &itemsList, m_d->bill->attributeModel(), this );
        dialog.exec();
    }
}

#include "editpriceitemdialog.h"

void BillTreeGUI::editBillItemPrice( const QModelIndex & index ){
    if( m_d->bill ){
        if( index.column() >= 0 && index.column() < 4 ){
            if( m_d->bill->priceList() == NULL ){
                QMessageBox msgBox;
                msgBox.setText( trUtf8("Al computo non è associato alcun elenco prezzi") );
                msgBox.setInformativeText(trUtf8("Prima di associare un prezzo ad una riga è necessario aver impostato l'elenco prezzi del computo.") );
                msgBox.setStandardButtons(QMessageBox::Ok );
                msgBox.exec();
            } else {
                if( !m_d->bill->billItem( index )->hasChildren() ){
                    EditPriceItemDialog dialog( m_d->EPAImportOptions, m_d->EPAFileName, m_d->bill->priceList(), m_d->bill->priceDataSet(), m_d->bill->billItem( index ), m_d->parser, m_d->project, this );
                    dialog.exec();
                }
            }
        }
    }
}

BillItem *BillTreeGUI::currentBillItem() {
    if( m_d->ui->treeView->selectionModel() ){
        if( m_d->bill != NULL ){
            if( m_d->ui->treeView->selectionModel()->currentIndex().isValid() ){
                return m_d->bill->billItem( m_d->ui->treeView->selectionModel()->currentIndex() );
            }
        }
    }
    return NULL;
}

void BillTreeGUI::setBill(Bill *b ) {
    if( m_d->bill != b ){
        if( m_d->bill != NULL ){
            disconnect( m_d->bill, SIGNAL(amountChanged(int,QString)), this, SLOT(updateAmountValue(int,QString) ));
            disconnect( m_d->ui->priceListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setPriceList()) );
            disconnect( m_d->bill, &Bill::aboutToBeDeleted, this, &BillTreeGUI::clear );
        }
        m_d->ui->priceListComboBox->setCurrentIndex( 0 );
        m_d->ui->currentPriceDataSetSpinBox->setMaximum(1);
        m_d->ui->currentPriceDataSetSpinBox->setValue(1);
        for( int i=0; i < m_d->amountLEditList.size(); ++i){
            if( i < m_d->priceFieldModel->fieldCount() ){
                m_d->amountLEditList.at(i)->clear();
            }
        }
        if( m_d->ui->treeView->selectionModel() ){
            disconnect( m_d->ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(changeCurrentItem(QModelIndex))  );
        }

        m_d->bill = b;

        m_d->ui->treeView->setModel( b );
        if( m_d->bill ){
            for( int i=0; i < m_d->amountLEditList.size(); ++i){
                if( i < m_d->priceFieldModel->fieldCount() ){
                    m_d->amountLEditList.at(i)->setText( m_d->bill->amountStr(i) );
                }
            }
            connect( m_d->bill, SIGNAL(amountChanged(int,QString)), this, SLOT(updateAmountValue(int,QString) ));

            setPriceListComboBox();
            connect( m_d->ui->priceListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setPriceList()) );
            setPriceColSpinBox();
            connect( m_d->bill, &Bill::aboutToBeDeleted, this, &BillTreeGUI::clear );
        }
        if( m_d->ui->treeView->selectionModel() ){
            connect( m_d->ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(changeCurrentItem(QModelIndex))  );
        }
    }
}

void BillTreeGUI::clear(){
    setBill(NULL);
}

void BillTreeGUI::addItems(){
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
            } else {
                m_d->bill->insertBillItems( NULL, 0 );
            }
        }
    }
}

void BillTreeGUI::addChildItems(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QModelIndexList rowListSel = m_d->ui->treeView->selectionModel()->selectedRows();
            for( int i=0; i < rowListSel.size(); ++i){
                m_d->bill->insertBillItems( NULL, 0, 1, rowListSel.at(i) );
            }
        }
    }
}

void BillTreeGUI::removeItems(){
    if( m_d->bill != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            // se la lista non contiene il genitore dell'oggetto lo rimuovo
            // altrimenti è sufficiente rimuovere il genitore
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QModelIndexList selRowsEff;
            for( int i=0; i < selRows.size(); ++i){
                if( !(selRows.contains( selRows.at(i).parent()) ) ){
                    selRowsEff << selRows.at(i);
                }
            }

            for( int i=selRowsEff.size() - 1; i >= 0; --i){
                m_d->bill->removeBillItems( selRowsEff.at(i).row(), 1, selRowsEff.at(i).parent() );
            }
        }
    }
}

void BillTreeGUI::changeCurrentItem(const QModelIndex &currentIndex  ) {
    if( currentIndex.isValid() ){
        if( m_d->bill != NULL ){
            emit currentItemChanged( m_d->bill->billItem( currentIndex ));
            return;
        }
    }
    emit currentItemChanged( NULL );
}

void BillTreeGUI::setPriceList() {
    QVariant v =  m_d->ui->priceListComboBox->itemData( m_d->ui->priceListComboBox->currentIndex() );
    PriceList * currentPriceList = (PriceList *) v.value<void *>();
    if( m_d->bill ){
        if( m_d->bill->priceList() && m_d->bill->rowCount() > 0 ){
            BillSetPriceListModeGUI modeGUI( this );
            Bill::SetPriceListMode plMode = modeGUI.returnValue();
            m_d->bill->setPriceList( currentPriceList, plMode );
        } else {
            m_d->bill->setPriceList( currentPriceList );
        }
        if( m_d->bill->priceList() ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->bill->priceList()->priceDataSetCount() );
        } else {
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        }
    }
}

void BillTreeGUI::setPriceCol() {
    if( m_d->bill ){
        if( m_d->bill->priceList() ){
            if( m_d->ui->currentPriceDataSetSpinBox->value() < 1 ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
            } else if(  m_d->ui->currentPriceDataSetSpinBox->value() > m_d->bill->priceList()->priceDataSetCount() ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->bill->priceList()->priceDataSetCount() );
            } else {
                m_d->bill->setPriceCol( m_d->ui->currentPriceDataSetSpinBox->value()-1 );
            }
        }
    }
}

void BillTreeGUI::updateAmountsNameValue(){
    for( QList<QLabel * >::iterator i = m_d->amountLabelList.begin(); i != m_d->amountLabelList.end(); ++i ){
        m_d->ui->amountsLayout->removeWidget( *i );
        delete *i;
    }
    m_d->amountLabelList.clear();
    for( QList<QLineEdit * >::iterator i = m_d->amountLEditList.begin(); i != m_d->amountLEditList.end(); ++i ){
        m_d->ui->amountsLayout->removeWidget( *i );
        delete *i;
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
        m_d->amountLabelList.append( label );
        m_d->amountLEditList.append( lEdit );
    }
}

void BillTreeGUI::updateAmountValue(int priceField, const QString & newVal ){
    if( priceField > -1 && priceField < m_d->amountLEditList.size() ){
        m_d->amountLEditList.at(priceField)->setText( newVal );
    }
}

void BillTreeGUI::updateAmountName( int priceField, const QString & newName ){
    if( priceField > -1 && priceField < m_d->amountLEditList.size() ){
        m_d->amountLabelList.at(priceField)->setText( newName );
    }
}

void BillTreeGUI::showEvent(QShowEvent *event) {
    if( event->type() == QEvent::Show ){
        disconnect( m_d->ui->priceListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setPriceList()) );
        populatePriceListComboBox();
        setPriceListComboBox();
        connect( m_d->ui->priceListComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setPriceList()) );

        disconnect( m_d->ui->currentPriceDataSetSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setPriceCol()) );
        setPriceColSpinBox();
        connect( m_d->ui->currentPriceDataSetSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setPriceCol()) );
    }
    QWidget::showEvent( event );
}

void BillTreeGUI::populatePriceListComboBox(){
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

void BillTreeGUI::setPriceListComboBox() {
    if( m_d->bill ){
        int i = m_d->ui->priceListComboBox->findData( qVariantFromValue((void *) m_d->bill->priceList() ));
        if( i < 0 ){
            i = 0;
        }
        m_d->ui->priceListComboBox->setCurrentIndex( i );
    }
}

void BillTreeGUI::setPriceColSpinBox() {
    if( m_d->bill ){
        if( m_d->bill->priceList() ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->bill->priceList()->priceDataSetCount() );
        }
        m_d->ui->currentPriceDataSetSpinBox->setValue( m_d->bill->priceDataSet()+1 );
    } else {
        m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
        m_d->ui->currentPriceDataSetSpinBox->setValue( 1 );
    }
}

void BillTreeGUI::resizeColumnsToContents(){
    if( m_d->ui->treeView->model() ){
        for( int i=0; i < m_d->ui->treeView->model()->columnCount(); ++i ){
            m_d->ui->treeView->resizeColumnToContents(i);
        }
    }
}
