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
#ifndef PRICELISTTREEGUI_H
#define PRICELISTTREEGUI_H

#include "pricelistdbwidget.h"

class MathParser;
class PriceFieldModel;
class UnitMeasureModel;
class PriceList;
class PriceItem;

#include <QModelIndex>

#include <QWidget>

class PriceListTreeGUIPrivate;

class PriceListTreeGUI : public QWidget {
    Q_OBJECT
public:
    explicit PriceListTreeGUI(QMap<PriceListDBWidget::ImportOptions, bool> * EPAImpOptions,
                               QString *EPAFileName,
                               PriceList * prList, int curPriceDataSet,
                               MathParser * prs, PriceFieldModel *pfm, UnitMeasureModel * uml,
                               QWidget *parent = 0 );
    ~PriceListTreeGUI();

    void setPriceList(PriceList *, int priceDataSet = 0  );

    PriceItem * currentPriceItem();

    void setCurrentPriceItem(PriceItem *pItem);



signals:
    void currentItemChanged( PriceItem * newPriceItem, int newCurrentPriceDataSet );
    void currentPriceDataSetChanged( int );

public slots:
    void setCurrentPriceDataSet(int newCurrPriceDataSet);

private slots:
    void addItems();
    void addChildItems();
    void removeItems();

    void sortByCode();
    void sortByCodeInv();

    void changeCurrentItem( QModelIndex currentIndex );

    void treeViewCustomMenuRequested(QPoint pos);
    void copyToClipboard();
    void cutToClipboard();
    void pasteFromClipboard();
    void resizeColumnsToContents();

    void importMultiPriceItemDB();
    void importMultiPriceItemDB(const QList<QList<QPair<QString, QVariant> > > &itemDataList, const QList<int> &hierarchy);

    void setCurrentPriceDataSetSpinBoxMaximum(int newPriceDataSetCount);
    void setCurrentPriceDataSetFromSpinBox(int newPriceDataSet);

    void setPriceListnullptr();

    void updateCurrentPriceDataSet();
private:
    PriceListTreeGUIPrivate * m_d;

    void loadMultiPriceItemDB( const QList<QList<QPair<QString, QVariant> > > &itemDataList,
                               const QList<int> &hierarchy,
                               int priceDataSet,
                               const QModelIndex & parent,
                               int row, int relParent);
};

#endif // PRICELISTGUI_H
