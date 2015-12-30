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
#include "pricelisttreegui.h"
#include "ui_pricelisttreegui.h"

#include "pricelistdelegate.h"

#include "importpriceitemdbdialog.h"
#include "qcostclipboarddata.h"
#include "project.h"
#include "pricelist.h"
#include "priceitem.h"
#include "unitmeasuremodel.h"

#include <QClipboard>
#include <QMenu>
#include <QAction>

class PriceListTreeGUIPrivate{
public:
    PriceListTreeGUIPrivate( QMap<PriceListDBWidget::ImportOptions, bool> * impOptions,
                             QString *fileName,
                             MathParser * prs, PriceFieldModel * pfm, UnitMeasureModel * uml ):
        priceList(NULL),
        parser(prs),
        priceFieldModel(pfm),
        unitMeasureModel(uml),
        ui(new Ui::PriceListTreeGUI),
        EPAImportOptions(impOptions),
        EPAFileName(fileName),
        currentPriceDataSet(-1){
    }
    ~PriceListTreeGUIPrivate(){
        delete ui;
    }

    PriceList * priceList;
    MathParser * parser;
    PriceFieldModel * priceFieldModel;
    UnitMeasureModel * unitMeasureModel;
    Ui::PriceListTreeGUI *ui;

    QMap<PriceListDBWidget::ImportOptions, bool> * EPAImportOptions;
    QString * EPAFileName;
    int currentPriceDataSet;
};

PriceListTreeGUI::PriceListTreeGUI(QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                                   QString *EPAFileName,
                                   PriceList * prList, int curPriceDataSet,
                                   MathParser * prs, PriceFieldModel * pfm, UnitMeasureModel * uml,
                                   QWidget *parent ) :
    QWidget(parent),
    m_d(new PriceListTreeGUIPrivate( EPAImpOptions, EPAFileName, prs, pfm, uml ) ) {

    m_d->ui->setupUi(this);

    m_d->ui->treeView->setItemDelegate( new PriceListDelegate( m_d->unitMeasureModel ));

    setPriceList( prList, curPriceDataSet );
    connect( m_d->priceFieldModel, &PriceFieldModel::endInsertPriceField, this, &PriceListTreeGUI::updateCurrentPriceDataSet );
    connect( m_d->priceFieldModel, &PriceFieldModel::endRemovePriceField, this, &PriceListTreeGUI::updateCurrentPriceDataSet );

    connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &PriceListTreeGUI::setCurrentPriceDataSetFromSpinBox );

    connect( m_d->ui->addPushButton, &QPushButton::clicked, this, &PriceListTreeGUI::addItems );
    connect( m_d->ui->addChildPushButton, &QPushButton::clicked, this, &PriceListTreeGUI::addChildItems );
    connect( m_d->ui->removePushButton, &QPushButton::clicked, this, &PriceListTreeGUI::removeItems );
    connect( m_d->ui->importPIDBPushButton, &QPushButton::clicked, this, static_cast<void(PriceListTreeGUI::*)( )>(&PriceListTreeGUI::importMultiPriceItemDB) );

    m_d->ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->treeView, &QTreeView::customContextMenuRequested, this, &PriceListTreeGUI::treeViewCustomMenuRequested );
}

PriceListTreeGUI::~PriceListTreeGUI() {
    delete m_d;
}

