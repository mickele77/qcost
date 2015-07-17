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
#include "qcostgui.h"

#include "pricelistprintergui.h"
#include "billprintergui.h"
#include "settingsdialog.h"
#include "generaldatagui.h"
#include "pricelistgui.h"
#include "accountingbillgui.h"
#include "billgui.h"
#include "pricelistdbviewer.h"
#include "projectitemsview.h"
#include "accountinggui.h"
#include "accountingtambillgui.h"

#include "billprinter.h"
#include "pricelistprinter.h"
#include "qcostclipboarddata.h"
#include "project.h"
#include "projectdataparentitem.h"
#include "projectaccountingparentitem.h"
#include "accountingbill.h"
#include "accountingtambill.h"
#include "bill.h"
#include "pricelist.h"
#include "mathparser.h"

#include <QLocale>
#include <QGridLayout>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStackedWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QIcon>
#include <QApplication>
#include <QCloseEvent>
#include <QXmlStreamReader>
#include <QClipboard>
#include <QProcess>
#include <QSettings>

class QCostGUIPrivate{
public:
    QCostGUIPrivate( QMainWindow * parent ):
        projectItemsView( new ProjectItemsView(parent)),
        projectItemsViewDock( new QDockWidget(QObject::trUtf8("Vista progetto"), parent)),
        project(NULL),
        parser( QLocale::system() ),
        mainWidget( new QStackedWidget(parent)),
        generalDataGUI(NULL),
        priceListGUI(NULL),
        billGUI(NULL),
        accountingGUI(NULL),
        accountingBillGUI(NULL),
        accountingTAMBillGUI(NULL),
        EPAFileName(){

        projectItemsViewDock->setWidget( projectItemsView );
        parent->addDockWidget( Qt::LeftDockWidgetArea, projectItemsViewDock  );
        parent->setCentralWidget( mainWidget );
    }

    ~QCostGUIPrivate(){
    }

    ProjectItemsView * projectItemsView;
    QDockWidget * projectItemsViewDock;

    Project * project;
    MathParser parser;

    QStackedWidget * mainWidget;
    GeneralDataGUI * generalDataGUI;
    PriceListGUI * priceListGUI;
    BillGUI * billGUI;
    AccountingGUI * accountingGUI;
    AccountingBillGUI * accountingBillGUI;
    AccountingTAMBillGUI * accountingTAMBillGUI;

    // *** GUI ***
    // file corrente
    QFile currentFile;
    // Elenco file recenti
    QStringList recentFiles;

    // Menu
    QMenu * fileMenu;
    QMenu * toolsMenu;
    QMenu * helpMenu;
    // Toolbar
    QToolBar * fileToolBar;
    // Azioni
    QAction * newEmptyProjectAct;
    QAction * newSimpleProjectAct;
    QAction * newProjectHumanNoDiscountAct;
    QAction * openFileAct;
    QAction * printAct;
    QAction * saveAct;
    QAction * saveAsAct;
    QAction * separatorAction;
    QAction * exitAct;
    QAction * viewerPLDBAct;
    QAction * optionsPLDBAct;
    QAction * aboutQtAct;
    QAction * aboutAct;
    QAction * recentFileActions[QCostGUI::MaxRecentFiles];

    // Impostazioni
#ifdef _WIN32
    QString settingsFile;
#endif
    QString sWordProcessorFile;

    QMap<PriceListDBWidget::ImportOptions, bool> EPAImportOptions;
    QString EPAFileName;
};

