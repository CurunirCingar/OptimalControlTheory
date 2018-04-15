using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Reflection;

public class GUIManager : MonoBehaviour
{
    MovementModel model;

    public Slider[] Sliders;
    public bool updateValues = true;

    private void Awake()
    {
        model = FindObjectOfType<MovementModel>();
        model.OnMovementUpdate += UpdateGUI;

        foreach(var s in Sliders)
        {
            s.minValue = -model.MuLimits.x;
            s.maxValue = model.MuLimits.x;
        }
    }

    private void UpdateGUI()
    {
        if (!updateValues)
            return;
        Sliders[0].value = model.MuX;
        Sliders[1].value = model.MuY;
        Sliders[2].value = model.MuZ;
        Sliders[3].value = model.RuX;
        Sliders[4].value = model.RuY;
        Sliders[5].value = model.RuZ;
    }
}
