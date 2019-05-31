#ifndef C2SUTILS_H
#define C2SUTILS_H

class C2SUtils {
	public:
		void c2sLog(std::string input);
		int getQuantityFromSku(std::string title);
		std::string getSkuWithoutQty(std::string title);
		int loadConfigValues();
		std::string getStringFromConfig(std::string);
		void c2sSleep(int lapse);
};

#endif
