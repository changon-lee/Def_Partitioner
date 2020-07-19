#include <iostream>
#include <vector>


typedef std::vector<std::string > STR_LIST;


int main(){
	STR_LIST s;
	s.push_back("HH");
	s.push_back("HH");
	s.push_back("HH");

	for(const auto &a : s){
		std::cout<<a<<"\n";
	}
}

