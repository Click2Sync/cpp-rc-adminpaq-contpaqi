#include <stdio.h>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include "contpaqiproxy.h"
#include "c2sproxy.h"
#include "adminpaq.h"
#include "c2sutils.h"
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <iostream>
#include <tinyutf8.h>
#include <locale>
#include <conio.h>

static C2SProxy c2s;
static C2SUtils utils;

using namespace std;
using json = nlohmann::json;

long int numFolio = 0;

Adminpaq adminpaq;
//load adminpaq dll
bool loaded = false;
//initialize sdk
int initSDK = 0;
char* ruta;
int abreEmpresa = 0;
int primerProducto = 0;

bool firstproduct = false;

bool ContpaqiProxy::sense(){
	
	loaded = adminpaq.fLoadDll();
	initSDK = adminpaq.fInicializaSDK();
	ruta = (char*)utils.getStringFromConfig("contpaqiempresapath").c_str();
	abreEmpresa = adminpaq.fAbreEmpresa(ruta);
	primerProducto = adminpaq.fPosPrimerProducto();
	return true;
	
}

void utf8tolatin1(char *s) {
  size_t i = 0, j = 0;
  char c;
  do {
    c = s[i++];
    if ((c&0xFC) == 0xC0 && s[i])
      c = (c<<6) + (s[i++]&0x3F);
    s[j++] = c;
  } while(c != 0);
}

int ContpaqiProxy::resetFolio(){
	numFolio = 0;
	
	return 0;
}

json ContpaqiProxy::nextProduct(){
	if(firstproduct) {
		adminpaq.fPosSiguienteProducto();
	}
	json product = adminpaq.convertContpaqiProductToC2SProduct();
	return product;
}

json ContpaqiProxy::nextOrder(){
	adminpaq.fPosSiguienteDocumento();
	json order = adminpaq.convertContpaqiOrderToC2SOrder();
	return order;
}

void ContpaqiProxy::setProductsStreamCursor(int offsetdate){
	firstproduct = true;
	if(offsetdate = 0) {
		adminpaq.fPosPrimerProducto();
		adminpaq.fPosSiguienteProducto();	
	}
}

bool ContpaqiProxy::hasMoreProducts(int offsetdate){
	int endofproducts = adminpaq.fPosEOFProducto();
	if(endofproducts) {
		return false;
	} else {
		return true;
	}
}

void ContpaqiProxy::setOrdersStreamCursor(int offsetdate){
	adminpaq.fPosPrimerDocumento();
}

bool ContpaqiProxy::hasMoreOrders(int offsetdate){
	int endoforders = adminpaq.fPosDocumentoEOF();
	if(endoforders) {
		return false;
	} else {
		return true;	
	}
}

