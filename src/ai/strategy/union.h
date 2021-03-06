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

#ifndef __STRATEGY__UNION__H__
#define __STRATEGY__UNION__H__

#include "Strategy.h"
#include <string>
#include <memory>

namespace RhobanSSL {
namespace Strategy {


class Union : public Strategy {
    private:
    std::list<std::shared_ptr<Strategy>> strategies_without_goal;
    std::shared_ptr<Strategy> strategy_with_goal;
    int min;
    int max;
 
    public:

    Union(Ai::AiData & ai_data);

    void  clear();

    void add_goalie_strategy( std::shared_ptr<Strategy> strategy );
    void add_strategy( std::shared_ptr<Strategy> strategy );

    virtual void update(double time);

    virtual void start(double time);
    virtual void stop(double time);
    virtual void pause(double time);
    virtual void resume(double time);

    virtual int min_robots() const;
    virtual int max_robots() const;
    virtual Goalie_need needs_goalie() const;

    virtual void assign_behavior_to_robots(
        std::function<
            void (int, std::shared_ptr<Robot_behavior::RobotBehavior>)
        > assign_behavior,
        double time, double dt
    );

    virtual ~Union();
};

};
};
#endif
