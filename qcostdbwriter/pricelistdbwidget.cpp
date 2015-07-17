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
#include "pricelistdbwidget.h"
#include "ui_pricelistdbwidget.h"

#include "loadfromtxtdialog.h"
#include "pricelistdbdelegate.h"

#include "pricelistdbmodel.h"
#include "unitmeasuremodel.h"

#include <QTextStream>
#include <QFileDialog>

#include <QDebug>

class PriceListDBWidgetPrivate{
public:
    PriceListDBWidgetPrivate(PriceListDBModel * m):
        ui(new Ui::PriceListDBWidget),
        model(m){
    }
    ~PriceListDBWidgetPrivate(){
        delete ui;
    }
    Ui::PriceListDBWidget *ui;
    PriceListDBModel * model;
    QPersistentModelIndex currentIndex;
};

PriceListDBWidget::PriceListDBWidget(PriceListDBModel * m, QWidget *parent) :
    QWidget(parent),
    m_d( new PriceListDBWidgetPrivate(m) ){
    m_d->ui->setupUi(this);
    m_d->ui->priceListView->setModel( m_d->model );
    m_d->ui->priceListView->setItemDelegate( new PriceListDBDelegate( m_d->model ) );
    m_d->ui->unitMeasureView->setModel( m_d->model->unitMeasureModel() );
    connect( m_d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateCurrentIndexData(QModelIndex,QModelIndex)) );
    populateUnitMeasureCB();
    connect( m_d->model->unitMeasureModel(), SIGNAL(modelChanged(bool)), this, SLOT(populateUnitMeasureCB()) );

    // sincronizza la descrizione lunga
    m_d->ui->longDescTextEdit->installEventFilter(this);
    connect( m_d->ui->codeLineEdit, SIGNAL(editingFinished()), this, SLOT(setCodeFromLE()) );
    connect( m_d->ui->shortDescLineEdit, SIGNAL(editingFinished()), this, SLOT(setShortDescriptionFromLE()) );
    connect( m_d->ui->unitMeasureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setUnitMeasureFromCB(int)) );
    connect( m_d->ui->priceTotalLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceTotalFromLE()) );
    connect( m_d->ui->priceHumanLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceHumanFromLE()) );
    connect( m_d->ui->priceEquipmentLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceEquipmentFromLE()) );
    connect( m_d->ui->priceMaterialLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceMaterialFromLE()) );
    connect( m_d->ui->overheadsLineEdit, SIGNAL(editingFinished()), this, SLOT(setOverheadsFromLE()) );
    connect( m_d->ui->profitsLineEdit, SIGNAL(editingFinished()), this, SLOT(setProfitsFromLE()) );

    connect( m_d->ui->priceListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(setCurrentPrice(QModelIndex)) );

    connect( m_d->ui->addPLPushButton, SIGNAL(clicked()), this, SLOT(addPLItems()));
    connect( m_d->ui->addPLChildPushButton, SIGNAL(clicked()), this, SLOT(addPLChildItems()));
    connect( m_d->ui->delPLPushButton, SIGNAL(clicked()), this, SLOT(delPLItems()));
    connect( m_d->ui->addUMPushButton, SIGNAL(clicked()), this, SLOT(addUMItems()));
    connect( m_d->ui->delUMPushButton, SIGNAL(clicked()), this, SLOT(delUMItems()));
}

PriceListDBWidget::~PriceListDBWidget() {
    delete m_d;
}

void PriceListDBWidget::clear() {
    disconnect( m_d->ui->codeLineEdit, SIGNAL(editingFinished()), this, SLOT(setCodeFromLE()) );
    disconnect( m_d->ui->shortDescLineEdit, SIGNAL(editingFinished()), this, SLOT(setShortDescriptionFromLE()) );
    disconnect( m_d->ui->unitMeasureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setUnitMeasureFromCB(int)) );
    disconnect( m_d->ui->priceTotalLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceTotalFromLE()) );
    disconnect( m_d->ui->priceHumanLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceHumanFromLE()) );
    disconnect( m_d->ui->priceEquipmentLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceEquipmentFromLE()) );
    disconnect( m_d->ui->priceMaterialLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceMaterialFromLE()) );
    disconnect( m_d->ui->overheadsLineEdit, SIGNAL(editingFinished()), this, SLOT(setOverheadsFromLE()) );
    disconnect( m_d->ui->profitsLineEdit, SIGNAL(editingFinished()), this, SLOT(setProfitsFromLE()) );

    m_d->currentIndex = QPersistentModelIndex();

    m_d->ui->codeLineEdit->clear();
    m_d->ui->shortDescLineEdit->clear();
    m_d->ui->longDescTextEdit->clear();
    m_d->ui->unitMeasureComboBox->clear();
    m_d->ui->priceTotalLineEdit->clear();
    m_d->ui->priceHumanLineEdit->clear();
    m_d->ui->priceEquipmentLineEdit->clear();
    m_d->ui->priceMaterialLineEdit->clear();
    m_d->ui->overheadsLineEdit->clear();
    m_d->ui->profitsLineEdit->clear();

    connect( m_d->ui->codeLineEdit, SIGNAL(editingFinished()), this, SLOT(setCodeFromLE()) );
    connect( m_d->ui->shortDescLineEdit, SIGNAL(editingFinished()), this, SLOT(setShortDescriptionFromLE()) );
    connect( m_d->ui->unitMeasureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setUnitMeasureFromCB(int)) );
    connect( m_d->ui->priceTotalLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceTotalFromLE()) );
    connect( m_d->ui->priceHumanLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceHumanFromLE()) );
    connect( m_d->ui->priceEquipmentLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceEquipmentFromLE()) );
    connect( m_d->ui->priceMaterialLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceMaterialFromLE()) );
    connect( m_d->ui->overheadsLineEdit, SIGNAL(editingFinished()), this, SLOT(setOverheadsFromLE()) );
    connect( m_d->ui->profitsLineEdit, SIGNAL(editingFinished()), this, SLOT(setProfitsFromLE()) );
}

