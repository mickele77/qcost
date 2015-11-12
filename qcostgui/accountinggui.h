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

    void showEvent(QShowEvent *event);

private:
    AccountingGUIPrivate * m_d;

    void updateAmounts();
};

#endif // ACCOUNTINGGUI_H
