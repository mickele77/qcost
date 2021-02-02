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
#ifndef BILLVARSGUI_H
#define BILLVARSGUI_H

class AttributeList;
class Bill;
class AccountingBill;
class MathParser;
class PriceFieldModel;

#include <QWidget>

class VarsGUIPrivate;

class VarsGUI : public QWidget {
    Q_OBJECT
public:
    explicit VarsGUI( Bill * b, QWidget *parent = 0);
    explicit VarsGUI( AccountingBill * b, QWidget *parent = 0);
    ~VarsGUI();

    void setBill( Bill * b);
    void setBill( AccountingBill * b);

private slots:
    void setBillnullptr();
    void addVar();
    void removeVar();

    void resizeVarColsToContents();

    void varsTableViewCustomMenuRequested(QPoint pos);
    void resizeVarsColToContents();

private:
    VarsGUIPrivate * m_d;

    void init();

};

#endif // BILLVARSGUI_H
