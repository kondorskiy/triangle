
/*======================================================================================================================

  The free computer code for calculating the contribution of the primary longitudinal plasmonic resonance
    of a triangular nanoprism with rounded corners to the extinction and scattering cross sections using
    the the analytical model, including the size-dependent dielectric function for silver and gold.

  Please cite the following paper: A.D. Kondorskiy, A.V. Mekshun, "Effect of Geometric Parameters
    of Metallic Nanoprisms on the Plasmonic Resonance Wavelength." J. Russ. Laser Res. 44 (6) 627--637 (2023).
    https://doi.org/10.1007/s10946-023-10171-5

  A similar code for a cylinder with rounded edges can be found at https://github.com/kondorskiy/rod.

  Corresponding author e-mail: kondorskiy@lebedev.ru, kondorskiy@gmail.com.

======================================================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <complex>
#include <math.h>


/***********************************************************************************************************************
  The analytical model for prism with equilateral triangle base.
***********************************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------------------
  Dipole polarizability in nm^3.
----------------------------------------------------------------------------------------------------------------------*/
std::complex<double> dipPolariz(
  const double &lambda,                     // Wavelength in nm.
  const std::complex<double> &eps_m,        // Dielectric permittivity of material.
  const double &eps_h,                      // Dielectric permittivity of host media.
  const double &L,                          // Edge length in nm.
  const double &H,                          // Thickness in nm.
  const double &R )                         // Triangle base corner radius in nm.
{
  const std::complex<double> IRE(1.0, 0.0);
  const std::complex<double> IIM(0.0, 1.0);

  double beta = -0.649487*pow(L/H, -1.27802) + 1.87718*pow(L/R, -0.928178) + 0.0784606*pow(H/R, -0.619604) + 0.617065;
  double eps_c = -1.73983*pow(L/H, 0.904851) + 23.7005*pow(L/R, -9.71985) + 3.73666*pow(H/R, -0.416187) - 4.23387;
  double a2 = 1.35181*pow(L/H, -0.556507) + 1.13818*pow(L/R, -0.483608) - 0.287856*pow(H/R, -0.468685) - 0.0564038;
  double a4 = -2.58813*pow(L/H, -0.447242) - 2.62882*pow(L/R, -2.97322) - 0.254773*pow(H/R, -0.125501) + 0.702526;

  double s = std::sqrt(eps_h)*L/lambda;
  double V0 = 0.25*std::sqrt(3.0)*L*L*H;
  double V1 = V0*beta;

  std::complex<double> Arc = IRE*s*s*a2 + 4.0*M_PI*M_PI*IIM*V1*s*s*s/(3.0*L*L*L) + IRE*s*s*s*s*a4;
  return (1.0/(4.0*M_PI))*V1/( IRE/(eps_m/eps_h - IRE) - IRE/(eps_c - 1.0) - Arc );
}


/*----------------------------------------------------------------------------------------------------------------------
  Scattering cross section in cm^2.
----------------------------------------------------------------------------------------------------------------------*/
double scatCSdip(
  const double &lambda,                     // Wavelength in nm.
  const std::complex<double> &eps_m,        // Dielectric permittivity of material.
  const double &eps_h,                      // Dielectric permittivity of host media.
  const double &L,                          // Edge length in nm.
  const double &H,                          // Thickness in nm.
  const double &R )                         // Triangle base corner radius in nm.
{
  std::complex<double> polariz = dipPolariz( lambda, eps_m, eps_h, L, H, R );
  double k = 2.0*M_PI*std::sqrt(eps_h)/lambda;
  return 8.0*M_PI*std::pow(k, 4)*std::pow(std::abs(polariz), 2)*1.0e-14/3.0;
}


/*----------------------------------------------------------------------------------------------------------------------
  Extinction cross section in cm^2.
----------------------------------------------------------------------------------------------------------------------*/
double extCSdip(
  const double &lambda,                     // Wavelength in nm.
  const std::complex<double> &eps_m,        // Dielectric permittivity of material.
  const double &eps_h,                      // Dielectric permittivity of host media.
  const double &L,                          // Edge length in nm.
  const double &H,                          // Thickness in nm.
  const double &R )                         // Triangle base corner radius in nm.
{
  std::complex<double> polariz = dipPolariz( lambda, eps_m, eps_h, L, H, R );
  double k = 2.0*M_PI*std::sqrt(eps_h)/lambda;
  return 4.0*M_PI*k*std::imag(polariz)*1.0e-14;
}


