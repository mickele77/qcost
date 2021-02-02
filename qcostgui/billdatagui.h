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
#ifndef BILLDATAGUI_H
#define BILLDATAGUI_H

class AttributeList;
class Bill;
class PriceFieldModel;

#include <QWidget>

class BillDataGUIPrivate;

class BillDataGUI : public QWidget {
    Q_OBJECT
public:
    explicit BillDataGUI(PriceFieldModel * pfm, Bill * b, QWidget *parent = 0);
    ~BillDataGUI();

    void setBill( Bill * b);

private slots:
    void setDescription();
    void setBillnullptr();

    // aggiorna etichette e importi di tutti i campi prezzo
    void updateAmountsNameValue();
    // aggiorna solo l'etichetta di un campo prezzo
    void updateAmountName( int priceField, const QString & newName );
    // aggiorna solo l'importo di un campo prezzo
    void updateAmountValue(int priceField, const QString &newVal);

private:
    BillDataGUIPrivate * m_d;
};

#endif // BILLDATAGUI_H
