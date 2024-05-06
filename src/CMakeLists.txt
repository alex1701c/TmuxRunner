add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.tmuxrunner\")

set(tmuxrunner_SRCS runner/tmuxrunner.cpp core/TmuxRunnerAPI.cpp)

kcoreaddons_add_plugin(krunner_tmuxrunner SOURCES ${tmuxrunner_SRCS} INSTALL_NAMESPACE "kf5/krunner")
target_link_libraries(krunner_tmuxrunner
    KF${QT_MAJOR_VERSION}::Runner
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::Notifications
)

set(kcm_krunner_tmuxrunner_SRCS config/tmuxrunner_config.cpp core/TmuxRunnerAPI.cpp core/TmuxRunnerAPI.h)

ki18n_wrap_ui(kcm_krunner_tmuxrunner_SRCS config/tmuxrunner_config.ui)
add_library(kcm_krunner_tmuxrunner MODULE ${kcm_krunner_tmuxrunner_SRCS})
target_link_libraries(kcm_krunner_tmuxrunner
    Qt::Core
    Qt::Gui
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::KCMUtils
    KF${QT_MAJOR_VERSION}::Notifications
)

add_dependencies(krunner_tmuxrunner kcm_krunner_tmuxrunner)

install(TARGETS kcm_krunner_tmuxrunner DESTINATION ${KDE_INSTALL_QTPLUGINDIR})
