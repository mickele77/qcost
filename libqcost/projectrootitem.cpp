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
#include "projectrootitem.h"

#include <QVariant>
#include <QList>

class ProjectRootItemPrivate{
public:
    ProjectRootItemPrivate(){
    };
    QList<ProjectItem *> childrenContainer;
};

ProjectRootItem::ProjectRootItem( ProjectItem *parent ):
    ProjectItem(parent),
    m_d(new ProjectRootItemPrivate() ){
}

ProjectRootItem::~ProjectRootItem(){
    delete m_d;
}

void ProjectRootItem::insertChild(ProjectItem *item) {
    m_d->childrenContainer.append( item );
}

ProjectItem *ProjectRootItem::child(int number) {
    if( number >= 0 && number < m_d->childrenContainer.size() ){
        return m_d->childrenContainer[number];
    }
    return NULL;
}

int ProjectRootItem::childCount() const {
    return m_d->childrenContainer.size();
}

int ProjectRootItem::childNumber(ProjectItem *item ) {
    ProjectItem * i = const_cast<ProjectItem * >(item);
     return m_d->childrenContainer.indexOf( i );
}

bool ProjectRootItem::canChildrenBeInserted() {
    return false;
}

bool ProjectRootItem::insertChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

bool ProjectRootItem::removeChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

Qt::ItemFlags ProjectRootItem::flags() const {
    return Qt::NoItemFlags;
}

QVariant ProjectRootItem::data() const {
    return QVariant();
}

bool ProjectRootItem::setData(const QVariant &value) {
    Q_UNUSED(value);
    return false;
}

void ProjectRootItem::setDataChanged(int column, ProjectItem *p) {
    emit dataChanged( column, p );
}
