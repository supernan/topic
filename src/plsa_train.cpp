/*************************************************************************
 > File Name: main.cpp
 > Author: zhounan
 > Mail: scutzhounan@foxmail.com 
 > Created Time: 2015年07月26日 星期日 09时56分02秒
 ************************************************************************/

#include "plsa.h"
#include "tinyxml.h"
#include "tinystr.h"
#include<iostream>
using namespace std;

int main(int argc, char **argv)
{
	omp_set_num_threads(4);
	google::InitGoogleLogging(argv[0]);
	google::SetStderrLogging(google::ERROR);
	google::SetLogDestination(google::GLOG_INFO,"../log/");
	FLAGS_colorlogtostderr=true;
	string conf_path = "../conf/plsa_conf.xml";
	nlp::Plsa plsa(conf_path);
	plsa.train_plsa();
	return 0;
}
