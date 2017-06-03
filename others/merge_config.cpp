#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <map>
#include <vector>

using namespace std;

const string SQL_PORT = "1027";
const int MAX_SERVER = 5;
string TEMP_FILE = "temp_merge_info.tmp";

string cur_ip;
string file;
string host_id;
string guest_id;
int server_cnt = 0;
int sub_servers = 0;

void fill_sub_ids(vector<string>& sub_ids, const string& ids)
{
	smatch m;
	regex e("[^ ]+");
	string ids_copy = ids;
	while (regex_search(ids_copy, m, e))
	{
		for (auto id : m)
		{
			++sub_servers;
			sub_ids.push_back(id);
		}
		ids_copy = m.suffix().str();
	}
}

struct ServerInfo
{
	string main_id;
	vector<string> sub_ids;
	string sub_id_str;
	string port;
	string sql_port;
	string ip;

	void merge(const ServerInfo* rhs)
	{
		sub_id_str += " " + rhs->sub_id_str;
		sub_ids.clear();
		fill_sub_ids(sub_ids, sub_id_str);
	}
};

map<string, ServerInfo> servers;	//main_id-->info
map<string, std::vector<string> > machines;	//ip-->(servers)
vector<string> comments;


bool isEmptyLine(const string& line)
{
	return line == "";
}

bool isCommentLine(const string& line)
{
	size_t pos = line.find_first_not_of(' ');
	if (pos != string::npos && line.size() >= pos + 1)
	{
		return line[pos] == '=' && line[pos] == '=';
	}
	return false;
}

bool isIpAddress(const string& line)
{
	//do not check ip rule, just check the format
	smatch m;
	regex e("([0-9]{1,3}\\.){3}[0-9]{1,3}");

	return regex_search(line, m, e);
}

int addServer(const string& ids, const string& sql_port, const string& port)
{
	std::vector<string> sub_ids;
	fill_sub_ids(sub_ids, ids);
	if (sub_ids.empty())
		return 1;

	ServerInfo server;
	server.main_id = sub_ids.front();
	server.sub_ids.swap(sub_ids);
	server.sub_id_str = ids;
	server.port = port;
	server.sql_port = sql_port;
	server.ip = cur_ip;
	servers.insert(make_pair(server.main_id, server));
	machines[cur_ip].push_back(server.main_id);

	++server_cnt;
	return 0;
}

void addServerInfo(const string& line)
{
	smatch m;
	regex e("\\[((?:xy\\d{1,3}\\s|\\d{1,5}\\s)*(?:xy\\d{1,3}|\\d{1,5}))\\]\\s(102[567])\\s(800[02468])");

	bool ret = regex_match(line, m, e);
	if (!ret || m.size() != 4)
	{
		//wrong line, do nothing!
		return;
	}

	addServer(m[1].str(), m[2].str(), m[3].str());
}

void process_line(const string& line) 
{
	//cout << line << endl;
	if (isIpAddress(line))
	{
		cur_ip = line;
	}
	else
	{
		addServerInfo(line);
	}
}

ServerInfo* getServerInfo(const string& main_id)
{
	auto it = servers.find(main_id);
	if (it != servers.end())
		return &it->second;

	return nullptr;
}

int read_file()
{
	cout << "File:" << file << " Host:" << host_id << " Guest:" << guest_id << endl;

	ifstream infile(file.c_str());
	if (!infile)
	{
		cout << "Can't Open " << file << ", No such file!" << endl;
		return 1;
	}

	string line;
	size_t i = 0;
	while (std::getline(infile, line))
	{
		if (isEmptyLine(line))
		{
			//do nothing
		}
		else if (isCommentLine(line))
		{
			comments.push_back(line);
		}
		else
		{
			process_line(line);
		}
	}

	return 0;
}

void write_info(const ServerInfo& info, ofstream& outfile)
{
	outfile << '[';
	for (size_t i = 0; i < info.sub_ids.size(); ++i)
	{
		outfile  << info.sub_ids[i];
		if ( i != info.sub_ids.size() - 1)
			outfile << ' ';
	}
	outfile << "] ";
	outfile << info.sql_port << " " << info.port << endl;
}

int write_file()
{
	ofstream outfile(file.c_str());
	if (!outfile)
	{
		cout << "Can't Open " << file << ", No such file!" << endl;
		return 1;
	}

	//TODO:add comment lines
	for (auto machine : machines)
	{
		outfile << machine.first << endl;
		for (auto main_id : machine.second)
		{
			if (ServerInfo* info = getServerInfo(main_id))
			{
				write_info(*info, outfile);
			}
		}
		outfile << endl;
	}
	for (auto comment : comments)
	{
		outfile << comment << endl;
	}

	return 0;
}

void eraseFromMachine(ServerInfo* guest_server)
{
	auto it = machines.find(guest_server->ip);
	if (it == machines.end())
		return;

	for (auto iter = it->second.begin(); iter != it->second.end(); ++iter)
	{
		if (*iter == guest_server->main_id)
		{
			it->second.erase(iter);
			return;
		}
	}
}
void eraseFromServers(ServerInfo* guest_server)
{
	servers.erase(guest_server->main_id);
}
void generate_merge_info()
{
	ofstream outfile(TEMP_FILE);
	if (!outfile)
	{
		cout << "Can't Open " << file << ", No such file!" << endl;
		return;
	}
	ServerInfo* host_server = getServerInfo(host_id);
	ServerInfo* guest_server = getServerInfo(guest_id);

	if (!host_server || !guest_server)
	{
		cout << "One of the servers doesn't exist!" << endl;
		exit(1);
	}

	if (host_server->sql_port != SQL_PORT || guest_server->sql_port != SQL_PORT)
	{
		cout << "One of the servers isn't using sql_port "<< SQL_PORT << "!" << endl;
		exit(1);
	}

	outfile << "HOST_IP=" << host_server->ip << endl;
	outfile << "GUEST_IP=" << guest_server->ip << endl;
	outfile << "HOST_ID=" << host_id << endl;
	outfile << "GUEST_ID=" << guest_id << endl;
	outfile << "GUEST_SUB_ID=\"";
	for (auto id : guest_server->sub_ids)
	{
		if (id.size() > 2 && id.find("xy") != string::npos)
		{
			outfile << id.substr(2);
		}
		else
		{
			outfile << id;
		}
		if (id != guest_server->sub_ids.back())
			outfile << " ";
	}
	outfile << "\"" << endl;
	outfile << "SQL_PORT=" << SQL_PORT << endl;

	//modify memory data
	host_server->merge(guest_server);
	eraseFromMachine(guest_server);
	eraseFromServers(guest_server);
}

void printAllServers()
{
	for (auto server : servers)
	{
		cout << "Server " << server.first << " " << server.second.port << " " << server.second.sql_port << endl;
		cout << "[ ";
		for (auto id : server.second.sub_ids)
			cout << id << " ";
		cout << "]" << endl;
	}
}

void printMachines()
{
	for (auto machine : machines)
	{
		cout << machine.first << endl;
		for (auto id : machine.second)
		{
			cout << id << " ";
		}
		cout << endl;
	}
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		cout << "need file name and 2 server id here!" << endl;
		return 1;
	}

	file = argv[1];
	host_id = argv[2];
	guest_id = argv[3];

	if (host_id == guest_id)
	{
		cout << "Host can't be the same with Guest!" << endl;
		return 1;
	}

	read_file();

	//printAllServers();
	//printMachines();

	generate_merge_info();

	write_file();

	return 0;
}
