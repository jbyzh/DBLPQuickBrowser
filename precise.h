#ifndef PRECISE_H
#define PRECISE_H

#include <QWidget>

namespace Ui {
class Precise;
}

class Precise : public QWidget
{
    Q_OBJECT

public:
    explicit Precise(QWidget *parent = nullptr);
    ~Precise();

private:
    Ui::Precise *ui;
};

#endif // PRECISE_H
