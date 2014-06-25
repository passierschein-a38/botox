#include <windows.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

typedef struct 
{
	HHOOK hook;
	HWND hWnd;
	DWORD tid;
}Info ;

static bool bIsRunning = true;
static std::vector<Info> infected;

// Hook function for keyboard hook
LRESULT CALLBACK KeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
	if( 0 > nCode )
	{
		return CallNextHookEx( NULL, nCode, wParam, lParam );
	}

	if( nCode != HC_ACTION )
	{
		return CallNextHookEx( NULL, nCode, wParam, lParam );
	}

   if( wParam == VK_F1 )
   {
	   MessageBox( 0, L"VK_F1", L"VK_F1", MB_OK );
   }

   return CallNextHookEx( NULL, nCode, wParam, lParam );
}

BOOL CALLBACK EnumWindowsProc( HWND hwnd, LPARAM lParam ) 
{
	const DWORD currentID = GetCurrentProcessId();
	DWORD procid = 0;
	const DWORD threadId = GetWindowThreadProcessId (hwnd, &procid);

	if( currentID == procid )
	{	
		if( IsWindow( hwnd ) )
		{
			Info entry = {};

			entry.hook = SetWindowsHookEx( WH_KEYBOARD, KeyboardProc, NULL, threadId );
			entry.hWnd = hwnd;
			entry.tid  = threadId;

			if( entry.hook != NULL )
			{			
				infected.push_back( entry );
			}
		}
	}

	return TRUE;
}

DWORD WINAPI ThreadProc(  LPVOID lpParameter )
{

   EnumWindows( EnumWindowsProc, 0 );
  
   while( bIsRunning )
   {
	   Sleep( 25 );
   }

	return 0;
}

void DoAction()
{
	CreateThread( NULL,
				  0,
				  ThreadProc,
				  0,
				  0,
				  0);	
}

void UndoAction()
{
	std::vector<Info>::const_iterator it = infected.begin();

	while( it != infected.end() )
	{
		UnhookWindowsHookEx( (*it).hook );
		++it;
	}

	infected.clear();
}


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /* lpvReserved */)
{
   if (dwReason == DLL_PROCESS_ATTACH )
   {
	    bIsRunning = true;
		DoAction();
   }

   if( dwReason == DLL_PROCESS_DETACH )
   {
	   bIsRunning = false;
	   UndoAction();
   }

   return TRUE;
}