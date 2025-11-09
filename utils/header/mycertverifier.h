#ifndef MYCERTVERIFIER_H
#define MYCERTVERIFIER_H

#include <vmime/vmime.hpp>
#include <memory>
#include <fstream>
#include <sstream>

class myCertVerifier : public vmime::security::cert::certificateVerifier
{
public:
    explicit myCertVerifier(const std::string& certPath);
    void verify(const std::shared_ptr<vmime::security::cert::certificateChain>& chain,
                const vmime::string& hostname) override;

private:
    std::string m_certPath;
};

#endif // MYCERTVERIFIER_H