json ContpaqiProxy::storeProduct(nlohmann::json product){
	utils.c2sLog("Entered storeProduct()");
	
	char *productPrice = (char*)malloc(100);
	char *productId = (char*)malloc(100);
	char *productTitle = (char*)malloc(100);
	char *productVariationData = (char*)malloc(100);
	
	strcpy(productPrice, (char*)"0");
	strcpy(productTitle, (char*)"test");
	
	bool err = false;
	json productempty;
	string errormessage;
	
	string sku = product.value("sku", "");
	
	string idForClick2sync = "";
	
	// try to read product title
	string getTitle;
	try{	
		getTitle = product.value("title", "");	
	} catch (json::exception& e) {
		utils.c2sLog("Error reading product Title");
		err = true;
		errormessage = errormessage + "Error reading product Title ";
	}
	if(getTitle == ""){
		productTitle = (char*)0;
	} else{
		if(productTitle != NULL) {
			strcpy(productTitle, getTitle.c_str());
		}
		utf8tolatin1(productTitle);	
	}
	
	utils.c2sLog("productTitle: " + getTitle);
	
	double getPrice;
	double getAvailableQty;
	json barcodearray;
	json getVariations;
	json arrayVariation;
	json getPrices;
	json arrayPrice;
	json getAvailabilities;
	json arrayAvailabilities;
	string getColor;
	string getSize;
	string colorSize;
	string _id;
	bool isUpdate = false;
	char* tmpPrice;
	
	//get _id to check if is update
	try{
		_id = product.value("_id", "");
		isUpdate = true;
	} catch (json::exception& e) {
		_id = "";
		isUpdate = false;
	}
	if(isUpdate) {
		idForClick2sync = _id;
	}
	
	//get variations
	try{
		getVariations = product.value("variations", productempty);
	} catch (json::exception& e) {
		utils.c2sLog("Error reading product variations");
		err = true;
		errormessage = errormessage + "Error reading product variations";
	}
	
	bool variationsSuccess = false;
	
	//read each variation and create or edit product
	for(int i=0; i<getVariations.size(); i++) {
		bool errVariation = false;
		string varId;
		if(!getVariations.empty()){
			arrayVariation = getVariations.at(i);
			string barcodeid;
			
			try{
				barcodearray = json::parse(arrayVariation.value("barcode", ""));
				barcodeid = barcodearray.at(0);
				if(barcodeid != "") {
					strcpy(productId, barcodeid.c_str());
					utf8tolatin1(productId);
					varId = barcodeid;
					utils.c2sLog("varId: " + varId);
					if(idForClick2sync == "") {
						string puresku = utils.getSkuWithoutQty(varId);
						idForClick2sync = puresku;
					}
				} else {
					utils.c2sLog("No id for product in the array");
					errVariation = true;
					errormessage = errormessage + "No id for product in the array";
				}
			} catch (json::exception& e) {
				if(isUpdate) {
					strcpy(productId, sku.c_str());
					utf8tolatin1(productId);
				} else {
					utils.c2sLog("Barcode in format not supported by this implementation");
					errVariation = true;
					errormessage = errormessage + "Error reading barcode, expecting array format for this implementation";	
				}
			}
			
			//get price of variation
			utils.c2sLog("get price");
			try{
				getPrices = arrayVariation.value("prices", productempty);
			} catch (json::exception& e) {
				utils.c2sLog("Error reading product price");
				errVariation = true;
				errormessage = errormessage + "Error reading product price ";
			}
			if(!getPrices.empty()){
				try{
					arrayPrice = getPrices.at(0);
					getPrice = arrayPrice["number"].get<double>();
				} catch (json::exception& e) {
					utils.c2sLog("Error reading product price");
					errVariation = true;
					errormessage = errormessage + "Error reading product price ";		
				}
			} else {
				productPrice = (char*)0;
			}
			
			//get availability of variation
			utils.c2sLog("get availability");
			try{
				getAvailabilities = arrayVariation.value("availabilities", productempty);
			} catch (json::exception& e) {
				utils.c2sLog("Error reading product availability");
				errVariation = true;
				errormessage = errormessage + "Error reading product availability ";		
			}
			if(!getAvailabilities.empty()) {
				try {
					arrayAvailabilities = getAvailabilities.at(0);
					getAvailableQty = arrayAvailabilities.value("quantity", 0);
				} catch (json::exception& e) {
					utils.c2sLog("Error reading product availability");
					errVariation = true;
					errormessage = errormessage + "Error reading product availability ";		
				}
			} else {
				getAvailableQty = 0;
			}
			
			//get color of variation
			utils.c2sLog("get color");
			try{
				getColor = arrayVariation.value("color", "");
			} catch (json::exception& e) {
				getColor = "";
				utils.c2sLog("Error reading product variation color");
				errormessage = errormessage + "Error reading product variation color";
				errVariation = true;
			}
			colorSize = "[\""+getColor+"\",";
			
			//get size of variation
			utils.c2sLog("get size");
			try{
				getSize = arrayVariation.value("size", "");
			} catch(json::exception& e) {
				getSize = "";
				utils.c2sLog("Error reading product size");
				errormessage = errormessage + "Error reading product variation size";
				errVariation = true;
			}
			colorSize = colorSize + "\"" + getSize + "\"]";
			
			if(colorSize == ""){
				productVariationData = (char*)0;
			} else{
				strcpy(productVariationData, colorSize.c_str());
				utf8tolatin1(productVariationData);	
			}
		} else {
			getPrice = 0;
		}
		if(productPrice){
			string priceToChar = to_string(getPrice);

			tmpPrice = (char*)priceToChar.c_str();
			strcpy(productPrice, tmpPrice);
		}
	
		
//		if(isUpdate) {
//			strcpy(productId, varId.c_str());
//			utf8tolatin1(productId);
//		}else{
//			strcpy(productId, _id.c_str());
//			utf8tolatin1(productId);
//		}
		
		// if err send err else create or edit product
		if(errVariation){
			utils.c2sLog("err before create or edit");
		} else {
			utils.c2sLog("about to check if edit or create");
			
			//Check for a -# on the title if there is none create the product else not but return success
			string stringproductId;
			stringproductId = string(productId);
			string skuNoQuantity = "";
			skuNoQuantity = utils.getSkuWithoutQty(stringproductId);
			int checkQuantitySku = utils.getQuantityFromSku(productId);
			int searchProductCode;
			strcpy(productId, skuNoQuantity.c_str());
			utf8tolatin1(productId); 
			
			searchProductCode = adminpaq.fBuscaCodigoProducto(productId);
			
			if(searchProductCode){
				//Create product
				utils.c2sLog("about to setAltaProducto");
				if(!productId || !productTitle || !productPrice || !productVariationData) {
					utils.c2sLog("required data for create is missing");
					errormessage = "Error creating product, required data for create is missing";
					errVariation = true;
				
				} else {
					utils.c2sLog("no err in data");
					utils.c2sLog("productId: " + string(productId));
					utils.c2sLog("productTitle: " + string(productTitle));
					utils.c2sLog("productPrice: " + string(productPrice));
					utils.c2sLog("productVariationData: " + string(productVariationData));
					int setAltaProducto = adminpaq.fSetDatoProducto(productId, productTitle, productPrice, productVariationData);	
					//if setAltaProducto not 0 return error else product was created
					if(setAltaProducto != 0){
						errVariation = true;
						utils.c2sLog("Error creating product");
						errormessage = "Error creating product";
					
					} else{
						utils.c2sLog("no error on setAltaProducto");
						//Add movimiento entrada for availability
						//Get last Folio
						if(numFolio == 0) {
							long int getLastFolio = adminpaq.GetLastFolio((char*)"34");
							numFolio = getLastFolio;
						}
						numFolio = numFolio + 1;
						long long int dateCreated;
						dateCreated = 0;
						long idDocumento = adminpaq.fAltaDocumento(dateCreated, numFolio, (char*)"34");
						int altaMovimiento = adminpaq.fAltaMovimiento(getAvailableQty, getPrice, productId, idDocumento);
						
						product["sku"]=idForClick2sync;
						product["_id"]=idForClick2sync;
						utils.c2sLog("finished creating product");
					}
				}		
			} else {
				utils.c2sLog("about to edit product");
				//Edit product
				//if editProduct not = 0 return error else product was created
				int editProduct = adminpaq.fEditaProducto(productTitle, productPrice, productVariationData);
				if(editProduct != 0){
					errVariation = true;
					utils.c2sLog("Error editing product");
					errormessage = "Error editing product";
				} else {
					product["sku"]=idForClick2sync;
					product["_id"]=idForClick2sync;	
				}
			}	
		}
		
		if(!errVariation){
			variationsSuccess = true;
		} else {
			err = true;
		}		
		
	}
	//free char memory alloc
	free(productPrice);
	free(productId);
	free(productTitle);
	
	if(variationsSuccess){
		utils.c2sLog("no error, about to return product json to send to c2s");
		for(int i=1; i<getVariations.size(); i++) {
			getVariations.erase(i);
		}
		product["variations"] = getVariations;
		string productdump = product.dump();
		return product;
		
	} else{
		if(err) {
			utils.c2sLog("error, send c2s error");
			c2s.sendProductPullSuccessNotification(sku, 0, false, errormessage);
			return productempty;
		} else {
			product["sku"] = sku;
			product["_id"] = sku;
			return product;
		}		
	}

}

