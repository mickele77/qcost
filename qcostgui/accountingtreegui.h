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
#ifndef ACCOUNTINGTREEGUI_H
#define ACCOUNTINGTREEGUI_H

#include "pricelistdbwidget.h"

class Project;
class AccountingBill;
class AccountingBillItem;
class AccountingTAMBill;
class AccountingTAMBillItem;
class PriceItem;
class MathParser;
class PriceFieldModel;

#include <QWidget>
#include <QModelIndex>

class AccountingTreeGUIPrivate;

class AccountingTreeGUI : public QWidget {
    Q_OBJECT
public:
    friend class AccountingGUI;
    friend class AccountingTAMBillGUI;
    explicit AccountingTreeGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString *EPAFileName, AccountingBill * b, MathParser *prs, Project *prj, QWidget *parent = 0);
    explicit AccountingTreeGUI(QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions, QString *EPAFileName, AccountingTAMBill *b, MathParser *prs, Project *prj, QWidget *parent);
    ~AccountingTreeGUI();
    
    AccountingBillItem * currentAccountingBill();
    void setAccountingBill( AccountingBill * b );
    AccountingTAMBillItem *currentAccountingTAMBill();
    void setAccountingTAMBill(AccountingTAMBill *b);

    void showEvent(QShowEvent * event);

signals:
    void currentBillItemChanged( AccountingBillItem * );
    void currentTAMBillItemChanged( AccountingTAMBillItem * );
    void editAccountingData( AccountingBillItem * );

private slots:
    void removeItems();
    void addItems();

    void changeCurrentItem( const QModelIndex & currentIndex );

    void editAccountingData( const QModelIndex & index);

    void setPriceList();
    void setPriceDatSet();

    void clear();
    void accountingTreeViewCustomMenuRequested(QPoint pos);
    void editAttributes();

    void copyToClipboard();
    void pasteFromClipboard();
    void cutToClipboard();
    void resizeColumnsToContents();

private:
    AccountingTreeGUIPrivate * m_d;

    void populatePriceListComboBox();
    void setPriceListComboBox();
    /**
     * Imposta il valore dello SpinBox m_d->ui->currentPriceDataSetSpinBox in base al valore di  m_d->accountingBill->priceDataSet()
     * Ricordarsi di chiamare
     * disconnect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTreeGUI::setPriceDatSet );
     * prima e
     * connect( m_d->ui->currentPriceDataSetSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &AccountingTreeGUI::setPriceDatSet );
     * dopo
     */
    void setCurrentPriceDataSetSpinBox();
};

#endif // ACCOUNTINGTREEGUI_H
