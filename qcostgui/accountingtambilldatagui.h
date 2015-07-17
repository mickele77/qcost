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
#ifndef ACCOUNTINGTAMBILLDATAGUI_H
#define ACCOUNTINGTAMBILLDATAGUI_H

class AttributeList;
class Project;
class AccountingTAMBill;
class MathParser;
class PriceFieldModel;

#include <QWidget>

class AccountingTAMBillDataGUIPrivate;

class AccountingTAMBillDataGUI : public QWidget {
    Q_OBJECT
public:
    explicit AccountingTAMBillDataGUI( PriceFieldModel * pfm, MathParser *prs, AccountingTAMBill * b, Project * prj, QString * wordProcessorFile = NULL, QWidget *parent = 0);
    ~AccountingTAMBillDataGUI();

    void setAccountingTAMBill( AccountingTAMBill * b);
    void showEvent(QShowEvent *event);

private slots:
    void setDiscount();
    void setAccountingNULL();
    void addAttribute();
    void removeAttribute();

    // aggiorna etichette e importi di tutti i campi prezzo
    void updateAmountsValue();
    // aggiorna solo l'importo di un campo prezzo
    void updateAmountValue(int priceField, const QString &newVal);

    void resizeAttributeColsToContents();

    void attributesTableViewCustomMenuRequested(QPoint pos);
    void resizeAttributesColToContents();

    bool printAttributeAccountingODT();

    void setPriceList();
    void setPriceDataSet();
private:
    AccountingTAMBillDataGUIPrivate * m_d;

    void populatePriceListComboBox();
    void setPriceListComboBox();
    void setPriceDataSetSpinBox();
};

#endif // ACCOUNTINGTAMBILLDATAGUI_H
