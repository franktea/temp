#pragma once

#include<string>

class EncDec {
public:
    EncDec() = default;
    virtual ~EncDec() = default;

    virtual std::string Encrypt(const std::string& str) { return str; }
    virtual std::string Decrypt(const std::string& str) { return str; }
};
