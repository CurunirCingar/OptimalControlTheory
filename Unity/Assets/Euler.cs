using System;
using F = System.Func<double, double[], double>;
using Fi = System.Func<int, double[], double>;

namespace StructCorrection
{
    class Euler
    {
        public static double[][] Integrate(double[] x, F[] f, double[] f0)
        {
            double[][] result = new double[f.Length][];
            if (f.Length != f0.Length || x.Length == 0) return result;
            for (int i = 0; i < f.Length; i++)
            {
                result[i] = new double[x.Length];
                result[i][0] = f0[i];
            }

            double[] fi_1 = new double[f.Length];
            for (int i = 0; i < f.Length; i++) fi_1[i] = result[i][0];

            for (int i = 1; i < x.Length; i++)
            {
                for (int j = 0; j < f.Length; j++)
                {
                    result[j][i] = (x[i] - x[i - 1]) * f[j](x[i - 1], fi_1) + fi_1[j];
                }
                for (int j = 0; j < f.Length; j++) fi_1[j] = result[j][i];
            }
            return result;
        }

        public static double[][] Integrate(double[] x, int x0, int xf, Fi[] f, double[] f0)
        {
            double[][] result = new double[f.Length][];
            if (f.Length != f0.Length) return result;
            if (x0 < 0 || xf >= x.Length || xf <= x0) return result;
            for (int i = 0; i < f.Length; i++)
            {
                result[i] = new double[xf - x0 + 1];
                result[i][0] = f0[i];
            }

            double[] fi_1 = new double[f.Length];
            for (int i = 0; i < f.Length; i++) fi_1[i] = result[i][0];
            for (int i = x0 + 1; i <= xf; i++)
            {
                for (int j = 0; j < f.Length; j++)
                {
                    result[j][i - x0] = (x[i] - x[i - 1]) * f[j](i - 1, fi_1) + fi_1[j];
                }
                for (int j = 0; j < f.Length; j++) fi_1[j] = result[j][i - x0];
            }
            return result;
        }

        public static double[][] IntegrateBackward(double[] x, int x0, int xf, Fi[] f, double[] ff)
        {
            double[][] result = new double[f.Length][];
            if (x0 < 0 || xf >= x.Length || xf <= x0) return result;
            for (int i = 0; i < f.Length; i++)
            {
                result[i] = new double[xf - x0 + 1];
                result[i][xf - x0] = ff[i];
            }

            double[] fi = new double[f.Length];
            for (int i = 0; i < f.Length; i++) fi[i] = result[i][xf - x0];
            for(int i = xf; i > x0; i--)
            {
                for (int j = 0; j < f.Length; j++)
                {
                    result[j][i - 1 - x0] = (x[i - 1] - x[i]) * f[j](i, fi) + fi[j];
                }
                for (int j = 0; j < f.Length; j++) fi[j] = result[j][i - 1 - x0];
            }
            return result;
        }

        public static double[][] Integrate(double x_start, double x_end, double dx, F[] f, double[] f0, out double[] x)
        {
            x = BuildX(x_start, x_end, dx);
            return Integrate(x, f, f0);
        }

        public static double[] BuildX(double x_start, double x_end, double dx)
        {
            int n = (int)Math.Abs((x_end - x_start) / dx) + 1;
            dx = Math.Sign(x_end - x_start) * Math.Abs(dx);
            double[] x;
            if ((n - 1) < Math.Abs((x_end - x_start) / dx))
                x = new double[n + 1];
            else
                x = new double[n];
            x[0] = x_start;
            for (int i = 1; i < n; i++)
                x[i] = x[i - 1] + dx;
            if (n != x.Length)
                x[x.Length - 1] = x_end;
            return x;
        }
    }

    public struct EulerPoint
    {
        public double X;
        public double Y;
        public EulerPoint(double x = 0, double y = 0)
        {
            X = x;
            Y = y;
        }
    }
}
