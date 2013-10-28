/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright 2013  Vishesh Handa <me@vhanda.in>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tagfetchjob.h"
#include "tag.h"

#include <QTimer>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <KDebug>

class TagFetchJob::Private {
public:
    Tag tag;
};

TagFetchJob::TagFetchJob(const Tag& tag, QObject* parent)
    : ItemFetchJob(parent)
    , d(new Private)
{
    d->tag = tag;
}

TagFetchJob::~TagFetchJob()
{
    delete d;
}

void TagFetchJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void TagFetchJob::doStart()
{
    QSqlQuery query;
    query.setForwardOnly(true);

    if (d->tag.id().size()) {
        const QByteArray& arr = d->tag.id();
        int id = arr.mid(4).toInt(); // "tag:" takes 4 characters

        if (id <= 0) {
            setError(Error_TagInvalidId);
            setErrorText("Invalid id " + d->tag.id());
            emitResult();
            return;
        }
        query.prepare("select name from tags where id = ?");
        query.addBindValue(id);

        if (!query.exec()) {
            setError(Error_ConnectionError);
            setErrorText(query.lastError().text());
            emitResult();
            return;
        }

        if (query.next()) {
            QString name = query.value(0).toString();
            d->tag.setName(name);
        }
        else {
            setError(Error_TagDoesNotExist);
            setErrorText("Invalid ID " + d->tag.id());
            emitResult();
            return;
        }

        emit itemReceived(d->tag);
        emit tagReceived(d->tag);
    }
    else if (d->tag.name().size()) {
        query.prepare("select id from tags where name = ?");
        query.addBindValue(d->tag.name());

        if (!query.exec()) {
            setError(Error_ConnectionError);
            setErrorText(query.lastError().text());
            emitResult();
            return;
        }

        if (query.next()) {
            int id = query.value(0).toInt();
            d->tag.setId(QByteArray("tag:") + QByteArray::number(id));
        }
        else {
            setError(Error_TagDoesNotExist);
            setErrorText("Tag " + d->tag.name() + " does not exist");
            emitResult();
            return;
        }

        emit itemReceived(d->tag);
        emit tagReceived(d->tag);
    }

    emitResult();
}
