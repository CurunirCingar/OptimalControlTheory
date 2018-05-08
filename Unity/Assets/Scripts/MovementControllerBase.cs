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

    public enum PlaneRestrictionType
    {
        None,
        X_Y,
        X_Z,
        Y_Z,
    }
    public Action<PlaneRestrictionType> OnPlaneRestrictionUpdate;
    private PlaneRestrictionType restrictPlane;
    public PlaneRestrictionType RestrictPlane
    {
        get { return restrictPlane; }
        set
        {
            if (value == restrictPlane)
                return;
            restrictPlane = value;
            if (OnPlaneRestrictionUpdate != null)
                OnPlaneRestrictionUpdate(restrictPlane);
            model.Mu = Vector3.zero;
            model.Ru = Vector3.zero;
        }
    }

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
