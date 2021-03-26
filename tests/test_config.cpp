#include "../src/log.h"
#include "../src/config.h"
// #include <yaml-cpp/yaml.h>

zcserver::ConfigVar<int>::ptr g_int_value_config(zcserver::Config::Lookup("system.port", (int)8080, "system port"));
zcserver::ConfigVar<float>::ptr g_float_value_config(zcserver::Config::Lookup("system.port", (float)10.2, "system value"));

// void test_yaml()
// {
//     YAML::Node root = YAML::LoadFile("../log.yml");
//     ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << root;
// }

int main()
{
    ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << g_int_value_config->getValue();
    ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << g_float_value_config->toString();
    
    return 0;
}