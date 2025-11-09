#include "utils/header/mycertverifier.h"
#include <vmime/utility/inputStreamStringAdapter.hpp>

myCertVerifier::myCertVerifier(const std::string& certPath)
    : m_certPath(certPath)
{
}

void myCertVerifier::verify(const std::shared_ptr<vmime::security::cert::certificateChain>& chain,
                            const vmime::string& hostname)
{
    std::ifstream certFile(m_certPath);
    if (!certFile) {
        throw vmime::exception("Не удалось открыть сертификат: " + m_certPath);
    }

    std::stringstream buffer;
    buffer << certFile.rdbuf();
    certFile.close();

    std::string certStr = buffer.str();

    // Используем inputStreamStringAdapter
    vmime::utility::inputStreamStringAdapter stream(certStr);
    std::vector<std::shared_ptr<vmime::security::cert::X509Certificate>> certs;
    vmime::security::cert::X509Certificate::import(stream, certs);

    if (certs.empty()) {
        throw vmime::exception("Не удалось импортировать сертификат из файла.");
    }

    // Пока принимаем сертификат как доверенный
    return;
}
