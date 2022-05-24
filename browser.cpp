#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtSql>
#include "utils.h"
#include "browser.h"


QList<QString> CHROME_SEARCH_FOLDERS = QList<QString>()
        <<"C:/Users/${user_name}/AppData/Local/Google/Chrome/User Data/Default"
        <<"${home_dir}/Library/Application Support/Google/Chrome/Default"
        <<"${home_dir}/.config/google-chrome/default";
QList<QString> FIREFOX_SEARCH_FOLDER= QList<QString>()
        <<"${home_dir}/Library/Application Support/Firefox/Profiles"
        <<"C:/Users/${user_name}/AppData/Roaming/Mozilla/Firefox/Profiles";

QString expandPlaceholders(QString stri) {
    return stri.replace("${user_name}", getUserName()).replace("${home_dir}", getHomeDir());
}

QString BrowserBookmarksExtractor::getChromeConfigFile() {
    if(!chromeConfigFile.isNull()) {
        return chromeConfigFile;
    }

    for(int i=0; i<CHROME_SEARCH_FOLDERS.size(); i++) {
        QString chromeConfigFolder=CHROME_SEARCH_FOLDERS[i];
        QString fullPath=expandPlaceholders(QString("%1/%2").arg(chromeConfigFolder, "Bookmarks"));
        QFileInfo configFile(fullPath);
        if(configFile.exists() && configFile.isFile()) {
            chromeConfigFile=configFile.absoluteFilePath();
            return chromeConfigFile;
        }
    }

    return QString();
}

QString BrowserBookmarksExtractor::getFirefoxConfigFile() {
    if(!firefoxConfigFile.isNull()) {
        return firefoxConfigFile;
    }
    for(int i=0; i<FIREFOX_SEARCH_FOLDER.size(); i++) {
        QDir firefoxConfigFolder=QDir(expandPlaceholders(FIREFOX_SEARCH_FOLDER[i]));
        if(!firefoxConfigFolder.exists()) {
            continue;
        }
        QStringList profiles = firefoxConfigFolder.entryList(QDir::NoDotAndDotDot|QDir::Dirs);
        foreach(QString profile, profiles) {
            QString path=firefoxConfigFolder.absolutePath()+"/"+profile+"/places.sqlite";
            QFile placesFile(path);
            if(placesFile.exists()) {
                firefoxConfigFile=path;
                return path;
            }
        }
    }
    return QString();
}

bool BrowserBookmarksExtractor::isChromeSupported() {
    return !getChromeConfigFile().isNull();
}

bool BrowserBookmarksExtractor::isFirefoxSupported() {
    return !getFirefoxConfigFile().isNull();
}

bool isBookmarkJson(QJsonObject jo) {
    return jo.contains("type") && jo["type"].toString("")=="url"
            && jo.contains("name") && jo.contains("url") && (jo.contains("guid")||jo.contains("id"));
}

void searchChromeUrlBookmarks(QJsonValue json, QList<Bookmark>&bookmarks) {
    if (json.isArray()) {
        QJsonArray ja = json.toArray();
        for (int i = 0; i < ja.size(); i++) {
            searchChromeUrlBookmarks(ja[i], bookmarks);
        }
    }
    if (json.isObject()) {
        QJsonObject jo = json.toObject();
        if (isBookmarkJson(jo)) {
            QString url = jo["url"].toString();
            QString name = jo["name"].toString();
            QString guid;
            if(jo.contains("guid")) {
                guid= jo["guid"].toString();
            }else{
                guid= jo["id"].toString();
            }
            bookmarks.append(Bookmark(url, name, guid, "chrome"));
        } else {
            foreach (QString key , jo.keys()) {
                searchChromeUrlBookmarks(jo[key], bookmarks);
            }
        }
    }
}

QList<Bookmark>BrowserBookmarksExtractor::extractChromeBookmarks() {
    if(!isChromeSupported()) {
        return QList<Bookmark>();
    }

    QList<Bookmark>result;
    QString chromeBookmarkFilePath=getChromeConfigFile();
    QFile chromeBookmarkFile(chromeBookmarkFilePath);
    chromeBookmarkFile.open(QIODevice::ReadOnly);
    QJsonDocument jsonDocument=QJsonDocument::fromJson(chromeBookmarkFile.readAll());
    QJsonObject jsonObject = jsonDocument.object();
    searchChromeUrlBookmarks(jsonObject, result);
    return result;
}

QList<Bookmark>BrowserBookmarksExtractor::extractFirefoxBookmarks() {
    if(!isFirefoxSupported()) {
        return QList<Bookmark>();
    }

    QSqlDatabase sdb = QSqlDatabase::addDatabase("QSQLITE");
    sdb.setDatabaseName(getFirefoxConfigFile());
    QList<Bookmark>result;
    if (sdb.open()) {
        QSqlQuery query("select b.id, b.title, p.url from moz_bookmarks b, moz_places p where b.type=1 and b.fk=p.id");
        while (query.next()) {
            QString id = query.value("id").toString();
            QString title = query.value("title").toString();
            QString url = query.value("url").toString();
            Bookmark bookmark(url,title,id,"ff");
            result.append(bookmark);
        }
    }else{
        qDebug()<<"Error while loading firefox bookmarks " << sdb.lastError().text();
    }
    return result;
}
