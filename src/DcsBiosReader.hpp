#pragma once

#include <windows.h>
#include <string>

class DcsBiosReader
{
public:
    explicit DcsBiosReader(const std::string& comPort);
    ~DcsBiosReader();

    bool openPort();
    void readLoop();

private:
    void processIncoming(const char* data, size_t length);
    void processLine(const std::string& line);

private:
    std::string comPort_;
    HANDLE hSerial_;
    std::string rxBuffer_;
};
