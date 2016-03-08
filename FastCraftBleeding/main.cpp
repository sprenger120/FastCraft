#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include "NBTAll.h"
#include <atomic>
//#include <vld.h>
#include "WorldInfo.h"
#include "FCException.h"
#include <Poco\Path.h>
#include <Poco\Exception.h>
#include <Poco\Stopwatch.h>
#include <Poco\ByteOrder.h>
#include <Poco\InflatingStream.h>
#include <sstream>
#include "ConstString.h"
using namespace std;

//int main() {
//
//	for (int x=0;x<=10000;x++) {
//		Poco::Path worldinfofile("F:\\Minecraft Projects\\FastCraft\\Build\\Server\\main\\world\\level.dat");
//		WorldInfo world(worldinfofile);
//		break;
//	}
//
//	return 0;
//}


//std::string :    450 @ 2,1ms  @ 212,955 per sec 

int main() {
	//fstream file("F:\\Minecraft Projects\\FastCraft\\Build\\Server\\main\\world\\region\\r.-1.0.mca",fstream::in | fstream::binary);
	//int parsed=0;
	//int iSizeTable[1024];
	//file.seekg(0);
	//file.read((char*)&iSizeTable,1024*4);

	//Poco::Stopwatch sw;
	//sw.start();

	//for(int x=0;x<=1023;x++) {
	//	iSizeTable[x] = Poco::ByteOrder::flipBytes((Poco::Int32)iSizeTable[x])>>8;
	//	if (iSizeTable[x] == 0) {continue;}

	//	file.seekg(iSizeTable[x]*4096);

	//	int iLen = 0;
	//	char iType = 0;
	//	file.read((char*)&iLen,4);

	//	iLen =  Poco::ByteOrder::flipBytes((Poco::Int32)iLen);
	//	if (iLen <= 0) {
	//		continue;
	//	}

	//	file.read(&iType,1);

	//	Poco::InflatingInputStream unzipStrm(file,Poco::InflatingStreamBuf::STREAM_ZLIB);

	//	NBTTagCompound* p = new NBTTagCompound(string(""));
	//	p->init(unzipStrm);

	//	/*delete p;*/

	//	parsed++;
	//	//if (parsed == 5) {break;}
	//}
	//sw.stop();
	//cout<<parsed<<" loaded "<<" in: "<< (sw.elapsed()/1000)<<"ms @"<<(double(parsed)/(double(sw.elapsed())/1000000.0))<<" elements/s\n";

	//std::stringstream ss;
	//ss<<"Hallo There here is blasbo baggins";

	//string stringSrc;
	//stringSrc.assign("Hallo There here is blasbo baggins");

	//const char* str = {"Hallo There here is blasbo baggins"};
	//const char* str2 = {"Hallo There here is blasbo baggins"};
	//


	//ConstString cStr1("Hello there"); //Constant
	//ConstString cStr2(ss,ss.str().length()); //Alloc

	//cStr1 = cStr2;

	//
	//cout<<cStr1.getPtr();
	


	return 1;
}



//
//class TestThreadMutex : public Poco::Runnable {
//private:
//	Poco::Mutex& _rMutex;
//	bool& _fStart;
//public:
//	TestThreadMutex(atomic<long long>&,Poco::Mutex&,bool&);
//	void run();
//};
//
//TestThreadMutex::TestThreadMutex(atomic<long long>& rVar ,Poco::Mutex& rMutex,bool& fStart)  : 
//	_rVar(rVar),
//	_rMutex(rMutex),
//	_fStart(fStart)
//{
//}
//
//void TestThreadMutex::run() {
//	while (!_fStart){}
//
//	for (int x=0;x<=999999;x++) {
//		/*_rMutex.lock();*/
//		_rVar++;
//		/*_rMutex.unlock();*/
//	}
//}
//
//
//int main() {
//	Poco::Thread t1,t2;
//	atomic<long long> Var = 0L;
//	bool fStart = false;
//	Poco::Mutex Mutex;
//	TestThreadMutex testThread1(Var,Mutex,fStart);
//	TestThreadMutex testThread2(Var,Mutex,fStart);
//
//	t1.start(testThread1);
//	t2.start(testThread2);
//
//	Poco::Stopwatch timer;
//	timer.start();
//
//	fStart=true;
//
//	t1.join();
//	t2.join();
//
//	timer.stop();
//	cout<<"Time:"<<(timer.elapsed()/1000)<<"ms\n";
//
//
//	/*
//	 Times:
//	 Without mutex: 14,14,13
//	 Mutex: 659,453,660
//	 atomic: 223,227,225
//
//	*/
//	system("pause");
//	return 0;
//}



