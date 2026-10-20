#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDateEdit>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFile>
#include <QFrame>
#include <QFontMetrics>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QHash>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QPixmap>
#include <QPolygon>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QSplitter>
#include <QTableWidgetItem>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>

namespace {
QString findCsvDirectory()
{
    const QStringList startingPoints = {
        QDir::currentPath(),
        QCoreApplication::applicationDirPath()
    };

    for (const QString& startingPoint : startingPoints) {
        QDir directory(startingPoint);
        while (directory.exists()) {
            if (directory.exists("CSVs")) return directory.filePath("CSVs");
            const QString currentPath = directory.absolutePath();
            if (!directory.cdUp() || directory.absolutePath() == currentPath) break;
        }
    }

    return QDir::current().filePath("CSVs");
}

bool createEmptyLolUsersCsv(const QString& filePath, QString* errorMessage)
{
    if (QFile::exists(filePath)) return true;

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage) *errorMessage = file.errorString();
        return false;
    }

    const QByteArray header =
        "user_id,player_id_number,riot_id,tagline,puuid,soloq_rank,winrate\n";
    if (file.write(header) != header.size()) {
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

QString riotReplyError(QNetworkReply* reply, const QByteArray& body)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (parseError.error == QJsonParseError::NoError && document.isObject()) {
        const QString message = document.object().value("status").toObject()
                                    .value("message").toString();
        if (!message.isEmpty()) {
            return QString("Riot API: %1").arg(message);
        }
    }

    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 0) {
        return QString("Riot API respondió con HTTP %1.").arg(status);
    }
    return reply->errorString();
}

QIcon countryFlagIcon(const QString& country)
{
    static const QHash<QString, QString> countryCodes = {
        {"argentina", "AR"}, {"bolivia", "BO"}, {"brasil", "BR"},
        {"brazil", "BR"}, {"chile", "CL"}, {"colombia", "CO"},
        {"costa rica", "CR"}, {"cuba", "CU"}, {"ecuador", "EC"},
        {"el salvador", "SV"}, {"guatemala", "GT"}, {"haití", "HT"},
        {"haiti", "HT"}, {"honduras", "HN"}, {"jamaica", "JM"},
        {"méxico", "MX"}, {"mexico", "MX"}, {"nicaragua", "NI"},
        {"panamá", "PA"}, {"panama", "PA"}, {"paraguay", "PY"},
        {"perú", "PE"}, {"peru", "PE"}, {"puerto rico", "PR"},
        {"república dominicana", "DO"}, {"republica dominicana", "DO"},
        {"uruguay", "UY"}, {"venezuela", "VE"}
    };

    const QString code = countryCodes.value(country.trimmed().toLower());
    if (code.isEmpty()) return {};

    const QColor blue("#75aadb");
    const QColor darkBlue("#0038a8");
    const QColor red("#d52b1e");
    const QColor yellow("#fcd116");
    const QColor green("#009739");
    const QColor white(Qt::white);
    QPixmap flag(36, 24);
    flag.fill(white);
    QPainter painter(&flag);
    painter.setRenderHint(QPainter::Antialiasing);

    const auto horizontal = [&](const QColor& first, const QColor& second,
                                const QColor& third) {
        painter.fillRect(0, 0, 36, 8, first);
        painter.fillRect(0, 8, 36, 8, second);
        painter.fillRect(0, 16, 36, 8, third);
    };
    const auto vertical = [&](const QColor& first, const QColor& second,
                              const QColor& third) {
        painter.fillRect(0, 0, 12, 24, first);
        painter.fillRect(12, 0, 12, 24, second);
        painter.fillRect(24, 0, 12, 24, third);
    };

    if (code == "AR") {
        horizontal(blue, white, blue);
        painter.setBrush(yellow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(18, 12), 2, 2);
    } else if (code == "BO") {
        horizontal(red, yellow, green);
    } else if (code == "BR") {
        painter.fillRect(flag.rect(), QColor("#009b3a"));
        painter.setBrush(yellow);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(QPolygon({{18, 3}, {33, 12}, {18, 21}, {3, 12}}));
        painter.setBrush(QColor("#002776"));
        painter.drawEllipse(QPoint(18, 12), 5, 5);
    } else if (code == "CL") {
        painter.fillRect(0, 12, 36, 12, red);
        painter.fillRect(0, 0, 12, 12, darkBlue);
        painter.fillRect(12, 0, 24, 12, white);
        painter.setBrush(white);
        painter.drawEllipse(QPoint(6, 6), 2, 2);
    } else if (code == "CO") {
        painter.fillRect(0, 0, 36, 12, yellow);
        painter.fillRect(0, 12, 36, 6, darkBlue);
        painter.fillRect(0, 18, 36, 6, red);
    } else if (code == "CR") {
        painter.fillRect(0, 0, 36, 4, darkBlue);
        painter.fillRect(0, 4, 36, 4, white);
        painter.fillRect(0, 8, 36, 8, red);
        painter.fillRect(0, 16, 36, 4, white);
        painter.fillRect(0, 20, 36, 4, darkBlue);
    } else if (code == "CU") {
        painter.fillRect(0, 0, 36, 5, darkBlue);
        painter.fillRect(0, 10, 36, 4, darkBlue);
        painter.fillRect(0, 19, 36, 5, darkBlue);
        painter.setBrush(red);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(QPolygon({{0, 0}, {15, 12}, {0, 24}}));
        painter.setBrush(white);
        painter.drawEllipse(QPoint(5, 12), 1, 1);
    } else if (code == "DO") {
        painter.fillRect(0, 0, 16, 10, darkBlue);
        painter.fillRect(20, 0, 16, 10, red);
        painter.fillRect(0, 14, 16, 10, red);
        painter.fillRect(20, 14, 16, 10, darkBlue);
        painter.fillRect(16, 0, 4, 24, white);
        painter.fillRect(0, 10, 36, 4, white);
    } else if (code == "EC") {
        painter.fillRect(0, 0, 36, 12, yellow);
        painter.fillRect(0, 12, 36, 6, darkBlue);
        painter.fillRect(0, 18, 36, 6, red);
    } else if (code == "GT") {
        vertical(blue, white, blue);
    } else if (code == "HN" || code == "NI" || code == "SV") {
        horizontal(darkBlue, white, darkBlue);
        painter.setBrush(code == "HN" ? darkBlue : blue);
        painter.setPen(Qt::NoPen);
        if (code == "HN") {
            painter.drawEllipse(QPoint(15, 10), 1, 1);
            painter.drawEllipse(QPoint(21, 10), 1, 1);
            painter.drawEllipse(QPoint(18, 12), 1, 1);
            painter.drawEllipse(QPoint(15, 14), 1, 1);
            painter.drawEllipse(QPoint(21, 14), 1, 1);
        } else {
            painter.drawEllipse(QPoint(18, 12), 2, 2);
        }
    } else if (code == "HT") {
        horizontal(darkBlue, red, white);
    } else if (code == "JM") {
        painter.fillRect(flag.rect(), QColor("#009b3a"));
        painter.setPen(QPen(yellow, 5));
        painter.drawLine(0, 0, 36, 24);
        painter.drawLine(0, 24, 36, 0);
        painter.fillRect(0, 0, 12, 8, QColor("#009b3a"));
        painter.fillRect(24, 16, 12, 8, QColor("#009b3a"));
        painter.fillRect(24, 0, 12, 8, Qt::black);
        painter.fillRect(0, 16, 12, 8, Qt::black);
    } else if (code == "MX") {
        vertical(green, white, red);
        painter.setBrush(QColor("#8c5a2b"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(18, 12), 2, 3);
    } else if (code == "PA") {
        painter.fillRect(0, 0, 18, 12, white);
        painter.fillRect(18, 0, 18, 12, red);
        painter.fillRect(0, 12, 18, 12, darkBlue);
        painter.fillRect(18, 12, 18, 12, white);
        painter.setBrush(darkBlue);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(9, 6), 2, 2);
        painter.setBrush(red);
        painter.drawEllipse(QPoint(27, 18), 2, 2);
    } else if (code == "PE") {
        vertical(red, white, red);
    } else if (code == "PR") {
        painter.fillRect(0, 0, 36, 5, red);
        painter.fillRect(0, 10, 36, 4, red);
        painter.fillRect(0, 19, 36, 5, red);
        painter.setBrush(darkBlue);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(QPolygon({{0, 0}, {18, 12}, {0, 24}}));
        painter.setBrush(white);
        painter.drawEllipse(QPoint(6, 12), 2, 2);
    } else if (code == "PY") {
        horizontal(red, white, darkBlue);
    } else if (code == "UY") {
        painter.fillRect(flag.rect(), white);
        for (int y = 5; y < 24; y += 5) painter.fillRect(0, y, 36, 2, blue);
        painter.fillRect(0, 0, 14, 11, white);
        painter.setBrush(yellow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPoint(7, 5), 3, 3);
    } else if (code == "VE") {
        horizontal(yellow, darkBlue, red);
        painter.setPen(QPen(white, 1));
        for (int i = 0; i < 7; ++i) {
            const qreal x = 7.0 + i * 3.7;
            const qreal y = 9.0 - qAbs(3 - i) * 0.8;
            painter.drawEllipse(QPointF(x, y), 0.7, 0.7);
        }
    } else {
        return {};
    }

    painter.end();
    QIcon icon(flag);
    return icon;
}

QIcon rankedEmblemIcon(const QString& rank)
{
    const QString tier = rank.section(QLatin1Char(' '), 0, 0).trimmed().toUpper();
    static const QHash<QString, QString> emblemNames = {
        {"UNRANKED", "Emblem_Unranked.png"},
        {"IRON", "Emblem_Iron.png"},
        {"BRONZE", "Emblem_Bronze.png"},
        {"SILVER", "Emblem_Silver.png"},
        {"GOLD", "Emblem_Gold.png"},
        {"PLATINUM", "Emblem_Platinum.png"},
        {"EMERALD", "Emblem_Emerald.png"},
        {"DIAMOND", "Emblem_Diamond.png"},
        {"MASTER", "Emblem_Master.png"},
        {"GRANDMASTER", "Emblem_Grandmaster.png"},
        {"CHALLENGER", "Emblem_Challenger.png"}
    };

    const QString fileName = emblemNames.value(tier);
    if (fileName.isEmpty()) return {};

    const QStringList startingPoints = {
        QDir::currentPath(),
        QCoreApplication::applicationDirPath()
    };
    for (const QString& startingPoint : startingPoints) {
        QDir directory(startingPoint);
        while (directory.exists()) {
            const QString imagePath =
                directory.filePath(QStringLiteral("assets/ranked_emblems/") + fileName);
            if (QFile::exists(imagePath)) return QIcon(imagePath);

            const QString currentPath = directory.absolutePath();
            if (!directory.cdUp() || directory.absolutePath() == currentPath) break;
        }
    }

    return {};
}

QIcon teamLogoIcon(const QString& teamName)
{
    QString fileName = teamName;
    fileName.replace(' ', '_');
    fileName.replace(QStringLiteral("Leviatán"), QStringLiteral("Leviatan"));
    const QString logoPath = QDir(findCsvDirectory()).filePath(
        QStringLiteral("../assets/team_logos/") + fileName + QStringLiteral(".png"));
    return QFile::exists(logoPath) ? QIcon(logoPath) : QIcon();
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->searchPlayerEdit, &QLineEdit::textChanged, this,
            [this](const QString& text) {
                for (int row = 0; row < ui->tablePlayers->rowCount(); ++row){
            bool matches = false;
                    for (int column = 0; column < ui->tablePlayers->columnCount(); ++column){
                const QTableWidgetItem* item = ui->tablePlayers ->item(row,column);
                        if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                    matches = true;
                            break;
                        }
                    }
                    ui->tablePlayers->setRowHidden(row, !matches);
                }
    });
    ui->tablePlayers->setColumnCount(6);
    ui->tablePlayers->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Apellido", "País", "Fecha de nacimiento", "Rol"});
    ui->tablePlayers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tablePlayers->verticalHeader()->setVisible(false);
    ui->tableLolUsers->setColumnCount(7);
    ui->tableLolUsers->setHorizontalHeaderLabels(
        {"ID cuenta", "ID jugador", "Riot ID", "Tagline", "PUUID", "Rango", "Winrate"});
    ui->tableLolUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableLolUsers->verticalHeader()->setVisible(false);
    connect(ui->updateLolAccountButton, &QPushButton::clicked,
            this, &MainWindow::updateSelectedLolAccount);
    connect(ui->updateAllLolAccountsButton, &QPushButton::clicked,
            this, &MainWindow::updateAllLolAccounts);
    connect(ui->addPlayerButton, &QPushButton::clicked, this, &MainWindow::addPlayer);
    connect(ui->editPlayerButton, &QPushButton::clicked, this, &MainWindow::editPlayer);
    connect(ui->removePlayerButton, &QPushButton::clicked, this, &MainWindow::removePlayer);
    connect(ui->changePlayerRoleButton, &QPushButton::clicked,
            this, &MainWindow::changeSelectedPlayerRole);
    connect(ui->undoPlayerRoleButton, &QPushButton::clicked,
            this, &MainWindow::undoLastRoleChange);
    connect(ui->tablePlayers, &QTableWidget::itemSelectionChanged, this,[this](){
    const int row = ui->tablePlayers->currentRow();
    if (row < 0) return;

    const QTableWidgetItem* nameitem = ui->tablePlayers->item(row, 1);
    if (nameitem){
        ui->statusbar->showMessage(
            QString("Jugador Seleccionado: %1").arg(nameitem->text()));
    }
    });

    playersCsvPath = QDir(findCsvDirectory()).filePath("state_players.csv");
    QString error;
    if (!core.loadPlayers(playersCsvPath, &error)) {
        ui->statusbar->showMessage(
            QString("No se pudieron cargar los jugadores desde %1: %2")
                .arg(playersCsvPath, error));
    } else {
        ui->statusbar->showMessage(
            QString("%1 jugadores cargados desde CSV.").arg(core.getPlayers().size()));
    }

    refreshPlayersTable();

    lolUsersCsvPath = QDir(findCsvDirectory()).filePath("state_lol_users.csv");
    QString lolUsersError;
    if (!createEmptyLolUsersCsv(lolUsersCsvPath, &lolUsersError)) {
        ui->lolUsersStatusLabel->setText(
            QString("No se pudo preparar el archivo de cuentas: %1").arg(lolUsersError));
        return;
    }
    if (!core.loadLolUsers(lolUsersCsvPath, &lolUsersError)) {
        ui->lolUsersStatusLabel->setText(
            QString("No se pudieron cargar las cuentas: %1").arg(lolUsersError));
        return;
    }

    refreshLolUsersTable();
    refreshSummaryReports();
    const auto& lolUsers = core.getLolUsers();
    ui->lolUsersStatusLabel->setText(
        lolUsers.isEmpty()
            ? QStringLiteral("Aún no hay cuentas.")
            : QString("%1 cuentas cargadas desde CSV.").arg(lolUsers.size()));

    addCsvDataTabs();
    addSummaryReportTabs();
}

