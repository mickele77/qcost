#ifndef ACCOUNTINGGUI_H
#define ACCOUNTINGGUI_H

class ProjectAccountingParentItem;
class AccountingGUIPrivate;

#include <QWidget>

class AccountingGUI : public QWidget
{
    Q_OBJECT

public:
    explicit AccountingGUI(ProjectAccountingParentItem *acc, QWidget *parent = 0);
    ~AccountingGUI();

private slots:
    void addWorkProgress();
    void delWorkProgress();
private:
    AccountingGUIPrivate * m_d;
};

#endif // ACCOUNTINGGUI_H
