#include <fstream>
#include <ctime>
#include <stdio.h>
#include <iomanip>
#include <stdlib.h>
#include "adminpaq.h"
#include "c2sutils.h"
#include <tchar.h>
#include <Windows.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <tinyutf8.h>
#include <sstream>
#pragma execution_character_set("utf-8")
#include "MGW_SDK.h"

using namespace std;
using json = nlohmann::json;

HINSTANCE hGetProcIDDLL;
static C2SUtils utils;

bool Adminpaq::fLoadDll() {
	
	hGetProcIDDLL = LoadLibrary(_T(utils.getStringFromConfig("contpaqidllfile").c_str()));

	if(NULL == hGetProcIDDLL) {
		DWORD dwError = 0;
		dwError = GetLastError();
		wchar_t buf[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			buf, (sizeof(buf) / sizeof(wchar_t)), NULL);
		//printf("%ls\n", buf);
		utils.c2sLog("Error loading dll");
		return false;
	} else {
		return true;
	}
}

int Adminpaq::fInicializaSDK() {
	int err = (int)GetProcAddress(hGetProcIDDLL, "fInicializaSDK")();
	if(err) {
		utils.c2sLog("Error on fInicializaSDK()");
	}
	return err;
}

int Adminpaq::fTerminaSDK() {
	int err = (int)GetProcAddress(hGetProcIDDLL, "fTerminaSDK")();
	return err;
}

int Adminpaq::fAbreEmpresa(char* ruta){
	typedef int (__stdcall *_fAbreEmpresa)(char* x);
	_fAbreEmpresa abreEmpresa = (_fAbreEmpresa)GetProcAddress(hGetProcIDDLL, "fAbreEmpresa");
	int err = abreEmpresa(ruta);
	if(err) {
		utils.c2sLog("Error on fAbreEmpresa()");
	}
	return err;
}

int Adminpaq::fCierraEmpresa() {
	int err = (int)GetProcAddress(hGetProcIDDLL, "fCierraEmpresa")();
	return err;
}

int Adminpaq::fBuscaIdProducto(int id){
	typedef int (__stdcall *_fBuscaIdProducto)(int x);
	_fBuscaIdProducto buscaIdProducto = (_fBuscaIdProducto)GetProcAddress(hGetProcIDDLL, "fBuscaIdProducto");
	int err = buscaIdProducto(id);
	if(err) {
		utils.c2sLog("Error on fBuscaIdProducto()");
	}
	return err;
}

int Adminpaq::fPosPrimerProducto(){
	int err = (int)GetProcAddress(hGetProcIDDLL, "fPosPrimerProducto")();
	if(err) {
		utils.c2sLog("Error on fPosPrimerProducto()");
	}
	return err;
}

int Adminpaq::fPosSiguienteProducto(){
	int err = (int)GetProcAddress(hGetProcIDDLL, "fPosSiguienteProducto")();
	if(err) {
		utils.c2sLog("Error on fPosSiguienteProducto()");
	}
	if(!fPosEOFProducto()) {
		//int err = (int)GetProcAddress(hGetProcIDDLL, "fPosSiguienteProducto")();	
		return 0;
	} else {
		return 1;
	}
}

int Adminpaq::fPosEOFProducto() {
	int iseof = (int)GetProcAddress(hGetProcIDDLL, "fPosEOFProducto")();
	return iseof;
}

int Adminpaq::fLeeDatoProducto(char* id, char* valor, int len){
	typedef int (__stdcall *_fLeeDatoProducto)(char* x, char *y, int z);
	_fLeeDatoProducto leeProducto = (_fLeeDatoProducto)GetProcAddress(hGetProcIDDLL, "fLeeDatoProducto");	
	int err = leeProducto(id, valor, len);
	if(err) {
		utils.c2sLog("Error on fLeeDatoProducto()");
	}
	return err;
}