QCostGUI::QCostGUI(QWidget *parent) :
    QMainWindow(parent),
    m_d( new QCostGUIPrivate(this)){

    connect( m_d->projectItemsView, &ProjectItemsView::currentItemChanged, this, &QCostGUI::setCurrentItem );

    createActions();

    createMenus();
    updateRecentFileActions();
    createToolBars();

    setCurrentProject( new Project( &(m_d->parser) ) );

    m_d->generalDataGUI = new GeneralDataGUI( m_d->project );
    m_d->mainWidget->addWidget( m_d->generalDataGUI);

    m_d->accountingGUI = new AccountingGUI( m_d->project->accounting(), this );
    m_d->mainWidget->addWidget( m_d->accountingGUI );

    m_d->accountingTAMBillGUI = new AccountingTAMBillGUI(  &(m_d->EPAImportOptions), &(m_d->EPAFileName), &(m_d->parser), NULL, m_d->project, &(m_d->sWordProcessorFile), this );
    m_d->mainWidget->addWidget( m_d->accountingTAMBillGUI );

    m_d->accountingBillGUI = new AccountingBillGUI( &(m_d->EPAImportOptions), &(m_d->EPAFileName), &(m_d->parser), NULL, m_d->project, &(m_d->sWordProcessorFile), this );
    m_d->mainWidget->addWidget( m_d->accountingBillGUI );

    m_d->billGUI = new BillGUI( &(m_d->EPAImportOptions), &(m_d->EPAFileName), &(m_d->parser), NULL, m_d->project, &(m_d->sWordProcessorFile), this );
    m_d->mainWidget->addWidget( m_d->billGUI );

    m_d->priceListGUI = new PriceListGUI( &(m_d->EPAImportOptions), &(m_d->EPAFileName), NULL, &(m_d->parser), m_d->project, this );
    m_d->mainWidget->addWidget( m_d->priceListGUI );

    if( m_d->project->billCount() > 0 ){
        m_d->projectItemsView->setCurrentItem( m_d->project->bill(0) );
    }

    setWindowTitle(QString("%1[*] - %2").arg(trUtf8("Senza titolo"))
                   .arg(trUtf8("QCost")));
    connect( m_d->project, &Project::modelChanged, this, static_cast<void(QCostGUI::*)()>(&QCostGUI::setModified) );
    setModified( false );

#ifdef _WIN32
    m_d->settingsFile = QApplication::applicationDirPath() + "/QCost.ini";
#endif
    loadSettings();

    setWindowIcon(QIcon(":/icons/qcost.svg"));
}

QCostGUI::~QCostGUI() {
    delete m_d;
}

void QCostGUI::loadSettings() {
#ifdef _WIN32
    if( QFileInfo(m_d->settingsFile).exists() ){
        QSettings settings( m_d->settingsFile, QSettings::IniFormat );
        m_d->sWordProcessorFile = settings.value("wordProcessorFile", "").toString();
        restoreGeometry(settings.value("geometry").toByteArray());
    }
#else
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "QCost" );
    m_d->sWordProcessorFile = settings.value("wordProcessorFile", "").toString();
    restoreGeometry(settings.value("geometry").toByteArray());
#endif
}

void QCostGUI::saveSettings() {
#ifdef _WIN32
    QSettings settings( m_d->settingsFile, QSettings::IniFormat );
#else
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "QCost" );
#endif

    settings.setValue("geometry", saveGeometry());
    settings.setValue("wordProcessorFile", m_d->sWordProcessorFile);
}

void QCostGUI::setCurrentItem(ProjectItem *item) {
    if( m_d->project ){
        if( dynamic_cast<AccountingTAMBill *>(item)){
            m_d->accountingTAMBillGUI->setAccountingTAMBill( dynamic_cast<AccountingTAMBill *>(item) );
            m_d->mainWidget->setCurrentWidget( m_d->accountingTAMBillGUI );
        } else if( dynamic_cast<AccountingBill *>(item)){
            m_d->accountingBillGUI->setAccountingBill( dynamic_cast<AccountingBill *>(item) );
            m_d->mainWidget->setCurrentWidget( m_d->accountingBillGUI );
        } else if( dynamic_cast<Bill *>(item)){
            m_d->billGUI->setBill( dynamic_cast<Bill *>(item) );
            m_d->mainWidget->setCurrentWidget( m_d->billGUI );
        } else if( dynamic_cast<PriceList *>(item)){
            m_d->priceListGUI->setPriceList( dynamic_cast<PriceList *>(item) );
            m_d->mainWidget->setCurrentWidget( m_d->priceListGUI );
        } else if( dynamic_cast<ProjectAccountingParentItem *>(item)){
            m_d->mainWidget->setCurrentWidget( m_d->accountingGUI );
        } else if( dynamic_cast<ProjectDataParentItem *>(item)){
            m_d->mainWidget->setCurrentWidget( m_d->generalDataGUI );
        }
    }
}

