#include "../src/log.h"
#include "../src/config.h"
#include <yaml-cpp/yaml.h>

zcserver::ConfigVar<int>::ptr g_int_value_config(zcserver::Config::Lookup("system.port", (int)8080, "system port"));

// log error
// zcserver::ConfigVar<float>::ptr g_int_valuex_config(zcserver::Config::Lookup("system.port", (float)8080, "system port"));

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
	YAML::Node root = YAML::LoadFile("C:/Users/98790/Desktop/zcserver/test.yml");

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



	YAML::Node root = YAML::LoadFile("C:/Users/98790/Desktop/zcserver/test.yml");
	zcserver::Config::LoadFromYaml(root);


	XX(g_int_vec_value_config, int_vec, after);
	XX(g_int_list_value_config, int_list, after);
	XX(g_int_set_value_config, int_set, after);
	XX(g_int_uset_value_config, int_uset, after);
	XX_M(g_str_int_map_value_config, str_int_map, after);
	XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

class Person
{
public:
	Person() {}
	std::string m_name;
	int m_age = 0;
	bool m_sex = 0;
	std::string toString() const 
	{
		std::stringstream ss;
		ss << "{Person name=" << m_name
		   << " age=" << m_age
		   << " sex=" << m_sex
		   << "}";
		return ss.str();
	}

	bool operator==(const Person &oth) const
	{
		return m_name == oth.m_name && m_age == oth.m_age && m_sex == oth.m_sex;
	}
};

namespace zcserver
{
	// specifically transfer std::string to std::list<T>
	template <>
	class LexicalCast<std::string, Person>
	{
	public:
		Person operator()(const std::string &v)
		{
			YAML::Node node = YAML::Load(v);
			Person p;
			p.m_name = node["name"].as<std::string>();
			p.m_age = node["age"].as<int>();
			p.m_sex = node["sex"].as<bool>();
			return p;
		}
	};

	// specifically transfer std::list<T> to std::string
	template <>
	class LexicalCast<Person, std::string>
	{
	public:
		std::string operator()(const Person &p)
		{
			YAML::Node node;
			node["name"] = p.m_name;
			node["age"] = p.m_age;
			node["sex"] = p.m_sex;
			std::stringstream ss;
			ss << node;
			return ss.str();
		}
	};
}

zcserver::ConfigVar<Person>::ptr g_person = zcserver::Config::Lookup("class.person", Person(), "system person");


zcserver::ConfigVar<std::map<std::string, Person>>::ptr g_person_map = zcserver::Config::Lookup("class.map", std::map<std::string, Person>(), "system person");

zcserver::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr g_person_vec_map = zcserver::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person>>(), "system person");

void test_class()
{
	// ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
	
#define XX_PM(g_var, prefix) \
{ \
	auto m = g_var->getValue(); \
	for (auto & i : m) \
	{ \
		ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
	} \
	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << prefix << ": size=" << m.size(); \
} \

	g_person->addListener(10, [](const Person &old_value, const Person &new_value){
		ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "old_value=" << old_value.toString() << " new_value=" << new_value.toString();
	});

	XX_PM(g_person_map, "class.map before: ");

	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "before: " << g_person_vec_map->toString();

	YAML::Node root = YAML::LoadFile("C:/Users/98790/Desktop/zcserver/test.yml");
	zcserver::Config::LoadFromYaml(root);

	// ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();

	XX_PM(g_person_map, "class.map after: ");
	ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "after: " << g_person_vec_map->toString();
}

void test_log()
{
	std::cout << zcserver::LoggerMgr::GetInstance()->toYamlString() << std::endl;
	YAML::Node root = YAML::LoadFile("C:/Users/98790/Desktop/zcserver/log.yml");
	// Config::LoadFromYaml intrigues a series of modifications of the Log config, when "void setValue(const T &v)" is called to call a call-back function, outputing "on logger config changed" log info.
	zcserver::Config::LoadFromYaml(root);

	std::cout << " ==================== " << std::endl;
	std::cout << zcserver::LoggerMgr::GetInstance()->toYamlString() << std::endl;
}

int main()
{
	// test_yaml();
	// test_config();
	// test_class();
	test_log();
	return 0;
}