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
    chart.setTheme(QChart::ChartThemeHighContrast);
    chart.setAnimationOptions(QChart::SeriesAnimations);
    SetupSeries(ChartTypes::X1_X2, "X2(X1)", Qt::red);
    SetupSeries(ChartTypes::T_X1, "X1(t)", Qt::green);
    SetupSeries(ChartTypes::T_X2, "X2(t)", Qt::green);
    SetupSeries(ChartTypes::T_U, "U(t)", Qt::blue);
    SetupSeries(ChartTypes::T_UCor, "UCor(t)", Qt::blue);
    SetupSeries(ChartTypes::T_H, "H(t)", Qt::red);
    SetupSeries(ChartTypes::T_P1, "P1(t)", Qt::green);
    SetupSeries(ChartTypes::T_P2, "P2(t)", Qt::green);
    SetupSeries(ChartTypes::Iteration_I, "I(IterNum)", Qt::green);
    SetActiveSeries(ChartTypes::X1_X2);
    ui->chart->setChart(&chart);
    ui->chart->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetupSeries(ChartTypes type, QString name, Qt::GlobalColor color)
{
    QLineSeries *series = new QLineSeries(this);
    series->setName(name);
    series->setColor(QColor(color));
    charts.insert(type, series);
    ui->comboBox->addItem(name);
}

void MainWindow::on_buttonCalculate_clicked()
{
    if(!calc.RunIteration())
    {
        autoCalculateTimer.stop();
        ui->buttonCalculate->setEnabled(false);
        ui->buttonCalculateAll->setEnabled(false);
    }
    else
    {
        ui->spinIterations->setValue(calc.GetPassedIterationsCount());
        currentSeries->clear();
        ShowSeriesOnChart(currentSeries);
        auto functional = calc.GetFunctionalValues().last();
        ui->spinFunctional->setValue(functional);
        ui->buttonCalculate->setEnabled(true);
        ui->buttonCalculateAll->setEnabled(true);
    }
}

QLineSeries* MainWindow::GetLineSeriesForType(ChartTypes type)
{
    if(!charts.contains(type))
        return charts.first();

    return charts[type];
}

void MainWindow::on_comboBox_activated(int index)
{
    currentChartType = (ChartTypes)index;
    chart.removeSeries(currentSeries);
    currentSeries = GetLineSeriesForType(currentChartType);
    ShowSeriesOnChart(currentSeries);
}

void MainWindow::NormalizeAxis(QPair<float, float> &axisConstr)
{
    qDebug() << axisConstr;

    if(fabs(axisConstr.second - axisConstr.first) < EPSILON)
    {
        axisConstr.first -= 0.5;
        axisConstr.second += 0.5;
    }

    if(axisConstr.first >= 0)
    {
        axisConstr.first = 0;
    }
    else if(axisConstr.second <= 0)
    {
        axisConstr.second = 0;
    }
    else
    {
        auto absFirst = fabsf(axisConstr.first);
        auto absSecond = fabsf(axisConstr.second);
        auto buf = absFirst > absSecond ? absFirst : absSecond;
        axisConstr.first = -buf;
        axisConstr.second = buf;
    }
}

void MainWindow::ShowSeriesOnChart(QLineSeries *series)
{
    series->clear();
    chart.addSeries(series);
    chart.createDefaultAxes();
    QPair<float, float> xAxisConstr(INFINITY, -INFINITY);
    QPair<float, float> yAxisConstr(INFINITY, -INFINITY);

    if(currentChartType != ChartTypes::Iteration_I)
    {
        auto values = calc.GetCalculateValues();

        for(int i = 0; i < values.length(); ++i)
        {
            QPointF valuePair = GetValuesForChart(values[i], currentChartType);
            series->append(valuePair);
            UpdateChartConstraints(xAxisConstr, valuePair.x());
            UpdateChartConstraints(yAxisConstr, valuePair.y());
        }
    }
    else
    {
        auto values = calc.GetIterationsCriterias();

        for(int i = 0; i < values.length(); ++i)
        {
            series->append(i, values[i]);
            UpdateChartConstraints(xAxisConstr, i);
            UpdateChartConstraints(yAxisConstr, values[i]);
        }
    }

    NormalizeAxis(xAxisConstr);
    NormalizeAxis(yAxisConstr);
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

        case MainWindow::T_UCor:
            return QPointF(value.t, value.uCorr);
            break;

        case MainWindow::T_H:
            return QPointF(value.t, value.H);
            break;

        case MainWindow::T_P1:
            return QPointF(value.t, value.p1);
            break;

        case MainWindow::T_P2:
            return QPointF(value.t, value.p2);
            break;

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
    cond.Ro1 = ui->spinRo1->value();
    cond.Ro2 = ui->spinRo2->value();
    cond.k = ui->spinK->value();
    calc.SetInitConditions(cond);
    on_buttonCalculate_clicked();
}

void MainWindow::on_buttonCalculateAll_clicked()
{
    if(!autoCalculateTimer.isActive())
        autoCalculateTimer.start(100);
    else
        autoCalculateTimer.stop();
}
