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
#ifndef ACCOUNTINGBILLPRICEGUI_H
#define ACCOUNTINGBILLPRICEGUI_H

class AttributeList;
class Project;
class AccountingBill;
class MathParser;
class PriceFieldModel;

#include <QWidget>

class AccountingBillPriceGUIPrivate;

class AccountingBillPriceGUI : public QWidget {
    Q_OBJECT
public:
    explicit AccountingBillPriceGUI( PriceFieldModel * pfm, MathParser *prs, AccountingBill * b, Project * prj, QString * wordProcessorFile = NULL, QWidget *parent = 0);
    ~AccountingBillPriceGUI();

    void setAccountingBill( AccountingBill * b);

    void showEvent(QShowEvent *event);
private slots:
    void setAccountingNULL();

    void setPriceList();
    void setPriceDataSet();
    void setDiscountFromLineEdit();
private:
    AccountingBillPriceGUIPrivate * m_d;

    void populatePriceListComboBox();
    void setPriceListComboBox();
    void setPriceDataSetSpinBox();
};

#endif // ACCOUNTINGBILLPRICEGUI_H
