#include "../src/log.h"
#include "../src/config.h"
#include <yaml-cpp/yaml.h>

zcserver::ConfigVar<int>::ptr g_int_value_config(zcserver::Config::Lookup("system.port", (int)8080, "system port"));

zcserver::ConfigVar<float>::ptr g_int_valuex_config(zcserver::Config::Lookup("system.port", (float)8080, "system port"));

zcserver::ConfigVar<float>::ptr g_float_value_config(zcserver::Config::Lookup("system.value", (float)10.2, "system value"));

zcserver::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config(zcserver::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vector"));

zcserver::ConfigVar<std::list<int>>::ptr g_int_list_value_config(zcserver::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int vector"));

zcserver::ConfigVar<std::set<int>>::ptr g_int_set_value_config(zcserver::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int set"));

zcserver::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config(zcserver::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2}, "system int uset"));

zcserver::ConfigVar<std::map<std::string, int>>::ptr g_str_int_map_value_config(zcserver::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k", 2}}, "system str int map"));

zcserver::ConfigVar<std::unordered_map<std::string, int>>::ptr g_str_int_umap_value_config(zcserver::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k", 2}}, "system str int umap"));

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
	// ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "before: " << g_float_value_config->toString();

#define XX(g_var, name, prefix) \
{ \
	auto &v = g_var->getValue(); \
	for (auto &i : v) \
	{ \
		ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << #prefix " " #name ": " << i; \
	} \
	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
} \

#define XX_M(g_var, name, prefix) \
{ \
	auto &v = g_var->getValue(); \
	for (auto &i : v) \
	{ \
		ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << #prefix " " #name ": {" \
		<< i.first << " - " << i.second << "}"; \
	} \
	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
} \

	XX(g_int_vec_value_config, int_vec, before);
	XX(g_int_list_value_config, int_list, before);
	XX(g_int_set_value_config, int_set, before);
	XX(g_int_uset_value_config, int_uset, before);
	XX_M(g_str_int_map_value_config, str_int_map, before);
	XX_M(g_str_int_umap_value_config, str_int_umap, before);



	YAML::Node root = YAML::LoadFile("C:/Users/98790/Desktop/zcserver/log.yml");
	zcserver::Config::LoadFromYaml(root);


	XX(g_int_vec_value_config, int_vec, after);
	XX(g_int_list_value_config, int_list, after);
	XX(g_int_set_value_config, int_set, after);
	XX(g_int_uset_value_config, int_uset, after);
	XX_M(g_str_int_map_value_config, str_int_map, after);
	XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

int main()
{
	// test_yaml();
	test_config();
	return 0;
}