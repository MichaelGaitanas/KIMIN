#ifndef INTEGRATION_HPP
#define INTEGRATION_HPP

#include"constant.hpp"
#include"typedef.hpp"
#include"conversion.hpp"
#include"properties_panel.hpp"

class integration
{
private:


public:
    prepare(const properties_panel &properties)
    {
        //Preparation 1 : If the user chose Keplerian elements as initial position/velocity, then transform them to Cartesian coords because the F2BP odes are written in Cartesian.
        if (properties.cart_kep_var_choice == 1)
            properties.cart = kep2cart(dvec6{properties.kep[0],
                                             properties.kep[1],
                                             properties.kep[2]*pi/180.0,
                                             properties.kep[3]*pi/180.0,
                                             properties.kep[4]*pi/180.0,
                                             properties.kep[5]*pi/180.0}, G*(M1+M2));

        //More preparations...

    }

};

#endif