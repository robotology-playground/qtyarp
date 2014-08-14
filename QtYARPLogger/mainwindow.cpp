#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logtab.h"
#include "ui_logtab.h"
#include <QString>
#include <QMenu>
#include <QTextStream>
#include <Ctime>
#include <yarp/os/YarprunLogger.h>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QDesktopServices>
#include <QUrl>

void MainWindow::updateMain()
{
    /*std::list<yarp::os::YarprunLogger::MessageEntry> messages;
    this->theLogger->get_messages(messages);
    QStringList List;
    std::list<yarp::os::YarprunLogger::MessageEntry>::iterator it;
    for (it=messages.begin(); it!=messages.end(); it++)
    {
        List << it->text.c_str();
    }
    model->setStringList(List);*/
    
    //model_yarprunports->clear();
    model_yarprunports->setHorizontalHeaderItem(0,new QStandardItem("yarprun"));
    model_yarprunports->setHorizontalHeaderItem(1,new QStandardItem("process"));
    model_yarprunports->setHorizontalHeaderItem(2,new QStandardItem("last heard"));
    model_yarprunports->setHorizontalHeaderItem(3,new QStandardItem("log size"));
    std::list<yarp::os::YarprunLogger::LogEntryInfo> infos;
    this->theLogger->get_infos (infos);
    std::list<yarp::os::YarprunLogger::LogEntryInfo>::iterator it;

    QStandardItem *itemsRoot = model_yarprunports->invisibleRootItem();
    for (it=infos.begin(); it!=infos.end(); it++)
    {
        const size_t string_size= 50;
        char time_text[string_size];
        std::tm* tm = localtime(&it->last_update);
        if (tm)
        sprintf ( time_text,"%02d:%02d:%02d",tm->tm_hour,tm->tm_min, tm->tm_sec);
        else
        sprintf ( time_text, "no data received yet");
        
        char logsize_text[string_size];
        sprintf (logsize_text, "%d", it->logsize);

//#define TREE_MODEL 1
#define IN_ROW_MODEL 1
#if IN_ROW_MODEL
        bool existing = false;
        for (int i=0; i<model_yarprunports->rowCount(); i++)
        {
            QStandardItem *item = model_yarprunports->item(i,1);
            if (item && item->text()==it->port_complete.c_str())
            {
                model_yarprunports->item(i,2)->setText(time_text);
                model_yarprunports->item(i,3)->setText(logsize_text);
                existing = true;
                break;
            }
        }
        if (existing == false)
        {
            QList<QStandardItem *> rowItems;
            rowItems << new QStandardItem(it->port_prefix.c_str()) << new QStandardItem(it->port_complete.c_str()) << new QStandardItem(time_text) << new QStandardItem(logsize_text);
            itemsRoot->appendRow(rowItems);
        }

#elif TREE_MODEL
        bool level1_exists = false;
        bool level2_exists = false;
        for (int i=0; i<model_yarprunports->rowCount(); i++)
        {
            QStandardItem *item_level1 = model_yarprunports->item(i,0);
            if (item_level1 && item_level1->text() == it->port_prefix.c_str())
            {
                level1_exists = true;
                for (int j=0; j<item_level1->rowCount(); j++)
                {
                    QStandardItem *item_level2 = item_level1->child(j,0);
                    if (item_level2 && item_level1->child(j,1)->text() == it->port_complete.c_str())
                    {
                        level2_exists = true;
                        item_level1->child(j,2)->text() = time_text;
                        break;
                    }
                }
                if (level2_exists == false)
                {
                    QList<QStandardItem *> rowItems;
                    rowItems << new QStandardItem(it->port_prefix.c_str()) << new QStandardItem(it->port_complete.c_str()) << new QStandardItem(time_text);
                    item_level1->appendRow(rowItems);
                    break;
                }
            }
        }
        if (level1_exists == false)
        {
            /*QList<QStandardItem *> rowItems_l1;
            QList<QStandardItem *> rowItems_l2;
            rowItems_l1 << new QStandardItem(it->port_prefix.c_str());
            rowItems_l2 << new QStandardItem(it->port_prefix.c_str()) << new QStandardItem(it->port_complete.c_str()) << new QStandardItem(time_text);
            rowItems_l1->appendRow(rowItems_l2);
            itemsRoot->appendRow(rowItems_l1);*/
            QList<QStandardItem *> rowItems_l1;
            rowItems_l1 << new QStandardItem(it->port_prefix.c_str());
            itemsRoot->appendRow(rowItems_l1);
        }
        /*if (existing == false)
        {
            QList<QStandardItem *> rowItems;
            rowItems << new QStandardItem(it->port_prefix.c_str()) << new QStandardItem(it->port_complete.c_str()) << new QStandardItem(time_text);
            itemsRoot->appendRow(rowItems);
        }
        bool rowfound = false;
        for (int i=0; i<model_yarprunports->rowCount(); i++)
        {
            QStandardItem *rowParent = model_yarprunports->item(i,0);
            if (rowParent && rowParent->text()==it->port_prefix.c_str())
            {
                rowParent->appendRow(rowItems);
                rowfound = true;
            }
        }
        if  (rowfound==false)
        {
            itemsRoot->appendRow(rowItems);
        }*/
#endif
    }
}

