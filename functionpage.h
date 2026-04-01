#ifndef FUNCTIONPAGE_H
#define FUNCTIONPAGE_H

#include <QWidget>

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

signals:
    void backToMain();

private slots:

    void on_pushButton_clicked();

private:
    Ui::FunctionPage *ui;
};

#endif // FUNCTIONPAGE_H