/*fstream file("C:\\Users\\Sprenger120\\Desktop\\FastCraft\\Build\\region\\r.0.-1.mca",fstream::in | fstream::binary);

int iSizeTable[1024];
file.seekg(0);
file.read((char*)&iSizeTable,1024*4);

Poco::Stopwatch sw;
sw.start();



for(int x=0;x<=1023;x++) {
iSizeTable[x] = Poco::ByteOrder::flipBytes((Poco::Int32)iSizeTable[x])>>8;
if (iSizeTable[x] == 0) {continue;}

file.seekg(iSizeTable[x]*4096);

int iLen = 0;
char iType = 0;
file.read((char*)&iLen,4);

iLen =  Poco::ByteOrder::flipBytes((Poco::Int32)iLen);
if (iLen <= 0) {continue;}

file.read(&iType,1);


Poco::InflatingInputStream unzipStrm(file,Poco::InflatingStreamBuf::STREAM_ZLIB);

NBTTagCompound* p = new NBTTagCompound("");
p->init(unzipStrm);
}
sw.stop();
cout<<"loaded: "<<" in "<< (sw.elapsed()/1000)<<" ms\n";*/

//std::map<int,int> map;
//Poco::HashMap<int,int> map;
//std::map<int,int> map;
//std::vector<int> vec;

//Poco::Stopwatch sw1,sw2;

//sw1.start();
//for(int x=10000;x>0;x--) {
//	map.insert(std::make_pair(x,x));
//	//heap.add((short)x,new int(x));
//}
//sw1.stop();


//sw2.start();
//for(int x=10000;x>0;x--) {
//	vec.push_back(x);
//}
//sw2.stop();


//cout<<"time1:"<<(sw1.elapsed()/1000)<<"\n";
//cout<<"time2:"<<(sw2.elapsed()/1000)<<"\n";
/*
#include <sstream>
#include <fstream>
#include <Poco\SHA1Engine.h>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <Poco\Random.h>
#include <Poco\Stopwatch.h>
*/
//vector<string>  vec;
//map<string,char*> Map;

//Poco::Random rnd;
//

//for(int x=0;x<=1000;x++)  {
//	stringstream ss;
//	ss<<x;
//	vec.push_back(ss.str());
//	Map[ss.str()] = (char*)1;
//	ss.clear();
//}


//Poco::Stopwatch sw;
//sw.start();
//bool fFound = false;

//for(int x=0;x<=1000;x++)  {
//	stringstream ss;
//	ss.clear();
//	fFound = false;
//	ss<<x;
//	
//	for(int index=0;index<=vec.size()-1;index++) {
//		if (ss.str().compare(vec[index]) == 0) {
//			fFound = true;
//			break;
//		}
//	}
//	if (!fFound) {
//		cout<<"Not found\n";
//	}
//}

//sw.stop();
//cout<<"vector: "<<(sw.elapsed()/1000)<<"ms\n";



//sw.restart();
//for(int x=0;x<=1000;x++)  {
//	stringstream ss;
//	ss.clear();
//	ss<<x;
//	
//	if ((int)Map[ss.str()] != 1) {
//		cout<<"element not found\n";
//	}
//}

//sw.stop();
//cout<<"map: "<<(sw.elapsed()/1000)<<"ms\n";

//void RC4(char*, int, char*, int);
//
//int main(int ArgC, char* Args[]) {
//	char Eingabe[] = "";
//	char Key[] = "";
//
//	RC4(Eingabe, sizeof(Eingabe), Key, sizeof(Key));
//
//	std::cout << Eingabe << "\n";
//
//	RC4(Eingabe, sizeof(Eingabe), Key, sizeof(Key));
//
//	std::cout << Eingabe << "\n";
//
//	return 0;
//}
//
//void RC4(char* Data, int DataLength, char* Key, int KeyLength)
//{
//	int S[256], J = 0, X = 0, I = 0, Y = 0;
//	int O = 0;
//	for(I = 0; I < 256; I++) S[I] = I;
//	
//	for(I = 0; I < 256; I++)
//	{
//		O = Key[I % KeyLength];
//		J = (J + S[I] + O) % 256;
//		X = S[I];
//		S[I] = S[J];
//		S[J] = X;
//	}
//	I = 0; J = 0;
//	for(Y = 0; Y < DataLength; Y++)
//	{
//		I = (I + 1) % 256;
//		J = (J + S[I]) % 256;
//		X = S[I];
//		S[I] = S[J];
//		S[J] = X;
//		Data[Y] = Data[Y] ^ char(S[(S[I] + S[J]) % 256]);
//	}
//	return;
//}