void PriceListTreeGUI::setPriceList(PriceList * pl, int priceDataSet) {
    if( pl != m_d->priceList || priceDataSet != m_d->currentPriceDataSet ){
        if( m_d->priceList != NULL ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( 1 );
            disconnect( m_d->priceList, &PriceList::priceDataSetCountChanged, this, &PriceListTreeGUI::setCurrentPriceDataSetSpinBoxMaximum );
            disconnect( m_d->priceList, &PriceList::aboutToBeDeleted, this, &PriceListTreeGUI::setPriceListNULL );
            disconnect( m_d->priceList, &PriceList::priceDataSetCountChanged, this, &PriceListTreeGUI::updateCurrentPriceDataSet );
        }
        m_d->ui->treeView->setModel( pl );
        m_d->priceList = pl;
        if( m_d->priceList != NULL ){
            m_d->ui->currentPriceDataSetSpinBox->setMaximum( m_d->priceList->priceDataSetCount() );
            m_d->ui->currentPriceDataSetSpinBox->setValue( priceDataSet+1 );
            connect( m_d->priceList, &PriceList::priceDataSetCountChanged, this, &PriceListTreeGUI::setCurrentPriceDataSetSpinBoxMaximum );
            connect( m_d->priceList, &PriceList::aboutToBeDeleted, this, &PriceListTreeGUI::setPriceListNULL );
            connect( m_d->priceList, &PriceList::priceDataSetCountChanged, this, &PriceListTreeGUI::updateCurrentPriceDataSet );

            connect( m_d->ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &PriceListTreeGUI::changeCurrentItem );
        }
        if( m_d->priceList != NULL ){
            setCurrentPriceDataSet( priceDataSet );
        } else {
            setCurrentPriceDataSet( 0 );
        }
    }
}

void PriceListTreeGUI::setPriceListNULL(){
    setPriceList( NULL );
}

void PriceListTreeGUI::setCurrentPriceDataSet( int newCurrPriceDataSet ){
    if( m_d->priceList != NULL ){
        if( (newCurrPriceDataSet != m_d->currentPriceDataSet) &&
                newCurrPriceDataSet < m_d->priceList->priceDataSetCount() ){
            m_d->currentPriceDataSet = newCurrPriceDataSet;
            emit currentPriceDataSetChanged( m_d->currentPriceDataSet );
            updateCurrentPriceDataSet();
            if( (m_d->ui->currentPriceDataSetSpinBox->value() - 1) != newCurrPriceDataSet ){
                m_d->ui->currentPriceDataSetSpinBox->setValue( newCurrPriceDataSet + 1 );
            }
        }
    }
}

void PriceListTreeGUI::updateCurrentPriceDataSet(){
    if( m_d->priceList != NULL ){
        if( m_d->currentPriceDataSet >= m_d->priceList->priceDataSetCount() ){
            m_d->currentPriceDataSet = m_d->priceList->priceDataSetCount() - 1;
        }
        int col = m_d->priceList->firstValueCol();
        for( int i=0; i < m_d->priceList->priceDataSetCount(); ++i ){
            for( int j=0; j < m_d->priceFieldModel->fieldCount(); ++j ){
                m_d->ui->treeView->setColumnHidden( col, i != m_d->currentPriceDataSet );
                col++;
            }
        }
    }
}

void PriceListTreeGUI::setCurrentPriceDataSetSpinBoxMaximum(int newPriceDataSetCount ){
    m_d->ui->currentPriceDataSetSpinBox->setMaximum( newPriceDataSetCount );
}

void PriceListTreeGUI::setCurrentPriceDataSetFromSpinBox(int newPriceDataSet ){
    setCurrentPriceDataSet( newPriceDataSet - 1 );
}

PriceItem *PriceListTreeGUI::currentPriceItem() {
    if( m_d->ui->treeView->selectionModel() ){
        if(  m_d->priceList != NULL ){
            if( m_d->ui->treeView->selectionModel()->currentIndex().isValid() ){
                return m_d->priceList->priceItem( m_d->ui->treeView->selectionModel()->currentIndex() );
            }
        }
    }
    return NULL;
}

void PriceListTreeGUI::addItems(){
    if( m_d->priceList != NULL ){
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
                m_d->priceList->insertPriceItems( rowList.last().row()+1, rowList.size(), rowList.last().parent() );
                if( m_d->ui->treeView->selectionModel() ){
                    m_d->ui->treeView->selectionModel()->clearSelection();
                    m_d->ui->treeView->selectionModel()->setCurrentIndex( m_d->priceList->index( rowList.last().row()+rowList.size(), 0, rowList.last().parent() ),
                                                                          QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent );
                }
            } else {
                m_d->priceList->insertPriceItems( 0 );
                if( m_d->ui->treeView->selectionModel() ){
                    m_d->ui->treeView->selectionModel()->clearSelection();
                    m_d->ui->treeView->selectionModel()->setCurrentIndex( m_d->priceList->index( 0, 0, QModelIndex() ),
                                                                          QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent );
                }
            }
        }
    }
}

