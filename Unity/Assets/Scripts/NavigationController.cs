using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class NavigationController : MonoBehaviour {

    [SerializeField]
    MovementControllerBase controller;
    [SerializeField]
    GameObject cameraObject;

    [Range(1f, 1000f)]
    public int GeneratedPointsCount = 15;
    public float PointsGenerationCubeSide = 100f;
    public bool GeneratePathPoints = false;
    private bool isPointsGenerated;

    public List<Transform> DefaultPathPoints;
    struct CachedTransformPosition
    {
        public Transform t;
        public Vector3 p;
        public CachedTransformPosition(Transform t)
        {
            this.t = t;
            p = t.position;
        }
    }
    private List<CachedTransformPosition> pathPoints = new List<CachedTransformPosition>();
    private int currentPoint;

    [SerializeField]
    private Material targetMat;
    [SerializeField]
    private Material notTargetMat;
    [SerializeField]
    private GameObject pathPointPrefab;

    private void Start()
    {
        if(!controller)
            controller = FindObjectOfType<MovementControllerBase>();

        foreach (var p in DefaultPathPoints)
        {
            p.gameObject.GetComponent<MeshRenderer>().material = notTargetMat;
            p.gameObject.SetActive(false);
        }

        controller.OnTargetReached += SetNextPoint;
        controller.OnPlaneRestrictionUpdate += UpdatePlaneRestriction;

        Restart();
    }

    private void SetNextPoint()
    {
        if (currentPoint >= pathPoints.Count)
            Restart();

        if (controller.target)
            controller.target.gameObject.GetComponent<MeshRenderer>().material = notTargetMat;

        controller.target = pathPoints[currentPoint].t;
        controller.target.gameObject.GetComponent<MeshRenderer>().material = targetMat;
        currentPoint++;
    }

    [ContextMenu("Restart")]
    public void Restart()
    {
        if(isPointsGenerated)
        {
            for (int i = 0; i < pathPoints.Count; i++)
                Destroy(pathPoints[i].t.gameObject);
        }
        pathPoints.Clear();

        if (GeneratePathPoints)
        {
            Vector3 startPoint = controller.transform.position;
            var halfSide = PointsGenerationCubeSide / 2f;
            for (int i = 0; i < GeneratedPointsCount; i++)
            {
                Vector3 newPoint = new Vector3(
                    Random.Range(0, PointsGenerationCubeSide) - halfSide,
                    Random.Range(0, PointsGenerationCubeSide) - halfSide,
                    Random.Range(0, PointsGenerationCubeSide) - halfSide
                );
                newPoint = startPoint + newPoint;
                var newPointObj = GameObject.Instantiate(pathPointPrefab, newPoint, Quaternion.identity);
                var cachedPos = new CachedTransformPosition(newPointObj.transform);
                pathPoints.Add(cachedPos);
            }
        }
        else
        {
            foreach (var p in DefaultPathPoints)
            {
                var newPointObj = GameObject.Instantiate(pathPointPrefab, p);
                var cachedPos = new CachedTransformPosition(newPointObj.transform);
                pathPoints.Add(cachedPos);
            }
        }
        UpdatePlaneRestriction(controller.RestrictPlane);
        isPointsGenerated = true;
        currentPoint = 0;
        SetNextPoint();
    }

    private void UpdatePlaneRestriction(MovementControllerBase.PlaneRestrictionType type)
    {
        float distance = 35f;
        switch (type)
        {
            case MovementControllerBase.PlaneRestrictionType.None:
                cameraObject.transform.localPosition = new Vector3(0f, 10f, -distance);
                foreach (var point in pathPoints)
                    point.t.position = point.p;
                break;
            case MovementControllerBase.PlaneRestrictionType.X_Y:
                cameraObject.transform.localPosition = new Vector3(0f, 0f, -distance);
                controller.transform.position =
                    new Vector3(controller.transform.position.x, controller.transform.position.y, 0f);
                foreach (var point in pathPoints)
                    point.t.position = new Vector3(point.p.x, point.p.y, 0f);
                break;
            case MovementControllerBase.PlaneRestrictionType.X_Z:
                cameraObject.transform.localPosition = new Vector3(0f, distance, 0f);
                controller.transform.position =
                    new Vector3(controller.transform.position.x, 0f, controller.transform.position.z);
                foreach (var point in pathPoints)
                    point.t.position = new Vector3(point.p.x, 0f, point.p.z);
                break;
            case MovementControllerBase.PlaneRestrictionType.Y_Z:
                cameraObject.transform.localPosition = new Vector3(-distance, 0f, 0f);
                controller.transform.position = 
                    new Vector3(0f, controller.transform.position.y, controller.transform.position.z);
                foreach (var point in pathPoints)
                    point.t.position = new Vector3(0f, point.p.y, point.p.z);
                break;
        }
        cameraObject.transform.LookAt(controller.transform);
        controller.transform.rotation = Quaternion.Euler(0, 0, 0);
    }
}
