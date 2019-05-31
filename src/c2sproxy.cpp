#include <stdio.h>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include "c2sproxy.h"
#include "c2sutils.h"
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <chrono>
#include <thread>
#include <time.h>
#include <windows.h>


using namespace std;
using json = nlohmann::json;
using namespace std::this_thread;
using namespace std::chrono;

json products = {{"products", json::array()}};
json orders = {{"orders", json::array()}};

const int pagesize = 50;

string globalGet(string path);
string globalPost(string path, string data);
string getPrivateKey();
string genKey();
string makePath(string privatekey, string path);
char* flushProductsToUpstream(char* strategy);
char* flushOrdersToUpstream(char* strategy);
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
string parseStringToJson(string stringtoparse, string valuetoobtain, string defaultvalue);

static C2SUtils utils;

bool C2SProxy::sense() {

	bool senseConnection;
	string readBuffer;
	CURL *curl;
	CURLcode res;
	 
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if(curl) {
	    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	    curl_easy_setopt(curl, CURLOPT_URL, utils.getStringFromConfig("click2syncendpoint").c_str());
	    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	    curl_easy_setopt(curl, CURLOPT_CAINFO, utils.getStringFromConfig("opensslcacertpath").c_str());
	    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);  
	    res = curl_easy_perform(curl);
	    if(res == CURLE_HTTP_RETURNED_ERROR || res != CURLE_OK){
	    	char* errcode = (char*)curl_easy_strerror(res);
	    	string errmsg = "failed: ";
	    	errmsg.append(errcode);
	    	utils.c2sLog("failed: " + errmsg);
	    	//printf("res = %d curl_easy_perform() failed: %s \n",res, curl_easy_strerror(res));
	    	senseConnection = false;
		} else {
			senseConnection = true;
		}

	    curl_easy_cleanup(curl);
		 
	}
	curl_global_cleanup();	
    return senseConnection;    
}

bool C2SProxy::checkIfFetchingRemote() {
	string key = getPrivateKey();
	string path = "/ping";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	if(response == "") { 
		utils.c2sLog("no response");
		return false;
	} else {
		string value = parseStringToJson(response, "connectionstatus", "");
		if(value != "" && value == "fetchingremote") {
			return true;
		} else {
			return false;
		}
	}
}

bool C2SProxy::checkIfPullingToRemote() {
	string key = getPrivateKey();
	string path = "/ping";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	if(response == "") { 
		utils.c2sLog("no response");
		return false;
	} else {
		string value = parseStringToJson(response, "connectionstatus", "");
		if(value != "" && value == "pulling") {
			return true;
		} else {
			return false;
		}
	}
}

string C2SProxy::getStrategy(){
	string key = getPrivateKey();
	string path = "/strategy";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	if(response == "") {
		utils.c2sLog("no response");
		return (string)"";
	} else {
		json jsonresponse;
		jsonresponse = json::parse(response);
		string value = jsonresponse.value("strategy", "");
		return value;
	}
}

string C2SProxy::getStatus(){
	string key = getPrivateKey();
	string path = "/status";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	if(response == "") {
		utils.c2sLog("no response");
		return (string)"";
	} else {
		json jsonresponse;
		jsonresponse = json::parse(response);
		string value = jsonresponse.value("status", "");
		return value;
	}
}

string C2SProxy::getUpstreamStatus(){
	string key = getPrivateKey();
	string path = "/upstreamstatus";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	if(response == "") {
		utils.c2sLog("no response");
		return (string)"";
	} else {
		json jsonresponse;
		jsonresponse = json::parse(response);
		string value = jsonresponse.value("upstreamstatus", "");
		return value;
	}
}

string C2SProxy::getEntity(){
	string key = getPrivateKey();
	string path = "/entity";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	if(response == "") {
		utils.c2sLog("no response");
		return (string)"";
	} else {
		json jsonresponse;
		jsonresponse = json::parse(response);
		string value = jsonresponse.value("entity", "");
		return value;
	}
}

int C2SProxy::getCursorOffset(){
	string key = getPrivateKey();
	string path = "/cursoroffset";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	if(response == "") {
		utils.c2sLog("no response");
		return 0;
	} else {
		json jsonresponse;
		jsonresponse = json::parse(response);
		int value = jsonresponse.value("cursoroffset", 0);
		return value;
	}
}

