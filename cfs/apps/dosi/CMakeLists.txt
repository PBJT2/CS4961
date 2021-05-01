cmake_minimum_required(VERSION 2.6.4)
project(CFS_DOSI C)

include_directories(fsw/src)
include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)

aux_source_directory(fsw/src APP_SRC_FILES)
aux_source_directory(fsw/tables APP_TABLE_FILES)

# Create the app module
add_cfe_app(dosi ${APP_SRC_FILES})
add_cfe_tables(dosi ${APP_TABLE_FILES})