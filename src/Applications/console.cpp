#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "console.h"

#include "../modulInfo.h"
#include "Arduino.h"

CConsole g_Console;

const char* CConsole::pPromtStr = "\r\n" DEVICE_NAME " >>>";

const char *CConsole::pLongInput = "r\nInput very long";

CConsole::CConsole() : m_bMakeRepeatCalls(false),m_bMakeClearDisplay(false),m_bMakeUpdateDisplay(false),m_cmdEmptyInd(0),m_curCmdInd(-1),m_curParamInd(0),
					   m_repeatCallCnt(0)
{
    m_pConsoleTask = &CConsole::processStream;
    m_bInit = false;
}


bool CConsole::OnInitProcess(void *param) {
    m_Lock.create();
    m_pStream = &Serial; //!!!Arduino call here arduino instance

    m_bInit = true;
    return true;
}

int CConsole::printData(const char* format, va_list &arglist) {
    int size = vsnprintf(TxBuffer, CONSOLE_BUFFER_SIZE-1, format, arglist);
    m_pStream->write(TxBuffer,size);  //!!!Arduino call here we need 
    return size;
}

bool CConsole::OnTimer() {
    my_printf(pPromtStr);

    addConsCmd("?", this);
    addConsCmd("vers", this);

    while(1) {
        taskDelayMs(1);
        (this->*m_pConsoleTask)();
    }
}

void CConsole::processStream() {
    uint8_t cntRxSmbls = m_pStream->available();

    if (cntRxSmbls != 0) {
        m_bMakeRepeatCalls = m_bMakeClearDisplay = false;
        if(m_bMakeUpdateDisplay)
        {
            m_bMakeUpdateDisplay = false;
            my_printf("\x1b[?25h"); // enable cursor
        }

		if((m_curRcvIndex + cntRxSmbls) > (sizeof(RxBuffer)/sizeof(*RxBuffer) - 1))
		{
			m_curRcvIndex = 0;
			my_printf(pLongInput);
			my_printf(pPromtStr);
			m_pStream->flush();
			return;
			//continue;
		}
		cntRxSmbls = m_pStream->readBytes(&RxBuffer[m_curRcvIndex],cntRxSmbls);
		
		bool sendEcho = true;
        
		bool findReturnCarriage = false;
		int i = m_curRcvIndex + cntRxSmbls - 1;
		for(;i >= m_curRcvIndex; i--)
		{		
			if(RxBuffer[i] == '\r')
				findReturnCarriage = true;
				break;
		}
		
	    bool findEscape = FindEscapeSequence(cntRxSmbls);
		if(findEscape)
			sendEcho = false;
			
		if(sendEcho)
			m_pStream->write(&RxBuffer[m_curRcvIndex],cntRxSmbls);      // echo
		
        if(findReturnCarriage)   //find 'r' 
        {    
            RxBuffer[i] = 0;
            m_curRcvIndex = 0;
            parseRxCmd();
            if(!m_bMakeRepeatCalls)
                my_printf(pPromtStr);
        }
        else
		{
			if(!findEscape)
				m_curRcvIndex+=cntRxSmbls;
		}
       
	} else {
		callCmdMngr();

    }
}

void CConsole::parseRxCmd() {
    
    memset(m_pCmdParams,0,sizeof(m_pCmdParams)/sizeof(m_pCmdParams[0]));
	char *pStrCmd = strtok(RxBuffer," ");
	// find command
	if(pStrCmd != NULL)
	{
		if(findConsCmd(pStrCmd) != -1)
		{
			m_curParamInd = 0;
			//~~~ get command parameters
			while(pStrCmd!=NULL)
			{
				if(m_curParamInd == sizeof(m_pCmdParams)/sizeof(m_pCmdParams[0]))
				{	
					my_printf("Unsupported count parameters\r\n");
					//~~~ not accept command
					break;
				}
				m_pCmdParams[m_curParamInd++] = pStrCmd;//
				pStrCmd = strtok(NULL," ");
				
				//check for --c param
				
				if(!strcmp(pStrCmd,"--c"))
				{    
					m_bMakeRepeatCalls = true;
					break;
				}
				else if(!strcmp(pStrCmd,"--v"))
				{
					m_bMakeRepeatCalls = true;
					m_bMakeClearDisplay = true;
					break;
				}
                else if(!strcmp(pStrCmd,"--u"))
                {
                    m_bMakeRepeatCalls = true;
                    m_bMakeUpdateDisplay = true;
                    my_printf("%c",0xC); //clear display
                    my_printf("\x1b[?25l"); // disable cursor
                }
			}
			my_printf("\r\n");
			callCmd();	
		}
	}
}

