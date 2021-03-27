#include "../src/log.h"
#include "../src/config.h"
#include <yaml-cpp/yaml.h>

zcserver::ConfigVar<int>::ptr g_int_value_config(zcserver::Config::Lookup("system.port", (int)8080, "system port"));
zcserver::ConfigVar<float>::ptr g_float_value_config(zcserver::Config::Lookup("system.port", (float)10.2, "system value"));

void print_yaml(YAML::Node node, int level)
{
	if (node.IsScalar())
	{
		ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
	}
	else if (node.IsNull())
	{
		ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL - " << node.Type() << " - " << level;
	}
	else if (node.IsMap())
	{
		for (auto it = node.begin(); it != node.end(); it++)
		{
			ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
			print_yaml(it->second, level + 1);
		}
	}
	else if (node.IsSequence())
	{
		for (size_t i = 0; i < node.size(); i++)
		{
			ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
			print_yaml(node[i], level + 1);
		}
	}
}

void test_yaml()
{
	YAML::Node root = YAML::LoadFile("C:/Users/98790/Desktop/zcserver/log.yml");

	print_yaml(root, 0);
	
	// ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << root;
}

void test_config()
{
	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "before: " << g_float_value_config->toString();

	YAML::Node root = YAML::LoadFile("C:/Users/98790/Desktop/zcserver/log.yml");
	zcserver::Config::LoadFromYaml(root);

	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "after: " << g_float_value_config->toString();


}

int main()
{
	// test_yaml();
	test_config();
	return 0;
}