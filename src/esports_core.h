#ifndef ESPORTS_CORE_H
#define ESPORTS_CORE_H

#include <QVector>
#include <QString>
#include <QStringList>
#include <string>

struct Player {
    std::string idNumber;
    std::string name;
    std::string lastName;
    std::string country;
    std::string birthDate;
    std::string role;
};

struct LoLUser {
    int userId = 0;
    std::string playerIdNumber;
    std::string riotId;
    std::string tagline;
    std::string puuid;
    std::string soloqRank = "Unranked";
    double winrate = 0.0;
};

struct CsvTableData {
    QStringList headers;
    QVector<QStringList> rows;
};

class EsportsCore {
public:
    bool loadPlayers(const QString& filePath, QString* errorMessage = nullptr);
    bool loadLolUsers(const QString& filePath, QString* errorMessage = nullptr);
    bool addPlayer(const Player& player, const QString& filePath,
                   QString* errorMessage = nullptr);
    bool updatePlayer(int index, const Player& player, const QString& filePath,
                      QString* errorMessage = nullptr);
    bool removePlayer(int index, const QString& filePath,
                      QString* errorMessage = nullptr);
    bool updateLolUser(int index, const LoLUser& user, const QString& filePath,
                       QString* errorMessage = nullptr);
    bool loadCsvTable(const QString& filePath, const QStringList& headers,
                      CsvTableData* table, QString* errorMessage = nullptr) const;
    bool saveCsvTable(const QString& filePath, const CsvTableData& table,
                      QString* errorMessage = nullptr) const;
    const QVector<Player>& getPlayers() const;
    const QVector<LoLUser>& getLolUsers() const;

private:
    QVector<Player> players;
    QVector<LoLUser> lolUsers;
};

#endif // ESPORTS_CORE_H
