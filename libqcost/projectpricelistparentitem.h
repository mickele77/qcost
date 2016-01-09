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
#ifndef PROJECTPRICELISTPARENTITEM_H
#define PROJECTPRICELISTPARENTITEM_H

#include "qcost_export.h"

class PriceList;
class UnitMeasureModel;
class PriceFieldModel;
class MathParser;
class QXmlStreamReader;
class QXmlStreamWriter;
class QTextStream;

#include "projectitem.h"
#include <QObject>

class ProjectPriceListParentItemPrivate;

class EXPORT_QCOST_LIB_OPT ProjectPriceListParentItem : public QObject, public ProjectItem {
    Q_OBJECT
public:
    friend class Project;

    explicit ProjectPriceListParentItem( ProjectItem *parent, PriceFieldModel * priceFields, MathParser * p = NULL );
    ~ProjectPriceListParentItem();

    ProjectItem *child(int number);
    int childCount() const;

    int childNumber( ProjectItem * item );

    bool canChildrenBeInserted();
    bool insertChildren(int position, int count = 1 );
    bool appendChildren( int count = 1 );
    bool removeChildren(int position, int count = 1);
    bool clear();

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    int priceListCount();
    PriceList * priceList( int i );
    PriceList * priceListId( unsigned int dd );

    void writeXml(QXmlStreamWriter * writer , const QString &vers) const;
    void readXml(QXmlStreamReader *reader, UnitMeasureModel *uml);

signals:
    void removePriceItemSignal(PriceList * pl, int position, int count, const QModelIndex &parent );
    void removePriceListSignal(int position, int count);

    void beginInsertChildren( ProjectItem * item, int first, int last );
    void endInsertChildren();
    void beginRemoveChildren( ProjectItem * item, int first, int last );
    void endRemoveChildren();

    void modelChanged();

private slots:
    void emitRemovePriceItemSignal(int position, int count, const QModelIndex &parent );

private:
    ProjectPriceListParentItemPrivate * m_d;

    bool removeChildrenUnsecure(int position, int count = 1);
    void writeXml10(QXmlStreamWriter *writer) const;
    void writeXml20(QXmlStreamWriter *writer) const;
};

#endif // PROJECTPRICELISTPARENTITEM_H
