#ifndef WFCONFIGURATIONTABHANDLER_H
#define WFCONFIGURATIONTABHANDLER_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include "configurationdialog.h"
#include "ui_configurationdialog.h"
#include "wfgraphicnodeitems.h"
#include "wfnodehandlers.h"


static int const ALIGNMENT_LEFT_BORDER=1;
static int const ALIGNMENT_RIGHT_BORDER=2;
static int const ALIGNMENT_HOR_CENTER=3;
static int const ALIGNMENT_TOP_BORDER=4;
static int const ALIGNMENT_BOTTOM_BORDER=5;
static int const ALIGNMENT_VER_CENTER=6;



class WorkflowListItem:public QListWidgetItem {
  public:
    WFWorkflow*workflow;
    WorkflowListItem(QListWidget*widget, WFWorkflow*workflow) : QListWidgetItem(widget) {
        this->workflow = workflow;
    }
};

class WorkflowConfigurationTabHandler {
    AppGlobals*appGlobals;
    ConfigurationDialog*dialog;
    WorkflowConfiguration*configuration;
    Ui::ConfigurationDialog *ui;
    CustomGraphicsScene*scene;
    QGraphicsView*view;

  public:
    WorkflowConfigurationTabHandler(AppGlobals*appGlobals,ConfigurationDialog*dialog, WorkflowConfiguration*configuration, Ui::ConfigurationDialog *ui) {
        this->appGlobals = appGlobals;
        this->dialog = dialog;
        this->configuration = configuration;
        this->ui = ui;
    }

    void init() {
        scene=new CustomGraphicsScene();
        scene->setContextMenuGenerator([this](QMenu*menu,QPointF screenPos) {createPopupMenu(menu, screenPos);});
        view=ui->wfScene;
        view->setScene(scene);
        view->setDragMode(QGraphicsView::RubberBandDrag);
        scene->setSceneRect(0, 0, 10000, 10000);
        QObject::connect(ui->wfAddWorkflowButton, &QPushButton::pressed,ui->wfAddWorkflowButton, [this](){addNewWorkflow();});
        QObject::connect(ui->wfRemoveWorkflowButton, &QPushButton::pressed,ui->wfRemoveWorkflowButton, [this](){removeSelectedWorkflow();});
        QObject::connect(ui->wfWorkflowsList, &QListWidget::doubleClicked,ui->wfWorkflowsList, [this](){editWorkflowDetails();});
        QObject::connect(ui->wfWorkflowsList, &QListWidget::itemSelectionChanged,ui->wfWorkflowsList, [this](){onWorkflowSelected();});

        for(int i=0;i<configuration->workflows.size();i++) {
            WFWorkflow*workflow=&configuration->workflows[i];
            WorkflowListItem*listItem=new WorkflowListItem( ui->wfWorkflowsList, workflow);
            listItem->setText(workflow->name);
            ui->wfWorkflowsList->addItem(listItem);
        }
        if(ui->wfWorkflowsList->count()>0) {
            ui->wfWorkflowsList->setCurrentRow(0);
            onWorkflowSelected();
        }

        updateEditableStatus();
    }

    QAction*createMenuAction(QString text, std::function<void()>onClick) {
        QAction*action=new QAction(text);
        QObject::connect(action, &QAction::triggered, action, onClick);
        return action;
    }

