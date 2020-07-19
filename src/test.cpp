
#include <iostream>
#include <cstring>
#include <vector>
#include <stdlib.h>

FILE* find_target(std::string deffile_name, std::string target);	
int main(){
	std::string deffile_name = "aes.def";
		
	
	FILE *deffile = find_target(deffile_name, "NETS");	
	
	char *line = NULL;
	size_t len = 0;
	ssize_t read;	
	if(!deffile){
	        fprintf(stdout,"Can't read deffile\n");
		return NULL;
	}
	std::string cur;
	while( (read = getline(&line, &len, deffile)) != -1 ) {
		cur = line;
		std::cout<<cur<<std::endl;
		break;
	}
}


bool is_str_in_line(std::string target, char* line){
	char* ptr = strstr(line, target.c_str());
	if(ptr == NULL) return false;
	else return true;
}
FILE* find_target(std::string deffile_name, std::string target){	
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	
	FILE *deffile = fopen(deffile_name.c_str(), "r");
	if(!deffile){
	        fprintf(stdout,"Can't read deffile\n");
		return NULL;
	}
	std::string cur;
	while( (read = getline(&line, &len, deffile)) != -1 ) {
		if(is_str_in_line(target,line)){
			return deffile;
		}
	}

}

		
