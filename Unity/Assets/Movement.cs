using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Movement : MonoBehaviour {

    public Vector3 Mu;
    public Vector3 Ru;
    public const float mass = 1000;
    public const float radius = 0.1f;
    public const float fullLenght = 1.8f;

    

    public float[] lenght = new float[]
    {
        0.135f,
        1.8f - (0.135f + 0.35f),
        0.35f
    };

    public enum Lambda
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
    public Dictionary<Lambda, float> Lambdas;

    public enum CoefL
    {
        _11,
        _12,
        _22
    }
    public Dictionary<CoefL, float> CoefsL;

    public Vector3 vecMass;
    public Vector3 vecJProjections;
    public Vector3 vecJ;

    [System.Serializable]
    public struct Params
    {
        public Vector3 V;
        public Vector3 w;
        public Vector3 dV;
        public Vector3 dw;
        public Vector3 rot;
    }

    [SerializeField]
    Params cur;
    [SerializeField]
    Params prev;

    void Start ()
    {
        Lambdas = new Dictionary<Lambda, float>() {
            {Lambda._11, 0f},
            {Lambda._22, 5.736f},
            {Lambda._26, 5.736f},
            {Lambda._33, 0f},
            {Lambda._35, 5.163f},
            {Lambda._44, 0f},
            {Lambda._55, 6.195f},
            {Lambda._66, 0f},
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

        //CoefsL = new Dictionary<CoefL, float>();
        //CoefsL.Add(CoefL._11,
        //    (Jyy + Lambdas[Lambda._55]) /
        //    (
        //        (mass * Lambdas[Lambda._22]) * (Jyy + Lambdas[Lambda._55]) -
        //        (Lambdas[Lambda._35] * Lambdas[Lambda._35])
        //    )
        //);
        //CoefsL.Add(CoefL._12,
        //    (Lambdas[Lambda._35]) /
        //    (
        //        (mass * Lambdas[Lambda._22]) * (Jyy + Lambdas[Lambda._55]) -
        //        (Lambdas[Lambda._35] * Lambdas[Lambda._35])
        //    )
        //);
        //CoefsL.Add(CoefL._22,
        //    (mass + Lambdas[Lambda._22]) /
        //    (
        //        (mass * Lambdas[Lambda._22]) * (Jyy + Lambdas[Lambda._55]) -
        //        (Lambdas[Lambda._35] * Lambdas[Lambda._35])
        //    )
        //);
        //CoefsL.Add(CoefL._11, 1f);
        //CoefsL.Add(CoefL._12, 1f);
        //CoefsL.Add(CoefL._22, 1f);
    }
	
	void FixedUpdate ()
    {
        cur = UpdateMovement(cur, Mu, Ru);
    }

    public Params UpdateMovement(Params prev, Vector3 Mu, Vector3 Ru)
    {
        Params cur = prev;
        float dt = Time.fixedDeltaTime;

        cur.dV.x = (
                (Ru.x - vecMass.z * prev.w.y * prev.V.z + 
                vecMass.y * prev.w.x * prev.V.y -
                Lambdas[Lambda._35] * prev.w.y * prev.w.y +
                Lambdas[Lambda._26] * prev.w.z * prev.w.z) / vecMass.x
        );

        cur.dV.y = dt * (
                (Ru.y - vecMass.x * prev.w.z * prev.V.x +
                vecMass.z * prev.w.x * prev.V.z -
                Lambdas[Lambda._35] * prev.w.x * prev.w.y +
                Lambdas[Lambda._26] * cur.dw.z / dt) / vecMass.y
        );

        cur.dV.z = (
                (Ru.z - vecMass.y * prev.w.x * prev.V.y +
                vecMass.y * prev.w.x * prev.V.y -
                Lambdas[Lambda._26] * prev.w.x * prev.w.z +
                Lambdas[Lambda._35] * cur.dw.y / dt) / vecMass.z
        );

        cur.dw.x = (
                (Mu.x -
                (Lambdas[Lambda._26] + Lambdas[Lambda._35]) *
                (prev.w.y * prev.V.y - prev.w.z * prev.V.z)) / vecJ.x
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
                (Mu.z -
                (Lambdas[Lambda._26] * prev.dV.y / dt -
                prev.w.x * prev.w.y * (vecJ.y - vecJ.x) - 
                prev.V.x * prev.V.y * (vecMass.y - vecMass.x) -
                Lambdas[Lambda._35] * prev.w.x * prev.V.z -
                Lambdas[Lambda._26] * prev.w.z * prev.V.x
                + Lambdas[Lambda._35]) *
                (prev.w.y * prev.V.y - prev.w.z * prev.V.z)) / vecJ.z
        );

        cur.V += cur.dV * dt;
        cur.w += cur.dw * dt;

        transform.Translate(new Vector3(transform.forward.x * prev.dV.x, transform.forward.y * prev.dV.y, transform.forward.z * prev.dV.z));
        transform.Rotate(prev.dw);

        //cur.Vx += Delta(
        //    -(mass + Lambdas[Lambda._22]) / (mass + Lambdas[Lambda._11]) * prev.Vz * prev.wy -
        //    Lambdas[Lambda._35] / (mass + Lambdas[Lambda._55]) * prev.wy * prev.wy +
        //    Rxu / (mass + Lambdas[Lambda._11])
        //);
        //cur.Vz += Delta(
        //    CoefsL[CoefL._11] * (mass + Lambdas[Lambda._11]) * prev.Vx * prev.wy +
        //    CoefsL[CoefL._12] * ( -(Lambdas[Lambda._11] - Lambdas[Lambda._22]) * prev.Vx * prev.Vz + Lambdas[Lambda._35] * prev.Vx * prev.wy ) +
        //    CoefsL[CoefL._12] * Myu
        //);
        //cur.wy += Delta(
        //    CoefsL[CoefL._12] * (mass + Lambdas[Lambda._11]) * prev.Vx * prev.wy +
        //    CoefsL[CoefL._22] * ( -(Lambdas[Lambda._11] - Lambdas[Lambda._22]) * prev.Vx * prev.Vz + Lambdas[Lambda._35] * prev.Vx * prev.wy ) +
        //    CoefsL[CoefL._22] * Myu
        //);
        //cur.phi += Delta(prev.wy);
        //cur.x += Delta(prev.Vx * Mathf.Cos(prev.phi) - prev.Vz * Mathf.Sin(prev.phi));
        //cur.z += Delta(-prev.Vx * Mathf.Sin(prev.phi) + prev.Vz * Mathf.Cos(prev.phi));
        return cur;
    }

    float Delta(float value)
    {
        float dt = Time.fixedDeltaTime;
        return value * dt;
    }


}
