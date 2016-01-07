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
#ifndef UNITMEASURE_H
#define UNITMEASURE_H

#include "qcost_export.h"

class QXmlStreamAttributes;
class QXmlStreamWriter;
class QTextStream;

#include <QObject>

class UnitMeasurePrivate;

class EXPORT_QCOST_LIB_OPT UnitMeasure : public QObject
{
    Q_OBJECT
public:
    explicit UnitMeasure(QObject *parent = 0);
    ~UnitMeasure();

    unsigned int id();
    void nextId();

    QString description();
    void setDescription(const QString &t);

    QString tag();
    void setTag(const QString &t);

    int precision();
    void setPrecision(int );

    double applyPrecision( double );
    static double applyPrecision(double value, int precision);

    void writeXml10(QXmlStreamWriter *writer) const;
    void loadFromXml( const QXmlStreamAttributes & attrs );

signals:
    void descriptionChanged( const QString & );
    void tagChanged( const QString & );
    void precisionChanged( int );

private:
    UnitMeasurePrivate * m_d;
};

#endif // UNITMEASURE_H