void MainWindow::updateSelectedLolAccount()
{
    const int index = ui->tableLolUsers->currentRow();
    if (index < 0 || index >= core.getLolUsers().size()) {
        QMessageBox::information(this, "Selecciona una cuenta",
                                 "Selecciona la cuenta de LoL que quieres actualizar.");
        return;
    }

    riotApiKey = qgetenv("RIOT_API_KEY").trimmed();
    if (riotApiKey.isEmpty()) {
        QMessageBox::warning(
            this, "Falta la API key",
            "Configura RIOT_API_KEY en el entorno de ejecución de Qt Creator. "
            "No escribas la clave directamente en el código.");
        return;
    }

    lolAccountUpdateQueue = {index};
    lolAccountUpdateTotal = 1;
    lolAccountUpdateSuccesses = 0;
    lolAccountUpdateErrors.clear();
    updatingAllLolAccounts = false;
    ui->updateLolAccountButton->setEnabled(false);
    ui->updateAllLolAccountsButton->setEnabled(false);
    ui->tableLolUsers->setEnabled(false);
    processNextLolAccountUpdate();
}

void MainWindow::updateAllLolAccounts()
{
    if (core.getLolUsers().isEmpty()) {
        QMessageBox::information(this, "No hay cuentas",
                                 "No hay cuentas de LoL para actualizar.");
        return;
    }

    riotApiKey = qgetenv("RIOT_API_KEY").trimmed();
    if (riotApiKey.isEmpty()) {
        QMessageBox::warning(
            this, "Falta la API key",
            "Configura RIOT_API_KEY en el entorno de ejecución de Qt Creator. "
            "No escribas la clave directamente en el código.");
        return;
    }

    if (QMessageBox::question(
            this, "Actualizar todas las cuentas",
            QString("Se consultarán %1 cuentas de Riot. Esto puede tardar un poco. ¿Continuar?")
                .arg(core.getLolUsers().size()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    lolAccountUpdateQueue.clear();
    for (int index = 0; index < core.getLolUsers().size(); ++index) {
        lolAccountUpdateQueue.append(index);
    }
    lolAccountUpdateTotal = lolAccountUpdateQueue.size();
    lolAccountUpdateSuccesses = 0;
    lolAccountUpdateErrors.clear();
    updatingAllLolAccounts = true;
    ui->updateLolAccountButton->setEnabled(false);
    ui->updateAllLolAccountsButton->setEnabled(false);
    ui->tableLolUsers->setEnabled(false);
    processNextLolAccountUpdate();
}

void MainWindow::processNextLolAccountUpdate()
{
    if (lolAccountUpdateQueue.isEmpty()) {
        const int failed = lolAccountUpdateTotal - lolAccountUpdateSuccesses;
        ui->updateLolAccountButton->setEnabled(true);
        ui->updateAllLolAccountsButton->setEnabled(true);
        ui->tableLolUsers->setEnabled(true);
        riotApiKey.clear();

        if (failed == 0) {
            ui->lolUsersStatusLabel->setText(
                QString("%1 cuentas actualizadas desde Riot y guardadas en CSV.")
                    .arg(lolAccountUpdateSuccesses));
        } else {
            ui->lolUsersStatusLabel->setText(
                QString("%1 de %2 cuentas actualizadas. Fallos: %3")
                    .arg(lolAccountUpdateSuccesses)
                    .arg(lolAccountUpdateTotal)
                    .arg(lolAccountUpdateErrors.join("; ")));
        }
        updatingAllLolAccounts = false;
        return;
    }

    const int index = lolAccountUpdateQueue.takeFirst();
    const LoLUser selectedUser = core.getLolUsers()[index];
    const QString riotId = QString::fromStdString(selectedUser.riotId).trimmed();
    const QString tagline = QString::fromStdString(selectedUser.tagline).trimmed();
    if (riotId.isEmpty() || tagline.isEmpty()) {
        finishLolAccountUpdate(index, "Riot ID o tagline vacío");
        return;
    }

    const int current = lolAccountUpdateTotal - lolAccountUpdateQueue.size();
    ui->lolUsersStatusLabel->setText(
        QString("Actualizando cuenta %1 de %2: %3#%4...")
            .arg(current).arg(lolAccountUpdateTotal).arg(riotId, tagline));

    const QString encodedRiotId = QString::fromLatin1(QUrl::toPercentEncoding(riotId));
    const QString encodedTagline = QString::fromLatin1(QUrl::toPercentEncoding(tagline));
    const QUrl accountUrl(QStringLiteral(
        "https://americas.api.riotgames.com/riot/account/v1/accounts/by-riot-id/%1/%2")
                              .arg(encodedRiotId, encodedTagline));
    QNetworkRequest accountRequest(accountUrl);
    accountRequest.setRawHeader("X-Riot-Token", riotApiKey);
    QNetworkReply* accountReply = networkManager.get(accountRequest);
    connect(accountReply, &QNetworkReply::finished, this,
            [this, accountReply, index, selectedUser]() {
        const QByteArray body = accountReply->readAll();
        if (accountReply->error() != QNetworkReply::NoError) {
            const QString error = riotReplyError(accountReply, body);
            accountReply->deleteLater();
            finishLolAccountUpdate(index, error);
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        const QString puuid = document.object().value("puuid").toString();
        accountReply->deleteLater();
        if (parseError.error != QJsonParseError::NoError || puuid.isEmpty()) {
            finishLolAccountUpdate(index, "Riot no devolvió un PUUID válido");
            return;
        }

        const QString encodedPuuid = QString::fromLatin1(QUrl::toPercentEncoding(puuid));
        const QUrl rankUrl(QStringLiteral(
            "https://la1.api.riotgames.com/lol/league/v4/entries/by-puuid/%1")
                               .arg(encodedPuuid));
        QNetworkRequest rankRequest(rankUrl);
        rankRequest.setRawHeader("X-Riot-Token", riotApiKey);
        QNetworkReply* rankReply = networkManager.get(rankRequest);
        connect(rankReply, &QNetworkReply::finished, this,
                [this, rankReply, index, selectedUser, puuid]() {
            const QByteArray rankBody = rankReply->readAll();
            if (rankReply->error() != QNetworkReply::NoError) {
                const QString error = riotReplyError(rankReply, rankBody);
                rankReply->deleteLater();
                finishLolAccountUpdate(index, error);
                return;
            }

            QJsonParseError rankParseError;
            const QJsonDocument rankDocument =
                QJsonDocument::fromJson(rankBody, &rankParseError);
            if (rankParseError.error != QJsonParseError::NoError
                || !rankDocument.isArray()) {
                rankReply->deleteLater();
                finishLolAccountUpdate(index, "Riot devolvió un formato de rango inesperado");
                return;
            }

            LoLUser updatedUser = selectedUser;
            updatedUser.puuid = puuid.toStdString();
            updatedUser.soloqRank = "Unranked";
            updatedUser.winrate = 0.0;
            for (const QJsonValue& value : rankDocument.array()) {
                const QJsonObject queue = value.toObject();
                if (queue.value("queueType").toString() != "RANKED_SOLO_5x5") continue;

                const QString tier = queue.value("tier").toString();
                const QString division = queue.value("rank").toString();
                const int leaguePoints = queue.value("leaguePoints").toInt();
                updatedUser.soloqRank =
                    QString("%1 %2 (%3 LP)").arg(tier, division).arg(leaguePoints).toStdString();
                const int wins = queue.value("wins").toInt();
                const int losses = queue.value("losses").toInt();
                if (wins + losses > 0) {
                    updatedUser.winrate = 100.0 * wins / (wins + losses);
                }
                break;
            }
            rankReply->deleteLater();

            QString error;
            if (!core.updateLolUser(index, updatedUser, lolUsersCsvPath, &error)) {
                finishLolAccountUpdate(index, QString("No se pudo guardar: %1").arg(error));
                return;
            }

            ++lolAccountUpdateSuccesses;
            refreshLolUsersTable();
            refreshSummaryReports();
            finishLolAccountUpdate(index);
        });
    });
}

void MainWindow::finishLolAccountUpdate(int index, const QString& error)
{
    if (!error.isEmpty()) {
        const LoLUser& user = core.getLolUsers()[index];
        const QString riotId = QString::fromStdString(user.riotId);
        const QString tagline = QString::fromStdString(user.tagline);
        lolAccountUpdateErrors.append(
            QString("%1#%2: %3").arg(riotId, tagline, error));
    }

    if (!updatingAllLolAccounts) {
        processNextLolAccountUpdate();
        return;
    }
    QTimer::singleShot(1000, this, &MainWindow::processNextLolAccountUpdate);
}

void MainWindow::refreshLolUsersTable()
{
    const auto& lolUsers = core.getLolUsers();
    ui->tableLolUsers->setRowCount(lolUsers.size());
    for (int row = 0; row < lolUsers.size(); ++row) {
        const LoLUser& user = lolUsers[row];
        const QStringList values = {
            QString::number(user.userId),
            QString::fromStdString(user.playerIdNumber),
            QString::fromStdString(user.riotId),
            QString::fromStdString(user.tagline),
            QString::fromStdString(user.puuid),
            QString::number(user.winrate, 'f', 2) + "%"
        };
        for (int i = 0; i < values.size(); ++i) {
            const int column = i < 5 ? i : i + 1;
            ui->tableLolUsers->setItem(row, column, new QTableWidgetItem(values[i]));
        }
        const QString rank = QString::fromStdString(user.soloqRank);
        auto* rankItem = new QTableWidgetItem(rank);
        const QIcon emblem = rankedEmblemIcon(rank);
        if (!emblem.isNull()) rankItem->setIcon(emblem);
        ui->tableLolUsers->setItem(row, 5, rankItem);
    }
}

void MainWindow::addPlayer()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Agregar jugador");
    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout;
    auto* idNumber = new QLineEdit(&dialog);
    auto* name = new QLineEdit(&dialog);
    auto* lastName = new QLineEdit(&dialog);
    auto* country = new QLineEdit(&dialog);
    auto* birthDate = new QDateEdit(QDate::currentDate(), &dialog);
    birthDate->setCalendarPopup(true);
    birthDate->setDisplayFormat("yyyy-MM-dd");
    form->addRow("ID:", idNumber);
    form->addRow("Nombre:", name);
    form->addRow("Apellido:", lastName);
    form->addRow("País:", country);
    form->addRow("Fecha de nacimiento:", birthDate);
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) return;

    const Player player{
        idNumber->text().trimmed().toStdString(),
        name->text().trimmed().toStdString(),
        lastName->text().trimmed().toStdString(),
        country->text().trimmed().toStdString(),
        birthDate->date().toString(Qt::ISODate).toStdString(),
        {}
    };
    QString error;
    if (!core.addPlayer(player, playersCsvPath, &error)) {
        QMessageBox::warning(this, "No se pudo agregar el jugador", error);
        return;
    }

    refreshPlayersTable();
    refreshSummaryReports();
    ui->statusbar->showMessage(
        QString("%1 jugadores guardados en CSV.").arg(core.getPlayers().size()));
}

void MainWindow::editPlayer()
{
    const int index = ui->tablePlayers->currentRow();
    if (index < 0 || index >= core.getPlayers().size()) {
        QMessageBox::information(this, "Selecciona un jugador",
                                 "Selecciona la fila del jugador que quieres editar.");
        return;
    }

    const Player& currentPlayer = core.getPlayers()[index];
    QDialog dialog(this);
    dialog.setWindowTitle("Editar jugador");
    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout;
    auto* idNumber = new QLineEdit(QString::fromStdString(currentPlayer.idNumber), &dialog);
    auto* name = new QLineEdit(QString::fromStdString(currentPlayer.name), &dialog);
    auto* lastName = new QLineEdit(QString::fromStdString(currentPlayer.lastName), &dialog);
    auto* country = new QLineEdit(QString::fromStdString(currentPlayer.country), &dialog);
    auto* birthDate = new QDateEdit(
        QDate::fromString(QString::fromStdString(currentPlayer.birthDate), Qt::ISODate), &dialog);
    if (!birthDate->date().isValid()) birthDate->setDate(QDate::currentDate());
    birthDate->setCalendarPopup(true);
    birthDate->setDisplayFormat("yyyy-MM-dd");
    form->addRow("ID:", idNumber);
    form->addRow("Nombre:", name);
    form->addRow("Apellido:", lastName);
    form->addRow("País:", country);
    form->addRow("Fecha de nacimiento:", birthDate);
    layout->addLayout(form);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return;

    const Player player{
        idNumber->text().trimmed().toStdString(),
        name->text().trimmed().toStdString(),
        lastName->text().trimmed().toStdString(),
        country->text().trimmed().toStdString(),
        birthDate->date().toString(Qt::ISODate).toStdString(),
        currentPlayer.role
    };
    QString error;
    if (!core.updatePlayer(index, player, playersCsvPath, &error)) {
        QMessageBox::warning(this, "No se pudo editar el jugador", error);
        return;
    }

    refreshPlayersTable();
    ui->tablePlayers->selectRow(index);
    refreshSummaryReports();
    ui->statusbar->showMessage("Jugador actualizado y guardado en CSV.");
}

void MainWindow::removePlayer()
{
    const int index = ui->tablePlayers->currentRow();
    if (index < 0 || index >= core.getPlayers().size()) {
        QMessageBox::information(this, "Selecciona un jugador",
                                 "Selecciona la fila del jugador que quieres eliminar.");
        return;
    }

    const Player& player = core.getPlayers()[index];
    const QString playerName = QStringLiteral("%1 %2")
                                   .arg(QString::fromStdString(player.name),
                                        QString::fromStdString(player.lastName));
    if (QMessageBox::question(
            this, "Confirmar eliminación",
            QStringLiteral("¿Eliminar a %1 (ID: %2)?")
                .arg(playerName, QString::fromStdString(player.idNumber)),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!core.removePlayer(index, playersCsvPath, &error)) {
        QMessageBox::warning(this, "No se pudo eliminar el jugador", error);
        return;
    }

    refreshPlayersTable();
    refreshSummaryReports();
    ui->statusbar->showMessage("Jugador eliminado y CSV actualizado.");
}

void MainWindow::addContract()
{
    QString error;
    if (!showContractDialog(-1, &error) && !error.isEmpty()) {
        QMessageBox::warning(this, "No se pudo crear el contrato", error);
    }
}

void MainWindow::editSelectedContract()
{
    CsvTableView* view = findCsvTable("state_contracts.csv");
    if (!view || !view->table) return;
    const int row = view->table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Selecciona un contrato",
                                 "Selecciona el contrato que quieres editar.");
        return;
    }

    QString error;
    if (!showContractDialog(row, &error) && !error.isEmpty()) {
        QMessageBox::warning(this, "No se pudo editar el contrato", error);
    }
}

void MainWindow::removeSelectedContract()
{
    CsvTableView* view = findCsvTable("state_contracts.csv");
    if (!view || !view->table) return;
    const int row = view->table->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Selecciona un contrato",
                                 "Selecciona el contrato que quieres eliminar.");
        return;
    }

    CsvTableData contracts;
    QString error;
    if (!core.loadCsvTable(view->filePath, view->csvHeaders, &contracts, &error)) {
        QMessageBox::warning(this, "No se pudieron cargar los contratos", error);
        return;
    }
    if (row >= contracts.rows.size()) {
        QMessageBox::warning(this, "Contrato no disponible",
                             "La tabla cambió. Actualiza la vista e inténtalo de nuevo.");
        return;
    }

    const QString contractId = contracts.rows[row][0];
    if (QMessageBox::question(
            this, "Eliminar contrato",
            QString("¿Eliminar el contrato %1?").arg(contractId),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    contracts.rows.removeAt(row);
    if (!core.saveCsvTable(view->filePath, contracts, &error)) {
        QMessageBox::warning(this, "No se pudo eliminar el contrato", error);
        return;
    }
    refreshCsvTable(*view);
    refreshSummaryReports();
}

bool MainWindow::showContractDialog(int contractRow, QString* errorMessage)
{
    CsvTableView* contractsView = findCsvTable("state_contracts.csv");
    CsvTableView* teamsView = findCsvTable("state_teams.csv");
    if (!contractsView || !teamsView) {
        if (errorMessage) *errorMessage = "No se encontraron las tablas de contratos o equipos.";
        return false;
    }
    if (core.getPlayers().isEmpty()) {
        if (errorMessage) *errorMessage = "Agrega jugadores antes de crear contratos.";
        return false;
    }

    CsvTableData contracts;
    CsvTableData teams;
    if (!core.loadCsvTable(contractsView->filePath, contractsView->csvHeaders,
                           &contracts, errorMessage)
        || !core.loadCsvTable(teamsView->filePath, teamsView->csvHeaders,
                              &teams, errorMessage)) {
        return false;
    }
    if (contractRow >= contracts.rows.size()) {
        if (errorMessage) *errorMessage = "El contrato seleccionado ya no existe.";
        return false;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(contractRow < 0 ? "Agregar contrato" : "Editar contrato");
    auto* layout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout;
    auto* playerCombo = new QComboBox(&dialog);
    for (const Player& player : core.getPlayers()) {
        const QString label =
            QString("%1 — %2 %3")
                .arg(QString::fromStdString(player.idNumber),
                     QString::fromStdString(player.name),
                     QString::fromStdString(player.lastName));
        playerCombo->addItem(label, QString::fromStdString(player.idNumber));
    }

    auto* teamCombo = new QComboBox(&dialog);
    teamCombo->addItem("Sin equipo", 0);
    for (const QStringList& team : teams.rows) {
        bool idValid = false;
        const int teamId = team[0].toInt(&idValid);
        if (!idValid) {
            if (errorMessage) *errorMessage = "La tabla de equipos contiene un ID inválido.";
            return false;
        }
        teamCombo->addItem(QString("%1 (%2)").arg(team[1], team[2]), teamId);
    }

    auto* startDate = new QDateEdit(QDate::currentDate(), &dialog);
    startDate->setCalendarPopup(true);
    startDate->setDisplayFormat("yyyy-MM-dd");
    auto* hasEndDate = new QCheckBox("Definir fecha de fin", &dialog);
    auto* endDate = new QDateEdit(QDate::currentDate(), &dialog);
    endDate->setCalendarPopup(true);
    endDate->setDisplayFormat("yyyy-MM-dd");
    endDate->setEnabled(false);
    connect(hasEndDate, &QCheckBox::toggled, endDate, &QDateEdit::setEnabled);
    auto* active = new QCheckBox("Contrato activo", &dialog);
    active->setChecked(true);

    form->addRow("Jugador:", playerCombo);
    form->addRow("Equipo:", teamCombo);
    form->addRow("Inicio:", startDate);
    form->addRow(hasEndDate, endDate);
    form->addRow(active);
    layout->addLayout(form);

    int contractId = 0;
    if (contractRow >= 0) {
        const QStringList& row = contracts.rows[contractRow];
        bool idValid = false;
        contractId = row[0].toInt(&idValid);
        if (!idValid) {
            if (errorMessage) *errorMessage = "El contrato seleccionado tiene un ID inválido.";
            return false;
        }
        const int playerIndex = playerCombo->findData(row[1]);
        if (playerIndex < 0) {
            if (errorMessage) *errorMessage = "El jugador asociado al contrato ya no existe.";
            return false;
        }
        playerCombo->setCurrentIndex(playerIndex);
        bool teamIdValid = false;
        const int teamId = row[2].toInt(&teamIdValid);
        const int teamIndex = teamIdValid ? teamCombo->findData(teamId) : -1;
        if (teamIndex < 0) {
            if (errorMessage) *errorMessage = "El equipo asociado al contrato ya no existe.";
            return false;
        }
        teamCombo->setCurrentIndex(teamIndex);
        const QDate parsedStart = QDate::fromString(row[4], Qt::ISODate);
        if (parsedStart.isValid()) startDate->setDate(parsedStart);
        const QDate parsedEnd = QDate::fromString(row[5], Qt::ISODate);
        if (parsedEnd.isValid()) {
            hasEndDate->setChecked(true);
            endDate->setDate(parsedEnd);
        }
        active->setChecked(row[6] == "1" || row[6].compare("true", Qt::CaseInsensitive) == 0);
    }

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    if (dialog.exec() != QDialog::Accepted) return false;

    const QString playerId = playerCombo->currentData().toString();
    const auto playerIt = std::find_if(
        core.getPlayers().cbegin(), core.getPlayers().cend(),
        [&playerId](const Player& player) {
            return QString::fromStdString(player.idNumber) == playerId;
        });
    if (playerIt == core.getPlayers().cend()) {
        if (errorMessage) *errorMessage = "El jugador seleccionado ya no existe.";
        return false;
    }

    for (int i = 0; i < contracts.rows.size(); ++i) {
        if (i == contractRow) continue;
        const QStringList& row = contracts.rows[i];
        const bool isActive =
            row[6] == "1" || row[6].compare("true", Qt::CaseInsensitive) == 0;
        if (row[1] == playerId && isActive && active->isChecked()) {
            if (errorMessage) {
                *errorMessage = "Ese jugador ya tiene un contrato activo. "
                                "Desactiva o edita el contrato existente primero.";
            }
            return false;
        }
    }

    const QString role = QString::fromStdString(playerIt->role);
    if (role.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "Asigna primero un rol al jugador en la pestaña Jugadores.";
        }
        return false;
    }
    if (hasEndDate->isChecked() && endDate->date() < startDate->date()) {
        if (errorMessage) {
            *errorMessage = "La fecha de fin no puede ser anterior a la fecha de inicio.";
        }
        return false;
    }

    if (contractRow < 0) {
        contractId = 1;
        for (const QStringList& row : contracts.rows) {
            bool idValid = false;
            const int existingId = row[0].toInt(&idValid);
            if (!idValid) {
                if (errorMessage) *errorMessage = "La tabla de contratos contiene un ID inválido.";
                return false;
            }
            contractId = qMax(contractId, existingId + 1);
        }
        contracts.rows.append(QStringList{
            QString::number(contractId), playerId,
            QString::number(teamCombo->currentData().toInt()), role,
            startDate->date().toString(Qt::ISODate),
            hasEndDate->isChecked() ? endDate->date().toString(Qt::ISODate) : QString{},
            active->isChecked() ? "1" : "0"
        });
    } else {
        contracts.rows[contractRow] = {
            QString::number(contractId), playerId,
            QString::number(teamCombo->currentData().toInt()), role,
            startDate->date().toString(Qt::ISODate),
            hasEndDate->isChecked() ? endDate->date().toString(Qt::ISODate) : QString{},
            active->isChecked() ? "1" : "0"
        };
    }

    if (!core.saveCsvTable(contractsView->filePath, contracts, errorMessage)) return false;
    refreshCsvTable(*contractsView);
    refreshSummaryReports();
    ui->statusbar->showMessage(
        contractRow < 0 ? "Contrato agregado y guardado." : "Contrato actualizado y guardado.");
    return true;
}

void MainWindow::refreshPlayersTable()
{
    const auto& players = core.getPlayers();
    ui->tablePlayers->setRowCount(players.size());

    for (int row = 0; row < players.size(); ++row) {
        const Player& player = players[row];

        ui->tablePlayers->setItem(row, 0,
                                  new QTableWidgetItem(QString::fromStdString(player.idNumber)));
        ui->tablePlayers->setItem(row, 1,
                                  new QTableWidgetItem(QString::fromStdString(player.name)));
        ui->tablePlayers->setItem(row, 2,
                                  new QTableWidgetItem(QString::fromStdString(player.lastName)));
        const QString country = QString::fromStdString(player.country);
        auto* countryItem = new QTableWidgetItem(country);
        const QIcon flag = countryFlagIcon(country);
        if (!flag.isNull()) countryItem->setIcon(flag);
        ui->tablePlayers->setItem(row, 3, countryItem);
        ui->tablePlayers->setItem(row, 4,
                                  new QTableWidgetItem(QString::fromStdString(player.birthDate)));
        ui->tablePlayers->setItem(row, 5,
                                  new QTableWidgetItem(QString::fromStdString(player.role)));
    }
}

void MainWindow::changeSelectedPlayerRole()
{
    const int playerIndex = ui->tablePlayers->currentRow();
    if (playerIndex < 0 || playerIndex >= core.getPlayers().size()) {
        QMessageBox::information(this, "Selecciona un jugador",
                                 "Selecciona el jugador al que quieres cambiarle el rol.");
        return;
    }

    static const QStringList roles = {"TOP", "JG", "MID", "ADC", "SUP"};
    const Player& player = core.getPlayers()[playerIndex];
    CsvTableView* contractsView = findCsvTable("state_contracts.csv");
    CsvTableData contracts;
    QString error;
    if (!contractsView
        || !core.loadCsvTable(contractsView->filePath, contractsView->csvHeaders,
                              &contracts, &error)) {
        QMessageBox::warning(this, "No se pudo leer el contrato",
                             error.isEmpty() ? "No se encontró la tabla de contratos."
                                             : error);
        return;
    }

    const QString playerId = QString::fromStdString(player.idNumber);
    int activeContractRow = -1;
    for (int row = 0; row < contracts.rows.size(); ++row) {
        const QStringList& contract = contracts.rows[row];
        const bool active = contract[6] == "1"
            || contract[6].compare("true", Qt::CaseInsensitive) == 0;
        if (contract[1] != playerId || !active) continue;
        if (activeContractRow >= 0) {
            QMessageBox::warning(
                this, "Contratos activos duplicados",
                "Este jugador tiene más de un contrato activo. Corrige los contratos "
                "antes de cambiar el rol.");
            return;
        }
        activeContractRow = row;
    }

    const QString currentRole = activeContractRow >= 0
        ? contracts.rows[activeContractRow][3]
        : QString::fromStdString(player.role);
    bool accepted = false;
    const QString newRole = QInputDialog::getItem(
        this, "Cambiar rol", "Nuevo rol:", roles,
        qMax(0, roles.indexOf(currentRole)), false, &accepted);
    if (!accepted) return;
    if (newRole == currentRole) {
        QMessageBox::information(this, "Sin cambios",
                                 "El jugador ya tiene asignado ese rol.");
        return;
    }

    if (!savePlayerRoleChange(playerIndex, newRole, "CAMBIO", 0, &error)) {
        QMessageBox::warning(this, "No se pudo cambiar el rol", error);
        return;
    }
    refreshPlayersTable();
    ui->tablePlayers->selectRow(playerIndex);
    if (CsvTableView* changes = findCsvTable("state_player_changes.csv")) {
        refreshCsvTable(*changes);
    }
    ui->statusbar->showMessage(
        "Rol actualizado en el jugador y su contrato activo; cambio guardado en la auditoría.");
}

bool MainWindow::savePlayerRoleChange(int playerIndex, const QString& newRole,
                                      const QString& action, int referenceChangeId,
                                      QString* errorMessage)
{
    if (playerIndex < 0 || playerIndex >= core.getPlayers().size()) {
        if (errorMessage) *errorMessage = "El jugador seleccionado ya no existe.";
        return false;
    }
    CsvTableView* changesView = findCsvTable("state_player_changes.csv");
    CsvTableView* contractsView = findCsvTable("state_contracts.csv");
    if (!changesView || !contractsView) {
        if (errorMessage) {
            *errorMessage = "No se encontró la tabla de auditoría o de contratos.";
        }
        return false;
    }

    CsvTableData changes;
    CsvTableData contracts;
    if (!core.loadCsvTable(changesView->filePath, changesView->csvHeaders,
                           &changes, errorMessage)
        || !core.loadCsvTable(contractsView->filePath, contractsView->csvHeaders,
                              &contracts, errorMessage)) {
        return false;
    }
    int nextChangeId = 1;
    for (const QStringList& row : changes.rows) {
        bool idValid = false;
        const int changeId = row[0].toInt(&idValid);
        if (!idValid) {
            if (errorMessage) *errorMessage = "La auditoría contiene un ID de cambio inválido.";
            return false;
        }
        nextChangeId = qMax(nextChangeId, changeId + 1);
    }

    const Player& currentPlayer = core.getPlayers()[playerIndex];
    const QString playerId = QString::fromStdString(currentPlayer.idNumber);
    int activeContractRow = -1;
    for (int row = 0; row < contracts.rows.size(); ++row) {
        const QStringList& contract = contracts.rows[row];
        const bool active = contract[6] == "1"
            || contract[6].compare("true", Qt::CaseInsensitive) == 0;
        if (contract[1] != playerId || !active) continue;
        if (activeContractRow >= 0) {
            if (errorMessage) {
                *errorMessage = "El jugador tiene más de un contrato activo; "
                                "corrige los contratos antes de cambiar el rol.";
            }
            return false;
        }
        activeContractRow = row;
    }

    const QString oldRole = activeContractRow >= 0
        ? contracts.rows[activeContractRow][3]
        : QString::fromStdString(currentPlayer.role);
    CsvTableData previousChanges = changes;
    const CsvTableData previousContracts = contracts;
    changes.rows.append({
        QString::number(nextChangeId),
        playerId,
        oldRole,
        newRole,
        QDateTime::currentDateTime().toString(Qt::ISODate),
        action,
        referenceChangeId > 0 ? QString::number(referenceChangeId) : QString{}
    });

    if (activeContractRow >= 0) {
        contracts.rows[activeContractRow][3] = newRole;
        if (!core.saveCsvTable(contractsView->filePath, contracts, errorMessage)) {
            return false;
        }
    }

    if (!core.saveCsvTable(changesView->filePath, changes, errorMessage)) {
        if (activeContractRow >= 0) {
            QString rollbackError;
            if (!core.saveCsvTable(contractsView->filePath, previousContracts,
                                   &rollbackError)
                && errorMessage) {
                *errorMessage += QStringLiteral(
                    " Además, no se pudo revertir el contrato: %1").arg(rollbackError);
            }
        }
        return false;
    }

    Player updatedPlayer = currentPlayer;
    updatedPlayer.role = newRole.toStdString();
    if (!core.updatePlayer(playerIndex, updatedPlayer, playersCsvPath, errorMessage)) {
        QString rollbackErrors;
        QString rollbackError;
        if (!core.saveCsvTable(changesView->filePath, previousChanges, &rollbackError)) {
            rollbackErrors += QStringLiteral(" auditoría: %1").arg(rollbackError);
        }
        if (activeContractRow >= 0
            && !core.saveCsvTable(contractsView->filePath, previousContracts,
                                  &rollbackError)) {
            rollbackErrors += QStringLiteral(" contrato: %1").arg(rollbackError);
        }
        if (!rollbackErrors.isEmpty() && errorMessage) {
            *errorMessage += QStringLiteral(" Además, no se pudieron revertir:%1")
                                 .arg(rollbackErrors);
        }
        return false;
    }
    if (activeContractRow >= 0) refreshCsvTable(*contractsView);
    refreshSummaryReports();
    return true;
}

void MainWindow::undoLastRoleChange()
{
    CsvTableView* changesView = findCsvTable("state_player_changes.csv");
    if (!changesView) {
        QMessageBox::warning(this, "No se pudo deshacer",
                             "No se encontró la tabla de auditoría de roles.");
        return;
    }

    CsvTableData changes;
    QString error;
    if (!core.loadCsvTable(changesView->filePath, changesView->csvHeaders,
                           &changes, &error)) {
        QMessageBox::warning(this, "No se pudo deshacer", error);
        return;
    }

    QSet<int> undoneChanges;
    for (const QStringList& row : changes.rows) {
        if (row[5] != "DESHACER") continue;
        bool referenceValid = false;
        const int referenceId = row[6].toInt(&referenceValid);
        if (!referenceValid) {
            QMessageBox::warning(
                this, "Auditoría inválida",
                "La auditoría contiene una referencia de deshacer inválida.");
            return;
        }
        undoneChanges.insert(referenceId);
    }

    int targetRow = -1;
    int targetChangeId = -1;
    for (int rowIndex = changes.rows.size() - 1; rowIndex >= 0; --rowIndex) {
        const QStringList& row = changes.rows[rowIndex];
        if (row[5] != "CAMBIO") continue;
        bool idValid = false;
        const int changeId = row[0].toInt(&idValid);
        if (!idValid) {
            QMessageBox::warning(this, "Auditoría inválida",
                                 "La auditoría contiene un ID de cambio inválido.");
            return;
        }
        if (!undoneChanges.contains(changeId)) {
            targetRow = rowIndex;
            targetChangeId = changeId;
            break;
        }
    }

    if (targetRow < 0) {
        QMessageBox::information(this, "Sin cambios pendientes",
                                 "No hay cambios de rol que se puedan deshacer.");
        return;
    }

    const QStringList& target = changes.rows[targetRow];
    int playerIndex = -1;
    for (int i = 0; i < core.getPlayers().size(); ++i) {
        if (QString::fromStdString(core.getPlayers()[i].idNumber) == target[1]) {
            playerIndex = i;
            break;
        }
    }
    if (playerIndex < 0) {
        QMessageBox::warning(this, "No se pudo deshacer",
                             "El jugador asociado al cambio ya no existe.");
        return;
    }

    const QString currentRole = QString::fromStdString(core.getPlayers()[playerIndex].role);
    if (currentRole != target[3]) {
        QMessageBox::warning(
            this, "No se pudo deshacer",
            "El rol actual ya no coincide con el cambio más reciente; no se modificó.");
        return;
    }

    if (QMessageBox::question(
            this, "Deshacer cambio de rol",
            QString("¿Restaurar el rol %1 a %2?")
                .arg(target[2], QString::fromStdString(core.getPlayers()[playerIndex].name)),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    if (!savePlayerRoleChange(playerIndex, target[2], "DESHACER",
                              targetChangeId, &error)) {
        QMessageBox::warning(this, "No se pudo deshacer", error);
        return;
    }
    refreshPlayersTable();
    ui->tablePlayers->selectRow(playerIndex);
    refreshCsvTable(*changesView);
    ui->statusbar->showMessage("Último cambio de rol deshecho y registrado.");
}

void MainWindow::addCsvDataTabs()
{
    const QString csvDirectory = findCsvDirectory();
    const QString changesFilePath =
        QDir(csvDirectory).filePath("state_player_changes.csv");
    const QStringList currentChangeHeaders = {
        "change_id", "player_id", "old_role", "new_role", "change_date",
        "action", "reference_change_id"
    };
    if (QFile::exists(changesFilePath)) {
        CsvTableData currentChanges;
        QString migrationError;
        if (!core.loadCsvTable(changesFilePath, currentChangeHeaders,
                               &currentChanges, &migrationError)) {
            const QStringList legacyHeaders = {
                "change_id", "player_id", "old_role", "new_role", "change_date"
            };
            CsvTableData legacyChanges;
            if (!core.loadCsvTable(changesFilePath, legacyHeaders,
                                   &legacyChanges, &migrationError)) {
                QMessageBox::warning(this, "No se pudo cargar la auditoría",
                                     migrationError);
                return;
            }
            for (QStringList& row : legacyChanges.rows) {
                row.append(QStringLiteral("CAMBIO"));
                row.append(QString{});
            }
            legacyChanges.headers = currentChangeHeaders;
            if (!core.saveCsvTable(changesFilePath, legacyChanges, &migrationError)) {
                QMessageBox::warning(this, "No se pudo actualizar la auditoría",
                                     migrationError);
                return;
            }
        }
    }

    csvTableViews = {
        {"Equipos", "state_teams.csv",
         {"team_id", "team_name", "league"}, {"ID equipo", "Equipo", "Liga"}},
        {"Campeones", "state_champions.csv",
         {"champion_id", "champion_name"}, {"ID campeón", "Campeón"}},
        {"Contratos", "state_contracts.csv",
         {"contract_id", "hired_id", "hiring_team_id", "role",
          "start_date", "end_date", "status"},
         {"ID contrato", "Jugador", "Equipo", "Rol",
          "Inicio", "Fin", "Activo"}},
        {"Maestrías", "state_masteries.csv",
         {"mastery_id", "lol_user_id", "champion_id", "champion_name",
          "mastery_level", "champion_points"},
         {"ID", "Invocador", "ID campeón", "Campeón", "Nivel", "Puntos"}},
        {"Partidas", "state_matches.csv",
         {"match_id", "game_mode", "game_duration", "game_creation"},
         {"ID partida", "Modo", "Duración (s)", "Fecha de creación"}},
        {"Estadísticas", "state_stats.csv",
         {"stats_id", "match_id", "lol_user_id", "champion_id", "kills",
          "deaths", "assists", "win", "gold", "damage", "vision",
          "performance_status"},
         {"ID", "Partida", "Invocador", "Campeón", "Kills", "Deaths",
          "Assists", "Victoria", "Oro", "Daño", "Visión", "Rendimiento"}},
        {"Cambios de rol", "state_player_changes.csv",
         currentChangeHeaders,
         {"ID", "ID jugador", "Rol anterior", "Rol nuevo", "Fecha", "Acción",
          "Cambio relacionado"}}
    };

    for (int index = 0; index < csvTableViews.size(); ++index) {
        CsvTableView& view = csvTableViews[index];
        view.filePath = QDir(csvDirectory).filePath(view.fileName);
        if (view.fileName == "state_champions.csv"
            || view.fileName == "state_matches.csv") {
            CsvTableData data;
            QString error;
            if (!core.loadCsvTable(view.filePath, view.csvHeaders, &data, &error)) {
                QMessageBox::warning(this, "No se pudo preparar la tabla",
                                     QString("%1: %2").arg(view.fileName, error));
                continue;
            }
            continue;
        }

        auto* page = new QWidget(ui->dataTabs);
        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(8, 6, 8, 8);
        layout->setSpacing(6);
        view.status = new QLabel(page);
        view.status->setWordWrap(false);
        view.status->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        view.status->setMaximumHeight(view.status->sizeHint().height());
        layout->addWidget(view.status);
        if (view.fileName == "state_contracts.csv") {
            auto* actions = new QHBoxLayout;
            view.addRowButton = new QPushButton("Agregar contrato", page);
            view.editRowButton = new QPushButton("Editar seleccionado", page);
            view.removeRowButton = new QPushButton("Eliminar seleccionado", page);
            actions->addWidget(view.addRowButton);
            actions->addWidget(view.editRowButton);
            actions->addWidget(view.removeRowButton);
            actions->addStretch();
            layout->addLayout(actions);
            connect(view.addRowButton, &QPushButton::clicked,
                    this, &MainWindow::addContract);
            connect(view.editRowButton, &QPushButton::clicked,
                    this, &MainWindow::editSelectedContract);
            connect(view.removeRowButton, &QPushButton::clicked,
                    this, &MainWindow::removeSelectedContract);
        }
        if (view.fileName == "state_stats.csv") {
            auto* downloadActions = new QHBoxLayout;
            downloadActions->setSpacing(6);
            view.downloadStatisticsButton = new QPushButton(
                "Descargar maestrías y estadísticas de Riot", page);
            connect(view.downloadStatisticsButton, &QPushButton::clicked,
                    this, &MainWindow::downloadRiotStatistics);
            view.downloadChampionImagesButton = new QPushButton(
                "Descargar imágenes de campeones", page);
            connect(view.downloadChampionImagesButton, &QPushButton::clicked,
                    this, &MainWindow::downloadChampionImages);
            downloadActions->addWidget(view.downloadStatisticsButton);
            downloadActions->addWidget(view.downloadChampionImagesButton);
            layout->addLayout(downloadActions);
            championImagesStatus = new QLabel(page);
            championImagesStatus->setWordWrap(false);
            championImagesStatus->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            championImagesStatus->setMaximumHeight(
                championImagesStatus->sizeHint().height());
            layout->addWidget(championImagesStatus);
        }
        view.table = new QTableWidget(page);
        view.table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        view.table->setAlternatingRowColors(true);
        view.table->setSelectionBehavior(QAbstractItemView::SelectRows);
        view.table->verticalHeader()->setVisible(false);
        view.table->setColumnCount(view.visibleHeaders.size());
        view.table->setHorizontalHeaderLabels(view.visibleHeaders);
        view.table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        if (view.fileName == "state_stats.csv"
            || view.fileName == "state_masteries.csv") {
            const int summonerColumn =
                view.fileName == "state_stats.csv" ? 2 : 1;
            view.table->horizontalHeader()->setSectionResizeMode(
                summonerColumn, QHeaderView::Fixed);
        }
        if (view.fileName == "state_stats.csv") {
            auto* splitter = new QSplitter(Qt::Horizontal, page);
            splitter->addWidget(view.table);
            auto* previewPanel = new QWidget(splitter);
            auto* previewLayout = new QVBoxLayout(previewPanel);
            championPreviewName = new QLabel("Vista previa del campeón", previewPanel);
            championPreviewName->setAlignment(Qt::AlignCenter);
            previewLayout->addWidget(championPreviewName);
            championPreviewImage = new QLabel(
                "Selecciona un campeón en la tabla para ver su imagen.", previewPanel);
            championPreviewImage->setAlignment(Qt::AlignCenter);
            championPreviewImage->setWordWrap(true);
            championPreviewImage->setFixedSize(300, 300);
            championPreviewImage->setFrameShape(QFrame::StyledPanel);
            previewLayout->addWidget(championPreviewImage, 0, Qt::AlignCenter);
            previewLayout->addStretch();
            previewPanel->setMinimumWidth(320);
            previewPanel->setMaximumWidth(360);
            splitter->addWidget(previewPanel);
            splitter->setStretchFactor(0, 1);
            splitter->setStretchFactor(1, 0);
            splitter->setSizes({900, 340});
            layout->addWidget(splitter);

            connect(view.table, &QTableWidget::cellClicked, this,
                    [this, table = view.table](int row, int column) {
                if (column != 3) return;
                const QTableWidgetItem* championItem = table->item(row, column);
                if (!championItem) return;
                bool keyValid = false;
                const int championKey =
                    championItem->data(Qt::UserRole).toInt(&keyValid);
                if (!keyValid) return;
                showChampionImage(championKey, championItem->text(),
                                  championPreviewName, championPreviewImage);
            });
        }
        if (view.fileName == "state_masteries.csv") {
            auto* splitter = new QSplitter(Qt::Horizontal, page);
            splitter->addWidget(view.table);
            auto* previewPanel = new QWidget(splitter);
            auto* previewLayout = new QVBoxLayout(previewPanel);
            masteryPreviewName = new QLabel("Vista previa del campeón", previewPanel);
            masteryPreviewName->setAlignment(Qt::AlignCenter);
            previewLayout->addWidget(masteryPreviewName);
            masteryPreviewImage = new QLabel(
                "Selecciona un campeón en la tabla para ver su imagen.", previewPanel);
            masteryPreviewImage->setAlignment(Qt::AlignCenter);
            masteryPreviewImage->setWordWrap(true);
            masteryPreviewImage->setFixedSize(300, 300);
            masteryPreviewImage->setFrameShape(QFrame::StyledPanel);
            previewLayout->addWidget(masteryPreviewImage, 0, Qt::AlignCenter);
            previewLayout->addStretch();
            previewPanel->setMinimumWidth(320);
            previewPanel->setMaximumWidth(360);
            splitter->addWidget(previewPanel);
            splitter->setStretchFactor(0, 1);
            splitter->setStretchFactor(1, 0);
            splitter->setSizes({900, 340});
            layout->addWidget(splitter);

            connect(view.table, &QTableWidget::cellClicked, this,
                    [this, table = view.table](int row, int column) {
                if (column != 3) return;
                const QTableWidgetItem* championItem = table->item(row, column);
                if (!championItem) return;
                bool keyValid = false;
                const int championKey =
                    championItem->data(Qt::UserRole).toInt(&keyValid);
                if (!keyValid) return;
                showChampionImage(championKey, championItem->text(),
                                  masteryPreviewName, masteryPreviewImage);
            });
        }
        else {
            if (view.fileName != "state_stats.csv") layout->addWidget(view.table);
        }
        ui->dataTabs->addTab(page, view.title);
        refreshCsvTable(view);
    }
}

void MainWindow::addSummaryReportTabs()
{
    const auto addReport = [this](const QString& title, QTableWidget** tableOut) {
        auto* page = new QWidget(ui->dataTabs);
        auto* layout = new QVBoxLayout(page);
        auto* table = new QTableWidget(page);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->verticalHeader()->setVisible(false);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        layout->addWidget(table);
        ui->dataTabs->addTab(page, title);
        *tableOut = table;
    };

    addReport("Jugadores registrados", &registeredPlayersTable);
    addReport("Bajo rendimiento", &badPerformersTable);
    addReport("Agentes libres", &freeAgentsTable);

    auto* recentPage = new QWidget(ui->dataTabs);
    auto* recentLayout = new QVBoxLayout(recentPage);
    auto* selectorLayout = new QHBoxLayout;
    selectorLayout->addWidget(new QLabel("Cuenta:", recentPage));
    recentAccountCombo = new QComboBox(recentPage);
    selectorLayout->addWidget(recentAccountCombo);
    selectorLayout->addStretch();
    recentLayout->addLayout(selectorLayout);
    recentMatchesTable = new QTableWidget(recentPage);
    recentMatchesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentMatchesTable->setAlternatingRowColors(true);
    recentMatchesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentMatchesTable->verticalHeader()->setVisible(false);
    recentMatchesTable->setColumnCount(6);
    recentMatchesTable->setHorizontalHeaderLabels(
        {"Resultado", "Partida", "Campeón", "K", "D", "A"});
    recentMatchesTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::ResizeToContents);
    recentLayout->addWidget(recentMatchesTable);
    ui->dataTabs->addTab(recentPage, "Últimas cinco partidas");
    connect(recentAccountCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, [this]() { refreshSummaryReports(); });
    refreshSummaryReports();
}

void MainWindow::refreshSummaryReports()
{
    CsvTableView* teamsView = findCsvTable("state_teams.csv");
    CsvTableView* contractsView = findCsvTable("state_contracts.csv");
    CsvTableView* matchesView = findCsvTable("state_matches.csv");
    CsvTableView* statsView = findCsvTable("state_stats.csv");
    CsvTableView* championsView = findCsvTable("state_champions.csv");
    if (!teamsView || !contractsView || !matchesView || !statsView
        || !championsView || !registeredPlayersTable || !badPerformersTable
        || !freeAgentsTable || !recentAccountCombo || !recentMatchesTable) {
        return;
    }

    CsvTableData teams;
    CsvTableData contracts;
    CsvTableData matches;
    CsvTableData stats;
    CsvTableData champions;
    QString error;
    if (!core.loadCsvTable(teamsView->filePath, teamsView->csvHeaders, &teams, &error)
        || !core.loadCsvTable(contractsView->filePath, contractsView->csvHeaders,
                              &contracts, &error)
        || !core.loadCsvTable(matchesView->filePath, matchesView->csvHeaders,
                              &matches, &error)
        || !core.loadCsvTable(statsView->filePath, statsView->csvHeaders,
                              &stats, &error)
        || !core.loadCsvTable(championsView->filePath, championsView->csvHeaders,
                              &champions, &error)) {
        ui->statusbar->showMessage(
            QString("No se pudieron actualizar los informes: %1").arg(error), 10000);
        return;
    }

    QHash<int, QString> teamNames;
    for (const QStringList& row : teams.rows) teamNames.insert(row[0].toInt(), row[1]);
    QHash<QString, QStringList> activeContracts;
    for (const QStringList& row : contracts.rows) {
        const bool active = row[6] == "1"
            || row[6].compare("true", Qt::CaseInsensitive) == 0;
        if (active && !activeContracts.contains(row[1])) {
            activeContracts.insert(row[1], row);
        }
    }
    QHash<QString, const LoLUser*> usersByPlayer;
    QHash<int, const LoLUser*> usersById;
    for (const LoLUser& user : core.getLolUsers()) {
        usersByPlayer.insert(QString::fromStdString(user.playerIdNumber), &user);
        usersById.insert(user.userId, &user);
    }

    registeredPlayersTable->clear();
    registeredPlayersTable->setColumnCount(7);
    registeredPlayersTable->setHorizontalHeaderLabels(
        {"ID", "Nombre", "Riot ID", "Rango", "Winrate", "Rol", "Equipo"});
    QVector<QStringList> registeredRows;
    for (const Player& player : core.getPlayers()) {
        const QString playerId = QString::fromStdString(player.idNumber);
        const auto contractIt = activeContracts.constFind(playerId);
        const auto userIt = usersByPlayer.constFind(playerId);
        if (contractIt == activeContracts.cend() || userIt == usersByPlayer.cend()) continue;
        const QString teamName = teamNames.value((*contractIt)[2].toInt());
        if (teamName.isEmpty()) continue;
        const LoLUser& user = **userIt;
        QString riotId = QString::fromStdString(user.riotId);
        const QString tagline = QString::fromStdString(user.tagline);
        if (!tagline.isEmpty()) riotId += "#" + tagline;
        registeredRows.append({
            playerId,
            QString("%1 %2").arg(QString::fromStdString(player.name),
                                  QString::fromStdString(player.lastName)),
            riotId,
            QString::fromStdString(user.soloqRank),
            QString::number(user.winrate, 'f', 2) + "%",
            QString::fromStdString(player.role),
            teamName
        });
    }
    registeredPlayersTable->setRowCount(registeredRows.size());
    for (int row = 0; row < registeredRows.size(); ++row) {
        for (int column = 0; column < registeredRows[row].size(); ++column) {
            auto* item = new QTableWidgetItem(registeredRows[row][column]);
            if (column == 6) {
                const QIcon logo = teamLogoIcon(registeredRows[row][column]);
                if (!logo.isNull()) item->setIcon(logo);
            }
            registeredPlayersTable->setItem(row, column, item);
        }
    }

    QHash<int, QString> championNames;
    for (const QStringList& row : champions.rows) {
        championNames.insert(row[0].toInt(), row[1]);
    }
    QHash<QString, QStringList> matchData;
    for (const QStringList& row : matches.rows) matchData.insert(row[0], row);

    struct BadPerformerRow {
        QStringList values;
        int deaths = 0;
    };
    QVector<BadPerformerRow> badRows;
    for (const QStringList& row : stats.rows) {
        if (row[11] != "Requiere Revision") continue;
        const auto matchIt = matchData.constFind(row[1]);
        const auto userIt = usersById.constFind(row[2].toInt());
        if (matchIt == matchData.cend() || (*matchIt)[1] != "CLASSIC"
            || userIt == usersById.cend()) {
            continue;
        }
        QString riotId = QString::fromStdString((*userIt)->riotId);
        const QString tagline = QString::fromStdString((*userIt)->tagline);
        if (!tagline.isEmpty()) riotId += "#" + tagline;
        badRows.append({
            {riotId, championNames.value(row[3].toInt(), row[3]),
             row[4], row[5], row[6], row[11], row[1]},
            row[5].toInt()
        });
    }
    std::stable_sort(badRows.begin(), badRows.end(),
                     [](const BadPerformerRow& left, const BadPerformerRow& right) {
        return left.deaths > right.deaths;
    });
    badPerformersTable->clear();
    badPerformersTable->setColumnCount(7);
    badPerformersTable->setHorizontalHeaderLabels(
        {"Riot ID", "Campeón", "K", "D", "A", "Estado", "Partida"});
    badPerformersTable->setRowCount(badRows.size());
    for (int row = 0; row < badRows.size(); ++row) {
        for (int column = 0; column < badRows[row].values.size(); ++column) {
            badPerformersTable->setItem(
                row, column, new QTableWidgetItem(badRows[row].values[column]));
        }
    }

    struct FreeAgentRow {
        QStringList values;
        double winrate = 0;
    };
    QVector<FreeAgentRow> freeRows;
    for (const Player& player : core.getPlayers()) {
        const QString playerId = QString::fromStdString(player.idNumber);
        if (activeContracts.contains(playerId)) continue;
        const auto userIt = usersByPlayer.constFind(playerId);
        const double winrate = userIt == usersByPlayer.cend() ? 0.0 : (*userIt)->winrate;
        QString lastRole = QString::fromStdString(player.role);
        if (lastRole.isEmpty()) lastRole = "-";
        freeRows.append({
            {playerId,
             QString("%1 %2").arg(QString::fromStdString(player.name),
                                   QString::fromStdString(player.lastName)),
             QString::fromStdString(player.country), lastRole,
             QString::number(winrate, 'f', 2) + "%"},
            winrate
        });
    }
    std::stable_sort(freeRows.begin(), freeRows.end(),
                     [](const FreeAgentRow& left, const FreeAgentRow& right) {
        return left.winrate > right.winrate;
    });
    freeAgentsTable->clear();
    freeAgentsTable->setColumnCount(5);
    freeAgentsTable->setHorizontalHeaderLabels(
        {"ID", "Nombre", "País", "Rol", "Winrate"});
    freeAgentsTable->setRowCount(freeRows.size());
    for (int row = 0; row < freeRows.size(); ++row) {
        for (int column = 0; column < freeRows[row].values.size(); ++column) {
            freeAgentsTable->setItem(
                row, column, new QTableWidgetItem(freeRows[row].values[column]));
        }
    }

    const QString selectedPlayerId = recentAccountCombo->currentData().toString();
    recentAccountCombo->blockSignals(true);
    recentAccountCombo->clear();
    for (const LoLUser& user : core.getLolUsers()) {
        const QString playerId = QString::fromStdString(user.playerIdNumber);
        QString label = QString::fromStdString(user.riotId);
        const QString tagline = QString::fromStdString(user.tagline);
        if (!tagline.isEmpty()) label += "#" + tagline;
        recentAccountCombo->addItem(label, playerId);
    }
    const int selectedIndex = recentAccountCombo->findData(selectedPlayerId);
    if (selectedIndex >= 0) recentAccountCombo->setCurrentIndex(selectedIndex);
    recentAccountCombo->blockSignals(false);

    recentMatchesTable->clearContents();
    recentMatchesTable->setRowCount(0);
    recentMatchesTable->setHorizontalHeaderLabels(
        {"Resultado", "Partida", "Campeón", "K", "D", "A"});
    if (recentAccountCombo->currentIndex() < 0) return;

    const QString recentPlayerId = recentAccountCombo->currentData().toString();
    const auto recentUserIt = usersByPlayer.constFind(recentPlayerId);
    if (recentUserIt == usersByPlayer.cend()) return;
    QVector<QStringList> recentRows;
    for (const QStringList& row : stats.rows) {
        if (row[2].toInt() != (*recentUserIt)->userId) continue;
        const auto matchIt = matchData.constFind(row[1]);
        const QString creation = matchIt == matchData.cend() ? QString{} : (*matchIt)[3];
        recentRows.append({
            row[7] == "1" ? "Victoria" : "Derrota",
            row[1],
            championNames.value(row[3].toInt(), row[3]),
            row[4],
            row[5],
            row[6],
            creation
        });
    }
    std::stable_sort(recentRows.begin(), recentRows.end(),
                     [](const QStringList& left, const QStringList& right) {
        return left[6].toLongLong() > right[6].toLongLong();
    });
    if (recentRows.size() > 5) recentRows.resize(5);
    recentMatchesTable->setRowCount(recentRows.size());
    for (int row = 0; row < recentRows.size(); ++row) {
        for (int column = 0; column < 6; ++column) {
            recentMatchesTable->setItem(
                row, column, new QTableWidgetItem(recentRows[row][column]));
        }
    }
}

void MainWindow::refreshCsvTable(CsvTableView& view)
{
    CsvTableData data;
    QString error;
    if (!core.loadCsvTable(view.filePath, view.csvHeaders, &data, &error)) {
        if (view.table) view.table->setRowCount(0);
        if (view.status) {
            view.status->setText(
                QString("No se pudo cargar %1: %2").arg(view.fileName, error));
        }
        return;
    }
    if (!view.table) return;

    if (view.fileName == "state_masteries.csv") {
        QStringList accountOrder;
        QHash<QString, QVector<QStringList>> masteriesByAccount;
        for (const QStringList& row : data.rows) {
            const QString accountId = row[1];
            if (!masteriesByAccount.contains(accountId)) {
                accountOrder.append(accountId);
            }
            masteriesByAccount[accountId].append(row);
        }

        data.rows.clear();
        for (const QString& accountId : accountOrder) {
            QVector<QStringList>& masteries = masteriesByAccount[accountId];
            std::stable_sort(
                masteries.begin(), masteries.end(),
                [](const QStringList& left, const QStringList& right) {
                    return left[5].toLongLong() > right[5].toLongLong();
                });
            const int visibleCount = qMin(5, masteries.size());
            for (int index = 0; index < visibleCount; ++index) {
                data.rows.append(masteries[index]);
            }
        }
    }

    QHash<int, QString> summonerNames;
    QHash<int, QString> championNames;
    QHash<QString, QString> playerNames;
    QHash<QString, QString> teamNames;
    if (view.fileName == "state_contracts.csv") {
        for (const Player& player : core.getPlayers()) {
            const QString playerId = QString::fromStdString(player.idNumber);
            const QString playerName =
                QString("%1 %2")
                    .arg(QString::fromStdString(player.name),
                         QString::fromStdString(player.lastName))
                    .trimmed();
            playerNames.insert(playerId, playerName);
        }

        if (CsvTableView* teamsView = findCsvTable("state_teams.csv")) {
            CsvTableData teams;
            if (!core.loadCsvTable(teamsView->filePath, teamsView->csvHeaders,
                                   &teams, &error)) {
                if (view.status) {
                    view.status->setText(
                        QString("No se pudieron cargar los nombres de equipos: %1")
                            .arg(error));
                }
                return;
            }
            for (const QStringList& team : teams.rows) {
                teamNames.insert(team[0], team[1]);
            }
        }
        teamNames.insert("0", "Sin equipo");
    }
    if (view.fileName == "state_stats.csv"
        || view.fileName == "state_masteries.csv") {
        for (const LoLUser& user : core.getLolUsers()) {
            QString summoner = QString::fromStdString(user.riotId);
            const QString tagline = QString::fromStdString(user.tagline);
            if (!tagline.isEmpty()) summoner += "#" + tagline;
            summonerNames.insert(user.userId, summoner);
        }

        if (CsvTableView* championsView = findCsvTable("state_champions.csv")) {
            CsvTableData champions;
            if (core.loadCsvTable(championsView->filePath,
                                  championsView->csvHeaders, &champions, &error)) {
                for (const QStringList& champion : champions.rows) {
                    championNames.insert(champion[0].toInt(), champion[1]);
                }
            }
        }
    }

    view.table->setRowCount(data.rows.size());
    view.table->clearSpans();
    if (view.fileName == "state_stats.csv"
        || view.fileName == "state_masteries.csv") {
        view.table->setWordWrap(false);
        int summonerColumnWidth = 0;
        const QFontMetrics metrics(view.table->font());
        for (const QString& summoner : summonerNames) {
            summonerColumnWidth =
                qMax(summonerColumnWidth, metrics.horizontalAdvance(summoner));
        }
        const int summonerColumn =
            view.fileName == "state_stats.csv" ? 2 : 1;
        view.table->setColumnWidth(summonerColumn,
                                   qMax(160, summonerColumnWidth + 24));
    }
    QVector<int> summonerGroupStarts;
    if (view.fileName == "state_stats.csv"
        || view.fileName == "state_masteries.csv") {
        summonerGroupStarts.resize(data.rows.size());
        const int accountColumn =
            view.fileName == "state_stats.csv" ? 2 : 1;
        const int summonerColumn = accountColumn;
        for (int row = 0; row < data.rows.size();) {
            const QString accountId = data.rows[row][accountColumn];
            int groupEnd = row + 1;
            while (groupEnd < data.rows.size()
                   && data.rows[groupEnd][accountColumn] == accountId) {
                ++groupEnd;
            }
            for (int groupedRow = row; groupedRow < groupEnd; ++groupedRow) {
                summonerGroupStarts[groupedRow] = row;
            }
            if (groupEnd - row > 1) {
                view.table->setSpan(row, summonerColumn, groupEnd - row, 1);
            }
            row = groupEnd;
        }
    }

    for (int row = 0; row < data.rows.size(); ++row) {
        for (int column = 0; column < data.rows[row].size(); ++column) {
            const int summonerColumn =
                view.fileName == "state_stats.csv" ? 2 : 1;
            if ((view.fileName == "state_stats.csv"
                 || view.fileName == "state_masteries.csv")
                && column == summonerColumn
                && summonerGroupStarts[row] != row) {
                continue;
            }
            QString text = data.rows[row][column];
            if (view.fileName == "state_masteries.csv" && column == 0) {
                text = QString::number(row + 1);
            } else if ((view.fileName == "state_stats.csv" && column == 2)
                || (view.fileName == "state_masteries.csv" && column == 1)) {
                const QString summoner = summonerNames.value(text.toInt(), text);
                text = summoner;
            } else if (view.fileName == "state_stats.csv" && column == 3) {
                text = championNames.value(text.toInt(), text);
            } else if (view.fileName == "state_masteries.csv" && column == 3) {
                bool championKeyValid = false;
                const int championKey =
                    data.rows[row][2].toInt(&championKeyValid);
                if (championKeyValid) {
                    text = championNames.value(championKey, text);
                }
            } else if (view.fileName == "state_contracts.csv" && column == 1) {
                text = playerNames.value(text, text);
            } else if (view.fileName == "state_contracts.csv" && column == 2) {
                text = teamNames.value(text, text);
            }
            auto* item = new QTableWidgetItem(text);
            if ((view.fileName == "state_stats.csv" && column == 2)
                || (view.fileName == "state_masteries.csv" && column == 1)) {
                const QString accountId = data.rows[row][column];
                item->setToolTip(summonerNames.value(accountId.toInt(), accountId));
                item->setTextAlignment(Qt::AlignCenter);
            }
            if ((view.fileName == "state_stats.csv" && column == 3)
                || (view.fileName == "state_masteries.csv" && column == 3)) {
                bool championKeyValid = false;
                const int keyColumn =
                    view.fileName == "state_stats.csv" ? column : 2;
                const int championKey =
                    data.rows[row][keyColumn].toInt(&championKeyValid);
                if (championKeyValid) {
                    item->setData(Qt::UserRole, championKey);
                    item->setToolTip("Haz clic para mostrar la imagen del campeón.");
                }
            }
            if (view.fileName == "state_teams.csv" && column == 1) {
                const QIcon logo = teamLogoIcon(text);
                if (!logo.isNull()) item->setIcon(logo);
            }
            view.table->setItem(row, column, item);
        }
    }
    view.status->setText(
        data.rows.isEmpty()
            ? QString("%1: sin registros.").arg(view.title)
            : QString("%1 registros cargados desde %2.")
                  .arg(data.rows.size()).arg(view.fileName));
}

void MainWindow::downloadChampionCatalog()
{
    CsvTableView* championsView = nullptr;
    for (CsvTableView& view : csvTableViews) {
        if (view.fileName == "state_champions.csv") {
            championsView = &view;
            break;
        }
    }
    if (!championsView) return;
    if (downloadingChampionCatalogInBackground) return;

    downloadingChampionCatalogInBackground = true;
    const auto report = [this, championsView](const QString& message) {
        downloadingChampionCatalogInBackground = false;
        if (championsView->status) {
            championsView->status->setText(message);
        } else {
            ui->statusbar->showMessage(message, 10000);
        }
        if (championsView->downloadButton) {
            championsView->downloadButton->setEnabled(true);
        }
    };
    if (championsView->downloadButton) {
        championsView->downloadButton->setEnabled(false);
    }
    if (championsView->status) {
        championsView->status->setText("Consultando la versión actual de Data Dragon...");
    } else {
        ui->statusbar->showMessage("Consultando la versión actual de Data Dragon...");
    }
    auto* versionsReply = networkManager.get(
        QNetworkRequest(QUrl("https://ddragon.leagueoflegends.com/api/versions.json")));
    connect(versionsReply, &QNetworkReply::finished, this,
            [this, versionsReply, championsView, report]() {
        const QByteArray versionsBody = versionsReply->readAll();
        if (versionsReply->error() != QNetworkReply::NoError) {
            report(QString("No se pudo consultar Data Dragon: %1")
                       .arg(versionsReply->errorString()));
            versionsReply->deleteLater();
            return;
        }

        QJsonParseError versionsParseError;
        const QJsonDocument versionsDocument =
            QJsonDocument::fromJson(versionsBody, &versionsParseError);
        if (versionsParseError.error != QJsonParseError::NoError
            || !versionsDocument.isArray() || versionsDocument.array().isEmpty()) {
            report("Data Dragon devolvió una lista de versiones inválida.");
            versionsReply->deleteLater();
            return;
        }
        const QString version = versionsDocument.array().first().toString();
        versionsReply->deleteLater();

        const QUrl championsUrl(
            QString("https://ddragon.leagueoflegends.com/cdn/%1/data/es_MX/champion.json")
                .arg(version));
        const QString downloadMessage =
            QString("Descargando campeones de Data Dragon %1...").arg(version);
        if (championsView->status) {
            championsView->status->setText(downloadMessage);
        } else {
            ui->statusbar->showMessage(downloadMessage);
        }
        auto* championsReply = networkManager.get(QNetworkRequest(championsUrl));
        connect(championsReply, &QNetworkReply::finished, this,
                [this, championsReply, championsView, version, report]() {
            const QByteArray body = championsReply->readAll();
            if (championsReply->error() != QNetworkReply::NoError) {
                report(QString("No se pudo descargar el catálogo: %1")
                           .arg(championsReply->errorString()));
                championsReply->deleteLater();
                return;
            }

            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                report("Data Dragon devolvió un catálogo JSON inválido.");
                championsReply->deleteLater();
                return;
            }

            const QJsonObject championObjects =
                document.object().value("data").toObject();
            CsvTableData data;
            data.headers = championsView->csvHeaders;
            for (const QJsonValue& value : championObjects) {
                const QJsonObject champion = value.toObject();
                const QString id = champion.value("key").toString();
                const QString name = champion.value("name").toString();
                if (!id.isEmpty() && !name.isEmpty()) data.rows.append({id, name});
            }
            if (data.rows.isEmpty()) {
                report("El catálogo recibido no contiene campeones válidos.");
                championsReply->deleteLater();
                return;
            }
            std::sort(data.rows.begin(), data.rows.end(),
                      [](const QStringList& first, const QStringList& second) {
                return first[0].toInt() < second[0].toInt();
            });

            QString error;
            if (!core.saveCsvTable(championsView->filePath, data, &error)) {
                report(QString("No se pudo guardar el catálogo: %1").arg(error));
                championsReply->deleteLater();
                return;
            }

            refreshCsvTable(*championsView);
            report(QString("%1 campeones cargados desde Data Dragon %2 y guardados en CSV.")
                       .arg(data.rows.size()).arg(version));
            if (CsvTableView* statsView = findCsvTable("state_stats.csv")) {
                refreshCsvTable(*statsView);
            }
            championsReply->deleteLater();
        });
    });
}

void MainWindow::downloadChampionImages()
{
    CsvTableView* statsView = findCsvTable("state_stats.csv");
    if (!statsView || !statsView->downloadChampionImagesButton
        || !championImagesStatus) {
        return;
    }
    if (!championImageQueue.isEmpty()) return;

    championImageErrors.clear();
    championImageProcessed = 0;
    championImageSkipped = 0;
    championImageDownloaded = 0;
    championImageDownloadTotal = 0;
    championImageDirectory = QDir(findCsvDirectory()).filePath(
        "../assets/champion_images");
    if (!QDir().mkpath(championImageDirectory)) {
        const QString message =
            QString("No se pudo crear la carpeta de imágenes: %1")
                .arg(championImageDirectory);
        championImagesStatus->setText(message);
        QMessageBox::warning(this, "No se pudieron descargar las imágenes", message);
        return;
    }

    statsView->downloadChampionImagesButton->setEnabled(false);
    championImagesStatus->setText("Consultando la versión y el catálogo de Data Dragon...");
    auto* versionsReply = networkManager.get(QNetworkRequest(
        QUrl("https://ddragon.leagueoflegends.com/api/versions.json")));
    connect(versionsReply, &QNetworkReply::finished, this,
            [this, versionsReply, statsView]() {
        const QByteArray versionsBody = versionsReply->readAll();
        if (versionsReply->error() != QNetworkReply::NoError) {
            const QString error = versionsReply->errorString();
            versionsReply->deleteLater();
            championImagesStatus->setText(
                QString("No se pudo consultar Data Dragon: %1").arg(error));
            statsView->downloadChampionImagesButton->setEnabled(true);
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument versionsDocument =
            QJsonDocument::fromJson(versionsBody, &parseError);
        if (parseError.error != QJsonParseError::NoError
            || !versionsDocument.isArray() || versionsDocument.array().isEmpty()) {
            versionsReply->deleteLater();
            championImagesStatus->setText(
                "Data Dragon devolvió una lista de versiones inválida.");
            statsView->downloadChampionImagesButton->setEnabled(true);
            return;
        }
        const QString version = versionsDocument.array().first().toString();
        championImageVersion = version;
        versionsReply->deleteLater();

        const QUrl catalogUrl(
            QString("https://ddragon.leagueoflegends.com/cdn/%1/data/es_MX/champion.json")
                .arg(version));
        auto* catalogReply = networkManager.get(QNetworkRequest(catalogUrl));
        connect(catalogReply, &QNetworkReply::finished, this,
                [this, catalogReply, version, statsView]() {
            const QByteArray body = catalogReply->readAll();
            if (catalogReply->error() != QNetworkReply::NoError) {
                const QString error = catalogReply->errorString();
                catalogReply->deleteLater();
                championImagesStatus->setText(
                    QString("No se pudo descargar el catálogo: %1").arg(error));
                statsView->downloadChampionImagesButton->setEnabled(true);
                return;
            }

            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                catalogReply->deleteLater();
                championImagesStatus->setText(
                    "Data Dragon devolvió un catálogo JSON inválido.");
                statsView->downloadChampionImagesButton->setEnabled(true);
                return;
            }

            const QJsonObject championObjects =
                document.object().value("data").toObject();
            QRegularExpression validChampionId(QStringLiteral("^[A-Za-z0-9]+$"));
            for (const QJsonValue& value : championObjects) {
                const QJsonObject champion = value.toObject();
                const QString championId = champion.value("id").toString();
                bool keyValid = false;
                const int championKey =
                    champion.value("key").toString().toInt(&keyValid);
                if (validChampionId.match(championId).hasMatch() && keyValid) {
                    championImageQueue.append(championId);
                    championImageIdsByKey.insert(championKey, championId);
                } else {
                    championImageErrors.append(
                        QString("Se omitió un identificador de campeón inválido: %1")
                            .arg(championId));
                }
            }
            catalogReply->deleteLater();
            if (championImageQueue.isEmpty()) {
                championImagesStatus->setText(
                    "El catálogo recibido no contiene identificadores válidos.");
                statsView->downloadChampionImagesButton->setEnabled(true);
                return;
            }

            championImageDownloadTotal = championImageQueue.size();
            championImagesStatus->setText(
                QString("Descargando imágenes de %1 campeones (Data Dragon %2)...")
                    .arg(championImageDownloadTotal).arg(version));
            processNextChampionImage();
        });
    });
}

void MainWindow::processNextChampionImage()
{
    CsvTableView* statsView = findCsvTable("state_stats.csv");
    if (!statsView || !statsView->downloadChampionImagesButton
        || !championImagesStatus) {
        return;
    }
    if (championImageQueue.isEmpty()) {
        QString summary =
            QString("Imágenes descargadas: %1; ya existentes: %2; errores: %3. Carpeta: %4")
                .arg(championImageDownloaded)
                .arg(championImageSkipped)
                .arg(championImageErrors.size())
                .arg(QDir::toNativeSeparators(championImageDirectory));
        if (!championImageErrors.isEmpty()) {
            summary += QStringLiteral("\n") + championImageErrors.mid(0, 10).join("; ");
            if (championImageErrors.size() > 10) {
                summary += QStringLiteral("; y %1 errores más.")
                               .arg(championImageErrors.size() - 10);
            }
        }
        championImagesStatus->setText(summary);
        statsView->downloadChampionImagesButton->setEnabled(true);
        ui->statusbar->showMessage(
            QString("Descarga de imágenes terminada: %1 de %2 procesadas.")
                .arg(championImageProcessed).arg(championImageDownloadTotal),
            10000);
        return;
    }

    const QString championId = championImageQueue.takeFirst();
    const QString imagePath =
        QDir(championImageDirectory).filePath(championId + QStringLiteral(".png"));
    if (QFile::exists(imagePath)) {
        ++championImageSkipped;
        ++championImageProcessed;
        championImagesStatus->setText(
            QString("Procesadas %1 de %2 imágenes...")
                .arg(championImageProcessed).arg(championImageDownloadTotal));
        processNextChampionImage();
        return;
    }

    championImagesStatus->setText(
        QString("Descargando %1 (%2 de %3)...")
            .arg(championId)
            .arg(championImageProcessed + 1)
            .arg(championImageDownloadTotal));
    const QUrl imageUrl(
        QString("https://ddragon.leagueoflegends.com/cdn/") +
        championImageVersion +
        QStringLiteral("/img/champion/") + championId + QStringLiteral(".png"));
    auto* imageReply = networkManager.get(QNetworkRequest(imageUrl));
    connect(imageReply, &QNetworkReply::finished, this,
            [this, imageReply, championId, imagePath]() {
        const QByteArray body = imageReply->readAll();
        if (imageReply->error() != QNetworkReply::NoError) {
            championImageErrors.append(
                QString("%1: %2").arg(championId, imageReply->errorString()));
        } else if (!body.startsWith(QByteArray::fromHex("89504e470d0a1a0a"))) {
            championImageErrors.append(
                QString("%1: la respuesta no es un PNG válido").arg(championId));
        } else {
            QSaveFile imageFile(imagePath);
            if (!imageFile.open(QIODevice::WriteOnly)
                || imageFile.write(body) != body.size()
                || !imageFile.commit()) {
                const QString error = imageFile.errorString();
                imageFile.cancelWriting();
                championImageErrors.append(
                    QString("%1: no se pudo guardar la imagen: %2")
                        .arg(championId, error));
            } else {
                ++championImageDownloaded;
            }
        }
        imageReply->deleteLater();
        ++championImageProcessed;
        processNextChampionImage();
    });
}

void MainWindow::showChampionImage(int championKey, const QString& championName,
                                   QLabel* previewName, QLabel* previewImage)
{
    const auto showImage =
        [this, championName, previewName, previewImage](const QString& championId) {
        const QString imagePath = QDir(findCsvDirectory()).filePath(
            QStringLiteral("../assets/champion_images/") + championId
            + QStringLiteral(".png"));
        if (!QFile::exists(imagePath)) {
            const QString message =
                QString("No se encontró la imagen de %1. Usa «Descargar imágenes de "
                        "campeones» en Estadísticas y vuelve a intentarlo.")
                    .arg(championName);
            setChampionPreview(championName, {}, previewName, previewImage);
            if (previewName == championPreviewName && championImagesStatus) {
                championImagesStatus->setText(message);
            }
            return;
        }
        setChampionPreview(championName, imagePath, previewName, previewImage);
        if (previewName == championPreviewName && championImagesStatus) {
            championImagesStatus->setText(
                QString("Mostrando la imagen de %1.").arg(championName));
        }
    };

    const auto cachedId = championImageIdsByKey.constFind(championKey);
    if (cachedId != championImageIdsByKey.cend()) {
        showImage(*cachedId);
        return;
    }

    if (!championImageIdsByKey.isEmpty()) {
        const QString message =
            "El identificador no aparece en el catálogo de Data Dragon.";
        setChampionPreview(championName, {}, previewName, previewImage);
        if (previewName == championPreviewName && championImagesStatus) {
            championImagesStatus->setText(message);
        }
        return;
    }

    if (previewName == championPreviewName && championImagesStatus) {
        championImagesStatus->setText("Buscando la imagen del campeón en Data Dragon...");
    }
    auto* versionsReply = networkManager.get(QNetworkRequest(
        QUrl("https://ddragon.leagueoflegends.com/api/versions.json")));
    connect(versionsReply, &QNetworkReply::finished, this,
            [this, versionsReply, championKey, championName, previewName,
             previewImage, showImage]() {
        const QByteArray versionsBody = versionsReply->readAll();
        if (versionsReply->error() != QNetworkReply::NoError) {
            const QString error = versionsReply->errorString();
            versionsReply->deleteLater();
            const QString message =
                QString("No se pudo consultar Data Dragon: %1").arg(error);
            setChampionPreview(championName, {}, previewName, previewImage);
            if (previewName == championPreviewName && championImagesStatus) {
                championImagesStatus->setText(message);
            }
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument versionsDocument =
            QJsonDocument::fromJson(versionsBody, &parseError);
        if (parseError.error != QJsonParseError::NoError
            || !versionsDocument.isArray() || versionsDocument.array().isEmpty()) {
            versionsReply->deleteLater();
            const QString message =
                "Data Dragon devolvió una lista de versiones inválida.";
            setChampionPreview(championName, {}, previewName, previewImage);
            if (previewName == championPreviewName && championImagesStatus) {
                championImagesStatus->setText(message);
            }
            return;
        }
        const QString version = versionsDocument.array().first().toString();
        versionsReply->deleteLater();

        const QUrl catalogUrl(
            QString("https://ddragon.leagueoflegends.com/cdn/%1/data/es_MX/champion.json")
                .arg(version));
        auto* catalogReply = networkManager.get(QNetworkRequest(catalogUrl));
        connect(catalogReply, &QNetworkReply::finished, this,
                [this, catalogReply, championKey, championName, previewName,
                 previewImage, showImage]() {
            const QByteArray body = catalogReply->readAll();
            if (catalogReply->error() != QNetworkReply::NoError) {
                const QString error = catalogReply->errorString();
                catalogReply->deleteLater();
                const QString message =
                    QString("No se pudo descargar el catálogo de campeones: %1")
                        .arg(error);
                setChampionPreview(championName, {}, previewName, previewImage);
                if (previewName == championPreviewName && championImagesStatus) {
                    championImagesStatus->setText(message);
                }
                return;
            }

            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
            if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
                catalogReply->deleteLater();
                const QString message =
                    "Data Dragon devolvió un catálogo JSON inválido.";
                setChampionPreview(championName, {}, previewName, previewImage);
                if (previewName == championPreviewName && championImagesStatus) {
                    championImagesStatus->setText(message);
                }
                return;
            }

            const QJsonObject championObjects =
                document.object().value("data").toObject();
            for (const QJsonValue& value : championObjects) {
                const QJsonObject champion = value.toObject();
                bool keyValid = false;
                const int key = champion.value("key").toString().toInt(&keyValid);
                const QString id = champion.value("id").toString();
                if (keyValid && !id.isEmpty()) {
                    championImageIdsByKey.insert(key, id);
                }
            }
            catalogReply->deleteLater();

            const auto championId = championImageIdsByKey.constFind(championKey);
            if (championId == championImageIdsByKey.cend()) {
                const QString message =
                    "El campeón no aparece en el catálogo de Data Dragon.";
                setChampionPreview(championName, {}, previewName, previewImage);
                if (previewName == championPreviewName && championImagesStatus) {
                    championImagesStatus->setText(message);
                }
                return;
            }
            showImage(*championId);
        });
    });
}

void MainWindow::setChampionPreview(const QString& championName,
                                    const QString& imagePath,
                                    QLabel* previewName, QLabel* previewImage)
{
    if (!previewName || !previewImage) return;
    previewName->setText(championName);
    const QPixmap sourceImage(imagePath);
    if (sourceImage.isNull()) {
        previewImage->setPixmap(QPixmap());
        previewImage->setText(
            QString("No hay imagen local para %1.\nDescárgala desde el botón superior.")
                .arg(championName));
        return;
    }
    previewImage->setText(QString{});
    previewImage->setPixmap(sourceImage.scaled(
        previewImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

MainWindow::CsvTableView* MainWindow::findCsvTable(const QString& fileName)
{
    for (CsvTableView& view : csvTableViews) {
        if (view.fileName == fileName) return &view;
    }
    return nullptr;
}

void MainWindow::downloadRiotStatistics()
{
    if (downloadingRiotStats) return;
    if (core.getLolUsers().isEmpty()) {
        QMessageBox::information(this, "No hay cuentas",
                                 "Primero agrega cuentas de LoL para descargar sus estadísticas.");
        return;
    }

    riotStatsApiKey = qgetenv("RIOT_API_KEY").trimmed();
    if (riotStatsApiKey.isEmpty()) {
        QMessageBox::warning(
            this, "Falta la API key",
            "Configura RIOT_API_KEY en el entorno de ejecución de Qt Creator. "
            "No escribas la clave directamente en el código.");
        return;
    }

    bool accepted = false;
    const int matchLimit = QInputDialog::getInt(
        this, "Descargar estadísticas de Riot",
        "¿Cuántas partidas recientes por cuenta quieres consultar?",
        10, 1, 100, 1, &accepted);
    if (!accepted) {
        riotStatsApiKey.clear();
        return;
    }
    if (QMessageBox::question(
            this, "Confirmar descarga",
            QString("Se consultarán hasta %1 partidas de cada una de las %2 cuentas, "
                    "además de sus maestrías. Esto puede tardar varios minutos. ¿Continuar?")
                .arg(matchLimit).arg(core.getLolUsers().size()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {
        riotStatsApiKey.clear();
        return;
    }

    riotStatsQueue.clear();
    for (int index = 0; index < core.getLolUsers().size(); ++index) {
        riotStatsQueue.append(index);
    }
    riotStatsMatchLimit = matchLimit;
    riotStatsAccountTotal = riotStatsQueue.size();
    riotStatsAccountSuccesses = 0;
    riotStatsErrors.clear();
    downloadingRiotStats = true;

    ui->updateLolAccountButton->setEnabled(false);
    ui->updateAllLolAccountsButton->setEnabled(false);
    ui->tableLolUsers->setEnabled(false);
    if (CsvTableView* statsView = findCsvTable("state_stats.csv")) {
        if (statsView->downloadStatisticsButton) {
            statsView->downloadStatisticsButton->setEnabled(false);
        }
    }
    processNextRiotStatsAccount();
}

void MainWindow::processNextRiotStatsAccount()
{
    if (riotStatsQueue.isEmpty()) {
        const int failed = riotStatsAccountTotal - riotStatsAccountSuccesses;
        riotStatsApiKey.clear();
        downloadingRiotStats = false;
        ui->updateLolAccountButton->setEnabled(true);
        ui->updateAllLolAccountsButton->setEnabled(true);
        ui->tableLolUsers->setEnabled(true);
        if (CsvTableView* statsView = findCsvTable("state_stats.csv")) {
            if (statsView->downloadStatisticsButton) {
                statsView->downloadStatisticsButton->setEnabled(true);
            }
            statsView->status->setText(
                QString("%1 de %2 cuentas sincronizadas. Maestrías, partidas y estadísticas "
                        "se guardan en sus CSV. %3")
                    .arg(riotStatsAccountSuccesses)
                    .arg(riotStatsAccountTotal)
                    .arg(failed == 0 ? QString()
                                     : QString("Fallos: %1").arg(riotStatsErrors.join("; "))));
        }
        return;
    }

    riotStatsAccountIndex = riotStatsQueue.takeFirst();
    riotStatsCurrentErrors.clear();
    const LoLUser user = core.getLolUsers()[riotStatsAccountIndex];
    const QString riotId = QString::fromStdString(user.riotId).trimmed();
    const QString tagline = QString::fromStdString(user.tagline).trimmed();
    const QString puuid = QString::fromStdString(user.puuid).trimmed();
    const int current = riotStatsAccountTotal - riotStatsQueue.size();

    CsvTableView* statsView = findCsvTable("state_stats.csv");
    if (!statsView) {
        finishRiotStatsAccount(riotStatsAccountIndex,
                               "No se encontró la tabla de estadísticas.");
        return;
    }
    statsView->status->setText(
        QString("Cuenta %1 de %2: %3#%4")
            .arg(current).arg(riotStatsAccountTotal).arg(riotId, tagline));

    if (riotId.isEmpty() || tagline.isEmpty()) {
        finishRiotStatsAccount(riotStatsAccountIndex, "Riot ID o tagline vacío.");
        return;
    }
    if (!puuid.isEmpty()) {
        riotStatsPuuid = puuid;
        fetchRiotMasteries(riotStatsAccountIndex, riotStatsPuuid);
        return;
    }

    const QString encodedRiotId = QString::fromLatin1(QUrl::toPercentEncoding(riotId));
    const QString encodedTagline = QString::fromLatin1(QUrl::toPercentEncoding(tagline));
    const QUrl accountUrl(QStringLiteral(
        "https://americas.api.riotgames.com/riot/account/v1/accounts/by-riot-id/%1/%2")
                              .arg(encodedRiotId, encodedTagline));
    QNetworkRequest request(accountUrl);
    request.setRawHeader("X-Riot-Token", riotStatsApiKey);
    const int accountIndex = riotStatsAccountIndex;
    QNetworkReply* reply = networkManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, accountIndex]() {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            const QString error = riotReplyError(reply, body);
            reply->deleteLater();
            finishRiotStatsAccount(accountIndex, error);
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        const QString puuid = document.object().value("puuid").toString();
        reply->deleteLater();
        if (parseError.error != QJsonParseError::NoError || puuid.isEmpty()) {
            finishRiotStatsAccount(accountIndex, "Riot no devolvió un PUUID válido.");
            return;
        }

        LoLUser updatedUser = core.getLolUsers()[accountIndex];
        updatedUser.puuid = puuid.toStdString();
        QString error;
        if (!core.updateLolUser(accountIndex, updatedUser,
                                lolUsersCsvPath, &error)) {
            finishRiotStatsAccount(
                accountIndex, QString("No se pudo guardar el PUUID: %1").arg(error));
            return;
        }
        refreshLolUsersTable();
        riotStatsPuuid = puuid;
        fetchRiotMasteries(accountIndex, puuid);
    });
}

void MainWindow::fetchRiotMasteries(int accountIndex, const QString& puuid)
{
    CsvTableView* masteriesView = findCsvTable("state_masteries.csv");
    if (!masteriesView) {
        finishRiotStatsAccount(accountIndex, "No se encontró la tabla de maestrías.");
        return;
    }

    const QString encodedPuuid = QString::fromLatin1(QUrl::toPercentEncoding(puuid));
    const QUrl url(QStringLiteral(
        "https://la1.api.riotgames.com/lol/champion-mastery/v4/"
        "champion-masteries/by-puuid/%1?count=10").arg(encodedPuuid));
    QNetworkRequest request(url);
    request.setRawHeader("X-Riot-Token", riotStatsApiKey);
    QNetworkReply* reply = networkManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, accountIndex, puuid]() {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            const QString error = riotReplyError(reply, body);
            reply->deleteLater();
            finishRiotStatsAccount(accountIndex, error);
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isArray()) {
            reply->deleteLater();
            finishRiotStatsAccount(accountIndex,
                                   "Riot devolvió un formato de maestrías inesperado.");
            return;
        }
        reply->deleteLater();

        CsvTableView* masteriesView = findCsvTable("state_masteries.csv");
        CsvTableView* championsView = findCsvTable("state_champions.csv");
        if (!masteriesView) {
            finishRiotStatsAccount(accountIndex, "No se encontró la tabla de maestrías.");
            return;
        }
        CsvTableData masteries;
        QString error;
        if (!core.loadCsvTable(masteriesView->filePath,
                               masteriesView->csvHeaders, &masteries, &error)) {
            finishRiotStatsAccount(accountIndex, error);
            return;
        }
        const int userId = core.getLolUsers()[accountIndex].userId;
        for (int i = masteries.rows.size() - 1; i >= 0; --i) {
            if (masteries.rows[i][1].toInt() == userId) masteries.rows.removeAt(i);
        }

        QHash<int, QString> championNames;
        if (championsView) {
            CsvTableData champions;
            if (core.loadCsvTable(championsView->filePath,
                                  championsView->csvHeaders, &champions, &error)) {
                for (const QStringList& champion : champions.rows) {
                    championNames.insert(champion[0].toInt(), champion[1]);
                }
            }
        }

        int nextMasteryId = 1;
        for (const QStringList& row : masteries.rows) {
            bool idValid = false;
            const int id = row[0].toInt(&idValid);
            if (!idValid) {
                finishRiotStatsAccount(accountIndex,
                                       "Hay un ID de maestría inválido en el CSV.");
                return;
            }
            nextMasteryId = qMax(nextMasteryId, id + 1);
        }
        for (const QJsonValue& value : document.array()) {
            const QJsonObject mastery = value.toObject();
            const int championId = mastery.value("championId").toInt();
            masteries.rows.append({
                QString::number(nextMasteryId++),
                QString::number(userId),
                QString::number(championId),
                championNames.value(championId, QString::number(championId)),
                QString::number(mastery.value("championLevel").toInt()),
                QString::number(mastery.value("championPoints").toInt())
            });
        }
        if (!core.saveCsvTable(masteriesView->filePath, masteries, &error)) {
            finishRiotStatsAccount(
                accountIndex, QString("No se pudieron guardar las maestrías: %1").arg(error));
            return;
        }
        refreshCsvTable(*masteriesView);
        fetchRiotMatchIds(accountIndex, puuid);
    });
}

void MainWindow::fetchRiotMatchIds(int accountIndex, const QString& puuid)
{
    CsvTableView* matchesView = findCsvTable("state_matches.csv");
    CsvTableView* statsView = findCsvTable("state_stats.csv");
    if (!matchesView || !statsView) {
        finishRiotStatsAccount(accountIndex, "No se encontraron las tablas de partidas o estadísticas.");
        return;
    }

    QString error;
    if (!core.loadCsvTable(matchesView->filePath, matchesView->csvHeaders,
                           &riotStatsMatches, &error)
        || !core.loadCsvTable(statsView->filePath, statsView->csvHeaders,
                              &riotStatsRows, &error)) {
        finishRiotStatsAccount(accountIndex, error);
        return;
    }

    const QString encodedPuuid = QString::fromLatin1(QUrl::toPercentEncoding(puuid));
    const QUrl url(QStringLiteral(
        "https://americas.api.riotgames.com/lol/match/v5/matches/by-puuid/%1/ids"
        "?start=0&count=%2").arg(encodedPuuid).arg(riotStatsMatchLimit));
    QNetworkRequest request(url);
    request.setRawHeader("X-Riot-Token", riotStatsApiKey);
    QNetworkReply* reply = networkManager.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, accountIndex, puuid]() {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            const QString error = riotReplyError(reply, body);
            reply->deleteLater();
            finishRiotStatsAccount(accountIndex, error);
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isArray()) {
            reply->deleteLater();
            finishRiotStatsAccount(accountIndex,
                                   "Riot devolvió un formato de historial inesperado.");
            return;
        }
        riotStatsMatchIds.clear();
        for (const QJsonValue& value : document.array()) {
            const QString matchId = value.toString();
            if (!matchId.isEmpty()) riotStatsMatchIds.append(matchId);
        }
        reply->deleteLater();
        riotStatsMatchIndex = 0;
        fetchNextRiotMatch(accountIndex, puuid);
    });
}