/***********************************************************************************************************************
  Dielectric functions of silver and gold.
***********************************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------------------
  Interpolation procedure.
----------------------------------------------------------------------------------------------------------------------*/
double interpolate(
  const int &n,                             // Size of arrays.
  const double x_arr[],                     // Array of arguments.
  const double y_arr[],                     // Array of function values.
  const double &x_val)                      // Argument value to interpolate for.
{
  int i = 0;
  bool flag = true;
  for (int j = 1; j < n; ++j)
    if (flag && (x_arr[j] > x_val)) {
      i = j - 1;
      flag = false;
    }
  if(i < 1) i = 1;
  if(i > n - 3) i = n - 3;

  double x[4], y[4];

  x[0] = x_arr[i - 1];  y[0] = y_arr[i - 1];
  x[1] = x_arr[i];      y[1] = y_arr[i];
  x[2] = x_arr[i + 1];  y[2] = y_arr[i + 1];
  x[3] = x_arr[i + 2];  y[3] = y_arr[i + 2];

  return (x_val - x[1])*(x_val - x[2])*(x_val - x[3])*y[0]/((x[0] - x[1])*(x[0] - x[2])*(x[0] - x[3]))
       + (x_val - x[0])*(x_val - x[2])*(x_val - x[3])*y[1]/((x[1] - x[0])*(x[1] - x[2])*(x[1] - x[3]))
       + (x_val - x[0])*(x_val - x[1])*(x_val - x[3])*y[2]/((x[2] - x[0])*(x[2] - x[1])*(x[2] - x[3]))
       + (x_val - x[0])*(x_val - x[1])*(x_val - x[2])*y[3]/((x[3] - x[0])*(x[3] - x[1])*(x[3] - x[2]));
}


/*----------------------------------------------------------------------------------------------------------------------
  Dielectric function of silver from [Johnson P.B., Christy R.W. Phys. Rev. B, 6, 470 (1972).]
----------------------------------------------------------------------------------------------------------------------*/
std::complex<double> epsAg(
  const double &lambda)                     // Wavelength in nm.
{
  const std::complex<double> IRE(1.0, 0.0);
  const std::complex<double> IIM(0.0, 1.0);

  const int num = 49;
  const double energy[] = { 0.64, 0.77, 0.89, 1.02, 1.14, 1.26, 1.39, 1.51, 1.64, 1.76,
                            1.88, 2.01, 2.13, 2.26, 2.38, 2.50, 2.63, 2.75, 2.88, 3.00,
                            3.12, 3.25, 3.37, 3.50, 3.62, 3.74, 3.87, 3.99, 4.12, 4.24,
                            4.36, 4.49, 4.61, 4.74, 4.86, 4.98, 5.11, 5.23, 5.36, 5.48,
                            5.60, 5.73, 5.85, 5.98, 6.10, 6.22, 6.35, 6.47, 6.60 };

  const double ndata[]  = { 0.24, 0.15, 0.13, 0.09, 0.04, 0.04, 0.04, 0.04, 0.03, 0.04,
                            0.05, 0.06, 0.05, 0.06, 0.05, 0.05, 0.05, 0.04, 0.04, 0.05,
                            0.05, 0.05, 0.07, 0.10, 0.14, 0.17, 0.81, 1.13, 1.34, 1.39,
                            1.41, 1.41, 1.38, 1.35, 1.33, 1.31, 1.30, 1.28, 1.28, 1.26,
                            1.25, 1.22, 1.20, 1.18, 1.15, 1.14, 1.12, 1.10, 1.07 };

  const double kdata[]  = { 14.080, 11.85, 10.10, 8.828, 7.795, 6.992, 6.312, 5.727, 5.242, 4.838,
                            4.483, 4.152, 3.858, 3.586, 3.324, 3.093, 2.869, 2.657, 2.462, 2.275,
                            2.070, 1.864, 1.657, 1.419, 1.142, 0.829, 0.392, 0.616, 0.964, 1.161,
                            1.264, 1.331, 1.372, 1.387, 1.393, 1.389, 1.378, 1.367, 1.357, 1.344,
                            1.342, 1.336, 1.325, 1.312, 1.296, 1.277, 1.255, 1.232, 1.212 };

  double omega = 1239.8/lambda;

  if ((omega < energy[0]) && (omega > energy[num - 1])) {
    std::cout << "Argument of dielectric function of silver is out of range" << std::endl;
    exit(0);
  }

  double n_val = interpolate(num, energy, ndata, omega);
  double k_val = interpolate(num, energy, kdata, omega);

  return IRE*(n_val*n_val - k_val*k_val) + IIM*2.0*n_val*k_val;
}


