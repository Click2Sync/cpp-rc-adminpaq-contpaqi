#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdio>

#include <stdio.h>
#include <curl/curl.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "c2sproxy.h"
#include "contpaqiproxy.h"
#include "c2sutils.h"


#include <tchar.h>
#include <nlohmann/json.hpp>

SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
SC_HANDLE             schSCManager = NULL;
SC_HANDLE             schService = NULL;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);

HANDLE hThread;

using namespace std;
using json = nlohmann::json;

const int longpauseMillis = 10000;
const int normalpauseMillis = 1000;
//commit test
//define functions used in main
bool senseC2SReachability();
bool senseContpaQiReachability();
char* work();
void pauseWork(bool longPause);
void abstractLoop();
void loop();
void fetchRemote(C2SProxy c2s, ContpaqiProxy contpaqi);
void pullToRemote(C2SProxy c2s,  ContpaqiProxy contpaqi);

C2SProxy c2sproxy;
ContpaqiProxy contpaqi;
static C2SUtils utils;
char* SERVICE_NAME;

//MAIN FOR USE IN SERVICE MODE
//Windows service code
int _tmain (int argc, TCHAR *argv[]) {
	
	utils.loadConfigValues();
	
	if(utils.getStringFromConfig("executionmode") == "service"){
		
		SERVICE_NAME = (char *)utils.getStringFromConfig("servicename").c_str();
		
		OutputDebugString(_T("Click2Sync Service: Main: Entry"));

	    SERVICE_TABLE_ENTRY ServiceTable[] = {
	        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
	        {NULL, NULL}
	    };
	
	    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE) {
	       OutputDebugString(_T("Click2Sync Service: Main: StartServiceCtrlDispatcher returned error"));
	       return GetLastError ();
	    }
	
	    OutputDebugString(_T("Click2Sync Service: Main: Exit"));
	    return 0;
		
	}else{
		
		if(argc > 1) {//debugging
			utils.c2sSleep(longpauseMillis);
		}else {//normal service run
			while(true){
				abstractLoop();
			}
		}
		return EXIT_SUCCESS;
		
	}
    
}


VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv) {
    DWORD Status = E_FAIL;

    OutputDebugString(_T("Click2Sync Service: ServiceMain: Entry"));

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL)
    {
        OutputDebugString(_T("Click2Sync Service: ServiceMain: OpenSCManager returned error"));
        goto EXIT;
    }
    
    schService = OpenService(schSCManager, SERVICE_NAME, SERVICE_STOP | 
        SERVICE_QUERY_STATUS | DELETE);
    if (schService == NULL)
    {
        OutputDebugString(_T("Click2Sync Service: ServiceMain: OpenService returned error"));
        goto EXIT;
    }
    
    g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL) {
        OutputDebugString(_T("Click2Sync Service: ServiceMain: RegisterServiceCtrlHandler returned error"));
        goto EXIT;
    }
    
    QueryServiceStatus(schService, &g_ServiceStatus);

    // Tell the service controller we are starting
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) {
        OutputDebugString(_T("Click2Sync Service: ServiceMain: SetServiceStatus returned error"));
    }

    /* 
     * Perform tasks neccesary to start the service here
     */
    OutputDebugString(_T("Click2Sync Service: ServiceMain: Performing Service Start Operations"));

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) {
        OutputDebugString(_T("Click2Sync Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error"));

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) {
            OutputDebugString(_T("Click2Sync Service: ServiceMain: SetServiceStatus returned error"));
        }
        goto EXIT; 
    }    

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) {
        OutputDebugString(_T("Click2Sync Service: ServiceMain: SetServiceStatus returned error"));
    }

    // Start the thread that will perform the main task of the service
    hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    OutputDebugString(_T("Click2Sync Service: ServiceMain: Waiting for Worker Thread to complete"));

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject (hThread, INFINITE);

    OutputDebugString(_T("Click2Sync Service: ServiceMain: Worker Thread Stop Event signaled"));


    /* 
     * Perform any cleanup tasks
     */
    OutputDebugString(_T("Click2Sync Service: ServiceMain: Performing Cleanup Operations"));

    CloseHandle (g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) {
        OutputDebugString(_T("Click2Sync Service: ServiceMain: SetServiceStatus returned error"));
    }

    EXIT:
    OutputDebugString(_T("Click2Sync Service: ServiceMain: Exit"));

    return;
}


VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode) {
    OutputDebugString(_T("Click2Sync Service: ServiceCtrlHandler: Entry"));

    switch (CtrlCode) {
     case SERVICE_CONTROL_STOP :

        OutputDebugString(_T("Click2Sync Service: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request"));

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
           break;

        /* 
         * Perform tasks neccesary to stop the service here 
         */

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) {
            OutputDebugString(_T("Click2Sync Service: ServiceCtrlHandler: SetServiceStatus returned error"));
        }

        // This will signal the worker thread to start shutting down
        SetEvent (g_ServiceStopEvent);

        break;

     default:
         break;
    }

    OutputDebugString(_T("Click2Sync Service: ServiceCtrlHandler: Exit"));
}

bool senseC2SReachability() {
	return c2sproxy.sense();
}

bool sensecontpaqiReachability() {
	return contpaqi.sense();
}

DWORD WINAPI ServiceWorkerThread (LPVOID lpParam) {
    OutputDebugString(_T("Click2Sync Service: ServiceWorkerThread: Entry"));
    int i = 0;
    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0) {
        bool senseC2S = senseC2SReachability();	
		bool senseContpaQi = sensecontpaqiReachability();
		if(senseC2S){
			loop();
		} else {
			pauseWork(true);
		}
    }

    OutputDebugString(_T("Click2Sync Service: ServiceWorkerThread: Exit"));

    return ERROR_SUCCESS;
}

//C2S code

char* work() {
	try {
		if(c2sproxy.checkIfFetchingRemote()) {
		fetchRemote(c2sproxy, contpaqi);
		}
		//check if we are pulling to remote
		if(c2sproxy.checkIfPullingToRemote()) {
			pullToRemote(c2sproxy, contpaqi);
		}
		return (char*) "work() success";
	} catch (int ){
		return (char*) "";
	}
}

int cursorOffset() {
	int cursoroffset = c2sproxy.getCursorOffset();
	std::cout << cursoroffset << std::endl;
	return cursoroffset;
}

void abstractLoop() {
	bool senseC2S = senseC2SReachability();	
	bool senseContpaQi = sensecontpaqiReachability();
	if(senseC2S){
		while(senseC2S) {
			loop();
		}
	} else {
		pauseWork(true);
		return;
	}
}

void loop() {
	//int getcursoroffset = cursorOffset();
	char* worked = work();
	if(strlen(worked) == 0){
		utils.c2sLog("Error on work()");
		pauseWork(true);
	}else{
		pauseWork(true);
	}
	return;
	
}

void fetchRemote(C2SProxy c2s, ContpaqiProxy contpaqi) {
	boolean uploadproducts = true;
	boolean uploadorders = true;
	
	string strategy = c2s.getStrategy();
	string upstreamstatus = c2s.getUpstreamStatus();
	string entity = c2s.getEntity();
	int offsetdate = c2s.getCursorOffset();
	
	if(upstreamstatus != "" && upstreamstatus == "waiting") {
		//if is waiting, good, change to initialized
		c2s.setInitializeUpload((char*)strategy.c_str());
		upstreamstatus = c2s.getUpstreamStatus();
	}
	if(upstreamstatus != "" && upstreamstatus == "initialized") {
		//if is initialized, check cursor
		//from cursor start upload
		//when packet uploaded, renew offsets
		if(entity != "" && entity == "products") {
			if(uploadproducts) {
				if(strategy != "" && strategy == "pingsample") {
					contpaqi.setProductsStreamCursor(offsetdate);
					if(contpaqi.hasMoreProducts(offsetdate)){
						json product = contpaqi.nextProduct();
						c2s.setProductToUploadOnBuffer(product, (char*)strategy.c_str());
					}
				} else {
					contpaqi.setProductsStreamCursor(offsetdate);
					while(contpaqi.hasMoreProducts(offsetdate)){
						json product = contpaqi.nextProduct();
						if(!product.empty()) {
							c2s.setProductToUploadOnBuffer(product, (char*)strategy.c_str());	
						}
					}
				}
			}
			c2s.setFinishProductUpload((char*)strategy.c_str());
			offsetdate = c2s.getCursorOffset();
			entity = c2s.getEntity();
		}
		if(entity != "" && entity == "orders") {
			if(uploadorders) {
				if(strategy != "" && strategy == "pingsample") {
					contpaqi.setOrdersStreamCursor(offsetdate);
					if(contpaqi.hasMoreOrders(offsetdate)) {
						json order = contpaqi.nextOrder();
						c2s.setOrderToUploadOnBuffer(order, (char*)strategy.c_str());
					}
				} else {
					contpaqi.setOrdersStreamCursor(offsetdate);
					while(contpaqi.hasMoreOrders(offsetdate)) {
						json order = contpaqi.nextOrder();
						if(!order.empty()) {
							c2s.setOrderToUploadOnBuffer(order, (char*)strategy.c_str());	
						}
					}
				}
			}
			c2s.setFinishOrderUpload((char*)strategy.c_str());
			upstreamstatus = c2s.getCursorOffset();
		}
		c2s.setFinishUpload((char*)strategy.c_str());
		//upstreamstatus = (int)c2s.getUpStreamStatus();
	}
	if(upstreamstatus == "" && (upstreamstatus != "finished" || upstreamstatus != "finishing")) {
		utils.c2sLog("Unknown upstreamstatus: " + upstreamstatus);
	}
	pauseWork(true);
}