void MainWindow::ctxMenu(const QPoint &pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);
    int model_row=index.row();
    QString logname = model_yarprunports->item(model_row,1)->text();
    theLogger->clear_messages_by_port_complete(logname.toStdString());
    for (int i=0; i<ui->logtabs->count(); i++)
        if (ui->logtabs->tabText(i) == logname) 
            {
                LogTab* l = (LogTab*) ui->logtabs->widget(i);
                l->clear_model_logs();
            }

    ///@@@@@ to be completed
    QMenu *menu = new QMenu;
    menu->addAction(tr("Test Item"), this, SLOT(test_slot()));
    menu->exec(ui->treeView->mapToGlobal(pos));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    theLogger = new yarp::os::YarprunLogger::LoggerEngine("/logger");

    ui->setupUi(this);

    model_yarprunports = new QStandardItemModel(this);
    statusBarLabel = new QLabel;
    statusBarLabel->setText("Ready");
    statusBarLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar()->addWidget(statusBarLabel);

    mainTimer = new QTimer(this);
    connect(mainTimer, SIGNAL(timeout()), this, SLOT(updateMain()));
    mainTimer->start(500);

    connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ctxMenu(const QPoint &)));

    ui->stopLogger->setEnabled(false);
    ui->refreshLogger->setEnabled(false);
}

MainWindow::~MainWindow()
{
    this->theLogger->stop_logging();
    if (mainTimer) {delete mainTimer; mainTimer=0;}
    if (ui)        {delete ui; ui=0;}
}

void MainWindow::loadTextFile()
{
    /*
    QFile inputFile("c:\\ptcsetup.log");
    inputFile.open(QIODevice::ReadOnly);

    QTextStream in(&inputFile);
    QString line = in.readAll();
    inputFile.close();

    ui->textEdit->setPlainText(line);
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
    */
}

void MainWindow::on_lineEdit_2_textChanged(const QString &arg1)
{
    QString filter = "*";
    filter.append(arg1);
    filter.append("*");
    QRegExp regExp(filter, Qt::CaseInsensitive, QRegExp::Wildcard);

    int currentTab = ui->logtabs->currentIndex();
    LogTab* logtab = ui->logtabs->currentWidget()->findChild<LogTab*>("logtab");

    logtab->proxyModel->setFilterRegExp(regExp);
}

void MainWindow::on_startLogger_clicked()
{
     statusBarLabel->setText("Running");
     this->theLogger->start_logging();

    ui->startLogger->setEnabled(false);
    ui->stopLogger->setEnabled(true);
    ui->refreshLogger->setEnabled(true);
}


void MainWindow::on_stopLogger_clicked()
{
    statusBarLabel->setText("Stopped");
     this->theLogger->stop_logging();

     ui->stopLogger->setEnabled(false);
     ui->startLogger->setEnabled(true);
     ui->refreshLogger->setEnabled(false);
}

void MainWindow::on_refreshLogger_clicked()
{
    statusBarLabel->setText("Searching for yarprun ports");
    std::list<std::string> ports;
    theLogger->discover(ports);
    updateMain();
    theLogger->connect(ports);
    ui->treeView->setModel(model_yarprunports);
    ui->treeView->expandAll();
    statusBarLabel->setText("Running");

    /*
    QStringList List;
    std::list<std::string>::iterator it;
    for (it=ports.begin(); it!=ports.end(); it++)
    {
        List << (*it).c_str() ;
    }
    model_yarprunports->setStringList(List);
    */
    
    /*QStringList l1;
    l1 << "yarprun ports";
    ui->treeView->setHorizontalHeaderLabels (l1);
    model_yarprunports->setHeaderData(0);*/

    
    /*
    std::list<std::string>::iterator it;
    for (it=ports.begin(); it!=ports.end(); it++)
    {

    }
    QStandardItemModel* model = new QStandardItemModel();
    QStandardItem* item0 = new QStandardItem(QIcon("test.png"), "1 first item");
    QStandardItem* item1 = new QStandardItem(QIcon("test.png"), "2 second item");
    QStandardItem* item3 = new QStandardItem(QIcon("test.png"), "3 third item");
    QStandardItem* item4 = new QStandardItem("4 forth item");
    model->appendRow(item0);
    item0->appendRow(item3);
    item0->appendRow(item4);
    model->appendRow(item1);
    ui->treeView->setModel(model);
    */
}