void QCostGUI::createActions(){
    m_d->newEmptyProjectAct = new QAction(tr("&Nuovo Progetto &vuoto"), this);
    m_d->newEmptyProjectAct->setShortcut(tr("Ctrl+n"));
    m_d->newEmptyProjectAct->setStatusTip(tr("Crea un progetto vuoto"));
    m_d->newEmptyProjectAct->setIcon( QIcon(":/icons/document-new-empty.svg"));
    connect(m_d->newEmptyProjectAct, &QAction::triggered, this, &QCostGUI::newProject );

    m_d->newSimpleProjectAct = new QAction(tr("Nuovo Progetto &base"), this);
    m_d->newSimpleProjectAct->setShortcut(tr("Ctrl+b"));
    m_d->newSimpleProjectAct->setStatusTip(tr("Crea un semplice progetto"));
    m_d->newSimpleProjectAct->setIcon( QIcon(":/icons/document-new-simple.svg"));
    connect(m_d->newSimpleProjectAct, &QAction::triggered, this, &QCostGUI::newProject );

    m_d->newProjectHumanNoDiscountAct = new QAction(tr("Nuovo Prog. M.O. non ribassabile"), this);
    m_d->newProjectHumanNoDiscountAct->setShortcut(tr("Ctrl+m"));
    m_d->newProjectHumanNoDiscountAct->setStatusTip(tr("Crea un progetto con costi della manodopera non soggetti a ribasso"));
    m_d->newProjectHumanNoDiscountAct->setIcon( QIcon(":/icons/document-new-human-no-discount.svg"));
    connect(m_d->newProjectHumanNoDiscountAct, &QAction::triggered, this, &QCostGUI::newProject );

    m_d->openFileAct = new QAction(tr("&Apri"), this);
    m_d->openFileAct->setShortcut(tr("Ctrl+A"));
    m_d->openFileAct->setStatusTip(tr("Apre un file esistente"));
    m_d->openFileAct->setIcon( QIcon(":/icons/document-open.svg"));
    connect(m_d->openFileAct, &QAction::triggered, this, &QCostGUI::openFile );

    m_d->saveAct = new QAction(tr("&Salva"), this);
    m_d->saveAct->setShortcut(tr("Ctrl+S"));
    m_d->saveAct->setStatusTip(tr("Salva il progetto corrente"));
    m_d->saveAct->setIcon( QIcon(":/icons/document-save.svg"));
    connect(m_d->saveAct, &QAction::triggered, this, &QCostGUI::save );

    m_d->saveAsAct = new QAction(tr("Salva con &nome..."), this);
    m_d->saveAsAct->setShortcut(tr("Ctrl+Shift+S"));
    m_d->saveAsAct->setStatusTip(tr("Salva il progetto corrente specificando il file di destinazione"));
    m_d->saveAsAct->setIcon( QIcon(":/icons/document-save-as.svg"));
    connect(m_d->saveAsAct, &QAction::triggered, this, &QCostGUI::saveAs );

    m_d->printAct = new QAction(tr("Stam&pa"), this);
    m_d->printAct->setShortcut(tr("Ctrl+Shift+P"));
    m_d->printAct->setStatusTip(tr("Stampa il documento attivo"));
    m_d->printAct->setIcon( QIcon(":/icons/document-print.svg"));
    connect(m_d->printAct, &QAction::triggered, this, &QCostGUI::printODT );

    for (int i = 0; i < MaxRecentFiles; ++i) {
        m_d->recentFileActions[i] = new QAction(this);
        m_d->recentFileActions[i]->setVisible(false);
        connect(m_d->recentFileActions[i], &QAction::triggered,
                this, &QCostGUI::openRecentFile );
    }

    m_d->exitAct = new QAction(tr("&Esci"), this);
    m_d->exitAct->setShortcut(tr("Ctrl+Q"));
    m_d->exitAct->setStatusTip(tr("Esci dall'applicazione"));
    m_d->exitAct->setIcon( QIcon(":/icons/system-log-out.svg"));
    connect(m_d->exitAct, &QAction::triggered, this, &QCostGUI::close );

    m_d->viewerPLDBAct = new QAction(trUtf8("Visualizza &EPA"), this);
    m_d->viewerPLDBAct->setShortcut(tr("Ctrl+E"));
    m_d->viewerPLDBAct->setStatusTip(trUtf8("Visualizzatore Elenco Prezzi Archvio"));
    connect(m_d->viewerPLDBAct, &QAction::triggered, this, &QCostGUI::viewPLDB );

    m_d->optionsPLDBAct = new QAction(trUtf8("&Opzioni"), this);
    m_d->optionsPLDBAct->setShortcut(tr("Ctrl+O"));
    m_d->optionsPLDBAct->setStatusTip(trUtf8("Opzioni disponibili"));
    connect(m_d->optionsPLDBAct, &QAction::triggered, this, &QCostGUI::setOptions );

    m_d->aboutQtAct = new QAction(tr("Informazioni sulle librerie Qt"), this);
    m_d->aboutQtAct->setStatusTip(tr("Mostra informazioni sulle librerie Qt"));
    connect(m_d->aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt );

    m_d->aboutAct = new QAction(tr("&Informazioni su QCost"), this);
    m_d->aboutAct->setStatusTip(tr("Mostra informazioni su QCost"));
    connect(m_d->aboutAct, &QAction::triggered, this, &QCostGUI::about );
}