char* C2SProxy::setInitializeUpload(char* strategy){
	string post = "{}";				
	string key = getPrivateKey();
	string verb = ((string)strategy == "pingsample") ? "/pingsample" : "/push";
	string endpoint = "/initialize";
	string path = verb + endpoint;
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, post);
	return (char*) "success";
}

char* C2SProxy::setInitializeDownload(){
	string post = "{}";				
	string key = getPrivateKey();
	string endpoint = "/pull/initialize";
	string fullpath = makePath(key, endpoint);
	string res = globalPost(fullpath, post);
	return (char*) "success";
}

char* C2SProxy::setProductToUploadOnBuffer(json product, char* strategy){
	products["products"].push_back(product);
	if(products["products"].size() >= pagesize) {
		string flus = flushProductsToUpstream(strategy);
		return (char*)flus.c_str();
	} else {
		return (char*)"no flush yet";
	}
}

char* C2SProxy::setOrderToUploadOnBuffer(json order, char* strategy) {
	orders["orders"].push_back(order);
	if(orders["orders"].size() >= pagesize) {
		string flus = flushOrdersToUpstream(strategy);
		return (char*)flus.c_str();
	} else {
		return (char*)"no flush yet";
	}
}

char* C2SProxy::flushProductsToUpstream(char* strategy){
	string key = getPrivateKey();
	string verb = ((string)strategy == "pingsample") ? "/pingsample" : "/push";
	string endpoint = "/products";
	string path = verb + endpoint;
	string fullpath = makePath(key, path);
	string productsparsed = products.dump();
	string res = globalPost(fullpath, productsparsed);
	products["products"].clear();
	return (char*)"succes";
}

char* C2SProxy::flushOrdersToUpstream(char* strategy){
	string key = getPrivateKey();
	string verb = ((string)strategy == "pingsample") ? "/pingsample" : "/push";
	string endpoint = "/orders";
	string path = verb + endpoint;
	string fullpath = makePath(key, path);
	string ordersparsed = orders.dump();
	string res = globalPost(fullpath, ordersparsed);
	orders["orders"].clear();
	return (char*)"succes";
}

bool C2SProxy::hasMoreUpdatedProducts(int attempts){	
	string key = getPrivateKey();
	string path = "/pull/products/hasmore";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	json jsonresponse;
	jsonresponse = json::parse(response);
	string hasMoreStatus = jsonresponse.value("hasmore", "");

	if(hasMoreStatus == "ready") {
		return true;
	} else if (hasMoreStatus == "waiting") {
		Sleep(1000);
		if (attempts > 10){
			return false;
		} else {
			return hasMoreUpdatedOrders(attempts +1);
		}
	} else {
		utils.c2sLog("no response");
		return false;
	}
}

json C2SProxy::nextProduct(){	
	string key = getPrivateKey();
	string path = "/pull/products/next";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
//	utils.c2sLog("reponse hasMoreUpdatedOrders: " + response);

	json productDefault = json({});
	json productValue;
	if(response == "") {
		utils.c2sLog("no response");
		return productValue;
	} else {
		json jsonresponse;
		jsonresponse = json::parse(response);
		try{
			productValue = jsonresponse.value("product", productDefault);	
		}
		catch (json::exception& e) {
			// output exception information
			utils.c2sLog("Error reading product");
			std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
		}
		return productValue;
	}
}

bool C2SProxy::hasMoreUpdatedOrders(int attempts){	
	string key = getPrivateKey();
	string path = "/pull/orders/hasmore";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	json jsonresponse;
	jsonresponse = json::parse(response);
	string hasMoreStatus = jsonresponse.value("hasmore", "");

	if(hasMoreStatus == "ready") {
		return true;
	} else if (hasMoreStatus == "waiting") {
		Sleep(1000);
		if (attempts > 10){
			return false;
		} else {
			return hasMoreUpdatedOrders(attempts +1);
		}
	} else {
		utils.c2sLog("no response");
		return false;
	}
}

json C2SProxy::nextOrder(){	
	string key = getPrivateKey();
	string path = "/pull/orders/next";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
//	utils.c2sLog("reponse hasMoreUpdatedOrders: %s\n", response.c_str());
	
	json orderDefault = json({});
	json orderValue;
	if(response == "") {
		utils.c2sLog("no response");
		return orderValue;
	} else {
		json jsonresponse;
		jsonresponse = json::parse(response);
		try{
			orderValue = jsonresponse.value("order", orderDefault);	
		}
		catch (json::exception& e) {
			// output exception information
			utils.c2sLog("Error reading Order");
			std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
		}
		return orderValue;
	}
}

