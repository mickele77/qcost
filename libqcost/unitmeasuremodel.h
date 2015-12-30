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
#ifndef UNITMEASUREMODEL_H
#define UNITMEASUREMODEL_H

#include "qcost_export.h"

class UnitMeasure;
class QTextStream;
class QXmlStreamReader;
class QXmlStreamWriter;
class MathParser;

#include <QAbstractTableModel>

class UnitMeasureModelPrivate;

class EXPORT_QCOST_LIB_OPT UnitMeasureModel : public QAbstractTableModel {
    Q_OBJECT
public:
    friend class Project;

    explicit UnitMeasureModel(MathParser *prs, QObject *parent = 0);
    ~UnitMeasureModel();

    int size();
    UnitMeasure * unitMeasure( int i );
    UnitMeasure * unitMeasureId( unsigned int id );
    UnitMeasure * unitMeasureTag(const QString &tag);
    int findTag(const QString &tag);

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex())  const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void insertStandardUnits();

    bool insert(int row, int count=1);
    bool append();
    int append(const QString &tag);

    void removeUnitMeasure(int row, int count=1);
    void clear();

    void writeXml( QXmlStreamWriter * writer );
    void readXml( QXmlStreamReader * reader );

signals:
    void removeSignal( int row, int count );
    void modelChanged();
    
private:
    UnitMeasureModelPrivate * m_d;
    bool removeUnitMeasurePrivate(int row, int count=1);
    void clearPrivate();
};

#endif // UNITMEASUREMODEL_H
