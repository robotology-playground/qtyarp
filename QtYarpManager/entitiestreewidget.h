#ifndef ENTITIESTREEWIDGET_H
#define ENTITIESTREEWIDGET_H

#include <QTreeWidget>
#include "manager.h"
#include <QAction>
#include <QMenu>

#if defined(WIN32)
#define TEXTEDITOR "notepad.exe"
#else
#define TEXTEDITOR "gedit"
#endif

class EntitiesTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit EntitiesTreeWidget(QWidget *parent = 0);

    void addApplication(Application *app);
    void addComputer(Computer* comp);
    void addModule(Module* mod);
    void addAppTemplate(AppTemplate* tmp);

    void clearApplication();
    void clearModules();
    void clearResources();
    void clearTemplates();

private:
    QTreeWidgetItem *applicationNode;
    QTreeWidgetItem *modulesNode;
    QTreeWidgetItem *resourcesNode;
    QTreeWidgetItem *templatesNode;

    QMenu topLevelMenu;
    QMenu secondLevelMenu;
    QMenu leafLevelMenu;

    QAction *openFile;
    QAction *importFile;
    QAction *loadFiles;
    QAction *reopen;
    QAction *remove;
    QAction *edit;


signals:
    void viewResource(Computer*);
    void viewModule(Module*);
    void viewApplication(Application*);
    void importFiles();
    void openFiles();
public slots:
    void onContext(QPoint);
    void onLoadFile();
    void onEdit();
    void onRemove();
    void onItemDoubleClicked(QTreeWidgetItem*item, int column);

};

#endif // ENTITIESTREEWIDGET_H