bool pullProductTransactionCompleted(){
	string key = getPrivateKey();
	string path = "/pull/products/readyfornext";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	json jsonresponse;
	jsonresponse = json::parse(response);
	bool ready = jsonresponse.value("ready", false);
	if(ready == true) {
		return true;
	} else {
		return false;
	}
}

char* C2SProxy::sendProductPullSuccessNotification(string id, json productstored, bool succeded, string error){
	
	string key = getPrivateKey();
	string verb = "/pull/products/";
	string endpoint = (succeded == true) ? "/succeded" : "/failed";
	string path = verb + id + endpoint;
	
	json productsToUpload;
	string productUploadParsed;
	if(succeded == true){
		productsToUpload["product"] = productstored;
		productUploadParsed = productsToUpload.dump();	
//		utils.c2sLog("-!-!- productUploadParsed json to send: " + productUploadParsed);
	} else {
		productsToUpload["error"] = "could not pull product";
		json reason;
		reason["message"] = error;
		reason["code"] = 400;
		productsToUpload["reasons"] = json::array({reason});
		productUploadParsed = productsToUpload.dump();	
		utils.c2sLog("Error order: " + productUploadParsed);
	}
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, productUploadParsed);
	while(pullProductTransactionCompleted() != true){
		Sleep(1000);
	}
	return (char*) "success";
}

bool pullOrderTransactionCompleted(){
	string key = getPrivateKey();
	string path = "/pull/orders/readyfornext";
	string fullpath = makePath(key, path);
	string response = globalGet(fullpath);
	json jsonresponse;
	jsonresponse = json::parse(response);
	bool ready = jsonresponse.value("ready", false);
	if(ready == true) {
		return true;
	} else {
		return false;
	}
}

char* C2SProxy::sendOrderPullSuccessNotification(string id, json orderstored, bool succeded, string error){
	
	string key = getPrivateKey();
	string verb = "/pull/orders/";
	string endpoint = (succeded == true) ? "/succeded" : "/failed";
	string path = verb + id + endpoint;
	
	json ordersToUpload;
	string orderUploadParsed;
	if(succeded == true){
		ordersToUpload["order"] = orderstored;
		orderUploadParsed = ordersToUpload.dump();
	} else {
		ordersToUpload["error"] = "could not pull order";
		json reason;
		reason["message"] = error;
		reason["code"] = 400;
		ordersToUpload["reasons"] = json::array({reason});
		orderUploadParsed = ordersToUpload.dump();	
		utils.c2sLog("--Error order: " + orderUploadParsed);
	}
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, orderUploadParsed);
	while(pullOrderTransactionCompleted() != true){
		Sleep(1000);
	}
	return (char*) "success";
}
	

char* C2SProxy::setFinishProductUpload(char* strategy){
	
	flushProductsToUpstream(strategy);
	
	string post="{}";
	string key = getPrivateKey();
	string verb = ((string)strategy == "pingsample") ? "/pingsample" : "/push";
	string endpoint = "/products/finish";
	string path = verb + endpoint;
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, post);
	return (char*)"success";
}

char* C2SProxy::setFinishOrderUpload(char* strategy){
	
	flushOrdersToUpstream(strategy);
	
	string post="{}";
	string key = getPrivateKey();
	string verb = ((string)strategy == "pingsample") ? "/pingsample" : "/push";
	utils.c2sLog("verb: " + verb);
	string endpoint = "/orders/finish";
	string path = verb + endpoint;
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, post);
	return (char*)"success";
}

char* C2SProxy::setFinishUpload(char* strategy){
	
	string post = "{}";				
	string key = getPrivateKey();
	string verb = ((string)strategy == "pingsample") ? "/pingsample" : "/push";
	string endpoint = "/finish";
	string path = verb + endpoint;
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, post);
	return (char*)"success";
	
}


char* C2SProxy::setFinishProductDownload(){

	string post = "{}";
	string key = getPrivateKey();
	string path = "/pull/products/finish";
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, post);
	return (char*)"success";
	
}

