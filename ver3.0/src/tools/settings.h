#ifndef _registry_settings_h_
#define _registry_settings_h_

#include "threadpool/thread.h"
#include "base/string.h"
#include "xml/xmlaccss.h"

class settings
{
    // Construction
public:
	settings(const char* filename, const char* path, const char* key);
	~settings();

    // Operations
public:
    void	GetSet(const char* subkey, const char* name, int& nValue);
	void	GetSet(const char* subkey, const char* name, TERIMBER::string_t& strValue);
    bool	Get(const char* subkey, const char* name, int& nValue);
    bool	Get(const char* subkey, const char* name, TERIMBER::string_t& strValue);
    void	Set(const char* subkey, const char* name, int nValue);
    void	Set(const char* subkey, const char* name, const TERIMBER::string_t& pszValue);

    // Data
private:
	void make_xpath(TERIMBER::string_t& path, const char* subkey); 
	TERIMBER::string_t	_filename;
	TERIMBER::string_t	_root;
	xml_designer*		_parser;
};

#endif // _registry_settings_h_
