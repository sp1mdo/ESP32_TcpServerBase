#ifndef _ECHO_H
#define _ECHO_H

#include <inttypes.h>
#include <cstdlib>
#include <memory>
#include <vector>

#include "BaseServer.hpp"

#define IAC 255 // Interpret as Command
#define SB 250
#define NAWS 31
#define SGA 3              // Suppress Go Ahead
#define SUBCOMMAND_END 248 // Suppress Go Ahead

namespace TelnetOption
{
    enum Option : uint8_t
    {
        BINARY_TRANSMISSION = 0,
        ECHO = 1,
        RECONNECTION = 2,                // Deprecated
        SUPPRESS_GO_AHEAD = 3,           // c
        APPROX_MSG_SIZE_NEGOTIATION = 4, // Deprecated
        STATUS = 5,
        TIMING_MARK = 6,
        REMOTE_CONTROLLED_TRANSMIT_ECHO = 7,
        OUTPUT_LINE_WIDTH = 8,                   // Deprecated
        OUTPUT_PAGE_SIZE = 9,                    // Deprecated
        OUTPUT_CARRIAGE_RETURN_DISPOSITION = 10, // Deprecated
        OUTPUT_HORIZONTAL_TAB_STOPS = 11,        // Deprecated
        OUTPUT_HORIZONTAL_TAB_DISPOSITION = 12,  // Deprecated
        OUTPUT_FORMFEED_DISPOSITION = 13,        // Deprecated
        OUTPUT_VERTICAL_TABSTOPS = 14,           // Deprecated
        OUTPUT_VERTICAL_TAB_DISPOSITION = 15,    // Deprecated
        OUTPUT_LINEFEED_DISPOSITION = 16,        // Deprecated
        EXTENDED_ASCII = 17,                     // Deprecated
        LOGOUT = 18,
        BYTE_MACRO = 19, // Deprecated
        DATA_ENTRY_TERMINAL = 20,
        SUPDUP = 21,
        SUPDUP_OUTPUT = 22,
        SEND_LOCATION = 23,
        TERMINAL_TYPE = 24, // c
        END_OF_RECORD = 25,
        TACACS_USER_IDENTIFICATION = 26,
        OUTPUT_MARKING = 27,
        TERMINAL_LOCATION_NUMBER = 28,
        REGIME_3270 = 29,
        X3_PAD = 30,
        NEGOTIATE_ABOUT_WINDOW_SIZE = 31, // c
        TERMINAL_SPEED = 32,
        REMOTE_FLOW_CONTROL = 33,
        LINE_MODE = 34,
        X_DISPLAY_LOCATION = 35,
        ENVIRONMENT_OPTION = 36,
        AUTHENTICATION_OPTION = 37,
        ENCRYPTION_OPTION = 38, // c s
        NEW_ENVIRONMENT_OPTION = 39,
    };
}

namespace TelnetCommand
{
    enum Command : uint8_t
    {
        DO = 253,
        DONT = 254,
        WILL = 251,
        WONT = 252,
        SUBOPTION = 250,
    };
}

struct NegotiationOption
{
    NegotiationOption(uint8_t opt, uint8_t cmd) : m_Option(opt), m_Command(cmd)
    {
    }

    NegotiationOption(uint8_t opt, uint8_t cmd, uint8_t value) : m_Option(opt), m_Command(250), m_value(value)
    {
    }

    uint8_t m_Option;
    uint8_t m_Command;
    uint8_t m_value;
};

class TelnetServer : public BaseTcpServer
{

public:
    TelnetServer(uint16_t port) : BaseTcpServer(port)
    {
        m_Demans.push_back(NegotiationOption(TelnetOption::TERMINAL_TYPE, TelnetCommand::DO));
        m_Demans.push_back(NegotiationOption(TelnetOption::TERMINAL_SPEED, TelnetCommand::DO));
        m_Demans.push_back(NegotiationOption(TelnetOption::X_DISPLAY_LOCATION, TelnetCommand::DO));
        m_Demans.push_back(NegotiationOption(TelnetOption::NEW_ENVIRONMENT_OPTION, TelnetCommand::DO));

        m_Demans.push_back(NegotiationOption(TelnetOption::ENVIRONMENT_OPTION, TelnetCommand::DO));
        m_Demans.push_back(NegotiationOption(TelnetOption::SUPPRESS_GO_AHEAD, TelnetCommand::DO));
        m_Demans.push_back(NegotiationOption(TelnetOption::NEGOTIATE_ABOUT_WINDOW_SIZE, TelnetCommand::WILL));
        m_Demans.push_back(NegotiationOption(TelnetOption::REMOTE_FLOW_CONTROL, TelnetCommand::DO));
        m_Demans.push_back(NegotiationOption(TelnetOption::LINE_MODE, TelnetCommand::DONT));
        m_Demans.push_back(NegotiationOption(TelnetOption::STATUS, TelnetCommand::DO));
        m_Demans.push_back(NegotiationOption(TelnetOption::TERMINAL_SPEED, 0x01));
        m_Demans.push_back(NegotiationOption(TelnetOption::NEW_ENVIRONMENT_OPTION, 0x01));
        m_Demans.push_back(NegotiationOption(TelnetOption::TERMINAL_TYPE, 0x01));
        m_Demans.push_back(NegotiationOption(TelnetOption::ECHO, TelnetCommand::WILL));
    }

private:
    void processSubnegotiation(const uint8_t *data, size_t len);
    void sendDemands(const int sock_fd);
    void sendWelcomeMessage(const int sock_fd) override;
    void processRx(const int sock_fd, const uint8_t *data, size_t len) override;
    std::vector<NegotiationOption> m_Demans;
    std::vector<NegotiationOption> m_ActualOptions;
    uint16_t m_WindowWidth;
    uint16_t m_WindowHeight;
};

#endif