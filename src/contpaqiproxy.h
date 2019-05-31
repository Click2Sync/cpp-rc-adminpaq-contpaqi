#ifndef CONTPAQIPROXY_H
#define CONTPAQIPROXY_H
#include <nlohmann/json.hpp>

class ContpaqiProxy {
	public:
		bool sense();
		int resetFolio();
		nlohmann::json nextProduct();
		nlohmann::json nextOrder();
		void setProductsStreamCursor(int offsetdate);
		void setOrdersStreamCursor(int offsetdate);
		bool hasMoreProducts(int offsetdate);
		bool hasMoreOrders(int offsetdate);
		nlohmann::json storeOrder(nlohmann::json order);
		nlohmann::json storeProduct(nlohmann::json product);
};

#endif
