#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QNetworkAccessManager>
#include <QStringList>
#include <QTableWidget>
#include "src/esports_core.h"
class QLabel;
class QPushButton;
class QComboBox;
class QPixmap;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    struct CsvTableView {
        QString title;
        QString fileName;
        QStringList csvHeaders;
        QStringList visibleHeaders;
        QString filePath;
        QTableWidget* table = nullptr;
        QLabel* status = nullptr;
        QPushButton* downloadButton = nullptr;
        QPushButton* downloadChampionImagesButton = nullptr;
        QPushButton* addRowButton = nullptr;
        QPushButton* editRowButton = nullptr;
        QPushButton* removeRowButton = nullptr;
    };

    void addPlayer();
    void editPlayer();
    void removePlayer();
    void changeSelectedPlayerRole();
    void undoLastRoleChange();
    bool savePlayerRoleChange(int playerIndex, const QString& newRole,
                              const QString& action, int referenceChangeId,
                              QString* errorMessage);
    void updateSelectedLolAccount();
    void updateAllLolAccounts();
    void configureRiotApiKey();
    void clearRiotApiKey();
    bool ensureRiotApiKey();
    void addLolAccount();
    void processNextLolAccountUpdate();
    void finishLolAccountUpdate(int index, const QString& error = {});
    void refreshPlayersTable();
    void refreshLolUsersTable();
    void addCsvDataTabs();
    void addSummaryReportTabs();
    void refreshSummaryReports();
    void refreshCsvTable(CsvTableView& view);
    void addContract();
    void editSelectedContract();
    void removeSelectedContract();
    bool showContractDialog(int contractRow, QString* errorMessage);
    void downloadChampionCatalog();
    void downloadChampionImages();
    void processNextChampionImage();
    void showChampionImage(int championKey, const QString& championName,
                           QLabel* previewName, QLabel* previewImage);
    void setChampionPreview(const QString& championName, const QString& imagePath,
                            QLabel* previewName, QLabel* previewImage);
    void downloadRiotStatistics();
    void processNextRiotStatsAccount();
    void fetchRiotMasteries(int accountIndex, const QString& puuid);
    void fetchRiotMatchIds(int accountIndex, const QString& puuid);
    void fetchNextRiotMatch(int accountIndex, const QString& puuid);
    void finishRiotStatsAccount(int accountIndex, const QString& error = {});
    CsvTableView* findCsvTable(const QString& fileName);

    Ui::MainWindow *ui;
    EsportsCore core;
    QString playersCsvPath;
    QString lolUsersCsvPath;
    QTableWidget* registeredPlayersTable = nullptr;
    QTableWidget* badPerformersTable = nullptr;
    QTableWidget* freeAgentsTable = nullptr;
    QTableWidget* recentMatchesTable = nullptr;
    QComboBox* recentAccountCombo = nullptr;
    QPushButton* downloadDataButton = nullptr;
    QNetworkAccessManager networkManager;
    QVector<CsvTableView> csvTableViews;
    QVector<int> lolAccountUpdateQueue;
    QByteArray riotApiKey;
    QStringList lolAccountUpdateErrors;
    int lolAccountUpdateTotal = 0;
    int lolAccountUpdateSuccesses = 0;
    bool updatingAllLolAccounts = false;
    bool downloadingChampionCatalogInBackground = false;
    QLabel* championImagesStatus = nullptr;
    QLabel* championPreviewName = nullptr;
    QLabel* championPreviewImage = nullptr;
    QLabel* masteryPreviewName = nullptr;
    QLabel* masteryPreviewImage = nullptr;
    QStringList championImageQueue;
    QStringList championImageErrors;
    QString championImageDirectory;
    QString championImageVersion;
    QHash<int, QString> championImageIdsByKey;
    int championImageDownloadTotal = 0;
    int championImageProcessed = 0;
    int championImageSkipped = 0;
    int championImageDownloaded = 0;
    QVector<int> riotStatsQueue;
    QString riotStatsPuuid;
    QStringList riotStatsMatchIds;
    QStringList riotStatsCurrentErrors;
    CsvTableData riotStatsMatches;
    CsvTableData riotStatsRows;
    int riotStatsAccountIndex = -1;
    int riotStatsMatchIndex = 0;
    int riotStatsMatchLimit = 10;
    int riotStatsAccountTotal = 0;
    int riotStatsAccountSuccesses = 0;
    QStringList riotStatsErrors;
    bool downloadingRiotStats = false;
};
#endif // MAINWINDOW_H
