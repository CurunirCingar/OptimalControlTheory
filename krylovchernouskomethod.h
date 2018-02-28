#ifndef KRYLOVCHERNOUSKOMETHOD_H
#define KRYLOVCHERNOUSKOMETHOD_H

#include <QVector>



class KrylovChernouskoMethod
{
public:
    KrylovChernouskoMethod();

    struct InitConditions
    {
        float x1Start;
        float x2Start;
        float x1End;
        float x2End;
        float tMin;
        float tMax;
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
        CalculateValues() : t(0), x1(0), x2(0), p1(0), p2(0), u(0) {}
        float t;
        float x1;
        float x2;
        float p1;
        float p2;
        float u;
        float uCorr;
        float uOptimal;
        float H;
    };

    void SetInitConditions(InitConditions initConditions);
    bool RunIteration();
    int GetPassedIterationsCount();

    QVector<CalculateValues> GetCalculateValues();
    QList<float> GetFunctionalValues();

private:
    InitConditions initConditions;
    QVector<CalculateValues> calculateValues;
    float dt;
    int iterationsCount;
    QList<float> qualityCriteriaValues;
    const float EPSILON = 0.00001;
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

#endif // KRYLOVCHERNOUSKOMETHOD_H
