#ifndef CONFIGURATION_H
#define CONFIGURATION_H
#include "includes.h"

class AbstractConfig {
  public:
    virtual QJsonObject serialize()=0;
    virtual void deserialize(QJsonObject json)=0;
};

class GeneralConfiguration:public AbstractConfig {
   public:
    QString toggleLauncherHotkey;
    bool isWhiteTrayIcon=false;
    int inputDialogPositionX;
    int inputDialogPositionY;

    virtual QJsonObject serialize() {
        QJsonObject json;
        json["whiteTrayIcon"]=isWhiteTrayIcon;
        json["toggleLauncherHotkey"]=toggleLauncherHotkey;
        json["inputDialogPositionX"]=inputDialogPositionX;
        json["inputDialogPositionY"]=inputDialogPositionY;
        return json;
    }
    virtual void deserialize(QJsonObject json) {
        isWhiteTrayIcon=json["whiteTrayIcon"].toBool(false);
        toggleLauncherHotkey= json["toggleLauncherHotkey"].toString();
        inputDialogPositionX= json["inputDialogPositionX"].toInt(-1);
        inputDialogPositionY= json["inputDialogPositionY"].toInt(-1);
    }
};
class BrowserConfiguration:public AbstractConfig {
  public:
    bool chromeEnabled=false;
    bool firefoxEnabled=false;
    virtual QJsonObject serialize() {
        QJsonObject json;
        json["chromeEnabled"]=chromeEnabled;
        json["firefoxEnabled"]=firefoxEnabled;
        return json;
    }
    virtual void deserialize(QJsonObject json) {
        chromeEnabled=json["chromeEnabled"].toBool(false);
        firefoxEnabled= json["firefoxEnabled"].toBool(false);
    }
};
class FolderConfig {
  public:
    QString directory;
    bool executableIncluded=false;
    int depth=1;
    QString allowedGlobs="";
    QString excludedGlobs="";

    QJsonObject serialize() {
        QJsonObject json;
        json["directory"]=directory;
        json["executableIncluded"]=executableIncluded;
        json["depth"]=depth;
        json["allowedGlobs"]=allowedGlobs;
        json["excludedGlobs"]=excludedGlobs;
        return json;
    }

    void deserialize(QJsonObject json) {
        directory=json["directory"].toString();
        allowedGlobs=json["allowedGlobs"].toString();
        excludedGlobs=json["excludedGlobs"].toString();
        depth=json["depth"].toInt();
        executableIncluded=json["executableIncluded"].toBool();
    }
};
class FileSystemConfiguration:public AbstractConfig {
  public:
    QList<FolderConfig>folderConfigs;
    virtual QJsonObject serialize() {
        QJsonObject json;
        QJsonArray jsonFolderConfigs;
        for(int i=0;i<folderConfigs.size();i++) {
            QJsonObject jsonFolderConfig=folderConfigs[i].serialize();
            jsonFolderConfigs.append(jsonFolderConfig);
        }
        json["folderConfigs"] = jsonFolderConfigs;
        return json;
    }

    virtual void deserialize(QJsonObject json) {
        folderConfigs.clear();
        if(json.contains("folderConfigs")) {
            QJsonArray jsonFolderConfigs=json["folderConfigs"].toArray();
            for(int i=0;i<jsonFolderConfigs.size();i++) {
                FolderConfig folderConfig;
                folderConfig.deserialize(jsonFolderConfigs[i].toObject());
                folderConfigs.append(folderConfig);
            }
        }
    }
};

class Snippet {
  public:
    QString name="";
    QString snippet="";
    Snippet() {}
    Snippet(QString name,QString snippet) {
        this->name=name;
        this->snippet=snippet;
    }
    QJsonObject serialize() {
        QJsonObject json;
        json["name"]=name;
        json["snippet"]=snippet;
        return json;
    }

    void deserialize(QJsonObject json) {
        name=json["name"].toString();
        snippet=json["snippet"].toString();
    }
};

class SnippetsCollection {
  public:
    QString name="";
    QString keyword="";
    QList<Snippet>snippets;
    SnippetsCollection() {}
    SnippetsCollection(QString name) {this->name=name;}
    QJsonObject serialize() {
        QJsonObject json;
        json["keyword"]=keyword;
        json["name"]=name;
        QJsonArray jsonSnippets;
        for(int i=0;i<snippets.size();i++) {
            QJsonObject jsonSnippet=snippets[i].serialize();
            jsonSnippets.append(jsonSnippet);
        }
        json["snippetCollections"] = jsonSnippets;
        return json;
    }

