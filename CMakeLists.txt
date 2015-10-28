cmake_minimum_required(VERSION 2.8.3)
project(robot)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  message_generation
)


## Generate messages in the 'msg' folder
 add_message_files(
   FILES
   Motors.msg
 )

# Generate services in the 'srv' folder
 add_service_files(
   FILES
   EncoderRequest.srv
 )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

# Generate added messages and services with any dependencies listed here
 generate_messages(
   DEPENDENCIES
   std_msgs
 )

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES beginner_tutorials
  CATKIN_DEPENDS roscpp rospy std_msgs message_runtime
#  DEPENDS system_lib
)

###########
## Build ##
###########

include_directories(
  ${catkin_INCLUDE_DIRS}
)

add_executable(mastermind src/mastermind.cpp)
target_link_libraries(mastermind ${catkin_LIBRARIES})
#add_dependencies(mastermind beginner_tutorials_generate_messages_cpp)
add_executable(fake_arduino src/fake_arduino.cpp)
target_link_libraries(fake_arduino ${catkin_LIBRARIES})
#add_dependencies(mastermind beginner_tutorials_generate_messages_cpp)
# This last add dependancies is worrysome, due to litterally being tutorial dependant