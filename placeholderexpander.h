#ifndef PLACEHOLDEREXPANDER_H
#define PLACEHOLDEREXPANDER_H

#include <QString>
#include "includes.h"
#include <QClipboard>

class PlaceholderExpander {
    AppGlobals*appGlobals;
    QString pattern="\\$\\{(?<key>.+?)(?<arg>:.+?)?\\}";
  public:
    PlaceholderExpander(AppGlobals*appGlobals) {
        this->appGlobals=appGlobals;
    }

    bool containsPlaceholder(QString line){
        return line.contains(QRegularExpression(pattern));
    }

    QString expand(QString snippet, WFExecutionContext*context, int level=0) {
        if(level>5) {
            return snippet;
        }

        return regexReplace(pattern, snippet,[this, level, context](QString matched,QRegularExpressionMatch*match) {
            Q_UNUSED(matched)
            QString placeholderKey = match->captured("key");
            QString placeholderArg = match->captured("arg");

            return processPlaceholder(placeholderKey, placeholderArg, level, context);
        });
    }
  private:
    QString processPlaceholder_date(QString arg) {
        QDate date=QDate::currentDate();
        if(arg=="") {
            return date.toString(Qt::SystemLocaleDate);
        }else if(arg=="long") {
            return date.toString(Qt::SystemLocaleLongDate);
        } else {
            return QString("Unknown Placeholder date argument '%1'").arg(arg);
        }
    }

    QString processPlaceholder_time(QString arg) {
        QTime timeObj=QTime::currentTime();
        if(arg=="") {
            return timeObj.toString(Qt::SystemLocaleDate);
        } else if(arg=="long") {
            return timeObj.toString(Qt::SystemLocaleLongDate);
        } else {
            return QString("Unknown Placeholder time argument '%1'").arg(arg);
        }
    }

    QString processPlaceholder_snippet(QString arg, int level, WFExecutionContext*context) {
        if(arg==NULL) {
            return "'snippet' placeholder expects snippet path. ${snippet:SNIPPET_COLLECTION/SNIPPET_NAME}";
        }

        Snippet*snippet=getSnippet(arg);
        return expand(snippet->snippet, context, level+1);
    }

    QString processPlaceholder_requestInput(QString arg) {
        QString description = arg.isEmpty() ? "Enter placeholder value" : arg;
        TypeTextInputMatcher*matcher=new TypeTextInputMatcher(appGlobals,description);
        QVariant result;
        appGlobals->inputDialog->select(matcher,[&result](InputItem*item) {
            result=item->text;
        });
        while(result.isNull()) {
            QCoreApplication::processEvents();
            QThread::msleep(2);
        }
        return result.toString();
    }

    QString processPlaceholder_vars(QString arg, WFExecutionContext*context) {
        QString varName=arg;
        if(context==NULL) {
            return "Variables not allowed there";
        }
        if(!context->variables.contains(varName)) {
            return QString("Variable [%1] not found").arg(varName);
        }

        return context->variables[varName].toString();
    }

    QString processPlaceholder_topLevelVar(QString key, QString arg, WFExecutionContext*context) {
        Q_UNUSED(arg);
        QString varName=key;
        if(context==NULL) {
            return "Variables not allowed there";
        }
        if(!context->topLevelVariables.contains(varName)) {
            return QString("Variable [%1] not found").arg(varName);
        }

        return context->topLevelVariables[varName].toString();
    }

    QString processPlaceholder(QString key, QString arg, int level, WFExecutionContext*context) {
        if (arg.length() != 0 && arg.startsWith(":")) {
            arg = arg.mid(1);
        }
        if(key=="user_name") {
            return getUserName();
        } else if(key=="os") {
            return QSysInfo::kernelType();
        } else if(key=="os_version") {
            return QSysInfo::kernelVersion();
        } else if(key=="hostname") {
            return QSysInfo::machineHostName();
        } else if(key=="cpu") {
            return QSysInfo::currentCpuArchitecture();
        } else if(key=="user_home") {
            return QDir::home().absolutePath();
        } else if(key=="clipboard") {
            QClipboard *clipboard = QGuiApplication::clipboard();
            return clipboard->text();
        }else if(key=="request_input") {
            return processPlaceholder_requestInput(arg);
        } else if(key=="date") {
             return processPlaceholder_date(arg);
        }else if(key=="time") {
            return processPlaceholder_time(arg);
        } else if(key=="snippet") {
            return processPlaceholder_snippet(arg, level, context);
        } else if(key=="input") {
            return processPlaceholder_vars("input", context);
        } else if(key=="var") {
            return processPlaceholder_vars(arg, context);
        } else if(context->topLevelVariables.contains(key)){
            return processPlaceholder_topLevelVar(key, arg, context);
        } else {
            return QString("Unknown placeholder [%1]").arg(key);
        }
    }

    Snippet*getSnippet(QString path) {
        QStringList pathParts= path.split("/");
        if(pathParts.size()!=2) {
            return NULL;
        }
        QString snippetsCollectionName=pathParts[0];
        QString snippetName=pathParts[1];

        SnippetsConfiguration&conf=appGlobals->configuration->snippersConfiguration;
        SnippetsCollection*snippetCollection=NULL;

        QRegExp collectionRegex(snippetsCollectionName,Qt::CaseInsensitive,QRegExp::Wildcard);
        for(int i=0;i<conf.snippetsCollections.size();i++) {
            SnippetsCollection*collection=&conf.snippetsCollections[i];
            if(collectionRegex.exactMatch(collection->name)) {
                snippetCollection=collection;
                break;
            }
        }
        if(snippetCollection==NULL) {
            return NULL;
        }
        QRegExp snippetNameRegex(snippetName,Qt::CaseInsensitive,QRegExp::Wildcard);
        for(int i=0;i<snippetCollection->snippets.size();i++) {
            Snippet*snippet=&snippetCollection->snippets[i];
            if(snippetNameRegex.exactMatch(snippet->name)) {
                return snippet;
            }
        }
        return NULL;
    }
};
#endif // PLACEHOLDEREXPANDER_H
