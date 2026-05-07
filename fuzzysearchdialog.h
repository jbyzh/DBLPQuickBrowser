#ifndef FUZZYSEARCHDIALOG_H
#define FUZZYSEARCHDIALOG_H

#include <QDialog>
#include <QTimer>

#include <future>

#include "search.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
QT_END_NAMESPACE

class FuzzySearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FuzzySearchDialog(const QString& xmlPath, QWidget* parent = nullptr);

private slots:
    void executeSearch();
    void showResultDetail(QListWidgetItem* item);
    void previousPage();
    void nextPage();
    void pollSearchResult();

private:
    void buildUi();
    void updatePage();
    void updateYearChart();
    QString resultSummaryText(const search_module::SearchResult& result) const;
    void showDetailDialog(search_module::SearchResult result);

    search_module::Searcher m_searcher;
    QVector<search_module::SearchResult> m_results;
    QString m_currentKeyword;
    int m_currentPage = 0;
    const int m_itemsPerPage = 10;
    QString m_xmlPath;
    std::future<QVector<search_module::SearchResult>> m_pendingSearch;
    QTimer* m_searchPollTimer = nullptr;

    QLineEdit* m_keywordEdit = nullptr;
    QListWidget* m_resultList = nullptr;
    QLabel* m_pageLabel = nullptr;
    QLabel* m_statusLabel = nullptr;
    QPushButton* m_prevButton = nullptr;
    QPushButton* m_nextButton = nullptr;
    QPushButton* m_searchButton = nullptr;
    QWidget* m_chartContainer = nullptr;
};

#endif