void PriceListTreeGUI::addChildItems(){
    if( m_d->priceList != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QModelIndexList rowListSel = m_d->ui->treeView->selectionModel()->selectedRows();
            for( int i=0; i < rowListSel.size(); ++i){
                m_d->priceList->insertPriceItems( 0, 1, rowListSel.at(i) );
            }
            if( m_d->ui->treeView->selectionModel() ){
                m_d->ui->treeView->selectionModel()->clearSelection();
                m_d->ui->treeView->selectionModel()->setCurrentIndex( m_d->priceList->index( 0, 0, rowListSel.last() ),
                                                                      QItemSelectionModel::Rows | QItemSelectionModel::SelectCurrent );
            }
        }
    }
}

void PriceListTreeGUI::removeItems(){
    if( m_d->priceList != NULL ){
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
                m_d->priceList->removeRows( selRowsEff.at(i).row(), 1, selRowsEff.at(i).parent() );
            }
        }
    }
}

void PriceListTreeGUI::copyToClipboard(){
    if( m_d->priceList != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QCostClipboardData *data = new QCostClipboardData();
            const QCostClipboardData *clipData = qobject_cast<const QCostClipboardData *>(QApplication::clipboard()->mimeData());
            if( clipData != NULL ){
                *data = *clipData;
            }
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QList<PriceItem *> copiedPriceItems;
            for( int i=0; i < selRows.size(); ++i ){
                copiedPriceItems << m_d->priceList->priceItem( selRows.at(i)) ;
            }
            data->setCopiedPriceItems( copiedPriceItems, m_d->priceList, QCostClipboardData::Copy );
            QApplication::clipboard()->setMimeData( data );
        }
    }
}

void PriceListTreeGUI::cutToClipboard(){
    if( m_d->priceList != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QCostClipboardData *data = new QCostClipboardData();
            const QCostClipboardData *clipData = qobject_cast<const QCostClipboardData *>(QApplication::clipboard()->mimeData());
            if( clipData != NULL ){
                *data = *clipData;
            }
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            QList<PriceItem *> copiedPriceItems;
            for( int i=0; i < selRows.size(); ++i ){
                copiedPriceItems << m_d->priceList->priceItem( selRows.at(i)) ;
            }
            data->setCopiedPriceItems( copiedPriceItems, m_d->priceList, QCostClipboardData::Cut );
            QApplication::clipboard()->setMimeData( data );
        }
    }
}

