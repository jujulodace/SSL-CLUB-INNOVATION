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

#ifndef __ROBOT_BEHAVIOR__PREDICTBALL__H__
#define __ROBOT_BEHAVIOR__PREDICTBALL__H__

#include "robot_behavior.h"
#include "factory.h"

namespace RhobanSSL
{
  namespace Robot_behavior {

    class PredictFutur : public RobotBehavior  {
    private:
      bool use_custom_vector;
      rhoban_geometry::Point striking_point;
      ConsignFollower* follower;
      RhobanSSLAnnotation::Annotations annotations;
    public:
      PredictFutur(Ai::AiData& ai_data);

      virtual void update(
        double time,
        const Ai::Robot & robot,
        const Ai::Ball & ball
      );

      virtual Control control() const;

      void declare_point_to_strik( rhoban_geometry::Point point );

      virtual RhobanSSLAnnotation::Annotations get_annotations() const;

      virtual ~PredictFutur();
    };

  };
}; //Namespace Rhoban

#endif
