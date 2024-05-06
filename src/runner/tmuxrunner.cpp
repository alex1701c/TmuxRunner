#include "tmuxrunner.h"

// KF
#include <KLocalizedString>
#include <KSharedConfig>
#include <QAction>
#include <QDir>
#include <QFile>

TmuxRunner::TmuxRunner(QObject *parent, const KPluginMetaData &data, const QVariantList &)
#if QT_VERSION_MAJOR == 5
    : KRunner::AbstractRunner(parent, data, QVariantList{})
#else
    : KRunner::AbstractRunner(parent, data)
#endif

    , api(config())
{
}

void TmuxRunner::reloadConfiguration()
{
    // Method was triggered using file watcher => get new state from file
    const KConfigGroup grp = config();
    enableTmuxinator = grp.readEntry("enable_tmuxinator", true);
    enableFlags = grp.readEntry("enable_flags", true);
    defaultProgram = grp.readEntry("program", "konsole");
    const QString actionChoiceText = grp.readEntry("action_program", "None");
    const QString actionChoice = actionChoiceText.toLower();
    if (actionChoice == QLatin1String("none")) {
        actionList.clear();
    } else {
        QIcon actionIcon = QIcon::fromTheme("utilities-terminal");
        if (actionChoice == QLatin1String("yakuake")) {
            actionProgram = "yakuake-session";
            actionIcon = QIcon::fromTheme("yakuake", actionIcon);
        } else if (actionChoice == QLatin1String("Simple Terminal (st)")) {
            actionProgram = "st";
        } else {
            // Konsole or custom can be youst uses because they are lowercase
            actionProgram = actionChoice;
        }

#if QT_VERSION_MAJOR == 5
        actionList = {new QAction(actionIcon, "Open session in " + actionChoiceText, this)};
#else
        actionList = {KRunner::Action("open-in", actionIcon.name(), "Open session in " + actionChoiceText)};
#endif
    }

    if (enableTmuxinator) {
        tmuxinatorConfigs = api.fetchTmuxinatorConfigs();
        enableTmuxinator = !tmuxinatorConfigs.isEmpty();
    }

    setSyntaxes({
        KRunner::RunnerSyntax("tmux", "List available tmux sessions"),
        KRunner::RunnerSyntax("tmux :q:", "Filter sessions or create new session for the given term"),
    });
}

void TmuxRunner::match(KRunner::RunnerContext &context)
{
    QString term = context.query();
    if (!term.startsWith(triggerWord))
        return;
    term.remove(triggerWordRegex);
    QList<KRunner::QueryMatch> matches;
    bool exactMatch = false;
    bool tmuxinator = false;

    QString program = defaultProgram;
    QStringList attached;

    // Flags to open other terminal emulator
    QString openIn;
    if (enableFlags) {
        QString flagProgram = api.parseQueryFlags(term, openIn);
        if (!flagProgram.isEmpty()) {
            program = flagProgram;
        }
    }

    // Session with Tmuxinator
    if (enableTmuxinator && term.startsWith(tmuxinatorQuery)) {
        tmuxinator = true;
        context.addMatches(addTmuxinatorMatches(term, openIn, program, attached));
    }

    // Attach to session options
    context.addMatches(addTmuxAttachMatches(term, openIn, program, attached, &exactMatch));

    // New session
    if (!exactMatch) {
        context.addMatches(addTmuxNewSessionMatches(term, openIn, program, tmuxinator));
    }

    context.addMatches(matches);
}

void TmuxRunner::run(const KRunner::RunnerContext &, const KRunner::QueryMatch &match)
{
    QMap<QString, QVariant> data = match.data().toMap();
    QString program = data.value("program", defaultProgram).toString();
    if (match.selectedAction()) {
        program = actionProgram;
    }
    const QString target = data.value("target").toString();

    if (data.value("action") == "attach") {
        api.executeAttatchCommand(program, target);
    } else {
        api.executeCreateCommand(program, target, data);
    }
}

