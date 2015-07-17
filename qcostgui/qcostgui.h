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
#ifndef QCOSTGUI_H
#define QCOSTGUI_H

#include <QMainWindow>

class QCostGUIPrivate;
class Project;
class ProjectItem;

class QCostGUI : public QMainWindow
{
    Q_OBJECT
    
public:
    /** Numero massimo di file recenti sisualizzati nel menu file */
    enum { MaxRecentFiles = 5 };

    explicit QCostGUI(QWidget *parent = 0);
    ~QCostGUI();
    
private:

    QCostGUIPrivate * m_d;

    void setCurrentProject(Project *proj);

    void createActions();
    void createMenus();
    void createToolBars();

    bool setCurrentFile(const QString &fileName, bool readContent = false );
    bool saveCurrentFile();

    void updateRecentFileActions();
    bool okToContinue();
    void closeEvent(QCloseEvent *event);

    void loadSettings();
    void saveSettings();
private slots:
    void setCurrentItem(ProjectItem* item);

    bool save();
    bool saveAs();
    bool printODT();
    void openRecentFile();
    void openFile();
    void newProject();
    void about();
    void setModified(bool v);
    void setModified();
    void viewPLDB();
    void setOptions();
};

#endif // QCOSTGUI_H
