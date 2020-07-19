#include "common.h"
#include "pystring.h"
#include <string>
void print_int_list(std::vector<int> v){
	for(const auto & a: v){
		std::cout<<a<<"\n";
	}
}

void hgr_to_verilog(std::string hmetis_outfile_name, EDGE_GRAPH &graph, STR_PAIR_LIST &node_list, PIN &pin_list, VERILOG_LIST &v, int partition_size){
	FILE* hmetis = fopen(hmetis_outfile_name.c_str(), "r");
	std::cout<<" - Now complete the partitioner in verilog format ..\n";
	std::string pure_name;
	STR_LIST name_arr;
	name_arr = split(hmetis_outfile_name,".");
	pure_name = name_arr[0];  

	std::vector<int> hmetis_list;
	char *line = NULL;
        size_t len = 0;
        ssize_t read;
	
	int chk_hmetis_output[partition_size] = {0,};
        while( (read = getline(&line, &len, hmetis)) != -1 ) {
		hmetis_list.push_back(atoi(line));
		chk_hmetis_output[atoi(line)] = 1;	
	}
	for(int i=0;i<partition_size;i++){
		if(chk_hmetis_output[i] == 0){
			fprintf(stdout,"%d th die is skipped in hmetis. Program exit\n");
			exit(0);
		}
	}
	//print_int_list(hmetis_list);
	
	int hmetis_list_size = hmetis_list.size();
	int size = v.size();
	int dst, wire_size;
	std::vector<std::vector<std::string> > edge_list(partition_size);

	std::vector<std::pair<int, std::string> > new_in;	
	std::vector<std::pair<int, std::string> > new_out;	
	new_in = convert_format(pin_list.in);
	new_out = convert_format(pin_list.out);

	std::cout<<" - - Make edge list in each module\n";
	for(int i=0;i<size;++i){
		dst = hmetis_list[i];
		wire_size = v[i].connected_wire.size();
		for(int j=0;j<wire_size;j++){
			std::string edge_name = v[i].connected_wire[j].second;
			if(edge_name.find("[") != std::string::npos) continue;
			if(!is_edge_in_pin(edge_name,new_in,new_out)) edge_list[dst].push_back(edge_name);
		}
	}	

	for(int i=0;i<partition_size;++i){
		sort(edge_list[i].begin(), edge_list[i].end());
        	edge_list[i].erase(unique(edge_list[i].begin(), edge_list[i].end()), edge_list[i].end());
	}
	
	STR_LIST TSV_list;
	std::vector<STR_LIST> nodeline_in_modules(partition_size); 
	std::vector<STR_LIST> module_wire_list(partition_size);
	std::vector<STR_LIST> module_input_list(partition_size);
	std::vector<STR_LIST> module_output_list(partition_size);


	std::cout<<" - - Make verilog format line of nodes \n";
	make_nodeline(nodeline_in_modules, v, hmetis_list);

	std::vector<std::pair<int,std::string> > found_index;
	std::pair<int, std::string> int_str_pair;

	int node_index, target_index, high;
	int graph_size = graph.size();
	std::cout<<" - - Make input, output, wire list in each die \n";
	for(int i=0;i<graph_size;++i){
		for(int j=0;j<partition_size;++j){
			high = edge_list[j].size()-1;
			if(search_index(graph[i].edge_name ,edge_list[j],0,high) != -1){ // if found
				int_str_pair = std::make_pair(j,graph[i].edge_name);
				found_index.push_back(int_str_pair);
			}		
		}	
		if(found_index.size() <= 1){ // It is not TSV
			if(graph[i].component_index_type[0].first == -1){ // It is pin
			}	
			else module_wire_list[found_index[0].first].push_back(graph[i].edge_name);
		} 
		else{ // It is TSV 
			TSV_list.push_back(graph[i].edge_name);	
			//Need to find I/O direction
			node_index = graph[i].component_index_type[0].first;
			target_index = hmetis_list[node_index];
			for(const auto &result : found_index){	
				if(result.first == target_index){
					module_output_list[target_index].push_back(result.second);
				}
				else{
					module_input_list[result.first].push_back(result.second);
				}
			}
		}
		found_index.clear();
	}
	std::cout<<" - - Write them into verilog file format \n";
	print_top_module(pure_name, new_in, new_out, TSV_list,edge_list,module_output_list, module_input_list);
	print_bottom_module(pure_name, new_in, new_out, module_wire_list[0], module_output_list[0], module_input_list[0],nodeline_in_modules[0]);
	for(int i=1;i<partition_size;i++){
		print_die_module(pure_name, module_wire_list[i], module_output_list[i], module_input_list[i], nodeline_in_modules[i],i);
	}
	fclose(hmetis);
	
}
void make_nodeline(std::vector<std::vector<std::string> > &nodeline_list, VERILOG_LIST &v, std::vector<int> hmetis_list){
	int partition_size = nodeline_list.size();
	int node_size = v.size();
	std::string line;
	int target_index;
	for(int i=0;i<node_size;++i){
		line = "  "+v[i].node_type+" "+v[i].node_name+" (";
		for(auto &connection : v[i].connected_wire){
			line = line + "."+connection.first+"( "+connection.second+" ) ";
			if(connection != v[i].connected_wire.back()) line = line+", ";
		}
		line = line+");";
		
		target_index = hmetis_list[i];
		nodeline_list[target_index].push_back(line);
	}		
}
void print_bottom_module(std::string pure_name, std::vector<std::pair<int, std::string> > &new_in, std::vector<std::pair<int, std::string> > &new_out, std::vector<std::string> &module_wire_list ,std::vector<std::string> &out_list,  std::vector<std::string> &in_list, std::vector<std::string> &node_line){
	std::string bottom_module_file_name = pure_name + "_die_0.v";
        std::string bottom_module_name = name_changer(pure_name);
        bottom_module_name = bottom_module_name+"_die_0";

        int count = 0;
        FILE *fp = fopen(bottom_module_file_name.c_str(),"w");
	fprintf(fp,"module %s ( ",bottom_module_name.c_str());
        for(auto &input_port : new_in){
		count++;
                fprintf(fp,"%s, ",input_port.second.c_str());
        }
        for(auto &output_port : new_out){
		count++;
                fprintf(fp,"%s, ",output_port.second.c_str());
        }
	for(auto &input : in_list){
		if(count > INDENTATION_SIZE){
			fprintf(fp,"\n       ");	
			count = 0;
		}
		count++;
		fprintf(fp,"%s, ",input.c_str());
	}
	count = 0;
	for(auto &output : out_list){
		if(count > INDENTATION_SIZE){
			fprintf(fp,"\n        ");	
			count = 0;
		}
		count++;
		if(output == out_list.back()) fprintf(fp,"%s ",output.c_str());
		else fprintf(fp,"%s, ",output.c_str());
	}
        fprintf(fp,");\n");
	count = 0;

        for(auto &input_port : new_in){
                if(input_port.first == 0){
                        fprintf(fp,"  input %s;\n",input_port.second.c_str());
                }
                else{
                        fprintf(fp,"  input [%d:0] %s;\n",input_port.first, input_port.second.c_str());
                }
        }
         for(auto &output_port : new_out){
                if(output_port.first == 0){
                        fprintf(fp,"  output %s;\n",output_port.second.c_str());
                }
                else{
                        fprintf(fp,"  output [%d:0] %s;\n",output_port.first, output_port.second.c_str());
                }
        }

	fprintf(fp,"  input ");
	for(auto &input_port : in_list){
		if(count > INDENTATION_SIZE){
			count = 0;
			fprintf(fp,"\n        ");
		}
		count++;
		if(input_port == in_list.back()) fprintf(fp, "%s;\n", input_port.c_str());
                else fprintf(fp,"%s, ",input_port.c_str());
        }
	count = 0;
	fprintf(fp, "  output ");
	for(auto &output_port : out_list){
		if(count > INDENTATION_SIZE){
			count = 0;
			fprintf(fp,"\n        ");
		}
		count++;
		if(output_port == out_list.back()) fprintf(fp, "%s;\n", output_port.c_str());
         	else fprintf(fp,"%s, ",output_port.c_str());
        }
	count = 0;
	fprintf(fp, "  wire ");
	for(auto &wire: module_wire_list){
                if(count > INDENTATION_SIZE){
                        count = 0;
                        fprintf(fp,"\n       ");
                }
                count++;
                if(wire == module_wire_list.back()) fprintf(fp," %s;\n", wire.c_str());
                else fprintf(fp,"%s, ",wire.c_str());
        }
	
	for(auto &node: node_line){
		fprintf(fp,"%s\n", node.c_str());
	}
	fprintf(fp,"endmodule\n");
	fclose(fp); 

}

