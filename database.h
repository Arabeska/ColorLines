#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QFile>
#include <QSqlRecord>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QDebug>

#include "structs.h"

#define VERSION_BASE    "1"
#define NAME_BASE       "ColorLinesDB"VERSION_BASE".db"

#define TABLE_INFO      "LastSessionInformation"
#define TABLE_POSITIONS "LastSessionPositions"

class DataBase : public QObject
{
    Q_OBJECT
public:
    DataBase(QObject *parent = 0);
    ~DataBase();

    void connectToDB();

    QJsonArray getLastProgressPosition();
    QJsonArray getLastInformation();

    void insertNewPosition(int id, const Cell cell);
    void insertNewBasicInfo(int id);
    void updateStatusPosition(int id, const Cell cell);
    void updateScore(int id, int score);
    void clearTablePositions();
    void clearTableInformation();
private:
    QSqlDatabase db;

    bool       querySQL(const QString query);
    QJsonArray querySQLJS(const QString query);

    bool openDataBase();
    bool restoreDataBase();
    void closeDataBase();
    bool createTables();
};

#endif // DATABASE_H
