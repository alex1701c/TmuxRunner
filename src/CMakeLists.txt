add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.tmuxrunner\")

set(tmuxrunner_SRCS tmuxrunner.cpp)

add_library(krunner_tmuxrunner MODULE ${tmuxrunner_SRCS})
target_link_libraries(krunner_tmuxrunner KF5::Runner KF5::I18n)

set(kcm_krunner_tmuxrunner_SRCS config/tmuxrunner_config.cpp)

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
        )

add_dependencies(krunner_tmuxrunner kcm_krunner_tmuxrunner)

install(TARGETS krunner_tmuxrunner kcm_krunner_tmuxrunner DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-tmuxrunner.desktop config/plasma-runner-tmuxrunner_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
