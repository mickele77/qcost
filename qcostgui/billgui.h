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
#ifndef BILLGUI_H
#define BILLGUI_H

#include "pricelistdbwidget.h"

class Project;
class Bill;
class BillItem;
class PriceItem;
class MathParser;
class BillGUIPrivate;

#include <QTabWidget>

class BillGUI : public QTabWidget {
    Q_OBJECT
public:
    explicit BillGUI( QMap<PriceListDBWidget::ImportOptions, bool> *EPAImpOptions,
                      QString * EPAFileName,
                      MathParser *prs, Bill *b, Project *p,
                      QString *wordProcessorFile = nullptr, QWidget *parent = 0);

    ~BillGUI();

    void setBill(Bill *);

private slots:
    void setBillItem( BillItem * );
    void setBillItemnullptr();
    void updateBillItemGUI();
private:
    BillGUIPrivate * m_d;
};

#endif // BILLGUI_H
