#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include "../UnitTest.h"

using namespace std;

map<string, string> options;
vector<string> lines;
size_t linesize = 0;
string make;

void makeMap(void){
    options["LIBRARY"] = "#define JSON_LIBRARY";
    options["DEBUG"] = "#define JSON_DEBUG";
    options["SAFE"] = "#define JSON_SAFE";
    options["STDERROR"] = "#define JSON_STDERROR";
    options["PREPARSE"] = "#define JSON_PREPARSE";
    options["LESS_MEMORY"] = "#define JSON_LESS_MEMORY";
    options["UNICODE"] = "#define JSON_UNICODE";
    options["REF_COUNT"] = "#define JSON_REF_COUNT";
    options["BINARY"] = "#define JSON_BINARY";
    options["MEMORY_CALLBACKS"] = "#define JSON_MEMORY_CALLBACKS";
    options["MEMORY_MANAGE"] = "#define JSON_MEMORY_MANAGE";
    options["MUTEX_CALLBACKS"] = "#define JSON_MUTEX_CALLBACKS";
    options["MUTEX_MANAGE"] = "#define JSON_MUTEX_MANAGE";
    options["ISO_STRICT"] = "#define JSON_ISO_STRICT";
    options["ITERATORS"] = "#define JSON_ITERATORS";
    options["WRITER"] = "#define JSON_WRITER";
    options["NEWLINE"] = "#define JSON_NEWLINE \"\\r\\n\"";
    options["COMMENTS"] = "#define JSON_COMMENTS";
    options["INDENT"] = "#define JSON_INDENT \"    \"";
    options["WRITE_BASH_COMMENTS"] = "#define JSON_WRITE_BASH_COMMENTS";
    options["WRITE_SINGLE_LINE_COMMENTS"] = "#define JSON_WRITE_SINGLE_LINE_COMMENTS";
    options["VALIDATE"] = "#define JSON_VALIDATE";
    options["UNIT_TEST"] = "#define JSON_UNIT_TEST";
    options["INDEX_TYPE"] = "#define JSON_INDEX_TYPE unsigned int";  
    options["CASE_INSENSITIVE_FUNCTIONS"] = "#define JSON_CASE_INSENSITIVE_FUNCTIONS";
}

void testRules(unsigned int i){
    remove("./testapp");
    system(make.c_str());
    if (FILE * fp = fopen("./testapp", "r")){
	   fclose(fp);
	   
	   remove("./out.html");
	   system("./testapp");
	   if (FILE * fp = fopen("./out.html", "r")){
		  char buffer[255];
		  fread(&buffer[0], 255, 1, fp);
		  buffer[254] = '\0';
		  fclose(fp);
		  string buf(&buffer[0]);
		  size_t pos = buf.find("Failed Tests: <c style=\"color:#CC0000\">");
		  if (pos == string::npos){
			 FAIL("Something Wrong");
		  } else {
			 if(buf[pos + 39] == '0'){
				PASS("GOOD");
			 } else {
				size_t pp = buf.find('<', pos + 39);
				FAIL(string("Didn't pass ") +  buf.substr(pos + 39, pp - pos - 39) +  " tests");
			 }
		  }
	   } else {
		  FAIL("Running crashed");
	   }   
    } else {
	   FAIL(string("Compilation failed - ") + lines[i]);
    }
}

bool makeTempOptions(unsigned int i){
    string & line = lines[i];
    if (line.length() < 5) return false;
    if (line[0] == '#') return false;
    
    if (FILE * fp = fopen("../JSONOptions.h", "w")){
	   string res("#ifndef JSON_OPTIONS_H\n#define JSON_OPTIONS_H\n");
	   for (
		   map<string, string>::iterator runner = options.begin(), end = options.end();
		   runner != end;
		   ++runner){
		  
		  if (line.find(runner -> first) != string::npos){
			 res += runner -> second + "\n";
		  }
	   }
	   res += "#endif\n";
	   
	   fwrite(res.c_str(), res.length(), 1, fp);
	   fclose(fp);
	   return true;
    }
    return false;
}

bool hideGoodOptions(void){
    return (rename("../JSONOptions.h", "../__JSONOptions.h") == 0);
}

bool loadTests(void){
    ifstream infile("All/Options.txt");
    
    if (!infile){
	   return false;
    }
    
    string line;
    while (getline(infile, line)){
	   size_t pos = line.find_first_not_of("\r\n\t ");
	   if (line[0] != '#') ++linesize;
	   lines.push_back(line.substr(pos));
    }
    infile.close();
    return true;
}

void Go(void){
    echo(make);
    size_t counter = 0;
    for (unsigned int i = 0; i < lines.size(); ++i){
	   if(makeTempOptions(i)){
		  stringstream mystream;
		  mystream << "Line " << i + 1;
		  cout << "Compiling " << ++counter << " of " << linesize <<  " - " << mystream.str() << endl;
		  cout << "     " << lines[i] << endl;
		  UnitTest::SetPrefix(mystream.str());
		  testRules(i);
		  remove("../JSONOptions.h");
		  UnitTest::SaveTo("progress.html");
	   }
    }
}
    

void RunTests(void){
    if (hideGoodOptions()){
	   if(loadTests()){
		  makeMap();
		  
		  make = "make single";
		  Go();
		  make = "make debug";
		  Go();
		  make = "make small";
		  Go();
	   } else {
		  FAIL("couldn't open options");  
	   }
	   rename("../__JSONOptions.h", "../JSONOptions.h");
    } else {
	   FAIL("Couldn't protect JSONOptions");
    }   
}

int main (int argc, char * const argv[]) { 
    UnitTest::StartTime();

    RunTests();
    
    remove("progress.html");
    UnitTest::SaveTo("out.html");
    return 0;
}
