using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class Example : MonoBehaviour
{
    public SimplestPlot.PlotType PlotExample = SimplestPlot.PlotType.TimeSeries;
    public int DataPoints = 100;
    public float updateDelta = 0.3f;
    private float updateDeltaBuf;
    private SimplestPlot SimplestPlotScript;
    private float Counter = 0;
    private Color[] MyColors = new Color[2];

    private System.Random MyRandom;
    private List<float> XValues = new List<float>();
    private List<float> Y1Values = new List<float>();
    private List<float> Y2Values = new List<float>();

    public Transform target;

    private Vector2 Resolution;
    // Use this for initialization
    void Start()
    {
        SimplestPlotScript = GetComponent<SimplestPlot>();

        MyRandom = new System.Random();
        MyColors[0] = Color.white;
        MyColors[1] = Color.blue;

        SimplestPlotScript.SetResolution(new Vector2(300, 300));
        SimplestPlotScript.BackGroundColor = new Color(0.1f, 0.1f, 0.1f, 0.4f);
        SimplestPlotScript.TextColor = Color.yellow;
        for (int Cnt = 0; Cnt < 2; Cnt++)
        {
            SimplestPlotScript.SeriesPlotY.Add(new SimplestPlot.SeriesClass());
            SimplestPlotScript.DistributionPlot.Add(new SimplestPlot.DistributionClass());
            SimplestPlotScript.PhaseSpacePlot.Add(new SimplestPlot.PhaseSpaceClass());

            SimplestPlotScript.SeriesPlotY[Cnt].MyColor = MyColors[Cnt];
            SimplestPlotScript.DistributionPlot[Cnt].MyColor = MyColors[Cnt];
            SimplestPlotScript.PhaseSpacePlot[Cnt].MyColor = MyColors[Cnt];

            SimplestPlotScript.DistributionPlot[Cnt].NumberOfBins = (Cnt + 1) * 5;
        }

        Resolution = SimplestPlotScript.GetResolution();
    }

    // Update is called once per frame
    void Update()
    {
        updateDeltaBuf += Time.deltaTime;
        if (updateDeltaBuf < updateDelta)
            return;
        updateDeltaBuf = 0f;

        Counter++;
        PrepareArrays();
        SimplestPlotScript.MyPlotType = PlotExample;
        switch (PlotExample)
        {
            case SimplestPlot.PlotType.TimeSeries:
                SimplestPlotScript.SeriesPlotY[0].YValues = Y1Values.ToArray();
                SimplestPlotScript.SeriesPlotY[1].YValues = Y2Values.ToArray();
                SimplestPlotScript.SeriesPlotX = XValues.ToArray();
                break;
            case SimplestPlot.PlotType.Distribution:
                SimplestPlotScript.DistributionPlot[0].Values = Y1Values.ToArray();
                SimplestPlotScript.DistributionPlot[1].Values = Y2Values.ToArray();
                break;
            case SimplestPlot.PlotType.PhaseSpace:
                SimplestPlotScript.PhaseSpacePlot[0].XValues = XValues.ToArray();
                SimplestPlotScript.PhaseSpacePlot[0].YValues = Y1Values.ToArray();
                //SimplestPlotScript.PhaseSpacePlot[1].XValues = Y1Values.ToArray();
                //SimplestPlotScript.PhaseSpacePlot[1].YValues = Y2Values.ToArray();
                break;
            default:
                break;
        }
        SimplestPlotScript.UpdatePlot();
    }
    private void PrepareArrays()
    {
        XValues.Add(Counter);
        Y1Values.Add(target.position.x);
        Y2Values.Add(target.position.z);
        if(XValues.Count > DataPoints)
        {
            XValues.RemoveAt(0);
            Y1Values.RemoveAt(0);
            Y2Values.RemoveAt(0);
        }
    }
}
