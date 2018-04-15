using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class NavigationController : MonoBehaviour {

    [SerializeField]
    MovementControllerBase controller;

    public Transform[] PathPoints;
    private int currentPoint;

    [SerializeField]
    private Material targetMat;
    [SerializeField]
    private Material notTargetMat;

    private void Awake()
    {
        if(!controller)
            controller = FindObjectOfType<MovementControllerBase>();

        foreach (var p in PathPoints)
            p.gameObject.GetComponent<MeshRenderer>().material = notTargetMat;

        currentPoint = 0;
        SetNextPoint();
        controller.OnTargetReached += SetNextPoint;
    }

    private void SetNextPoint()
    {
        if (currentPoint >= PathPoints.Length)
            return;

        if (controller.target)
            controller.target.gameObject.GetComponent<MeshRenderer>().material = notTargetMat;

        controller.target = PathPoints[currentPoint];
        controller.target.gameObject.GetComponent<MeshRenderer>().material = targetMat;
        currentPoint++;
    }
}
