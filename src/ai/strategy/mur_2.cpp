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

#include "mur_2.h"

#include <robot_behavior/goalie.h>
#include <robot_behavior/striker.h>
#include <robot_behavior/mur_defensor.h>
#include <robot_behavior/degageur.h>

namespace RhobanSSL {
namespace Strategy {

Mur_2::Mur_2(Ai::AiData & ai_data):
    Strategy(ai_data)
{
}

Mur_2::~Mur_2(){
}

/*
 * We define the minimal number of robot in the field.
 * The goalkeeper is not counted.
 */
int Mur_2::min_robots() const {
    return 2;
}

/*
 * We define the maximal number of robot in the field.
 * The goalkeeper is not counted.
 */
int Mur_2::max_robots() const {
    return 2;
}

Goalie_need Mur_2::needs_goalie() const {
    return Goalie_need::NO;
}

const std::string Mur_2::name = "mur_2";

void Mur_2::start(double time){
    DEBUG("START PREPARE KICKOFF");
    behaviors_are_assigned = false;
}
void Mur_2::stop(double time){
    DEBUG("STOP PREPARE KICKOFF");
}

void Mur_2::update(double time){

  int nearest_ally_robot_from_ball = GameInformations::get_nearest_ball( Vision::Team::Ally );
  is_closest_0 = false;
  is_closest_1 = false;

  if ( nearest_ally_robot_from_ball == player_id(0) ){
      is_closest_0 = true;
  } else {
      if (nearest_ally_robot_from_ball == player_id(1)) {
        is_closest_1 = true;
      }
  }

}

void Mur_2::assign_behavior_to_robots(
    std::function<
        void (int, std::shared_ptr<Robot_behavior::RobotBehavior>)
    > assign_behavior,
    double time, double dt
){

    std::shared_ptr<Robot_behavior::RobotBehavior> mur1(
            new Robot_behavior::Mur_defensor(ai_data, 1)
    );
    static_cast<Robot_behavior::Mur_defensor*>( mur1.get() )->declare_mur_robot_id( 0, 2 );

    std::shared_ptr<Robot_behavior::RobotBehavior> mur2(
            new Robot_behavior::Mur_defensor(ai_data, 1)
    );
    static_cast<Robot_behavior::Mur_defensor*>( mur2.get() )->declare_mur_robot_id( 1, 2 );
    std::shared_ptr<Robot_behavior::RobotBehavior> deg1(
            new Robot_behavior::Degageur(ai_data)
    );

    if( not(behaviors_are_assigned) ){

        assert( get_player_ids().size() == 2 );

        assign_behavior( player_id(0), mur1 );
        assign_behavior( player_id(1), mur2 );

        behaviors_are_assigned = true;
    } else {
        if ( is_closest_0 ) {
            assign_behavior( player_id(0), deg1 );
            assign_behavior( player_id(1), mur2 );
        } else {
            if ( is_closest_1 ) {
                assign_behavior( player_id(0), mur1 );
                assign_behavior( player_id(1), deg1 );
            } else {
                assign_behavior( player_id(0), mur1 );
                assign_behavior( player_id(1), mur2 );
            }

        }
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
> Mur_2::get_starting_positions( int number_of_avalaible_robots ){
    assert( min_robots() <= number_of_avalaible_robots );
    assert(
        max_robots()==-1 or
        number_of_avalaible_robots <= max_robots()
    );

    return {
        std::pair<rhoban_geometry::Point,ContinuousAngle>(
            ally_goal_center(),
            0.0
        )
    };
}

//
// This function return false if you don't want to
// give a staring position. So the manager will chose
// a default position for you.
//
bool Mur_2::get_starting_position_for_goalie(
    rhoban_geometry::Point & linear_position,
    ContinuousAngle & angular_position
){
    linear_position =  ally_goal_center();
    angular_position = ContinuousAngle(0.0);
    return true;
}

RhobanSSLAnnotation::Annotations Mur_2::get_annotations() const {
    RhobanSSLAnnotation::Annotations annotations;

    for (auto it = this->get_player_ids().begin(); it != this->get_player_ids().end(); it++)
    {
        const rhoban_geometry::Point & robot_position = get_robot(*it).get_movement().linear_position( time() );
        //annotations.addText("Behaviour: " + this->name, robot_position.getX() + 0.15, robot_position.getY(), "white");
        annotations.addText("Strategy: " + this->name, robot_position.getX() + 0.15, robot_position.getY() + 0.30, "white");
    }
    return annotations;
}


}
}
