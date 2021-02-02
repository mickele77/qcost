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
#ifndef ACCOUNTINGLSBILLITEMGUI_H
#define ACCOUNTINGLSBILLITEMGUI_H

#include "pricelistdbwidget.h"

class Project;
class AccountingLSBill;
class AccountingLSBillItem;
class Bill;
class PriceItem;
class MathParser;

#include <QWidget>

class AccountingLSBillItemGUIPrivate;

class AccountingLSBillItemGUI : public QWidget {
    Q_OBJECT
public:
    explicit AccountingLSBillItemGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                      QString *EPAFileName,
                                      MathParser * prs, Project * prj,
                                      QWidget *parent = 0);
    ~AccountingLSBillItemGUI();

    void setBill( AccountingLSBill * b );

public slots:
    void setBillItem(AccountingLSBillItem *b );
    void clear();
    void setBillnullptr();

private slots:
    void connectPriceItem(PriceItem *oldPriceItem, PriceItem *newPriceItem);
    void disconnectPriceItem( PriceItem * priceItem );

    void connectPriceUnitMeasure();

    void addMeasureLines();
    void delMeasureLines();
    void importMeasuresTXT();

    void addAttribute();
    void removeAttribute();

    void editMeasureDate(const QModelIndex &index);
signals:
    void editPriceItemAP( PriceItem * pItem, Bill * APToEdit );

private:
    AccountingLSBillItemGUIPrivate * m_d;
};

#endif // ACCOUNTINGLSBILLITEMGUI_H