char* C2SProxy::setFinishOrderDownload(){

	string post = "{}";
	string key = getPrivateKey();
	string path = "/pull/orders/finish";
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, post);
	return (char*)"success";
	
}

char* C2SProxy::setFinishDownload(){

	string post = "{}";
	string key = getPrivateKey();
	string path = "/pull/finish";
	string fullpath = makePath(key, path);
	string res = globalPost(fullpath, post);
	return (char*)"success";
	
}

string makePath(string privatekey, string path) {
	std::string fullpath;
	fullpath = privatekey;
	fullpath += path;
	return fullpath;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
    
}

string globalGet(string path) {
	
	string url;
	url = utils.getStringFromConfig("click2syncendpoint");
	url += "/api/adapters/custom/reverse/connection/";
	url += path;
	const char* charUrl = url.c_str();
	string logmsg = "get: ";
	logmsg.append(charUrl); 
	utils.c2sLog(logmsg);
	
	CURL *curl;
	CURLcode res;
	string readBuffer;
	
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, charUrl);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_setopt(curl, CURLOPT_CAINFO, utils.getStringFromConfig("opensslcacertpath").c_str());
		
		CURLcode res;
		res = curl_easy_perform(curl);
		if(res == CURLE_HTTP_RETURNED_ERROR) {
			utils.c2sSleep(5);
			res = curl_easy_perform(curl);
			if(res == CURLE_HTTP_RETURNED_ERROR) {
				utils.c2sLog("res error: " + readBuffer);
				readBuffer = "";
			}
		}
		curl_easy_cleanup(curl);
	}
	utils.c2sLog("res: " + readBuffer);
	return readBuffer;
	
}

string globalPost(string path, string data) {

	struct curl_slist *headers=NULL;
	string url;
	url = utils.getStringFromConfig("click2syncendpoint");
	url += "/api/adapters/custom/reverse/connection/";
	url += path;
	const char* charUrl = url.c_str();
	string logmsg = "post: ";
	logmsg.append(charUrl);
	utils.c2sLog(logmsg);
	CURL *curl;
	CURLcode res;
	string readBuffer;
	
	curl = curl_easy_init();
	if(curl) {
		headers = curl_slist_append(headers, "Accept: application/json");  
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_URL, charUrl);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_setopt(curl, CURLOPT_CAINFO, utils.getStringFromConfig("opensslcacertpath").c_str());
		
		CURLcode res = curl_easy_perform(curl);
		
		if(res != CURLE_OK){
            utils.c2sLog("Error with Post");
		} else {
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			utils.c2sLog("res: " + readBuffer);
		}
	}

	return readBuffer;
	
}

string parseStringToJson(string stringtoparse, string valuetoobtain, string defaultvalue) {
	try {
		json jsonresponse;
		jsonresponse = json::parse(stringtoparse);	
		string value = jsonresponse.value(valuetoobtain, "");
		return value;
	} catch (json::parse_error& e) {
		utils.c2sLog("Error parsing reponse from click2sync");
		return (string) "";
	}
}

string getPrivateKey() {
	
	json privatekeyjson;
	string privatekeystr;
	ifstream in_stream;
    in_stream.open(utils.getStringFromConfig("privatekeypath").c_str());
    if (in_stream.fail())
    {
        utils.c2sLog("Could not open file to read.");
        ofstream fout;
		fout.open(utils.getStringFromConfig("privatekeypath").c_str());
		if(fout.good()) {
			privatekeystr = genKey();
			privatekeyjson = {
				{"key", privatekeystr}
			};
			string jsonstring = privatekeyjson.dump();
			fout << jsonstring << std::endl;
		}
		fout.close();
    } else {
    	std::ifstream privatekeyfile(utils.getStringFromConfig("privatekeypath").c_str());
		try {
	    	privatekeyfile >> privatekeyjson;
	    	privatekeystr = privatekeyjson.value("key", "0");
		}
		catch (json::exception& e) {
			// output exception information
			utils.c2sLog("Error reading private key");
			//std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
		}
	}
	return privatekeystr;
	
}

string genKey() {
	string possible_characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    random_device rd;
    mt19937 engine(rd());
    uniform_int_distribution<> dist(0, possible_characters.size()-1);
    string ret = "";
    for(int i = 0; i < 256; i++){
        int random_index = dist(engine); //get index between 0 and possible_characters.size()-1
        ret += possible_characters[random_index];
    }
    return ret;
}
