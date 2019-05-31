#include <string>
#include <cstdio>
#include "c2sutils.h"
#include <regex>
#include <nlohmann/json.hpp>
#include <fstream>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
using json = nlohmann::json;

json configjson;

void C2SUtils::c2sLog(string input) {
	if(configjson != NULL && configjson.value("log","") == "file"){
		FILE *f = fopen(configjson.value("logpath","").c_str(), "a+");
	    fprintf(f, "%s\n", input.c_str());
	    fclose(f);
	}else{
		printf("%s\n", input.c_str());
	}
}

int C2SUtils::getQuantityFromSku(string title) {
	regex rgx("(?:.*[\\-\\_])(\\d{0,4}.)$");
    smatch match;
    string stringmatch;
    int quantity;
    if (regex_search(title, match, rgx)) {
    	stringmatch = match[1];
    	quantity = stoi(stringmatch);
    	return quantity;
	} else {
		return 1;
	}
}

string C2SUtils::getSkuWithoutQty(string title) {
	regex rgx("(.*)(?:[\\-\\_])(?:\\d{0,2}.)$");
    smatch match;
    string stringmatch;
    if (regex_search(title, match, rgx)) {
    	stringmatch = match[1];
    	return stringmatch;
	} else {
		return title;
	}
}

int C2SUtils::loadConfigValues() {
	
	ifstream in_stream;
	in_stream.open("C:\\c2ssrvc\\config.json");
    if (in_stream.fail()){
        c2sLog("could not open config file to read.");
    } else {
    	std::ifstream cfgfile("C:\\c2ssrvc\\config.json");
		try {
	    	cfgfile >> configjson;
		}
		catch (json::exception& e) {
			c2sLog("error reading config file");
		}
	}
	return 0;
	
}

string C2SUtils::getStringFromConfig(string param) {
	
	string cfgstr;
	if(configjson != NULL){
		try {
	    	cfgstr = configjson.value(param, "");
		}
		catch (json::exception& e) {
			c2sLog("error reading config key "+param);
		}
	}
	return cfgstr;
	
}

void C2SUtils::c2sSleep(int lapse) {
	#ifdef _WIN32
		Sleep(lapse);
	#else
		usleep(lapse*1000);  /* sleep for 100 milliSeconds */
	#endif
}
