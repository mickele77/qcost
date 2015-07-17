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
#ifndef PRICELISTDBWIDGET_H
#define PRICELISTDBWIDGET_H

class PriceListDBModel;

#include <QWidget>

class PriceListDBWidgetPrivate;

class PriceListDBWidget : public QWidget {
    Q_OBJECT
public:
    explicit PriceListDBWidget(PriceListDBModel *m, QWidget *parent = 0);
    ~PriceListDBWidget();

    void clear();
    
private slots:
    void setCurrentPrice(const QModelIndex &current);
    void setCodeFromLE();
    void setShortDescriptionFromLE();
    void setUnitMeasureFromCB(int);
    void setPriceTotalFromLE();
    void setPriceHumanFromLE();
    void setPriceEquipmentFromLE();
    void setPriceMaterialFromLE();
    void setOverheadsFromLE();
    void setProfitsFromLE();

    void populateUnitMeasureCB();

    void addPLItems();
    void delPLItems();
    void addPLChildItems();
    void updateCurrentIndexData( const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void delUMItems();
    void addUMItems();
private:
    PriceListDBWidgetPrivate * m_d;
    bool eventFilter(QObject *object, QEvent *event);
};

#endif // PRICELISTDBWIDGET_H