static utf8_string convertAnsiToUtf8(char* ansi){

	utf8_string elchido = utf8_string("");
	for(int i=0; i<strlen(ansi); i++){
		char elbyte = ansi[i];
		uint8_t asd = elbyte;
		if(asd <= 127){
			elchido += utf8_string(elbyte);
		}else{
			elchido += utf8_string(asd);
		}
	}
	return elchido;
}

int Adminpaq::fPosPrimerDocumento(){
	int err = (int)GetProcAddress(hGetProcIDDLL, "fPosPrimerDocumento")();
	if(err) {
		utils.c2sLog("Error on fPosPrimerDocumento()");
	}
	return err;
}

int Adminpaq::fPosSiguienteDocumento(){
	int err = (int)GetProcAddress(hGetProcIDDLL, "fPosSiguienteDocumento")();
	return err;
}

int Adminpaq::fPosUltimoDocumento(){
	int err = (int)GetProcAddress(hGetProcIDDLL, "fPosUltimoDocumento")();
	return err;
}

int Adminpaq::fPosDocumentoEOF(){
	int eof = (int)GetProcAddress(hGetProcIDDLL, "fPosEOF")();
	return eof;
}

int Adminpaq::fLeeDatoDocumento(char* id, char* valor, int len){
	typedef int (__stdcall *_fLeeDatoDocumento)(char* x, char *y, int z);
	_fLeeDatoDocumento leeDocumento = (_fLeeDatoDocumento)GetProcAddress(hGetProcIDDLL, "fLeeDatoDocumento");
	int err = leeDocumento(id, valor, len);
	if(err) {
		utils.c2sLog("Error on fLeeDatoDocumento()");
	}
	return err;
}

int Adminpaq::fSetFiltroMovimiento(int id){
	typedef int (__stdcall *_fSetFiltroMovimiento)(int x);
	_fSetFiltroMovimiento setFiltroMovimiento = (_fSetFiltroMovimiento)GetProcAddress(hGetProcIDDLL, "fSetFiltroMovimiento");
	int err = setFiltroMovimiento(id);
	if(err) {
		utils.c2sLog("Error on fSetFiltroMovimiento()");
	}
	return err;
}

int Adminpaq::fCancelaFiltroMovimiento(){
	int err = (int)GetProcAddress(hGetProcIDDLL, "fCancelaFiltroMovimiento")();
	if(err) {
		utils.c2sLog("Error on fCancelaFiltroMovimiento()");
	}
	return err;
}

int Adminpaq::fPosSiguienteMovimiento(){
	int err = (int)GetProcAddress(hGetProcIDDLL, "fPosSiguienteMovimiento")();
	return err;
}

int Adminpaq::fPosMovimientoEOF(){
	int eof = (int)GetProcAddress(hGetProcIDDLL, "fPosMovimientoEOF")();
	return eof;
}

int Adminpaq::fPosUltimoMovimiento(){
	int err = (int)GetProcAddress(hGetProcIDDLL, "fPosUltimoMovimiento")();
	if(err) {
		utils.c2sLog("Error on fPosUltimoMovimiento()");
	}
	return err;
}

int Adminpaq::fLeeDatoMovimiento(char* id, char* valorMov, int len){
	typedef int (__stdcall *_fLeeDatoMovimiento)(char* x, char *y, int z);
	_fLeeDatoMovimiento leeMovimiento = (_fLeeDatoMovimiento)GetProcAddress(hGetProcIDDLL, "fLeeDatoMovimiento");
	int err = leeMovimiento(id, valorMov, len);
	if(err) {
		utils.c2sLog("Error on fLeeDatoMovimiento()");
	}
	return err;
}

int Adminpaq::fBuscaIdCteProv(int id){
	typedef int (__stdcall *_fBuscaIdCteProv)(int x);
	_fBuscaIdCteProv buscaIdCteProv = (_fBuscaIdCteProv)GetProcAddress(hGetProcIDDLL, "fBuscaIdCteProv");
	int err = buscaIdCteProv(id);
	if(err) {
		utils.c2sLog("Error on fBuscaIdCteProv()\n");
	}
	return err;
}

