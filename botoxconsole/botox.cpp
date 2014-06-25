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

 Botox is distributed in the hope that it will be useful,y
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Botox.If not, see <http://www.gnu.org/licenses/>.


	29.11.2009 18:47
*/

#include <sstream>
#include <shlobj.h>

#include "botox.h"
#include "resource.h"

namespace{

struct AutoFreeHandle
{
	AutoFreeHandle( HANDLE hHandle )
		: m_h( hHandle )
	{
	}

	~AutoFreeHandle()
	{
		CloseHandle( m_h );
	}

private:

	HANDLE m_h;
};

} //end anonymous namespace

namespace Botox{

BotoxPtr CreateBotox( DWORD processId )
{

	BotoxPtr ptr;
	Botox* pObj = new Botox( processId );

	if( true == pObj->Init() )
	{	
		ptr.reset( pObj );
	}else{
		delete pObj;
		pObj = NULL;
	}

	return ptr;
}

Botox::~Botox()
{
}

bool Botox::Init()
{
	TCHAR szTmpPath[MAX_PATH];
	TCHAR szTmpFile[MAX_PATH];

	if( NULL == GetTempPath( MAX_PATH , szTmpPath ) )
	{
		SetLastError( L"GetTempPath failed" );
		return false;
	}

	if( NULL ==	GetTempFileName( szTmpPath, 
								 L".tmp",
								 0, 
								szTmpFile ) )
	{
		SetLastError( L"GetTempFileName failed" );
		return false;
	}


	HANDLE hFile = CreateFile( szTmpFile,
							   GENERIC_WRITE,
							   0,
							   NULL,
							   CREATE_ALWAYS,
							   FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_OFFLINE,
							   NULL );

	if( INVALID_HANDLE_VALUE == hFile )
	{
		SetLastError( L"GetTempFileName failed" );
		return false;
	}

	AutoFreeHandle af( hFile );
	
	std::wstringstream wstr;
	wstr << szTmpFile << L":" << GetTickCount();

	m_dllName = wstr.str();

	return true;
}

bool Botox::Extract() const
{
	const HMODULE hModule = GetModuleHandle( NULL );

	if( NULL == hModule )
	{
		return false;
	}

	HRSRC hResource	 = FindResource( hModule, MAKEINTRESOURCE( IDR_BOTOX_DLL ), L"BINARY" );

	if( NULL == hModule )
	{
		return false;
	}

	HGLOBAL hFileResource = LoadResource( hModule, hResource );

	if( NULL == hModule )
	{
		return false;
	}

	LPVOID lpFile = LockResource( hFileResource );

	if( NULL == lpFile )
	{
		return false;
	}

	DWORD dwSize = SizeofResource( hModule, hResource );

	if( 0 == dwSize )
	{
		return false;
	}
	
	HANDLE hFile = CreateFile( m_dllName.c_str(), 
							   GENERIC_READ | GENERIC_WRITE, 
							   0, 
							   NULL, 
							   CREATE_ALWAYS, 
							   FILE_ATTRIBUTE_NORMAL, 
							   NULL );           

	HANDLE hFilemap = CreateFileMapping( hFile, 
										 NULL, PAGE_READWRITE, 
										 0, 
										 dwSize, 
										 NULL );
	
	LPVOID lpBaseAddress = MapViewOfFile( hFilemap, FILE_MAP_WRITE, 0, 0, 0 );      

	CopyMemory(lpBaseAddress, lpFile, dwSize);             
	UnmapViewOfFile(lpBaseAddress);           
	CloseHandle(hFilemap);           
	CloseHandle(hFile);     
	
	return true;
	
}

const bool Botox::Inject() const
{

	if( false == EnableDebug() )
	{
		return false;
	}

	if( false == Extract() )
	{
		return false;
	}

    std::string dll( m_dllName.length() , ' ');
	std::copy( m_dllName.begin(), m_dllName.end(), dll.begin() );


   HANDLE hRemoteProcessHandle = OpenProcess( PROCESS_ALL_ACCESS, 
											  FALSE, 
											  m_dwProcessId );
   
   if( NULL == hRemoteProcessHandle )
   {
	   SetLastError( L"OpenProcess with PROCESS_ALL_ACCESS failed" );
	   return false;
   }

   AutoFreeHandle afh( hRemoteProcessHandle );
   
   LPVOID pLoadLibrary = reinterpret_cast<LPVOID>( GetProcAddress( GetModuleHandle( L"kernel32.dll" ), "LoadLibraryA" ) );

   if( NULL == pLoadLibrary )
   {
	   SetLastError( L"Failed to resolve function pointer in kernel32.dll::LoadLibraryA" );
	   return false;
   }

   LPVOID pRemoteMemory = reinterpret_cast<LPVOID>( VirtualAllocEx( hRemoteProcessHandle, 
																  NULL, 
																  dll.length(),
																  MEM_RESERVE|MEM_COMMIT, 
																  PAGE_EXECUTE_READWRITE ) );

   if( NULL == pRemoteMemory )
   {
	   SetLastError( L"VirtualAllocEx failed with MEM_RESERVE|MEM_COMMIT, PAGE_EXECUTE_READWRITE" );
	   return false;
   }
   
   if( FALSE == WriteProcessMemory( hRemoteProcessHandle, 
									reinterpret_cast<LPVOID>( pRemoteMemory ), 
									dll.c_str(),
									dll.length(), 
									NULL) )
   {
	   SetLastError( L"WriteProcessMemory failed" );
	   return false;
   }

   HANDLE hThreadHandle = CreateRemoteThread( hRemoteProcessHandle, 
											  NULL, 
											  NULL, 
											  reinterpret_cast<LPTHREAD_START_ROUTINE>(pLoadLibrary), 
											  reinterpret_cast<LPVOID>(pRemoteMemory), 
											  NULL, 
											  NULL );

   if( NULL == hThreadHandle )
   {
	   SetLastError( L"CreateRemoteThread failed" );
	   return false;
   }

   AutoFreeHandle afh2( hThreadHandle );

   Sleep( 2500 );
   DeleteFile( m_dllName.c_str() );

   return true;
}

const bool Botox::EnableDebug() const
{
	HANDLE hAppToken = INVALID_HANDLE_VALUE;
		
	if( FALSE == OpenProcessToken( GetCurrentProcess(), 
								   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, 
								   &hAppToken ) )
	{
		SetLastError( L"OpenProcessToken failed with TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY" );
		return false;
	}
	
	AutoFreeHandle afh( hAppToken );
	
	LUID debugLuid = {};

	if( FALSE == LookupPrivilegeValue( NULL, 
									   SE_DEBUG_NAME, 
									   &debugLuid ) )
	{
		SetLastError( L"LookupPrivilegeValue failed for SE_DEBUG_NAME" );
		return false;
	}

	TOKEN_PRIVILEGES tokenPrivileges = {};
	tokenPrivileges.PrivilegeCount = 1;
	tokenPrivileges.Privileges[0].Luid = debugLuid;
	tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if( FALSE == AdjustTokenPrivileges( hAppToken, 
										FALSE, 
										&tokenPrivileges, 
										sizeof( tokenPrivileges ),
										NULL, 
										NULL ) )
	{
		SetLastError( L"AdjustTokenPrivileges failed" );
		return false;
	}

	return true;
}

std::wstring Botox::GetLastError() const
{ 
	return m_lastError;
}

void Botox::SetLastError( const std::wstring& msg ) const
{
	m_lastError = msg;
}

Botox::Botox( DWORD processId )
: m_dwProcessId( processId )
{	
}
	
} //end namespace Botox