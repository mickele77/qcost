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
#ifndef PRICELIST_H
#define PRICELIST_H

#include "qcost_export.h"

class ProjectPriceListParentItem;
class PriceFieldModel;
class PriceItem;
class UnitMeasureModel;
class UnitMeasure;
class MathParser;
class QXmlStreamReader;
class QXmlStreamWriter;
class QXmlStreamAttributes;
class QTextCursor;
class QTextStream;

#include "pricelistprinter.h"

#include "projectitem.h"
#include <QAbstractItemModel>

class PriceListPrivate;

class EXPORT_QCOST_LIB_OPT PriceList : public QAbstractItemModel, public ProjectItem {
    Q_OBJECT
public:
    friend class Project;

    explicit PriceList(const QString &n, PriceFieldModel * priceFields, ProjectItem *parent = NULL, MathParser *p = NULL);

    virtual ~PriceList();

    PriceList &operator =(const PriceList &cp);

    int childNumber( ProjectItem * priceItem );

    ProjectItem *child(int number);
    int childCount() const;

    bool canChildrenBeInserted();
    bool insertChildren(int position, int count);
    bool removeChildren(int position, int count);

    Qt::ItemFlags flags() const;
    QVariant data() const;
    bool setData( const QVariant &value);

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool insertPriceItems(int position = -1, int count = 1, const QModelIndex &parent = QModelIndex() );
    bool insertPriceItem(int inputPos = -1, const QModelIndex &parent = QModelIndex() );
    PriceItem *appendPriceItem();
    bool removeRows(int position = -1, int rows = 1, const QModelIndex &parent = QModelIndex() );
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationRow);

    PriceItem *priceItem(const QModelIndex &index) const;
    PriceItem *priceItemId( unsigned int i );
    PriceItem *defaultPriceItem();
    PriceItem *priceItemCode( const QString & c );

    QString name() const;
    QString description() const;

    int firstValueCol();
    int priceDataSetCount();

    bool isUsingUnitMeasure( UnitMeasure * );

    QList<PriceItem *> priceItemList();

    void writeXml10(QXmlStreamWriter *writer) const;
    void readXml10(QXmlStreamReader *reader, UnitMeasureModel *uml);
    void readFromXmlTmp10(ProjectPriceListParentItem *priceLists);
    void writeXml20( QXmlStreamWriter * writer ) const;
    void readXml20(QXmlStreamReader *reader, UnitMeasureModel *uml);
    void readFromXmlTmp20(ProjectPriceListParentItem *priceLists);

    void nextId();
    unsigned int id();

    void sortByCode();
    void sortByCodeInv();

    QModelIndex index(PriceItem *priceItem, int column) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;

    void writeODTOnTable( QTextCursor *cursor,
                          PriceListPrinter::PrintPriceItemsOption printOption,
                          const QList<int> fieldsToPrint,
                          int priceColToPrint,
                          bool printNumLetters = false ) const;

public slots:
    void setName(const QString &value);
    void setDescription(const QString &value);

signals:
    void removePriceItemSignal(int position, int rows, const QModelIndex &parent);
    void nameChanged( QString );
    void descriptionChanged( QString );
    void aboutToBeDeleted();
    void modelChanged();
    void priceDataSetCountChanged( int newPricColCount );

private slots:
    void updateValueTotal(PriceItem *, int column );
    void updateChildrenChanged(PriceItem* priceItem, QList<int> cols);

    void beginChangingColumns();

    void endChangingColumns();

private:
    PriceListPrivate * m_d;
    void loadFromXml10(const QXmlStreamAttributes &attrs);
    void loadFromXml20(const QXmlStreamAttributes &attrs);
    bool removePriceItems(int position = -1, int rows = 1, const QModelIndex &parent = QModelIndex() );
};

#endif // PRICELIST_H
