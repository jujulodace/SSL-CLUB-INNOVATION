#include "Match.h"

// The different strategies
#include <strategy/halt.h>
#include <strategy/tare_and_synchronize.h>
#include <strategy/sandbox.h>
#include <strategy/prepare_kickoff.h>
#include <strategy/prepare_to_run.h>
#include <strategy/from_robot_behavior.h>
#include <strategy/enseirb_project_wrapper.h>
#include <robot_behavior/goalie.h>
#include <core/print_collection.h>

namespace RhobanSSL {
namespace Manager {

Match::Match(
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
        Strategy::Prepare_to_run::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Prepare_to_run(ai_data)
        )
    );
    register_strategy(
        Strategy::Enseirb_project_wrapper::name,
        std::shared_ptr<Strategy::Strategy>(
            new Strategy::Enseirb_project_wrapper(ai_data)
        )
    );
    register_strategy(
        "Goalie", std::shared_ptr<Strategy::Strategy>(
            new Strategy::From_robot_behavior(
                ai_data,
                [&](double time, double dt){
                    Robot_behavior::Goalie* goalie = new Robot_behavior::Goalie(ai_data);
                    return std::shared_ptr<Robot_behavior::RobotBehavior>(goalie);
                }, true //it is a goal
            )
        )
    );
    assign_strategy(
        Strategy::Halt::name, 0.0, 
        get_team_ids()
    ); // TODO TIME !
}

void Match::analyse_data(double time){
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
void Match::choose_a_strategy(double time){
    if( referee.edge_entropy() > last_referee_changement ){
        clear_strategy_assignement();
        if( referee.get_state() == Referee_Id::STATE_INIT ){
        } else if( referee.get_state() == Referee_Id::STATE_HALTED ){
            assign_strategy( Strategy::Halt::name, time, get_valid_team_ids() );
        } else if( referee.get_state() == Referee_Id::STATE_STOPPED ){
            if( get_valid_team_ids().size() > 0  ){
                assign_strategy( Strategy::Tare_and_synchronize::name, time, get_valid_team_ids() );
            }
        } else if( referee.get_state() == Referee_Id::STATE_PREPARE_KICKOFF ){
            assign_strategy( Strategy::Prepare_kickoff::name, time, get_valid_team_ids() );
            if( get_team() == referee.kickoff_team() ){
                get_strategy_<Strategy::Prepare_kickoff>().set_kicking(true);
            }else{
                get_strategy_<Strategy::Prepare_kickoff>().set_kicking(false);
            }
        } else if( referee.get_state() == Referee_Id::STATE_PREPARE_PENALTY ){
        } else if( referee.get_state() == Referee_Id::STATE_RUNNING ){
            //assign_strategy( Strategy::Enseirb_project_wrapper::name, time, get_valid_team_ids() );
            assign_strategy( "Goalie", time, get_valid_team_ids() );
        } else if( referee.get_state() == Referee_Id::STATE_TIMEOUT ){
            assign_strategy( Strategy::Halt::name, time, get_valid_team_ids() );
        }
        last_referee_changement = referee.edge_entropy();
    }
}

void Match::update(double time){
    //update_strategies(time);
    update_current_strategies(time);
    analyse_data(time);
    choose_a_strategy(time);
}

Match::~Match(){ }

};
};
