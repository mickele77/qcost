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
#ifndef QCOSTDBWRITERGUI_H
#define QCOSTDBWRITERGUI_H

class QCostDBWriterGUIPrivate;

#include <QMainWindow>

class QCostDBWriterGUI : public QMainWindow
{
    Q_OBJECT
public:
    /** Numero massimo di file recenti sisualizzati nel menu file */
    enum { MaxRecentFiles = 5 };

    explicit QCostDBWriterGUI(QWidget *parent = 0);

signals:

public slots:

private:
    QCostDBWriterGUIPrivate * m_d;

    void createActions();
    void createMenus();
    void createToolBars();

    void updateRecentFileActions();

    void closeEvent(QCloseEvent *event);
    bool okToContinue();
    bool saveCurrentFile();

private slots:
    bool setCurrentFile(const QString &fileName, bool openExistent = false );
    void setModified( bool v );
    void newFile();
    void openFile();
    void importFromDB();
    void importFromTXT();
    void openRecentFile();
    bool save();
    bool saveAs();
    void about();
};

#endif // QCOSTDBWRITERGUI_H