bool PriceListDBWidget::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::FocusOut)     {
        if (object == m_d->ui->longDescTextEdit)  {
            m_d->model->setLongDescription( m_d->ui->longDescTextEdit->toPlainText(), m_d->currentIndex );
        }
    }
    return false;
}

void PriceListDBWidget::setCurrentPrice(const QModelIndex &current ) {
    if( current.isValid() ){
        m_d->currentIndex = QPersistentModelIndex( current );
        m_d->ui->codeLineEdit->setText( m_d->model->code(m_d->currentIndex) );
        m_d->ui->shortDescLineEdit->setText( m_d->model->shortDescription(m_d->currentIndex) );
        m_d->ui->longDescTextEdit->setPlainText( m_d->model->longDescription(m_d->currentIndex) );

        populateUnitMeasureCB();

        m_d->ui->priceTotalLineEdit->setText( m_d->model->locale()->toString( m_d->model->priceTotal(m_d->currentIndex), 'f', 2) );
        m_d->ui->priceHumanLineEdit->setText( m_d->model->locale()->toString( m_d->model->priceHuman(m_d->currentIndex), 'f', 2) );
        m_d->ui->priceEquipmentLineEdit->setText( m_d->model->locale()->toString( m_d->model->priceEquipment(m_d->currentIndex), 'f', 2) );
        m_d->ui->priceMaterialLineEdit->setText( m_d->model->locale()->toString( m_d->model->priceMaterial(m_d->currentIndex), 'f', 2) );
        m_d->ui->overheadsLineEdit->setText( m_d->model->locale()->toString( m_d->model->overheads(m_d->currentIndex), 'f', 4) );
        m_d->ui->profitsLineEdit->setText( m_d->model->locale()->toString( m_d->model->profits(m_d->currentIndex), 'f', 4) );
    }
}

