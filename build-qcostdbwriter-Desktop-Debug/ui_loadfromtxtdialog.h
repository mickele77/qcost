/********************************************************************************
** Form generated from reading UI file 'loadfromtxtdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOADFROMTXTDIALOG_H
#define UI_LOADFROMTXTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

class Ui_LoadFromTXTDialog
{
public:
    QGridLayout *gridLayout;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_3;
    QLabel *decimalSepLabel;
    QLabel *thousandSepLabel;
    QLineEdit *decimalSepLineEdit;
    QLineEdit *thousandSepLineEdit;
    QSpacerItem *horizontalSpacer_3;
    QGroupBox *groupBox_3;
    QGridLayout *gridLayout_5;
    QLineEdit *overheadsLineEdit;
    QLabel *overheadsLabel;
    QLabel *profitsLabel;
    QLineEdit *profitsLineEdit;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_2;
    QToolButton *addColButton;
    QToolButton *delColButton;
    QGridLayout *comboBoxLayout;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer;
    QPlainTextEdit *plainTextEdit;
    QDialogButtonBox *buttonBox;
    QGroupBox *generalOptionsGroupBox;
    QGridLayout *gridLayout_4;
    QCheckBox *setShortDescFromLongCheckBox;

    void setupUi(QDialog *LoadFromTXTDialog)
    {
        if (LoadFromTXTDialog->objectName().isEmpty())
            LoadFromTXTDialog->setObjectName(QString::fromUtf8("LoadFromTXTDialog"));
        LoadFromTXTDialog->resize(559, 378);
        gridLayout = new QGridLayout(LoadFromTXTDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        groupBox_2 = new QGroupBox(LoadFromTXTDialog);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_3 = new QGridLayout(groupBox_2);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        decimalSepLabel = new QLabel(groupBox_2);
        decimalSepLabel->setObjectName(QString::fromUtf8("decimalSepLabel"));

        gridLayout_3->addWidget(decimalSepLabel, 0, 0, 1, 1);

        thousandSepLabel = new QLabel(groupBox_2);
        thousandSepLabel->setObjectName(QString::fromUtf8("thousandSepLabel"));

        gridLayout_3->addWidget(thousandSepLabel, 0, 2, 1, 1);

        decimalSepLineEdit = new QLineEdit(groupBox_2);
        decimalSepLineEdit->setObjectName(QString::fromUtf8("decimalSepLineEdit"));
        decimalSepLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(decimalSepLineEdit, 0, 1, 1, 1);

        thousandSepLineEdit = new QLineEdit(groupBox_2);
        thousandSepLineEdit->setObjectName(QString::fromUtf8("thousandSepLineEdit"));
        thousandSepLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_3->addWidget(thousandSepLineEdit, 0, 3, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_3->addItem(horizontalSpacer_3, 0, 4, 1, 1);


        gridLayout->addWidget(groupBox_2, 1, 0, 1, 1);

        groupBox_3 = new QGroupBox(LoadFromTXTDialog);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        gridLayout_5 = new QGridLayout(groupBox_3);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        overheadsLineEdit = new QLineEdit(groupBox_3);
        overheadsLineEdit->setObjectName(QString::fromUtf8("overheadsLineEdit"));

        gridLayout_5->addWidget(overheadsLineEdit, 0, 2, 1, 1);

        overheadsLabel = new QLabel(groupBox_3);
        overheadsLabel->setObjectName(QString::fromUtf8("overheadsLabel"));

        gridLayout_5->addWidget(overheadsLabel, 0, 0, 1, 1);

        profitsLabel = new QLabel(groupBox_3);
        profitsLabel->setObjectName(QString::fromUtf8("profitsLabel"));

        gridLayout_5->addWidget(profitsLabel, 0, 3, 1, 1);

        profitsLineEdit = new QLineEdit(groupBox_3);
        profitsLineEdit->setObjectName(QString::fromUtf8("profitsLineEdit"));

        gridLayout_5->addWidget(profitsLineEdit, 0, 4, 1, 1);


        gridLayout->addWidget(groupBox_3, 1, 1, 1, 1);

        groupBox = new QGroupBox(LoadFromTXTDialog);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout_2 = new QGridLayout(groupBox);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        addColButton = new QToolButton(groupBox);
        addColButton->setObjectName(QString::fromUtf8("addColButton"));
        addColButton->setMinimumSize(QSize(30, 0));

        gridLayout_2->addWidget(addColButton, 0, 0, 1, 1);

        delColButton = new QToolButton(groupBox);
        delColButton->setObjectName(QString::fromUtf8("delColButton"));
        delColButton->setMinimumSize(QSize(30, 0));

        gridLayout_2->addWidget(delColButton, 0, 1, 1, 1);

        comboBoxLayout = new QGridLayout();
        comboBoxLayout->setObjectName(QString::fromUtf8("comboBoxLayout"));

        gridLayout_2->addLayout(comboBoxLayout, 1, 0, 1, 3);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 1, 3, 1, 1);

        horizontalSpacer = new QSpacerItem(287, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 0, 2, 1, 2);


        gridLayout->addWidget(groupBox, 2, 0, 1, 2);

        plainTextEdit = new QPlainTextEdit(LoadFromTXTDialog);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setReadOnly(true);

        gridLayout->addWidget(plainTextEdit, 3, 0, 1, 2);

        buttonBox = new QDialogButtonBox(LoadFromTXTDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 4, 0, 1, 2);

        generalOptionsGroupBox = new QGroupBox(LoadFromTXTDialog);
        generalOptionsGroupBox->setObjectName(QString::fromUtf8("generalOptionsGroupBox"));
        gridLayout_4 = new QGridLayout(generalOptionsGroupBox);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        setShortDescFromLongCheckBox = new QCheckBox(generalOptionsGroupBox);
        setShortDescFromLongCheckBox->setObjectName(QString::fromUtf8("setShortDescFromLongCheckBox"));

        gridLayout_4->addWidget(setShortDescFromLongCheckBox, 0, 0, 1, 1);


        gridLayout->addWidget(generalOptionsGroupBox, 0, 0, 1, 2);


        retranslateUi(LoadFromTXTDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), LoadFromTXTDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), LoadFromTXTDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(LoadFromTXTDialog);
    } // setupUi

    void retranslateUi(QDialog *LoadFromTXTDialog)
    {
        LoadFromTXTDialog->setWindowTitle(QCoreApplication::translate("LoadFromTXTDialog", "Dialog", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("LoadFromTXTDialog", "Separatori", nullptr));
        decimalSepLabel->setText(QCoreApplication::translate("LoadFromTXTDialog", "Decimali", nullptr));
        thousandSepLabel->setText(QCoreApplication::translate("LoadFromTXTDialog", "Migliaia", nullptr));
        decimalSepLineEdit->setText(QString());
        thousandSepLineEdit->setText(QString());
        groupBox_3->setTitle(QCoreApplication::translate("LoadFromTXTDialog", "Spese Generali e Utili", nullptr));
        overheadsLabel->setText(QCoreApplication::translate("LoadFromTXTDialog", "Spese Generali", nullptr));
        profitsLabel->setText(QCoreApplication::translate("LoadFromTXTDialog", "Utili", nullptr));
        groupBox->setTitle(QCoreApplication::translate("LoadFromTXTDialog", "Campi", nullptr));
        addColButton->setText(QCoreApplication::translate("LoadFromTXTDialog", "+", nullptr));
        delColButton->setText(QCoreApplication::translate("LoadFromTXTDialog", "-", nullptr));
        generalOptionsGroupBox->setTitle(QCoreApplication::translate("LoadFromTXTDialog", "Opzioni", nullptr));
        setShortDescFromLongCheckBox->setText(QCoreApplication::translate("LoadFromTXTDialog", "Deriva denominazione da descrizione, se non presente", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoadFromTXTDialog: public Ui_LoadFromTXTDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOADFROMTXTDIALOG_H
