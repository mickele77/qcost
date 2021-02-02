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
#include "projectitemsview.h"
#include "ui_projectitemsview.h"

#include "project.h"
#include "projectbillparentitem.h"
#include "projectpricelistparentitem.h"
#include "projectdataparentitem.h"
#include "accountinglsbills.h"
#include "bill.h"
#include "pricelist.h"

#include <QMenu>
#include <QAction>

class ProjectItemsViewPrivate{
public:
    ProjectItemsViewPrivate():
        ui(new Ui::ProjectItemsView),
        project(0){
    };
    ~ProjectItemsViewPrivate(){
        delete ui;
    };

    Ui::ProjectItemsView * ui;
    Project * project;
};

ProjectItemsView::ProjectItemsView( QWidget *parent) :
    QWidget(parent),
    m_d( new ProjectItemsViewPrivate() ) {
    m_d->ui->setupUi(this);
    connect( m_d->ui->addPushButton, &QPushButton::clicked, this, &ProjectItemsView::insertChildren );
    connect( m_d->ui->delPushButton, &QPushButton::clicked, this, &ProjectItemsView::removeChildren );

    m_d->ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_d->ui->treeView, &QTreeView::customContextMenuRequested, this, &ProjectItemsView::treeViewCustomMenuRequested );
}

ProjectItemsView::~ProjectItemsView() {
    delete m_d;
}

void ProjectItemsView::setProject(Project *proj) {
    m_d->project = proj;
    m_d->ui->treeView->setModel( proj );
    connect( m_d->ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, static_cast<void(ProjectItemsView::*)(const QModelIndex &)>(&ProjectItemsView::setCurrentItem) );
}

void ProjectItemsView::insertChildren() {
    if( m_d->project ){
        QModelIndex curIndex = m_d->ui->treeView->selectionModel()->currentIndex();
        if( m_d->project->canChildrenBeInserted(curIndex)){
            m_d->project->appendRow( curIndex );
        }
        if( m_d->project->canChildrenBeInserted(curIndex.parent() )){
            m_d->project->appendRow( curIndex.parent() );
        }
    }
}

void ProjectItemsView::removeChildren() {
    if( m_d->project ){
        QModelIndex curIndex = m_d->ui->treeView->selectionModel()->currentIndex();
        if( m_d->project->canChildrenBeInserted(curIndex.parent() )){
            m_d->project->removeRows( curIndex.row(), 1, curIndex.parent() );
        }
    }
}

void ProjectItemsView::setCurrentItem( const QModelIndex & current ) {
    ProjectItem * item = static_cast<ProjectItem *>(current.internalPointer());
    emit currentItemChanged( item );
}

ProjectItem * ProjectItemsView::currentItem() {
    QModelIndex curIndex = m_d->ui->treeView->selectionModel()->currentIndex();
    ProjectItem * item = static_cast<ProjectItem *>(curIndex.internalPointer());
    return item;
}

void ProjectItemsView::setCurrentItem(ProjectItem *item) {
    m_d->ui->treeView->setCurrentIndex( m_d->project->index(0, item) );
}

void ProjectItemsView::treeViewCustomMenuRequested(QPoint pos){
    if( m_d->ui->treeView->selectionModel() ){
        QModelIndex index = m_d->ui->treeView->selectionModel()->currentIndex();
        ProjectItem * item = m_d->project->getItem(index);

        if( dynamic_cast<ProjectDataParentItem *>(item) == nullptr ){
            QMenu *menu=new QMenu(this);

            QModelIndex index = m_d->ui->treeView->selectionModel()->currentIndex();
            ProjectItem * item = m_d->project->getItem(index);
            if( dynamic_cast<ProjectBillParentItem *>(item) ){
                QAction * addAction = new QAction( tr("Aggiungi Computo"), this);
                connect( addAction, &QAction::triggered, this, &ProjectItemsView::insertChildren );
                menu->addAction(addAction);
            } else if( dynamic_cast<Bill *>(item) ){
                QAction * addAction = new QAction( tr("Aggiungi Computo"), this);
                connect( addAction, &QAction::triggered, this, &ProjectItemsView::insertChildren );
                menu->addAction(addAction);
                QAction * removeAction = new QAction( tr("Elimina Computo"), this);
                connect( removeAction, &QAction::triggered, this, &ProjectItemsView::removeChildren );
                menu->addAction(removeAction);
            } else if( dynamic_cast<ProjectPriceListParentItem *>(item) ){
                QAction * addAction = new QAction( tr("Aggiungi E.P."), this);
                connect( addAction, &QAction::triggered, this, &ProjectItemsView::insertChildren );
                menu->addAction(addAction);
            } else if( dynamic_cast<PriceList *>(item) ){
                QAction * addAction = new QAction( tr("Aggiungi E.P."), this);
                connect( addAction, &QAction::triggered, this, &ProjectItemsView::insertChildren );
                menu->addAction(addAction);
                QAction * removeAction = new QAction( tr("Elimina E.P."), this);
                connect( removeAction, &QAction::triggered, this, &ProjectItemsView::removeChildren );
                menu->addAction(removeAction);
            } else if( dynamic_cast<AccountingLSBills *>(item) ){
                QAction * addAction = new QAction( tr("Aggiungi Categoria"), this);
                connect( addAction, &QAction::triggered, this, &ProjectItemsView::insertChildren );
                menu->addAction(addAction);
            }
            menu->popup( m_d->ui->treeView->viewport()->mapToGlobal(pos) );
        }
    }
}
