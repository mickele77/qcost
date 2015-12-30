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
#include "projectdataparentitem.h"

#include <QVariant>

ProjectDataParentItem::ProjectDataParentItem( ProjectItem * parent ):
    ProjectItem(parent) {
}

ProjectItem *ProjectDataParentItem::child(int number) {
    Q_UNUSED(number);
    return NULL;
}

int ProjectDataParentItem::childCount() const {
    return 0;
}

int ProjectDataParentItem::childNumber(ProjectItem *item) {
    Q_UNUSED(item);
    return -1;
}

bool ProjectDataParentItem::canChildrenBeInserted() {
    return false;
}

bool ProjectDataParentItem::insertChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

bool ProjectDataParentItem::removeChildren(int position, int count) {
    Q_UNUSED(position);
    Q_UNUSED(count);
    return false;
}

Qt::ItemFlags ProjectDataParentItem::flags() const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant ProjectDataParentItem::data() const {
    return QVariant( QObject::trUtf8("Dati generali") );
}

bool ProjectDataParentItem::setData(const QVariant &value) {
    Q_UNUSED(value);
    return false;
}
