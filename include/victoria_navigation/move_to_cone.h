// Copyright 2017 Michael Wimble

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
// or promote products derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef __MOVE_TO_CONE
#define __MOVE_TO_CONE

#include <ros/ros.h>
#include <string>

#include "victoria_perception/ObjectDetector.h"
#include "victoria_navigation/strategy_fn.h"

// A behavior that attempts to move to a RoboMagellan cone.
//
// The behavior depends on a few other components and parameters..
//		* "cone_detector_topic_name" is a topic listened to for an indication if a RoboMagellan cone is detected.
//		* "cmd_vel_topic_name" defines a topic to be used for moving the robot. Messages will be published
//		  to that topic. The robot will end up in a stopped state at the end of this behavior.
//
// The behavior works as follows:
//	* Wait until messages are received from the cone detecto.
//  * If the cone is not seen, indicate FAILURE and stop the robot.
//	* If the cone is not more or less dead ahead, rotate a bit so that it is more centered.
//	* [TEMPORARY] if the cone is "very close", stop the robot and indicate success.
//	* [MISSING] If the robot is touching the cone, stop the robot and indicate success.
//	* Else move the robot forward a bit.

class MoveToCone : public StrategyFn {
private:
	enum STATE {
		kMOVING_TO_CENTERING_POSITION,	// Rotate so the cone is more or less dead center ahead.
		kMOVING_TO_TOUCH				// Move forward to touch the cone.
	};

	// Parameters.
	std::string cmd_vel_topic_name_;			// Topic name containing cmd_vel message.
	std::string cone_detector_topic_name_;	// Topic name containing cone_detector message

	// Publishers.
	ros::Publisher cmd_vel_pub_;

	// Subscribers.
	ros::Subscriber	cone_detector_sub_;

	// Algorithm variables.
	STATE state_;
	
	// Process one cone detector topic message.
	long int count_ObjectDetector_msgs_received_;
	victoria_perception::ObjectDetector last_object_detected_;
	void coneDetectorCb(const victoria_perception::ObjectDetectorConstPtr& msg);

	// Reset goal. After this, someone must request the goal again and it will start over.
	void resetGoal();

	// Singleton pattern.
	MoveToCone();
	MoveToCone(MoveToCone const&) {};
	MoveToCone& operator=(MoveToCone const&) {};

public:
	RESULT_T tick();

	std::string goalName() { return "/strategy/need_to_move_cone"; }

	std::string name() { return std::string("MoveToCone"); };

	static MoveToCone& singleton();

	std::string stateName(STATE state) {
		switch (state) {
			case kMOVING_TO_CENTERING_POSITION:		return "kMOVING_TO_CENTERING_POSITION";
			case kMOVING_TO_TOUCH:					return "kMOVING_TO_TOUCH";
			default:								return "!!UNKNOWN!!";
		}
	}

};

#endif