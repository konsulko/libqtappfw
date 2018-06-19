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

#ifndef PBAP_H
#define PBAP_H

#include <QDebug>
#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>

#include "messageengine.h"

class PhoneCall : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString number READ number NOTIFY numberChanged)
    Q_PROPERTY(QString datetime READ datetime NOTIFY datetimeChanged)
    Q_PROPERTY(QString type READ type NOTIFY typeChanged)

    public:
        explicit PhoneCall(QString name, QString number, QString datetime, QString type);
        virtual ~PhoneCall();

        QString name() {return m_name;};
        QString number() {return m_number;};
        QString datetime() {return m_datetime;};
        QString type() {return m_type;};

    signals:
        void nameChanged();
        void numberChanged();
        void datetimeChanged();
        void typeChanged();

    private:
        QString m_name;
        QString m_number;
        QString m_datetime;
        QString m_type;
};

class Pbap : public QObject
{
    Q_OBJECT

    public:
        explicit Pbap(QUrl &url, QQmlContext *context, QObject * parent = Q_NULLPTR);
        virtual ~Pbap();

        Q_INVOKABLE void refreshContacts(int max_entries);
        Q_INVOKABLE void refreshHistory(int max_entries);
        Q_INVOKABLE void search(QString number);

    signals:
	    void searchResults(QString name);

    private:
        MessageEngine *m_mloop;
        QQmlContext *m_context;
        QMap<QString, QObject *>m_contacts;
        QList<QObject *>m_history;
        void updateContacts(QString);
        void updateHistory(QString);
        void sendSearchResults(QJsonArray);

        // slots
        void onMessageReceived(MessageType, Message*);
};

#endif // PBAP_H