int Adminpaq::fLeeDatoCteProv(char* id, char* valor, int len){
	typedef int (__stdcall *_fLeeDatoCteProv)(char* x, char *y, int z);
	_fLeeDatoCteProv leeDatoCteProv = (_fLeeDatoCteProv)GetProcAddress(hGetProcIDDLL, "fLeeDatoCteProv");
	int err = leeDatoCteProv(id, valor, len);
	if(err) {
		utils.c2sLog("Error on fLeeDatoCteProv()\n");
	}
	return err;
}

int Adminpaq::fRegresaExistencia(char* idprod, char* idalmacen, char* anio, char* mes, char* dia, double* aExistencia){
	typedef int (__stdcall *_fRegresaExistencia)(char* a, char* b, char* c, char* d, char* e, double* f);
	_fRegresaExistencia regresaExistencia = (_fRegresaExistencia)GetProcAddress(hGetProcIDDLL, "fRegresaExistencia");
	int err = regresaExistencia(idprod, idalmacen, anio, mes, dia, aExistencia);
	if(err) {
		utils.c2sLog("Error on fRegresaExistencia()\n");
	}
	return err;
}

json Adminpaq::convertContpaqiProductToC2SProduct() {
	
	json product;
	json variation;
	json colorsize;
	
	char cCodigoProducto[50];
	char* pCodigoProducto = cCodigoProducto;
	int errCodigoProducto = fLeeDatoProducto((char*)"cCodigoProducto", pCodigoProducto, 50);
	utf8_string sCodigoProducto = convertAnsiToUtf8(cCodigoProducto);
	if(sCodigoProducto.empty()) {
		return product;
	} else {
		char cNombreProducto[50];
		char* pNombreProducto = cNombreProducto;
		int errNombreProducto = fLeeDatoProducto((char*)"cNombreProducto", pNombreProducto, 50);
	    utf8_string sNombreProducto = convertAnsiToUtf8(cNombreProducto);
		
		//This is used for variation size and color
		char cDescripcionProducto[50];
		char* pDescripcionProducto = cDescripcionProducto;
		int errDescripcionProducto = fLeeDatoProducto((char*)"CTEXTOEX01", pDescripcionProducto, 50);
		utf8_string sDescripcionProducto = convertAnsiToUtf8(cDescripcionProducto);
		string descripcionarr(cDescripcionProducto);
		
		if(descripcionarr == "") {
			descripcionarr = "[]";
		}
		try{
			colorsize = json::parse(descripcionarr);
		} catch (json::exception& e) {
			utils.c2sLog("Error parsing description to retrieve array");
		}
		
		
		char cTimestamp[50]; //used in last updated
		char* pTimestamp = cTimestamp;
		int errTimestamp = fLeeDatoProducto((char*)"CTIMESTAMP", pTimestamp, 50);
	
		char cPrecio1[50];
		char* pPrecio1 = cPrecio1;
		int errPrecio1 = fLeeDatoProducto((char*)"cPrecio1", pPrecio1, 50);
		
		char cControlExistencia[50];
		char* pControlExistencia = cControlExistencia;
		int errControlExistencia = fLeeDatoProducto((char*)"cControlExistencia", pControlExistencia, 50);
		
		product["_id"] = sCodigoProducto.cpp_str();
		product["sku"] = sCodigoProducto.cpp_str();
		product["title"] = sNombreProducto.cpp_str();
		product["last_updated"] = 1517360038797;
		/* TODO (Daniel#1#): Hacer que este dato no este hardcodeado 
		                     terminando la funcion de humantime to 
		                     epoch */
		
		//product["descripcion"] = sDescripcionProducto.c_str();
		
		
		double existencia;
		std::time_t t = std::time(nullptr);
    	std::tm currentdate = *std::localtime(&t);
		string year = to_string(currentdate.tm_year+1900);
		string month = to_string(currentdate.tm_mon+1);
		string day = to_string(currentdate.tm_mday);
		int errRegresaExistencia = fRegresaExistencia(pCodigoProducto, (char*)"1", (char*)year.c_str(), (char*)month.c_str(), (char*)day.c_str(), &existencia);
		
		variation["availabilities"].push_back({
			{"tag", "default"},
			{"quantity", existencia}
		});
		variation["prices"].push_back({
			{"tag", "default"},
			{"currency", "MXN"},
			{"number", pPrecio1}
		});
		try{
			if(colorsize.size() > 0) {
				string color;
				color = colorsize.at(0);
				utils.c2sLog("color: " + color);
				variation["color"] = color;
			}
		} catch (json::exception& e) {
			utils.c2sLog("Error retrieving color from description");
		}
		try{
			if(colorsize.size() > 0) {
				string size;
				size = colorsize.at(1);
				utils.c2sLog("size: " + size);
				variation["size"] = size;
			}
		} catch (json::exception& e) {
			utils.c2sLog("Error retrieving size from description");
		}
		
		product["variations"].push_back(variation);
		
		return product;
	}
};