void PriceListDBWidget::updateCurrentIndexData(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
    if( topLeft.parent() == m_d->currentIndex.parent() ){
        if( m_d->currentIndex.row() >= topLeft.row() && m_d->currentIndex.row() <= bottomRight.row() ){
            disconnect( m_d->ui->codeLineEdit, SIGNAL(editingFinished()), this, SLOT(setCodeFromLE()) );
            disconnect( m_d->ui->shortDescLineEdit, SIGNAL(editingFinished()), this, SLOT(setShortDescriptionFromLE()) );
            disconnect( m_d->ui->unitMeasureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setUnitMeasureFromCB(int)) );
            disconnect( m_d->ui->priceTotalLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceTotalFromLE()) );
            disconnect( m_d->ui->priceHumanLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceHumanFromLE()) );
            disconnect( m_d->ui->priceEquipmentLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceEquipmentFromLE()) );
            disconnect( m_d->ui->priceMaterialLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceMaterialFromLE()) );
            disconnect( m_d->ui->overheadsLineEdit, SIGNAL(editingFinished()), this, SLOT(setOverheadsFromLE()) );
            disconnect( m_d->ui->profitsLineEdit, SIGNAL(editingFinished()), this, SLOT(setProfitsFromLE()) );

            m_d->ui->codeLineEdit->setText( m_d->model->code(m_d->currentIndex) );
            m_d->ui->shortDescLineEdit->setText( m_d->model->shortDescription(m_d->currentIndex) );
            m_d->ui->longDescTextEdit->setPlainText( m_d->model->longDescription(m_d->currentIndex) );
            m_d->ui->unitMeasureComboBox->setCurrentIndex( m_d->ui->unitMeasureComboBox->findData( QVariant(m_d->model->unitMeasure(m_d->currentIndex)) ) );
            m_d->ui->priceTotalLineEdit->setText( m_d->model->locale()->toString( m_d->model->priceTotal(m_d->currentIndex), 'f', 2) );
            m_d->ui->priceHumanLineEdit->setText( m_d->model->locale()->toString( m_d->model->priceHuman(m_d->currentIndex), 'f', 2) );
            m_d->ui->priceEquipmentLineEdit->setText( m_d->model->locale()->toString( m_d->model->priceEquipment(m_d->currentIndex), 'f', 2) );
            m_d->ui->priceMaterialLineEdit->setText( m_d->model->locale()->toString( m_d->model->priceMaterial(m_d->currentIndex), 'f', 2) );
            m_d->ui->overheadsLineEdit->setText( m_d->model->locale()->toString( m_d->model->overheads(m_d->currentIndex), 'f', 4) );
            m_d->ui->profitsLineEdit->setText( m_d->model->locale()->toString( m_d->model->profits(m_d->currentIndex), 'f', 4) );

            connect( m_d->ui->codeLineEdit, SIGNAL(editingFinished()), this, SLOT(setCodeFromLE()) );
            connect( m_d->ui->shortDescLineEdit, SIGNAL(editingFinished()), this, SLOT(setShortDescriptionFromLE()) );
            connect( m_d->ui->unitMeasureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setUnitMeasureFromCB(int)) );
            connect( m_d->ui->priceTotalLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceTotalFromLE()) );
            connect( m_d->ui->priceHumanLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceHumanFromLE()) );
            connect( m_d->ui->priceEquipmentLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceEquipmentFromLE()) );
            connect( m_d->ui->priceMaterialLineEdit, SIGNAL(editingFinished()), this, SLOT(setPriceMaterialFromLE()) );
            connect( m_d->ui->overheadsLineEdit, SIGNAL(editingFinished()), this, SLOT(setOverheadsFromLE()) );
            connect( m_d->ui->profitsLineEdit, SIGNAL(editingFinished()), this, SLOT(setProfitsFromLE()) );
        }
    }
}

void PriceListDBWidget::setCodeFromLE() {
    m_d->model->setCode( m_d->ui->codeLineEdit->text(), m_d->currentIndex );
}

void PriceListDBWidget::setShortDescriptionFromLE() {
    m_d->model->setShortDescription( m_d->ui->shortDescLineEdit->text(), m_d->currentIndex );
}

void PriceListDBWidget::setUnitMeasureFromCB(int curIndex ) {
    m_d->model->setUnitMeasure( m_d->ui->unitMeasureComboBox->itemData( curIndex ).toInt(), m_d->currentIndex );
}

void PriceListDBWidget::setPriceTotalFromLE() {
    m_d->model->setPriceTotal( m_d->model->locale()->toDouble( m_d->ui->priceTotalLineEdit->text() ), m_d->currentIndex );
}

void PriceListDBWidget::setPriceHumanFromLE(){
    m_d->model->setPriceHuman( m_d->model->locale()->toDouble( m_d->ui->priceHumanLineEdit->text() ), m_d->currentIndex );
}

void PriceListDBWidget::setPriceEquipmentFromLE(){
    m_d->model->setPriceEquipment( m_d->model->locale()->toDouble( m_d->ui->priceEquipmentLineEdit->text() ), m_d->currentIndex );
}

void PriceListDBWidget::setPriceMaterialFromLE(){
    m_d->model->setPriceMaterial( m_d->model->locale()->toDouble( m_d->ui->priceMaterialLineEdit->text() ), m_d->currentIndex );
}

void PriceListDBWidget::setOverheadsFromLE(){
    m_d->model->setOverheads( m_d->model->locale()->toDouble( m_d->ui->overheadsLineEdit->text() ), m_d->currentIndex );
}

void PriceListDBWidget::setProfitsFromLE(){
    m_d->model->setProfits( m_d->model->locale()->toDouble( m_d->ui->overheadsLineEdit->text() ), m_d->currentIndex );
}

void PriceListDBWidget::populateUnitMeasureCB() {
    disconnect( m_d->ui->unitMeasureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setUnitMeasureFromCB(int)) );

    m_d->ui->unitMeasureComboBox->clear();
    QList< QPair<QString, int> > umList = m_d->model->unitMeasureList();
    for( int i=0; i < umList.size(); ++i){
        m_d->ui->unitMeasureComboBox->addItem( umList.at(i).first, QVariant(umList.at(i).second) );
    }
    if( m_d->currentIndex.isValid() ){
        m_d->ui->unitMeasureComboBox->setCurrentIndex( m_d->ui->unitMeasureComboBox->findData( QVariant(m_d->model->unitMeasure(m_d->currentIndex)) ) );
    } else {
        m_d->ui->unitMeasureComboBox->setCurrentIndex(0);
    }

    connect( m_d->ui->unitMeasureComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setUnitMeasureFromCB(int)) );
}