void PriceListTreeGUI::pasteFromClipboard(){
    if( m_d->priceList != NULL ){
        if( m_d->ui->treeView->selectionModel() ){
            QClipboard * clp = QApplication::clipboard();
            const QMimeData * mimeData = clp->mimeData();
            const QCostClipboardData *data = qobject_cast<const QCostClipboardData *>( mimeData );

            if( data != NULL ){
                QModelIndex currIndex = m_d->ui->treeView->currentIndex();
                int currRow = m_d->priceList->rowCount( )-1;
                QModelIndex currParent = QModelIndex();
                if( currIndex.isValid() ){
                    currRow = currIndex.row();
                    currParent = currIndex.parent();
                }
                QList<PriceItem *> itemsToCopy;
                PriceList * itemsToCopyPriceList = NULL;
                QCostClipboardData::Mode mode;
                data->getCopiedPriceItems( &itemsToCopy, itemsToCopyPriceList, &mode);
                if( itemsToCopyPriceList != NULL ){
                    if( mode == QCostClipboardData::Copy ){
                        for( QList<PriceItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                            bool containsParent = false;
                            for( QList<PriceItem *>::iterator j=itemsToCopy.begin(); j != itemsToCopy.end(); ++j ){
                                if( i != j ){
                                    if( (*i)->isDescending(*j) ){
                                        containsParent = true;
                                        break;
                                    }
                                }
                            }
                            if( !containsParent ){
                                if( m_d->priceList->insertRows( currRow+1, 1, currParent )) {
                                    *(m_d->priceList->priceItem( m_d->priceList->index(currRow+1, 0, currParent ) ) ) = *(*i);
                                }
                            }
                            currRow++;
                        }
                    } else if( mode == QCostClipboardData::Cut ){
                        if( itemsToCopyPriceList == m_d->priceList ){
                            for( QList<PriceItem *>::iterator i=itemsToCopy.begin(); i != itemsToCopy.end(); ++i ){
                                bool containsParent = false;
                                for( QList<PriceItem *>::iterator j=itemsToCopy.begin(); j != itemsToCopy.end(); ++j ){
                                    if( i != j ){
                                        if( (*i)->isDescending(*j) ){
                                            containsParent = true;
                                            break;
                                        }
                                    }
                                }
                                if( !containsParent ){
                                    m_d->priceList->moveRows( m_d->priceList->index((*i)->parentItem(), 0), (*i)->childNumber(), 1, currParent, currRow+1);
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

void PriceListTreeGUI::changeCurrentItem( QModelIndex currentIndex  ) {
    if( currentIndex.isValid() ){
        if( m_d->priceList != NULL ){
            emit currentItemChanged( m_d->priceList->priceItem( currentIndex ), m_d->currentPriceDataSet );
            return;
        }
    }
    emit currentItemChanged( NULL, m_d->currentPriceDataSet );
}

void PriceListTreeGUI::setCurrentPriceItem(PriceItem *pItem ) {
    m_d->ui->treeView->setCurrentIndex( m_d->priceList->index( pItem, 0 ) );
}

void PriceListTreeGUI::treeViewCustomMenuRequested(QPoint pos){
    QMenu *menu=new QMenu(this);
    QAction * addAction = new QAction( trUtf8("Aggiungi"), this);
    connect( addAction, &QAction::triggered, this, &PriceListTreeGUI::addItems );
    menu->addAction( addAction );
    QAction * addChildAction = new QAction( trUtf8("Aggiungi ▼"), this);
    connect( addChildAction, &QAction::triggered, this, &PriceListTreeGUI::addChildItems );
    menu->addAction( addChildAction );
    QAction * delAction = new QAction( trUtf8("Elimina"), this);
    connect( delAction, &QAction::triggered, this, &PriceListTreeGUI::removeItems );
    menu->addAction( delAction );

    menu->addSeparator();
    QAction * cutAction = new QAction( trUtf8("Taglia"), this);
    connect( cutAction, &QAction::triggered, this, &PriceListTreeGUI::cutToClipboard );
    menu->addAction( cutAction );
    QAction * copyAction = new QAction( trUtf8("Copia"), this);
    connect( copyAction, &QAction::triggered, this, &PriceListTreeGUI::copyToClipboard );
    menu->addAction( copyAction );
    QAction * pasteAction = new QAction( trUtf8("Incolla"), this);
    connect( pasteAction, &QAction::triggered, this, &PriceListTreeGUI::pasteFromClipboard );
    menu->addAction( pasteAction );

    menu->addSeparator();
    QAction * resizeColToContent = new QAction( trUtf8("Ottimizza colonne"), this);
    connect( resizeColToContent, &QAction::triggered, this, &PriceListTreeGUI::resizeColumnsToContents );
    menu->addAction( resizeColToContent );

    menu->popup( m_d->ui->treeView->viewport()->mapToGlobal(pos) );
}

void PriceListTreeGUI::resizeColumnsToContents(){
    if( m_d->ui->treeView->model() ){
        for( int i=0; i < m_d->ui->treeView->model()->columnCount(); ++i ){
            m_d->ui->treeView->resizeColumnToContents(i);
        }
    }
}

void PriceListTreeGUI::importMultiPriceItemDB() {
    ImportPriceItemDBDialog dialog( m_d->EPAImportOptions, m_d->EPAFileName, "PriceListTreeGUI::importMultiPriceItemDB", m_d->parser, m_d->unitMeasureModel, this );
    connect( &dialog, static_cast<void (ImportPriceItemDBDialog::*)(const QList< QList< QPair<QString, QVariant> > > &, const QList<int> &) > (&ImportPriceItemDBDialog::importMultiPriceItemDB),
             this, static_cast<void (PriceListTreeGUI::*)(const QList< QList< QPair<QString, QVariant> > > &, const QList<int> &) > (&PriceListTreeGUI::importMultiPriceItemDB) );
    dialog.exec();
}

void PriceListTreeGUI::importMultiPriceItemDB(const QList<QList<QPair<QString, QVariant> > > & itemDataList, const QList<int> & hierarchy ) {
    if( m_d->ui->treeView->selectionModel() ){
        if( m_d->priceList != NULL ){
            QModelIndexList selRows = m_d->ui->treeView->selectionModel()->selectedRows();
            if( selRows.size() > 0 ){
                QModelIndex currIndex = selRows.last();
                if( currIndex.isValid() ){
                    loadMultiPriceItemDB( itemDataList, hierarchy, m_d->currentPriceDataSet, currIndex.parent(), currIndex.row()+1, -1 );
                    return;
                }
            }
            loadMultiPriceItemDB( itemDataList, hierarchy, m_d->currentPriceDataSet, QModelIndex(), m_d->priceList->rowCount(), -1 );
        }
    }
}

void PriceListTreeGUI::loadMultiPriceItemDB( const QList<QList<QPair<QString, QVariant> > > & itemDataList,
                                             const QList<int> & hierarchy,
                                             int priceDataSet,
                                             const QModelIndex & parent,
                                             int row, int relParent ) {
    int currI = hierarchy.indexOf( relParent );

    while (currI > -1 ) {

        if( m_d->priceList->insertPriceItem( row, parent ) ){
            QModelIndex currIndex = m_d->priceList->index( row, 0, parent );
            PriceItem * currItem = m_d->priceList->priceItem( currIndex );
            currItem->setInheritCodeFromParent( false );

            for(int j=0; j < itemDataList.at(currI).size(); ++j){
                QList<QPair<QString, QVariant> > data = itemDataList.at(currI);
                if( data.at(j).first.toUpper() == "CODE" ){
                    currItem->setCode( data.at(j).second.toString());
                } else if( data.at(j).first.toUpper() == "SHORTDESC" ){
                    currItem->setShortDescription( data.at(j).second.toString());
                } else if( data.at(j).first.toUpper() == "LONGDESC" ){
                    currItem->setLongDescription( data.at(j).second.toString());
                } else if( data.at(j).first.toUpper() == "UNITMEASURE" ){
                    if( data.at(j).second.toString() == "---"){
                        currItem->setUnitMeasure( NULL );
                    } else {
                        int umRow = m_d->unitMeasureModel->findTag( data.at(j).second.toString() );
                        if( umRow < 0 ){
                            umRow = m_d->unitMeasureModel->append( data.at(j).second.toString() );
                        }
                        if( umRow > -1 ){
                            currItem->setUnitMeasure( m_d->unitMeasureModel->unitMeasure(umRow) );
                        }
                    }
                } else if( data.at(j).first.toUpper() == "PRICETOTAL" ){
                    currItem->setValue( PriceFieldModel::PriceTotal, priceDataSet, data.at(j).second.toDouble());
                } else if( data.at(j).first.toUpper() == "PRICEHUMAN" ){
                    currItem->setValue( PriceFieldModel::PriceHuman, priceDataSet, data.at(j).second.toDouble());
                } else if( data.at(j).first.toUpper() == "PRICEEQUIPMENT" ){
                    currItem->setValue( PriceFieldModel::PriceEquipment, priceDataSet, data.at(j).second.toDouble());
                } else if( data.at(j).first.toUpper() == "PRICEMATERIAL" ){
                    currItem->setValue( PriceFieldModel::PriceMaterial, priceDataSet, data.at(j).second.toDouble());
                } else if( data.at(j).first.toUpper() == "OVERHEADS" ){
                    currItem->setInheritOverheadsFromRoot( priceDataSet, false );
                    currItem->setOverheads( priceDataSet, data.at(j).second.toDouble() );
                } else if( data.at(j).first.toUpper() == "PROFITS" ){
                    currItem->setInheritProfitsFromRoot( priceDataSet, false );
                    currItem->setProfits( priceDataSet, data.at(j).second.toDouble() );
                }
            }
            loadMultiPriceItemDB( itemDataList, hierarchy, priceDataSet, currIndex, m_d->priceList->rowCount( currIndex ), currI );
        }
        currI = hierarchy.indexOf( relParent, currI + 1 );
        row++;
    }
}