json Adminpaq::convertContpaqiOrderToC2SOrder() {
	json order;
	int base = 10;
	char *end;
	
	//id
	char cIdDocum01[50];
	char* pIdDocum01 = cIdDocum01;
	int errIdDocum01 = fLeeDatoDocumento((char*)"cIdDocum01", pIdDocum01, 50);
	
	//Concepto
	char cConcepto[50];
	char* pConcepto = cConcepto;
	int errConcepto = fLeeDatoDocumento((char*)"cIdConce01", pConcepto, 50);
	
	long int iddoclong = strtol(cIdDocum01, &end, base);
	long int conceptolong = strtol(cConcepto, &end, base);
	
	fSetFiltroMovimiento(iddoclong);
	bool hasNoMovs = fPosMovimientoEOF();
	
	if(hasNoMovs || conceptolong != 4) {
		return order;
	} else {
		//Folio
		char cFolio[50];
		char* pFolio = cFolio;
		int errFolio = fLeeDatoDocumento((char*)"cFolio", pFolio, 50);
		long int foliolong = strtol(cFolio, &end, base);
		
		//lastupdated
		char cFechaUl01[50];
		char* pFechaUl01 = cFechaUl01;
		int errFechaUl01 = fLeeDatoDocumento((char*)"cFechaUl01", pFechaUl01, 50);
		
		//total
		char cTotal[50];
		char* pTotal = cTotal;
		int errTotal = fLeeDatoDocumento((char*)"cTotal", pTotal, 50);
		
		//get lines of order(movimientos)
		while(!fPosMovimientoEOF()) {
			//get codprod of given id
			char cIdProdu01[50];
			char* pIdProdu01 = cIdProdu01;
			int errIdProdu01 = fLeeDatoMovimiento((char*)"cIdProdu01", pIdProdu01, 50);
			
			long int idprodlong = strtol(cIdProdu01, &end, base);
			fBuscaIdProducto(idprodlong);
			char cCodigoP01[50];
			char* pCodigoP01 = cCodigoP01;
			int errCodigoP01 = fLeeDatoProducto((char*)"cCodigoP01", pCodigoP01, 50);
			utf8_string sCodigoP01 = convertAnsiToUtf8(cCodigoP01);
			
			char cPrecio[50];
			char* pPrecio = cPrecio;
			int errPrecio = fLeeDatoMovimiento((char*)"cPrecio", pPrecio, 50);
			
			char cUnidades[50];
			char* pUnidades = cUnidades;
			int errUnidades = fLeeDatoMovimiento((char*)"cUnidades", pUnidades, 50);
			
			order["orderItems"].push_back({
				{"id", sCodigoP01.cpp_str()},
				{"variation_id", "0"},
				{"quantity", cUnidades},
				{"unitPrice", cPrecio},
				{"currencyId", "MXN"}
			});
			fPosSiguienteMovimiento();
		};
		fCancelaFiltroMovimiento();
		
		//get buyer info
		char cIdClien01[50];
		char* pIdClien01 = cIdClien01;
		int errIdClien01 = fLeeDatoDocumento((char*)"cIdClien01", pIdClien01, 50);
		long int idclientlong = strtol(cIdClien01, &end, base);
		
		fBuscaIdCteProv((int) idclientlong);
		
		char cCodigoC01[50];
		char* pCodigoC01 = cCodigoC01;
		int errCodigoC01 = fLeeDatoCteProv((char*)"cCodigoC01", pCodigoC01, 50);
		utf8_string sCodigoC01 = convertAnsiToUtf8(cCodigoC01);
		
		char cRazonSo01[50];
		char* pRazonSo01 = cRazonSo01;
		int errRazonSo01 = fLeeDatoCteProv((char*)"cRazonSo01", pRazonSo01, 50);
		utf8_string sRazonSo01 = convertAnsiToUtf8(cRazonSo01);
		
		char cEmail1[50];
		char* pEmail1 = cEmail1;
		int errEmail1 = fLeeDatoCteProv((char*)"cEmail1", pEmail1, 50);
		utf8_string sEmail1 = convertAnsiToUtf8(cEmail1);
		
		//build order json
		order["_id"] = foliolong;
		order["orderid"] = foliolong;
		order["last_updated"] = "1517360038797";
		order["status"] = "paid";
		order["dateCreated"] = "";
		order["dateClosed"] = "";
		order["total"] = {
			{"amount", cTotal},
			{"currency", "MXN"}
		};
		order["buyer"] = {
			{"id", sCodigoC01.cpp_str()},
			{"email", sEmail1.cpp_str()},
			{"phone", ""},
			{"firstName", sRazonSo01.cpp_str()},
			{"lastName", ""}
		};
		
		string orderstring = order.dump();
		
		return order;	
	}
};

