/*
 * Copyright (C) 2018 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vcard/vcard.h>

#include "message.h"
#include "messageengine.h"
#include "pbap.h"
#include "pbapmessage.h"
#include "responsemessage.h"

PhoneCall::PhoneCall(QString name, QString number, QString datetime, QString type)
{
    m_name = name;
    m_number = number;
    m_datetime = datetime;
    m_type = type;
}

PhoneCall::~PhoneCall()
{
}

Pbap::Pbap (QUrl &url, QQmlContext *context, QObject * parent) :
    QObject(parent),
    m_mloop(nullptr)
{
    m_mloop = new MessageEngine(url);
    m_context = context;
    m_context->setContextProperty("CallHistoryModel", QVariant::fromValue(m_history));
    QObject::connect(m_mloop, &MessageEngine::messageReceived, this, &Pbap::onMessageReceived);
#if 1
    this->refreshContacts(50);
#endif
}

Pbap::~Pbap()
{
    delete m_mloop;
}

void Pbap::refreshContacts(int max_entries)
{
    PbapMessage *tmsg = new PbapMessage();
    QJsonObject parameter;

    if (max_entries >= 0)
        parameter.insert("max_entries", max_entries);

    tmsg->createRequest("contacts", parameter);
    qInfo() << "sending contacts verbs with entries = " << max_entries;
    m_mloop->sendMessage(tmsg);
    tmsg->deleteLater();
}

void Pbap::refreshHistory(int max_entries)
{
    PbapMessage *tmsg = new PbapMessage();
    QJsonObject parameter;

    parameter.insert("list", "cch");
    if (max_entries >= 0)
        parameter.insert("max_entries", max_entries);

    tmsg->createRequest("history", parameter);
    m_mloop->sendMessage(tmsg);
    tmsg->deleteLater();
}

void Pbap::search(QString number)
{
    PbapMessage *tmsg = new PbapMessage();
    QJsonObject parameter;

    if (!number.isEmpty())
        parameter.insert("number", number);
    parameter.insert("max_entries", 1);

    tmsg->createRequest("search", parameter);
    m_mloop->sendMessage(tmsg);
    tmsg->deleteLater();
}

void Pbap::updateContacts(QString vcards)
{
    QString name, number, type;

    qInfo() << "contacts vcards: " << vcards;
    QList<vCard> contacts_vcards = vCard::fromByteArray(vcards.toUtf8());

    for (auto vcard : contacts_vcards) {
        vCardProperty name_prop = vcard.property(VC_FORMATTED_NAME);
        qInfo() << "Property name: " << name_prop.name();
        qInfo() << "Property values: " << name_prop.values();
//        qInfo() << "Property params: " << name_prop.params();
        QStringList values = name_prop.values();
        name = values.at(vCardProperty::DefaultValue);
        qInfo() << "Name: " << name;
        vCardProperty number_prop = vcard.property(VC_TELEPHONE);
        if (number_prop.isValid()) {
            QStringList values = number_prop.values();
            number = values.at(0);
            qInfo() << "Telephone: " << number;
            vCardParamList params = number_prop.params();
            type = params.at(0).value();
            qInfo() << "Type: " << type;
        }
    }
}

#define VC_DATETIME "X-IRMC-CALL-DATETIME"

void Pbap::updateHistory(QString vcards)
{
    QString name, number, datetime, type;
//    QList<QObject *> m_history;

    qInfo() << "history vcards: " << vcards;
    QList<vCard> history_vcards = vCard::fromByteArray(vcards.toUtf8());


    for (auto vcard : history_vcards) {
        vCardProperty number_prop = vcard.property(VC_TELEPHONE);
        if (number_prop.isValid()) {
            QStringList values = number_prop.values();
            name = number = values.at(0);
            qInfo() << "Telephone: " << name;
        }
        vCardProperty datetime_prop = vcard.property(VC_DATETIME);
        if (datetime_prop.isValid()) {
            vCardParamList params = datetime_prop.params();
            QStringList values = datetime_prop.values();
            type = params.at(0).value();
            datetime = values.at(0);
            qInfo() << "Type: " << type;
            qInfo() << "Date/Time: " << datetime;
        }
        m_history.append(new PhoneCall(name, number, datetime, type));
    }

    m_context->setContextProperty("CallHistoryModel", QVariant::fromValue(m_history));
}

void Pbap::sendSearchResults(QJsonArray results)
{
    QString name;

    if (results.empty())
        name = "Not Found";
    else
        name = results.at(0).toObject().value("name").toString();

    emit searchResults(name);
}

void Pbap::onMessageReceived(MessageType type, Message *msg)
{
    if (msg->isReply() && type == ResponseRequestMessage) {
        ResponseMessage *tmsg = qobject_cast<ResponseMessage*>(msg);

        if (tmsg->requestVerb() == "contacts") {
            updateContacts(tmsg->replyData().value("vcards").toString());
        } else if (tmsg->requestVerb() == "history") {
            updateHistory(tmsg->replyData().value("vcards").toString());
        } else if (tmsg->requestVerb() == "search") {
            sendSearchResults(tmsg->replyData().value("results").toArray());
        }
    }

    msg->deleteLater();
}