    void createPopupMenu(QMenu*menu,QPointF scenePos) {
        //Add triggers items
        QAction*addKeywordAction=createMenuAction("Keyword", [scenePos, this]() {
            addNodeItem(addWFNodeAndHandlerByType("keyword"), scenePos);
            dialogModified();
        });
        QAction*addHotkeyAction=createMenuAction("Hotkey", [scenePos, this]() {
            addNodeItem(addWFNodeAndHandlerByType("hotkey"), scenePos);
            dialogModified();
        });
        QAction*addExternalTriggerAction=createMenuAction("External trigger", [scenePos, this]() {
            addNodeItem(addWFNodeAndHandlerByType("externalTrigger"), scenePos);
            dialogModified();
        });

        QMenu*triggersSubMenu=new QMenu("Triggers");
        triggersSubMenu->addAction(addKeywordAction);
        triggersSubMenu->addAction(addHotkeyAction);
        triggersSubMenu->addAction(addExternalTriggerAction);

        //Add filter items
        QAction*addExtScriptAction=createMenuAction("External Script", [scenePos, this]() {
           addNodeItem(addWFNodeAndHandlerByType("extScript"), scenePos);
           dialogModified();
        });
        QMenu*transformersSubMenu=new QMenu("Transformers");
        transformersSubMenu->addAction(addExtScriptAction);

        //Add actions items
        QAction*addExecAction=createMenuAction("Execute App", [scenePos, this]() {
           addNodeItem(addWFNodeAndHandlerByType("execapp"), scenePos);
           dialogModified();
        });
        //Add actions items
        QAction*addNotificationAction=createMenuAction("Notification", [scenePos, this]() {
           addNodeItem(addWFNodeAndHandlerByType("notification"), scenePos);
           dialogModified();
        });
        QAction*addPlaySoundAction=createMenuAction("Play Sound", [scenePos, this]() {
           addNodeItem(addWFNodeAndHandlerByType("playSound"), scenePos);
           dialogModified();
        });
        QMenu*actionsSubMenu=new QMenu("Actions");
        actionsSubMenu->addAction(addExecAction);
        actionsSubMenu->addAction(addNotificationAction);
        actionsSubMenu->addAction(addPlaySoundAction);


        //Add align items
        QMenu*alignSubMenu=new QMenu("Align");
        alignSubMenu->addAction(createMenuAction("Left Border", [this, scenePos]() {
            alignSelectedItems(ALIGNMENT_LEFT_BORDER, scenePos);
            dialogModified();
        }));
        alignSubMenu->addAction(createMenuAction("Right Border", [this, scenePos]() {
            alignSelectedItems(ALIGNMENT_RIGHT_BORDER, scenePos);
            dialogModified();
        }));
        alignSubMenu->addAction(createMenuAction("H. Center", [this, scenePos]() {
            alignSelectedItems(ALIGNMENT_RIGHT_BORDER, scenePos);
            dialogModified();
        }));
        alignSubMenu->addSeparator();
        alignSubMenu->addAction(createMenuAction("Top Border", [this, scenePos]() {
            alignSelectedItems(ALIGNMENT_TOP_BORDER, scenePos);
            dialogModified();
        }));
        alignSubMenu->addAction(createMenuAction("Bottom Border", [this, scenePos]() {
            alignSelectedItems(ALIGNMENT_BOTTOM_BORDER, scenePos);
            dialogModified();
        }));
        alignSubMenu->addAction(createMenuAction("V. Center", [this, scenePos]() {
            alignSelectedItems(ALIGNMENT_VER_CENTER, scenePos);
            dialogModified();
        }));

        //Other
        QAction*deleteItem=new QAction("Delete");
        deleteItem->setEnabled(!scene->selectedItems().isEmpty());
        QObject::connect(deleteItem, &QAction::triggered, deleteItem,[this]() {deleteSelectedItem();});
        QAction*showPropertiesItem=new QAction("Properties");
        showPropertiesItem->setEnabled(isNodeIsSelected());
        QObject::connect(showPropertiesItem, &QAction::triggered, showPropertiesItem,[this]() {
            showSelectedNodeProperties();
            scene->clearSelection();

        });

        menu->addMenu(triggersSubMenu);
        menu->addMenu(transformersSubMenu);
        menu->addMenu(actionsSubMenu);
        menu->addSeparator();
        menu->addMenu(alignSubMenu);
        menu->addSeparator();
        menu->addAction(deleteItem);
        menu->addSeparator();
        menu->addAction(showPropertiesItem);
    }

