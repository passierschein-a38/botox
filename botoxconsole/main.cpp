/*
 Botox - code injection tool

 Axel Sauerhoefer
 Jockgrimerstr. 13
 76764 Rheinzabern

 axel[at]willcodeqtforfood.de
 http://www.willcodeqtforfood.de

 This file is part of botox.

 Botox is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Botox is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Botox.If not, see <http://www.gnu.org/licenses/>.


	29.11.2009 18:35
*/

#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include <windows.h>
#include <psapi.h>
#include <tchar.h>
#include <iostream>
#include <string>

#include "botox.h"

namespace{

void PrintProcess( const DWORD pid )
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, pid );

    // Get the process name.

    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName, 
                               sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }

	if( 0 == wcscmp( szProcessName, TEXT("<unknown>") ) )
	{
		return;
	}	

    // Print the process name and identifier.

    _tprintf( TEXT("%s  (PID: %u)\n"), szProcessName, pid );

	std::wcout << szProcessName << std::endl;

    CloseHandle( hProcess );
}

void PrintProcess()
{
   DWORD lst[1024];
   DWORD bytesNeeded = 0;  

   if( FALSE == EnumProcesses( lst, sizeof( lst ), &bytesNeeded ) )
   {
	   std::cout << "Unable to EnumProcesses" << std::endl;
       exit( -1 );
   }
   
   const DWORD count = bytesNeeded / sizeof( DWORD );

   for( unsigned int i=0; i<count; ++i )
   {
		PrintProcess( lst[i] ); 
   }
}

}

using namespace Botox;

int main( int argc, char** argv )
{
	PrintProcess();

	DWORD pid = 0;

	std::cout << "pid: ";
	std::cin >> pid;

	BotoxPtr pBotox = CreateBotox( pid );
	const bool bSuccess = pBotox->Inject();

	if( false == bSuccess ) 
	{
		std::wcout << L"FAILED: " << pBotox->GetLastError();
		exit( -100 );
	}

	return static_cast<int>( bSuccess );
}