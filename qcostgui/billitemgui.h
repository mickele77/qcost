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
#ifndef BILLITEMGUI_H
#define BILLITEMGUI_H

#include "pricelistdbwidget.h"

class Project;
class Bill;
class BillItem;
class PriceItem;
class MathParser;

#include <QWidget>

class BillItemGUIPrivate;

class BillItemGUI : public QWidget {
    Q_OBJECT
public:
    explicit BillItemGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                          QString *EPAFileName,
                          BillItem *b, MathParser * prs, Project * prj,
                          QWidget *parent = 0);
    ~BillItemGUI();

    void setBill( Bill * b );

public slots:
    void setBillItem( BillItem * b );

private slots:
    void connectPriceItem(PriceItem *oldPriceItem, PriceItem *newPriceItem);
    void disconnectPriceItem( PriceItem * priceItem );

    void connectPriceUnitMeasure();

    void associateLinesModel( bool ass );
    void addMeasureLines();
    void delMeasureLines();
    void importBillItemMeasuresTXT();

    void setBillItemNULL();

    void setQuantityLE();

    void addAttribute();
    void removeAttribute();

    void updatePriceAmountNamesValues();
    void updatePriceName(int priceField, const QString &newName);
    void updatePriceValue(int priceField, int priceCol, const QString &newValue);
    void updatePriceValues(int priceCol);
    void updateAmountName(int priceField, const QString &newName);
    void updateAmountValue(int priceField, const QString &newVal);
    void updateAmountValues();

signals:
    void editPriceItemAP( PriceItem * pItem, Bill * APToEdit );

private:
    BillItemGUIPrivate * m_d;

};

#endif // BILLITEMGUI_H
