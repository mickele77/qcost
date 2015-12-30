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
#ifndef ACCOUNTINGTAMBILLITEMPPUGUI_H
#define ACCOUNTINGTAMBILLITEMPPUGUI_H

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

class AccountingItemPPUGUIPrivate;

class AccountingItemPPUGUI : public QWidget {
    Q_OBJECT
public:
    explicit AccountingItemPPUGUI(QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                          QString *EPAFileName,
                          MathParser * prs, Project * prj,
                          QWidget *parent = 0);
    ~AccountingItemPPUGUI();

    void setAccountingTAMBill(AccountingTAMBill *b);
    void setAccountingBill(AccountingBill *b);

public slots:
    void setItem(AccountingTAMBillItem *b );
    void setItem(AccountingBillItem *b );
    void setAccountingItemNULL();

private slots:
    void connectPriceItem(PriceItem *oldPriceItem, PriceItem *newPriceItem);
    void disconnectPriceItem( PriceItem * priceItem );

    void connectPriceUnitMeasure();

    void associateLinesModel( bool ass );
    void addMeasureLines();
    void delMeasureLines();
    void importAccountingMeasureMeasuresTXT();


    void setDateLE();
    void changeItemDateGUI();

    void setQuantityLE();

    void addAttribute();
    void removeAttribute();

signals:
    void editPriceItemAP( PriceItem * pItem, Bill * APToEdit );

private:
    AccountingItemPPUGUIPrivate * m_d;

    bool eventFilter(QObject *object, QEvent *event);
};

#endif // ACCOUNTINGITEMGUI_H
