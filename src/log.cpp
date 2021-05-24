#include <iostream>
#include <stdio.h>
#include <map>
#include <functional>
#include <stdarg.h>
#include "util.h"
#include "config.h"

namespace zcserver
{
    /*********************************
     * class LogLevel
     *********************************/
    const char *LogLevel::ToString(LogLevel::Level level)
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
            break;
        case LogLevel::INFO:
            return "INFO";
            break;
        case LogLevel::WARN:
            return "WARN";
            break;
        case LogLevel::ERROR:
            return "ERROR";
            break;
        case LogLevel::FATAL:
            return "FATAL";
            break;
        default:
            return "UNKNOWN";
        }
        return "UNKNOWN";
    }

    LogLevel::Level LogLevel::FromString(const std::string &str)
    {
#define XX(level, v)            \
    if (str == #v)              \
    {                           \
        return LogLevel::level; \
    }

        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        return LogLevel::UNKNOWN;
#undef XX
    }

    /*********************************
     * class LogEvent
     *********************************/
    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t tid, uint32_t fid, uint64_t time) : m_logger(logger), m_level(level), m_file(file), m_line(line), m_elapse(elapse), m_tid(tid), m_fid(fid), m_time(time) {}

    void LogEvent::format(const char *fmt, ...)
    {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char *fmt, va_list al)
    {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1)
        {
            m_ss << std::string(buf, 10);
            free(buf);
        }
    }

    /*********************************
     * class LogFormatter
     *********************************/
    LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern)
    {
        init();
    }

    void LogFormatter::init()
    {
        // tuple format: (str, format, type)
        std::vector<std::tuple<std::string, std::string, int>> vec;
        std::string nstr;
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            // find '%'
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }

            // m_pattern[i] is not '%'
            if ((i + 1) < m_pattern.size())
            {
                if (m_pattern[i + 1] == '%')
                {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;

            std::string str;
            std::string fmt;
            while (n < m_pattern.size())
            {
                if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }
                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1; //解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                else if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        //std::cout << "#" << fmt << std::endl;
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if (n == m_pattern.size())
                {
                    if (str.empty())
                    {
                        str = m_pattern.substr(i + 1);
                    }
                }
            }

            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(std::make_tuple(nstr, std::string(), 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(str, fmt, 1));
                i = n - 1;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
                m_error = true;
            }
        }

        if (!nstr.empty())
        {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }

        // map: string -> function<ptr FormatItem(const std::string &str)>
        static std::map<std::string, std::function<std::shared_ptr<FormatItem>(const std::string &str)>> s_format_items = {
            {"m", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new MessageFormatItem(fmt)); }},
            {"p", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new LevelFormatItem(fmt)); }},
            {"r", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new ElapseFormatItem(fmt)); }},
            {"c", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new NameFormatItem(fmt)); }},
            {"t", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new ThreadIdFormatItem(fmt)); }},
            {"n", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new NewLineFormatItem(fmt)); }},
            {"d", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new DateTimeFormatItem(fmt)); }},
            {"f", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new FilenameFormatItem(fmt)); }},
            {"l", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new LineFormatItem(fmt)); }},
            {"T", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new TabFormatItem(fmt)); }},
            {"F", [](const std::string &fmt) { return std::shared_ptr<FormatItem>(new FiberIdFormatItem(fmt)); }}};

        for (auto &i : vec)
        {
            if (std::get<2>(i) == 0)
            {
                m_items.push_back(std::shared_ptr<FormatItem>(new StringFormatItem(std::get<0>(i))));
            }
            else
            {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end())
                {
                    m_items.push_back(std::shared_ptr<FormatItem>(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    m_error = true;
                }
                else
                {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }
        }
    }

    std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event)
    {
        std::stringstream ss;
        for (auto &i : m_items)
        {
            // using subclass method "format"
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }

    /*********************************
     * class StdoutLogAppender
     *********************************/
    void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event)
    {
        if (level >= m_level)
        {
            std::cout << m_formatter->format(logger, level, event);
        }
    }

    std::string StdoutLogAppender::toYamlString()
    {
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        if (m_level != LogLevel::UNKNOWN)
            node["level"] = LogLevel::ToString(m_level);
        if (m_hasFormatter && m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void LogAppender::setFormatter(std::shared_ptr<LogFormatter> val) 
    {
        m_formatter = val;
        if (m_formatter)
            m_hasFormatter = true;
        else
            m_hasFormatter = false;
    }


    /*********************************
     * class FileLogAppender
     *********************************/
    FileLogAppender::FileLogAppender(const std::string &filename)
        : m_filename(filename)
    {
        reopen();
    }

    bool FileLogAppender::reopen()
    {
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename, std::ios::app);
        return !!m_filestream;
    }

    void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event)
    {
        if (level >= m_level)
        {
            m_filestream << m_formatter->format(logger, level, event);
        }
    }

    std::string FileLogAppender::toYamlString()
    {
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        if (m_level != LogLevel::UNKNOWN)
            node["level"] = LogLevel::ToString(m_level);
        if (m_hasFormatter && m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }


    /*********************************
     * class Logger
     *********************************/
    Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::DEBUG)
    {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

    void Logger::log(LogLevel::Level level, std::shared_ptr<LogEvent> event)
    {
        if (level >= m_level)
        {
            auto self = shared_from_this();
            if (!m_appenders.empty())
            {
                for (auto &i : m_appenders)
                {
                    i->log(self, level, event);
                }
            }
            // if logAppender is empty, use m_root
            else if (m_root)
            {
                m_root->log(level, event);
            }
        }
    }

    void Logger::debug(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::INFO, event);
    }
    void Logger::warn(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::WARN, event);
    }
    void Logger::error(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(std::shared_ptr<LogEvent> event)
    {
        log(LogLevel::FATAL, event);
    }

    void Logger::addAppender(std::shared_ptr<LogAppender> appender)
    {
        if (!appender->getFormatter())
        {
            // friend class
            // set appender without setting the m_hasFormatter
            appender->m_formatter = m_formatter;
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(std::shared_ptr<LogAppender> appender)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppenders()
    {
        m_appenders.clear();
    }

    void Logger::setFormatter(std::shared_ptr<LogFormatter> val)
    {
        m_formatter = val;
        
        // if there is no formatter set in advance,
        // it will use root formatter
        // so when modifying the formatter, it should influence the formatter of appenders
        for (auto &i : m_appenders)
        {
            if (!i->m_hasFormatter)
            {
                i->m_formatter = m_formatter;
            }
        }
    }

    void Logger::setFormatter(const std::string &val)
    {
        std::shared_ptr<LogFormatter> new_val(new LogFormatter(val));
        if (new_val->isError())
        {
            std::cout << "Logger setFormatter name=" << m_name
                      << " value=" << val << " invalid formatter"
                      << std::endl;
            return;
        }
        setFormatter(new_val);
    }

    std::shared_ptr<LogFormatter> Logger::getFormatter()
    {
        return m_formatter;
    }

    std::string Logger::toYamlString()
    {
        YAML::Node node;
        node["name"] = m_name;
        if (m_level != LogLevel::UNKNOWN)
            node["level"] = LogLevel::ToString(m_level);
        if (m_formatter)
            node["formatter"] = m_formatter->getPattern();
        
        for (auto &i : m_appenders)
        {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    /*********************************
     * class LogEventWrap
     *********************************/
    LogEventWrap::LogEventWrap(std::shared_ptr<LogEvent> event) : m_event(event) {}

    LogEventWrap::~LogEventWrap()
    {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }

    std::stringstream &LogEventWrap::getSS()
    {
        return m_event->getSS();
    }

    /*********************************
     * class LoggerManager and yaml input
     *********************************/
    LoggerManager::LoggerManager()
    {
        m_root.reset(new Logger);
        m_root->addAppender(std::shared_ptr<LogAppender>(new StdoutLogAppender));

        m_loggers[m_root->m_name] = m_root;
    }

    std::string LoggerManager::toYamlString()
    {
        YAML::Node node;
        for (auto &i : m_loggers)
        {
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }


    // config items from yaml
    struct LogAppenderDefine
    {
        // 1 Fileout
        // 2 Stdout
        int type = 0;
        LogLevel::Level level = LogLevel::UNKNOWN;
        std::string formatter;
        std::string file;

        bool operator==(const LogAppenderDefine &oth) const
        {
            return type == oth.type && level == oth.level && formatter == oth.formatter && file == oth.file;
        }
    };

    // LogDefine is a logger record in the yaml file
    // including a number of LogAppender definitions
    // which stored in a vector
    struct LogDefine
    {
        std::string name;
        LogLevel::Level level = LogLevel::UNKNOWN;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogDefine &oth) const
        {
            return name == oth.name && level == oth.level && formatter == oth.formatter && appenders == oth.appenders;
        }

        bool operator<(const LogDefine &oth) const
        {
            return name < oth.name;
        }
    };

    // paritial specialize
    template <>
    class LexicalCast<std::string, std::set<LogDefine>>
    {
    public:
        std::set<LogDefine> operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            std::set<LogDefine> vec;
            for (size_t i = 0; i < node.size(); i++)
            {
                // problem 1:
                // wrong: auto &n = node[i]
                auto n = node[i];
                if (!n["name"].IsDefined())
                {
                    std::cout << "log config error: name is null, node at " << n
                              << std::endl;
                    continue;
                }

                LogDefine ld;
                ld.name = n["name"].as<std::string>();
                ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
                if (n["formatter"].IsDefined())
                {
                    ld.formatter = n["formatter"].as<std::string>();
                }

                if (n["appenders"].IsDefined())
                {
                    for (size_t x = 0; x < n["appenders"].size(); x++)
                    {
                        auto a = n["appenders"][x];
                        if (!a["type"].IsDefined())
                        {
                            std::cout << "log config error: appender type is null, node at " << a << std::endl;
                            continue;
                        }
                        std::string type = a["type"].as<std::string>();
                        LogAppenderDefine lad;
                        if (a["level"].IsDefined())
                        {
                            lad.level = LogLevel::FromString(a["level"].as<std::string>());
                        }
                        if (type == "FileLogAppender")
                        {
                            lad.type = 1;
                            if (!a["file"].IsDefined())
                            {
                                std::cout << "log config error: file is null, node at " << a << std::endl;
                                continue;
                            }
                            lad.file = a["file"].as<std::string>();
                            if (a["formatter"].IsDefined())
                            {
                                lad.formatter = a["formatter"].as<std::string>();
                            }
                        }
                        else if (type == "StdoutLogAppender")
                        {
                            lad.type = 2;
                        }
                        else
                        {
                            std::cout << "log config error: appender type is invalid, node at " << a << std::endl;
                            continue;
                        }
                        ld.appenders.push_back(lad);
                    }
                }
                vec.insert(ld);
            }
            return vec;
        }
    };

    template <>
    class LexicalCast<std::set<LogDefine>, std::string>
    {
    public:
        std::string operator()(const std::set<LogDefine> &v)
        {
            YAML::Node node;
            for (auto &i : v)
            {
                YAML::Node n;
                n["name"] = i.name;
                if (i.level != LogLevel::UNKNOWN)
                {
                    n["level"] = LogLevel::ToString(i.level);
                }
                if (!i.formatter.empty())
                {
                    n["formatter"] = i.formatter;
                }

                for (auto &a : i.appenders)
                {
                    YAML::Node na;
                    if (a.type == 1)
                    {
                        na["type"] = "FileLogAppender";
                        na["file"] = a.file;
                    }
                    else if (a.type == 2)
                    {
                        na["type"] = "StdoutLogAppender";
                    }
                    if (a.level != LogLevel::UNKNOWN)
                    {
                        na["level"] = LogLevel::ToString(a.level);
                    }

                    if (!a.formatter.empty())
                    {
                        na["formatter"] = a.formatter;
                    }

                    n["appenders"].push_back(na);
                }
                node.push_back(n);
            }
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    // before main
    // a config item is a set of LogDefine with different names
    // std::set is appropriate

    ConfigVar<std::set<LogDefine>>::ptr g_log_defines = Config::Lookup("logs", std::set<LogDefine>(), "logs config");

    // global listener
    struct LogIniter
    {
        LogIniter()
        {
            g_log_defines->addListener(0xF1E231, [](const std::set<LogDefine> &old_value, const std::set<LogDefine> &new_value) {
                ZCSERVER_LOG_INFO(ZCSERVER_LOG_ROOT()) << "on logger config changed";
                // add a logger
                for (auto &i : new_value)
                {
                    std::shared_ptr<Logger> logger;
                    auto it = old_value.find(i);

                    // it is a new logger
                    if (it == old_value.end())
                    {
                        logger = ZCSERVER_LOG_NAME(i.name);
                    }
                    else
                    {
                        // modified logger
                        if (!(i == *it))
                            logger = ZCSERVER_LOG_NAME(i.name);
                    }

                    // common operator
                    // setLevel, setFormatter, setAppenders
                    logger->setLevel(i.level);
                    if (!i.formatter.empty())
                    {
                        logger->setFormatter(i.formatter);
                    }

                    // add appenders
                    logger->clearAppenders();
                    for (auto &a : i.appenders)
                    {
                        std::shared_ptr<LogAppender> ap;
                        if (a.type == 1)
                        {
                            ap.reset(new FileLogAppender(a.file));
                        }
                        else if (a.type == 2)
                        {
                            ap.reset(new StdoutLogAppender);
                        }
                        ap->setLevel(a.level);
                        if (!a.formatter.empty())
                        {
                            std::shared_ptr<LogFormatter> fmt(new LogFormatter(a.formatter));
                            if (!fmt->isError())
                            {
                                ap->setFormatter(fmt);
                            }
                            else
                            {
                                std::cout << "log.name=" << i.name << " appender type=" << a.type << " formatter=" << a.formatter << " is invalid" << std::endl;
                            }
                        }
                        logger->addAppender(ap);
                    }
                }

                // delete a logger
                for (auto &i : old_value)
                {
                    auto it = new_value.find(i);
                    if (it == new_value.end())
                    {
                        // delete all appenders
                        // set LogLevel to a highest level
                        // equivalent to delete the logger
                        auto logger = ZCSERVER_LOG_NAME(i.name);
                        logger->setLevel((LogLevel::Level)100);
                        logger->clearAppenders();
                    }
                }
            });
        }
    };

    static LogIniter __log_init;

    std::shared_ptr<Logger> LoggerManager::getLogger(const std::string &name)
    {
        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
            return it->second;
        // if not exist, create a new logger
        std::shared_ptr<Logger> logger(new Logger(name));
        // empty logAppender, using root instead
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }
}
