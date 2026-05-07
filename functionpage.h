#ifndef FUNCTIONPAGE_H
#define FUNCTIONPAGE_H

#include <QPointer>
#include <QString>
#include <QWidget>

class AnalyticsWindow;
class Clique;
class IntervalWindow;
class Precise;
class SearchDialog;
class FuzzySearchDialog;
class QPushButton;

QT_BEGIN_NAMESPACE
namespace Ui {
class FunctionPage;
}
QT_END_NAMESPACE

class FunctionPage : public QWidget
{
    Q_OBJECT

public:
    explicit FunctionPage(QWidget *parent = nullptr);
    ~FunctionPage();

    void setDataPath(const QString& dataPath);

signals:
    void backToMain();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void showFeatureHelp();

private:
    Ui::FunctionPage *ui;
    QPointer<AnalyticsWindow> m_analyticsWindow;
    QPointer<Clique> m_cliqueWindow;
    QPointer<IntervalWindow> m_intervalWindow;
    QPointer<Precise> m_preciseWindow;
    QPushButton* m_helpButton = nullptr;
    QString m_dataPath;
};

#endif // FUNCTIONPAGE_H
