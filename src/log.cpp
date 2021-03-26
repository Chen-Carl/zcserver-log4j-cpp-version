#include <iostream>
#include <stdio.h>
#include <map>
#include <functional>
#include <stdarg.h>
#include "util.h"
#include "log.h"

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

    /*********************************
     * class LogEvent
     *********************************/
    LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t tid, uint32_t fid, uint64_t time) : m_logger(logger), m_level(level), m_file(file), m_line(line), m_elapse(elapse), m_tid(tid), m_fid(fid), m_time(time) {}

    int LogEvent::vasprintf(char **strp, const char *fmt, va_list ap)
    {
        // _vscprintf tells you how big the buffer needs to be
        int len = _vscprintf(fmt, ap);
        if (len == -1)
        {
            return -1;
        }
        size_t size = (size_t)len + 1;
        char *str = (char*)malloc(size);
        if (!str)
        {
            return -1;
        }
        // _vsprintf_s is the "secure" version of vsprintf
        int r = vsprintf_s(str, len + 1, fmt, ap);
        if (r == -1)
        {
            free(str);
            return -1;
        }
        *strp = str;
        return r;
    }

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
            for (auto &i : m_appenders)
            {
                i->log(self, level, event);
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
            appender->setFormatter(m_formatter);
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

    void Logger::clearAppender()
    {
        m_appenders.clear();
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
     * class LoggerManager
     *********************************/
    LoggerManager::LoggerManager()
    {
        m_root.reset(new Logger);
        m_root->addAppender(std::shared_ptr<LogAppender>(new StdoutLogAppender));
    }

    std::shared_ptr<Logger> LoggerManager::getLogger(const std::string &name)
    {
        auto it = m_loggers.find(name);
        return it == m_loggers.end() ? m_root : it->second;
    }
}