void QCostGUI::createMenus(){
    m_d->fileMenu = menuBar()->addMenu(trUtf8("&File"));
    m_d->fileMenu->addAction( m_d->newEmptyProjectAct );
    m_d->fileMenu->addAction( m_d->newSimpleProjectAct );
    m_d->fileMenu->addAction( m_d->newProjectHumanNoDiscountAct );
    m_d->fileMenu->addSeparator();
    m_d->fileMenu->addAction( m_d->openFileAct );
    m_d->fileMenu->addAction( m_d->saveAct );
    m_d->fileMenu->addAction( m_d->saveAsAct );
    m_d->fileMenu->addAction( m_d->printAct );
    m_d->separatorAction = m_d->fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i){
        m_d->fileMenu->addAction(m_d->recentFileActions[i]);
    }
    m_d->fileMenu->addSeparator();
    m_d->fileMenu->addAction( m_d->exitAct );

    m_d->toolsMenu = menuBar()->addMenu(trUtf8("&Strumenti"));
    m_d->toolsMenu->addAction( m_d->viewerPLDBAct );
    m_d->toolsMenu->addSeparator();
    m_d->toolsMenu->addAction( m_d->optionsPLDBAct );

    m_d->helpMenu = menuBar()->addMenu(trUtf8("&Aiuto"));
    m_d->helpMenu->addAction( m_d->aboutQtAct );
    m_d->helpMenu->addAction( m_d->aboutAct );
}

void QCostGUI::createToolBars(){
    m_d->fileToolBar = new QToolBar(trUtf8("File"), this );
    m_d->fileToolBar->addAction( m_d->newEmptyProjectAct );
    m_d->fileToolBar->addAction( m_d->newSimpleProjectAct );
    m_d->fileToolBar->addAction( m_d->newProjectHumanNoDiscountAct );
    m_d->fileToolBar->addAction( m_d->openFileAct );
    m_d->fileToolBar->addAction( m_d->saveAct );
    m_d->fileToolBar->addAction( m_d->saveAsAct );
    m_d->fileToolBar->addAction( m_d->printAct );
    m_d->fileToolBar->addAction( m_d->exitAct );
    addToolBar( Qt::TopToolBarArea, m_d->fileToolBar );
}


void QCostGUI::setCurrentProject( Project * proj ){
    m_d->project = proj;
    m_d->projectItemsView->setProject( proj );
    if( dynamic_cast<GeneralDataGUI *>(centralWidget()) ){
        dynamic_cast<GeneralDataGUI *>(centralWidget())->setProject( proj );
    }
}

bool QCostGUI::setCurrentFile(const QString &fileName, bool readContent ) {
    if( (QFileInfo(fileName).fileName() != m_d->currentFile.fileName()) ){
        // chiude il file esistente
        if( m_d->currentFile.exists() ){
            if( m_d->currentFile.isOpen() ){
                m_d->currentFile.close();
            }
        }

        // imposta il nuovo file
        m_d->currentFile.setFileName( fileName );

        // apre e legge il nuovo file
        if( !m_d->currentFile.fileName().isEmpty() ){
            if (!m_d->currentFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
                return false;
            }
            // legge il contenuto del file se richiesto
            if( readContent ){
                m_d->project->clear();
                QXmlStreamReader reader( &m_d->currentFile );
                m_d->project->readXml( &reader );
            }
        }

        // aggiorna il titolo della finestra
        QString shownName = trUtf8("Senza titolo");
        if( m_d->currentFile.isOpen() ){
            shownName = m_d->currentFile.fileName();
            m_d->recentFiles.removeAll(fileName);
            m_d->recentFiles.prepend(fileName);
            updateRecentFileActions();
        }
        setWindowTitle(QString("%1[*] - %2").arg(shownName)
                       .arg(tr("QCost")));

        setModified(false);

        return true;
    }
    return false;
}

void QCostGUI::closeEvent(QCloseEvent *event) {
    if (okToContinue()) {
        if( m_d->currentFile.isOpen() ){
            m_d->currentFile.close();
        }
        event->accept();
    } else {
        event->ignore();
    }
    saveSettings();
}