void humanToEpochTime(){
	/* TODO (Daniel#1#): Terminar para que no este harcodeado esto */
	
	struct tm t;
    time_t t_of_day;

    t.tm_year = 2011-1900;
    t.tm_mon = 7;           // Month, 0 - jan
    t.tm_mday = 8;          // Day of the month
    t.tm_hour = 16;
    t.tm_min = 11;
    t.tm_sec = 42;
    t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
    t_of_day = mktime(&t);

    //utils.c2sLog("seconds since the Epoch: %ld\n", (long) t_of_day);
};	

void epochToHumanTime(char* humantime, long long int orderDate){
	time_t now = orderDate*0.001;
    struct tm ts;
    char buf[80];
    
    if(now == 0){
		time(&now);
	}

    ts = *localtime(&now);
    strftime(buf, sizeof(buf), "%m/%d/%Y", &ts);
	strcpy(humantime, buf);
};

long Adminpaq::fAltaDocumento(long long int orderDate, long int newFolio, char* codConcepto){
	//codConcepto = 4 for Orden de venta
	//codConcepto = 34 for Entrada al Almacen
	
	int lResult;
	REGDOCUMENTO lstDocto;
	
	char *humantime = (char*) malloc(80);
	epochToHumanTime(humantime, orderDate);
	//utils.c2sLog("order Time: %s\n", humantime);
		
	strcpy(lstDocto.aCodConcepto, codConcepto);
	strcpy(lstDocto.aSerie, "");
	lstDocto.aFolio = newFolio;
	strcpy(lstDocto.aFecha, humantime);
	strcpy(lstDocto.aCodigoCteProv, "0001"); 
	strcpy(lstDocto.aCodigoAgente, "");
	lstDocto.aNumMoneda = (int)1;
	lstDocto.aTipoCambio = (double)0;
	lstDocto.aImporte = (double)0;
	strcpy(lstDocto.aReferencia, "");
	lstDocto.aDescuentoDoc1 = (double)0;
	lstDocto.aDescuentoDoc2 = (double)0;
	lstDocto.aSistemaOrigen = 6;
	lstDocto.aAfecta = true;
	lstDocto.aGasto1 = (double)0;
	lstDocto.aGasto1 = (double)0;
	lstDocto.aGasto1 = (double)0;
	
	typedef int (__stdcall *_fAltaDocumento)(long* x, REGDOCUMENTO* astDocumento);
	_fAltaDocumento fAltaDocumento = (_fAltaDocumento)GetProcAddress(hGetProcIDDLL, "fAltaDocumento");
	
	long lid;
	lResult = fAltaDocumento(&lid, &lstDocto);
	
	free(humantime);
	return lid;
	
}