void pullToRemote(C2SProxy c2s, ContpaqiProxy contpaqi) {
	bool downloadproducts = true;
	bool downloadorders = true;
	bool err = false;
	int resetDocumentFolio;
	string upstreamstatus = c2s.getUpstreamStatus();
	string entity = c2s.getEntity();
	
	if(upstreamstatus == "waiting") {
		//if is waiting, good, change to initialized
		c2s.setInitializeDownload();
		upstreamstatus = c2s.getUpstreamStatus();
		//(if webonline responds error, then notify of strange behavior to server)
	}
	if(upstreamstatus == "initialized") {
		//if is initialized, check cursor
		//from cursor start upload
		//when packet uploaded, renew offsets
		if(entity == "products") {
			if(downloadproducts) {
				while(c2s.hasMoreUpdatedProducts(1)) {
					err = false;
					json product = c2s.nextProduct();

					if(!product.empty()){
						string sku;
						try{
							sku = product.value("sku", "");	
						}
						catch (json::exception& e) {
							utils.c2sLog("could not read product sku");
							err = true;
						}
						
						if(!err){
							json productstored = contpaqi.storeProduct(product);
							if(!productstored.empty()) {
								c2s.sendProductPullSuccessNotification(sku, productstored, true, "");
							}
						}		
					} else {
						utils.c2sLog("could not read product");
						pauseWork(false);
					}
				}
			}
			c2s.setFinishProductDownload();
			entity = c2s.getEntity();
			resetDocumentFolio = contpaqi.resetFolio();
		}
		if(entity == "orders") {
			if(downloadorders) {
				while(c2s.hasMoreUpdatedOrders(1)) {
					err = false;
					json order = c2s.nextOrder();
					
					if(!order.empty()){
						string id;
						try{
							id = order.value("orderid", "");
						}
						catch (json::exception& e) {
							utils.c2sLog("could not read order id");
							err = true;
						}
						
						if(!err){
							json orderstored = contpaqi.storeOrder(order);
							if(!orderstored.empty()) {
								c2s.sendOrderPullSuccessNotification(id, orderstored, true, "");
							}
						}
					} else {
						utils.c2sLog("could not read order");	
						pauseWork(false);
					}					
				}
			}
			c2s.setFinishOrderDownload();
			resetDocumentFolio = contpaqi.resetFolio();
		}
		c2s.setFinishDownload();
		upstreamstatus = c2s.getUpstreamStatus();
	}
	if(upstreamstatus != "finished" || upstreamstatus != "finishing") {
		utils.c2sLog("Unknown upstreamstatus: " + upstreamstatus);
	} else {
		
	}
}

void pauseWork(bool longPause) {

	if(longPause) {
		utils.c2sSleep(longpauseMillis);
	} else {
		utils.c2sSleep(normalpauseMillis);
	}

}