    void alignSelectedItems(int alignment, QPointF clickPosition=QPointF(-1,-1)) {
        QList<WFNodeItem*>nodes=getSelectedNodeItems();
        if(nodes.size()<=1) {
            return;
        }

        //trying to determine the base item that will be used as anchor to align other items
        //as a fallback - first item will be used
        QRectF baseRect=nodes.first()->sceneBoundingRect();
        //it is horizontal alignment
        foreach(WFNodeItem*node, nodes) {
            QRectF rect=node->sceneBoundingRect();
            //if it is item user clicked -  it will be anchor item
            if(rect.contains(clickPosition)) {
                baseRect=rect;
                break;
            }
            //if align horizontally - anchor item will be the most left item.
            //if align vertically - anchor item will be the most top item.
            if(alignment==ALIGNMENT_LEFT_BORDER||alignment==ALIGNMENT_RIGHT_BORDER||alignment==ALIGNMENT_HOR_CENTER) {
                if(rect.left()<baseRect.left()) {
                    baseRect=rect;
                }
            }else {
                if(rect.top()<baseRect.top()) {
                    baseRect=rect;
                }
            }
        }

        for(int i=0;i<nodes.size();i++) {
            WFNodeItem*node=nodes[i];
            QRectF rect=node->sceneBoundingRect();
            if(alignment==ALIGNMENT_LEFT_BORDER) {
                moveNode(node, baseRect.left()-rect.left(), 0);
            }
            if(alignment==ALIGNMENT_RIGHT_BORDER) {
                moveNode(node, baseRect.right()-rect.right(), 0);
            }
            if(alignment==ALIGNMENT_HOR_CENTER) {
                moveNode(node, baseRect.center().x()-rect.center().x(), 0);
            }

            if(alignment==ALIGNMENT_TOP_BORDER) {
                moveNode(node, 0, baseRect.top()-rect.top());
            }
            if(alignment==ALIGNMENT_BOTTOM_BORDER) {
                moveNode(node, 0, baseRect.bottom()-rect.bottom());
            }
            if(alignment==ALIGNMENT_VER_CENTER) {
                moveNode(node, 0, baseRect.center().y()-rect.center().y());
            }
        }
    }

    void moveNode(WFNodeItem*node, float dx, float dy) {
        node->moveBy(dx, dy);
    }

    bool splitLinkString(QString linkString, int&portIndex, QString&nodeId) {
        int colonIndex=linkString.indexOf(':');
        if(colonIndex==-1){
            return false;
        }
        QStringList parts=linkString.split(':');
        portIndex=parts[0].toInt();
        nodeId=parts[1];
        return true;
    }

    void onWorkflowSelected() {
        scene->clear();
        updateEditableStatus();
        WFWorkflow*wf=getSelectedWorkflow();
        if(wf==NULL) {
            return;
        }

        QList<WFNode>&nodes=wf->nodes;
        for(int i=0;i<nodes.size();i++) {
            WFNode*node=&nodes[i];
            WFNodeHandler*handler=createWFHandlerByType(node->nodeType, node, appGlobals);
            if(handler==NULL){
                qDebug()<<"Unknown node type:"<<node->nodeType;
                continue;
            }

            node->handler=handler;
            addNodeItem(node, QPointF(node->x, node->y));
        }

        QMap<QString, QList<QString>>&links= wf->links;
        QList<QString>sourceLinkStrings=links.keys();
        for(int i=0;i<sourceLinkStrings.size();i++) {
            QString sourceLinkString=sourceLinkStrings[i];
            int sourcePortIndex=0;
            QString sourceNodeId;
            splitLinkString(sourceLinkString, sourcePortIndex, sourceNodeId);
            QList<QString>&linked=links[sourceLinkString];
            WFNodeItem*sourceNode=getWFNodeItemById(sourceNodeId);
            if(sourceNode==NULL) {
                links.remove(sourceLinkString);
                continue;
            }

            OutputPortItem*sourcePort=NULL;
            if(sourcePortIndex>=sourceNode->outputs.size()) {
                links.remove(sourceLinkString);
                continue;
            }

            sourcePort=sourceNode->outputs[sourcePortIndex];
            for(int j=0;j<linked.size();j++) {
                QString destLinkString = linked[j];
                int destPortIndex=0;
                QString destNodeId;
                splitLinkString(destLinkString, destPortIndex, destNodeId);
                WFNodeItem*destNode=getWFNodeItemById(destNodeId);
                if(destNode==NULL) {
                    linked.takeAt(j);
                    j--;
                    continue;
                }
                InputPortItem*destPort=NULL;
                if(destPortIndex>=destNode->inputs.size()) {
                    linked.takeAt(j);
                    j--;
                    continue;
                }
                destPort=destNode->inputs[destPortIndex];
                sourcePort->createLink(destPort, false);
            }
        }
    }

    void addNewWorkflow() {
        QVariantMapWithCheck result;
        WfEditDetailsDialog addDetailsDialog(dialog);
        addDetailsDialog.setWindowTitle("Add workflow");
        if(addDetailsDialog.editWorkflowDetails(result, "", "", "", "")) {
            WFWorkflow workflow;
            workflow.workflowId=generateRandomString();
            workflow.name=result.getString("name");
            workflow.description=result.getString("description");
            workflow.author=result.getString("author");
            workflow.website=result.getString("website");
            workflow.versionId=generateRandomString();
            configuration->workflows.append(workflow);
            WFWorkflow*wfPointer=&configuration->workflows.last();
            WorkflowListItem*listItem=new WorkflowListItem( ui->wfWorkflowsList, wfPointer);
            listItem->setText(workflow.name);
            ui->wfWorkflowsList->addItem(listItem);
            ui->wfWorkflowsList->setCurrentRow(ui->wfWorkflowsList->count()-1);
            dialogModified();
        }
        updateEditableStatus();
    }