bool QCostGUI::okToContinue() {
    if (isWindowModified()) {
        int r = QMessageBox::warning(this, tr("QCost"),
                                     trUtf8("Il progetto è stato modificato.\n"
                                            "Vuoi salvare i cambiamenti?"),
                                     QMessageBox::Yes | QMessageBox::No
                                     | QMessageBox::Cancel);
        if (r == QMessageBox::Yes) {
            return save();
        } else if (r == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void QCostGUI::newProject() {
    QAction * callingAction = dynamic_cast<QAction *> (sender());
    if( callingAction ){
        if (okToContinue()) {
            if( m_d->project ){
                m_d->mainWidget->setCurrentWidget( m_d->generalDataGUI );
                if( callingAction == m_d->newEmptyProjectAct ){
                    m_d->project->createSimpleProject( Project::ProjectEmpty );
                } else if( callingAction == m_d->newSimpleProjectAct ){
                    m_d->project->createSimpleProject( Project::ProjectSimple );
                } else if( callingAction == m_d->newProjectHumanNoDiscountAct ){
                    m_d->project->createSimpleProject( Project::ProjectHumanNetNoDiscount );
                } else {
                    m_d->project->clear();
                }
                setCurrentFile("");
            }
        }
    }
}

void QCostGUI::openFile() {
    if (okToContinue()) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        trUtf8("Apri il progetto"), ".",
                                                        trUtf8("File progetto QCost(*.qct)"));
        setCurrentFile( fileName, true );
    }
}

void QCostGUI::openRecentFile()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action){
            setCurrentFile( action->data().toString() );
        }
    }
}

bool QCostGUI::save(){
    if ( m_d->currentFile.isOpen() ) {
        return saveCurrentFile();
    } else {
        return saveAs();
    }
}

bool QCostGUI::saveAs(){
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    trUtf8("Salva il progetto"), ".",
                                                    trUtf8("File progetto QCost(*.qct)"));
    if (fileName.isEmpty()){
        return false;
    } else {
        setCurrentFile( fileName );
        return saveCurrentFile();
    }
}

bool QCostGUI::printODT() {
    PriceList * pl = dynamic_cast< PriceList *> (m_d->projectItemsView->currentItem());
    if( pl ){
        PriceListPrinter::PrintPriceItemsOption printItemsOption = PriceListPrinter::PrintLongDesc;
        QList<int> fieldsToPrint;
        int priceDataSetToPrint = 0;
        double paperWidth = 210.0;
        double paperHeight = 297.0;
        Qt::Orientation paperOrientation = Qt::Vertical;
        bool printPriceList = true;
        bool printAP = false;
        bool APgroupPrAm = false;
        PriceListPrinterGUI gui( &printItemsOption, &fieldsToPrint, &paperWidth, &paperHeight, &paperOrientation, &priceDataSetToPrint, &printPriceList, &printAP, &APgroupPrAm, pl->priceDataSetCount(), m_d->project->priceFieldModel(), this );

        if( gui.exec() == QDialog::Accepted ){
            QString fileName = QFileDialog::getSaveFileName(this,
                                                            trUtf8("Stampa Elenco Prezzi"), ".",
                                                            trUtf8("Documento di testo (*.odt)"));
            if (!fileName.isEmpty()){
                QString suf = fileName.split(".").last().toLower();
                if( suf != "odt"){
                    fileName.append( ".odt" );
                }
                PriceListPrinter writer( pl );
                bool ret = writer.printODT( printItemsOption, fieldsToPrint, priceDataSetToPrint, printPriceList, printAP, APgroupPrAm, fileName, paperWidth, paperHeight, paperOrientation );
                if( !m_d->sWordProcessorFile.isEmpty() ){
                    if( QFileInfo(m_d->sWordProcessorFile).exists() ){
                        QStringList args;
                        args << fileName;
                        QProcess *proc = new QProcess(this);
                        proc->start(m_d->sWordProcessorFile, args);
                    }
                }
                return ret;
            }
        }
        return false;
    }

    Bill * b = dynamic_cast< Bill *> (m_d->projectItemsView->currentItem());
    if( b != NULL ){
        BillPrinter::PrintBillItemsOption prItemsOption = BillPrinter::PrintLongDesc;
        BillPrinter::PrintOption prOptions = BillPrinter::PrintBill;
        QList<int> fieldsToPrint;
        double paperWidth = 210.0, paperHeight = 297.0;
        Qt::Orientation paperOrientation = Qt::Vertical;
        bool groupPrAm = false;
        BillPrinterGUI gui( &prItemsOption, &prOptions, &fieldsToPrint, &paperWidth, &paperHeight, &paperOrientation,  &groupPrAm, m_d->project->priceFieldModel(), this );
        if( gui.exec() == QDialog::Accepted ){
            QString fileName = QFileDialog::getSaveFileName(this,
                                                            trUtf8("Stampa Computo Metrico Estimativo"), ".",
                                                            trUtf8("Documento di testo (*.odt)"));
            if (!fileName.isEmpty()){
                QString suf = fileName.split(".").last().toLower();
                if( suf != "odt"){
                    fileName.append( ".odt" );
                }
                BillPrinter writer( b, m_d->project->priceFieldModel(), &(m_d->parser) );
                bool ret = writer.printODT( prItemsOption, prOptions, fieldsToPrint, fileName, paperWidth, paperHeight, paperOrientation, groupPrAm );
                if( !m_d->sWordProcessorFile.isEmpty() ){
                    if( QFileInfo(m_d->sWordProcessorFile).exists() ){
                        QStringList args;
                        args << fileName;
                        QProcess *proc = new QProcess(this);
                        proc->start(m_d->sWordProcessorFile, args);
                    }
                }
                return ret;
            }
        }
        return false;
    }

    return false;
}