void MainWindow::fetchNextRiotMatch(int accountIndex, const QString& puuid)
{
    const int userId = core.getLolUsers()[accountIndex].userId;
    while (riotStatsMatchIndex < riotStatsMatchIds.size()) {
        const QString& matchId = riotStatsMatchIds[riotStatsMatchIndex];
        const bool alreadySaved = std::any_of(
            riotStatsRows.rows.cbegin(), riotStatsRows.rows.cend(),
            [matchId, userId](const QStringList& row) {
                return row[1] == matchId && row[2].toInt() == userId;
            });
        if (!alreadySaved) break;
        ++riotStatsMatchIndex;
    }

    if (riotStatsMatchIndex >= riotStatsMatchIds.size()) {
        if (riotStatsCurrentErrors.isEmpty()) ++riotStatsAccountSuccesses;
        finishRiotStatsAccount(accountIndex);
        return;
    }

    const QString matchId = riotStatsMatchIds[riotStatsMatchIndex];
    CsvTableView* statsView = findCsvTable("state_stats.csv");
    if (statsView) {
        statsView->status->setText(
            QString("Descargando partida %1 de %2 para la cuenta %3...")
                .arg(riotStatsMatchIndex + 1)
                .arg(riotStatsMatchIds.size())
                .arg(QString::fromStdString(core.getLolUsers()[accountIndex].riotId)));
    }

    const QString encodedMatchId = QString::fromLatin1(QUrl::toPercentEncoding(matchId));
    const QUrl url(QStringLiteral(
        "https://americas.api.riotgames.com/lol/match/v5/matches/%1").arg(encodedMatchId));
    QNetworkRequest request(url);
    request.setRawHeader("X-Riot-Token", riotStatsApiKey);
    QNetworkReply* reply = networkManager.get(request);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, accountIndex, puuid, matchId, userId]() {
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            const QString error = riotReplyError(reply, body);
            reply->deleteLater();
            riotStatsCurrentErrors.append(QString("%1: %2").arg(matchId, error));
            ++riotStatsMatchIndex;
            QTimer::singleShot(1100, this, [this, accountIndex, puuid]() {
                fetchNextRiotMatch(accountIndex, puuid);
            });
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
        const QJsonObject info = document.object().value("info").toObject();
        const QJsonArray participants = info.value("participants").toArray();
        if (parseError.error != QJsonParseError::NoError || info.isEmpty()) {
            reply->deleteLater();
            riotStatsCurrentErrors.append(
                QString("%1: detalle de partida inválido").arg(matchId));
            ++riotStatsMatchIndex;
            QTimer::singleShot(1100, this, [this, accountIndex, puuid]() {
                fetchNextRiotMatch(accountIndex, puuid);
            });
            return;
        }

        QJsonObject participant;
        for (const QJsonValue& value : participants) {
            const QJsonObject item = value.toObject();
            if (item.value("puuid").toString() == puuid) {
                participant = item;
                break;
            }
        }
        if (participant.isEmpty()) {
            reply->deleteLater();
            riotStatsCurrentErrors.append(
                QString("%1: no se encontró la cuenta en los participantes").arg(matchId));
            ++riotStatsMatchIndex;
            QTimer::singleShot(1100, this, [this, accountIndex, puuid]() {
                fetchNextRiotMatch(accountIndex, puuid);
            });
            return;
        }
        reply->deleteLater();

        CsvTableView* matchesView = findCsvTable("state_matches.csv");
        CsvTableView* statsView = findCsvTable("state_stats.csv");
        if (!matchesView || !statsView) {
            finishRiotStatsAccount(accountIndex,
                                   "No se encontraron las tablas de partidas o estadísticas.");
            return;
        }

        CsvTableData updatedMatches = riotStatsMatches;
        const bool matchExists = std::any_of(
            updatedMatches.rows.cbegin(), updatedMatches.rows.cend(),
            [&matchId](const QStringList& row) { return row[0] == matchId; });
        if (!matchExists) {
            updatedMatches.rows.append({
                matchId,
                info.value("gameMode").toString(),
                QString::number(info.value("gameDuration").toInt()),
                QString::number(info.value("gameCreation").toVariant().toLongLong())
            });
        }

        int nextStatsId = 1;
        for (const QStringList& row : riotStatsRows.rows) {
            bool idValid = false;
            const int id = row[0].toInt(&idValid);
            if (!idValid) {
                riotStatsCurrentErrors.append(
                    QString("%1: hay un ID de estadísticas inválido en el CSV").arg(matchId));
                ++riotStatsMatchIndex;
                QTimer::singleShot(1100, this, [this, accountIndex, puuid]() {
                    fetchNextRiotMatch(accountIndex, puuid);
                });
                return;
            }
            nextStatsId = qMax(nextStatsId, id + 1);
        }

        CsvTableData updatedStats = riotStatsRows;
        const int kills = participant.value("kills").toInt();
        const int deaths = participant.value("deaths").toInt();
        const int assists = participant.value("assists").toInt();
        QString performanceStatus = "Estable";
        if (deaths >= 15) {
            performanceStatus = "Requiere Revision";
        } else if (kills + assists > deaths * 3) {
            performanceStatus = "Excelente Rendimiento";
        }
        updatedStats.rows.append({
            QString::number(nextStatsId),
            matchId,
            QString::number(userId),
            QString::number(participant.value("championId").toInt()),
            QString::number(kills),
            QString::number(deaths),
            QString::number(assists),
            participant.value("win").toBool() ? "1" : "0",
            QString::number(participant.value("goldEarned").toInt()),
            QString::number(participant.value("totalDamageDealtToChampions").toInt()),
            QString::number(participant.value("visionScore").toInt()),
            performanceStatus
        });

        QString error;
        if (!core.saveCsvTable(matchesView->filePath, updatedMatches, &error)) {
            riotStatsCurrentErrors.append(
                QString("%1: no se pudo guardar la partida: %2").arg(matchId, error));
        } else if (!core.saveCsvTable(statsView->filePath, updatedStats, &error)) {
            riotStatsMatches = updatedMatches;
            refreshCsvTable(*matchesView);
            riotStatsCurrentErrors.append(
                QString("%1: no se pudieron guardar las estadísticas: %2").arg(matchId, error));
        } else {
            riotStatsMatches = updatedMatches;
            riotStatsRows = updatedStats;
            refreshCsvTable(*matchesView);
            refreshCsvTable(*statsView);
            refreshSummaryReports();
        }

        ++riotStatsMatchIndex;
        QTimer::singleShot(1100, this, [this, accountIndex, puuid]() {
            fetchNextRiotMatch(accountIndex, puuid);
        });
    });
}

void MainWindow::finishRiotStatsAccount(int accountIndex, const QString& error)
{
    if (!error.isEmpty()) riotStatsCurrentErrors.append(error);
    if (!riotStatsCurrentErrors.isEmpty()) {
        const LoLUser& user = core.getLolUsers()[accountIndex];
        riotStatsErrors.append(
            QString("%1#%2: %3")
                .arg(QString::fromStdString(user.riotId),
                     QString::fromStdString(user.tagline),
                     riotStatsCurrentErrors.join(", ")));
    }
    riotStatsPuuid.clear();
    riotStatsMatchIds.clear();
    if (riotStatsQueue.isEmpty()) {
        processNextRiotStatsAccount();
        return;
    }
    QTimer::singleShot(1100, this, &MainWindow::processNextRiotStatsAccount);
}

MainWindow::~MainWindow()
{
    delete ui;
}
