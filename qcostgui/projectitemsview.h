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
#ifndef PROJECTITEMSVIEW_H
#define PROJECTITEMSVIEW_H

#include <QWidget>

class ProjectItemsViewPrivate;
class Project;
class ProjectItem;

class ProjectItemsView : public QWidget {
    Q_OBJECT

public:
    explicit ProjectItemsView(QWidget *parent = 0);
    ~ProjectItemsView();

    void setProject( Project * proj );

    ProjectItem *currentItem();
    void setCurrentItem(ProjectItem * item);

private slots:
    void insertChildren();
    void removeChildren();
    void setCurrentItem(const QModelIndex & );
    void treeViewCustomMenuRequested(QPoint pos);
signals:
    void currentItemChanged(ProjectItem * current);

private:
    ProjectItemsViewPrivate * m_d;
};

#endif // PROJECTITEMSVIEW_H
