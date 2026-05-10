#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

#include "search.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(const QString& xmlPath, QWidget* parent = nullptr);

private slots:
    void executeSearch();
    void showResultDetail(QListWidgetItem* item);
    void previousPage();
    void nextPage();
    void updatePlaceholderText();

private:
    void buildUi();
    void updatePage();
    QString resultSummaryText(const search_module::SearchResult& result) const;
    void showDetailDialog(search_module::SearchResult result);

    search_module::Searcher m_searcher;
    QVector<search_module::SearchResult> m_results;
    int m_currentPage = 0;
    const int m_itemsPerPage = 20;

    QComboBox* m_modeCombo = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QListWidget* m_resultList = nullptr;
    QLabel* m_resultCountLabel = nullptr;
    QLabel* m_pageLabel = nullptr;
    QPushButton* m_prevButton = nullptr;
    QPushButton* m_nextButton = nullptr;
};

#endif
