/********************************************************************************
** Form generated from reading UI file 'pricelistdbwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRICELISTDBWIDGET_H
#define UI_PRICELISTDBWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PriceListDBWidget
{
public:
    QGridLayout *gridLayout_3;
    QTabWidget *tabWidget;
    QWidget *priceListTab;
    QGridLayout *gridLayout_4;
    QSplitter *splitter;
    QWidget *layoutWidget;
    QGridLayout *gridLayout;
    QTreeView *priceListView;
    QPushButton *delPLPushButton;
    QPushButton *addPLPushButton;
    QPushButton *addPLChildPushButton;
    QGroupBox *priceItemGroupBox;
    QGridLayout *gridLayout_2;
    QLineEdit *codeLineEdit;
    QLabel *codeLabel;
    QLabel *shortDescLabel;
    QLabel *unitMeasureLabel;
    QLabel *longDescLabel;
    QLabel *priceTotalLabel;
    QLineEdit *shortDescLineEdit;
    QPlainTextEdit *longDescTextEdit;
    QLabel *priceHumanLabel;
    QComboBox *unitMeasureComboBox;
    QLineEdit *priceTotalLineEdit;
    QLabel *priceEquipmentLabel;
    QLineEdit *priceHumanLineEdit;
    QLabel *priceMaterialLabel;
    QLineEdit *priceEquipmentLineEdit;
    QLabel *overheadsLabel;
    QLineEdit *priceMaterialLineEdit;
    QLineEdit *overheadsLineEdit;
    QLabel *profitsLabel;
    QLineEdit *profitsLineEdit;
    QWidget *unitMeasureTab;
    QGridLayout *gridLayout_5;
    QTableView *unitMeasureView;
    QPushButton *addUMPushButton;
    QPushButton *delUMPushButton;

    void setupUi(QWidget *PriceListDBWidget)
    {
        if (PriceListDBWidget->objectName().isEmpty())
            PriceListDBWidget->setObjectName(QString::fromUtf8("PriceListDBWidget"));
        PriceListDBWidget->resize(512, 441);
        gridLayout_3 = new QGridLayout(PriceListDBWidget);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        tabWidget = new QTabWidget(PriceListDBWidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        priceListTab = new QWidget();
        priceListTab->setObjectName(QString::fromUtf8("priceListTab"));
        gridLayout_4 = new QGridLayout(priceListTab);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        splitter = new QSplitter(priceListTab);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        layoutWidget = new QWidget(splitter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        gridLayout = new QGridLayout(layoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        priceListView = new QTreeView(layoutWidget);
        priceListView->setObjectName(QString::fromUtf8("priceListView"));
        priceListView->setSelectionMode(QAbstractItemView::ExtendedSelection);

        gridLayout->addWidget(priceListView, 0, 0, 1, 3);

        delPLPushButton = new QPushButton(layoutWidget);
        delPLPushButton->setObjectName(QString::fromUtf8("delPLPushButton"));

        gridLayout->addWidget(delPLPushButton, 1, 0, 1, 1);

        addPLPushButton = new QPushButton(layoutWidget);
        addPLPushButton->setObjectName(QString::fromUtf8("addPLPushButton"));

        gridLayout->addWidget(addPLPushButton, 1, 1, 1, 1);

        addPLChildPushButton = new QPushButton(layoutWidget);
        addPLChildPushButton->setObjectName(QString::fromUtf8("addPLChildPushButton"));

        gridLayout->addWidget(addPLChildPushButton, 1, 2, 1, 1);

        splitter->addWidget(layoutWidget);
        priceItemGroupBox = new QGroupBox(splitter);
        priceItemGroupBox->setObjectName(QString::fromUtf8("priceItemGroupBox"));
        gridLayout_2 = new QGridLayout(priceItemGroupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        codeLineEdit = new QLineEdit(priceItemGroupBox);
        codeLineEdit->setObjectName(QString::fromUtf8("codeLineEdit"));
        codeLineEdit->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridLayout_2->addWidget(codeLineEdit, 0, 1, 1, 1);

        codeLabel = new QLabel(priceItemGroupBox);
        codeLabel->setObjectName(QString::fromUtf8("codeLabel"));

        gridLayout_2->addWidget(codeLabel, 0, 0, 1, 1);

        shortDescLabel = new QLabel(priceItemGroupBox);
        shortDescLabel->setObjectName(QString::fromUtf8("shortDescLabel"));

        gridLayout_2->addWidget(shortDescLabel, 1, 0, 1, 1);

        unitMeasureLabel = new QLabel(priceItemGroupBox);
        unitMeasureLabel->setObjectName(QString::fromUtf8("unitMeasureLabel"));

        gridLayout_2->addWidget(unitMeasureLabel, 4, 0, 1, 1);

        longDescLabel = new QLabel(priceItemGroupBox);
        longDescLabel->setObjectName(QString::fromUtf8("longDescLabel"));

        gridLayout_2->addWidget(longDescLabel, 2, 0, 1, 1);

        priceTotalLabel = new QLabel(priceItemGroupBox);
        priceTotalLabel->setObjectName(QString::fromUtf8("priceTotalLabel"));

        gridLayout_2->addWidget(priceTotalLabel, 5, 0, 1, 1);

        shortDescLineEdit = new QLineEdit(priceItemGroupBox);
        shortDescLineEdit->setObjectName(QString::fromUtf8("shortDescLineEdit"));

        gridLayout_2->addWidget(shortDescLineEdit, 1, 1, 1, 1);

        longDescTextEdit = new QPlainTextEdit(priceItemGroupBox);
        longDescTextEdit->setObjectName(QString::fromUtf8("longDescTextEdit"));

        gridLayout_2->addWidget(longDescTextEdit, 3, 0, 1, 2);

        priceHumanLabel = new QLabel(priceItemGroupBox);
        priceHumanLabel->setObjectName(QString::fromUtf8("priceHumanLabel"));

        gridLayout_2->addWidget(priceHumanLabel, 6, 0, 1, 1);

        unitMeasureComboBox = new QComboBox(priceItemGroupBox);
        unitMeasureComboBox->setObjectName(QString::fromUtf8("unitMeasureComboBox"));

        gridLayout_2->addWidget(unitMeasureComboBox, 4, 1, 1, 1);

        priceTotalLineEdit = new QLineEdit(priceItemGroupBox);
        priceTotalLineEdit->setObjectName(QString::fromUtf8("priceTotalLineEdit"));
        priceTotalLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(priceTotalLineEdit, 5, 1, 1, 1);

        priceEquipmentLabel = new QLabel(priceItemGroupBox);
        priceEquipmentLabel->setObjectName(QString::fromUtf8("priceEquipmentLabel"));

        gridLayout_2->addWidget(priceEquipmentLabel, 7, 0, 1, 1);

        priceHumanLineEdit = new QLineEdit(priceItemGroupBox);
        priceHumanLineEdit->setObjectName(QString::fromUtf8("priceHumanLineEdit"));
        priceHumanLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(priceHumanLineEdit, 6, 1, 1, 1);

        priceMaterialLabel = new QLabel(priceItemGroupBox);
        priceMaterialLabel->setObjectName(QString::fromUtf8("priceMaterialLabel"));

        gridLayout_2->addWidget(priceMaterialLabel, 8, 0, 1, 1);

        priceEquipmentLineEdit = new QLineEdit(priceItemGroupBox);
        priceEquipmentLineEdit->setObjectName(QString::fromUtf8("priceEquipmentLineEdit"));
        priceEquipmentLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(priceEquipmentLineEdit, 7, 1, 1, 1);

        overheadsLabel = new QLabel(priceItemGroupBox);
        overheadsLabel->setObjectName(QString::fromUtf8("overheadsLabel"));

        gridLayout_2->addWidget(overheadsLabel, 9, 0, 1, 1);

        priceMaterialLineEdit = new QLineEdit(priceItemGroupBox);
        priceMaterialLineEdit->setObjectName(QString::fromUtf8("priceMaterialLineEdit"));
        priceMaterialLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(priceMaterialLineEdit, 8, 1, 1, 1);

        overheadsLineEdit = new QLineEdit(priceItemGroupBox);
        overheadsLineEdit->setObjectName(QString::fromUtf8("overheadsLineEdit"));
        overheadsLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(overheadsLineEdit, 9, 1, 1, 1);

        profitsLabel = new QLabel(priceItemGroupBox);
        profitsLabel->setObjectName(QString::fromUtf8("profitsLabel"));

        gridLayout_2->addWidget(profitsLabel, 10, 0, 1, 1);

        profitsLineEdit = new QLineEdit(priceItemGroupBox);
        profitsLineEdit->setObjectName(QString::fromUtf8("profitsLineEdit"));
        profitsLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(profitsLineEdit, 10, 1, 1, 1);

        splitter->addWidget(priceItemGroupBox);

        gridLayout_4->addWidget(splitter, 0, 0, 1, 1);

        tabWidget->addTab(priceListTab, QString());
        unitMeasureTab = new QWidget();
        unitMeasureTab->setObjectName(QString::fromUtf8("unitMeasureTab"));
        gridLayout_5 = new QGridLayout(unitMeasureTab);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        unitMeasureView = new QTableView(unitMeasureTab);
        unitMeasureView->setObjectName(QString::fromUtf8("unitMeasureView"));

        gridLayout_5->addWidget(unitMeasureView, 0, 0, 1, 2);

        addUMPushButton = new QPushButton(unitMeasureTab);
        addUMPushButton->setObjectName(QString::fromUtf8("addUMPushButton"));

        gridLayout_5->addWidget(addUMPushButton, 1, 0, 1, 1);

        delUMPushButton = new QPushButton(unitMeasureTab);
        delUMPushButton->setObjectName(QString::fromUtf8("delUMPushButton"));

        gridLayout_5->addWidget(delUMPushButton, 1, 1, 1, 1);

        tabWidget->addTab(unitMeasureTab, QString());

        gridLayout_3->addWidget(tabWidget, 0, 0, 1, 1);


        retranslateUi(PriceListDBWidget);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(PriceListDBWidget);
    } // setupUi

    void retranslateUi(QWidget *PriceListDBWidget)
    {
        PriceListDBWidget->setWindowTitle(QCoreApplication::translate("PriceListDBWidget", "Form", nullptr));
        delPLPushButton->setText(QCoreApplication::translate("PriceListDBWidget", "-", nullptr));
        addPLPushButton->setText(QCoreApplication::translate("PriceListDBWidget", "+", nullptr));
        addPLChildPushButton->setText(QCoreApplication::translate("PriceListDBWidget", "\342\226\274 +", nullptr));
        priceItemGroupBox->setTitle(QCoreApplication::translate("PriceListDBWidget", "Dettaglio prezzo", nullptr));
        codeLabel->setText(QCoreApplication::translate("PriceListDBWidget", "Codice", nullptr));
        shortDescLabel->setText(QCoreApplication::translate("PriceListDBWidget", "Denominazione", nullptr));
        unitMeasureLabel->setText(QCoreApplication::translate("PriceListDBWidget", "UdM", nullptr));
        longDescLabel->setText(QCoreApplication::translate("PriceListDBWidget", "Descrizione", nullptr));
        priceTotalLabel->setText(QCoreApplication::translate("PriceListDBWidget", "Costo Unitario", nullptr));
        priceHumanLabel->setText(QCoreApplication::translate("PriceListDBWidget", "C.U. Manodopera", nullptr));
        priceEquipmentLabel->setText(QCoreApplication::translate("PriceListDBWidget", "C.U. Mezzi d'opera", nullptr));
        priceMaterialLabel->setText(QCoreApplication::translate("PriceListDBWidget", "C.U. Materiali", nullptr));
        overheadsLabel->setText(QCoreApplication::translate("PriceListDBWidget", "Spese Generali", nullptr));
        profitsLabel->setText(QCoreApplication::translate("PriceListDBWidget", "Utili", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(priceListTab), QCoreApplication::translate("PriceListDBWidget", "Elenco Prezzi", nullptr));
        addUMPushButton->setText(QCoreApplication::translate("PriceListDBWidget", "+", nullptr));
        delUMPushButton->setText(QCoreApplication::translate("PriceListDBWidget", "-", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(unitMeasureTab), QCoreApplication::translate("PriceListDBWidget", "Unit\303\240 di misura", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PriceListDBWidget: public Ui_PriceListDBWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRICELISTDBWIDGET_H
