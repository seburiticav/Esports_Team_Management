#include "esports_core.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QTextStream>

namespace {
bool parseCsvRow(const QString& line, QStringList& fields)
{
    fields.clear();
    QString field;
    bool quoted = false;
    bool closedQuote = false;

    for (qsizetype i = 0; i < line.size(); ++i) {
        const QChar ch = line[i];
        if (quoted) {
            if (ch == QLatin1Char('"')) {
                if (i + 1 < line.size() && line[i + 1] == QLatin1Char('"')) {
                    field += QLatin1Char('"');
                    ++i;
                } else {
                    quoted = false;
                    closedQuote = true;
                }
            } else {
                field += ch;
            }
        } else if (ch == QLatin1Char(',') && !closedQuote) {
            fields.append(field);
            field.clear();
        } else if (ch == QLatin1Char('"') && field.isEmpty() && !closedQuote) {
            quoted = true;
        } else if (ch == QLatin1Char(',') && closedQuote) {
            fields.append(field);
            field.clear();
            closedQuote = false;
        } else if (closedQuote && ch != QLatin1Char(' ') && ch != QLatin1Char('\t')) {
            return false;
        } else {
            field += ch;
        }
    }

    if (quoted) return false;
    fields.append(field);
    return true;
}

QString escapeCsvField(const QString& field)
{
    QString escaped = field;
    escaped.replace(QStringLiteral("\""), QStringLiteral("\"\""));
    if (field.contains(QLatin1Char(',')) || field.contains(QLatin1Char('"'))
        || field.contains(QLatin1Char('\n')) || field.contains(QLatin1Char('\r'))) {
        escaped = QLatin1Char('"') + escaped + QLatin1Char('"');
    }
    return escaped;
}

bool validatePlayer(const Player& player, const QVector<Player>& existingPlayers,
                    int ignoredIndex, QString* errorMessage)
{
    const QStringList values = {
        QString::fromStdString(player.idNumber),
        QString::fromStdString(player.name),
        QString::fromStdString(player.lastName),
        QString::fromStdString(player.country),
        QString::fromStdString(player.birthDate)
    };
    for (const QString& value : values) {
        if (value.trimmed().isEmpty()) {
            if (errorMessage) *errorMessage = QStringLiteral("Todos los campos son obligatorios.");
            return false;
        }
    }

    const QString role = QString::fromStdString(player.role).trimmed();
    static const QStringList validRoles = {"TOP", "JG", "MID", "ADC", "SUP"};
    if (!role.isEmpty() && !validRoles.contains(role)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("El rol debe ser TOP, JG, MID, ADC o SUP.");
        }
        return false;
    }

    const QString idNumber = values[0].trimmed();
    for (int i = 0; i < existingPlayers.size(); ++i) {
        if (i != ignoredIndex
            && QString::fromStdString(existingPlayers[i].idNumber).trimmed() == idNumber) {
            if (errorMessage) *errorMessage = QStringLiteral("Ya existe un jugador con ese ID.");
            return false;
        }
    }
    return true;
}

