#include "krylovchernouskomethod.h"

#include <QDebug>
#include <QMessageBox>

KrylovChernouskoMethod::KrylovChernouskoMethod()
{
}

void KrylovChernouskoMethod::SetInitConditions(InitConditions initConditions)
{
    this->initConditions = initConditions;
    iterationsCount = 0;
    auto n = initConditions.stepsCount;
    dt = (initConditions.tMax - initConditions.tMin) / n;
    calculateValues.clear();
    calculateValues.resize(n);
    auto initValues = &calculateValues[0];
    initValues->x1 = initConditions.x1Start;
    initValues->x2 = initConditions.x2Start;
    initValues->t = 0;

    for(int i = 0; i < n; ++i)
        calculateValues[i].u = initConditions.startControl;
}

float KrylovChernouskoMethod::Clamp(float min, float max, float value)
{
    if(value < min)
        return min;

    if(value > max)
        return max;

    return value;
}

float KrylovChernouskoMethod::CalcFunctional()
{
    auto firstValues = &calculateValues.first();
    auto lastValues = &calculateValues.last();
    float Functional = (lastValues->x1 - initConditions.x1End) * (lastValues->x1 - initConditions.x1End) +
                       (lastValues->x2 - initConditions.x2End) * (lastValues->x2 - initConditions.x2End);
    return Functional;
}

bool KrylovChernouskoMethod::RunIteration()
{
    bool result;

    if(iterationsCount == 0)
    {
        Iteration();
        float Functional = CalcFunctional();
        float result = 0;

        for(int i=0; i<initConditions.stepsCount; i++)
        {
            auto value = &calculateValues[i];
            result += value->u*value->uOptimal*dt;
        }

        Functional = Functional * 0.5 + result * 0.5;
        functionalValues.append(Functional);
        result = true;
    }
    else
    {
        result = OptimalControlSearch();
        Iteration();
        float Functional = CalcFunctional();
        float result = 0;

        for(int i=0; i<initConditions.stepsCount; i++)
        {
            auto value = &calculateValues[i];
            result += value->u*value->uOptimal*dt;
        }

        Functional = Functional * 0.5 + result * 0.5;
        functionalValues.append(Functional);
    }

    iterationsCount++;
    return result;
}

void KrylovChernouskoMethod::Iteration()
{
    for(int i = 1, total = initConditions.stepsCount; i < total; ++i)
    {
        auto prev = &calculateValues[i-1];
        auto cur = &calculateValues[i];
        cur->x1 = prev->x1 + prev->x2 * dt;
        cur->x2 = prev->x2 + prev->u * dt;
        cur->t = prev->t+ dt;
    }

    auto firstValues = &calculateValues.first();
    auto lastValues = &calculateValues.last();
    lastValues->p1 = lastValues->x1 - initConditions.x1End;
    lastValues->p2 = lastValues->x2 - initConditions.x2End;
    lastValues->H = lastValues->p1 * lastValues->x2 + lastValues->p2 * lastValues->u;
    lastValues->uCorr = lastValues->p2 > 0 ? initConditions.minControl : initConditions.maxControl;

    for(int i = initConditions.stepsCount-2; i >= 0; --i)
    {
        auto prev = &calculateValues[i+1];
        auto cur = &calculateValues[i];
        cur->p1 = prev->p1;
        cur->p2 = prev->p2 - prev->p1 * dt;
        cur->H = cur->p1 * cur->x2 + cur->p2 * cur->u;
        cur->uCorr = lastValues->p2 > 0 ? initConditions.minControl : initConditions.maxControl;
    }
}

bool KrylovChernouskoMethod::OptimalControlSearch()
{
    int sIter = 0;
    float prevFunctional = functionalValues.last();
    float Functional;

    do
    {
        for(int i = 0; i < initConditions.stepsCount; ++i)
        {
            auto value = &calculateValues[i];
            value->uOptimal = value->u + genS(sIter) * (value->uCorr - value->u);
        }

        for(int i = 1, total = initConditions.stepsCount; i < total; ++i)
        {
            auto prev = &calculateValues[i-1];
            auto cur = &calculateValues[i];
            cur->x1 = prev->x1 + prev->x2 * dt;
            cur->x2 = prev->x2 + prev->uOptimal * dt;
            cur->t = prev->t + dt;
        }

        Functional = CalcFunctional();
        float result = 0;

        for(int i=0; i<initConditions.stepsCount; i++)
        {
            auto value = &calculateValues[i];
            result += value->uOptimal*value->uOptimal*dt;
        }

        Functional = Functional * 0.5 + result * 0.5;

        if(fabs(prevFunctional - Functional) < EPSILON)
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
    while(prevFunctional < Functional);

    for(int i = 0; i < initConditions.stepsCount; ++i)
    {
        auto value = &calculateValues[i];
        value->x1 = 0;
        value->x2 = 0;
        value->p1 = 0;
        value->p2 = 0;
        value->u = value->uOptimal;
    }

    auto initValues = &calculateValues[0];
    initValues->x1 = initConditions.x1Start;
    initValues->x2 = initConditions.x2Start;
    initValues->t = 0;
    //    functionalValues.append(Functional);
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
    return functionalValues;
}























