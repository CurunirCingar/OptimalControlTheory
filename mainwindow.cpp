#include "krylovchernouskomethod.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <limits>

void MainWindow::SetActiveSeries(ChartTypes type)
{
    currentChartType = type;
    currentSeries = GetLineSeriesForType(currentChartType);
    connect(&autoCalculateTimer, &QTimer::timeout, ui->buttonCalculate, &QPushButton::click);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    AddSeries(chart_x1_x2, "X2(X1)", Qt::red);
    AddSeries(chart_t_x1, "X1(t)", Qt::green);
    AddSeries(chart_t_x2, "X2(t)", Qt::blue);
    AddSeries(chart_t_u, "U(t)", Qt::cyan);
    SetActiveSeries(ChartTypes::X1_X2);
    ui->chart->setChart(&chart);
    ui->chart->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::AddSeries(QLineSeries &series, QString name, Qt::GlobalColor color)
{
    series.setName(name);
    series.setColor(QColor(color));
}

void MainWindow::on_buttonCalculate_clicked()
{
    if(!calc.RunIteration())
        autoCalculateTimer.stop();

    currentSeries->clear();
    ShowSeriesOnChart(currentSeries);
    auto functional = calc.GetFunctionalValues().last();
    ui->spinFunctional->setValue(functional);
}

QLineSeries* MainWindow::GetLineSeriesForType(ChartTypes type)
{
    switch (type)
    {
        case ChartTypes::X1_X2:
            return &chart_x1_x2;

        case ChartTypes::T_X1:
            return &chart_t_x1;

        case ChartTypes::T_X2:
            return &chart_t_x2;

        case ChartTypes::T_U:
            return &chart_t_u;

        default:
            return &chart_x1_x2;
    }
}

void MainWindow::on_comboBox_activated(int index)
{
    currentChartType = (ChartTypes)index;
    chart.removeSeries(currentSeries);
    currentSeries = GetLineSeriesForType(currentChartType);
    ShowSeriesOnChart(currentSeries);
}

void MainWindow::ShowSeriesOnChart(QLineSeries *series)
{
    series->clear();
    chart.addSeries(series);
    chart.createDefaultAxes();
    auto values = calc.GetCalculateValues();
    QPair<float, float> xAxisConstr(INFINITY, -INFINITY);
    QPair<float, float> yAxisConstr(INFINITY, -INFINITY);

    for(int i = 0; i < values.length(); ++i)
    {
        QPointF valuePair = GetValuesForChart(values[i], currentChartType);
        series->append(valuePair);
        UpdateChartConstraints(xAxisConstr, valuePair.x());
        UpdateChartConstraints(yAxisConstr, valuePair.y());
    }

    if(fabs(xAxisConstr.second - xAxisConstr.first) < EPSILON)
    {
        xAxisConstr.first -= 0.5;
        xAxisConstr.second += 0.5;
    }

    if(fabs(yAxisConstr.second - yAxisConstr.first) < EPSILON)
    {
        yAxisConstr.first -= 0.5;
        yAxisConstr.second += 0.5;
    }

    chart.axisX()->setRange(xAxisConstr.first, xAxisConstr.second);
    chart.axisY()->setRange(yAxisConstr.first, yAxisConstr.second);
}

QPointF MainWindow::GetValuesForChart(KrylovChernouskoMethod::CalculateValues value, ChartTypes type)
{
    switch (type)
    {
        case MainWindow::X1_X2:
            return QPointF(value.x1, value.x2);

        case MainWindow::T_X1:
            return QPointF(value.t, value.x1);

        case MainWindow::T_X2:
            return QPointF(value.t, value.x2);

        case MainWindow::T_U:
            return QPointF(value.t, value.u);

        default:
            return QPointF(value.x1, value.x2);
            break;
    }
}

void MainWindow::UpdateChartConstraints(QPair<float, float> &constr, float value)
{
    constr.first = (constr.first > value) ? value : constr.first;
    constr.second = (constr.second < value) ? value : constr.second;
}
void MainWindow::on_buttonStartCalculation_clicked()
{
    cond.maxControl = ui->spinMaxControl->value();
    cond.minControl = ui->spinMinControl->value();
    cond.stepsCount = ui->spinSteps->value();
    cond.tMax = ui->spinEndTime->value();
    cond.tMin = ui->spinStartTime->value();
    cond.x1Start = ui->spinStartX1->value();
    cond.x2Start = ui->spinStartX2->value();
    cond.x1End = ui->spinEndX1->value();
    cond.x2End = ui->spinEndX2->value();
    cond.startControl = ui->spinStartControl->value();
    calc.SetInitConditions(cond);
    calc.RunIteration();
    ShowSeriesOnChart(currentSeries);
    auto functional = calc.GetFunctionalValues().last();
    ui->spinFunctional->setValue(functional);
}

void MainWindow::on_buttonCalculateAll_clicked()
{
    if(!autoCalculateTimer.isActive())
        autoCalculateTimer.start(100);
    else
        autoCalculateTimer.stop();
}
