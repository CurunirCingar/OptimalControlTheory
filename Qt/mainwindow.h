#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "krylovchernouskomethod.h"

#include <QtCharts>
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_buttonCalculate_clicked();

    void on_comboBox_activated(int index);

    void ShowSeriesOnChart(QLineSeries *series);


    void on_buttonStartCalculation_clicked();

    void on_buttonCalculateAll_clicked();

private:
    enum ChartTypes
    {
        X1_X2,
        T_X1,
        T_X2,
        T_U,
        T_UCor,
        T_H,
        T_P1,
        T_P2,
        Iteration_I,
    };

    Ui::MainWindow *ui;
    QChart chart;
    QTimer autoCalculateTimer;

    ChartTypes currentChartType;
    QLineSeries *currentSeries;

    QMap<ChartTypes, QLineSeries*> charts;

    KrylovChernouskoMethod calc;
    KrylovChernouskoMethod::InitConditions cond;

    const float EPSILON = 0.00001;

    void UpdateChartConstraints(QPair<float, float> &constr, float value);
    QPointF GetValuesForChart(KrylovChernouskoMethod::CalculateValues value, ChartTypes type);
    void SetupSeries(ChartTypes type, QString name, Qt::GlobalColor color);
    void SetActiveSeries(ChartTypes type);
    QLineSeries *GetLineSeriesForType(ChartTypes type);
    void NormalizeAxis(QPair<float, float> &axisConstr);
};

#endif // MAINWINDOW_H
