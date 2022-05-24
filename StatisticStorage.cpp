#include <QDateTime>
#include <QStandardPaths>
#include <QTextStream>
#include "statisticstorage.h"

StatisticStorage::StatisticStorage() {
    QString historyFilePath=QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"/history.dat";
    historyFile = new QFile(historyFilePath);
    loadHistoryFromFile();
    historyStream = openHistoryFile(true);
}

void StatisticStorage::setMaxEntriesInFile(int maxEntriesInFile) {
    this->maxEntriesInFile = maxEntriesInFile;
}

void StatisticStorage::setMaxEntriesInMap(int maxEntriesInMap) {
    this->maxEntriesInMap = maxEntriesInMap;
}

QTextStream* StatisticStorage::openHistoryFile(bool append) {
    historyFile->open(QIODevice::WriteOnly | QIODevice::Text|(append?QIODevice::Append: QIODevice::Truncate));
    QTextStream*outFile=new QTextStream(historyFile);
    return outFile;
}

void StatisticStorage::loadHistoryFromFile() {
    if (!historyFile->exists()) {
        return;
    }
    entriesInHistoryFile = 0;
    if (historyFile->open(QIODevice::ReadOnly)) {
       QTextStream in(historyFile);
       while (!in.atEnd())
       {
          QString line = in.readLine().trimmed();
          if (line.isEmpty()) {
              continue;
          }
          QStringList parts=line.split(SEPARATOR);
          if(parts.size()!=3) {
              continue;
          }
          QString keyword = parts[0];
          QString id = parts[1];
          QString timeString = parts[2];
          if (!keywordToId.contains(keyword)) {
              keywordToId[keyword]=IdWithTime(id, timeString.toULongLong());
          }
          entriesInHistoryFile++;
       }
       historyFile->close();
    }
}

void StatisticStorage::addUsage(QString id, QString keyword) {
    keyword = keyword.toLower();
    QString tid=keywordToId.contains(keyword)?keywordToId[keyword].id:BAD_ID;

    if (id!=tid) {
        quint64 currentTimestamp= QDateTime::currentMSecsSinceEpoch();
        IdWithTime idWithTime(id, currentTimestamp);
        keywordToId[keyword] = idWithTime;
        if (keywordToId.size() > maxEntriesInMap) {
            cleanupKeywordMap();
        }
        saveToFile(idWithTime, keyword);
    }
}

void StatisticStorage::cleanupKeywordMap() {
    QStringList keywordList(keywordToId.keys());
    std::sort(keywordList.begin(), keywordList.end(), [this](const QString &keyword1, const QString&keyword2) {
        quint64 time1 = keywordToId[keyword1].timestamp;
        quint64 time2 = keywordToId[keyword2].timestamp;
        return time2-time1;
    });

    int newSize = (int)(keywordToId.size() / MAP_CLEANUP_RATIO);
    int removeItemsCount = keywordList.size() - newSize;
    for (int i = 0; i < removeItemsCount; i++) {
        QString keywordToDelete = keywordList.takeAt(keywordList.size() - 1);
        keywordToId.remove(keywordToDelete);
    }
}

void StatisticStorage::saveToFile(IdWithTime idWithTime, QString keyword) {
    *historyStream<<(idAndKeyword2String(idWithTime, keyword))<<"\n";
    historyStream->flush();
    entriesInHistoryFile++;
    if (entriesInHistoryFile > maxEntriesInFile) {
        serializeKeyWordMap();
    }
}

QString StatisticStorage::idAndKeyword2String(IdWithTime idWithTime, QString keyword) {
    return escape(keyword) + SEPARATOR + escape(idWithTime.id) + SEPARATOR + QString::number(idWithTime.timestamp);
}

void StatisticStorage::serializeKeyWordMap() {
    historyStream->flush();
    delete historyStream;

    historyFile->close();
    historyStream = openHistoryFile(false);
    foreach (const QString&keyword, keywordToId.keys()) {
        IdWithTime idWithTime = keywordToId[keyword];
        *historyStream<<idAndKeyword2String(idWithTime, keyword);
    }
    entriesInHistoryFile = keywordToId.size();
    historyStream->flush();
}

QString StatisticStorage::escape(QString str) {
    return str.replace("`", "\\`").replace("\n","").replace("\r","");
}

QString StatisticStorage::getIdByKeyword(QString keyword) {
    keyword=escape(keyword);
    keyword = keyword.toLower();
    if(keywordToId.contains(keyword)) {
        return keywordToId[keyword].id;
    }
    return BAD_ID;
}
