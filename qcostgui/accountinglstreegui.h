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
#ifndef ACCOUNTINGLSTREEGUI_H
#define ACCOUNTINGLSTREEGUI_H

#include "pricelistdbwidget.h"

class Project;
class AccountingLSBill;
class AccountingLSBillItem;
class PriceItem;
class MathParser;
class PriceFieldModel;

#include <QWidget>
#include <QModelIndex>

class AccountingLSTreeGUIPrivate;

class AccountingLSTreeGUI : public QWidget {
    Q_OBJECT
public:
    friend class AccountingGUI;
    friend class AccountingLSBillGUI;
    explicit AccountingLSTreeGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                                  QString *EPAFileName, MathParser *prs, Project *prj, QWidget *parent = 0);
    ~AccountingLSTreeGUI();
    
    AccountingLSBillItem * currentItem();
    void setBill( AccountingLSBill * b );

signals:
    void currentItemChanged( AccountingLSBillItem * );
    void editAccountingData( AccountingLSBillItem * );

private slots:
    void addItems();
    void addChildItems();
    void removeItems();

    void changeCurrentItem( const QModelIndex & currentIndex );

    void editAccountingData( const QModelIndex & index);

    void clear();
    void accountingTreeViewCustomMenuRequested(QPoint pos);
    void editAttributes();

    void copyToClipboard();
    void pasteFromClipboard();
    void cutToClipboard();
    void resizeColumnsToContents();

private:
    AccountingLSTreeGUIPrivate * m_d;

};

#endif // ACCOUNTINGLSTREEGUI_H
