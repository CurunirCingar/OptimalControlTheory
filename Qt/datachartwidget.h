#ifndef DATACHARTWIDGET_H
#define DATACHARTWIDGET_H

#include <QChart>
#include <QLineSeries>
#include <QtCore>
using namespace QtCharts;


class DataChartWidget : public QObject
{
    Q_OBJECT
public:
    explicit DataChartWidget(QObject *parent = nullptr);

signals:

public slots:
    void AddPoint(float x, float y);
    void ClearChart();

private:
    QChart chart;
    QLineSeries currentSeries;
};

#endif // DATACHARTWIDGET_H
