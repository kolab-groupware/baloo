/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
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

#ifndef _BALOO_FILE_H
#define _BALOO_FILE_H

#include "filefetchjob.h"
#include <kfilemetadata/properties.h>

namespace Baloo {

class FilePrivate;

/**
 * @short Provides acess to all File Metadata
 *
 * The File class acts as a temporary container for all the file metadata.
 * It needs to be filled via the FileFetchJob, and any modifications made
 * are not saved until the FileModifyJob is called.
 */
class BALOO_FILE_EXPORT File
{
public:
    File();
    File(const File& f);

    /**
     * Constructor
     *
     * \p url the local url of the file
     */
    File(const QString& url);
    ~File();

    const File& operator =(const File& f);

    static File fromId(const QByteArray& id);

    /**
     * The local url of the file
     */
    QString url() const;
    void setUrl(const QString& url);

    /**
     * Represents a unique identifier for this file. This identifier
     * is unique and will never change unlike the url of the file
     */
    QByteArray id() const;
    void setId(const QByteArray& id);

    /**
     * Gives a variant map of the properties that have been extracted
     * from the file by the indexer
     */
    KFileMetaData::PropertyMap properties() const;
    QVariant property(KFileMetaData::Property::Property property) const;

    /**
     * Set the rating for the file. This will not be saved until
     * a FileModifyJob is called
     */
    void setRating(int rating);
    int rating() const;

    /**
     * Add a tag to the list of tags. This will not be saved until
     * a FileModifyJob is called.
     */
    void addTag(const QString& tag);
    void setTags(const QStringList& tags);
    QStringList tags() const;

    QString userComment() const;
    void setUserComment(const QString& comment);

private:
    FilePrivate* d;
    friend class FileFetchJob;
};

}

#endif // _BALOO_FILE_H
