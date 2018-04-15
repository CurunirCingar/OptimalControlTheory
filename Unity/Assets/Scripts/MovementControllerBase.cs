using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MovementControllerBase : MonoBehaviour
{
    protected MovementModel model;

    [SerializeField]
    private bool controllerEnabled = true;

    public Transform target;
    public float ReachDistance;
    public Action OnTargetReached;

    void Awake()
    {
        model = GetComponent<MovementModel>();
        model.OnMovementUpdate += () =>
        {
            if (controllerEnabled)
                UpdateMovement();
            if (Vector3.Distance(transform.position, target.position) < ReachDistance &&
                OnTargetReached != null)
                OnTargetReached();
        };
        OnAwake();
    }

    protected virtual void OnAwake()
    {

    }

    public virtual void UpdateMovement()
    {

    }
}
