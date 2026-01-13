#include "DcsBiosReader.hpp"
#include <iostream>

DcsBiosReader::DcsBiosReader(const std::string& comPort)
    : comPort_(comPort),
    hSerial_(INVALID_HANDLE_VALUE)
{
}

DcsBiosReader::~DcsBiosReader()
{
    if (hSerial_ != INVALID_HANDLE_VALUE)
        CloseHandle(hSerial_);
}

bool DcsBiosReader::openPort()
{
    std::string fullPort = "\\\\.\\" + comPort_;

    hSerial_ = CreateFileA(
        fullPort.c_str(),
        GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (hSerial_ == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to open " << comPort_ << "\n";
        return false;
    }

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(hSerial_, &dcb))
        return false;

    dcb.BaudRate = 250000;   // typical for text hardware protocol
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;

    if (!SetCommState(hSerial_, &dcb))
        return false;

    COMMTIMEOUTS timeouts{};
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutConstant = 1;
    timeouts.ReadTotalTimeoutMultiplier = 1;
    SetCommTimeouts(hSerial_, &timeouts);

    return true;
}

void DcsBiosReader::readLoop()
{
    char buffer[256];

    
        DWORD bytesRead = 0;

        if (ReadFile(hSerial_, buffer, sizeof(buffer), &bytesRead, nullptr) &&
            bytesRead > 0)
        {
            processIncoming(buffer, bytesRead);
        }

    
}

void DcsBiosReader::processIncoming(const char* data, size_t length)
{
    rxBuffer_.append(data, length);

    size_t pos;
    while ((pos = rxBuffer_.find('\n')) != std::string::npos)
    {
        std::string line = rxBuffer_.substr(0, pos);
        rxBuffer_.erase(0, pos + 1);

        // Trim optional CR
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

       // std::cout << line;

        if (!line.empty())
            processLine(line);
    }
}

void DcsBiosReader::processLine(const std::string& line)
{
    auto spacePos = line.find(' ');
    if (spacePos == std::string::npos)
    {
        std::cerr << "[Malformed] " << line << "\n";
        return;
    }

    std::string controlIdentifier = line.substr(0, spacePos);
    std::string argument = line.substr(spacePos + 1);

    if (controlIdentifier.empty() || argument.empty())
    {
        std::cerr << "[Invalid] " << line << "\n";
        return;
    }

    std::cout << "CONTROL: " << controlIdentifier
        << " | ARG: " << argument << "\n";
}
