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
#ifndef ACCOUNTINGBILLDATAGUI_H
#define ACCOUNTINGBILLDATAGUI_H

class AttributeList;
class Project;
class AccountingBill;
class MathParser;
class PriceFieldModel;

#include <QWidget>

class AccountingBillDataGUIPrivate;

class AccountingBillDataGUI : public QWidget {
    Q_OBJECT
public:
    explicit AccountingBillDataGUI( PriceFieldModel * pfm, MathParser *prs, AccountingBill * b, Project * prj, QString * wordProcessorFile = NULL, QWidget *parent = 0);
    ~AccountingBillDataGUI();

    void setAccountingBill( AccountingBill * b);

    void showEvent(QShowEvent *event);
private slots:
    void setDescription();
    void setDiscount();
    void setAccountingNULL();
    void addAttribute();
    void removeAttribute();

    void resizeAttributeColsToContents();

    void attributesTableViewCustomMenuRequested(QPoint pos);
    void resizeAttributesColToContents();

    bool printAttributeAccountingODT();

    void setPriceList();
    void setPriceDataSet();
private:
    AccountingBillDataGUIPrivate * m_d;

    void populatePriceListComboBox();
    void setPriceListComboBox();
    void setPriceDataSetSpinBox();
};

#endif // ACCOUNTINGBILLDATAGUI_H
