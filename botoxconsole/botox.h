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


	29.11.2009 18:47
*/

#ifndef _BOTOX_BOTOX_H_
#define _BOTOX_BOTOX_H_ 1

#include <windows.h>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace Botox{

//typedefs and forwaders
class Botox;
typedef boost::shared_ptr<Botox> BotoxPtr;

/* CreateBotox
* 
* Create an BotoxPtr by passing a pid. 
*
* @param processId
* @return BotoxPtr
*/
BotoxPtr CreateBotox( DWORD processId );

/*
* @brief Botox
*
* This class implements the code injection for remote processes.
* It is very easy to use, without any knowledge about proccess threads and that
* stuff.
*
* Howto use:
*
* @code
*
* BotoxPtr pBotox = CreateBotox( 123455 );
* 
* const bool bSuccess = pBotox->Inject();
*
* if( false == bSuccess )
* {
*	const std::wstring lastError = pBotox->GetLastError();
*   //do error handling
*   return;
* }
*
* //at this point the injection was successfully. The thread was created successfully.
*
* @endcode
*/
class Botox : private boost::noncopyable
{
	/*
	* friend creator
	*/
	friend BotoxPtr CreateBotox( DWORD processId );

public:

	/*
	* D'tor
	*/
	~Botox();

	/*
	* Inject
	*
	* The inject mehtod do all the work. It loads the kernel.dll
	* take the LoadLibrary address, allocates virtual memory, writes the fitting
	* bytes to the right places and creates a remote thread.
	* This method returns true if the remote thread was created, otherwise
	* false is returned.
	*
	* @return bool true on success, otherwise false
	*/
	const bool Inject() const;

	/*
	* GetLastError
	* 
	* Get the last error if occured, otherwise a empty string is returned.
	*/
	std::wstring GetLastError() const;

private:

	/*
	* C'tor
	*
	* Pass the process id into it.
	*
	* @param processId to inject
	*/
	explicit Botox( DWORD processId );

	bool Init();

	/*
	* EnableDebug
	*
	* Try to enable debug privileges on our current process.
	* This method returns true on success, otherwise false.
	*
	* @return bool true on success, otherwise false
	*/
	const bool EnableDebug() const;

	bool Extract() const;

	/*
	* SetLastError
	*
	* Set the last error occured
	*
	* @param msg std::wstring the msg
	*/
	void SetLastError( const std::wstring& msg ) const;

	/*
	* Store the last error
	*/
	mutable std::wstring m_lastError;

	/*
	* Store the remote process id
	*/
	DWORD m_dwProcessId;

	std::wstring m_dllName;

}; //end class Botox


} //end namespace Botox


#endif //_BOTOX_BOTOX_H_