#include "Config.h"
#include <gtest/gtest.h>

class ConfigTest : public ::testing::Test{	
protected:
	void SetUp() override{
		try{
			conf = new Config("Config_unittest.ini");
		}catch(File_Not_Found& e){
			ASSERT_NE(e.what(), nullptr) << "Config_unittest Open Failed !!";
		}
	}
	void TearDown() override{
		delete conf;
	}
	Config* conf;
};

TEST_F(ConfigTest, FileNotFound){
	try{
		Config conf("abc");
	}catch(File_Not_Found& e){
		std::string ex_ret = "Cannot find the file : abc";
		EXPECT_EQ(ex_ret, std::string(e.what()));
	}
}

TEST_F(ConfigTest, KeywordNotFound){
	try{
		std::string tmp;
		conf->Read("abc", tmp);
	}catch(Key_Not_Found& e){
		EXPECT_EQ(e.what(), std::string("Cannot find the keyword : abc"));
	}
}

TEST_F(ConfigTest, ReadString){
	std::string filename;
	conf->Read("STRING", filename);

	EXPECT_EQ(filename, std::string("Config_unittest.ini"));
}

TEST_F(ConfigTest, ReadInt){
	int var;
	conf->Read("INT", var);
	EXPECT_EQ(var, 10);
}

TEST_F(ConfigTest, ReadDouble){
	double var;
	conf->Read("DOUBLE", var);
	EXPECT_EQ(var, 3.14159);
}

TEST_F(ConfigTest, ReadBoolFalse){
	bool var_1;
	conf->Read("BOOL_FALSE", var_1);
	EXPECT_EQ(var_1, false) << "BOOL_FALSE";
	bool var_2;
	conf->Read("BOOL_NO", var_2);
	EXPECT_EQ(var_2, false) << "BOOL_NO";
	bool var_3;
	conf->Read("BOOL_NONE", var_3);
	EXPECT_EQ(var_3, false) << "BOOL_NONE";
	bool var_4;
	conf->Read("BOOL_CLOSE", var_4);
	EXPECT_EQ(var_4, false) << "BOOL_CLOSE";
	bool var_5;
	conf->Read("BOOL_ZERO", var_5);
	EXPECT_EQ(var_5, false) << "BOOL_ZERO";
	bool var_6;
	conf->Read("BOOL_OFF", var_6);
	EXPECT_EQ(var_6, false) << "BOOL_OFF";
}

TEST_F(ConfigTest, ReadBoolTrue){
	bool var_1;
	conf->Read("BOOL_TRUE", var_1);
	EXPECT_EQ(var_1, true) << "BOOL_TRUE";
	bool var_2;
	conf->Read("BOOL_ONE", var_2);
	EXPECT_EQ(var_2, true) << "BOOL_ONE";
	bool var_3;
	conf->Read("BOOL_OPEN", var_3);
	EXPECT_EQ(var_3, true) << "BOOL_OPEN";
	bool var_4;
	conf->Read("BOOL_ON", var_4);
	EXPECT_EQ(var_4, true) << "BOOL_ON";
}