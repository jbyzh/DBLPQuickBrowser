#include "searchdialog.h"

#include <algorithm>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

void showStyledMessageBox(QWidget* parent,
                          QMessageBox::Icon icon,
                          const QString& title,
                          const QString& text)
{
    QMessageBox box(parent);
    box.setIcon(icon);
    box.setWindowTitle(title);
    box.setText(text);
    box.setWindowIcon(QIcon(":/picture/book.jpeg"));
    box.setStyleSheet(
        "QMessageBox{background-color:rgba(255,255,255,0.98);}"
        "QLabel{color:#1F2D3D; font:12px 'Microsoft YaHei'; min-width:140px; background:transparent;}"
        "QPushButton{background:#409EFF; color:white; border:none; border-radius:6px; padding:6px 14px; min-width:60px;}"
        "QPushButton:hover{background:#66B1FF;}"
    );
    box.exec();
}

}

SearchDialog::SearchDialog(const QString& xmlPath, QWidget* parent)
    : QDialog(parent)
    , m_searcher(xmlPath)
{
    buildUi();
}

void SearchDialog::buildUi()
{
    setWindowTitle(QString::fromUtf8("基础文献搜索"));
    setWindowIcon(QIcon(":/picture/book.jpeg"));
    resize(900, 640);
    setAttribute(Qt::WA_StyledBackground, true);
    setObjectName("SearchDialogBg");
    setStyleSheet(
        "#SearchDialogBg{border-image:url(:/picture/bg.png);}"
        "QDialog{background:transparent;}"
        "QLabel,QPushButton,QComboBox,QLineEdit,QListWidget{font:12px 'Microsoft YaHei';}"
        "QLabel{color:#1F2D3D; background:transparent;}"
        "QComboBox,QLineEdit{background-color:rgba(255,255,255,0.88); color:#1F2D3D; border:1px solid rgba(64,158,255,0.18); border-radius:6px; padding:6px 8px;}"
        "QListWidget{background-color:rgba(255,255,255,0.78); color:#1F2D3D; border:1px solid rgba(255,255,255,0.36); border-radius:8px; outline:0;}"
        "QListWidget::item{color:#1F2D3D; background:transparent; padding:10px 12px; border-bottom:1px solid rgba(31,45,61,0.08);}"
        "QListWidget::item:selected{background-color:rgba(64,158,255,0.22); color:#0F172A;}"
        "QListWidget::item:hover{background-color:rgba(255,255,255,0.35); color:#1F2D3D;}"
        "QPushButton{background:#409EFF; color:white; border:none; border-radius:6px; padding:8px 14px;}"
        "QPushButton:hover{background:#66B1FF;}"
    );

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(12);

    QLabel* title = new QLabel(QString::fromUtf8("基础文献搜索"), this);
    title->setStyleSheet("font:700 22px 'Microsoft YaHei'; padding:6px 2px;");
    root->addWidget(title);

    QHBoxLayout* top = new QHBoxLayout();
    top->addWidget(new QLabel(QString::fromUtf8("搜索模式"), this));

    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItem(QString::fromUtf8("按作者名搜索"));
    m_modeCombo->addItem(QString::fromUtf8("按论文标题搜索"));
    top->addWidget(m_modeCombo);

    m_searchEdit = new QLineEdit(this);
    top->addWidget(m_searchEdit, 1);

    QPushButton* searchButton = new QPushButton(QString::fromUtf8("搜索"), this);
    top->addWidget(searchButton);
    root->addLayout(top);

    m_resultList = new QListWidget(this);
    m_resultList->setWordWrap(true);
    root->addWidget(m_resultList, 1);

    QHBoxLayout* pager = new QHBoxLayout();
    m_resultCountLabel = new QLabel(QString::fromUtf8("共 0 条结果"), this);
    pager->addWidget(m_resultCountLabel);
    pager->addSpacing(14);
    m_pageLabel = new QLabel(QString::fromUtf8("第 0 / 0 页"), this);
    pager->addWidget(m_pageLabel);
    pager->addStretch(1);

    m_prevButton = new QPushButton(QString::fromUtf8("上一页"), this);
    m_nextButton = new QPushButton(QString::fromUtf8("下一页"), this);
    pager->addWidget(m_prevButton);
    pager->addWidget(m_nextButton);
    root->addLayout(pager);

    connect(searchButton, &QPushButton::clicked, this, &SearchDialog::executeSearch);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &SearchDialog::executeSearch);
    connect(m_resultList, &QListWidget::itemDoubleClicked, this, &SearchDialog::showResultDetail);
    connect(m_prevButton, &QPushButton::clicked, this, &SearchDialog::previousPage);
    connect(m_nextButton, &QPushButton::clicked, this, &SearchDialog::nextPage);
    connect(m_modeCombo, &QComboBox::currentIndexChanged, this, &SearchDialog::updatePlaceholderText);

    updatePlaceholderText();
    updatePage();
}

