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
#ifndef PRICELISTGUI_H
#define PRICELISTGUI_H

#include "pricelistdbwidget.h"

class PriceList;
class Project;
class PriceItem;
class MathParser;

#include <QTabWidget>

class PriceListGUIPrivate;

class PriceListGUI : public QTabWidget {
    Q_OBJECT
public:
    explicit PriceListGUI( QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions, QString * EPAFileName,
                           PriceList * pl, MathParser * prs, Project * prj,
                           QWidget *parent = 0);
    ~PriceListGUI();

    void setPriceList( PriceList * pr);

private slots:
    void setPriceItem(PriceItem *newItem);
    void setPriceItemNULL();
private:
    PriceListGUIPrivate * m_d;
    void updatePriceItemGUI();
};

#endif // PRICELISTGUI_H