void PriceListDBWidget::addPLItems(){
    if( m_d->ui->priceListView->selectionModel() ){
        int count = m_d->ui->priceListView->selectionModel()->selectedRows().size() > 0? m_d->ui->priceListView->selectionModel()->selectedRows().size(): 1;
        if( m_d->ui->priceListView->selectionModel()->currentIndex().isValid() ){
            m_d->model->insertRows( m_d->ui->priceListView->selectionModel()->currentIndex().row() + 1, count, m_d->ui->priceListView->selectionModel()->currentIndex().parent());
        } else {
            m_d->model->insertRows( 0, count, QModelIndex() );
        }
    }
}

void PriceListDBWidget::addPLChildItems(){
    if( m_d->ui->priceListView->selectionModel() ){
        int count = m_d->ui->priceListView->selectionModel()->selectedRows().size() > 0? m_d->ui->priceListView->selectionModel()->selectedRows().size(): 1;
        QModelIndex currIndex = m_d->ui->priceListView->selectionModel()->currentIndex();
        if( currIndex.isValid() ){
            m_d->model->insertRows( m_d->model->rowCount( currIndex ), count, currIndex );
        }
    }
}

void PriceListDBWidget::delPLItems(){
    if( m_d->ui->priceListView->selectionModel() ){
        QModelIndexList selRows = m_d->ui->priceListView->selectionModel()->selectedRows();
        if( selRows.size() > 0 ){
            // prendiamo le righe contenute nella selezione, senza ripetizioni,
            // verificando che i relativi genitori non siano già contenuti
            // (se si canella il genitori sono rimossi automaticamente anche i figli)
            QList< QPair<QModelIndex, int> > rowToDel;
            for( int i=0; i < selRows.size(); ++i ){
                if( !selRows.contains( selRows.at(i).parent() )){
                    QPair<QModelIndex, int> v = qMakePair( selRows.at(i).parent(), selRows.at(i).row());
                    if( !rowToDel.contains(v) ){
                        rowToDel.append( v );
                    }
                }
            }
            // riordina rowToDel
            // devono essere rimosse prima le righe con indice piu alto
            for( int i=0; i < rowToDel.size(); ++i ){
                for( int j=i+1; j < rowToDel.size(); ++j ){
                    if( (rowToDel.at(i).first.parent() == rowToDel.at(j).first.parent()) &&
                            (rowToDel.at(i).second < rowToDel.at(j).second ) ){
                        rowToDel.swap( i, j );
                    }
                }
            }
            // adesso possiamo procedere con la rimozione
            for( int i=0; i < rowToDel.size(); ++i ){
                m_d->model->removeRows( rowToDel.at(i).second, 1, rowToDel.at(i).first);
            }
        }
    }
}

void PriceListDBWidget::addUMItems(){
    if( m_d->ui->unitMeasureView->selectionModel() ){
        int count = m_d->ui->unitMeasureView->selectionModel()->selectedRows().size() > 0? m_d->ui->unitMeasureView->selectionModel()->selectedRows().size(): 1;
        if( m_d->ui->unitMeasureView->selectionModel()->currentIndex().isValid() ){
            m_d->model->insertRows( m_d->ui->unitMeasureView->selectionModel()->currentIndex().row() + 1, count );
        } else {
            m_d->model->insertRows( 0, count, QModelIndex() );
        }
    }
}

void PriceListDBWidget::delUMItems(){
    if( m_d->ui->unitMeasureView->selectionModel() ){
        QModelIndexList selRows = m_d->ui->unitMeasureView->selectionModel()->selectedRows();
        if( selRows.size() > 0 ){
            // prendiamo le righe contenute nella selezione, senza ripetizioni,
            // verificando che i relativi genitori non siano già contenuti
            // (se si canella il genitori sono rimossi automaticamente anche i figli)
            QList< int > rowToDel;
            for( int i=0; i < selRows.size(); ++i ){
                if( !rowToDel.contains( selRows.at(i).row() ) ){
                    rowToDel.append( selRows.at(i).row() );
                }
            }
            // riordina rowToDel
            // devono essere rimosse prima le righe con indice piu alto
            for( int i=0; i < rowToDel.size(); ++i ){
                for( int j=i+1; j < rowToDel.size(); ++j ){
                    if( rowToDel.at(i) < rowToDel.at(j) ){
                        rowToDel.swap( i, j );
                    }
                }
            }
            // adesso possiamo procedere con la rimozione
            for( int i=0; i < rowToDel.size(); ++i ){
                m_d->model->removeRows( rowToDel.at(i), 1 );
            }
        }
    }
}
