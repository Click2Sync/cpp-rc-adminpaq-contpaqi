#ifndef ADMINPAQ_H
#define ADMINPAQ_H
#include <nlohmann/json.hpp>

class Adminpaq {
	public:
		bool fLoadDll();
		int fInicializaSDK();
		int fTerminaSDK();
		int fAbreEmpresa(char* ruta);
		int fCierraEmpresa();
		int fPosPrimerProducto();
		int fPosSiguienteProducto();
		int fPosEOFProducto();
		int fLeeDatoProducto(char* id, char* valor, int len);
		nlohmann::json convertContpaqiProductToC2SProduct();
		int fBuscaIdProducto(int id);
		int fPosPrimerDocumento();
		int fPosUltimoDocumento();
		int fPosSiguienteDocumento();
		int fPosDocumentoEOF();
		int fLeeDatoDocumento(char* id, char* valor, int len);
		nlohmann::json convertContpaqiOrderToC2SOrder();
		int fSetFiltroMovimiento(int id);
		int fCancelaFiltroMovimiento();
		int fPosSiguienteMovimiento();
		int fPosMovimientoEOF();
		int fPosUltimoMovimiento();
		int fLeeDatoMovimiento(char* id, char* valorMov, int len);
		int fBuscaIdCteProv(int id);
		int fLeeDatoCteProv(char* id, char* valor, int len);
		int fRegresaExistencia(char* idprod, char* idalmacen, char* anio, char* mes, char* dia, double* aExistencia);
		long fAltaDocumento(long long int orderDate, long int newFolio, char* codConcepto);
		int fAltaMovimiento(double quantity, double unitPrice, const char* prodId, long DocumentId);
		int GetLastFolio(char* codConcepto);
		int fAfectaDocto(long int docFolio);
		int fBorraDocumento();
		int fSetDatoProducto(char* productId, char* productTitle, char* productPrice, char* productVariationData);
		int fBuscaCodigoProducto(char* prductId);
		int fEditaProducto(char* productTitle, char* productPrice, char* productVariationData);
};

#endif
