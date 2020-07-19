
#include "common.h"
#include "pystring.h"
#include <iostream>
#include <vector>

void print_str_list(STR_LIST list){
	for(const auto &a: list){
		std::cout<<a<<"\n";
	}
}
void print_str_pair_list(STR_PAIR_LIST list){
	for(const auto &a: list){
		std::cout<<a.first<<" "<<a.second<<"\n";
	}
}

void print_edge_graph(EDGE_GRAPH graph){
	int size = graph.size();
	for(int i=0;i<size;++i){
		std::cout<<"edge name : "<<graph[i].edge_name<<"\n";
		std::cout<<"connected with \n";
		int comp_size = graph[i].component_index_type.size();
		for(int j=0;j<comp_size;++j){
			std::cout<<"Index : "<<graph[i].component_index_type[j].first;
			std::cout<<"Type : "<<graph[i].component_index_type[j].second<<"\n";
		}
	}
}
void def_to_hgr(std::string deffile_name, EDGE_GRAPH &edge_graph, STR_PAIR_LIST &node_list, PIN &pin_list){
	STR_LIST name_list;
	std::cout<<" - Convert def file to hgr file..\n";
	name_list = split(deffile_name, ".");
	std::string base_name = name_list[0];
		
	std::string hgrfile_name = base_name + ".hgr";
	std::string fixfile_name = base_name + ".fix";
		
	STR_LIST outfileList;
	
	STR_PAIR_LIST hgr_list;

	FILE *deffile = find_target(deffile_name, "COMPONENTS");
	if(!deffile){
		exit(0);
	}
	// First, make component list
	// Using vector pair<string,string>. which is name and component type
	// (name, type) -> (name, type) -> ...	
	std::cout<<" - - Make node list\n";
	node_list = make_node_list(deffile);
	// and sort (Since I use binary search later)
	std::cout<<" - - Sort node list\n";
	sort(node_list.begin(),node_list.end());
	//print_str_pair_list(node_list);
	
	// Second, make pin list
	// Using vector pair<string, string>, which is name and direction.
	// (name, direction) -> (name, direction) -> ...
	deffile = find_target(deffile_name, "PINS");	
	std::cout<<" - - Make pin list\n";
	pin_list = make_pin_list(deffile);	

	deffile = find_target(deffile_name, "NETS");	
	std::cout<<" - - Make edge graph\n";
	edge_graph = make_edge_graph(deffile, node_list);
	//print_edge_graph(edge_graph);
	
	convert_to_hgr(edge_graph, node_list.size(),hgrfile_name, fixfile_name);
}
void convert_to_hgr(const EDGE_GRAPH &edge_graph, const int node_count, std::string hgrfile_name, std::string fixfile_name){
	FILE* hgrfile = fopen(hgrfile_name.c_str(), "w");
	FILE* fixfile = fopen(fixfile_name.c_str(), "w");
	if(!hgrfile || !fixfile){
		fprintf(stdout,"hgr,fix file open error!\n");
		exit(0);
	}
	std::cout<<" - - Make hgr file & fix file .. ";	
	//make hgr file first.
	int size = edge_graph.size();
	int node_size, node_index;
	std::vector<int> fixfile_index_list;
	std::vector<std::vector<int> > hgrfile_index_list;
	std::vector<int> tmp;
	bool insert = false;
	tmp.push_back(edge_graph.size());
	tmp.push_back(node_count);
	hgrfile_index_list.push_back(tmp);

	for(int i=0;i<size;++i){
		tmp.clear();
		node_size = edge_graph[i].component_index_type.size();
		// if edge is I/O pin need fix file
		if(edge_graph[i].component_index_type[0].first == -1){
			if(node_size >= 3) insert = true;
			for(int j=1;j<node_size;++j){
				node_index = edge_graph[i].component_index_type[j].first;
				fixfile_index_list.push_back(node_index);
				if(insert){
					tmp.push_back(node_index+1);
				}
			}
			if(insert) hgrfile_index_list.push_back(tmp);
		}
		// if edge is not I/O pin
		else{
			if(node_size >= 2) insert = true;
			if(insert){
				for(int j=0; j<node_size;++j){
					node_index = edge_graph[i].component_index_type[j].first;
					tmp.push_back(node_index+1);
				}
				hgrfile_index_list.push_back(tmp);
			}
		}
		insert = false;
	}	
	hgrfile_index_list[0][0] = hgrfile_index_list.size()-1;
	write_hgr_from_list(hgrfile_index_list,hgrfile);
	write_fix_from_list(fixfile_index_list,fixfile, node_count);
	std::cout<<"Done!\n";


	fclose(hgrfile);	
	fclose(fixfile);	
}
void write_hgr_from_list(std::vector<std::vector<int> > list, FILE *hgrfile){
	int size = list.size();
	for(int i=0;i<size;++i){
		int line_size = list[i].size();
		for(int j=0;j<line_size;++j){
			fprintf(hgrfile, "%d ", list[i][j]);		
		}
		fprintf(hgrfile, "\n");
	}				
}
void write_fix_from_list(std::vector<int> list, FILE *fixfile, const int node_count){
	sort(list.begin(),list.end());
	list.erase(unique(list.begin(), list.end()), list.end());

	int size = list.size();	
	int cur = 0;
	for(int i=0;i<node_count;++i){
		if(i == list[cur]){
			fprintf(fixfile,"0\n");				
			if(cur < size) cur++;
		}
		else fprintf(fixfile,"-1\n");
	}	
}