int Adminpaq::fAltaMovimiento(double quantity, double unitPrice, const char* prodId, long DocumentId){
	typedef int (__stdcall *_fAltaMovimiento)(long x, long* y, REGMOVIMIENTO* astMovimiento);
	_fAltaMovimiento altmovimiento = (_fAltaMovimiento)GetProcAddress(hGetProcIDDLL, "fAltaMovimiento");
	
//	typedef int (__stdcall *_fError)(int x, char* y, int z);
//	_fError fError = (_fError)GetProcAddress(hGetProcIDDLL, "fError");
	
	int mResultado;
	REGMOVIMIENTO lstMovimiento;
	
	lstMovimiento.aConsecutivo = (int)1;
	lstMovimiento.aUnidades = quantity;
	lstMovimiento.aPrecio = unitPrice;
	lstMovimiento.aCosto = (double)0;
	strcpy(lstMovimiento.aCodProdSer, prodId);
	strcpy(lstMovimiento.aCodAlmacen, "1");
	strcpy(lstMovimiento.aReferencia, "");
	strcpy(lstMovimiento.aCodClasificacion, "");
	
	long iddoc = DocumentId;
	long lid;
	
	mResultado = altmovimiento(iddoc, &lid, &lstMovimiento);
//	char aMensaje[3500];
//	fError(mResultado, aMensaje, 3500);
//	printf("error msg: %s \n", aMensaje);

	return mResultado;
}

int Adminpaq::GetLastFolio(char* codConcepto){
	long int UltimoFolio = 0;
	char *end;
	fPosPrimerDocumento();
	
	while(!fPosDocumentoEOF()){
		char cCodConcepto[50];
		char* pCodConcepto = cCodConcepto;
//		string strcodConcepto(pCodConcepto);
		int errCodConcepto = fLeeDatoDocumento((char*)"cIdConce01", pCodConcepto, 50);
		long int codConDoc = strtol(pCodConcepto, &end, 10);
		
		char cFolio[50];
		char* pFolio = cFolio;
		int errFolio = fLeeDatoDocumento((char*)"cFolio", pFolio, 50);
		long int numFolio = strtol(cFolio, &end, 10);
		
		if(strcmp(codConcepto, "4") == 0) {
			if(codConDoc == 4 && numFolio > UltimoFolio){
				UltimoFolio = numFolio;
			}	
		} else if(strcmp(codConcepto, "34") == 0) {
			if(codConDoc == 34 && numFolio > UltimoFolio){
				UltimoFolio = numFolio;
			}
		}
		fPosSiguienteDocumento();
		
	}
	
	return UltimoFolio;
}

int Adminpaq::fAfectaDocto(long int docFolio) {
	typedef int (__stdcall *_fAfectaDocto)(REGLLAVEDOC* astAfecta, bool y);
	_fAfectaDocto afectaDocument = (_fAfectaDocto)GetProcAddress(hGetProcIDDLL, "fAfectaDocto");
		
	int aResultado;
	REGLLAVEDOC lstLlaveDoc;
	
	strcpy(lstLlaveDoc.aCodConcepto, "4");
	strcpy(lstLlaveDoc.aSerie, "");
	lstLlaveDoc.aFolio = docFolio;

	aResultado = afectaDocument(&lstLlaveDoc, true);
	if(aResultado) {
		utils.c2sLog("Error on fAfectaDocto()");
	}
	
	return aResultado;

}

int Adminpaq::fBorraDocumento() {
	int err = (int)GetProcAddress(hGetProcIDDLL, "fBorraDocumento")();
	return err;
}

