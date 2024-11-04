#ifndef INTEGRATION_HPP
#define INTEGRATION_HPP

#include"constant.hpp"
#include"typedef.hpp"
#include"linalg.hpp"
#include"conversion.hpp"
#include"properties_panel.hpp"

class integration
{
private:
    dvec6 cart;
    dvec4 q1, q2;
    dvec3 w1i, w2i, w1b, w2b;

public:
    void prepare(const properties_panel &properties, std::atomic<bool> &abort_flag, std::atomic<float> &progress)
    {
        if (abort_flag.load())
            return;

        //Preparation 1 : If the user chose Keplerian elements as initial position/velocity, then, transform
        //them to Cartesian coords because the F2BP odes are written in Cartesian form.
        if (properties.cart_kep_var_choice == 1)
            cart = kep2cart(dvec6{properties.kep[0],
                                  properties.kep[1],
                                  properties.kep[2]*pi/180.0,
                                  properties.kep[3]*pi/180.0,
                                  properties.kep[4]*pi/180.0,
                                  properties.kep[5]*pi/180.0}, G*(properties.M1 + properties.M2));

        //Preparation 2 : Transform the orientations to quaternions because the F2BP odes are written in quaternion form.
        if (properties.orient_var_choice == 0)
        {
            //In this case, the user chose Euler angles and assigned them in [deg] as the gui requires.
            //Note : ang2quat() ensures that the quaternion is normalized, so we don't need to apply quat2unit().
            q1 = ang2quat(properties.rpy1*pi/180.0);
            q2 = ang2quat(properties.rpy2*pi/180.0);
        }
        else
        {
            //Just assign them to the private q1, q2.
            //There's no need to normalize them, coz this has already happened in the properties_panel::validate() function.
            q1 = properties.q1;
            q2 = properties.q2; 
        }

        //Preparation 3 : Transform the angular velocities to the corresponding body frames because the Euler equations of rotation are written in body frame form.
        if (properties.frame_type_choice == 0)
        {
            //In this case, the user chose angular velocities to be in the inertial/world frame, so we convert them to the body frames.
            w1b = iner2body(properties.w1i, quat2mat(q1));
            w2b = iner2body(properties.w2i, quat2mat(q2));
        }
        else
        {
            //Just assign them to the private w1b, w2b.
            w1b = properties.w1b;
            w2b = properties.w2b;
        }

        progress.store(1.0f);
    }
};

#endif