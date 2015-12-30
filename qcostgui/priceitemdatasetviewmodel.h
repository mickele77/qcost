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
#ifndef PRICEITEMDATASETVIEWMODEL_H
#define PRICEITEMDATASETVIEWMODEL_H

class PriceItemDataSetModel;
class MathParser;

#include <QAbstractTableModel>

class PriceItemDataSetViewModelPrivate;

class PriceItemDataSetViewModel: public QAbstractTableModel {
    Q_OBJECT

public:
    PriceItemDataSetViewModel( PriceItemDataSetModel * m, int curPriceDataSet, MathParser * prs, QObject * parent = 0 );

    ~PriceItemDataSetViewModel();

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool insertPriceDataSet(int column, int count = 1);
    bool appendPriceDataSet( int count = 1 );
    bool removePriceDataSet(int column, int count = 1);

    void setModel(PriceItemDataSetModel *m);
    void setCurrentPriceDataSet(int newDataSet);

    int associatedAPRow();

private slots:
    void beginInsertPriceDataSets(int firstCol, int lastCol);
    void endInsertPriceDataSets(int firstCol, int lastCol);
    void beginRemovePriceDataSets(int firstCol, int lastCol);
    void endRemovePriceDataSets(int firstCol, int lastCol);
    void beginInsertPriceField(int firstRow, int lastRow);
    void endInsertPriceField(int firstRow, int lastRow);
    void beginRemovePriceField(int firstRow, int lastRow);
    void endRemovePriceField(int firstRow, int lastRow);
    void dataHaveChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
private:
    PriceItemDataSetViewModelPrivate * m_d;

    int currentPriceDataSet() const;
};

#endif // PRICEITEMDATASETVIEWMODEL_H