int CConsole::findConsCmd(const char *strName) {
    if(m_cmdEmptyInd != 0)
	{
		int i = 0;
        for(; i < m_cmdEmptyInd; i++)
		{
			if(!strcmp(strName,m_ConsoleCmdBuf[i].cmdName))
			{	
                m_curCmdInd = i;
                break;
            }
		}
        if(i == m_cmdEmptyInd)
        {
            my_printf("\r\n@%s@",strName);
        }
	}
	else 
	{	
        m_curCmdInd = -1;
    }
	return m_curCmdInd;
}

void CConsole::callCmd() {
    if(m_ConsoleCmdBuf[m_curCmdInd].pConsCmd != NULL)
	{	
		if(m_bMakeClearDisplay)
			clear_screen();
        else if(m_bMakeUpdateDisplay)
            my_printf("\x1b[1;1H");	// move cursor 1,1
		m_ConsoleCmdBuf[m_curCmdInd].pConsCmd->Command(m_curParamInd, m_pCmdParams);
	}
}

void CConsole::callCmdMngr() {
    if (m_bMakeRepeatCalls) {
        if (m_repeatCallCnt++ == 1000) {
            callCmd();
            m_repeatCallCnt = 0;
        }
    }
}

bool CConsole::addConsCmd(const char* strName, ConsoleCmd *pCmdExec) {
    if(m_cmdEmptyInd == sizeof(m_ConsoleCmdBuf)/sizeof(m_ConsoleCmdBuf[0]))
	{	
		my_printf("Can't add cmd %s cmd buffer is full\r\n",strName);
		return false;
	}
	//find if command already exsists
	if(m_cmdEmptyInd != 0)
	{
		for(int i = 0;i < m_cmdEmptyInd; i++)
		{
			if(!strcmp(m_ConsoleCmdBuf[i].cmdName,strName))
			{
				my_printf("Command %s already exists\r\n");
				return false;
			}
		}
	}
    
    char *pName = new char[strlen(strName) + 1];
    strcpy(pName,strName);
	m_ConsoleCmdBuf[m_cmdEmptyInd].cmdName = pName;
    m_ConsoleCmdBuf[m_cmdEmptyInd].pConsCmd = pCmdExec;
	m_cmdEmptyInd++;
	return true;
}

void CConsole::Command(int argc,char* argv[])
{
    if(!strcmp(argv[0],"vers"))	{
		my_printf("System CPU: esp32 Wrover\r\n");
		my_printf("Project: %s\r\n", DEVICE_NAME );
    ///    my_printf("Version: %s\r\n", GetVersionString());
    ///    my_printf("Hardware vers: %s\r\n", GetHardwareVersion());
    ///    my_printf("Vers String: %s\r\n", GetFullVersionString());
	}
    if(!strcmp(argv[0],"?")) {
        for(int i = 0; i < m_cmdEmptyInd;i++)
        {
            my_printf("%s\r\n",m_ConsoleCmdBuf[i].cmdName);
        }
    }
}

bool add_console_command(const char* strName,ConsoleCmd *pCmdExec)
{
	return g_Console.addConsCmd(strName,pCmdExec);
}


void my_printf(const char* format, ...)
{
    if(g_Console.isConsoleInit())
    {
        CScopedCritSec critSec(g_Console.m_Lock);
        va_list paramList;
        va_start(paramList, format);
        int iResult = g_Console.printData(format, paramList);
        va_end(paramList);   
    }
}

void clear_screen()
{
	if(g_Console.isConsoleInit())
    {
		CScopedCritSec critSec(g_Console.m_Lock);
		g_Console.m_pStream->write(0xC);
	}
}