    void deserialize(QJsonObject json) {
        snippets.clear();
        name=json["name"].toString();
        keyword=json["keyword"].toString();
        if(json.contains("snippetCollections")) {
            QJsonArray jsonSnippets=json["snippetCollections"].toArray();
            for(int i=0;i<jsonSnippets.size();i++) {
                Snippet snippet;
                snippet.deserialize(jsonSnippets[i].toObject());
                snippets.append(snippet);
            }
        }
    }
};

class SnippetsConfiguration:public AbstractConfig {
  public:
    QString activationKeyword="";
    QList<SnippetsCollection>snippetsCollections;

    virtual QJsonObject serialize() {
        QJsonObject json;
        QJsonArray jsonSnipCollectionsConfigs;
        for(int i=0;i<snippetsCollections.size();i++) {
            QJsonObject jsonSnippetCollectionConfig=snippetsCollections[i].serialize();
            jsonSnipCollectionsConfigs.append(jsonSnippetCollectionConfig);
        }
        json["snippetsCollection"] = jsonSnipCollectionsConfigs;
        json["activationKeyword"] = activationKeyword;
        return json;
    }

    virtual void deserialize(QJsonObject json) {
        snippetsCollections.clear();
        activationKeyword=json["activationKeyword"].toString();
        if(json.contains("snippetsCollection")) {
            QJsonArray jsonSnippetsCollection=json["snippetsCollection"].toArray();
            for(int i=0;i<jsonSnippetsCollection.size();i++) {
                SnippetsCollection snippetCollectionConfig;
                snippetCollectionConfig.deserialize(jsonSnippetsCollection[i].toObject());
                snippetsCollections.append(snippetCollectionConfig);
            }
        }
    }
};

class WebSearch{
  public:
    QString url;
    QString keyword;
    QString title;
    WebSearch() {}
    WebSearch(QString url,QString keyword,QString title) {
        this->url=url;
        this->keyword=keyword;
        this->title=title;
    }

    QJsonObject serialize() {
        QJsonObject json;
        json["url"]=url;
        json["keyword"]=keyword;
        json["title"]=title;
        return json;
    }

    void deserialize(QJsonObject json) {
        url=json["url"].toString();
        keyword=json["keyword"].toString();
        title=json["title"].toString();
    }
};

class WebSearchConfiguration:public AbstractConfig {
  public:
    QList<WebSearch>searchCollection;

    virtual QJsonObject serialize() {
        QJsonObject json;
        QJsonArray jsonSearchCollection;
        for(int i=0;i<searchCollection.size();i++) {
            QJsonObject jsonSearchCollectionConfig=searchCollection[i].serialize();
            jsonSearchCollection.append(jsonSearchCollectionConfig);
        }
        json["searchCollection"] = jsonSearchCollection;
        return json;
    }

    virtual void deserialize(QJsonObject json) {
        searchCollection.clear();
        if(json.contains("searchCollection")) {
            QJsonArray jsonSearchCollection=json["searchCollection"].toArray();
            for(int i=0;i<jsonSearchCollection.size();i++) {
                WebSearch webSearch;
                webSearch.deserialize(jsonSearchCollection[i].toObject());
                searchCollection.append(webSearch);
            }
        }
    }
};

class WFNode{
  public:
    QString nodeType;
    QString id;
    int x;
    int y;
    QVariantMap props;

    WFNodeHandler*handler;

    QJsonObject serialize() {
        QJsonObject json;
        json["nodeType"]=nodeType;
        json["id"]=id;
        json["x"]=x;
        json["y"]=y;
        json["details"]=QJsonDocument::fromVariant(props).object();
        return json;
    }

    void deserialize(QJsonObject json) {
        nodeType=json["nodeType"].toString();
        id=json["id"].toString();
        x=json["x"].toInt();
        y=json["y"].toInt();
        props=json["details"].toObject().toVariantMap();
    }
};

class WFWorkflow {
  public:
    QString workflowId;
    QString name;
    QString description;
    QString versionId;
    QString author;
    QString website;
    QList<WFNode>nodes;
    virtual ~WFWorkflow() {}
    /**
     * The information is stored as:
     * key: outPortIndex:sourceNodeId
     * value: List of inputPortIndex:destNodeId
     */
    QMap<QString, QList<QString>>links;

