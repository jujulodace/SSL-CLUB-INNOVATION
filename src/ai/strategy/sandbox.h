#ifndef __STRATEGY__SANDBOX__H__
#define __STRATEGY__SANDBOX__H__

#include "Strategy.h"
#include <string>
#include <AiData.h>

namespace RhobanSSL {
namespace Strategy {

class Sandbox : public Strategy {
    private:
        Ai::AiData & game_state;
        bool behavior_has_been_assigned;
    public:
        int min_robots() const;
        int max_robots() const;

        Sandbox(Ai::AiData & game_state);

        static const std::string name;

        void start(double time);
        void stop(double time);
        
        void assign_behavior_to_robots(
            std::function<
                void (int, std::shared_ptr<RobotBehavior>)
            > assign_behavior,
            double time, double dt
        );
        virtual ~Sandbox();
}; 

};
};

#endif