/*----------------------------------------------------------------------------------------------------------------------
  Size-dependent dielectric function of silver.
----------------------------------------------------------------------------------------------------------------------*/
std::complex<double> epsAgSD(
  const double &lambda,                     // Wavelength in nm.
  const double &D)                          // Size parameter in nm.
{
  const std::complex<double> IRE(1.0, 0.0);
  const std::complex<double> IIM(0.0, 1.0);

  const double vF = 1.39e8;         // cm/s (Fermi velocity).
  const double lam_inf = 5.2e-6;    // cm (mean electron free path).
  const double omega_p = 9.1;       // eV (plasma angular frequency).
  const double A = 2.5;             // empirical constant.

  const double h_bar = 6.582e-16;
  const double gam_inf = h_bar*vF/lam_inf;
  const double gam_r = gam_inf + A*h_bar*vF*1.0e7*(2.0/D);

  double omega = 1239.8/lambda;

  return epsAg(lambda)
    + omega_p*omega_p*( IRE/(IRE*omega*omega + IIM*omega*gam_inf) - IRE/(IRE*omega*omega + IIM*omega*gam_r) );
}


/*----------------------------------------------------------------------------------------------------------------------
  Dielectric function of gold from [R. L. Olmon, B. Slovick, T. W. Johnson, D. Shelton, S.-H. Oh,
  G. D. Boreman, and M. B. Raschke. Phys. Rev. B, 86, 235147 (2012).]
----------------------------------------------------------------------------------------------------------------------*/
std::complex<double> epsAu(
  const double &lambda)                     // Wavelength in nm.
{
  const std::complex<double> IRE(1.0, 0.0);
  const std::complex<double> IIM(0.0, 1.0);

  const int num = 448;
  const double energy[] = { 0.0497329, 0.0516601, 0.0535569, 0.0554739, 0.0574001, 0.0592942, 0.0612268, 0.0631284,
    0.0650494, 0.0669461, 0.0688801, 0.0707672, 0.0726754, 0.0745994, 0.0765334, 0.0784214, 0.0803527, 0.0822722,
    0.0841712, 0.0861001, 0.0879944, 0.0899088, 0.0918401, 0.0937144, 0.0956668, 0.0975485, 0.0995058, 0.101377,
    0.10332, 0.10525, 0.10716, 0.109045, 0.110997, 0.112815, 0.1148, 0.116636, 0.118645, 0.12049, 0.122393, 0.124345,
    0.126257, 0.128176, 0.130085, 0.131996, 0.133907, 0.135828, 0.13773, 0.139653, 0.141567, 0.143467, 0.145385,
    0.147302, 0.149217, 0.151126, 0.153048, 0.154961, 0.156863, 0.158771, 0.160685, 0.162602, 0.164523, 0.166422,
    0.168342, 0.170261, 0.172176, 0.174086, 0.175989, 0.177908, 0.179818, 0.181742, 0.183653, 0.18555, 0.187486,
    0.189376, 0.191304, 0.193212, 0.195128, 0.197051, 0.198948, 0.200849, 0.202787, 0.204696, 0.206606, 0.208517,
    0.210428, 0.212338, 0.214246, 0.216151, 0.21809, 0.219986, 0.221915, 0.223839, 0.225713, 0.227661, 0.229558,
    0.231487, 0.233404, 0.235309, 0.237199, 0.239121, 0.241027, 0.242963, 0.244883, 0.246784, 0.248665, 0.250625,
    0.252514, 0.254431, 0.256325, 0.258247, 0.260143, 0.262068, 0.263965, 0.265889, 0.267842, 0.269707, 0.271657,
    0.273575, 0.275459, 0.27737, 0.279307, 0.281207, 0.283134, 0.285021, 0.286934, 0.288873, 0.29077, 0.292692,
    0.294569, 0.296542, 0.298398, 0.300349, 0.302253, 0.304181, 0.306058, 0.307959, 0.309883, 0.311831, 0.313725,
    0.315642, 0.317582, 0.319465, 0.321369, 0.323296, 0.325247, 0.327135, 0.329045, 0.330978, 0.332843, 0.334731,
    0.336639, 0.33857, 0.340522, 0.342403, 0.344305, 0.346228, 0.348172, 0.35004, 0.352028, 0.353937, 0.355765,
    0.357715, 0.359687, 0.361575, 0.363483, 0.365412, 0.367252, 0.36922, 0.371099, 0.372997, 0.374914, 0.376852,
    0.378809, 0.38067, 0.382549, 0.384566, 0.386364, 0.3883, 0.390256, 0.392107, 0.394101, 0.395989, 0.397895, 0.39982,
    0.401763, 0.403594, 0.405575, 0.407441, 0.409324, 0.411224, 0.413143, 0.415079, 0.417034, 0.419007, 0.420856,
    0.422721, 0.424749, 0.426649, 0.428566, 0.430351, 0.432302, 0.43427, 0.436103, 0.438107, 0.439972, 0.441854,
    0.443752, 0.445666, 0.447596, 0.449544, 0.451508, 0.453324, 0.455322, 0.457169, 0.459031, 0.461079, 0.462973,
    0.464883, 0.466808, 0.468749, 0.470528, 0.472501, 0.47449, 0.476313, 0.47815, 0.480187, 0.482054, 0.483935,
    0.485831, 0.487743, 0.489669, 0.491611, 0.493568, 0.49554, 0.497329, 0.499332, 0.501149, 0.503183, 0.505027,
    0.506885, 0.508757, 0.510854, 0.512755, 0.514671, 0.518329, 0.520286, 0.522259, 0.524246, 0.526025, 0.528042,
    0.529847, 0.533724, 0.535569, 0.537426, 0.539531, 0.543314, 0.545225, 0.54715, 0.549088, 0.55276, 0.554739,
    0.556732, 0.558487, 0.562287, 0.564334, 0.566138, 0.570042, 0.571883, 0.575867, 0.577745, 0.579636, 0.583455,
    0.585383, 0.589278, 0.590964, 0.594934, 0.596939, 0.600699, 0.60245, 0.60628, 0.608362, 0.612268, 0.614087,
    0.617759, 0.619921, 0.623036, 0.626183, 0.629361, 0.632572, 0.635816, 0.639094, 0.642405, 0.645751, 0.649132,
    0.652548, 0.656001, 0.65949, 0.663017, 0.666582, 0.670185, 0.673827, 0.677509, 0.681232, 0.684995, 0.688801,
    0.692649, 0.69654, 0.700476, 0.704456, 0.708481, 0.712553, 0.716672, 0.720838, 0.725054, 0.729319, 0.733634,
    0.738001, 0.74242, 0.746893, 0.751419, 0.756001, 0.760639, 0.765334, 0.770088, 0.774901, 0.779775, 0.78471,
    0.789708, 0.79477, 0.799898, 0.805092, 0.810354, 0.815685, 0.821087, 0.826561, 0.832109, 0.837731, 0.84343,
    0.849207, 0.855063, 0.861001, 0.867022, 0.918401, 0.925255, 0.932212, 0.939274, 0.946444, 0.953724, 0.961118,
    0.968626, 0.976253, 0.984001, 0.991873, 0.999872, 1.008, 1.01626, 1.02466, 1.0332, 1.04188, 1.05071, 1.05969,
    1.06883, 1.07812, 1.08758, 1.09721, 1.107, 1.11697, 1.12713, 1.13747, 1.148, 1.15873, 1.16966, 1.1808, 1.19216,
    1.20373, 1.21553, 1.22757, 1.23984, 1.25237, 1.26514, 1.27819, 1.2915, 1.3051, 1.31898, 1.33316, 1.34765, 1.36246,
    1.3776, 1.39308, 1.40891, 1.42511, 1.44168, 1.45864, 1.476, 1.49379, 1.512, 1.53067, 1.5498, 1.56942, 1.58954,
    1.61018, 1.63137, 1.65312, 1.67546, 1.69841, 1.722, 1.74626, 1.7712, 1.79687, 1.8233, 1.85051, 1.87855, 1.90745,
    1.93725, 1.968, 1.99974, 2.03253, 2.0664, 2.10143, 2.13766, 2.17516, 2.214, 2.25426, 2.296, 2.33932, 2.38431,
    2.43106, 2.47968, 2.53029, 2.583, 2.63796, 2.69531, 2.7552, 2.81782, 2.88335, 2.952, 3.024, 3.0996, 3.17908,
    3.26274, 3.35092, 3.444, 3.54241, 3.64659, 3.7571, 3.87451, 3.99949, 4.13281 };

  const double ndata[]  = { 42.79, 40.92, 39.16, 37.49, 35.91, 34.41, 33, 31.66, 30.39, 29.19, 28.05, 26.97, 25.94,
    24.97, 24.04, 23.17, 22.33, 21.54, 20.78, 20.06, 19.38, 18.72, 18.1, 17.51, 16.94, 16.4, 15.88, 15.38, 14.91, 14.46,
    14.02, 13.61, 13.21, 12.83, 12.46, 12.11, 11.77, 11.45, 11.14, 10.84, 10.55, 10.27, 10.01, 9.749, 9.502, 9.263,
    9.034, 8.812, 8.598, 8.392, 8.193, 8, 7.814, 7.634, 7.461, 7.293, 7.13, 6.973, 6.82, 6.673, 6.53, 6.392, 6.258,
    6.127, 6.001, 5.879, 5.76, 5.645, 5.533, 5.424, 5.319, 5.216, 5.117, 5.02, 4.926, 4.834, 4.745, 4.658, 4.574, 4.492,
    4.412, 4.334, 4.258, 4.184, 4.112, 4.042, 3.973, 3.906, 3.841, 3.778, 3.716, 3.655, 3.596, 3.539, 3.482, 3.427,
    3.374, 3.321, 3.27, 3.22, 3.171, 3.123, 3.077, 3.031, 2.986, 2.942, 2.899, 2.858, 2.817, 2.776, 2.737, 2.699, 2.661,
    2.624, 2.588, 2.553, 2.518, 2.484, 2.451, 2.418, 2.386, 2.355, 2.324, 2.294, 2.264, 2.235, 2.207, 2.179, 2.152,
    2.125, 2.098, 2.072, 2.047, 2.022, 1.998, 1.974, 1.95, 1.927, 1.904, 1.881, 1.859, 1.838, 1.817, 1.796, 1.775,
    1.755, 1.735, 1.716, 1.697, 1.678, 1.659, 1.641, 1.623, 1.605, 1.588, 1.571, 1.554, 1.538, 1.521, 1.505, 1.49,
    1.474, 1.459, 1.444, 1.429, 1.414, 1.4, 1.386, 1.372, 1.358, 1.345, 1.331, 1.318, 1.305, 1.293, 1.28, 1.268, 1.256,
    1.244, 1.232, 1.22, 1.209, 1.197, 1.186, 1.175, 1.164, 1.153, 1.143, 1.132, 1.122, 1.112, 1.102, 1.092, 1.082,
    1.073, 1.063, 1.054, 1.045, 1.036, 1.027, 1.018, 1.009, 1, 0.9919, 0.9835, 0.9752, 0.967, 0.9589, 0.9509, 0.943,
    0.9352, 0.9275, 0.9199, 0.9123, 0.9049, 0.8976, 0.8904, 0.8832, 0.8761, 0.8692, 0.8623, 0.8554, 0.8487, 0.8421,
    0.8355, 0.829, 0.8226, 0.8162, 0.8099, 0.8037, 0.7976, 0.7916, 0.7856, 0.7797, 0.7738, 0.768, 0.7623, 0.7566,
    0.7511, 0.7455, 0.7401, 0.7347, 0.7293, 0.724, 0.7136, 0.7085, 0.7034, 0.6984, 0.6935, 0.6886, 0.6837, 0.6742,
    0.6695, 0.6649, 0.6603, 0.6512, 0.6468, 0.6423, 0.638, 0.6294, 0.6252, 0.621, 0.6168, 0.6087, 0.6047, 0.6007,
    0.5929, 0.589, 0.5814, 0.5776, 0.5739, 0.5666, 0.563, 0.5559, 0.5524, 0.5455, 0.5421, 0.5354, 0.5321, 0.5256,
    0.5224, 0.516, 0.5129, 0.5068, 0.5031, 0.4989, 0.4942, 0.4896, 0.4851, 0.4805, 0.476, 0.4715, 0.4671, 0.4627,
    0.4583, 0.454, 0.4497, 0.4454, 0.4412, 0.4369, 0.4328, 0.4286, 0.4245, 0.4204, 0.4163, 0.4123, 0.4082, 0.4042,
    0.4003, 0.3963, 0.3924, 0.3885, 0.3847, 0.3808, 0.377, 0.3732, 0.3695, 0.3657, 0.362, 0.3583, 0.3546, 0.351, 0.3474,
    0.3437, 0.3402, 0.3366, 0.3331, 0.3295, 0.326, 0.3226, 0.3191, 0.3157, 0.3123, 0.3089, 0.3055, 0.3021, 0.2988,
    0.2955, 0.2922, 0.2889, 0.2857, 0.2824, 0.2551, 0.252, 0.249, 0.246, 0.243, 0.24, 0.2371, 0.2342, 0.2313, 0.2281,
    0.225, 0.2218, 0.2187, 0.2157, 0.2126, 0.2096, 0.2066, 0.2036, 0.2007, 0.1978, 0.1949, 0.1921, 0.1893, 0.1865,
    0.1837, 0.181, 0.1783, 0.1757, 0.1731, 0.1705, 0.168, 0.1655, 0.163, 0.1606, 0.1582, 0.1559, 0.1536, 0.1514, 0.1493,
    0.1471, 0.1451, 0.1431, 0.1412, 0.1393, 0.1375, 0.1358, 0.1342, 0.1326, 0.1312, 0.1298, 0.1286, 0.1275, 0.1265,
    0.1256, 0.1249, 0.1244, 0.124, 0.1239, 0.1239, 0.1242, 0.1248, 0.1257, 0.1268, 0.1284, 0.1304, 0.1328, 0.1358,
    0.1394, 0.1436, 0.1487, 0.1546, 0.1616, 0.1698, 0.1794, 0.1908, 0.2041, 0.2199, 0.2388, 0.2617, 0.2899, 0.3256,
    0.3724, 0.4363, 0.5263, 0.6527, 0.8197, 1.011, 1.193, 1.333, 1.422, 1.472, 1.505, 1.537, 1.567, 1.585, 1.588,
    1.585, 1.588, 1.606, 1.637, 1.671, 1.695, 1.701, 1.685, 1.649, 1.596 };

  const double kdata[]  = { 137.5, 133.9, 130.4, 127.1, 124, 121, 118.2, 115.5, 112.9, 110.4, 108.1, 105.8, 103.6,
    101.5, 99.47, 97.52, 95.64, 93.83, 92.08, 90.39, 88.76, 87.18, 85.66, 84.18, 82.76, 81.37, 80.03, 78.73, 77.47,
    76.25, 75.07, 73.91, 72.8, 71.71, 70.65, 69.63, 68.63, 67.65, 66.71, 65.78, 64.89, 64.01, 63.16, 62.33, 61.52,
    60.73, 59.96, 59.2, 58.47, 57.75, 57.05, 56.37, 55.7, 55.04, 54.4, 53.78, 53.17, 52.57, 51.98, 51.41, 50.85, 50.3,
    49.76, 49.24, 48.72, 48.21, 47.72, 47.23, 46.76, 46.29, 45.83, 45.38, 44.94, 44.51, 44.08, 43.67, 43.26, 42.85,
    42.46, 42.07, 41.69, 41.32, 40.95, 40.59, 40.23, 39.88, 39.54, 39.2, 38.87, 38.54, 38.22, 37.9, 37.59, 37.29, 36.98,
    36.69, 36.4, 36.11, 35.82, 35.55, 35.27, 35, 34.73, 34.47, 34.21, 33.96, 33.71, 33.46, 33.21, 32.97, 32.74, 32.5,
    32.27, 32.04, 31.82, 31.6, 31.38, 31.16, 30.95, 30.74, 30.53, 30.33, 30.13, 29.93, 29.73, 29.54, 29.34, 29.15,
    28.97, 28.78, 28.6, 28.42, 28.24, 28.07, 27.89, 27.72, 27.55, 27.38, 27.22, 27.06, 26.89, 26.73, 26.58, 26.42,
    26.27, 26.11, 25.96, 25.81, 25.67, 25.52, 25.38, 25.23, 25.09, 24.95, 24.81, 24.68, 24.54, 24.41, 24.28, 24.14,
    24.01, 23.89, 23.76, 23.63, 23.51, 23.39, 23.26, 23.14, 23.02, 22.91, 22.79, 22.67, 22.56, 22.45, 22.33, 22.22,
    22.11, 22, 21.89, 21.79, 21.68, 21.57, 21.47, 21.37, 21.26, 21.16, 21.06, 20.96, 20.86, 20.77, 20.67, 20.57, 20.48,
    20.38, 20.29, 20.2, 20.11, 20.02, 19.93, 19.84, 19.75, 19.66, 19.57, 19.49, 19.4, 19.32, 19.23, 19.15, 19.06, 18.98,
    18.9, 18.82, 18.74, 18.66, 18.58, 18.5, 18.43, 18.35, 18.27, 18.2, 18.12, 18.05, 17.97, 17.9, 17.83, 17.75, 17.68,
    17.61, 17.54, 17.47, 17.4, 17.33, 17.26, 17.2, 17.13, 17.06, 17, 16.93, 16.86, 16.8, 16.73, 16.67, 16.61, 16.54,
    16.42, 16.36, 16.3, 16.24, 16.17, 16.11, 16.06, 15.94, 15.88, 15.82, 15.76, 15.65, 15.59, 15.54, 15.48, 15.37,
    15.32, 15.26, 15.21, 15.1, 15.05, 15, 14.89, 14.84, 14.74, 14.69, 14.64, 14.54, 14.49, 14.39, 14.35, 14.25, 14.2,
    14.11, 14.06, 13.97, 13.93, 13.84, 13.79, 13.7, 13.65, 13.59, 13.52, 13.46, 13.39, 13.32, 13.25, 13.19, 13.12,
    13.05, 12.99, 12.92, 12.85, 12.78, 12.72, 12.65, 12.58, 12.52, 12.45, 12.38, 12.31, 12.25, 12.18, 12.11, 12.04,
    11.98, 11.91, 11.84, 11.77, 11.71, 11.64, 11.57, 11.5, 11.44, 11.37, 11.3, 11.23, 11.16, 11.1, 11.03, 10.96, 10.89,
    10.82, 10.76, 10.69, 10.62, 10.55, 10.48, 10.41, 10.35, 10.28, 10.21, 10.14, 10.07, 10, 9.934, 9.865, 9.796, 9.214,
    9.145, 9.075, 9.005, 8.935, 8.865, 8.795, 8.725, 8.655, 8.581, 8.506, 8.432, 8.358, 8.283, 8.208, 8.134, 8.059,
    7.984, 7.909, 7.834, 7.759, 7.683, 7.608, 7.532, 7.456, 7.38, 7.304, 7.228, 7.152, 7.075, 6.999, 6.922, 6.845,
    6.768, 6.69, 6.613, 6.535, 6.457, 6.379, 6.301, 6.222, 6.143, 6.064, 5.985, 5.905, 5.825, 5.745, 5.664, 5.583,
    5.501, 5.42, 5.338, 5.255, 5.172, 5.088, 5.004, 4.92, 4.834, 4.749, 4.662, 4.575, 4.487, 4.398, 4.308, 4.217,
    4.126, 4.033, 3.938, 3.843, 3.746, 3.647, 3.547, 3.444, 3.34, 3.232, 3.122, 3.009, 2.891, 2.769, 2.641, 2.507,
    2.364, 2.213, 2.058, 1.908, 1.787, 1.72, 1.716, 1.758, 1.812, 1.855, 1.877, 1.887, 1.898, 1.911, 1.915, 1.904,
    1.877, 1.846, 1.825, 1.821, 1.835, 1.858, 1.881, 1.895, 1.888 };

  double omega = 1239.8/lambda;

  if ((omega < energy[0]) && (omega > energy[num - 1])) {
    std::cout << "Argument of dielectric function of gold is out of range" << std::endl;
    exit(0);
  }

  double n_val = interpolate(num, energy, ndata, omega);
  double k_val = interpolate(num, energy, kdata, omega);

  return IRE*(n_val*n_val - k_val*k_val) + IIM*2.0*n_val*k_val;
}


