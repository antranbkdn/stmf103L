CFLAGS		+= -I./sources/app
CPPFLAGS	+= -I./sources/app

VPATH += sources/app

# CPP source files
SOURCES_CPP += sources/app/app.cpp
SOURCES_CPP += sources/app/app_screen.cpp
SOURCES_CPP += sources/app/rf_remote_ctrl.cpp
SOURCES_CPP += sources/app/shell.cpp

SOURCES_CPP += sources/app/task_shell.cpp
SOURCES_CPP += sources/app/task_life.cpp
SOURCES_CPP += sources/app/task_list.cpp
SOURCES_CPP += sources/app/task_if.cpp
SOURCES_CPP += sources/app/task_gw_if.cpp
SOURCES_CPP += sources/app/task_time.cpp
SOURCES_CPP += sources/app/task_sm.cpp
SOURCES_CPP += sources/app/task_ir.cpp
SOURCES_CPP += sources/app/task_sensor.cpp
SOURCES_CPP += sources/app/task_ui.cpp
SOURCES_CPP += sources/app/task_setting.cpp
SOURCES_CPP += sources/app/task_firmware.cpp
