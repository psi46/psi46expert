#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <fstream>
#include <iostream>
#include <stdio.h>

#include <string>
#include <vector>

using namespace std;

struct ConfigItem{
  string line;
  string id;
  int idx1;
  int idx2;
  vector<string> values;
};

class ConfigReader{

 private:
 static const int kNoIndex=-123456;
 char fileName[101];
 vector<ConfigItem> items;
 //ConfigItem* findItem(const char* name);
 ConfigItem* findItem(const char* name, const int idx=kNoIndex);
 ConfigItem* createItem(const char* name, const int idx=kNoIndex);
 void warnLength(ConfigItem* a, int nRequest);

 public:
 ConfigReader(const char* f, const char* d="");
 ~ConfigReader();
 void rewrite(int verbose=0);

 // get single values
 void get (const char* name, int &value, int defaultValue=0);
 void get (const char* name, const int idx, int &value, int defaultValue=0);
 void get (const char* name, double &value, double defaultValue=0);
 void get (const char* name, const int idx, double &value, double defaultValue=0);

 // get arrays
 void geta(const char* name, const unsigned int nval, int* v);
 void geta(const char* name, const int idx, const unsigned int nval, int* v);
 void geta(const char* name, const unsigned int nval, double* v);
 void geta(const char* name, const int idx, const unsigned int nval, double* v);
 // void get(const char* name, const int idx1, float* a, int n);

 ConfigItem * updateHelper(const char* name, const unsigned int nval=1, const int idx=kNoIndex);
 void updatea(const char* name, const unsigned int nval, double *v, 
				  const char* format);
 void updatea(const char* name, const unsigned int nval, int *v, 
				  const char* format);
 void updatea(const char* name, int idx, const unsigned int nval, double *v, 
				  const char* format);
 void updatea(const char* name, int idx, const unsigned int nval, int *v, 
				  const char* format);

 void Tokenize(const string& str,
	       vector<string>& tokens,
	       const string& delimiters = " ");
};
#endif