void print_die_module(std::string pure_name, std::vector<std::string> &edge_node_list ,std::vector<std::string> &out_list,  std::vector<std::string> &in_list, std::vector<std::string> &node_line, int die_num){
	std::string module_file_name = pure_name + "_die_"+std::to_string(die_num)+".v";
        std::string module_name = name_changer(pure_name);
        module_name = module_name+"_die_"+std::to_string(die_num);

        int count = 0;
        FILE *fp = fopen(module_file_name.c_str(),"w");
	fprintf(fp,"module %s ( ",module_name.c_str());
        for(auto &input : in_list){
		if(count > INDENTATION_SIZE){
			fprintf(fp,"\n       ");
			count = 0;
		}
		count++;
                fprintf(fp,"%s, ",input.c_str());
        }
	count = 0;
        for(auto &output : out_list){
		if(count > INDENTATION_SIZE){
			fprintf(fp,"\n        ");
			count = 0;
		}
		count++;
                if(output == out_list.back()) fprintf(fp,"%s ",output.c_str());
                else fprintf(fp,"%s, ",output.c_str());
        }
        fprintf(fp,");\n");
	count = 0;
	fprintf(fp,"  input ");
	for(auto &input_port : in_list){
		if(count > INDENTATION_SIZE){
			count = 0;
			fprintf(fp,"\n        ");
		}
		count++;
		if(input_port == in_list.back()) fprintf(fp, "%s;\n", input_port.c_str());
                else fprintf(fp,"%s, ",input_port.c_str());
        }
	count = 0;
	fprintf(fp, "  output ");
	for(auto &output_port : out_list){
		if(count > INDENTATION_SIZE){
			count = 0;
			fprintf(fp,"\n        ");
		}
		count++;
		if(output_port == out_list.back()) fprintf(fp, "%s;\n", output_port.c_str());
                else fprintf(fp,"%s, ",output_port.c_str());
        }
 
	count = 0;
	fprintf(fp,"  wire ");
	for(auto &wire: edge_node_list){
		if(count > INDENTATION_SIZE){
			count = 0;
			fprintf(fp,"\n       ");
		}
		count++;
		if(wire == edge_node_list.back()) fprintf(fp," %s;\n", wire.c_str());
		else fprintf(fp,"%s, ",wire.c_str());
	}
	for(auto &node: node_line){
		fprintf(fp,"%s\n", node.c_str());
	}
	fprintf(fp,"endmodule\n\n");
	fclose(fp); 

}
bool is_edge_in_pin(std::string edge_name, std::vector<std::pair<int, std::string> > &in, std::vector<std::pair<int, std::string> > &out){
	int in_size= in.size();
	int out_size= out.size();
		
	for(int i=0;i<in_size;++i){
		 if(in[i].second == edge_name) return true;}
	for(int i=0;i<out_size;++i){
		 if(out[i].second == edge_name) return true;}
	return false;
}
void print_top_module(std::string pure_name, std::vector<std::pair<int, std::string> > &new_in, std::vector<std::pair<int, std::string> > &new_out, STR_LIST &tsv_list, std::vector<std::vector<std::string> > &edge_list,   std::vector<std::vector<std::string> > out_list,  std::vector<std::vector<std::string> > in_list){
	std::string topmodule_file_name = pure_name + "_top.v";
	std::string topmodule_name = name_changer(pure_name);
	FILE *fp = fopen(topmodule_file_name.c_str(),"w");
	fprintf(fp,"module %s ( ",topmodule_name.c_str());
        for(auto &input_port : new_in){
                fprintf(fp,"%s, ",input_port.second.c_str());
        }
        for(auto &output_port : new_out){
                if(new_out.back() == output_port){
                        fprintf(fp, "%s ",output_port.second.c_str());
                }
                else fprintf(fp,"%s, ",output_port.second.c_str());
        }
        fprintf(fp,");\n");

        for(auto &input_port : new_in){
                if(input_port.first == 0){
                        fprintf(fp,"  input %s;\n",input_port.second.c_str());
                }
                else{
                        fprintf(fp,"  input [%d:0] %s;\n",input_port.first, input_port.second.c_str());
                }
        }
	 for(auto &output_port : new_out){
                if(output_port.first == 0){
                        fprintf(fp,"  output %s;\n",output_port.second.c_str());
                }
                else{
                        fprintf(fp,"  output [%d:0] %s;\n",output_port.first, output_port.second.c_str());
                }
        }
	
	int count = 0;
        fprintf(fp,"\n  wire ");

        for(auto &wire_name : tsv_list){
                count++;
                if(count > INDENTATION_SIZE){
                        fprintf(fp,"\n       ");
                        count = 0;
                }
                if(wire_name == tsv_list.back()) fprintf(fp, "%s ",wire_name.c_str());
                else fprintf(fp,"%s, ",wire_name.c_str());
        }
        fprintf(fp,";\n\n");

	int partition_size = edge_list.size();
	std::string module_name;
	for(int i=0;i<partition_size;++i){
		count = 0;
		module_name = topmodule_name+"_die_"+std::to_string(i);
		fprintf(fp,"  %s %s ( ",module_name.c_str(), ("u"+std::to_string(i)).c_str());
		if(i == 0){
			for(auto &input_port : new_in){
				fprintf(fp, "%s, ", input_port.second.c_str());
			}
			for(auto &output_port : new_out){
				fprintf(fp, "%s, ", output_port.second.c_str());
			}
			count++;
		}
		for(auto &tsv : in_list[i] ){
			if(count > INDENTATION_SIZE){
				count = 0;
				fprintf(fp,"\n      ");
			}
			count++;
			fprintf(fp,"%s, ",tsv.c_str());
		}
		for(auto &tsv : out_list[i] ){
			if(count > INDENTATION_SIZE){
				count = 0;
				fprintf(fp,"\n      ");
			}
			count++;
			if(tsv == out_list[i].back()){
				fprintf(fp,"%s ",tsv.c_str());
			}
			else fprintf(fp,"%s, ",tsv.c_str());
		}
		fprintf(fp,");\n");
	}		

        fprintf(fp,"endmodule\n");
	
	fclose(fp);

}
