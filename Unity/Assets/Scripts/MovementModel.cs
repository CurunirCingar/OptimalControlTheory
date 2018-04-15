using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MovementModel : MonoBehaviour
{
    public Vector3 Mu;
    public Vector3 Ru;

    public Vector3 MuLimits = new Vector3(10f, 10f, 10f);
    public Vector3 RuLimits = new Vector3(10f, 10f, 10f);

    #region PropertiesForInspector
    float Clamp(float value, float limit)
    {
        return limit < Math.Abs(value) ? limit * Math.Sign(value) : value;
    }

    public float MuX
    {
        get { return Mu.x; }
        set { Mu.x = Clamp(value, MuLimits.x); }
    }

    public float MuY
    {
        get { return Mu.y; }
        set { Mu.y = Clamp(value, MuLimits.y); }
    }

    public float MuZ
    {
        get { return Mu.z; }
        set { Mu.z = Clamp(value, MuLimits.z); }
    }

    public float RuX
    {
        get { return Ru.x; }
        set { Ru.x = Clamp(value, RuLimits.x); }
    }

    public float RuY
    {
        get { return Ru.y; }
        set { Ru.y = Clamp(value, RuLimits.y); }
    }

    public float RuZ
    {
        get { return Ru.z; }
        set { Ru.z = Clamp(value, RuLimits.z); }
    }
    #endregion

    [System.Serializable]
    public struct Params
    {
        public Vector3 V;
        public Vector3 w;
        public Vector3 dV;
        public Vector3 dw;
        public Vector3 pos;
        public Vector3 rot;
    }

    [SerializeField]
    Params currentParams;
    public Params CurrentParams
    {
        get { return currentParams; }
        private set { currentParams = value; }
    }
    public Action OnMovementUpdate;

    [HideInInspector]
    public List<Params> paramsArray = new List<Params>();
    public const int maxParamsArrayCapacity = 1000;

    #region SetupCoefs
    const float mass = 1000;
    const float radius = 0.1f;
    const float fullLenght = 1.8f;

    float[] lenght = new float[]
    {
        0.135f,
        1.8f - (0.135f + 0.35f),
        0.35f
    };

    enum Lambda
    {
        _11,
        _22,
        _26,
        _33,
        _35,
        _44,
        _55,
        _66
    }
    Dictionary<Lambda, float> Lambdas;

    enum CoefL
    {
        _11,
        _12,
        _22
    }
    Dictionary<CoefL, float> CoefsL;

    Vector3 vecMass;
    Vector3 vecJProjections;
    Vector3 vecJ; 
    #endregion

    void Start ()
    {
        Lambdas = new Dictionary<Lambda, float>() {
            {Lambda._11, 5.736f},
            {Lambda._22, 5.736f},
            {Lambda._26, -5.163f},
            {Lambda._33, 5.736f},
            {Lambda._35, 5.163f},
            {Lambda._44, 0f},
            {Lambda._55, 6.195f},
            {Lambda._66, 6.195f},
        };

        vecMass = new Vector3(
            (1 + Lambdas[Lambda._11]) * mass,
            (1 + Lambdas[Lambda._22]) * mass,
            (1 + Lambdas[Lambda._33]) * mass
        );

        vecJProjections = new Vector3(
            (mass * radius * radius) / 2f + (mass * fullLenght * fullLenght) / 12f,
            (mass * radius * radius) / 2f + (mass * fullLenght * fullLenght) / 12f,
            (mass * radius * radius) / 2f + (mass * fullLenght * fullLenght) / 12f
        );

        vecJ = new Vector3(
            (1 + Lambdas[Lambda._44]) * vecJProjections.x,
            (1 + Lambdas[Lambda._55]) * vecJProjections.y,
            (1 + Lambdas[Lambda._66]) * vecJProjections.z
        );
    }
	
	void FixedUpdate ()
    {
        float dt = Time.fixedDeltaTime;
        var newParams = UpdateMovement(CurrentParams, Mu, Ru, dt);
        paramsArray.Add(newParams);
        if (paramsArray.Count > maxParamsArrayCapacity)
            paramsArray.RemoveAt(0);

        CurrentParams = newParams;
        if (OnMovementUpdate != null)
            OnMovementUpdate();
    }

    public Params UpdateMovement(Params prev, Vector3 Mu, Vector3 Ru, float dt)
    {
        Params cur = prev;

        cur.dV.x = (
                (
                    Ru.x - 
                    vecMass.z * prev.w.y * prev.V.z + 
                    vecMass.y * prev.w.x * prev.V.y -
                    Lambdas[Lambda._35] * prev.w.y * prev.w.y +
                    Lambdas[Lambda._26] * prev.w.z * prev.w.z
                ) / vecMass.x
        );

        cur.dV.y = (
                (
                    Ru.y - 
                    vecMass.x * prev.w.z * prev.V.x +
                    vecMass.z * prev.w.x * prev.V.z +
                    Lambdas[Lambda._35] * prev.w.x * prev.w.y -
                    Lambdas[Lambda._26] * cur.dw.z / dt
                ) / vecMass.y
        );

        cur.dV.z = (
                (
                    Ru.z - 
                    vecMass.y * prev.w.x * prev.V.y +
                    vecMass.x * prev.w.y * prev.V.x -
                    Lambdas[Lambda._26] * prev.w.x * prev.w.z -
                    Lambdas[Lambda._35] * cur.dw.y / dt
                ) / vecMass.z
        );

        cur.dw.x = (
                    (Mu.x -
                    (Lambdas[Lambda._26] + Lambdas[Lambda._35]) *
                    (prev.w.y * prev.V.y - prev.w.z * prev.V.z)
                ) / vecJ.x
        );

        cur.dw.y = (
                    (Mu.y -
                    (Lambdas[Lambda._35] * prev.dV.z / dt) -
                    prev.w.x * prev.w.y * (vecJ.x - vecJ.z) -
                    prev.V.x * prev.V.z * (vecMass.x - vecMass.z) +
                    Lambdas[Lambda._26] * prev.w.x * prev.V.y +
                    Lambdas[Lambda._35] * prev.w.y * prev.V.x
                ) / vecJ.y
        );

        cur.dw.z = (
                (
                    Mu.z -
                    Lambdas[Lambda._26] * prev.dV.y / dt -
                    prev.w.x * prev.w.y * (vecJ.y - vecJ.x) - 
                    prev.V.x * prev.V.y * (vecMass.y - vecMass.x) -
                    Lambdas[Lambda._35] * prev.w.x * prev.V.z -
                    Lambdas[Lambda._26] * prev.w.z * prev.V.x
                ) / vecJ.z
        );

        cur.V = cur.dV * dt;
        cur.w = cur.dw * dt;

        transform.Translate(prev.dV);
        transform.Rotate(prev.dw);

        cur.pos = transform.position;
        cur.rot = transform.rotation.eulerAngles;

        return cur;
    }
}
