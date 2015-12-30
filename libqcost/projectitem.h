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
#ifndef PROJECTITEM_H
#define PROJECTITEM_H

#include "qcost_export.h"

class ProjectItemPrivate;
class QVariant;

#include <Qt>

class EXPORT_QCOST_LIB_OPT ProjectItem {
public:
    explicit ProjectItem(ProjectItem *parent = 0 );

    virtual ProjectItem *child(int number) = 0;
    virtual int childCount() const = 0;
    virtual bool canChildrenBeInserted() = 0;
    virtual bool insertChildren(int position, int count) = 0;
    virtual bool removeChildren(int position, int count) = 0;

    ProjectItem * parentItem();
    // indice dell'elemento nel contenitore dell'elemento genitore
    int childNumber() const;
    // indice dell'elemento nel contenitore dell'elemento genitore
    virtual int childNumber( ProjectItem * ) = 0;

    virtual Qt::ItemFlags flags() const = 0;
    virtual QVariant data() const = 0;
    virtual bool setData( const QVariant &value) = 0;

    virtual void setDataChanged(int column, ProjectItem *p);

protected:
    ProjectItem * m_parentItem;
};

#endif // PROJECTITEM_H
