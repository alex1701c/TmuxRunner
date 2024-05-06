add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.tmuxrunner\")

kcoreaddons_add_plugin(krunner_tmuxrunner SOURCES runner/tmuxrunner.cpp core/TmuxRunnerAPI.cpp INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/krunner")
target_link_libraries(krunner_tmuxrunner
    KF${QT_MAJOR_VERSION}::Runner
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::Notifications
)

add_library(kcm_krunner_tmuxrunner MODULE config/tmuxrunner_config.cpp core/TmuxRunnerAPI.cpp)
ki18n_wrap_ui(kcm_krunner_tmuxrunner config/tmuxrunner_config.ui)
target_link_libraries(kcm_krunner_tmuxrunner
    Qt::Core
    Qt::Gui
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::KCMUtils
    KF${QT_MAJOR_VERSION}::Notifications
)

install(TARGETS kcm_krunner_tmuxrunner DESTINATION ${KDE_INSTALL_QTPLUGINDIR})
