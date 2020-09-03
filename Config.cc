#include "Config.h"
using namespace std;

Config::Config(const char* file, const string delim, const string comment):m_Delimiter(delim),m_Comment(comment){
	ifstream fin(file);
	if(!fin)
		throw File_Not_Found(string("Cannot find the file : ") + file);
	fin >> (*this);
}

Config::Config(const char* file):m_Delimiter("="),m_Comment("#"){
	ifstream fin(file);
	if(!fin)
		throw File_Not_Found(string("Cannot find the file : ") + file);
	fin >> (*this);
}

void Config::Trim(string& str){
	static const string whitespace(" \n\t\v\r\f");
	str.erase(0, str.find_first_not_of(whitespace));
	str.erase(str.find_last_not_of(whitespace) + 1U);
}

ostream& operator<<(ostream& os, const Config& cf){
	for(auto iter = cf.m_Content.cbegin();iter != cf.m_Content.cend();++iter){
		os << iter->first << " " << cf.m_Delimiter << " " << iter->second << endl;
	}
	return os;
}

istream& operator>>(istream& is, Config& cf){
	using pos = string::size_type;
	//进行配置文件的解析需要三个东西：分隔符、注释符和分隔符的长度
	const string& delim = cf.m_Delimiter;
	const string& comm = cf.m_Comment;
	const pos skip = delim.length();
	//nextline的作用是为了避免折行输入
	string nextline = "";
	while(is || nextline.length() > 0){
		string line = "";
		if(nextline.length() > 0){
			line = nextline;
			nextline = "";
		}else{
			getline(is, line);
		}
		//step 1: 去掉注释部分
		line = line.substr(0, line.find(comm));
		//step 2: 将单条记录分为 key 和 value
		pos delim_pos = line.find(delim);
		if(delim_pos < string::npos){
			string key = line.substr(0, delim_pos);
			line.replace(0, delim_pos + skip, "");
			bool terminate = false;
			while(!terminate && is){
				getline(is, nextline);
				terminate = true;
				nextline = nextline.substr(0,nextline.find(comm));
				Config::Trim(nextline);
				if(nextline == "")
					continue;
				if(nextline.find(delim) != string::npos)
					continue;
				line += nextline;
				terminate = false;
			}
			Config::Trim(key);
			Config::Trim(line);
			cf.m_Content[key] = line;
		}
	}
	return is;
}