bool QCostGUI::saveCurrentFile(){
    if( m_d->currentFile.isOpen() && m_d->currentFile.isWritable() ){
        m_d->currentFile.resize(0);
        QXmlStreamWriter writer(&m_d->currentFile);
        if( m_d->project ){
            m_d->project->writeXml( &writer );
        }
        m_d->currentFile.flush();
        setModified( false );
        return true;
    }
    return false;
}

void QCostGUI::updateRecentFileActions() {
    QMutableStringListIterator i(m_d->recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < m_d->recentFiles.count()) {
            QString text = QString("&%1 %2")
                    .arg(j + 1)
                    .arg(QFileInfo( m_d->recentFiles[j]).fileName() );
            m_d->recentFileActions[j]->setText(text);
            m_d->recentFileActions[j]->setData(m_d->recentFiles[j]);
            m_d->recentFileActions[j]->setVisible(true);
        } else {
            m_d->recentFileActions[j]->setVisible(false);
        }
    }
    m_d->separatorAction->setVisible(!m_d->recentFiles.isEmpty());
}

void QCostGUI::viewPLDB(){
    PriceListDBViewer pldbViewer( &(m_d->parser), this );
    pldbViewer.exec();
}

void QCostGUI::setOptions(){
    SettingsDialog settingsDialog( &(m_d->sWordProcessorFile), this );
    if( settingsDialog.exec() == QDialog::Accepted ){
        saveSettings();
    }
}

void QCostGUI::about() {
    QMessageBox::about(this, trUtf8("Informazioni su QCost"),
                       trUtf8("<h1>QCost</h1>"
                              "<h2>v0.9.0</h2>"
                              "<p>QCost è un software per la redazione di computi metrici estimativi."
                              "<p>Questo programma è software libero; puoi ridistribuirlo e/o modificarlo nei termini della GNU General Public License cos' come pubblicata dalla Free Software Foundation; sia nei termini della versione 3 della licenza che (a tua scelta) nei termini di qualsiasi altra versione successiva."
                              "<p>Questo programma è distribuito nella speranza che sia utile, ma SENZA ALCUNA GARANZIA; neanche l'implicita garanzia di COMMERCIABILITÀ o di IDONEITÀ AD UN PARTICOLARE IMPIEGO. Consulta la GNU General Public License per maggiori dettagli. "
                              "<p>Con il programma dovresti aver ricevuto una copia della GNU General Public License; qualora non l'avessi ricevuta, scrivi alla Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA."
                              "<p>Se incontri problemi nell'uso del programma, lascia un post nel <a href=\"http://ingegnerialibera.altervista.org/forum/viewforum.php?f=3\">forum di QCost</a>.<p>Il sito di riferimento è <a href=\"http://ingegnerialibera.altervista.org/wiki/doku.php/qcost:indice\">ingegnerialibera.altervista.org/wiki/doku.php/qcost:indice</a>." ) );
}

void QCostGUI::setModified(bool v) {
    m_d->saveAct->setEnabled( v );
    setWindowModified( v );
}

void QCostGUI::setModified() {
    setModified( true );
}