/*----------------------------------------------------------------------------------------------------------------------
  Size-dependent dielectric function of gold.
----------------------------------------------------------------------------------------------------------------------*/
std::complex<double> epsAuSD(
  const double &lambda,                     // Wavelength in nm.
  const double &D)                          // Size parameter in nm.
{
  const std::complex<double> IRE(1.0, 0.0);
  const std::complex<double> IIM(0.0, 1.0);

  const double vF = 1.38e8;         // cm/s (Fermi velocity).
  const double lam_inf = 1.28e-6;   // cm (mean electron free path).
  const double omega_p = 9.0;       // eV (plasma angular frequency).
  const double A = 2.0;             // empirical constant.

  const double h_bar = 6.582e-16;
  const double gam_inf = h_bar*vF/lam_inf;
  const double gam_r = gam_inf + A*h_bar*vF*1.0e7*(2.0/D);

  double omega = 1239.8/lambda;

  return epsAu(lambda)
    + omega_p*omega_p*( IRE/(IRE*omega*omega + IIM*omega*gam_inf) - IRE/(IRE*omega*omega + IIM*omega*gam_r) );
}


/*----------------------------------------------------------------------------------------------------------------------
  Diameter of a sphere of the same volume as the prism with equilateral triangle base.
----------------------------------------------------------------------------------------------------------------------*/
double diameter(
  const double &L,                          // Edge length in nm.
  const double &H)                          // Thickness in nm.
{
  return 2.0*pow( (3.0*sqrt(3.0)*L*L*H/(16.0*M_PI)), 1.0/3.0 );
}