void MainWindow::on_logtabs_tabCloseRequested(int index)
{
    delete ui->logtabs->widget(index);
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    QTabWidget* tab = new QTabWidget(this);
    int model_row=index.row();
    QString tabname = model_yarprunports->item(model_row,1)->text();
    LogTab* tmpLogTab = new LogTab(theLogger, tabname.toStdString(), tab);
    tmpLogTab->setObjectName("logtab");

    int exists = -1;
    for (int i=0; i<ui->logtabs->count(); i++)
        if (ui->logtabs->tabText(i) == tabname) exists = i;

    if (exists>=0)
    {
        ui->logtabs->setCurrentIndex(exists);
    }
    else
    {
        int newtab_index = ui->logtabs->addTab(tab, tabname);
        ui->logtabs->setCurrentIndex(newtab_index);
    }
}

void MainWindow::on_DisplayErrorEnable_toggled(bool checked)
{
    QRegExp regExp ("*", Qt::CaseInsensitive, QRegExp::Wildcard);
    if (checked) regExp.setPattern("*");
    else         regExp.setPattern("*");
    for (int i=0; i<ui->logtabs->count(); i++)
    {
        LogTab* logtab = ui->logtabs->widget(i)->findChild<LogTab*>("logtab");
        if (logtab) logtab->proxyModel->setFilterRegExp(regExp);
    }
}

void MainWindow::on_DisplayWarningEnable_toggled(bool checked)
{
    QRegExp regExp ("*", Qt::CaseInsensitive, QRegExp::Wildcard);
    if (checked) regExp.setPattern("*");
    else         regExp.setPattern("*");
    for (int i=0; i<ui->logtabs->count(); i++)
    {
        LogTab* logtab = ui->logtabs->widget(i)->findChild<LogTab*>("logtab");
        if (logtab) logtab->proxyModel->setFilterRegExp(regExp);
    }
}

void MainWindow::on_DisplayDebugEnable_toggled(bool checked)
{
    QRegExp regExp ("*", Qt::CaseInsensitive, QRegExp::Wildcard);
    if (checked) regExp.setPattern("*");
    else         regExp.setPattern("*");
    for (int i=0; i<ui->logtabs->count(); i++)
    {
        LogTab* logtab = ui->logtabs->widget(i)->findChild<LogTab*>("logtab");
        if (logtab) logtab->proxyModel->setFilterRegExp(regExp);
    }
}

void MainWindow::on_DisplayInfoEnable_toggled(bool checked)
{
    QRegExp regExp ("*", Qt::CaseInsensitive, QRegExp::Wildcard);
    if (checked) regExp.setPattern("*");
    else         regExp.setPattern("*");
    for (int i=0; i<ui->logtabs->count(); i++)
    {
        LogTab* logtab = ui->logtabs->widget(i)->findChild<LogTab*>("logtab");
        if (logtab) logtab->proxyModel->setFilterRegExp(regExp);
    }
}

void MainWindow::on_DisplayUnformattedEnable_toggled(bool checked)
{
    QRegExp regExp ("*", Qt::CaseInsensitive, QRegExp::Wildcard);
    if (checked) regExp.setPattern("*");
    else         regExp.setPattern("*");
    for (int i=0; i<ui->logtabs->count(); i++)
    {
        LogTab* logtab = ui->logtabs->widget(i)->findChild<LogTab*>("logtab");
        if (logtab) logtab->proxyModel->setFilterRegExp(regExp);
    }
}

void MainWindow::on_actionShow_Timestamps_toggled(bool arg1)
{
    for (int i=0; i<ui->logtabs->count(); i++)
    {
        LogTab* logtab = ui->logtabs->widget(i)->findChild<LogTab*>("logtab");
        if (logtab) logtab->displayTimestamp=arg1;
    }
}

void MainWindow::on_actionAbout_QtYarpLogger_triggered()
{
    QDesktopServices::openUrl(QUrl("http://wiki.icub.org/yarpdoc/qtyarplogger.html"));
}

void MainWindow::on_actionStart_Logger_triggered()
{
    this->theLogger->start_logging();

    QMenu *m=ui->menuFile;
   // m->fi
        //QMenu::actions.at(0).setEnabled(false);

  //  ui->startLogger->setEnabled(false);
 //   ui->stopLogger->setEnabled(true);
  //  ui->refreshLogger->setEnabled(true);
}
