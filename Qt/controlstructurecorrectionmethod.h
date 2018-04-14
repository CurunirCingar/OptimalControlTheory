#ifndef CONTROLSTRUCTURECORRECTIONMETHOD_H
#define CONTROLSTRUCTURECORRECTIONMETHOD_H

#include <QtCore>

class ControlStructureCorrectionMethod : public QObject
{
    Q_OBJECT
public:
    explicit ControlStructureCorrectionMethod(QObject *parent = nullptr);

    struct InitConditions
    {
        float x1Start;
        float x2Start;
        float x1End;
        float x2End;
        float tMin;
        float tMax;
        float tSwitch;
        int stepsCount;
        float minControl;
        float maxControl;
        float startControl;
        float Ro1;
        float Ro2;
        float k;
    };

    struct CalculateValues
    {
        CalculateValues() {}
        float t;

        float x1;
        float x2;
        float y;
        float tu;

        float p1;
        float p2;
        float p3;
        float pu;
        float u;

        float x1Corr;
        float x2Corr;
        float uCorr;
        float uOptimal;
        float H;
    };

    void SetInitConditions(InitConditions conds);
    bool RunIteration();
    int GetPassedIterationsCount();
    QList<float> GetIterationsCriterias();

    QVector<CalculateValues> GetCalculateValues();
    QList<float> GetFunctionalValues();

private:
    InitConditions initConditions;
    QVector<CalculateValues> calculateValues;
    float dt;
    int iterationsCount;
    QList<float> qualityCriteriaValues;
    const float EPSILON = 0.0000001;
    const int optimalSearchIterationsLimit = 100;


    float Clamp(float min, float max, float value);
    template <typename T> int sign(T val)
    {
        return (T(0) < val) - (val < T(0));
    }
    void Iteration();
    bool OptimalControlSearch();
    float genS(int i);

    void ClearControlFunctionsValues();
    float CalcQualityCriteriaForCurrentControl();
    float CalcQualityCriteriaForOptimalControl();
    float CalculateControlValue(float p2);
    float CalculateEndConditions();
};

#endif // CONTROLSTRUCTURECORRECTIONMETHOD_H