bool savePlayers(const QVector<Player>& players, const QString& filePath,
                 QString* errorMessage)
{
    QString contents = QStringLiteral("id_number,name,last_name,country,birth_date,role\n");
    for (const Player& item : players) {
        const QStringList fields = {
            QString::fromStdString(item.idNumber),
            QString::fromStdString(item.name),
            QString::fromStdString(item.lastName),
            QString::fromStdString(item.country),
            QString::fromStdString(item.birthDate),
            QString::fromStdString(item.role)
        };
        QStringList escapedFields;
        for (const QString& field : fields) escapedFields.append(escapeCsvField(field));
        contents += escapedFields.join(QLatin1Char(',')) + QLatin1Char('\n');
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    const QByteArray data = contents.toUtf8();
    if (file.write(data) != data.size()) {
        if (errorMessage) *errorMessage = file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }
    return true;
}

bool saveLolUsers(const QVector<LoLUser>& users, const QString& filePath,
                  QString* errorMessage)
{
    QString contents = QStringLiteral(
        "user_id,player_id_number,riot_id,tagline,puuid,soloq_rank,winrate\n");
    for (const LoLUser& user : users) {
        const QStringList fields = {
            QString::number(user.userId),
            QString::fromStdString(user.playerIdNumber),
            QString::fromStdString(user.riotId),
            QString::fromStdString(user.tagline),
            QString::fromStdString(user.puuid),
            QString::fromStdString(user.soloqRank),
            QString::number(user.winrate, 'f', 2)
        };
        QStringList escapedFields;
        for (const QString& field : fields) escapedFields.append(escapeCsvField(field));
        contents += escapedFields.join(QLatin1Char(',')) + QLatin1Char('\n');
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    const QByteArray data = contents.toUtf8();
    if (file.write(data) != data.size()) {
        if (errorMessage) *errorMessage = file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }
    return true;
}
}

bool EsportsCore::loadPlayers(const QString& filePath, QString* errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    QTextStream input(&file);
    QString header = input.readLine();
    if (header.startsWith(QChar(0xFEFF))) header.remove(0, 1);
    const bool hasRoleColumn =
        header == QStringLiteral("id_number,name,last_name,country,birth_date,role");
    if (!hasRoleColumn
        && header != QStringLiteral("id_number,name,last_name,country,birth_date")) {
        if (errorMessage) *errorMessage = QStringLiteral("El encabezado del CSV no es valido.");
        return false;
    }

    QVector<Player> loadedPlayers;
    int lineNumber = 1;
    while (!input.atEnd()) {
        const QString line = input.readLine();
        ++lineNumber;
        if (line.trimmed().isEmpty()) continue;

        QStringList fields;
        if (!parseCsvRow(line, fields)
            || fields.size() != (hasRoleColumn ? 6 : 5)) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Formato CSV invalido en la fila %1.")
                                    .arg(lineNumber);
            }
            return false;
        }

        loadedPlayers.append(Player{
            fields[0].toStdString(),
            fields[1].toStdString(),
            fields[2].toStdString(),
            fields[3].toStdString(),
            fields[4].toStdString(),
            hasRoleColumn ? fields[5].toStdString() : std::string{}
        });
    }

    players = loadedPlayers;
    return true;
}

bool EsportsCore::loadLolUsers(const QString& filePath, QString* errorMessage)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    QTextStream input(&file);
    QString header = input.readLine();
    if (header.startsWith(QChar(0xFEFF))) header.remove(0, 1);
    if (header != QStringLiteral(
            "user_id,player_id_number,riot_id,tagline,puuid,soloq_rank,winrate")) {
        if (errorMessage) *errorMessage = QStringLiteral("El encabezado del CSV de cuentas no es válido.");
        return false;
    }

    QVector<LoLUser> loadedUsers;
    int lineNumber = 1;
    while (!input.atEnd()) {
        const QString line = input.readLine();
        ++lineNumber;
        if (line.trimmed().isEmpty()) continue;

        QStringList fields;
        if (!parseCsvRow(line, fields) || fields.size() != 7) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Formato CSV inválido en la fila %1.")
                                    .arg(lineNumber);
            }
            return false;
        }

        bool userIdValid = false;
        bool winrateValid = false;
        const int userId = fields[0].toInt(&userIdValid);
        const double winrate = fields[6].toDouble(&winrateValid);
        if (!userIdValid || !winrateValid) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("ID o winrate inválido en la fila %1.")
                                    .arg(lineNumber);
            }
            return false;
        }

        loadedUsers.append(LoLUser{
            userId,
            fields[1].toStdString(),
            fields[2].toStdString(),
            fields[3].toStdString(),
            fields[4].toStdString(),
            fields[5].toStdString(),
            winrate
        });
    }

    lolUsers = loadedUsers;
    return true;
}

bool EsportsCore::addPlayer(const Player& player, const QString& filePath,
                            QString* errorMessage)
{
    if (!validatePlayer(player, players, -1, errorMessage)) return false;
    QVector<Player> updatedPlayers = players;
    updatedPlayers.append(player);
    if (!savePlayers(updatedPlayers, filePath, errorMessage)) return false;
    players = updatedPlayers;
    return true;
}

