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
#ifndef PROJECTROOTITEM_H
#define PROJECTROOTITEM_H

#include "qcost_export.h"

#include <QObject>
#include "projectitem.h"

class ProjectRootItemPrivate;

class EXPORT_QCOST_LIB_OPT ProjectRootItem : public QObject, public ProjectItem {
    Q_OBJECT
public:
    ProjectRootItem(ProjectItem *parent=NULL);
    ~ProjectRootItem();

    void insertChild( ProjectItem * item );

    ProjectItem *child(int number);
    int childCount() const;
    int childNumber( ProjectItem * item );

    bool canChildrenBeInserted();
    bool insertChildren(int position, int count);
    bool removeChildren(int position, int count);

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    void setDataChanged(int column, ProjectItem *p);

signals:
    void dataChanged( int column, ProjectItem * p );

private:
    ProjectRootItemPrivate * m_d;
};

#endif // PROJECTROOTITEM_H
