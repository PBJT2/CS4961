cmake_minimum_required(VERSION 2.6.4)
project(CFS_DOSD C)

include_directories(fsw/src)
include_directories(fsw/mission_inc)
include_directories(fsw/platform_inc)

aux_source_directory(fsw/src APP_SRC_FILES)
aux_source_directory(fsw/tables APP_TABLE_FILES)

# Create the app module
add_cfe_app(dosd ${APP_SRC_FILES})
add_cfe_tables(dosd ${APP_TABLE_FILES})