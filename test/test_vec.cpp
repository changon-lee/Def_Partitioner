#include <iostream>
#include <vector>

int main(){
	std::vector<std::pair<std::string, std::string> > s;
	std::pair<std::string, std::string> pr = std::make_pair("WErg","WERG");
	s.push_back(pr);
	std::cout<<s[0].first<<std::endl;
		

}
