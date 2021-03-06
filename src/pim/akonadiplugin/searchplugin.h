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

#ifndef BALOO_PIM_SEARCHPLUGIN_H
#define BALOO_PIM_SEARCHPLUGIN_H

#include <akonadi/abstractsearchplugin.h>

namespace Baloo {
class Query;
}

class SearchPlugin : public QObject, public Akonadi::AbstractSearchPlugin
{
    Q_OBJECT
    Q_INTERFACES(Akonadi::AbstractSearchPlugin)
public:
    virtual QSet<qint64> search( const QString &query, const QList<qint64> &collections, const QStringList &mimeTypes );
private:
    Baloo::Query fromAkonadiQuery(const QString &akonadiQuery, const QList<qint64> &collections, const QStringList &mimeTypes);
};

#endif