json ContpaqiProxy::storeOrder(nlohmann::json order){
	
	bool err = false;
	json orderempty;
	string errormessage;
	string id = order.value("orderid", "");
	
	//Get last Folio
	if(numFolio == 0){
		long int getLastFolio = adminpaq.GetLastFolio((char*)"4");
		numFolio = getLastFolio;
	}

	//Get DateCreated
	long long int dateCreated;
	dateCreated = 0;
	try{
//		dateCreated = order.value("dateCreated", 0);
	} catch (json::exception& e) {
		utils.c2sLog("Error reading dateCreated");
		//std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
		err = true;
		errormessage = "Error reading dateCreated";
	}
	
	//if err send error else keep going
	if(err) {
		c2s.sendOrderPullSuccessNotification(id, 0, false, errormessage);
		return orderempty;
	} else {
		
		numFolio = numFolio + 1;
		long idDocumento = adminpaq.fAltaDocumento(dateCreated, numFolio, (char*)"4");
		//Get OrderItems
		json orderDefault = json({});
		json orderItems;
		try{
			orderItems = order.value("orderItems", orderDefault);	
		} catch (json::exception& e) {
			utils.c2sLog("Error reading orderItems");
			//std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
			err = true;
			errormessage = "Error reading orderItems";
		}
		
		//if err send error else keep going
		if(err) {
			c2s.sendOrderPullSuccessNotification(id, 0, false, errormessage);
			int deleteDocument = adminpaq.fBorraDocumento();
			numFolio = numFolio - 1;
			return orderempty;
		} else {
			
			//Iterate the OrderItems
		    for (auto& items : orderItems.items()){ 
				json itemsValue = items.value();

				//Get OrderItem Id
				string prodstring;
				try{
					prodstring = itemsValue.value("id", "");
					if(prodstring == ""){
						utils.c2sLog("Error, read a blank at product id");
						err = true;
						errormessage = errormessage + "Error, read a blank at product id";
					}
				} catch (json::exception& e) {
					utils.c2sLog("Error reading orderItem id");
					//std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
					err = true;
					errormessage = errormessage + "Error reading orderItem id ";
				}
				char *prodId = (char*)malloc(100);
				if(prodId != NULL) {
					strcpy(prodId, prodstring.c_str());
				}
				utf8tolatin1(prodId);
				
				
				//Get OrderItem Quantity
				int quantityFromTitle = utils.getQuantityFromSku(prodstring);
				double quantity;
				double totalQuantity;
				try {
					quantity = itemsValue.value("quantity", 0);
					totalQuantity = quantity * quantityFromTitle;
				} catch (json::exception& e) {
					utils.c2sLog("Error reading orderItem quantity");
					//std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
					err = true;
					errormessage = errormessage + "Error reading orderItem quantity ";
				}
				
				//Get OrderItem UnitPrice
				double unitPrice;
				try{
					unitPrice = itemsValue.value("unitPrice", 0.0d);
					unitPrice = unitPrice*(1/1.16);
				} catch (json::exception& e) {
					utils.c2sLog("Error reading orderItem unitPrice");
					//std::cout << "message: " << e.what() << '\n' << "exception id: " << e.id << std::endl;
					err = true;
					errormessage = errormessage + "Error reading orderItem unitPrice ";
				}
				
				//if err send error keep going
				if(err){
					c2s.sendOrderPullSuccessNotification(id, 0, false, errormessage);
					int deleteDocument = adminpaq.fBorraDocumento();
					numFolio = numFolio - 1;
					free(prodId);
					return orderempty;	
				} else {
					
					//Create an movement in contpaq
					if(prodstring != "shipping" && prodstring != "surcharge"){
						int altaMovimiento = adminpaq.fAltaMovimiento(totalQuantity, unitPrice, prodId, idDocumento);
						free(prodId);
						
						if(altaMovimiento != 0){
							int deleteDocument = adminpaq.fBorraDocumento();
							utils.c2sLog("Error creating order at " + prodstring);
							errormessage = "Error creating order at " + prodstring;
							c2s.sendOrderPullSuccessNotification(id, 0, false, errormessage);
		
							return orderempty;
						}
					}
				}
			}

			order["_id"]=numFolio;
			order["orderid"]=numFolio;
			
			return order;
		}
	}
}
