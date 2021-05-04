#pragma once
#include <stdarg.h>
#include <stdint.h>
#include "MyLib/OSWrappers.h"

void my_printf(const char* format, ...);
void clear_screen();

const int CONSOLE_BUFFER_SIZE = 256;
const int NUM_CONSOLE_COMMANDS = 30;
const int NUM_CONSOLE_PARAMS = 10;

class ConsoleCmd {
public:
    virtual void Command(int argc, char* argv[]) = 0;
    virtual ~ConsoleCmd(){}
};

typedef struct {
    const char *cmdName;
    ConsoleCmd *pConsCmd;
}ConsCmd;

bool add_console_command(const char *strName, ConsoleCmd *pCmdExec);

class Stream;

/*
class CConsoleInterface {
protected:
    virtual int printData(const char* format, va_list &arglist) = 0; 
public:
    virtual bool isInit() = 0;
};*/

class CConsole;
typedef void (CConsole::*ConsoleTask)(void);

class CConsole : public ConsoleCmd {
    friend void my_printf(const char* format, ...);
    friend void clear_screen();
    friend bool add_console_command(const char *strName, ConsoleCmd *pCmdExec);
private:
    bool m_bInit;
    int m_curRcvIndex;
    bool m_bMakeRepeatCalls;
    bool m_bMakeClearDisplay;
    bool m_bMakeUpdateDisplay;
    int  m_cmdEmptyInd;
    int  m_curCmdInd;
    int m_curParamInd;
    int m_repeatCallCnt;

    Stream *m_pStream;

    char TxBuffer[CONSOLE_BUFFER_SIZE];
    char RxBuffer[CONSOLE_BUFFER_SIZE];
    char *m_pCmdParams[NUM_CONSOLE_PARAMS];
    ConsCmd m_ConsoleCmdBuf[NUM_CONSOLE_COMMANDS];
    
    static const char *pPromtStr;
    static const char *pLongInput;
public:
    CMutex m_Lock;
private:
    

    bool addConsCmd(const char *strName, ConsoleCmd *pCmdExec);
    int findConsCmd(const char *strName);
    bool executeConsCmd();
    virtual void Command(int argc, char* argv[]);
    void callCmdMngr();
    void callCmd();
    void parseRxCmd();
    bool findEscapeSequence(uint8_t numRxBytes);
    int  printData(const char* format, va_list &arglist);
protected:
public:
    CConsole();
    bool init();
    void processStream();
    void printPrompt();
    void clearScreen();
    void moveCursorAtStart();
    void enableCursor();
    void disableCursor();
    void setStreamInterface(Stream *pStream) {m_pStream = pStream;}
    bool isInit()  {return m_bInit;}
    
};



