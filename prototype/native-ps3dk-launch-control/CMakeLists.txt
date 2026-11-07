cmake_minimum_required(VERSION 3.20)
project(ps3_airplay_native_launch_control C)

include(ps3-self)

add_executable(ps3-airplay-native-control source/main.c)
target_compile_features(ps3-airplay-native-control PRIVATE c_std_11)
target_compile_options(ps3-airplay-native-control PRIVATE
    -Wall -Wextra -Werror
)
target_link_libraries(ps3-airplay-native-control PRIVATE
    rsx gcm_cmd gcm_sys io sysutil rt lv2
)

ps3_add_self(ps3-airplay-native-control)
ps3_add_pkg(ps3-airplay-native-control
    TITLE "PS3 AirPlay Native Launch Control"
    APPID "AIRTEST01"
    CONTENTID "UP0001-AIRTEST01_00-0000000000000000"
    SFOXML "${CMAKE_CURRENT_SOURCE_DIR}/sfo.xml"
)
