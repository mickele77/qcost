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
#ifndef ACCOUNTINGTAMBILLITEMGUI_H
#define ACCOUNTINGTAMBILLITEMGUI_H

#include "pricelistdbwidget.h"

class Project;
class Bill;
class AccountingBill;
class AccountingBillItem;
class AccountingTAMBill;
class AccountingTAMBillItem;
class PriceItem;
class MathParser;

#include <QWidget>

class AccountingTAMBillItemGUIPrivate;

class AccountingTAMBillItemGUI : public QWidget {
    Q_OBJECT
public:
    explicit AccountingTAMBillItemGUI(QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                          QString *EPAFileName,
                          MathParser * prs, Project * prj,
                          QWidget *parent = 0);
    ~AccountingTAMBillItemGUI();

    void setAccountingTAMBill(AccountingTAMBill *b);

public slots:
    void setItem(AccountingTAMBillItem *b );
    void setAccountingItemnullptr();

private slots:
    void connectPriceItem(PriceItem *oldPriceItem, PriceItem *newPriceItem);
    void disconnectPriceItem( PriceItem * priceItem );

    void connectPriceUnitMeasure();

    void changeItemDateGUI();

    void addAttribute();
    void removeAttribute();

private:
    AccountingTAMBillItemGUIPrivate * m_d;
};

#endif // ACCOUNTINGITEMGUI_H
