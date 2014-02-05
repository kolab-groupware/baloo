/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "filemodifyjob.h"
#include "filemapping.h"
#include "db.h"
#include "file.h"
#include "searchstore.h"
#include "filecustommetadata.h"

#include <KDebug>

#include <QTimer>
#include <QFile>
#include <QStringList>

#include <xapian.h>

#include <QDBusMessage>
#include <QDBusConnection>

using namespace Baloo;

class Baloo::FileModifyJob::Private {
public:
    QList<File> files;
    int rating;
    QString comment;
    QStringList tags;

    Private() : rating(0) {}
};

FileModifyJob::FileModifyJob(QObject* parent)
    : KJob(parent)
    , d(new Private)
{
}

FileModifyJob::FileModifyJob(const File& file, QObject* parent)
    : KJob(parent)
    , d(new Private)
{
    d->files.append(file);
    d->rating = file.rating();
    d->comment = file.userComment();
    d->tags = file.tags();
}

FileModifyJob::~FileModifyJob()
{
    delete d;
}

void FileModifyJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

namespace {

void removeTerms(Xapian::Document& doc, const std::string& p) {
    QByteArray prefix = QByteArray::fromRawData(p.c_str(), p.size());

    Xapian::TermIterator it = doc.termlist_begin();
    it.skip_to(p);
    while (it != doc.termlist_end()){
        const std::string t = *it;
        const QByteArray term = QByteArray::fromRawData(t.c_str(), t.size());
        if (!term.startsWith(prefix)) {
            break;
        }

        // The term should not just be the prefix
        if (term.size() <= prefix.size())
            break;

        // The term should not contain any more upper case letters
        if (isupper(term.at(prefix.size())))
            break;

        it++;
        doc.remove_term(t);
    }
}

}

void FileModifyJob::doStart()
{
    QStringList updatedFiles;

    Q_FOREACH (const File& file, d->files) {
        FileMapping fileMap(file.url());
        if (!file.id().isEmpty()) {
            int id = deserialize("file", file.id());
            fileMap.setId(id);
        }

        if (!fileMap.fetched()) {
            if (fileMap.empty()) {
                setError(Error_EmptyFile);
                setErrorText(QLatin1String("Invalid Argument"));
                emitResult();
                return;
            }

            if (!fileMap.fetch(fileMappingDb())) {
                fileMap.create(fileMappingDb());
            }
        }

        if (!QFile::exists(fileMap.url())) {
            setError(Error_FileDoesNotExist);
            setErrorText("Does not exist " + fileMap.url());
            emitResult();
            return;
        }

        updatedFiles << fileMap.url();
        const QString furl = fileMap.url();

        if (d->rating) {
            const QString rat = QString::number(d->rating);
            setCustomFileMetaData(furl, QLatin1String("user.baloo.rating"), rat);
        }

        if (!d->tags.isEmpty()) {
            QString tags = d->tags.join(",");
            setCustomFileMetaData(furl, QLatin1String("user.baloo.tags"), tags);
        }

        if (!d->comment.isEmpty()) {
            setCustomFileMetaData(furl, QLatin1String("user.xdg.comment"), d->comment);
        }

        // Save in Xapian
        Xapian::WritableDatabase db(fileIndexDbPath(), Xapian::DB_CREATE_OR_OPEN);
        Xapian::Document doc;

        try {
            doc = db.get_document(fileMap.id());

            removeTerms(doc, "R");
            removeTerms(doc, "T");
            removeTerms(doc, "TAG");
            removeTerms(doc, "C");
        }
        catch (const Xapian::DocNotFoundError&) {
        }

        const int rating = d->rating;
        if (rating > 0) {
            const QString ratingStr = QLatin1Char('R') + QString::number(d->rating);
            doc.add_boolean_term(ratingStr.toUtf8().constData());
        }

        Xapian::TermGenerator termGen;
        termGen.set_document(doc);

        Q_FOREACH (const QString& tag, d->tags) {
            termGen.index_text(tag.toUtf8().constData(), 1, "T");

            const QString tagStr = QLatin1String("TAG") + tag;
            doc.add_boolean_term(tagStr.toUtf8().constData());
        }

        if (!d->comment.isEmpty()) {
            Xapian::TermGenerator termGen;
            termGen.set_document(doc);

            const QByteArray str = d->comment.toUtf8();
            termGen.index_text(str.constData(), 1, "C");
        }

        db.replace_document(fileMap.id(), doc);
        db.commit();
    }

    // Notify the world?
    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(updatedFiles);
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);

    emitResult();
}

namespace {
    QList<File> convertToFiles(const QStringList& fileurls)
    {
        QList<File> files;
        Q_FOREACH (const QString& url, fileurls) {
            files << File(url);
        }
        return files;
    }
}

FileModifyJob* FileModifyJob::modifyRating(const QStringList& files, int rating)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = convertToFiles(files);
    job->d->rating = rating;

    return job;
}

FileModifyJob* FileModifyJob::modifyTags(const QStringList& files, const QStringList& tags)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = convertToFiles(files);
    job->d->tags = tags;

    return job;
}

FileModifyJob* FileModifyJob::modifyUserComment(const QStringList& files, const QString& comment)
{
    FileModifyJob* job = new FileModifyJob();
    job->d->files = convertToFiles(files);
    job->d->comment = comment;

    return job;
}