    void editWorkflowDetails() {
        WorkflowListItem*workflowListItem = getSelectedWorkflowItem();
        if(workflowListItem==NULL) {
            return;
        }

        WfEditDetailsDialog editDetailsDialog(dialog);
        editDetailsDialog.setWindowTitle("Edit workflow");
        QVariantMapWithCheck result;
        WFWorkflow*workflow=workflowListItem->workflow;
        if(editDetailsDialog.editWorkflowDetails(result, workflow->name, workflow->description, workflow->author, workflow->website)) {
            workflow->name=result.getString("name");
            workflow->description=result.getString("description");
            workflow->author =result.getString("author");
            workflow->website=result.getString("website");
            workflowListItem->setText(workflow->name);
            dialogModified();
        }
        updateEditableStatus();
    }

    void removeSelectedWorkflow() {
        WorkflowListItem*workflowListItem=getSelectedWorkflowItem();
        if(workflowListItem==NULL) {
            return;
        }

        int row=ui->wfWorkflowsList->row(workflowListItem);
        ui->wfWorkflowsList->takeItem(row);
        configuration->workflows.takeAt(row);
        dialogModified();
        updateEditableStatus();
    }

    WFNodeItem* addNodeItem(WFNode*node, QPointF scenePos) {
        WFNodeItem*nodeItem=new WFNodeItem(node,[this](LinkItem*link){
            onLinkCreated(link);
        },[this](WFNodeItem*item){
            onItemMoved(item);
        });
        nodeItem->setPos(scenePos);
        nodeItem->node->x=scenePos.x();
        nodeItem->node->y=scenePos.y();
        scene->addItem(nodeItem);
        nodeItem->enableMoveCallback(true);
        node->handler->setNodeItem(nodeItem);
        return nodeItem;
    }

    void onLinkCreated(LinkItem*link){
        //update configuration with information about the link
        WFWorkflow*workflow=getSelectedWorkflow();
        QString sourceItemId=link->sourcePortItem->owner->node->id;
        int sourcePortIndex=link->sourcePortItem->getIndex();
        QString destItemId=link->destinationPortItem->owner->node->id;
        int destPortIndex=link->destinationPortItem->getIndex();
        QString sourceLinkString=QString::number(sourcePortIndex)+":"+sourceItemId;
        QString destLinkString=QString::number(destPortIndex)+":"+destItemId;
        workflow->links[sourceLinkString].append(destLinkString);
        dialogModified();
    }

    void onItemMoved(WFNodeItem*item){
        QPointF scenePos=item->scenePos();
        item->node->x=scenePos.x();
        item->node->y=scenePos.y();
        dialogModified();
    }

    bool isNodeIsSelected() {
        if(!scene->selectedItems().isEmpty()) {
            return dynamic_cast<WFNodeItem*>(scene->selectedItems().first())!=NULL;
        }
        return false;
    }

    WFNode*addNewWFNodeToWorkflow() {
        WorkflowListItem*workflow = getSelectedWorkflowItem();
        QList<WFNode>&nodes = workflow->workflow->nodes;
        nodes.append(WFNode());
        WFNode*newNode = &nodes.last();
        newNode->id = generateRandomString();
        return newNode;
    }

    WFNode*addWFNodeAndHandlerByType(QString type) {
        WFNode*newConfigNode=addNewWFNodeToWorkflow();
        newConfigNode->nodeType=type;
        WFNodeHandler*handler=createWFHandlerByType(type, newConfigNode, appGlobals);
        newConfigNode->handler=handler;
        return newConfigNode;
    }

    WFNodeItem* getWFNodeItemById(QString id) {
        QList<QGraphicsItem*>items = scene->items();

        for(int i=0; i<items.size(); i++) {
            WFNodeItem*nodeItem=dynamic_cast<WFNodeItem*>(items[i]);
            if(nodeItem!=NULL){
                if(nodeItem->node->id==id){
                    return nodeItem;
                }
            }
        }
        return NULL;
    }

