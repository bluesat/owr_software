cmake_minimum_required(VERSION 2.8.7)
project(bluesat_owr_protobuf)


## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS roscpp std_msgs message_generation)
find_package(Protobuf REQUIRED)





## System dependencies are found with CMake's conventions
# find_package(Boost REQUIRED COMPONENTS system)


## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
# catkin_python_setup()

################################################
## Declare ROS messages, services and actions ##
################################################

## To declare and build messages, services or actions from within this
## package, follow these steps:
## * Let MSG_DEP_SET be the set of packages whose message types you use in
##   your messages/services/actions (e.g. std_msgs, actionlib_msgs, ...).
## * In the file package.xml:
##   * add a build_depend and a run_depend tag for each package in MSG_DEP_SET
##   * If MSG_DEP_SET isn't empty the following dependencies might have been
##     pulled in transitively but can be declared for certainty nonetheless:
##     * add a build_depend tag for "message_generation"
##     * add a run_depend tag for "message_runtime"
## * In this file (CMakeLists.txt):
##   * add "message_generation" and every package in MSG_DEP_SET to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * add "message_runtime" and every package in MSG_DEP_SET to
##     catkin_package(CATKIN_DEPENDS ...)
##   * uncomment the add_*_files sections below as needed
##     and list every .msg/.srv/.action file to be processed
##   * uncomment the generate_messages entry below
##   * add every package in MSG_DEP_SET to generate_messages(DEPENDENCIES ...)

#coverts files from .proto to message
execute_process(COMMAND  ${CMAKE_CURRENT_SOURCE_DIR}/build/convert.pl  ${CMAKE_CURRENT_SOURCE_DIR}/src/message1.proto ${CMAKE_CURRENT_SOURCE_DIR}/msg/message1_ros.msg)
execute_process(COMMAND  ${CMAKE_CURRENT_SOURCE_DIR}/build/convert.pl  ${CMAKE_CURRENT_SOURCE_DIR}/src/battery.proto ${CMAKE_CURRENT_SOURCE_DIR}/msg/battery_ros.msg)
execute_process(COMMAND  ${CMAKE_CURRENT_SOURCE_DIR}/build/convert.pl  ${CMAKE_CURRENT_SOURCE_DIR}/src/signal.proto ${CMAKE_CURRENT_SOURCE_DIR}/msg/signal_ros.msg)
execute_process(COMMAND  ${CMAKE_CURRENT_SOURCE_DIR}/build/convert.pl  ${CMAKE_CURRENT_SOURCE_DIR}/src/tilt.proto ${CMAKE_CURRENT_SOURCE_DIR}/msg/tilt_ros.msg)
## Generate messages in the 'msg' folder
add_message_files(
  FILES
  message1_ros.msg
  battery_ros.msg
  signal_ros.msg
  tilt_ros.msg
)

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
generate_messages(
  DEPENDENCIES
  std_msgs
)

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)
###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS include
# LIBRARIES bluesat_owr_protobuf
  CATKIN_DEPENDS roscpp std_msgs message_runtime
# DEPENDS system_lib convert
)

###########
## Build ##
###########






PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS src/signal.proto src/battery.proto  src/message1.proto src/tilt.proto)


## Declare a cpp library
# add_library(bluesat_owr_protobuf
#   src/${PROJECT_NAME}/bluesat_owr_protobuf.cpp
# )


## Declare a cpp executable
#add_executable(PBufftoRosRelay  src/pbuffToROS.cpp src/Message1Relay.cpp  ${PROTO_SRCS} ${PROTO_HDRS})
#add_executable(ROStoPBuffRelay  src/rosToPbuff.cpp src/Message1Relay.cpp  ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(PBufftoRosRelayBattery  src/pbuffToROSBattery.cpp src/BatteryRelay.cpp  ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(ROStoPBuffRelayBattery  src/rosToPbuffBattery.cpp src/BatteryRelay.cpp  ${PROTO_SRCS} ${PROTO_HDRS})

add_executable(PBufftoRosRelaySignal  src/pbuffToROSSignal.cpp src/SignalRelay.cpp  ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(ROStoPBuffRelaySignal  src/rosToPbuffSignal.cpp src/SignalRelay.cpp  ${PROTO_SRCS} ${PROTO_HDRS})

add_executable(PBufftoRosRelayTilt  src/pbuffToROSSignal.cpp src/SignalRelay.cpp  ${PROTO_SRCS} ${PROTO_HDRS})
add_executable(ROStoPBuffRelayTilt  src/rosToPbuffSignal.cpp src/SignalRelay.cpp  ${PROTO_SRCS} ${PROTO_HDRS})
## Add cmake target dependencies of the executable/library
## as an example, message headers may need to be generated before nodes
add_dependencies(PBufftoRosRelaySignal bluesat_owr_protobuf_generate_messages_cpp)
add_dependencies(ROStoPBuffRelaySignal bluesat_owr_protobuf_generate_messages_cpp)
add_dependencies(PBufftoRosRelayBattery bluesat_owr_protobuf_generate_messages_cpp)
add_dependencies(ROStoPBuffRelayBattery bluesat_owr_protobuf_generate_messages_cpp)
add_dependencies(PBufftoRosRelayTilt bluesat_owr_protobuf_generate_messages_cpp)
add_dependencies(ROStoPBuffRelayTilt bluesat_owr_protobuf_generate_messages_cpp)
## Specify libraries to link a library or executable target against
 target_link_libraries(PBufftoRosRelayBattery
   ${catkin_LIBRARIES}
   ${PROTOBUF_LIBRARIES}
 )
 target_link_libraries(ROStoPBuffRelayBattery
   ${catkin_LIBRARIES}
   ${PROTOBUF_LIBRARIES}
 )
 target_link_libraries(PBufftoRosRelaySignal
   ${catkin_LIBRARIES}
   ${PROTOBUF_LIBRARIES}
 )
 target_link_libraries(ROStoPBuffRelaySignal
   ${catkin_LIBRARIES}
   ${PROTOBUF_LIBRARIES}
 )
 target_link_libraries(PBufftoRosRelayTilt
   ${catkin_LIBRARIES}
   ${PROTOBUF_LIBRARIES}
 )
 target_link_libraries(ROStoPBuffRelayTilt
   ${catkin_LIBRARIES}
   ${PROTOBUF_LIBRARIES}
 )
#  target_link_libraries(PBufftoRosRelay
#   ${catkin_LIBRARIES}
#   ${PROTOBUF_LIBRARIES}
# )
# target_link_libraries(ROStoPBuffRelay
#   ${catkin_LIBRARIES}
#   ${PROTOBUF_LIBRARIES}
# )
#add_dependencies(PBuffRelay ${catkin_EXPORTED_TRAGETS})

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html

## Mark executable scripts (Python etc.) for installation
## in contrast to setup.py, you can choose the destination
# install(PROGRAMS
#   scripts/my_python_script
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark executables and/or libraries for installation
 install(TARGETS PBufftoRosRelayBattery ROStoPBuffRelayBattery PBufftoRosRelaySignal ROStoPBuffRelaySignal
   ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
 )

## Mark cpp header files for installation
 install(DIRECTORY include/${PROJECT_NAME}/
   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
   FILES_MATCHING PATTERN "*.h"
   PATTERN ".svn" EXCLUDE
 )

## Mark other files for installation (e.g. launch and bag files, etc.)
# install(FILES
#   # myfile1
#   # myfile2
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_bluesat_owr_protobuf.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)