void SearchDialog::executeSearch()
{
    const QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        showStyledMessageBox(this, QMessageBox::Warning, QString::fromUtf8("提示"), QString::fromUtf8("请输入搜索内容。"));
        return;
    }

    if (m_modeCombo->currentIndex() == 0) {
        m_results = m_searcher.searchByAuthor(keyword);
    } else {
        m_results = m_searcher.searchByTitle(keyword);
    }

    for (search_module::SearchResult& result : m_results) {
        m_searcher.readPaperDetails(result);
    }

    m_currentPage = 0;
    updatePage();
}

void SearchDialog::updatePage()
{
    m_resultList->clear();

    if (m_resultCountLabel) {
        m_resultCountLabel->setText(QString::fromUtf8("共 %1 条结果").arg(m_results.size()));
    }

    const int totalPages = m_results.isEmpty() ? 0 : ((m_results.size() + m_itemsPerPage - 1) / m_itemsPerPage);
    const int start = m_currentPage * m_itemsPerPage;
    const int end = std::min(start + m_itemsPerPage, static_cast<int>(m_results.size()));

    if (m_results.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8("未找到相关结果"), m_resultList);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    }

    for (int i = start; i < end; ++i) {
        QListWidgetItem* item = new QListWidgetItem(resultSummaryText(m_results[i]), m_resultList);
        item->setData(Qt::UserRole, i);
        item->setSizeHint(QSize(item->sizeHint().width(), 62));
    }

    m_pageLabel->setText(QString::fromUtf8("第 %1 / %2 页")
                             .arg(totalPages == 0 ? 0 : m_currentPage + 1)
                             .arg(totalPages));
    m_prevButton->setEnabled(m_currentPage > 0);
    m_nextButton->setEnabled(totalPages > 0 && m_currentPage < totalPages - 1);
}

QString SearchDialog::resultSummaryText(const search_module::SearchResult& result) const
{
    if (!result.year.isEmpty() && result.year != "0") {
        return QString("%1 (%2)").arg(result.title, result.year);
    }
    return result.title;
}

void SearchDialog::showResultDetail(QListWidgetItem* item)
{
    const int index = item->data(Qt::UserRole).toInt();
    if (index < 0 || index >= m_results.size()) {
        return;
    }
    showDetailDialog(m_results[index]);
}

void SearchDialog::showDetailDialog(search_module::SearchResult result)
{
    m_searcher.readPaperDetails(result);

    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8("论文详情"));
    dialog.setWindowIcon(QIcon(":/picture/book.jpeg"));
    dialog.resize(720, 520);
    dialog.setAttribute(Qt::WA_StyledBackground, true);
    dialog.setObjectName("SearchDetailDialogBg");
    dialog.setStyleSheet(
        "#SearchDetailDialogBg{border-image:url(:/picture/bg.png);}"
        "QDialog{background:transparent;}"
        "QTextEdit{background-color:rgba(255,255,255,0.84); color:#1F2D3D; border:1px solid rgba(255,255,255,0.35); border-radius:8px; font:12px 'Microsoft YaHei';}"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTextEdit* textEdit = new QTextEdit(&dialog);
    textEdit->setReadOnly(true);

    QString detail;
    if (!result.title.isEmpty()) detail += QString::fromUtf8("标题: %1\n\n").arg(result.title);
    if (!result.authors.isEmpty()) detail += QString::fromUtf8("作者: %1\n\n").arg(result.authors);
    if (!result.year.isEmpty()) detail += QString::fromUtf8("年份: %1\n\n").arg(result.year);
    if (!result.venue.isEmpty()) detail += QString::fromUtf8("会议/书名: %1\n\n").arg(result.venue);
    if (!result.journal.isEmpty()) detail += QString::fromUtf8("期刊: %1\n\n").arg(result.journal);
    if (!result.volume.isEmpty()) detail += QString::fromUtf8("卷号: %1\n\n").arg(result.volume);
    if (!result.number.isEmpty()) detail += QString::fromUtf8("期号: %1\n\n").arg(result.number);
    if (!result.pages.isEmpty()) detail += QString::fromUtf8("页码: %1\n\n").arg(result.pages);
    if (!result.doi.isEmpty()) detail += QString::fromUtf8("链接/DOI: %1\n\n").arg(result.doi);
    textEdit->setText(detail.trimmed());
    layout->addWidget(textEdit, 1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    layout->addWidget(buttonBox);
    dialog.exec();
}

void SearchDialog::previousPage()
{
    if (m_currentPage > 0) {
        --m_currentPage;
        updatePage();
    }
}

void SearchDialog::nextPage()
{
    const int totalPages = m_results.isEmpty() ? 0 : ((m_results.size() + m_itemsPerPage - 1) / m_itemsPerPage);
    if (m_currentPage + 1 < totalPages) {
        ++m_currentPage;
        updatePage();
    }
}

void SearchDialog::updatePlaceholderText()
{
    if (m_modeCombo->currentIndex() == 0) {
        m_searchEdit->setPlaceholderText(QString::fromUtf8("支持作者名部分匹配，例如 Wang"));
    } else {
        m_searchEdit->setPlaceholderText(QString::fromUtf8("请输入完整论文标题"));
    }
}