/*vector<string> str(0);

str.push_back(string("/home/user/Desktop/CppApplication_1/dist/Debug/GNU-Linux-x86/cppapplication_1(_Z15printStacktracev+0x2c) [0x8048e10]"));
str.push_back(string("/home/user/Desktop/CppApplication_1/dist/Debug/GNU-Linux-x86/cppapplication_1(main+0x1e) [0x8048ed4]"));
str.push_back(string("/lib/libc.so.6(__libc_start_main+0xe7) [0xdd6ce7]"));
str.push_back(string("/home/user/Desktop/CppApplication_1/dist/Debug/GNU-Linux-x86/cppapplication_1() [0x8048d51]"));


char *pRealName;
for (int x=0;x<=3;x++) {
for(int i=str[x].length()-1;i>0;i--) {
if (str[x][i] =='(') {str[x].erase(0,i+1); break;} 
if (str[x][i] ==')' || str[x][i] =='+') {
str[x].erase(i,str[x].length()-1);
} 
}

cout<<str[x]<<"\n";
}*/

//Poco::Stopwatch sw;
//double angle;
//Poco::Timestamp::TimeDiff first,second;

///* Normal */

//sw.start();
//for(int x=0;x<=50000000;x++) {
//	angle = cos(sin(cos(sin((double)x))));
//}
//sw.stop();
//cout<<"normal:"<< (first = sw.elapsed() / 1000) << "ms\n";


//sw.reset();
//sw.start();

///* OpenMP */ 

//#pragma omp parallel for  
//for(int x = 0;x<=50000000;x++) {
//	angle = cos(sin(cos(sin((double)x))));
//}
//sw.stop();
//cout<<"OpenMP:"<< (second = sw.elapsed() / 1000) << "ms\n";


//cout<<"faktor:"<< first/second<<"\n";


//#include <iostream>
//#include <fstream>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <Poco/URI.h>
//#include <Poco/Net/HTTPClientSession.h>
//#include <Poco/Net/HTTPRequest.h>
//#include <Poco/Net/HTTPResponse.h>
//#include <Poco/Net/StringPartSource.h>
//#include <Poco/DeflatingStream.h>
//#include <Poco/StreamCopier.h>
//#include <Poco/Net/HTMLForm.h>
//#include <Poco/Net/FilePartSource.h>
//#include <Poco/Net/PartHandler.h>
//#include <Poco/CountingStream.h>
//#include <Poco/NullStream.h>
//
//using namespace Poco::Net;
//using namespace Poco;
//
///*
//const unsigned char TestData[] =
//{
//#include "Logo.hpp"
//};
//*/
//class MyPartHandler: public Poco::Net::PartHandler
//{
//public:
//	MyPartHandler():
//		_length(0)
//	{
//	}
//
//	void handlePart(const MessageHeader& header, std::istream& stream)
//	{
//		/*_type = header.get("Content-Type", "image/jpg");
//		if (header.has("Content-Disposition"))
//		{
//		std::string disp;
//		NameValueCollection params;
//		MessageHeader::splitParameters(header["Content-Disposition"], disp, params);
//		_name = params.get("name", "file");
//		_fileName = params.get("filename", "file.jpg");
//		}
//
//		CountingInputStream istr(stream);
//		NullOutputStream ostr;
//		StreamCopier::copyStream(istr, ostr);
//		_length = istr.chars();
//		*/
//
//		_disp = header["Content-Disposition"]; 
//		_type = header["Content-Type"]; 
//		// read from stream and do something wit
//	}
//
//	int length() const
//	{
//		return _length;
//	}
//
//	const std::string& contentType() const
//	{
//		return _type;
//	}
//
//private:
//	int _length;
//	std::string _disp;
//	std::string _type; 
//};
//
//int main(int ArgC, char** Args[]){
//	Poco::URI Url("http://mf-i.co.de/index.php");
//	std::string Path(Url.getPathAndQuery());
//
//	Net::HTTPClientSession Session (Url.getHost(), Url.getPort());
//	Net::HTTPRequest Request (Poco::Net::HTTPRequest::HTTP_POST, Path, Poco::Net::HTTPMessage::HTTP_1_1);
//	Poco::Net::MessageHeader messageheader;
//	MyPartHandler parthandler;
//
//
//
//	Net::FilePartSource FilePart("C:\\Sniperava.jpg", "Sniperava.jpg", "image/jpg");
//	parthandler.handlePart(messageheader,FilePart.stream());
//
//
//	Poco::Net::HTMLForm Form(Request,FilePart.stream(),parthandler);
//
//
//	Form.prepareSubmit(Request);
//	Session.sendRequest(Request);
//
//
//
//	Net::HTTPResponse Response;
//	std::istream& ResponseIn = Session.receiveResponse(Response);
//	Poco::StreamCopier::copyStream(ResponseIn, std::cout);
//
//	return EXIT_SUCCESS;
//}
//
//
//
//