EDGE_GRAPH make_edge_graph(FILE* deffile, STR_PAIR_LIST &node_list){
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	STR_LIST tmp;
	std::string cur,name,type;
	int index;

	EDGE_NODE graph_node;
	EDGE_GRAPH edge_graph;
	std::pair<int, std::string> index_type_pair;
	int high = node_list.size()-1;
	int low = 0;

	std::string original_name;

	while( (read = getline(&line, &len, deffile)) != -1 ) {
		if(is_str_in_line("NETS",line)){
			edge_graph.push_back(graph_node);
			break;
		}
		cur = line;
		tmp = split(cur);

		if(tmp[0] == "-"){
			if(!graph_node.component_index_type.empty()){
				edge_graph.push_back(graph_node);
				graph_node.component_index_type.clear();
			}
			original_name = tmp[1];
			name = name_changer(tmp[1]);
			graph_node.edge_name = name;
		}
		else if(tmp[0] == "("){
			if(tmp[1] == "PIN"){
				graph_node.edge_name = original_name;
				index_type_pair = std::make_pair(-1,"PIN!");
				graph_node.component_index_type.push_back(index_type_pair);
				continue;
			}
			name = name_changer(tmp[1]);
			index = search_index(name, node_list, low, high);
			index_type_pair = make_pair(index, tmp[2]);
			graph_node.component_index_type.push_back(index_type_pair);
		}
	}
	sort(edge_graph.begin(),edge_graph.end(),edge_cmp);
	return edge_graph;
}
bool edge_cmp(EDGE_NODE &a, EDGE_NODE &b){
	return a.edge_name < b.edge_name;
}
int search_index(const std::string target, const STR_PAIR_LIST &node_list, int low, int high){
	if(low > high) {
		fprintf(stdout," Can't find target in node list ! \n");
		return -1;
	}
	int mid = (low + high)/2;
	if(node_list[mid].first == target) {
		return mid;
	}
	else if(node_list[mid].first > target) return search_index(target,node_list, low, mid-1);	
	else return search_index(target,node_list,mid+1,high);
}
int search_index(const std::string target, const STR_LIST &list, int low, int high){
	if(low > high) {
		return -1;
	}
	int mid = (low + high)/2;
	if(list[mid] == target) {
		return mid;
	}
	else if(list[mid] > target) return search_index(target,list, low, mid-1);	
	else return search_index(target,list,mid+1,high);
}
STR_PAIR_LIST make_node_list(FILE* deffile){
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	STR_LIST tmp;
	std::string cur,name,type;
	STR_PAIR_LIST outlist;
	std::pair<std::string, std::string> name_type_pair;
	while( (read = getline(&line, &len, deffile)) != -1 ) {
		if(is_str_in_line("COMPONENTS",line)){
			break;
		}
		cur = line;
		tmp = split(cur);
		name = name_changer(tmp[1]);
		type = tmp[2];
		name_type_pair = std::make_pair(name,type);
		outlist.push_back(name_type_pair);
	}
	return outlist;
}

PIN make_pin_list(FILE* deffile){
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	STR_LIST tmp;
	std::string cur,name,direction;
	PIN outlist;

	while( (read = getline(&line, &len, deffile)) != -1 ) {
		if(is_str_in_line("PINS",line)){
			break;
		}
		cur = line;
		tmp = split(cur);
		name = tmp[1];
		direction = tmp[7];
		if(direction == "INPUT"){
			outlist.in.push_back(name);
		}
		else if(direction == "OUTPUT"){
			outlist.out.push_back(name);
		}
		else{
			fprintf(stdout,"There are other options than in/out, should handle this.\n");
			exit(0);
		}
	}
	return outlist;
}
std::string name_changer(std::string &name){
	name = replace(name,"\\","");
	name = replace(name,"/","_");
	name = replace(name,"[","_");
	name = replace(name,"]","");
	return name;
}


STR_PAIR_LIST make_list_from_nets(FILE* deffile,STR_LIST &edge_list, const STR_PAIR_LIST &node_list){
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	STR_LIST tmp;
	std::string cur, tag;

	std::string wire_target = "-";
	std::vector<std::pair<int, std::string> > hgr_list;
	int index = -1;
	while( (read = getline(&line, &len, deffile)) != -1 ) {
		if(is_str_in_line("NETS",line)){
                        break;
                }
		cur = line;
		tmp = split(cur);
		if(tmp[0] == wire_target){
			edge_list.push_back(tmp[1]);	
		}
		else{
			if(tmp[1] == "PIN") continue;
			tag = name_changer(tmp[1]);
			index = find_node_index(node_list,tag);
			std::pair<int, std::string> pr = std::make_pair(index, tmp[2]);
			hgr_list.push_back(pr);
		}				
	}
}

int find_node_index(const STR_PAIR_LIST &node_list, const std::string &target){
	int size = node_list.size();
	for(int i=0;i<size;++i){
		if(node_list[i].first == target) return i;
	}
	std::cout<<"ERROR!"<<"\n";
	return -1;
}
bool is_str_in_line(std::string target, char* line){
	char* ptr = strstr(line, target.c_str());
	if(ptr == NULL) return false;
	else return true;
}
bool is_str_in_line(std::string target, std::string line){
	if(line.find(target) == std::string::npos) return false;
	return true;
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

		
	
