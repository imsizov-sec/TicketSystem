#include "utils/header/mailservice.h"
#include "utils/header/mycertverifier.h"

#include <vmime/vmime.hpp>
#include <vmime/platforms/windows/windowsHandler.hpp>
#include <vmime/utility/outputStreamStringAdapter.hpp>
#include <vmime/text.hpp>
#include <vmime/charsetConverter.hpp>
#include <vmime/messageParser.hpp>
#include <vmime/utility/outputStreamStringAdapter.hpp>

#include <memory>
#include <QDebug>

MailService::MailService(const QString& certPath_)
    : certPath(certPath_) {}

QList<MailMessage> MailService::fetchAllMail() {
    QList<MailMessage> result;

    try {
        vmime::platform::setHandler<vmime::platforms::windows::windowsHandler>();

        auto session = vmime::net::session::create();
        vmime::utility::url url("imaps://imap.mail.ru:993");

        const std::string username = "ticket_system@mail.ru";
        const std::string password = "uKRATnFXaiGAfo81kbcV";

        auto store = session->getStore(url);
        store->setProperty("auth.username", username);
        store->setProperty("auth.password", password);
        store->setProperty("connection.tls", true);
        store->setProperty("connection.tls.required", true);
        store->setProperty("connection.tls.verification", true);

        auto verifier = std::make_shared<myCertVerifier>(certPath.toStdString());
        store->setCertificateVerifier(verifier);
        store->connect();

        auto folder = store->getFolder(vmime::net::folder::path("INBOX"));
        folder->open(vmime::net::folder::MODE_READ_ONLY);

        const int count = folder->getMessageCount();
        if (count == 0)
            return result;

        const int messagesToFetch = std::min(count, 20);
        auto mset = vmime::net::messageSet::byNumber(1, messagesToFetch);
        auto messages = folder->getMessages(mset);

        vmime::net::fetchAttributes attrs;
        attrs.add(vmime::net::fetchAttributes::ENVELOPE);
        attrs.add(vmime::net::fetchAttributes::UID);
        folder->fetchMessages(messages, attrs);

        for (size_t i = 0; i < messages.size(); ++i) {
            MailMessage msg;
            auto m = messages[i];

            // UID
            msg.uid = QString::fromStdString(static_cast<std::string>(m->getUID()));

            // Тема
            auto hdr = m->getHeader();
            if (auto subj = hdr->findField("Subject")) {
                auto txt = std::dynamic_pointer_cast<const vmime::text>(subj->getValue());
                msg.subject = txt ? QString::fromStdString(txt->getConvertedText(vmime::charsets::UTF_8))
                                  : "(не удалось декодировать)";
            } else {
                msg.subject = "(отсутствует)";
            }

            // Отправитель
            if (hdr->hasField("From")) {
                auto fromField = hdr->From();
                auto mailboxPtr = std::dynamic_pointer_cast<const vmime::mailbox>(fromField->getValue());
                if (mailboxPtr)
                    msg.sender = QString::fromStdString(mailboxPtr->getEmail().toString());
            }

            // Тело письма
            try {
                std::string rawMessage;
                vmime::utility::outputStreamStringAdapter os(rawMessage);
                m->extract(os);

                auto parsedMsg = vmime::make_shared<vmime::message>();
                parsedMsg->parse(rawMessage);

                vmime::messageParser parser(parsedMsg);

                for (size_t j = 0; j < parser.getTextPartCount(); ++j) {
                    auto part = parser.getTextPartAt(j);
                    if (part->getType().getSubType() == vmime::mediaTypes::TEXT_PLAIN) {
                        std::string textContent;
                        vmime::utility::outputStreamStringAdapter textStream(textContent);
                        part->getText()->extract(textStream);
                        msg.body = QString::fromStdString(textContent);
                        break;
                    }
                }

                if (msg.body.isEmpty() && parser.getTextPartCount() > 0) {
                    auto part = parser.getTextPartAt(0);
                    std::string fallbackText;
                    vmime::utility::outputStreamStringAdapter fallbackStream(fallbackText);
                    part->getText()->extract(fallbackStream);
                    msg.body = QString::fromStdString(fallbackText);
                }
            } catch (const vmime::exception& e) {
                qDebug() << "Ошибка при извлечении тела письма:" << e.what();
                msg.body = "(не удалось извлечь тело письма)";
            }

            result.append(msg);
        }

        folder->close(false);
        store->disconnect();
    } catch (const vmime::exception& e) {
        qDebug() << "VMime error:" << e.what();
    } catch (const std::exception& e) {
        qDebug() << "Exception:" << e.what();
    }

    return result;
}
