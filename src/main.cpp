#include "partitioner.h"
#include "common.h"
#include <stdlib.h>

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("Need benchmark folder name, partitioning size\n");
		printf("ex ) %s aes 4\n", argv[0]);
		exit(0);
	}

	clock_t start,end;
	start = clock();		
	std::cout<<"****************************************\n";
	std::cout<<"*      Cpp implemented Partitioner     *\n";
	std::cout<<"*              2020-07-11              *\n";
	std::cout<<"****************************************\n";

	std::string input_name = argv[1];
	std::string deffile_name = input_name+"/"+input_name+".def";
	int partition_size = atoi(argv[2]);

	STR_LIST name_arr;
	name_arr = split(deffile_name, ".");
	std::string pure_name = name_arr[0];
	std::string hgr_name = pure_name + ".hgr";
	std::string fix_name = pure_name + ".fix";

	std::string verilog_name = pure_name + "_converted.v";
	EDGE_GRAPH graph;
	STR_PAIR_LIST node_list;
	PIN pin_list;
	VERILOG_LIST v;

	//convert def to hgr format
	def_to_hgr(deffile_name,graph,node_list,pin_list);	

	//convert def to verilog ( not needed, but useful )
	v = def_to_verilog(graph, node_list,pin_list, verilog_name);
	
	std::string partition_size_str = argv[2];
	std::string hmetis_outfile_name = pure_name + ".hgr.part."+partition_size_str;
	
	//run hmetis
	std::string program_line = "./run_hmetis.csh "+hgr_name+" "+fix_name+" "+partition_size_str;
	std::string cp_line = "cp "+pure_name+"_top.v ";
	std::string cat_line = "cat "+pure_name+"_top.v ";
	for(int i=0;i<partition_size;++i){
		cp_line = cp_line + pure_name+"_die_"+std::to_string(i)+".v ";
		cat_line = cat_line + pure_name+"_die_"+std::to_string(i)+".v ";
	}
	cat_line = cat_line + "> "+pure_name+"_all.v";
	cp_line = cp_line + pure_name+"_all.v ";
	system(program_line.c_str());
	
	//complete partitioner.
	hgr_to_verilog(hmetis_outfile_name, graph, node_list, pin_list, v, partition_size);
	system(cat_line.c_str());

	std::string folder_name = "benchmark/"+input_name+"_partition_size_"+partition_size_str;
	cp_line = cp_line + folder_name+"/.";
	system(("mkdir "+folder_name).c_str());
	system(cp_line.c_str());
	
	end = clock();
	double result = (double)(end - start);
	
	printf("%.3f sec\n", result/CLOCKS_PER_SEC);
}

