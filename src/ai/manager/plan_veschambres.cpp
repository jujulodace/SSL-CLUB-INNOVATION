/*
    This file is part of SSL.

    Copyright 2018 Bezamat Jérémy (jeremy.bezamat@gmail.com)

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

#include "plan_veschambres.h"

// The different strategies
#include <strategy/halt.h>
#include <strategy/tare_and_synchronize.h>
#include <strategy/placer.h>
#include <strategy/prepare_kickoff.h>
#include <strategy/from_robot_behavior.h>

#include <strategy/offensive.h>
#include <strategy/defensive.h>
#include <strategy/defensive_2.h>
#include <strategy/mur.h>
#include <strategy/mur_2.h>
#include <strategy/attaque_with_support.h>



#include <robot_behavior/goalie.h>
#include <robot_behavior/protect_ball.h>

#include <core/collection.h>
#include <core/print_collection.h>


#define GOALIE "goalie"
#define PROTECT_BALL "protect_ball"


namespace RhobanSSL {
namespace Manager {

PlanVeschambres::PlanVeschambres(
    Ai::AiData & ai_data,
    const Referee & referee
):
    Manager(ai_data),
    referee(referee),
    last_referee_changement(0)
{

    register_strategy(
        Strategy::Halt::name, std::shared_ptr<Strategy::Strategy>(
            new Strategy::Halt(ai_data)
        )
    );
    register_strategy(
        Strategy::Tare_and_synchronize::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Tare_and_synchronize(ai_data)
        )
    );
    register_strategy(
        Strategy::Prepare_kickoff::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Prepare_kickoff(ai_data)
        )
    );
    register_strategy(
        GOALIE, std::shared_ptr<Strategy::Strategy>(
            new Strategy::From_robot_behavior(
                ai_data,
                [&](double time, double dt){
                    Robot_behavior::Goalie* goalie = new Robot_behavior::Goalie(ai_data);
                    return std::shared_ptr<Robot_behavior::RobotBehavior>(goalie);
                }, true
            )
        )
    );
    register_strategy(
        PROTECT_BALL, std::shared_ptr<Strategy::Strategy>(
            new Strategy::From_robot_behavior(
                ai_data,
                [&](double time, double dt){
                    Robot_behavior::ProtectBall* protect_ball = new Robot_behavior::ProtectBall(ai_data);
                    return std::shared_ptr<Robot_behavior::RobotBehavior>(protect_ball);
                }, false
            )
        )
    );
    register_strategy(
        Strategy::Offensive::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Offensive(ai_data)
        )
    );
    register_strategy(
        Strategy::Defensive::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Defensive(ai_data)
        )
    );
    register_strategy(
        Strategy::Defensive2::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Defensive2(ai_data)
        )
    );
    register_strategy(
        Strategy::Mur::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Mur(ai_data)
        )
    );
    register_strategy(
        Strategy::Mur_2::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Mur_2(ai_data)
        )
    );
    register_strategy(
        Strategy::AttaqueWithSupport::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::AttaqueWithSupport(ai_data)
        )
    );
    assign_strategy(
        Strategy::Halt::name, 0.0,
        get_team_ids()
    ); // TODO TIME !
}

void PlanVeschambres::analyse_data(double time){
    // We change the point of view of the team
    change_team_and_point_of_view(
        referee.get_team_color( get_team_name() ),
        referee.blue_have_it_s_goal_on_positive_x_axis()
    );
    change_ally_and_opponent_goalie_id(
        referee.blue_goalie_id(),
        referee.yellow_goalie_id()
    );
}


void PlanVeschambres::choose_a_strategy(double time){
    if( referee.edge_entropy() > last_referee_changement ){
        clear_strategy_assignement();
        if( referee.get_state() == Referee_Id::STATE_INIT ){
        } else if( referee.get_state() == Referee_Id::STATE_HALTED ){
            assign_strategy( Strategy::Halt::name, time, get_valid_team_ids() );
        } else if( referee.get_state() == Referee_Id::STATE_STOPPED ){
            if(get_valid_team_ids().size() > 0){
                if( not( get_strategy_<Strategy::Tare_and_synchronize>().is_tared_and_synchronized() ) ){
                    assign_strategy( Strategy::Tare_and_synchronize::name, time, get_valid_player_ids() );
                }else{
                    place_all_the_robots(time, future_strats);
                }
            }
        } else if( referee.get_state() == Referee_Id::STATE_PREPARE_KICKOFF ){
            if( get_team() == referee.kickoff_team() ){
                get_strategy_<Strategy::Prepare_kickoff>().set_kicking(true);
            }else{
                get_strategy_<Strategy::Prepare_kickoff>().set_kicking(false);
            }
            future_strats = { Strategy::Prepare_kickoff::name};
            declare_and_assign_next_strategies( future_strats );
        } else if( referee.get_state() == Referee_Id::STATE_PREPARE_PENALTY ){

            clear_strategy_assignement();
            future_strats = { GOALIE, Strategy::Mur_2::name, Strategy::Defensive2::name, PROTECT_BALL };
            declare_and_assign_next_strategies(future_strats);
            last_referee_changement = referee.edge_entropy();

        } else if( referee.get_state() == Referee_Id::STATE_RUNNING ){
            // future_strats = { Strategy::Defensive2::name };
            if (ball_position().getX() <= 0) {
              //DEFENSIVE
              future_strats = { GOALIE, Strategy::Mur_2::name, Strategy::Defensive2::name, Strategy::Offensive::name };
              is_in_offensive_mode = false;
            }else{
              //OFFENSIVE
              future_strats = { GOALIE, Strategy::Mur::name, Strategy::Defensive2::name, Strategy::AttaqueWithSupport::name };
              is_in_offensive_mode = true;
            }
            declare_and_assign_next_strategies(future_strats);
        } else if( referee.get_state() == Referee_Id::STATE_TIMEOUT ){
            assign_strategy( Strategy::Halt::name, time, get_valid_team_ids() );
        }
        last_referee_changement = referee.edge_entropy();
    }
    else if ( referee.get_state() == Referee_Id::STATE_RUNNING ){
      if ( is_in_offensive_mode && ball_position().getX() <= 0) {
        //DEFENSIVE
        DEBUG("defensive !!!! ");
        future_strats = { Strategy::Mur_2::name, Strategy::Defensive2::name, Strategy::Offensive::name };
        is_in_offensive_mode = false;
        clear_strategy_assignement();
        declare_and_assign_next_strategies(future_strats);
      }
      if( not(is_in_offensive_mode) && ball_position().getX() > 0 ){
        //OFFENSIVE
        DEBUG("offensive !!!! ");
        future_strats = { Strategy::Mur::name, Strategy::Defensive2::name, Strategy::AttaqueWithSupport::name };
        is_in_offensive_mode = true;
        clear_strategy_assignement();
        declare_and_assign_next_strategies(future_strats);
      }
    }
}

void PlanVeschambres::update(double time){
    //update_strategies(time);
    update_current_strategies(time);
    analyse_data(time);
    choose_a_strategy(time);
}

PlanVeschambres::~PlanVeschambres(){ }

};
};
