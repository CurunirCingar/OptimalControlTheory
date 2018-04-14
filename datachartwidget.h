#ifndef DATACHARTWIDGET_H
#define DATACHARTWIDGET_H

#include <QObject>

class DataChartWidget : public QObject
{
    Q_OBJECT
public:
    explicit DataChartWidget(QObject *parent = nullptr);

signals:

public slots:
};

#endif // DATACHARTWIDGET_H