    QList<WFNodeItem*>getSelectedNodeItems() {
        QList<WFNodeItem*>result;
        QList<QGraphicsItem*>selected=scene->selectedItems();
        foreach(QGraphicsItem*item, selected){
            WFNodeItem*nodeItem = dynamic_cast<WFNodeItem*>(item);
            if(nodeItem!=NULL) {
                result.append(nodeItem);
            }
        }
        return result;
    }

    void showSelectedNodeProperties(){
        if(!scene->selectedItems().isEmpty()) {
            QList<WFNodeItem*>selected=getSelectedNodeItems();
            WFNodeItem*nodeItem= selected.first();
            if(nodeItem->node->handler->showConfiguration(view)){
                nodeItem->updateAfterSettingsChange();
                dialogModified();
            }
        }
    }

    QSet<LinkItem*>getWFNodeItemLinks(WFNodeItem*item) {
        QSet<LinkItem*>result;
        foreach(OutputPortItem*portItem, item->outputs) {
            foreach(LinkItem*link, portItem->outputLinks) {
                result.insert(link);
            }
        }
        foreach(InputPortItem*portItem, item->inputs) {
            foreach(LinkItem*link, portItem->inputLinks) {
                result.insert(link);
            }
        }
        return result;
    }

    void deleteSelectedItem() {
        QList<QGraphicsItem *>selectedItems=scene->selectedItems();
        QSet<LinkItem*>linksToDelete;
        QSet<WFNodeItem*>nodesToDelete;
        foreach(QGraphicsItem*item,selectedItems) {
            LinkItem*link=dynamic_cast<LinkItem*>(item);
            if(link!=NULL) {
                linksToDelete.insert(link);
                continue;
            }

            WFNodeItem*wfNode=dynamic_cast<WFNodeItem*>(item);
            if(wfNode!=NULL) {
                nodesToDelete.insert(wfNode);
                linksToDelete.unite(getWFNodeItemLinks(wfNode));
                continue;
            }
        }

        foreach(LinkItem*link, linksToDelete) {
            deleteLink(link);
        }
        foreach(WFNodeItem*node, nodesToDelete) {
            deleteWFNode(node);
        }

        dialogModified();
    }

    void deleteWFNode(WFNodeItem*wfNode) {
        QList<WFNode>&workflowNodes = getSelectedWorkflow()->nodes;
        for(int i=0;i<workflowNodes.size();i++){
            WFNode*n=&workflowNodes[i];
            if(n==wfNode->node){
                workflowNodes.takeAt(i);
                break;
            }
        }
        delete wfNode;
    }

    void deleteLink( LinkItem*link) {
        link->sourcePortItem->removeLink(link);
        link->destinationPortItem->removeLink(link);
        QString sourceNodeId=link->sourcePortItem->owner->node->id;
        QString destNodeId=link->destinationPortItem->owner->node->id;
        WFWorkflow*workflow=getSelectedWorkflow();

        QString sourceLinkString = QString::number(link->sourcePortItem->getIndex()) + ":" + sourceNodeId;
        QString destLinkString = QString::number(link->destinationPortItem->getIndex()) + ":" + destNodeId;
        if(!workflow->links.contains(sourceLinkString)) {
            qDebug()<<"Cannot remove link "<<sourceLinkString<<" because it is not found in configuration";
        }
        QList<QString>&links=workflow->links[sourceLinkString];
        if(!links.contains(destLinkString)) {
            qDebug()<<"Cannot remove link "<<destLinkString<<" because it is not found in configuration";
        }
        links.removeOne(destLinkString);
        delete link;
    }

    void updateEditableStatus() {
        bool workflowSelected=getSelectedWorkflow()!=NULL;
        ui->wfScene->setEnabled(workflowSelected);
        ui->wfRemoveWorkflowButton->setEnabled(workflowSelected);
        if(!workflowSelected){
            scene->clear();
        }
    }

    WorkflowListItem*getSelectedWorkflowItem() {
        if(ui->wfWorkflowsList->selectedItems().size()>0) {
            WorkflowListItem*wfListItem=(WorkflowListItem*)ui->wfWorkflowsList->selectedItems().first();
            return wfListItem;
        }
        return NULL;
    }

    WFWorkflow*getSelectedWorkflow() {
        WorkflowListItem*workflowItem=getSelectedWorkflowItem();
        if(workflowItem==NULL) {
            return NULL;
        }
        return workflowItem->workflow;
    }

    void dialogModified() {
        dialog->modified(CONF_WORKFLOW);
    }
};
#endif // WFCONFIGURATIONTABHANDLER_H
