#include "config.h"

namespace zcserver
{
    // initialize static member s_datas
    Config::ConfigVarMap Config::s_datas;

    ConfigVarBase::ptr Config::LookupBase(const std::string &name)
    {
        auto it = s_datas.find(name);
        return it == s_datas.end() ? nullptr : it->second;
    }

    // List all members in a yaml node and store them to the output list
    // output list is a pair consists of (string, YAML::Node)
    static void ListAllMember(const std::string &prefix, const YAML::Node &node, std::list<std::pair<std::string, const YAML::Node>> &output)
    {
        if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
        {
            ZCSERVER_LOG_ERROR(ZCSERVER_LOG_ROOT()) << "Config invalid name: " << prefix << ": " << node;
            return;
        }
        output.push_back(std::make_pair(prefix, node));
        if (node.IsMap())
        {
            for (auto it = node.begin(); it != node.end(); it++)
            {
                ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
            }
        }
    }

    void Config::LoadFromYaml(const YAML::Node &root)
    {
        // create a list to store all nodes
        std::list<std::pair<std::string, const YAML::Node>> all_nodes;
        // store the node in all_nodes
        // all_nodes (string, YAML::Node)
        ListAllMember("", root, all_nodes);

        for (auto &i : all_nodes)
        {
            std::string key = i.first;
            // ignore the empty key
            if (key.empty())
            {
                continue;
            }
            // Key is a string as a name. Transform the name into lower case.
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);

            // Check whether the key is already in s_datas
            // var is a pointer to a s_datas entry
            ConfigVarBase::ptr var = LookupBase(key);
            // if exists
            if (var)
            {
                // common scalar
                if (i.second.IsScalar())
                {
                    // modify the m_val of the ConfigVar pointed by pointer var
                    var->fromString(i.second.Scalar());
                }
                else
                {
                    // complex type
                    std::stringstream ss;
                    ss << i.second;
                    var->fromString(ss.str());
                }
            }
        }
    }
}