KRunner::QueryMatch TmuxRunner::createMatch(const QString &text, const QMap<QString, QVariant> &data, float relevance)
{
    KRunner::QueryMatch match(this);
    match.setIcon(icon);
    match.setText(text);
    match.setData(data);
    match.setRelevance(relevance);
    match.setActions(actionList);
    return match;
}

QList<KRunner::QueryMatch> TmuxRunner::addTmuxinatorMatches(QString &term, const QString &openIn, const QString &program, QStringList &attached)
{
    QList<KRunner::QueryMatch> matches;
    const static QRegularExpression tmuxinatorQueryRegex = QRegularExpression(R"(inator(?: (\w+) *(.+)?)?)");
    const QRegularExpressionMatch tmuxinatorMatch = tmuxinatorQueryRegex.match(term);

    const static QRegularExpression tmuxinatorClearRegex = QRegularExpression("^inator *");
    term.remove(tmuxinatorClearRegex);
    const QString filter = tmuxinatorMatch.captured(1);
    for (const auto &tmuxinatorConfig : std::as_const(tmuxinatorConfigs)) {
        if (tmuxinatorConfig.startsWith(filter)) {
            if (!tmuxSessions.contains(tmuxinatorConfig)) {
                matches.append(
                    createMatch("Create Tmuxinator  " + tmuxinatorConfig + openIn,
                                {{"action", "tmuxinator"}, {"program", program}, {"args", tmuxinatorMatch.captured(2)}, {"target", tmuxinatorConfig}},
                                1));
            } else {
                attached.append(tmuxinatorConfig);
                matches.append(createMatch("Attach Tmuxinator  " + tmuxinatorConfig + openIn,
                                           {
                                               {"action", "attach"},
                                               {"program", program},
                                               {"target", tmuxinatorConfig},
                                           },
                                           0.99));
            }
        }
    }
    return matches;
}

QList<KRunner::QueryMatch>
TmuxRunner::addTmuxAttachMatches(QString &term, const QString &openIn, const QString &program, QStringList &attached, bool *exactMatch)
{
    QList<KRunner::QueryMatch> matches;
    const auto queryName = term.contains(' ') ? term.split(' ').first() : term;
    for (const auto &session : std::as_const(tmuxSessions)) {
        if (session.startsWith(queryName)) {
            if (session == queryName)
                *exactMatch = true;
            if (attached.contains(session))
                continue;
            matches.append(createMatch("Attach to " + session + openIn,
                                       {{"action", "attach"}, {"program", program}, {"target", session}},
                                       (float)queryName.length() / (float)session.length()));
        }
    }
    return matches;
}

QList<KRunner::QueryMatch> TmuxRunner::addTmuxNewSessionMatches(QString &term, const QString &openIn, const QString &program, bool tmuxinator)
{
    QList<KRunner::QueryMatch> matches;
    // Name and optional path, Online tester : https://regex101.com/r/FdZcIZ/1
    const static QRegularExpression regex(R"(^([\w-]+)(?: +(.+)?)?$)");
    const auto matchResult = regex.match(term);

    // No path
    if (matchResult.captured(2).isEmpty()) {
        if (!matchResult.captured(1).isEmpty() || !tmuxinator) {
            matches.append(createMatch("New session " + matchResult.captured(1) + openIn,
                                       {{"program", program}, {"action", "new"}, {"target", matchResult.captured(1)}},
                                       0));
        }
        // With path
    } else {
        matches.append(createMatch("New session " + matchResult.captured(1) + " in " + matchResult.captured(2) + openIn,
                                   {{"program", program}, {"action", "new"}, {"path", matchResult.captured(2)}, {"target", matchResult.captured(1)}},
                                   0));
    }
    return matches;
}

K_PLUGIN_CLASS_WITH_JSON(TmuxRunner, "tmuxrunner.json")

#include "moc_tmuxrunner.cpp"
#include "tmuxrunner.moc"
