/*
    This file is part of SSL.

    Copyright 2018 Boussicault Adrien (adrien.boussicault@u-bordeaux.fr)

    SSL is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SSL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with SSL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "robot_control.h"


#include <assert.h>
#include <math/curve.h>
#include "pid.h"
#include <math/matrix2d.h>
#include <algorithm>

#define CALCULUS_ERROR 0.00000001

CurveForRobot::CurveForRobot(
    const std::function<Vector2d (double u)> & translation,
    double translation_velocity, double translation_acceleration,
    const std::function<double (double u)> & rotation,
    double angular_velocity, double angular_acceleration, 
    double calculus_step
):
    rotation_fct(rotation),
    translation_curve( translation, calculus_step ),
    angular_curve( rotation_fct, calculus_step ),
    tranlsation_consign(
        translation_curve.size(),
        translation_velocity, translation_acceleration 
    ),
    angular_consign(
        angular_curve.size(), 
        angular_velocity, angular_acceleration 
    ),
    calculus_error_for_translation(
        std::min(
            0.0001, // 0.1 mm (in meter)
            calculus_step/100
        )
    ),
    calculus_error_for_rotation(
        std::min(
            0.0001, // approx. 1/100 degree (in radian)
            calculus_step/100
        )
    ),
    // TODO Calculet un time_step plus précisement
    translation_movment(
        translation_curve, tranlsation_consign,
        calculus_step,
        calculus_error_for_translation        
    ),
    rotation_movment(
        angular_curve, angular_consign,
        calculus_step,
        calculus_error_for_rotation
    )
{ };

Vector2d CurveForRobot::translation(double t) const {
    return translation_movment(t);
}
double CurveForRobot::rotation(double t) const {
    return rotation_movment(t)[0];
}

void CurveForRobot::print_translation_movment( double dt ) const {
    std::cout << "translation movment : " << std::endl;
    std::cout << "--------------------- " << std::endl;
    double max_time = translation_movment.max_time();
    std::cout << "   max time : " << max_time;
    for( double t=0; t<max_time; t += dt ){
        Vector2d v = translation_movment(t);
        std::cout << "   {" << t << " : "  << v[0] << "," << v[1] << "}, " << std::endl; 
    }
}

void CurveForRobot::print_translation_curve( double dt ) const {
    std::cout << "translation curve : " << std::endl;
    std::cout << "------------------- " << std::endl;
    for( double t=0; t<1.0; t += dt ){
        Vector2d v = translation_curve(t);
        std::cout << "   {" << t << " : " << v[0] << "," << v[1] << "}, " << std::endl; 
    }
}

void CurveForRobot::print_rotation_movment( double dt ) const {
    std::cout << "rotation movment : " << std::endl;
    std::cout << "--------------------- " << std::endl;
    double max_time = rotation_movment.max_time();
    std::cout << "   max time : " << max_time;
    for( double t=0; t<max_time; t += dt ){
        Vector2d v = rotation_movment(t);
        std::cout << "   {" << t << " : " << v[0] << "," << v[1] << "}, " << std::endl; 
    }
}

void CurveForRobot::print_rotation_curve( double dt ) const {
    std::cout << "rotation curve : " << std::endl;
    std::cout << "------------------- " << std::endl;
    for( double t=0; t<1.0; t += dt ){
        Vector2d v = angular_curve(t);
        std::cout << "   {" << t << " : " << v[0] << "," << v[1] << "}, " << std::endl; 
    }
}






void RobotControl::set_limits(
    double translation_velocity_limit,
    double rotation_velocity_limit,
    double translation_acceleration_limit,
    double rotation_acceleration_limit
){
    this->translation_velocity_limit = translation_velocity_limit;
    this->rotation_velocity_limit = ContinuousAngle(rotation_velocity_limit);
    this->translation_acceleration_limit = translation_acceleration_limit;
    this->rotation_acceleration_limit = ContinuousAngle(rotation_acceleration_limit);
} 

RobotControl::RobotControl(): 
    translation_velocity_limit(-1),
    rotation_velocity_limit(-1),
    translation_acceleration_limit(-1),
    rotation_acceleration_limit(-1)
{ };


PidControl RobotControl::limited_control(
    const Vector2d & robot_position, 
    const ContinuousAngle & robot_orientation,
    const Vector2d & robot_linear_velocity, 
    const ContinuousAngle & robot_angular_velocity
) const {
    PidControl res = no_limited_control();
    if( res.velocity_rotation.value() != 0.0 ){
        double max_angular_velocity;
        if( rotation_acceleration_limit >= ContinuousAngle(0.0) ){ 
            max_angular_velocity = robot_angular_velocity.value() + get_dt() * rotation_acceleration_limit.value();
            if( rotation_velocity_limit >= 0 ){
                max_angular_velocity = std::min( rotation_velocity_limit.value(), max_angular_velocity );
            }
        }else{
            max_angular_velocity = rotation_velocity_limit.value();
        }
        double min_angular_velocity = -1;
        if( rotation_acceleration_limit >= ContinuousAngle(0.0) ){ 
            min_angular_velocity = std::max( 0.0,  robot_angular_velocity.value() - get_dt() * rotation_acceleration_limit.value() );
        }


        if( max_angular_velocity > 0.0 ){ 
            if( res.velocity_rotation.abs() >= max_angular_velocity ){
                assert( res.velocity_rotation.value() != 0 );
                res.velocity_rotation *= (
                    max_angular_velocity / (
                        std::fabs( res.velocity_rotation.value() )/security_margin
                    ) 
                );
            }
        }
        if( min_angular_velocity > 0.0 ){ 
            if( res.velocity_rotation.abs() < min_angular_velocity ){
                assert( res.velocity_rotation.value() != 0 );
                res.velocity_rotation *= (
                    min_angular_velocity / (
                        std::fabs( res.velocity_rotation.value() )*security_margin
                    ) 
                );
            }
        }
    }
    if( res.velocity_translation.norm() !=0 ){
        double max_linear_velocity;
        if( translation_acceleration_limit >= 0.0 ){ 
            max_linear_velocity = robot_linear_velocity.norm() + translation_acceleration_limit * get_dt();
            if( translation_velocity_limit >= 0 ){
                max_linear_velocity = std::min( translation_velocity_limit, max_linear_velocity );
            }
        }else{
            max_linear_velocity = translation_velocity_limit;
        }
        double min_linear_velocity = - 1.0;
        if( translation_acceleration_limit >= 0.0 ){ 
            min_linear_velocity = std::max( 0.0, robot_linear_velocity.norm() - get_dt()*this->translation_acceleration_limit );
        }


        if( max_linear_velocity > 0.0 ){
            if( res.velocity_translation.norm() >= max_linear_velocity ){

                assert( res.velocity_translation.norm() != 0 );
                res.velocity_translation *= ( 
                    max_linear_velocity /
                    (res.velocity_translation.norm()/security_margin)
                );
            }
        }


        if( min_linear_velocity > 0.0 ){
            if( res.velocity_translation.norm() < min_linear_velocity ){

                assert( res.velocity_translation.norm() != 0 );
                res.velocity_translation *= ( 
                    min_linear_velocity /
                    (res.velocity_translation.norm() * security_margin)
                );
            }
        }
    }
    return res;
}

PidControl RobotControlWithPid::no_limited_control() const {
    return PidController::no_limited_control();
}

double RobotControlWithPid::get_dt() const {
    return PidController::get_dt();
}

