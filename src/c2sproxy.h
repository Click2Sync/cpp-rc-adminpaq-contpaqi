#ifndef C2SPROXY_H
#define C2SPROXY_H
#include <nlohmann/json.hpp>

class C2SProxy {
	public:
		bool sense();
		bool checkIfFetchingRemote();
		bool checkIfPullingToRemote();
		std::string getStrategy();
		std::string getStatus();
		std::string getUpstreamStatus();
		std::string getEntity();
		int getCursorOffset();
		char* setInitializeUpload(char* strategy);
		char* setInitializeDownload();
		char* setProductToUploadOnBuffer(nlohmann::json product, char* strategy);
		char* setOrderToUploadOnBuffer(nlohmann::json order, char* strategy);
		char* flushProductsToUpstream(char* strategy);
		char* flushOrdersToUpstream(char* strategy);
		bool hasMoreUpdatedProducts(int attempts);
		nlohmann::json nextProduct();
		bool hasMoreUpdatedOrders(int attempts);
		nlohmann::json nextOrder();
		char* sendProductPullSuccessNotification(std::string id, nlohmann::json productstored, bool succeded, std::string error);
		char* sendOrderPullSuccessNotification(std::string id, nlohmann::json orderstored, bool succeded, std::string error);
		char* setFinishProductUpload(char* strategy);
		char* setFinishOrderUpload(char* strategy);
		char* setFinishUpload(char* strategy);
		char* setFinishProductDownload();
		char* setFinishOrderDownload();
		char* setFinishDownload();
};

#endif
