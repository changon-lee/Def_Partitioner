#include <iostream>

#include "pystring.h"

void print_list(std::vector<std::string> s)
{
	for(const auto& a: s){
		std::cout<<a<<std::endl;
	}
}

int main(){
	std::cout<<find("","")<< 0<<"\n";
	std::cout<<find("","a")<< -1<<"\n";
	std::cout<<find("a","")<< 0<<"\n";
	std::cout<<find("a","a")<< 0<<"\n";
	std::cout<<find("","")<< 0<<"\n";
	std::cout<<find("abcdef","")<< 0<<"\n";

	std::cout<<find("abcdef","",-5)<< 1<<"\n";
	std::cout<<find("abcdef","",-2)<< 4<<"\n";
	std::cout<<find("abcdef","def")<< 3<<"\n";
	std::cout<<find("abcdef","def",4)<< -1<<"\n";
	std::vector<std::string> s;
	s = split("abe egr erg werg", " ");

	print_list(s);
	s.clear();
	std::string test;
	test = "aberewrgrbrn\n";
	std::cout<<test<<std::endl;
	test = strip(test);
	std::cout<<test<<std::endl;
	test = replace(test,"a","A");
	std::cout<<test<<std::endl;
	
		
}
	
