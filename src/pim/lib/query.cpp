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

#include "query.h"
#include "contactquery.h"

#include <QVariant>
#include <KDebug>

#include <qjson/parser.h>

using namespace Baloo::PIM;

Query::Query()
{

}

Query::~Query()
{

}

Query* Query::fromJSON(const QByteArray& json)
{
    QJson::Parser parser;
    bool ok = false;

    QVariantMap result = parser.parse(json, &ok).toMap();
    if (!ok) {
        kError() << "Could not parse json query";
        return 0;
    }

    const QString type = result["type"].toString().toLower();
    if (type != "contact") {
        kError() << "Can only handle contact queries";
        return 0;
    }

    ContactQuery* cq = new ContactQuery();
    cq->matchName(result["name"].toString());
    cq->matchNickname(result["nick"].toString());
    cq->matchEmail(result["email"].toString());
    cq->matchUID(result["uid"].toString());
    cq->match(result["$"].toString());

    const QString criteria = result["matchCriteria"].toString().toLower();
    if (criteria == "exact")
        cq->setMatchCriteria(ContactQuery::ExactMatch);
    else if (criteria == "startswith")
        cq->setMatchCriteria(ContactQuery::StartsWithMatch);

    cq->setLimit(result["limit"].toInt());

    return cq;
}
