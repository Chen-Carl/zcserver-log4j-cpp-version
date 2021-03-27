// Config System

#ifndef __ZCSERVER_CONFIG_H__
#define __ZCSERVER_CONFIG_H__

#include <memory>
#include <sstream>
#include <string>
#include <algorithm>
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

    // subclass template
    /*
        A ConfigVar contains name, value, description
    */
    template <class T>
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
                return boost::lexical_cast<std::string>(m_val);
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
                m_val = boost::lexical_cast<T>(val);
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