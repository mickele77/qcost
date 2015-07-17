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
#ifndef BILLSETPRICELISTMODEGUI_H
#define BILLSETPRICELISTMODEGUI_H

class BillSetPriceListModeGUIPrivate;

#include <QDialog>

#include "bill.h"

class BillSetPriceListModeGUI : public QDialog {
    Q_OBJECT
    
public:
    explicit BillSetPriceListModeGUI(QWidget *parent = 0);
    ~BillSetPriceListModeGUI();

    Bill::SetPriceListMode returnValue();
    
private:
    BillSetPriceListModeGUIPrivate * m_d;
};

#endif // BILLSETPRICELISTMODEGUI_H
