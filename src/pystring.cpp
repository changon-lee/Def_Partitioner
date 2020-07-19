#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

#include "pystring.h"

 
STR_LIST split_whitespace(const std::string & str)
{
	std::string::size_type i, j, len = str.size();
	std::vector<std::string > result;
	for (i = j = 0; i < len; )
	{
		while ( i < len && isspace( str[i] ) ) i++;
		j = i;
		while ( i < len && ! isspace( str[i]) ) i++;
		if (j < i)
		{
		//	if ( maxsplit-- <= 0 ) break;
			result.push_back( str.substr( j, i - j ));
			while ( i < len && isspace( str[i])) i++;
			j = i;
		}
	}
	if (j < len)
	{
		result.push_back( str.substr( j, len - j ));
	}
	return result;
}

STR_LIST split( const std::string & str, const std::string & sep)
{
	std::vector<std::string > result;
	if ( sep.size() == 0 )
	{
	    result = split_whitespace(str);
	    return result;
	}

	std::string::size_type i,j, len = str.size(), n = sep.size();
	i = j = 0;
	while ( i+n <= len )
	{
	    if ( str[i] == sep[0] && str.substr( i, n ) == sep )
	    {
		result.push_back( str.substr( j, i - j ) );
		i = j = i + n;
	    }
	    else
	    {
		i++;
	    }
	}
	result.push_back( str.substr( j, len-j ) );
	return result;
}

#define LEFTSTRIP 0
#define RIGHTSTRIP 1
#define BOTHSTRIP 2

std::string do_strip(const std::string & str,int striptype, const std::string & chars){
	Py_ssize_t len = (Py_ssize_t) str.size(), i, j, charslen = (Py_ssize_t) chars.size();
	if(charslen == 0)
	{
		i = 0;
		if( striptype != RIGHTSTRIP )
		{
			while(i < len && isspace( str[i] ) ) i++;
		}
		j = len;
		if( striptype != LEFTSTRIP)
		{
			do{
				j--;
			}
			while(j >= 1 && isspace(str[j]));
			j++;
		}
	}
	else{
		const char *sep = chars.c_str();
		i = 0;
		if( striptype != RIGHTSTRIP ){
			while( i < len && memchr(sep,str[i], charslen) ) i++;
		}
		j = len;
		if( striptype != LEFTSTRIP ){
			do{
				j--;
			}
			while( j >= i && memchr(sep,str[j], charslen) );
			j++;
		}
	}
	
	if( i == 0 && j == len) return str;
	else return str.substr(i,j-i);
}

std::string strip(const std::string &str, const std::string &chars){
	return do_strip( str, BOTHSTRIP, chars);
}
std::string lstrip(const std::string &str, const std::string &chars){
	return do_strip( str, LEFTSTRIP, chars);
}
std::string rstrip(const std::string &str, const std::string &chars){
	return do_strip( str, RIGHTSTRIP, chars);
}

std::string join( const std::string &str, const STR_LIST &seq)
{
	STR_LIST::size_type seqlen = seq.size(), i;
	if( seqlen == 0) return "";
	if( seqlen == 1) return seq[0];
	
	std::string result( seq[0] );
	for( i = 1; i < seqlen ; ++i) result += str + seq[i];
	return result;
}

bool isspace(const std::string &str){
	std::string::size_type len = str.size(), i;
	if( len == 0) return false;
	if( len == 1) return isspace( str[0]);
	
	for (i = 0; i < len; ++i){
		if( !isspace(str[i]) ) return false;
	}
	return true;
}
#define ADJUST_INDICES(start, end, len) \
	if(end > len)			\
		end = len;		\
	else if(end < 0){		\
		end += len;		\
		if(end < 0)		\
			end = 0;	\
	}				\
	if(start < 0){			\
		start+=len;		\
		if(start <0)		\
			start=0;	\
	}				\
	


int find(const std::string &str, const std::string &sub, int start, int end)
{
	ADJUST_INDICES(start, end, (int)str.size());
	std::string::size_type result = str.find( sub, start);
	if( result == std::string::npos || (result + sub.size() > (std::string::size_type)end) ){
		return -1;
	}
	return (int)result;
}

std::string replace(const std::string &str, const std::string &oldstr, const std::string &newstr, int count){
	int sofar = 0;
	int cursor = 0;
	std::string s( str );
	std::string::size_type oldlen = oldstr.size(), newlen = newstr.size();
	cursor = find(s, oldstr, cursor);
	
	while( cursor != -1 && cursor <= (int)s.size()){
		if( count > -1 && sofar >= count) break;
		s.replace(cursor, oldlen, newstr);
		cursor += (int) newlen;
		
		if( oldlen != 0) cursor = find(s, oldstr, cursor);
		else ++cursor;
		
		++sofar;
	}
	return s;
}
