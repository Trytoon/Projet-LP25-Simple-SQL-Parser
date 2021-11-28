cmake_minimum_required(VERSION 3.17)
project(CSQL C)

set(CMAKE_C_STANDARD 99)

add_executable(CSQL project.c check.c check.h database.c database.h project.c sql.c sql.h table.c table.h utils.c utils.h)