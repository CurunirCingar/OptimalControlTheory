#include "controlstructurecorrectionmethod.h"

#include <QMessageBox>

ControlStructureCorrectionMethod::ControlStructureCorrectionMethod(QObject *parent) : QObject(parent)
{
}

void ControlStructureCorrectionMethod::ClearControlFunctionsValues()
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

void ControlStructureCorrectionMethod::SetInitConditions(InitConditions conds)
{
    this->initConditions = conds;
    dt = (conds.tMax - conds.tMin) / (conds.stepsCount - 1);
    qualityCriteriaValues.clear();
    calculateValues.resize(conds.stepsCount);
    calculateValues[0].t = 0;
    auto firstValues = &calculateValues[0];
    firstValues->x1 = conds.x1Start;
    firstValues->x2 = conds.x2Start;
    firstValues->x1Corr = conds.x1Start;
    firstValues->x2Corr = conds.x2Start;

    for(int i = 0; i < conds.stepsCount; ++i)
    {
        auto value = &calculateValues[i];
        value->u = conds.startControl * (i < initConditions.stepsCount/2 ? -1 : 1);
        value->t = i * dt;
    }

    iterationsCount = 0;
}

float ControlStructureCorrectionMethod::Clamp(float min, float max, float value)
{
    if(value < min)
        return min;

    if(value > max)
        return max;

    return value;
}

float ControlStructureCorrectionMethod::CalcQualityCriteriaForCurrentControl()
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
        //        Integral += ((value->x1 * value->x1) + (value->x2 * value->x2)) * dt;
    }

    float endConditions = CalculateEndConditions();
    return endConditions * 0.5 + Integral * 0.5;
    //    return endConditions;
}

float ControlStructureCorrectionMethod::CalcQualityCriteriaForOptimalControl()
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
        Integral += (value->uOptimal * value->uOptimal) /
                    (initConditions.k * initConditions.k) * dt;
        //        Integral += ((value->x1 * value->x1) + (value->x2 * value->x2)) * dt;
    }

    auto lastValues = &calculateValues.last();
    float endConditions =
        initConditions.Ro1 * (lastValues->x1- initConditions.x1End) *
        (lastValues->x1- initConditions.x1End) +
        initConditions.Ro2 * (lastValues->x2- initConditions.x2End) *
        (lastValues->x2 - initConditions.x2End);
    return endConditions * 0.5 + Integral * 0.5;
    //    return endConditions;
}

float ControlStructureCorrectionMethod::CalculateEndConditions()
{
    auto lastValues = &calculateValues.last();
    float endConditions =
        initConditions.Ro1 * (lastValues->x1 - initConditions.x1End) *
        (lastValues->x1 - initConditions.x1End) +
        initConditions.Ro2 * (lastValues->x2 - initConditions.x2End) *
        (lastValues->x2 - initConditions.x2End);
    return endConditions;
}

bool ControlStructureCorrectionMethod::RunIteration()
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

        if(result)
            Iteration();
    }

    iterationsCount++;
    return result;
}

int ControlStructureCorrectionMethod::GetPassedIterationsCount()
{
    return iterationsCount;
}

QList<float> ControlStructureCorrectionMethod::GetIterationsCriterias()
{
    return qualityCriteriaValues;
}

void ControlStructureCorrectionMethod::Iteration()
{
    float criteria = CalcQualityCriteriaForCurrentControl();
    qualityCriteriaValues.append(criteria);
    auto lastValues = &calculateValues.last();
    lastValues->p1 = (lastValues->x1 - initConditions.x1End) * initConditions.Ro1;
    lastValues->p2 = (lastValues->x2 - initConditions.x2End) * initConditions.Ro2;
    lastValues->p3 = 0;
    lastValues->pu = 0;
    //    lastValues->H = lastValues->p1 * lastValues->x2 + lastValues->p2 * lastValues->u + 0.5 * lastValues->u * lastValues->u;
    lastValues->uCorr = CalculateControlValue(-lastValues->p2);

    for(int i = initConditions.stepsCount-2; i >= 0; --i)
    {
        auto prev = &calculateValues[i+1];
        auto cur = &calculateValues[i];
        cur->p1 = prev->p1;
        cur->p2 = prev->p2 - prev->p1 * dt;
        cur->p3 = prev->p3 - prev->p2 * (1 - 2 * Clamp(0, 1, prev->t - initConditions.tSwitch) ) * dt;
        //        cur->pu = prev->pu +
        //        cur->H = cur->p1 * cur->x2 + cur->p2 * cur->u +
        //                 0.5 * cur->u * cur->u / (initConditions.k * initConditions.k);
        cur->uCorr = CalculateControlValue(-cur->p2);
    }
}

float ControlStructureCorrectionMethod::CalculateControlValue(float p2)
{
    auto value = Clamp(initConditions.minControl, initConditions.maxControl, p2);
    //    auto value = p2 < 0 ? initConditions.minControl : initConditions.maxControl;
    return value;// * (initConditions.k * initConditions.k);
}

bool ControlStructureCorrectionMethod::OptimalControlSearch()
{
    int sIter = 0;
    float prevCriteria = qualityCriteriaValues.last();
    float currentCriteria;
    float linearCoef = 0;

    do
    {
        linearCoef = genS(sIter);

        for(int i = 0; i < initConditions.stepsCount; ++i)
        {
            auto value = &calculateValues[i];
            value->uOptimal = value->u + linearCoef * (value->uCorr - value->u);
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

float ControlStructureCorrectionMethod::genS(int i)
{
    float result = 1;

    for(int j=0; j<i; j++)
        result /= 2;

    return result;
}

QVector<ControlStructureCorrectionMethod::CalculateValues> ControlStructureCorrectionMethod::GetCalculateValues()
{
    return calculateValues;
}

QList<float> ControlStructureCorrectionMethod::GetFunctionalValues()
{
    return qualityCriteriaValues;
}
