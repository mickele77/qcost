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
#ifndef UNITMEASUREMODEL_H
#define UNITMEASUREMODEL_H

class QSqlDatabase;
class QSqlQuery;

class UnitMeasureModelPrivate;
#include <QAbstractTableModel>

class UnitMeasureModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit UnitMeasureModel(QSqlDatabase * db, QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
    bool appendRow(int * newId, const QString & newTag );
    bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

    void clear();
    bool execPendingTransaction();

signals:
    void modelChanged( bool );

private:
    UnitMeasureModelPrivate * m_d;
    int nextId();
    QSqlQuery execTransaction(const QString &queryStr);
};

#endif // UNITMEASUREMODEL_H
