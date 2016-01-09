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
#ifndef PROJECTBILLPARENTITEM_H
#define PROJECTBILLPARENTITEM_H

#include "qcost_export.h"

class Bill;
class ProjectPriceListParentItem;
class PriceList;
class PriceItem;
class AttributeList;
class PriceFieldModel;
class MathParser;
class QXmlStreamReader;
class QXmlStreamWriter;
class QTextStream;

#include <QObject>
#include "projectitem.h"

class ProjectBillParentItemPrivate;

class EXPORT_QCOST_LIB_OPT ProjectBillParentItem : public QObject, public ProjectItem {
    Q_OBJECT
public:
    ProjectBillParentItem( ProjectItem * parent, PriceFieldModel * pfm, MathParser * p = NULL );

    int billCount();
    Bill * bill( int i );

    ProjectItem *child(int number);
    int childCount() const;

    int childNumber( ProjectItem *item );

    bool canChildrenBeInserted();
    bool insertChildren(int position, int count = 1);
    bool appendChild();
    bool removeChildren(int position, int count = 1);
    bool clear();

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    bool isUsingPriceList( PriceList * pl );
    bool isUsingPriceItem(PriceItem * p );

    void writeXml(QXmlStreamWriter * writer , const QString &vers) const;
    void readXml(QXmlStreamReader *reader, ProjectPriceListParentItem *priceLists, const QString & vers);

signals:
    void beginInsertChildren( int first, int last );
    void endInsertChildren();
    void beginRemoveChildren( int first, int last );
    void endRemoveChildren();
    void modelChanged();

private:
    ProjectBillParentItemPrivate * m_d;
    void writeXml10(QXmlStreamWriter *writer) const;
    void readXml10(QXmlStreamReader *reader, ProjectPriceListParentItem *priceLists);
};

#endif // PROJECTBILLPARENTITEM_H
