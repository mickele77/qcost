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
#include "generaldatagui.h"
#include "ui_generaldatagui.h"

#include "pricefieldtabledelegate.h"
#include "project.h"
#include "unitmeasuremodel.h"
#include "pricefieldmodel.h"

class GeneralDataGUIPrivate{
public:
    GeneralDataGUIPrivate():
        ui(new Ui::GeneralDataGUI),
        fieldTableDelegate( new PriceFieldTableDelegate() ),
        project( NULL ){
    };
    ~GeneralDataGUIPrivate(){
        delete ui;
    }

    Ui::GeneralDataGUI *ui;
    PriceFieldTableDelegate * fieldTableDelegate;
    Project * project;
};

GeneralDataGUI::GeneralDataGUI(Project *p, QWidget *parent) :
    QWidget(parent),
    m_d( new GeneralDataGUIPrivate() ){
    m_d->ui->setupUi(this);

    setProject(p);

    m_d->ui->unitMeasureView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    connect( m_d->ui->unitMeasureAddPButton, &QPushButton::clicked, this, &GeneralDataGUI::insertUnitMeasure );
    connect( m_d->ui->unitMeasureDelPButton, &QPushButton::clicked, this, &GeneralDataGUI::removeUnitMeasure );

    connect( m_d->ui->addPriceFieldPushButton, &QPushButton::clicked, this, &GeneralDataGUI::insertPriceField );
    connect( m_d->ui->delPriceFieldPushButton, &QPushButton::clicked, this, &GeneralDataGUI::removePriceField );

    m_d->ui->priceFieldTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_d->ui->priceFieldTableView->setItemDelegate( m_d->fieldTableDelegate );
}

GeneralDataGUI::~GeneralDataGUI() {
    delete m_d;
}

void GeneralDataGUI::setProject( Project * pr ){
    if( m_d->project != pr ){
        m_d->project = pr;
        if( m_d->project != NULL ){
            m_d->ui->unitMeasureView->setModel( m_d->project->unitMeasureModel() );
            m_d->ui->unitMeasureView->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );
            m_d->ui->priceFieldTableView->setModel( m_d->project->priceFieldModel() );
            m_d->ui->priceFieldTableView->horizontalHeader()->resizeSections( QHeaderView::ResizeToContents );
        }
    }
}

void GeneralDataGUI::insertUnitMeasure(){
    if( m_d->project != NULL ){
        QModelIndexList listIndexs = m_d->ui->unitMeasureView->selectionModel()->selectedIndexes();
        QList<int> listRows;
        for( int i=0; i < listIndexs.size(); ++i){
            if( !listRows.contains( listIndexs.at(i).row() )){
                listRows.append( listIndexs.at(i).row() );
            }
        }
        qSort( listRows );
        if( listRows.size() > 0 ){
            m_d->project->unitMeasureModel()->insert( listRows.last()+1, listRows.size() );
        } else {
            m_d->project->unitMeasureModel()->insert( 0 );
        }
    }
}

void GeneralDataGUI::removeUnitMeasure(){
    if( m_d->project ){
        QModelIndexList listRows = m_d->ui->unitMeasureView->selectionModel()->selectedRows();
        if( listRows.size() > 0 ){
            m_d->project->unitMeasureModel()->removeUnitMeasure( listRows.first().row(), listRows.size());
        }
    }
}

void GeneralDataGUI::insertPriceField(){
    if( m_d->project != NULL ){
        QModelIndexList listIndexs = m_d->ui->priceFieldTableView->selectionModel()->selectedIndexes();
        QList<int> listRows;
        for( int i=0; i < listIndexs.size(); ++i){
            if( !listRows.contains( listIndexs.at(i).row() )){
                listRows.append( listIndexs.at(i).row() );
            }
        }
        qSort( listRows );
        if( listRows.size() > 0 ){
            m_d->project->priceFieldModel()->insertRows( listRows.last()+1, listRows.size() );
        } else {
            m_d->project->priceFieldModel()->insertRows( 0 );
        }
    }
}

void GeneralDataGUI::removePriceField(){
    if( m_d->project ){
        if( m_d->ui->priceFieldTableView->selectionModel() ){
            QModelIndexList selectedRows = m_d->ui->priceFieldTableView->selectionModel()->selectedRows();
            int count = selectedRows.size();
            if( count > 0 ){
                int row = selectedRows.at(0).row();
                for( int i=1; i<selectedRows.size(); ++i){
                    if( selectedRows.at(i).row() < row ){
                        row = selectedRows.at(i).row();
                    }
                }
                m_d->project->priceFieldModel()->removeRows( row, count );
            }
        }
    }
}
