#include "krylovchernouskomethod.h"

#include <QDebug>
#include <QMessageBox>

KrylovChernouskoMethod::KrylovChernouskoMethod()
{
}

void KrylovChernouskoMethod::ClearControlFunctionsValues()
{
    auto firstValues = &calculateValues[0];
    firstValues->x1 = initConditions.x1Start;
    firstValues->x2 = initConditions.x2Start;

    for(int i = 1; i < initConditions.stepsCount; ++i)
    {
        auto value = &calculateValues[i];
        value->x1 = 0;
        value->x2 = 0;
    }
}

void KrylovChernouskoMethod::SetInitConditions(InitConditions initConditions)
{
    this->initConditions = initConditions;
    dt = (initConditions.tMax - initConditions.tMin) / initConditions.stepsCount;
    calculateValues.resize(initConditions.stepsCount);
    calculateValues[0].t = 0;
    auto firstValues = &calculateValues[0];
    firstValues->x1 = initConditions.x1Start;
    firstValues->x2 = initConditions.x2Start;

    for(int i = 0; i < initConditions.stepsCount; ++i)
    {
        auto value = &calculateValues[i];
        value->u = initConditions.startControl;
        value->t = i * dt;
    }

    iterationsCount = 0;
}

float KrylovChernouskoMethod::Clamp(float min, float max, float value)
{
    if(value < min)
        return min;

    if(value > max)
        return max;

    return value;
}

float KrylovChernouskoMethod::CalcQualityCriteriaForCurrentControl()
{
    for(int i = 1, total = initConditions.stepsCount; i < total; ++i)
    {
        auto prev = &calculateValues[i-1];
        auto cur = &calculateValues[i];
        cur->x1 = prev->x1 + prev->x2 * dt;
        cur->x2 = prev->x2 + prev->u * dt;
    }

    float Integral = 0;

    for(int i=0; i<initConditions.stepsCount; i++)
    {
        auto value = &calculateValues[i];
        Integral += (value->u * value->u) / (initConditions.k * initConditions.k) * dt;
    }

    float endConditions = CalculateEndConditions();
    return endConditions * 0.5 + Integral * 0.5;
}



float KrylovChernouskoMethod::CalcQualityCriteriaForOptimalControl()
{
    for(int i = 1, total = initConditions.stepsCount; i < total; ++i)
    {
        auto prev = &calculateValues[i-1];
        auto cur = &calculateValues[i];
        cur->x1 = prev->x1 + prev->x2 * dt;
        cur->x2 = prev->x2 + prev->uOptimal * dt;
    }

    float Integral = 0;

    for(int i=0; i<initConditions.stepsCount; i++)
    {
        auto value = &calculateValues[i];
        Integral += (value->uOptimal * value->uOptimal) / (initConditions.k * initConditions.k) * dt;
    }

    float endConditions = CalculateEndConditions();
    return endConditions * 0.5 + Integral * 0.5;
}

float KrylovChernouskoMethod::CalculateEndConditions()
{
    auto lastValues = &calculateValues.last();
    float endConditions =
        initConditions.Ro1 * (lastValues->x1 - initConditions.x1End) *
        (lastValues->x1 - initConditions.x1End) +
        initConditions.Ro2 * (lastValues->x2 - initConditions.x2End) *
        (lastValues->x2 - initConditions.x2End);
    return endConditions;
}

bool KrylovChernouskoMethod::RunIteration()
{
    bool result;

    if(iterationsCount == 0)
    {
        Iteration();
        result = true;
    }
    else
    {
        result = OptimalControlSearch();
        Iteration();
    }

    iterationsCount++;
    return result;
}

int KrylovChernouskoMethod::GetPassedIterationsCount()
{
    return iterationsCount;
}

void KrylovChernouskoMethod::Iteration()
{
    float criteria = CalcQualityCriteriaForCurrentControl();
    qualityCriteriaValues.append(criteria);
    auto lastValues = &calculateValues.last();
    lastValues->p1 = (lastValues->x1 - initConditions.x1End) * initConditions.Ro1;
    lastValues->p2 = (lastValues->x2 - initConditions.x2End) * initConditions.Ro2;
    lastValues->H = lastValues->p1 * lastValues->x2 + lastValues->p2 * lastValues->u;
    lastValues->uCorr = CalculateControlValue(-lastValues->p2);

    for(int i = initConditions.stepsCount-2; i >= 0; --i)
    {
        auto prev = &calculateValues[i+1];
        auto cur = &calculateValues[i];
        cur->p1 = prev->p1;
        cur->p2 = prev->p2 - prev->p1 * dt;
        cur->H = cur->p1 * cur->x2 + cur->p2 * cur->u;
        cur->uCorr = CalculateControlValue(-cur->p2);
    }
}

float KrylovChernouskoMethod::CalculateControlValue(float p2)
{
    auto value = Clamp(initConditions.minControl, initConditions.maxControl, p2);
    return value * (initConditions.k * initConditions.k);
}

bool KrylovChernouskoMethod::OptimalControlSearch()
{
    int sIter = 0;
    float prevCriteria = qualityCriteriaValues.last();
    float currentCriteria;

    do
    {
        for(int i = 0; i < initConditions.stepsCount; ++i)
        {
            auto value = &calculateValues[i];
            value->uOptimal = value->u + genS(sIter) * (value->uCorr - value->u);
        }

        currentCriteria = CalcQualityCriteriaForOptimalControl();

        if(fabs(prevCriteria - currentCriteria ) < EPSILON)
        {
            QMessageBox::warning(NULL, "Внимание!", QString("Дальнейшая отимизация невозможна: "
                                 "изменение функционала при итерациях "
                                 "изменяется менее чем на %1.").arg(EPSILON));
            return false;
        }

        if(sIter > optimalSearchIterationsLimit)
        {
            QMessageBox::warning(NULL, "Внимание!", QString("Дальнейшая отимизация невозможна: "
                                 "количество итераций превысило %1").arg(optimalSearchIterationsLimit));
            return false;
        }

        sIter++;
    }
    while(prevCriteria < currentCriteria );

    for(int i = 0; i < initConditions.stepsCount; ++i)
    {
        auto value = &calculateValues[i];
        value->u = value->uOptimal;
    }

    //ClearControlFunctionsValues();
    return true;
}

float KrylovChernouskoMethod::genS(int i)
{
    float result = 1;

    for(int j=0; j<i; j++)
        result /= 2;

    return result;
}

QVector<KrylovChernouskoMethod::CalculateValues> KrylovChernouskoMethod::GetCalculateValues()
{
    return calculateValues;
}

QList<float> KrylovChernouskoMethod::GetFunctionalValues()
{
    return qualityCriteriaValues;
}























