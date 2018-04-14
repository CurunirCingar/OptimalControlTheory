#include "datachartwidget.h"

DataChartWidget::DataChartWidget(QObject *parent) : QObject(parent)
{
    chart.addSeries(&currentSeries);
    chart.createDefaultAxes();
}

void DataChartWidget::AddPoint(float x, float y)
{
    currentSeries.append(x, y);
}

void DataChartWidget::ClearChart()
{
}
