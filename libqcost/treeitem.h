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
#ifndef TREEITEM_H
#define TREEITEM_H

#include "library_common.h"

class QVariant;

class EXPORT_LIB_OPT TreeItem {
public:
    explicit TreeItem(){
    };

    virtual int columnCount() const = 0;
    virtual TreeItem *child(int number) = 0;
    virtual QVariant data(int column, int role) const = 0;
    virtual bool setData(int column, const QVariant &value) = 0;

    virtual int childrenCount() const = 0;
    virtual bool insertChildren(int position, int count ) = 0;
    virtual bool removeChildren(int position, int count) = 0;
    virtual int childNumber() const = 0;

protected:
    virtual TreeItem *parentInternal() = 0;
};

#endif // TREEITEM_H