    virtual QJsonObject serialize() {
        QJsonObject json;
        QJsonArray nodesArray;
        for(int i=0;i<nodes.size();i++) {
            WFNode*node=&nodes[i];
            nodesArray.append(node->serialize());
        }

        json["nodes"]=nodesArray;
        QJsonObject linksJson;
        foreach(QString nodeId,links.keys()) {
            QList<QString>linkedIds=links[nodeId];
            QJsonArray linksArray;
            for(int i=0;i<linkedIds.size();i++) {
                linksArray.append(linkedIds[i]);
            }
            linksJson[nodeId]=linksArray;
        }
        json["workflowId"]=workflowId;
        json["links"]=linksJson;
        json["name"]=name;
        json["description"]=description;
        json["versionId"]=versionId;
        json["author"]=author;
        json["website"]=website;
        return json;
    }

    virtual void deserialize(QJsonObject json) {
        nodes.clear();
        links.clear();
        this->workflowId=json["workflowId"].toString();
        this->name=json["name"].toString();
        this->description=json["description"].toString();
        this->versionId=json["versionId"].toString();
        this->author=json["author"].toString();
        this->website=json["website"].toString();
        QJsonArray nodesArray=json["nodes"].toArray();
        for(int i=0;i<nodesArray.size();i++) {
            QJsonObject nodeJson=nodesArray[i].toObject();
            WFNode node;
            node.deserialize(nodeJson);
            nodes.append(node);
        }
        QJsonObject linksJson=json["links"].toObject();
        foreach(QString sourceNodeId,linksJson.keys()) {
            QJsonArray linksArray=linksJson[sourceNodeId].toArray();
            QStringList resultList;
            for(int i=0;i<linksArray.size();i++) {
                resultList.append(linksArray[i].toString());
            }
            links[sourceNodeId]=resultList;
        }
    }
    WFNode*findNodeById(QString nodeId){
        for(int i=0;i<nodes.size();i++) {
            WFNode*node=&nodes[i];
            if(node->id==nodeId) {
                return node;
            }
        }

        return NULL;
    }
};

class WorkflowConfiguration:public AbstractConfig {
  public:
    QList<WFWorkflow>workflows;

    virtual QJsonObject serialize() override {
        QJsonArray workflowsArray;
        for(int i=0;i<workflows.size();i++) {
            WFWorkflow*workflow=&workflows[i];
            workflowsArray.append(workflow->serialize());
        }

        QJsonObject json;
        json["workflows"]=workflowsArray;
        return json;
    }

    virtual void deserialize(QJsonObject json) override {
        workflows.clear();
        QJsonArray workflowsJson=json["workflows"].toArray();
        for(int i=0;i<workflowsJson.size();i++) {
            QJsonObject workflowJson=workflowsJson[i].toObject();
            WFWorkflow workflow;
            workflow.deserialize(workflowJson);
            workflows.append(workflow);
        }
    }
    WFWorkflow*findWorkflow(QString workflowId){
        for(int i=0;i<workflows.size();i++){
            WFWorkflow*workflow=&workflows[i];
            if(workflow->workflowId==workflowId) {
                return workflow;
            }
        }
        return NULL;
    }
};

class Configuration {
  public:
    GeneralConfiguration generalConfiguration;
    FileSystemConfiguration fileSystemConfiguration;
    BrowserConfiguration browserConfiguration;
    SnippetsConfiguration snippersConfiguration;
    WebSearchConfiguration webSearchConfiguration;
    WorkflowConfiguration workflowConfiguration;
    QMap<QString,AbstractConfig*>configurations;

    Configuration() {
        configurations.insert(CONF_GENERAL, &generalConfiguration);
        configurations.insert(CONF_FILE_SYSTEM, &fileSystemConfiguration);
        configurations.insert(CONF_SNIPPETS, &snippersConfiguration);
        configurations.insert(CONF_BROWSER, &browserConfiguration);
        configurations.insert(CONF_WEB_SEARCH, &webSearchConfiguration);
        configurations.insert(CONF_WORKFLOW, &workflowConfiguration);
    }

    Configuration* load(AppGlobals*);

    Configuration* saveAll(QStringList changedConfigurations);
};
#endif // CONFIGURATION_H
