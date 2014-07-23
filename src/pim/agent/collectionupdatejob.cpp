/*
 * Copyright 2014 Christian Mollekopf <mollekopf@kolabsys.com>
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
#include "collectionupdatejob.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>

CollectionUpdateJob::CollectionUpdateJob(Index &index, const Akonadi::Collection &col, QObject* parent)
    : KJob(parent),
    mCol(col),
    mIndex(index)
{

}

void CollectionUpdateJob::start()
{
    Akonadi::Collection::List ancestors;
    Akonadi::Collection ancestor = mCol.parentCollection();
    while (ancestor.isValid() && (ancestor != Akonadi::Collection::root())) {
        ancestors << ancestor;
        ancestor = ancestor.parentCollection();
    }
    if (!ancestors.isEmpty()) {
        // Fetch ancestors to get the display attribute
        Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(ancestors, Akonadi::CollectionFetchJob::Base, this);
        fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
        connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(onAncestorsFetched(KJob*)));
    } else {
        mAncestorsFetched = true;
    }

    { //Fetch children to update the path accordingly
        Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(mCol, Akonadi::CollectionFetchJob::Recursive, this);
        fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
        fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
        connect(fetchJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), this, SLOT(onCollectionsReceived(Akonadi::Collection::List)));
        connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(onCollectionsFetched(KJob*)));
    }
}

void CollectionUpdateJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    mChildCollections << list;
}

void CollectionUpdateJob::onCollectionsFetched(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
    }
    mChildrenFetched = true;
    finalize();
}

static Akonadi::Collection replaceParent(const Akonadi::Collection &col, const QHash<Akonadi::Collection::Id, Akonadi::Collection> &ancestors)
{
    if (!col.isValid()) {
        return col;
    }
    const Akonadi::Collection parent = replaceParent(col.parentCollection(), ancestors);
    if (!parent.isValid() || parent == Akonadi::Collection::root()) {
        return col;
    }
    Akonadi::Collection collection = col;
    //The first collection is usually not in ancestors
    if(ancestors.contains(col.id())) {
        collection = ancestors.value(col.id());
    }
    collection.setParentCollection(parent);
    return collection;
}

void CollectionUpdateJob::onAncestorsFetched(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
    }
    Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob*>(job);
    mAncestorsFetched = true;
    Akonadi::Collection::List matchingCollections;
    Q_FOREACH (const Akonadi::Collection &c, fetchJob->collections()) {
        mAncestors.insert(c.id(), c);
    }
    finalize();
}

void CollectionUpdateJob::finalize()
{
    if (!mChildrenFetched || !mAncestorsFetched) {
        return;
    }
    const Akonadi::Collection c = replaceParent(mCol, mAncestors);
    mAncestors.insert(c.id(), c);
    mIndex.change(c);

    //Required to update the path
    while (!mChildCollections.isEmpty()) {
        Akonadi::Collection child = mChildCollections.takeFirst();
        if (mAncestors.contains(child.parentCollection().id())) {
            const Akonadi::Collection c = replaceParent(child, mAncestors);
            mAncestors.insert(c.id(), c);
            mIndex.change(c);
        } else {
            mChildCollections.append(child);
        }
    }

    emitResult();
}

