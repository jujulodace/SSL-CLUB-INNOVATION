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

#include "base.h"

#include <robot_behavior/goalie.h>
#include <robot_behavior/striker.h>
#include <robot_behavior/mur_defensor.h>
#include <robot_behavior/defensor.h>
#include <robot_behavior/passive_defensor.h>

namespace RhobanSSL {
namespace Strategy {

Base::Base(Ai::AiData & ai_data):
    Strategy(ai_data)
{


}
    
Base::~Base(){
}        

/*
 * We define the minimal number of robot in the field.
 * The goalkeeper is not counted.
 */ 
int Base::min_robots() const {
    return 5;
}

/*
 * We define the maximal number of robot in the field.
 * The goalkeeper is not counted.
 */ 
int Base::max_robots() const {
    return 5;
}

Goalie_need Base::needs_goalie() const {
    return Goalie_need::YES;
}

const std::string Base::name = "base";

void Base::start(double time){
    DEBUG("START PREPARE KICKOFF");
    behaviors_are_assigned = false;
}
void Base::stop(double time){
    DEBUG("STOP PREPARE KICKOFF");
}

void Base::update(double time){
}

void Base::assign_behavior_to_robots(
    std::function<
        void (int, std::shared_ptr<Robot_behavior::RobotBehavior>)
    > assign_behavior,
    double time, double dt
){
    if( not(behaviors_are_assigned) ){
        // We first assign the behhavior of the goalie.

        std::shared_ptr<Robot_behavior::RobotBehavior> goalie(
                new Robot_behavior::Goalie(ai_data)
         );
        
        std::shared_ptr<Robot_behavior::RobotBehavior> striker(
                new Robot_behavior::Striker(ai_data)
        );

        std::shared_ptr<Robot_behavior::RobotBehavior> mur(
                new Robot_behavior::Mur_defensor(ai_data)
        );

        std::shared_ptr<Robot_behavior::RobotBehavior> dp1(
                new Robot_behavior::Passive_defensor(ai_data)
        );
        static_cast<Robot_behavior::Passive_defensor*>( dp1.get() )->set_robot_to_obstacle( 0 );

        std::shared_ptr<Robot_behavior::RobotBehavior> dp2(
                new Robot_behavior::Passive_defensor(ai_data)
        );
        static_cast<Robot_behavior::Passive_defensor*>( dp2.get() )->set_robot_to_obstacle( 1 );

        std::shared_ptr<Robot_behavior::RobotBehavior> defensor(
                new Robot_behavior::Defensor(ai_data)
        );


        assign_behavior( get_goalie(), goalie );

        //we assign now all the other behavior 
        assert( get_player_ids().size() == 5 );

        assign_behavior( player_id(0), striker );
        assign_behavior( player_id(1), mur);
        assign_behavior( player_id(2), dp1);
        assign_behavior( player_id(3), dp2);
        assign_behavior( player_id(4), defensor);

        behaviors_are_assigned = true;
    }
}

// We declare here the starting positions that are used to :
//   - place the robot during STOP referee state
//   - to compute the robot order of get_player_ids(),
//     we minimize the distance between 
//     the startings points and all the robot position, just
//     before the start() or during the STOP referee state.
std::list<
    std::pair<rhoban_geometry::Point,ContinuousAngle>
> Base::get_starting_positions( int number_of_avalaible_robots ){
    assert( min_robots() <= number_of_avalaible_robots );
    assert(
        max_robots()==-1 or
        number_of_avalaible_robots <= max_robots() 
    );

    return {
        std::pair<rhoban_geometry::Point,ContinuousAngle>(
            ai_data.relative2absolute(-1.0/3.0, 0.0),
            0.0
        )
    };
}

//
// This function return false if you don't want to 
// give a staring position. So the manager will chose 
// a default position for you.
// 
bool Base::get_starting_position_for_goalie(
    rhoban_geometry::Point & linear_position, 
    ContinuousAngle & angular_position
){
    linear_position =  ally_goal_center();
    angular_position = ContinuousAngle(0.0);
    return true;
} 



}
}
