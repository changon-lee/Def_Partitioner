
#include <string>
#include <vector>


#define MAX_SIZE 65535

typedef std::vector<std::string > STR_LIST;
typedef int Py_ssize_t;

STR_LIST split_whitespace(const std::string & str);
STR_LIST split( const std::string & str, const std::string & sep="");

std::string do_strip(const std::string & str,int striptype, const std::string & chars);
std::string strip(const std::string &str, const std::string &chars = "");
std::string lstrip(const std::string &str, const std::string &chars = "");
std::string rstrip(const std::string &str, const std::string &chars = "");

std::string join( const std::string &str, const STR_LIST &seq);
bool isspace(const std::string &str);
int find(const std::string &str, const std::string &sub, int start = 0, int end = MAX_SIZE);

std::string replace(const std::string &str, const std::string &oldstr, const std::string &newstr, int count = -1);

