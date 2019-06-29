/*
   Copyright %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tmuxrunner.h"

// KF
#include <KLocalizedString>
#include <QtGui/QtGui>
#include <KSharedConfig>


TmuxRunner::TmuxRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args) {
    setObjectName(QStringLiteral("TmuxRunner"));
}

TmuxRunner::~TmuxRunner() = default;


void TmuxRunner::init() {
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("TmuxRunner");
    config.writeEntry("program", "konsole");
    connect(this, SIGNAL(prepare()), this, SLOT(prepareForMatchSession()));
}

void TmuxRunner::prepareForMatchSession() {
    tmuxSessions.clear();
    QProcess process;
    process.start("tmux", QStringList() << "ls");
    process.waitForFinished();
    while (process.canReadLine()) {
        QString line = process.readLine();
        if (line.contains(':')) {
            tmuxSessions.append(line.split(':').first());
        }
    }

}

void TmuxRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;
    QString term = context.query();
    if (!term.startsWith("tmux")) return;
    term.replace(QRegExp("tmux *"), "");
    QList<Plasma::QueryMatch> matches;

    QString tmpTerm;
    if (term.contains(' ')) {
        tmpTerm = term.split(' ').first();
    } else {
        tmpTerm = term;
    }
    for (const auto &session:tmuxSessions) {
        if (session.startsWith(tmpTerm)) {
            matches.append(addMatch("Attatch to " + session, "attatch|" + session,
                                    (float) tmpTerm.length() / (float) session.length()));
        }
    }

    if (matches.empty()) {
        // Name, speces, path
        QRegExp regex(R"(^([\w-]+)(?: +(.+)?)?$)");
        regex.indexIn(term);
        const auto texts = regex.capturedTexts();
        //New session
        if (texts.size() == 2 || texts.at(2).isEmpty()) {
            matches.append(addMatch("New session " + texts.at(1), "new|" + texts.at(1), 1));
            //New session with path
        } else if (texts.size() == 3) {
            matches.append(addMatch("New session " + texts.at(1) + " in " + texts.at(2),
                                    "new|" + texts.at(1) + "|" + texts.at(2), 1));
        }
    }

    context.addMatches(matches);
}

void TmuxRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)
    QList<QString> entries = match.data().toString().split('|');
    QString option = config.readEntry("program", "konsole");
    QString program = "konsole";

    if (entries.first() == "attatch") {
        QStringList args = {"-e", "tmux", "a", "-t", entries.last()};

        if (option == "yakuake") {
            program = "yakuake-session";
            args.clear();
            args.append({"-t", entries.last(), "-e", "tmux", "attach-session", "-t", entries.last()});
        } else if (option == "terminator") {
            program = "terminator";
            args.clear();
            args.append({"-x", "tmux", "a", "-t", entries.last()});
        } else if (option == "st") {
            program = "st";
            args.clear();
            args.append({"tmux", "attach-session", "-t", entries.last()});
        }
        QProcess::startDetached(program, args);
    } else {
        QStringList args = {"-e", "tmux", "new-session", "-s", entries.at(1)};
        if (entries.size() == 3) {
            QString path = entries.last();
            if (!(path.startsWith('/') || path.startsWith('~'))) {
                path.insert(0, "~/");
            }
            args.append({"-c", path});
        }
        qInfo() << program << args;
        QProcess::startDetached(program, args);
    }
}

Plasma::QueryMatch TmuxRunner::addMatch(const QString &text, const QString &data, float relevance) {
    Plasma::QueryMatch match(this);
    match.setIconName("utilities-terminal");
    match.setText(text);
    match.setData(data);
    match.setRelevance(relevance);
    return match;
}


K_EXPORT_PLASMA_RUNNER(tmuxrunner, TmuxRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "tmuxrunner.moc"
