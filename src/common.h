#include <cstdio>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <time.h>
#include <string>

#define INDENTATION_SIZE 8
typedef std::vector<std::pair<std::string, std::string> > STR_PAIR_LIST;
typedef std::vector<std::vector<int> > INT_VEC;
typedef std::vector<std::string> STR_LIST;

typedef struct PIN{
        STR_LIST in;
        STR_LIST out;
}PIN;

typedef struct EDGE_NODE{
        std::string edge_name;
        std::vector<std::pair<int,std::string> > component_index_type;
}EDGE_NODE;

typedef std::vector<struct EDGE_NODE> EDGE_GRAPH;


typedef struct VERILOG_NODE{
	std::string node_type;
        std::string node_name;
        STR_PAIR_LIST connected_wire;
}VERILOG_NODE;

typedef std::vector<struct VERILOG_NODE> VERILOG_LIST;


void print_str_list(STR_LIST list);
void print_str_pair_list(STR_PAIR_LIST list);

void def_to_hgr(std::string deffile_name, EDGE_GRAPH &edge_graph, STR_PAIR_LIST &node_list, PIN &pin_list);
STR_PAIR_LIST make_node_list(FILE* deffile);
std::string name_changer(std::string &name);
STR_PAIR_LIST make_list_from_nets(FILE* deffile,STR_LIST &edge_list, const STR_PAIR_LIST &node_list);
int find_node_index(const STR_PAIR_LIST &node_list, const std::string &target);
bool is_str_in_line(std::string target,char* line);
bool is_str_in_line(std::string target, std::string line);
FILE* find_target(std::string deffile_name, std::string target);


void print_edge_graph(EDGE_GRAPH graph);
EDGE_GRAPH make_edge_graph(FILE* deffile, STR_PAIR_LIST &node_list);
bool edge_cmp(EDGE_NODE &a, EDGE_NODE &b);
PIN make_pin_list(FILE* deffile);
int search_index(const std::string target, const STR_PAIR_LIST &node_list, int low, int high);
int search_index(const std::string target, const STR_LIST &list, int low, int high);

void convert_to_hgr(const EDGE_GRAPH &edge_graph, const int node_count, std::string hgrfile_name, std::string fixfile_name);

void write_hgr_from_list(std::vector<std::vector<int> > list, FILE *hgrfile);
void write_fix_from_list(std::vector<int> list, FILE *fixfile, const int node_count);


void make_io_wire_list(const EDGE_GRAPH &graph,PIN &pin, STR_LIST &in, STR_LIST &out, STR_LIST &wire);


void print_verilog_list(const VERILOG_LIST &v);
VERILOG_LIST make_verilog_list(EDGE_GRAPH &graph, STR_PAIR_LIST &node_list);
VERILOG_LIST def_to_verilog(EDGE_GRAPH &edge_graph, STR_PAIR_LIST &node_list, PIN &pin,std::string verilog_name);
void initialize_v_list(VERILOG_LIST &v, const STR_PAIR_LIST &node_list);
void write_to_verilog(STR_LIST &in, STR_LIST &out, const STR_LIST &wire, VERILOG_LIST &v, std::string &name);
std::vector<std::pair<int,std::string>> convert_format(STR_LIST &io);

void print_int_list(std::vector<int> v);
void hgr_to_verilog(std::string hmetis_outfile_name, EDGE_GRAPH &graph, STR_PAIR_LIST &node_list, PIN &pin_list, VERILOG_LIST &v, int partition_size);
bool is_edge_in_pin(std::string edge_name, std::vector<std::pair<int, std::string>> &in, std::vector<std::pair<int, std::string>> &out);
void print_top_module(std::string pure_name, std::vector<std::pair<int, std::string> > &new_in, std::vector<std::pair<int, std::string> > &new_out, STR_LIST &tsv_list, std::vector<std::vector<std::string> > &edge_list,   std::vector<std::vector<std::string> > out_list,  std::vector<std::vector<std::string> > in_list);

void make_nodeline(std::vector<std::vector<std::string> > &nodeline_list, VERILOG_LIST &v, std::vector<int> hmetis_list);
void print_die_module(std::string pure_name, std::vector<std::string> &edge_node_list ,std::vector<std::string> &out_list,  std::vector<std::string> &in_list, std::vector<std::string> &node_line, int die_num);
void print_bottom_module(std::string pure_name, std::vector<std::pair<int, std::string> > &new_in, std::vector<std::pair<int, std::string> > &new_out, std::vector<std::string> &module_wire_list ,std::vector<std::string> &out_list,  std::vector<std::string> &in_list, std::vector<std::string> &node_line);
