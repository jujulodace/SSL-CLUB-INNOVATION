#include "enseirb_project_wrapper.h"
#include <robot_behavior/do_nothing.h>
#include <robot_behavior/position_follower.h>
#include <robot_behavior/apply_enseirb_project_action.h>

namespace RhobanSSL {
namespace Strategy {

Enseirb_project_wrapper::Enseirb_project_wrapper(Ai::AiData & ai_data):
    Strategy(ai_data),
    robots(0), nb_robots(0)
{
}

const std::string Enseirb_project_wrapper::name="enseirb_project_wrapper";

int Enseirb_project_wrapper::min_robots() const {
    return 0;
}
int Enseirb_project_wrapper::max_robots() const {
    return -1;
}

void Enseirb_project_wrapper::allocate_enseirb_data(){
    nb_robots = 0;
    for( auto team: {Vision::Ally, Vision::Opponent} ){
        nb_robots+=ai_data.robots[team].size();
    }
    robot_actions = std::vector<enseirb::Action>(get_robot_ids().size());
    robots = new enseirb::Robot[nb_robots];
}

void Enseirb_project_wrapper::initialize_enseirb_data(){
    config.width = ai_data.field.fieldLength;
    config.height = ai_data.field.fieldWidth;
    config.goal_size = ai_data.field.goalWidth;
    config.margin = ai_data.field.boundaryWidth;
    ball.radius = ai_data.constants.radius_ball;
    int i=0;
    for( auto team: {Vision::Ally, Vision::Opponent} ){
        for( const std::pair<int, Ai::Robot> & elem : ai_data.robots[team] ){
            int id = elem.first;
            // index_2_robot[i] = std::pair<Vision::Team, int>( team, id );
            robot_2_index[std::pair<Vision::Team, int>( team, id )] = i;
            robots[i].id = id;
            robots[i].radius = ai_data.constants.robot_radius;
            robots[i].team = ( (team == Vision::Ally)? enseirb::Team::ALLY : enseirb::Team::OPPONENT );
            robots[i].is_goal = (
                ( (team==Vision::Ally) and (get_goalie()==id) )
                or 
                ( (team==Vision::Opponent) and (get_goalie_opponent()==id) )
            );
            robots[i].is_valid = ai_data.robot_is_valid(id);
            //Robots[i].behaviour.id = // TODO 
            i+=1;
        }
    }
}

void Enseirb_project_wrapper::desallocate_enseirb_data(){
    delete robots;
    robots = 0;
}

void Enseirb_project_wrapper::start(double time){
    DEBUG("START ENSEIRB");
    allocate_enseirb_data();
    initialize_enseirb_data();
    
    start_strategy(&config, robots, nb_robots, enseirb::Team::ALLY);

    behavior_has_been_assigned = false;
}

void Enseirb_project_wrapper::stop(double time){
    stop_strategy(&config, robots, nb_robots, enseirb::Team::ALLY);
    desallocate_enseirb_data();
    DEBUG("STOP ENSEIRB");
}

void Enseirb_project_wrapper::assign_behavior_to_robots(
    std::function<
        void (int, std::shared_ptr<Robot_behavior::RobotBehavior>)
    > assign_behavior,
    double time, double dt
){
    if( ! behavior_has_been_assigned ){
        setBehaviour(&config, robots, nb_robots, enseirb::Team::ALLY);
        for( unsigned int i=0; i<get_robot_ids().size(); i++ ){
            Robot_behavior::Apply_enseirb_project_action *robot_behavior = new Robot_behavior::Apply_enseirb_project_action(
                ai_data,
                robot_actions[i],
                time, dt
            ) ;

            robot_behavior->set_translation_pid(
                ai_data.constants.p_translation,
                ai_data.constants.i_translation, 
                ai_data.constants.d_translation
            );
            robot_behavior->set_orientation_pid(
                ai_data.constants.p_orientation, ai_data.constants.i_orientation, 
                ai_data.constants.d_orientation
            );
            robot_behavior->set_limits(
                ai_data.constants.translation_velocity_limit,
                ai_data.constants.rotation_velocity_limit,
                ai_data.constants.translation_acceleration_limit,
                ai_data.constants.rotation_acceleration_limit
            );

            assign_behavior(
                robot_id(i), std::shared_ptr<Robot_behavior::RobotBehavior>( robot_behavior )
            );
        }
        behavior_has_been_assigned = true;
    }
}

void Enseirb_project_wrapper::update(double time){
    ContinuousAngle angular_position;
    ContinuousAngle angular_velocity;

    //update ball
    const RhobanSSL::Movement & mov = ai_data.ball.get_movement();

    rhoban_geometry::Point linear_position = mov.linear_position(time );
    Vector2d linear_velocity = mov.linear_velocity(time);

    this->ball.x = linear_position.getX();
    this->ball.y = linear_position.getY();
    this->ball.vx = linear_velocity.getX();
    this->ball.vy = linear_velocity.getY();

    //update all robot positions
    for( auto team: {Vision::Ally, Vision::Opponent} ){
        for( const std::pair<int, Ai::Robot> & elem : ai_data.robots[team] ){
            int id = elem.first;
            const RhobanSSL::Movement & robot_mov = ai_data.ball.get_movement();
            linear_position = robot_mov.linear_position(time );
            angular_position = robot_mov.angular_position(time);
            linear_velocity = robot_mov.linear_velocity(time);
            angular_velocity = robot_mov.angular_velocity(time);
            int i = robot_2_index.at( std::pair<Vision::Team, int>(team, id) );
            robots[i].x = linear_position.getX();
            robots[i].y = linear_position.getY(); 
            robots[i].t = angular_position.value();
            robots[i].vx = linear_velocity.getX();
            robots[i].vy = linear_velocity.getY(); 
            robots[i].vt = angular_velocity.value();
        }
    }
 
    update_strategy(&config, robots, nb_robots, enseirb::Team::ALLY, time);

    // get robot actions
    for( unsigned int i=0; i<get_robot_ids().size(); i++ ){
        enseirb::Action action = getBehaviour(
            &config, robots, nb_robots, &ball, 
            robot_2_index.at(
                std::pair<Vision::Team, int>(Vision::Team::Ally, robot_id(i))
            )
        );
        robot_actions[i] = action;
    }
}

Enseirb_project_wrapper::~Enseirb_project_wrapper(){
    desallocate_enseirb_data();
}


}
}