/***********************************************************************************************************************
  The example program.
***********************************************************************************************************************/

int main(int argc, char **argv)
{

  // ----- Calculation parameters. -----
  const double L_size =  50.0;            // Edge length in nm.
  const double R_size =   2.0;            // Triangle base corner radius in nm.
  const double H_size =  20.0;            // Thickness in nm.

  const double eps_h = 1.0;               // Dielectric permittivity of host media.
  const bool is_silver = true;            // Switch to choose material: true - silver, false - gold.

  const double wl_min =  300.0;           // Minimal value of wavelength range to print results in nm.
  const double wl_max =  800.0;           // Maximal value of wavelength range to print results in nm.
  const double wl_step =   2.0;           // Wavelength step to print results in nm.

  // Effective size (diameter) to calculate size-dependent dielectric function.
  const double D_SD = diameter(L_size, H_size); 
  std::cout << "Effective size to calculate size-dependent dielectric function = " << D_SD << " nm." << std::endl;

  int wl_n = (int)((wl_max - wl_min)/wl_step) + 1;
  std::string file_name_re = "analytic_model-polarizability_re.dat";
  std::string file_name_im = "analytic_model-polarizability_im.dat";
  std::string file_name_sc = "analytic_model-scattering_cs.dat";
  std::string file_name_ex = "analytic_model-extinction_cs.dat";
  std::ofstream fout_re(file_name_re.c_str(), std::ios::out);
  std::ofstream fout_im(file_name_im.c_str(), std::ios::out);
  std::ofstream fout_sc(file_name_sc.c_str(), std::ios::out);
  std::ofstream fout_ex(file_name_ex.c_str(), std::ios::out);
  for (int i = 0; i < wl_n; ++i) {
    double wl = wl_min + i*wl_step;
    std::complex<double> eps_m;
    if (is_silver) eps_m = epsAgSD(wl, D_SD); else eps_m = epsAuSD(wl, D_SD);
    std::complex<double> val = dipPolariz(wl, eps_m, eps_h, L_size, H_size, R_size);
    fout_re << wl << " " << std::real(val) << std::endl;
    fout_im << wl << " " << std::imag(val) << std::endl;
    fout_sc << wl << " " << scatCSdip(wl, eps_m, eps_h, L_size, H_size, R_size) << std::endl;
    fout_ex << wl << " " << extCSdip(wl, eps_m, eps_h, L_size, H_size, R_size) << std::endl;
  }
  fout_re.close();
  fout_im.close();
  fout_sc.close();
  fout_ex.close();

  return 0;
};

//======================================================================================================================
