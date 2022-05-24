#include <QDesktopServices>
#include "plugins.h"


class FileItemToMatch {
  public:
    QString file;
    QString fileNameLowerCase;
    QString fullPath;
};

class FileSystemMatcher:public AbstractMatcher {
  private:
    QList<FileItemToMatch>fileItems;
    AppGlobals*appGlobals;
  public:
    FileSystemMatcher(AppGlobals*appGlobals) {
        this->appGlobals=appGlobals;
        matchingTags<<"default"<<"filesystem";
    }

    void setFilesList(QList<FileItemToMatch>&fileItems) {
        this->fileItems=fileItems;
    }

    void match(MatchContext*matchContext) override {
        if (matchContext->stringToSearch.isEmpty())
            return;
        int count = 0;
        for (int i=0; i<fileItems.size();i++) {
            FileItemToMatch&fileItem = fileItems[i];
            if (defaultMatch(matchContext, fileItem.fileNameLowerCase, false)) {
                matchContext->collectedData.append(createInputItem(fileItem));
                count++;
                int maxOutputFilesCount = 20;
                if (count > maxOutputFilesCount) {
                    break;
                }
            }
        }
    }

    InputItem createInputItem(FileItemToMatch&fileItem) {
        QFile file(fileItem.fullPath);
        QString icon = "fileicon://" + fileItem.fullPath;
        InputItem item;
        item.id="fileItem_" + fileItem.fullPath;
        item.icon=icon;
        item.text=fileItem.file;
        item.keyword=fileItem.fileNameLowerCase;
        item.smallDescription=fileItem.fullPath;
        item.executable=true;
        item.ownerMatcher=this;
        item.userObject=fileItem.fullPath;
        return item;
    }

    void execute(InputItem*inputItem) override {
        QString fullFilePath = inputItem->userObject.toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(fullFilePath));
    }
};

void FileSystemPlugin::initDefaultConfiguration(AbstractConfig*) {
}

void FileSystemPlugin::init(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
    matcher=new FileSystemMatcher(appGlobals);
    refresh();
}

QList<AbstractMatcher*> FileSystemPlugin::getMatchers() {
    return QList<AbstractMatcher*>()<<matcher;
}

void addFile(QFileInfo&fileInfo, QList<FileItemToMatch>&result, QSet<QString>&addedPathSet) {
    QString fileNameLowerCase=fileInfo.fileName().toLower();
    if(addedPathSet.contains(fileNameLowerCase)) {
        return;
    }

    addedPathSet.insert(fileNameLowerCase);
    FileItemToMatch fim;
    fim.file=fileInfo.fileName();
    fim.fileNameLowerCase=fileNameLowerCase;
    fim.fullPath=fileInfo.absoluteFilePath();
    result.append(fim);
}

bool matchWithListOfRegex(QList<QRegExp>&regexpList, QString string) {
    for(int i=0;i<regexpList.size();i++) {
        QRegExp&regexp=regexpList[i];
        if(regexp.exactMatch(string)) {
            return true;
        }
    }
    return false;
}

void walkFolder(QSet<QString>&addedPathSet, QString currentPath, FolderConfig&folderConfig,QList<QRegExp>&allowGlob, QList<QRegExp>&excludeGlob, QList<FileItemToMatch>&result, int level) {
    if(level>folderConfig.depth) {
        return;
    }

    QFileInfo current(currentPath);
    if(current.isDir()) {
        //MAC .app folders
        if(folderConfig.executableIncluded&&isExecutable(currentPath)) {
            addFile(current, result,addedPathSet);
            return;
        }
        QDir currentDir=QDir(current.absoluteFilePath());
        QFileInfoList children = currentDir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries|QDir::System);
        for(int i=0;i<children.size();i++) {
            QFileInfo&fi=children[i];
            walkFolder(addedPathSet, fi.absoluteFilePath(),folderConfig, allowGlob, excludeGlob, result, level+1);
        }
    } else {
        QString fileName=current.fileName();
        if(matchWithListOfRegex(excludeGlob,fileName)) {
            return;
        }
        bool include=false;
        if(matchWithListOfRegex(allowGlob,fileName)) {
            include=true;
        } else if(folderConfig.executableIncluded && current.isExecutable()) {
            include=true;
        }

        if(include) {
            addFile(current, result,addedPathSet);
        }
    }
}

QList<QRegExp>prepareGlobList(QString string) {
    QStringList lines=string.split(QRegExp("[\r\n]"), Qt::SkipEmptyParts);
    QList<QRegExp>result;
    for(int i=0;i<lines.size();i++) {
        QString line=lines[i].trimmed();
        if(!line.isEmpty()) {
            QRegExp regex(line, Qt::CaseInsensitive,QRegExp::Wildcard);
            result.append(regex);
        }
    }
    return result;
}

void FileSystemPlugin::refresh() {
    QList<FileItemToMatch>result;
    FileSystemConfiguration&conf=appGlobals->configuration->fileSystemConfiguration;
    QSet<QString>addedFiles;
    for(int i=0; i<conf.folderConfigs.size(); i++) {
        FolderConfig&folderConfig=conf.folderConfigs[i];
        QList<QRegExp>allowedGlobs=prepareGlobList(folderConfig.allowedGlobs);
        QList<QRegExp>excludedGlobs=prepareGlobList(folderConfig.excludedGlobs);
        walkFolder(addedFiles,folderConfig.directory, folderConfig, allowedGlobs, excludedGlobs, result, 0);
    }
    matcher->setFilesList(result);
}
