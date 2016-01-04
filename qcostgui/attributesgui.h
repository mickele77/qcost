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
#ifndef ATTRIBUTESGUI_H
#define ATTRIBUTESGUI_H

class Bill;
class AccountingBill;
class AccountingTAMBill;
class AccountingLSBill;
class AccountingLSBills;
class MathParser;
class PriceFieldModel;

#include <QWidget>

class AttributesGUIPrivate;

class AttributesGUI : public QWidget {
    Q_OBJECT
public:
    explicit AttributesGUI( PriceFieldModel * pfm, MathParser *prs, QString * wordProcessorFile = NULL, QWidget *parent = 0);
    ~AttributesGUI();

    void setBill( Bill * b);
    void setBill( AccountingBill * b);
    void setBill( AccountingTAMBill * b);
    void setBill( AccountingLSBills *b);
    void setBill( AccountingLSBill *b);

    void activateAttributeModel();

private slots:
    void setBillNULL();
    void addAttribute();
    void removeAttribute();

    void resizeAttributeColsToContents();

    void attributesTableViewCustomMenuRequested(QPoint pos);
    void resizeAttributesColToContents();

    bool printAttributeBillODT();
private:
    AttributesGUIPrivate * m_d;

    void init();
};

#endif // ATTRIBUTESGUI_H
