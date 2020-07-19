#include "common.h"
#include "pystring.h"

void print_verilog_list(const VERILOG_LIST &v){
	int size = v.size();
	for(int i=0;i<size;++i){
		std::cout<<"node name : "<< v[i].node_name<<"\n";
		std::cout<<"node type : "<< v[i].node_type<<"\n";
		int wire_size = v[i].connected_wire.size();	
		for(int j=0;j<wire_size;++j){
			std::cout<<v[i].connected_wire[j].first<<" "<<v[i].connected_wire[j].second<<"\n";
		}
	}
}

void make_io_wire_list(const EDGE_GRAPH &graph,PIN &pin, STR_LIST &in, STR_LIST &out, STR_LIST &wire){
	int graph_size = graph.size();
	sort(pin.in.begin(), pin.in.end());
	sort(pin.out.begin(), pin.out.end());
	int pin_check;
	std::string name;

	for(int i=0; i< graph_size; ++i){
		pin_check =graph[i].component_index_type[0].first;
		if(pin_check == -1) continue;
	
		name = graph[i].edge_name;
		wire.push_back(name);	
	}
		
	in = pin.in;
	out = pin.out;	
}	
VERILOG_LIST make_verilog_list(EDGE_GRAPH &graph, STR_PAIR_LIST &node_list){
	int node_size = node_list.size();

	VERILOG_LIST v(node_size);

	STR_PAIR_LIST tmp;	
	std::pair<std::string ,std::string> str_pair;
	int graph_size = graph.size();
	int node_connected_size, target;
	std::string edge_n, edge_type;

	initialize_v_list(v, node_list);
	
	for(int i=0;i<graph_size;++i){
		edge_n = graph[i].edge_name;
		node_connected_size = graph[i].component_index_type.size();

		for(int j=0;j<node_connected_size;++j){
			target = graph[i].component_index_type[j].first;
			if(target >= 0){
				edge_type = graph[i].component_index_type[j].second;
				str_pair = std::make_pair(edge_type, edge_n);
				v[target].connected_wire.push_back(str_pair);
			}
		}
	}	
	return v;
}	

void initialize_v_list(VERILOG_LIST &v, const STR_PAIR_LIST &node_list){
	int size = node_list.size();
	for(int i=0;i<size;++i){	
		v[i].node_name = node_list[i].first;
		v[i].node_type = node_list[i].second;
	}
}

	
VERILOG_LIST def_to_verilog(EDGE_GRAPH &edge_graph, STR_PAIR_LIST &node_list, PIN &pin,std::string verilog_name){
	std::cout<<" - Convert def to verilog format ..\n";
	STR_LIST input_port_list;
	STR_LIST output_port_list;
	STR_LIST wire_list;
	VERILOG_LIST verilog_list;

	std::cout<<" - - Make I/O pin and wire list\n";
	make_io_wire_list(edge_graph, pin,  input_port_list, output_port_list, wire_list);
	std::cout<<" - - Verilog format list\n";
	verilog_list = make_verilog_list(edge_graph, node_list);

	// now, write to verilog file
	std::cout<<" - - Wrtie into verilog format. Can skip this step\n";
	write_to_verilog(input_port_list, output_port_list, wire_list, verilog_list, verilog_name);
	return verilog_list;
}
std::vector<std::pair<int,std::string> > convert_format(STR_LIST &io){
	std::vector<std::pair<int,std::string> > new_io;
	std::pair<int, std::string> pair;
	STR_LIST tmp;
	STR_LIST ignore_array;

	int size = io.size();
	std::string saved_str;
	char *cmp;

	for(int i=0; i<size;++i){
		tmp = split(io[i],"[");
		ignore_array.push_back(tmp[0]);
	}
	int count=0;
	saved_str = ignore_array[0];

	for(int i=1;i<size;++i){
		if(saved_str != ignore_array[i] || i == size-1){
			if(i == size-1) count++;
			pair = std::make_pair(count, saved_str);
			count = 0;
			new_io.push_back(pair);
		}
		else{
			count++;
		}
		saved_str = ignore_array[i];
	}	
	return new_io;
}
void write_to_verilog(STR_LIST &in, STR_LIST &out, const STR_LIST &wire, VERILOG_LIST &v, std::string &name){
	FILE *fp = fopen(name.c_str(),"w");
	STR_LIST name_arr;
	name_arr = split(name,".");
	std::string target = name_arr[0];

	std::vector<std::pair<int, std::string> > new_in;
	std::vector<std::pair<int, std::string> > new_out;
	//Need to convert I/O in array form.
	new_in = convert_format(in);
	new_out = convert_format(out);
	target = name_changer(target);
	//Write module
	fprintf(fp,"module %s ( ",target.c_str());	

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

	//Write wire
	int count = 0;
	fprintf(fp,"\n  wire ");

	for(auto &wire_name : wire){
		count++;
		if(count > 10){
			fprintf(fp,"\n       ");
			count = 0;
		}
		if(wire_name == wire.back()) fprintf(fp, "%s ",wire_name.c_str());
		else fprintf(fp,"%s, ",wire_name.c_str());
	}
	fprintf(fp,";\n\n");

	for(auto &node : v){
		fprintf(fp, "  %s ",node.node_type.c_str());
		fprintf(fp, "%s (",node.node_name.c_str());
		for(auto & connection : node.connected_wire){
			if(connection == node.connected_wire.back()) fprintf(fp, ".%s( %s ) ",connection.first.c_str(), connection.second.c_str());
			else fprintf(fp, ".%s( %s ), ",connection.first.c_str(), connection.second.c_str());
		}
		fprintf(fp,");\n");
	}
	fprintf(fp,"endmodule\n");
	//Write nodes
	
	fclose(fp);
}

