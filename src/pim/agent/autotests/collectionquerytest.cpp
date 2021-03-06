/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2014  Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QTest>
#include <Akonadi/Collection>
#include <KABC/Addressee>
#include <QDir>

#include <query.h>
#include <index.h>
#include <../pim/lib/collectionquery.h>
#include <../pim/lib/resultiterator.h>

Q_DECLARE_METATYPE(QSet<qint64>)
Q_DECLARE_METATYPE(QList<qint64>)

class CollectionQueryTest : public QObject
{
    Q_OBJECT
private:
    QString collectionsDir;

    bool removeDir(const QString & dirName)
    {
        bool result = true;
        QDir dir(dirName);

        if (dir.exists(dirName)) {
            Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
                if (info.isDir()) {
                    result = removeDir(info.absoluteFilePath());
                }
                else {
                    result = QFile::remove(info.absoluteFilePath());
                }

                if (!result) {
                    return result;
                }
            }
            result = dir.rmdir(dirName);
        }
        return result;
    }

    QString dbPrefix;
    Index index;

private Q_SLOTS:
    void init() {
        dbPrefix = QDir::tempPath() + QLatin1String("/collectiontest");
        index.setOverrideDbPrefixPath(dbPrefix);
        index.createIndexers();

    }

    void cleanup() {
        removeDir(dbPrefix);
    }

    void searchByName() {
        Akonadi::Collection col1(3);
        col1.setName(QLatin1String("col3"));
        index.index(col1);

        Akonadi::Collection col2(4);
        col2.setName(QLatin1String("col4"));
        col2.setParentCollection(col1);
        index.index(col2);

        Baloo::PIM::CollectionQuery query;
        query.setDatabaseDir(dbPrefix+QLatin1String("/collections/"));
        query.nameMatches(col1.name());
        Baloo::PIM::ResultIterator it = query.exec();
        QList<qint64> results;
        while (it.next()) {
            // kDebug() << "result " << it.id();
            results << it.id();
        }
        QCOMPARE(results.size(), 1);
        QCOMPARE(results.at(0), col1.id());
    }

    void searchHierarchy() {
        Akonadi::Collection col1(1);
        col1.setName(QLatin1String("col1"));
        index.index(col1);

        Akonadi::Collection col2(2);
        col2.setName(QLatin1String("col2"));
        col2.setParentCollection(col1);
        index.index(col2);

        {
            Baloo::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix+QLatin1String("/collections/"));
            query.pathMatches(col1.name());
            Baloo::PIM::ResultIterator it = query.exec();
            QList<qint64> results;
            while (it.next()) {
                // kDebug() << "result " << it.id();
                results << it.id();
            }
            qSort(results);
            QCOMPARE(results.size(), 2);
            QCOMPARE(results.at(0), col1.id());
            QCOMPARE(results.at(1), col2.id());
        }
        {
            Baloo::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix+QLatin1String("/collections/"));
            query.pathMatches(QLatin1String("/col1/col2"));
            Baloo::PIM::ResultIterator it = query.exec();
            QList<qint64> results;
            while (it.next()) {
                // kDebug() << "result " << it.id();
                results << it.id();
            }
            qSort(results);
            QCOMPARE(results.size(), 1);
            QCOMPARE(results.at(0), col2.id());
        }

    }

    QList<qint64> getResults(Baloo::PIM::ResultIterator it)
    {
        QList<qint64> results;
        while (it.next()) {
            // kDebug() << "result " << it.id();
            results << it.id();
        }
        return results;
    }

    void collectionChanged() {
        Akonadi::Collection col1(1);
        col1.setName(QLatin1String("col1"));
        index.index(col1);

        Akonadi::Collection col2(2);
        col2.setName(QLatin1String("col2"));
        col2.setParentCollection(col1);
        index.index(col2);

        Akonadi::Collection col3(3);
        col3.setName(QLatin1String("col3"));
        col3.setParentCollection(col2);
        index.index(col3);

        col1.setName(QLatin1String("colX"));
        index.change(col1);
        col2.setParent(col1);
        index.change(col2);
        col3.setParent(col2);
        index.change(col3);

        //Old name gone
        {
            Baloo::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix+QLatin1String("/collections/"));
            query.nameMatches(QLatin1String("col1"));
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 0);
        }
        //New name
        {
            Baloo::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix+QLatin1String("/collections/"));
            query.nameMatches(QLatin1String("colX"));
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 1);
            QCOMPARE(results.at(0), col1.id());
        }
        //Old path gone
        {
            Baloo::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix+QLatin1String("/collections/"));
            query.pathMatches(QLatin1String("/col1/col2"));
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 0);
        }
        //New paths
        {
            Baloo::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix+QLatin1String("/collections/"));
            query.pathMatches(QLatin1String("/colX/col2"));
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 2);
            QCOMPARE(results.at(0), col2.id());
            QCOMPARE(results.at(1), col3.id());
        }
        {
            Baloo::PIM::CollectionQuery query;
            query.setDatabaseDir(dbPrefix+QLatin1String("/collections/"));
            query.pathMatches(QLatin1String("/colX/col2/col3"));
            QList<qint64> results = getResults(query.exec());
            QCOMPARE(results.size(), 1);
            QCOMPARE(results.at(0), col3.id());
        }
    }

};

QTEST_MAIN(CollectionQueryTest)

#include "collectionquerytest.moc"

