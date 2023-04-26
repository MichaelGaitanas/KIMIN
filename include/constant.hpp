#ifndef CONSTANT_HPP
#define CONSTANT_HPP

const double machine_zero = 1e-14;
const double pi = 3.1415926535897932384626433832795;
const double G = 6.67430e-20; //[km^3/(kg*sec^2)]

const char *str_ell_semiaxes1[] = {"a1 ", "b1 ", "c1 "};
const char *str_ell_semiaxes2[] = {"a2 ", "b2 ", "c2 "};

const char *str_raycast_axes[] = {"x axis ", "y axis ", "z axis "};

const char *str_time_params[] = {"Epoch     ", "Duration  ", "Step        "};

const char *str_cart[] = {"x    ", "y    ", "z    ", "υx  ", "υy  ", "υz  "};
const char *str_cart_units[] = {"[km]", "[km]", "[km]", "[km/sec]", "[km/sec]", "[km/sec]"};

const char *str_kep[] = {"a       ", "e       ", "i        ", "Ω   ", "ω       ", "M      "};
const char *str_kep_units[] = {"[km]", "[  ]", "[deg]", "[deg]", "[deg]", "[deg]"};

const char *str_rpy1[] = {"roll 1    ", "pitch 1 ", "yaw 1  "};
const char *str_rpy2[] = {"roll 2    ", "pitch 2 ", "yaw 2  "};

const char *str_q1[] = {"q10   ", "q11   ", "q12   ", "q13   "};
const char *str_q2[] = {"q20   ", "q21   ", "q22   ", "q23   "};

const char *str_w1[] = {"ω1x   ", "ω1y   ", "ω1z   "};
const char *str_w2[] = {"ω2x   ", "ω2y   ", "ω2z   "};

const char *str_plot1_cart[] = {"Position x", "Position y", "Position z", "Velocity υx", "Velocity υy", "Velocity υz"};
const char *str_plot2_cart[] = {"x(t)", "y(t)", "z(t)", "υx(t)", "υy(t)", "υz(t)"};
const char *str_plot3_cart[] = {"x [km]", "y [km]", "z [km]", "υx [km/sec]", "υy [km/sec]", "υz [km/sec]"};

const char *str_plot1_kep[] = {"Semi-major axis a", "Eccentricity e", "Inclination i", "Longitude of ascending node Ω", "Argument of periapsis ω", "Mean anomaly M"};
const char *str_plot2_kep[] = {"a(t)", "e(t)", "i(t)", "Ω(t)", "ω(t)", "M(t)"};
const char *str_plot3_kep[] = {"a [km]", "e [ ]", "i [rad]", "Ω [rad]", "ω [rad]", "M [rad]"};

const char *str_plot1_ener_mom[] = {"Energy relative error", "Momentum relative error"};
const char *str_plot2_ener_mom[] = {"ener_rel_err(t)", "mom_rel_err(t)"};

#endif