add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.tmuxrunner\")

set(tmuxrunner_SRCS runner/tmuxrunner.cpp core/TmuxRunnerAPI.cpp)

kcoreaddons_add_plugin(krunner_tmuxrunner SOURCES ${tmuxrunner_SRCS} INSTALL_NAMESPACE "kf5/krunner")
target_link_libraries(krunner_tmuxrunner KF5::Runner KF5::I18n KF5::Notifications)

set(kcm_krunner_tmuxrunner_SRCS config/tmuxrunner_config.cpp core/TmuxRunnerAPI.cpp core/TmuxRunnerAPI.h)

ki18n_wrap_ui(kcm_krunner_tmuxrunner_SRCS config/tmuxrunner_config.ui)
add_library(kcm_krunner_tmuxrunner MODULE ${kcm_krunner_tmuxrunner_SRCS})
target_link_libraries(kcm_krunner_tmuxrunner
        Qt5::Core
        Qt5::Gui
        KF5::CoreAddons
        KF5::ConfigCore
        KF5::I18n
        KF5::ConfigWidgets
        KF5::Runner
        KF5::Notifications
        )

add_dependencies(krunner_tmuxrunner kcm_krunner_tmuxrunner)

install(TARGETS kcm_krunner_tmuxrunner DESTINATION ${KDE_INSTALL_QTPLUGINDIR})
