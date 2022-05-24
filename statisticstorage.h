#ifndef STATISTICSTORAGE_H
#define STATISTICSTORAGE_H
#include <QMap>
#include <QFile>

class IdWithTime {
public:
    QString id;
    quint64 timestamp;
    IdWithTime() {}
    IdWithTime(QString&id, quint64 timestamp) {
        this->id = id;
        this->timestamp = timestamp;
    }
};
class QTextStream;
class StatisticStorage {
    QString BAD_ID = " ~ ";
    QString SEPARATOR = "|\f-\f|";
    QMap<QString, IdWithTime> keywordToId;
    QFile*historyFile = NULL;
    QTextStream*historyStream = NULL;
    float MAP_CLEANUP_RATIO = 1.5f;
    int maxEntriesInMap = 1000;
    int maxEntriesInFile = maxEntriesInMap * 2;
    int entriesInHistoryFile = 0;
    QTextStream* openHistoryFile(bool append);
    void loadHistoryFromFile();
    void cleanupKeywordMap();
    void saveToFile(IdWithTime idWithTime, QString keyword);
    QString idAndKeyword2String(IdWithTime idWithTime, QString keyword);
    void serializeKeyWordMap();
    QString escape(QString str);
public:
    StatisticStorage();

    void setMaxEntriesInFile(int maxEntriesInFile);
    void setMaxEntriesInMap(int maxEntriesInMap);
    void addUsage(QString id, QString keyword);
    QString getIdByKeyword(QString keyword);
};

#endif // STATISTICSTORAGE_H
