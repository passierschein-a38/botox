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


	29.11.2009 18:27
*/

#include <windows.h>
#include <iostream>

int main( int argc, char** argv )
{
	HMODULE hHandle = LoadLibrary( L"botox.dll" );

	if( NULL == hHandle )
	{
		std::cout << "failed to load botox.dll" << std::endl;
		return -1;
	}

	char c = 0;

	while( c != 'x' )
	{
		system( "cls" );
		std::cout << "press x to exit" << std::endl;
		std::cin >> c;
	}

    FreeLibrary( hHandle );
	
	return 1;
}