int Adminpaq::fSetDatoProducto(char* productId, char* productTitle, char* productPrice, char* productVariationData){
	
	char *humantime = (char*) malloc(80);
	epochToHumanTime(humantime, 0);
	bool err = false;
	
	int fInsertaProducto = (int)GetProcAddress(hGetProcIDDLL, "fInsertaProducto")();
	if(fInsertaProducto) {
		utils.c2sLog("error fInsertaProducto");
		err = true;
	}
		
	typedef int (__stdcall *_fSetDatoProducto)(char* x, char* y);
	_fSetDatoProducto fSetDatoProducto = (_fSetDatoProducto)GetProcAddress(hGetProcIDDLL, "fSetDatoProducto");
	
	int cCodigoProducto = fSetDatoProducto((char*)"cCodigoProducto", productId);
	if(cCodigoProducto){
		utils.c2sLog("error fInsertaProducto cCodigoProducto");
		err = true;
	}
	
	int cNombreProducto = fSetDatoProducto((char*)"cNombreProducto", productTitle);
	if(cNombreProducto){
		utils.c2sLog("error fInsertaProducto cNombreProducto");
		err = true;
	}
	
	//We are storing product variation data inside descripcion in an json array, first the product [0]sku then [1]barcode, example: ["MINI BOTELLA","758475402897"]
	int cDescripcionProducto = fSetDatoProducto((char*)"CTEXTOEX01", productVariationData);
	if(cDescripcionProducto){
		utils.c2sLog("error fInsertaProducto cDescripcionProducto");
		err = true;
	}
	
	int cImpuesto1 = fSetDatoProducto((char*)"cImpuesto1", (char*)"16");
	if(cImpuesto1){
		utils.c2sLog("error fInsertaProducto cImpuesto1");
		err = true;
	}
	
	int cPrecio1 = fSetDatoProducto((char*)"cPrecio1", productPrice);
	if(cPrecio1){
		utils.c2sLog("error fInsertaProducto cPrecio1");
		err = true;
	}
		
	int cStatusProducto = fSetDatoProducto((char*)"cStatusProducto", (char*)"1");
	if(cStatusProducto){
		utils.c2sLog("error fInsertaProducto cStatusProducto");
		err = true;
	}
	
	int cFechaAltaProducto = fSetDatoProducto((char*)"cFechaAltaProducto", humantime);
	if(cFechaAltaProducto){
		utils.c2sLog("error fInsertaProducto cFechaAltaProducto");
		err = true;
	}
		
	int cMetodoCosteo = fSetDatoProducto((char*)"cMetodoCosteo", (char*)"1");
	if(cMetodoCosteo){
		utils.c2sLog("error fInsertaProducto cMetodoCosteo");
		err = true;
	}
	
	int cControlExistencia = fSetDatoProducto((char*)"cControlExistencia", (char*)"1"); 
	if(cControlExistencia){
		utils.c2sLog("error fInsertaProducto cControlExistencia");
		err = true;
	}

	int CIDUNIDA01 = fSetDatoProducto((char*)"CIDUNIDA01", (char*)"1");
	if(CIDUNIDA01){
		utils.c2sLog("error fInsertaProducto CIDUNIDA01");
		err = true;
	}
	
	int CIDUNIDA02 = fSetDatoProducto((char*)"CIDUNIDA02", (char*)"0");
	if(CIDUNIDA02){
		utils.c2sLog("error fInsertaProducto CIDUNIDA02");
		err = true;
	}
	
//	printf("cCodigoProducto: %d \n", fInsertaProducto);
//	printf("cNombreProducto: %d \n", fInsertaProducto);
//	printf("cImpuesto1: %d \n", cImpuesto1);
//	printf("cPrecio1: %d \n", cPrecio1);
//	printf("cStatusProducto: %d \n", cStatusProducto);
//	printf("cFechaAltaProducto: %d \n", cFechaAltaProducto);
//	printf("cMetodoCosteo: %d \n", cMetodoCosteo);
//	printf("cControlExistencia: %d \n", cControlExistencia);
//	printf("CIDUNIDA01: %d \n", CIDUNIDA01);
//	printf("CIDUNIDA02: %d \n", CIDUNIDA02);
//	printf("fGuardaProducto: %d \n", fGuardaProducto);

	
//	typedef int (__stdcall *_fError)(int x, char* y, int z);
//	_fError fError = (_fError)GetProcAddress(hGetProcIDDLL, "fError");
//	char aMensaje[3500];
//	fError(cCodigoUnidadNoConvertible, aMensaje, 3500);
//	printf("error msg: %s \n", aMensaje);
	
	if(err){
		return 1;
	} else {
		int fGuardaProducto = (int)GetProcAddress(hGetProcIDDLL, "fGuardaProducto")();
//		free(humantime);
		if(fGuardaProducto){
			return 1;
		} else {
			return 0;	
		}
	}
}

