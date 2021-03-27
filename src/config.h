// Config System

#ifndef __ZCSERVER_CONFIG_H__
#define __ZCSERVER_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include "log.h"

namespace zcserver
{
    class ConfigVarBase
    {
    public:
        // type  std::shared_ptr<ConfigBarBase>
        typedef std::shared_ptr<ConfigVarBase> ptr;
        // constructor
        ConfigVarBase(const std::string &name, const std::string &description) : m_name(name), m_description(description) 
        {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }

        // virtual deconstructor of base class
        virtual ~ConfigVarBase() {}

        const std::string &getName() const { return m_name; }
        const std::string &getDescription() const { return m_description; }

        virtual std::string toString() = 0;
        // parse protected information from a string
        virtual bool fromString(const std::string &val) = 0;

    protected:
        std::string m_name;
        std::string m_description;
    };

    /*
        F: From type
        T: To type
    */
    template <class F, class T>
    class LexicalCast
    {
    public:
        T operator()(const F &v)
        {
            return boost::lexical_cast<T>(v);
        }
    };

    // partial specification 偏特化
    // specifically transfer std::string to std::vector<T>
    template <class T>
    class LexicalCast<std::string, std::vector<T>>
    {
    public:
        std::vector<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::vector<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); i++)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            // equivalent to std::move(vec)
            // After C++11, the complier will optimize itself.
            return vec;
        }
    };

    // specifically transfer std::vector<T> to std::string
    template <class T>
    class LexicalCast<std::vector<T>, std::string>
    {
    public:
        std::string operator()(const std::vector<T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                // using YAML::Load to generate a Node
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // specifically transfer std::string to std::list<T>
    template <class T>
    class LexicalCast<std::string, std::list<T>>
    {
    public:
        std::list<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::list<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); i++)
            {
                ss.str("");
                ss << node[i];
                vec.push_back(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    // specifically transfer std::list<T> to std::string
    template <class T>
    class LexicalCast<std::list<T>, std::string>
    {
    public:
        std::string operator()(const std::list<T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                // using YAML::Load to generate a Node
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // specifically transfer std::string to std::set<T>
    template <class T>
    class LexicalCast<std::string, std::set<T>>
    {
    public:
        std::set<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::set<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); i++)
            {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    // specifically transfer std::set<T> to std::string
    template <class T>
    class LexicalCast<std::set<T>, std::string>
    {
    public:
        std::string operator()(const std::set<T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                // using YAML::Load to generate a Node
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // specifically transfer std::string to std::unordered_set<T>
    template <class T>
    class LexicalCast<std::string, std::unordered_set<T>>
    {
    public:
        std::unordered_set<T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_set<T> vec;
            std::stringstream ss;
            for (size_t i = 0; i < node.size(); i++)
            {
                ss.str("");
                ss << node[i];
                vec.insert(LexicalCast<std::string, T>()(ss.str()));
            }
            return vec;
        }
    };

    // specifically transfer std::unordered_set<T> to std::string
    template <class T>
    class LexicalCast<std::unordered_set<T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_set<T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                // using YAML::Load to generate a Node
                node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // specifically transfer std::string to std::map<T>
    template <class T>
    class LexicalCast<std::string, std::map<std::string, T>>
    {
    public:
        std::map<std::string, T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::map<std::string, T> vec;
            std::stringstream ss;
            for (auto it = node.begin(); it != node.end(); it++)
            {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };

    // specifically transfer std::map<T> to std::string
    template <class T>
    class LexicalCast<std::map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::map<std::string, T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                // using YAML::Load to generate a Node
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // specifically transfer std::string to std::unordered_map<T>
    template <class T>
    class LexicalCast<std::string, std::unordered_map<std::string, T>>
    {
    public:
        std::unordered_map<std::string, T> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            typename std::unordered_map<std::string, T> vec;
            std::stringstream ss;
            for (auto it = node.begin(); it != node.end(); it++)
            {
                ss.str("");
                ss << it->second;
                vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
            }
            return vec;
        }
    };

    // specifically transfer std::map<T> to std::string
    template <class T>
    class LexicalCast<std::unordered_map<std::string, T>, std::string>
    {
    public:
        std::string operator()(const std::unordered_map<std::string, T> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                // using YAML::Load to generate a Node
                node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // subclass template
    /*
        A ConfigVar contains name, value, description.

        Support generic type.
        The main idea is to realize a functional object and 2 methods
            FromStr T operator()(const std::string &)
            ToStr std::string operator()(const T&)
    */
    template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase
    {
    private:
        T m_val;

    public:
        // shared pointer
        typedef std::shared_ptr<ConfigVar> ptr;
        // constructor
        ConfigVar(const std::string &name, const T &default_value, const std::string description = "") : ConfigVarBase(name, description), m_val(default_value) {}

        // succeed from father class's virtual method
        // for testing or debugging, change m_val to string
        std::string toString() override
        {
            try
            {
                // return boost::lexical_cast<std::string>(m_val);
                return ToStr()(m_val);
            }
            catch (std::exception &e)
            {
                ZCSERVER_LOG_ERROR(ZCSERVER_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() << " convert: " << typeid(m_val).name() << "to string";
            }
            return "";
        }

        // read m_val from a string
        bool fromString(const std::string &val)
        {
            try
            {
                // m_val = boost::lexical_cast<T>(val);
                setValue(FromStr()(val));
            }
            catch(std::exception& e)
            {
                ZCSERVER_LOG_ERROR(ZCSERVER_LOG_ROOT()) << "ConfigVar::toString exception" << e.what() << " convert: string to" << typeid(m_val).name();
            }
            return false;
        }

        const T getValue() const { return m_val; }
        void setValue(const T &v) { m_val = v; }
    };

    /*
        Class Config is a collection of ConfigVar
        ConfigVarMap is a dict: use name to find ConfigVarBase::ptr
    */

    class Config
    {
    public:
        typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    public:
        /*
            Look up for a ConfigVar with the certain name. If the name does not exist, create a new ConfigVar and add it to s_datas.
        */
        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name, const T &default_value, const std::string &description = "")
        {
            auto tmp = Lookup<T>(name);
            if (tmp)
            {
                ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "Lookup name = " << name << " exists";
                return tmp;
            }

            if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
            {
                ZCSERVER_LOG_ERROR(ZCSERVER_LOG_ROOT()) << "Lookup name invalid " << name;
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
            s_datas[name] = v;
            return v;
        }

        template <class T>
        static typename ConfigVar<T>::ptr Lookup(const std::string &name)
        {
            auto it = s_datas.find(name);
            if (it == s_datas.end())
                return nullptr;
            return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
        }

        // Lookup returns a ConfigVar<T>::ptr
        // LookupBase returns a ConfigVarBase::ptr
        static ConfigVarBase::ptr LookupBase(const std::string &name);
        static void LoadFromYaml(const YAML::Node &root);

    private:
        // string -> ConfigVarBase::ptr
        static ConfigVarMap s_datas;
        
    };
}

#endif