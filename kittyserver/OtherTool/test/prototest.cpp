#include "test.pb.h"
#include <fstream>
#include <iostream>
using namespace std;

/*
int main(int argc,char *argv[])
{
	Test u;
	u.set_name("zhangshan");
	u.set_id(1001);
	u.set_opt("654321");
	fstream output("user.pb",ios::out | ios::trunc | ios::binary);
	if (!u.SerializeToOstream(&output))
	{
		cerr<<"Failed to write user info"<<endl;
		return -1;
	}
	else
	{
		cout<<"Success to write user info"<<endl;
	}

	return 0;
}
*/


int main(int argc,char *argv[])
{
	Test u;
	fstream input("user.pb",ios::in | ios::binary);
	if (!u.ParseFromIstream(&input))
	{
		cerr<<"Failed to parse user info"<<endl;
	}
	else
	{
		cout<<"id="<<u.id()<<endl;
		cout<<"name="<<u.name()<<endl;
	}
	return 0;
}