int Adminpaq::fBuscaCodigoProducto(char* prductId){
	typedef int (__stdcall *_fBuscaCodigoProducto)(char* x);
	_fBuscaCodigoProducto buscaProducto = (_fBuscaCodigoProducto)GetProcAddress(hGetProcIDDLL, "fBuscaProducto");
	
	int err = buscaProducto(prductId);
	
//	typedef int (__stdcall *_fError)(int x, char* y, int z);
//	_fError fError = (_fError)GetProcAddress(hGetProcIDDLL, "fError");
//	char aMensaje[3500];
//	fError(err, aMensaje, 3500);
//	printf("error msg: %s \n", aMensaje);

	return err;
}

int Adminpaq::fEditaProducto(char* productTitle, char* productPrice, char* productVariationData){
	
	bool err = false;
	
	if(!productTitle && !productPrice && !productVariationData) {
		utils.c2sLog("error on fEditaProducto data");
		err = true;
	} else {
		utils.c2sLog("no error on fEditaProducto data");
		int fEditaProducto = (int)GetProcAddress(hGetProcIDDLL, "fEditaProducto")();
		if(fEditaProducto){
			utils.c2sLog("error fEditaProducto");
			err = true;
		}
		
		typedef int (__stdcall *_fSetDatoProducto)(char* x, char* y);
		_fSetDatoProducto fSetDatoProducto = (_fSetDatoProducto)GetProcAddress(hGetProcIDDLL, "fSetDatoProducto");
		
		//We are storing product variation data inside descripcion in an json array, first the product [0]sku then [1]barcode, example: ["MINI BOTELLA","758475402897"]
		
		if(productVariationData){
			int cDescripcionProducto = fSetDatoProducto((char*)"CTEXTOEX01", productVariationData);
			if(cDescripcionProducto) {
				utils.c2sLog("error fEditaProducto CTEXTOEX01");
				err = true;	
			} else {
				utils.c2sLog("no error fEditaProducto CTEXTOEX01");
			}
		} 
		
		if(productTitle){
			utils.c2sLog("has productTitle");
			int cNombreProducto = fSetDatoProducto((char*)"cNombreProducto", productTitle);
			if(cNombreProducto){
				utils.c2sLog("error fEditaProducto cNombreProducto");
				err = true;
			} else {
				utils.c2sLog("no error fEditaProducto cNombreProducto");
			}
		}
		
		if(productPrice){
			utils.c2sLog("has productPrice");
			utils.c2sLog("price " + string(productPrice));
			int cPrecio1 = fSetDatoProducto((char*)"cPrecio1", productPrice);
			if(cPrecio1){
				utils.c2sLog("error fEditaProducto cPrecio1");
				err = true;
			} else {
				utils.c2sLog("no error fEditaProducto cPrecio1");
			}
		}
		utils.c2sLog("about to fGuardaProducto");
		int fGuardaProducto = (int)GetProcAddress(hGetProcIDDLL, "fGuardaProducto")();
		if(fGuardaProducto){
			utils.c2sLog("error fEditaProducto fGuardaProducto");
			err = true;
		} else {
			utils.c2sLog("no error fEditaProducto fGuardaProducto");
		}
	}
	
	if(err){
		return 1;
	} else {
		return 0;
	}
}




