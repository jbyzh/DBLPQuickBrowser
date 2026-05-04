#ifndef FUNCTIONPAGE_H
#define FUNCTIONPAGE_H

#include <QPointer>
#include <QString>
#include <QWidget>

class AnalyticsWindow;
class IntervalWindow;

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
    void on_pushButton_3_clicked();
    void on_pushButton_5_clicked();

private:
    Ui::FunctionPage *ui;
    QPointer<AnalyticsWindow> m_analyticsWindow;
    QPointer<IntervalWindow> m_intervalWindow;
    QString m_dataPath;
};

#endif // FUNCTIONPAGE_H
