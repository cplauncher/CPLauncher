#include "includes.h"

Configuration* Configuration::load(AppGlobals*appGlobals) {
   QStringList configNames;
   configNames<<CONF_GENERAL<<CONF_BROWSER<<CONF_FILE_SYSTEM<<CONF_SNIPPETS<<CONF_WEB_SEARCH<<CONF_WORKFLOW;
   foreach(QString configName, configNames) {
       AbstractConfig*config=configurations[configName];
       if(config==NULL) {
           qDebug()<<"[ERROR] Loading configuration "<<configName<<" is not implemented";
           continue;
       }

       QFile settingsFile(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"/"+configName+".json");
       if(!settingsFile.exists()) {
           appGlobals->plugins[configName]->initDefaultConfiguration(config);
           continue;
       }

       settingsFile.open(QIODevice::ReadOnly);
       QJsonDocument jsonDocument=QJsonDocument::fromJson(settingsFile.readAll());
       QJsonObject jsonObject = jsonDocument.object();
       if(config!=NULL) {
           config->deserialize(jsonObject);
       }
   }
   return this;
}

Configuration* Configuration::saveAll(QStringList changedConfigurations) {
    foreach(QString configName,changedConfigurations) {
        if(!configurations.contains(configName)) {
            qDebug()<<"[ERROR] Saving configuration "<<configName<<" is not implemented";
            continue;
        }

        QJsonObject json=configurations[configName]->serialize();
        QFileInfo settingsFileInfo(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/" + configName + ".json");
        QDir dir = settingsFileInfo.absoluteDir();
        if (!dir.exists()) {
            dir.root().mkdir(dir.absolutePath());
        }

        QFile settingsFile(settingsFileInfo.absoluteFilePath());
        settingsFile.open(QIODevice::WriteOnly);

        QJsonDocument jsonDoc(json);
        QByteArray jsonResult = jsonDoc.toJson(QJsonDocument::Indented);
        settingsFile.write(jsonResult);
        settingsFile.close();
    }
    return this;
}

QJsonArray serializeVariables(QList<Variable>&variables){
    QJsonArray jsonArray;
    for(int i=0;i<variables.count();i++){
        jsonArray.append(variables[i].serialize());
    }
    return jsonArray;
}

QList<Variable>deserializeVariables(QJsonArray jsonArray){
    QList<Variable>result;
    for(int i=0;i<jsonArray.count();i++){
        Variable var;
        var.deserialize(jsonArray[i].toObject());
        result.append(var);
    }
    return result;
}
