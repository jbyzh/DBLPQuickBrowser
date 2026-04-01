// FunctionPage.h
#ifndef FUNCTIONPAGE_H
#define FUNCTIONPAGE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class FunctionPage : public QWidget
{
    Q_OBJECT
public:
    explicit FunctionPage(QWidget *parent = nullptr);

signals:
    void backToMain(); // 返回主界面的信号

private slots:
               //void onAuthorSearchClicked();  // 作者查询
               //void onYearStatClicked();     // 年份统计
               //void onArticleSearchClicked();// 文章检索
               // ... 其他功能按钮

private:
    QLabel* titleLabel;
    QPushButton* authorBtn;
    QPushButton* yearBtn;
    QPushButton* articleBtn;
    QPushButton* backBtn;
};

#endif // FUNCTIONPAGE_H