bool EsportsCore::updatePlayer(int index, const Player& player, const QString& filePath,
                               QString* errorMessage)
{
    if (index < 0 || index >= players.size()) {
        if (errorMessage) *errorMessage = QStringLiteral("El jugador seleccionado ya no existe.");
        return false;
    }
    if (!validatePlayer(player, players, index, errorMessage)) return false;

    QVector<Player> updatedPlayers = players;
    updatedPlayers[index] = player;
    if (!savePlayers(updatedPlayers, filePath, errorMessage)) return false;
    players = updatedPlayers;
    return true;
}

bool EsportsCore::removePlayer(int index, const QString& filePath, QString* errorMessage)
{
    if (index < 0 || index >= players.size()) {
        if (errorMessage) *errorMessage = QStringLiteral("El jugador seleccionado ya no existe.");
        return false;
    }

    QVector<Player> updatedPlayers = players;
    updatedPlayers.removeAt(index);
    if (!savePlayers(updatedPlayers, filePath, errorMessage)) return false;
    players = updatedPlayers;
    return true;
}

bool EsportsCore::updateLolUser(int index, const LoLUser& user, const QString& filePath,
                                QString* errorMessage)
{
    if (index < 0 || index >= lolUsers.size()) {
        if (errorMessage) *errorMessage = QStringLiteral("La cuenta seleccionada ya no existe.");
        return false;
    }
    if (user.userId != lolUsers[index].userId
        || user.playerIdNumber != lolUsers[index].playerIdNumber) {
        if (errorMessage) *errorMessage = QStringLiteral("No se pueden cambiar los IDs de la cuenta.");
        return false;
    }

    QVector<LoLUser> updatedUsers = lolUsers;
    updatedUsers[index] = user;
    if (!saveLolUsers(updatedUsers, filePath, errorMessage)) return false;
    lolUsers = updatedUsers;
    return true;
}

bool EsportsCore::loadCsvTable(const QString& filePath, const QStringList& headers,
                               CsvTableData* table, QString* errorMessage) const
{
    if (!table || headers.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("La definición de la tabla CSV no es válida.");
        return false;
    }

    QFile file(filePath);
    if (!file.exists()) {
        CsvTableData emptyTable{headers, {}};
        if (!saveCsvTable(filePath, emptyTable, errorMessage)) return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    QTextStream input(&file);
    QString header = input.readLine();
    if (header.startsWith(QChar(0xFEFF))) header.remove(0, 1);
    if (header != headers.join(QLatin1Char(','))) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("El encabezado de %1 no coincide con el esperado.")
                                .arg(QFileInfo(filePath).fileName());
        }
        return false;
    }

    CsvTableData loadedTable;
    loadedTable.headers = headers;
    int lineNumber = 1;
    while (!input.atEnd()) {
        const QString line = input.readLine();
        ++lineNumber;
        if (line.trimmed().isEmpty()) continue;

        QStringList fields;
        if (!parseCsvRow(line, fields) || fields.size() != headers.size()) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("Formato CSV inválido en %1, fila %2.")
                                    .arg(QFileInfo(filePath).fileName())
                                    .arg(lineNumber);
            }
            return false;
        }
        loadedTable.rows.append(fields);
    }

    *table = loadedTable;
    return true;
}

bool EsportsCore::saveCsvTable(const QString& filePath, const CsvTableData& table,
                               QString* errorMessage) const
{
    if (table.headers.isEmpty()) {
        if (errorMessage) *errorMessage = QStringLiteral("La tabla CSV no tiene encabezados.");
        return false;
    }

    QString contents = table.headers.join(QLatin1Char(',')) + QLatin1Char('\n');
    for (const QStringList& row : table.rows) {
        if (row.size() != table.headers.size()) {
            if (errorMessage) *errorMessage = QStringLiteral("Una fila no coincide con las columnas de la tabla.");
            return false;
        }
        QStringList escapedFields;
        for (const QString& field : row) escapedFields.append(escapeCsvField(field));
        contents += escapedFields.join(QLatin1Char(',')) + QLatin1Char('\n');
    }

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    const QByteArray data = contents.toUtf8();
    if (file.write(data) != data.size()) {
        if (errorMessage) *errorMessage = file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }
    return true;
}

const QVector<Player>& EsportsCore::getPlayers() const
{
    return players;
}

const QVector<LoLUser>& EsportsCore::getLolUsers() const
{
    return lolUsers;
}