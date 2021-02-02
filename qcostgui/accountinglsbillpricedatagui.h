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
#ifndef ACCOUNTINGLSBILLPRICEDATAGUI_H
#define ACCOUNTINGLSBILLPRICEDATAGUI_H

class AttributeList;
class Project;
class AccountingLSBill;
class MathParser;
class PriceFieldModel;

#include <QWidget>

class AccountingLSBillPriceDataGUIPrivate;

class AccountingLSBillPriceDataGUI : public QWidget {
    Q_OBJECT
public:
    explicit AccountingLSBillPriceDataGUI( Project * prj, QWidget *parent = 0);
    ~AccountingLSBillPriceDataGUI();

    void setAccountingBill(AccountingLSBill *b);

    void showEvent(QShowEvent *event);
private slots:
    void setAccountingBillnullptr();

    void setPriceList();
    void setPriceDataSet();

private:
    AccountingLSBillPriceDataGUIPrivate * m_d;

    void populatePriceListComboBox();
    void setPriceListComboBox();
    void setPriceDataSetSpinBox();
};

#endif // ACCOUNTINGLSBILLPRICEDATAGUI_H
