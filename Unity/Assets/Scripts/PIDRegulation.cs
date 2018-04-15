using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(MovementModel))]
public class PIDRegulation : MovementControllerBase
{
    public float P;
    public float I;
    public float D;

    public Vector3 posError;
    public Vector3 rotError;

    private class PIDValueRegulator
    {
        public static float P;
        public static float I;
        public static float D;

        private Func<float> errorFunc;
        private Action<float> regulationFunc;
        private float errorAccum;
        private const float accumMax = 1000f;
        private float prevError;

        public PIDValueRegulator(Func<float> errorFunc, Action<float> regulationFunc)
        {
            this.errorFunc = errorFunc;
            this.regulationFunc = regulationFunc;
        }

        public void UpdateRegulation()
        {
            float error = errorFunc();
            errorAccum += error;
            var res = P * error + I * errorAccum + D * (error - prevError);
            prevError = error;
            regulationFunc(res);
        }
    }

    private List<PIDValueRegulator> valuesRegulator = new List<PIDValueRegulator>();

    protected override void OnAwake()
    {
        base.OnAwake();
        
        valuesRegulator.Add(new PIDValueRegulator(
            () => { return rotError.x; },
            (float val) => { model.MuX = val; })
        );
        valuesRegulator.Add(new PIDValueRegulator(
            () => { return rotError.y; },
            (float val) => { model.MuY = val; })
        );
        valuesRegulator.Add(new PIDValueRegulator(
            () => { return rotError.z; },
            (float val) => { model.MuZ = val; })
        );

        valuesRegulator.Add(new PIDValueRegulator(
            () => { return posError.x; },
            (float val) => { model.RuX = val; })
        );
        valuesRegulator.Add(new PIDValueRegulator(
            () => { return posError.y; },
            (float val) => { model.RuY = val; })
        );
        valuesRegulator.Add(new PIDValueRegulator(
            () => { return posError.z; },
            (float val) => { model.RuZ = val; })
        );
    }

    public override void UpdateMovement()
    {
        PIDValueRegulator.P = P;
        PIDValueRegulator.I = I;
        PIDValueRegulator.D = D;

        posError = target.position - transform.position;
        var q = Quaternion.LookRotation(target.position - transform.position);
        rotError = (q.eulerAngles - transform.eulerAngles);
        rotError.x = NormalizeValue(rotError.x);
        rotError.y = NormalizeValue(rotError.y);
        rotError.z = NormalizeValue(rotError.z);
        Debug.Log(rotError);
        foreach (var reg in valuesRegulator)
            reg.UpdateRegulation();

    }

    float NormalizeValue(float value)
    {
        if (Math.Abs(value) > 180)
            return -Math.Sign(value) * 180 + value % 180;
        else
            return value;
    }

}
