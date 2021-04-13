/*
    Instructions:
        1. in advance
        - Create LogAppender with its own LogFormatter and its own log level;

        2. when output a log
        - Use LogEvent to wrap a log event's information that is also related to a logger;
        - The logger contains multiple of LogAppenders to process different output places.
*/

#ifndef __ZCSERVER_LOG_H__
#define __ZCSERVER_LOG_H__

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <ctime>
#include <map>
#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>
#include "util.h"
#include "singleton.h"

/*********************************
 * output definitions
 *********************************/

#define ZCSERVER_LOG_LEVEL(logger, level)   \
    if (logger->getLevel() <= level)        \
        zcserver::LogEventWrap(std::shared_ptr<zcserver::LogEvent>(new zcserver::LogEvent(logger, level,                     \
        __FILE__, __LINE__, 0, zcserver::GetFiberId(), zcserver::GetFiberId(), time(0)))).getSS() 

#define ZCSERVER_LOG_DEBUG(logger) ZCSERVER_LOG_LEVEL(logger, zcserver::LogLevel::DEBUG)
#define ZCSERVER_LOG_INFO(logger) ZCSERVER_LOG_LEVEL(logger, zcserver::LogLevel::INFO)
#define ZCSERVER_LOG_WARN(logger) ZCSERVER_LOG_LEVEL(logger, zcserver::LogLevel::WARN)
#define ZCSERVER_LOG_ERROR(logger) ZCSERVER_LOG_LEVEL(logger, zcserver::LogLevel::ERROR)
#define ZCSERVER_LOG_FATAL(logger) ZCSERVER_LOG_LEVEL(logger, zcserver::LogLevel::FATAL)

#define ZCSERVER_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        zcserver::LogEventWrap(std::shared_ptr<zcserver::LogEvent>(new zcserver::LogEvent(logger, level, \
        __FILE__, __LINE__, 0, zcserver::GetThreadId(),\
        zcserver::GetFiberId(), time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define ZCSERVER_LOG_FMT_DEBUG(logger, fmt, ...) ZCSERVER_LOG_FMT_LEVEL(logger, zcserver::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define ZCSERVER_LOG_FMT_INFO(logger, fmt, ...) ZCSERVER_LOG_FMT_LEVEL(logger, zcserver::LogLevel::INFO, fmt, __VA_ARGS__)
#define ZCSERVER_LOG_FMT_WARN(logger, fmt, ...) ZCSERVER_LOG_FMT_LEVEL(logger, zcserver::LogLevel::WARN, fmt, __VA_ARGS__)
#define ZCSERVER_LOG_FMT_ERROR(logger, fmt, ...) ZCSERVER_LOG_FMT_LEVEL(logger, zcserver::LogLevel::ERROR, fmt, __VA_ARGS__)
#define ZCSERVER_LOG_FMT_FATAL(logger, fmt, ...) ZCSERVER_LOG_FMT_LEVEL(logger, zcserver::LogLevel::FATAL, fmt, __VA_ARGS__)

#define ZCSERVER_LOG_ROOT() zcserver::LoggerMgr::GetInstance()->getRoot()
#define ZCSERVER_LOG_NAME(name) zcserver::LoggerMgr::GetInstance()->getLogger(name)

namespace zcserver
{
    class Logger;
    class LoggerManager;

    // log level
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        // static method: transform the enum Level to a string
        static const char *ToString(LogLevel::Level level);
        // static method: transform the string to an enum Level
        static LogLevel::Level FromString(const std::string &str);
    };

    // a wrapper for the information of a log event
    class LogEvent
    {
    private:
        std::string m_threadName;         // thread name
        std::stringstream m_ss;           // thread information
        std::shared_ptr<Logger> m_logger; // related Logger
        LogLevel::Level m_level;          // log level
        const char *m_file = nullptr;
        int32_t m_line = 0;               // line number
        uint32_t m_elapse = 0;            // running time
        uint32_t m_tid = 0;               // thread id
        uint32_t m_fid = 0;               // fiber id
        uint64_t m_time = 0;              // time


    public:
        LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char *file, int32_t line, uint32_t elapse, uint32_t tid, uint32_t fid, uint64_t time);

        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_tid; }
        uint32_t getFiberId() const { return m_fid; }
        uint64_t getTime() const { return m_time; }
        const std::string &getThreadName() const { return m_threadName; }
        std::string getContent() const { return m_ss.str(); }
        std::shared_ptr<Logger> getLogger() const { return m_logger; }
        LogLevel::Level getLevel() const { return m_level; }
        std::stringstream &getSS() { return m_ss; }

        void format(const char *fmt, ...);
        void format(const char *fmt, va_list al);
        int vasprintf(char **strp, const char *fmt, va_list al);
    };

    /*
        LogFormatter:
            Create a pattern to output the log information.
            The pattern is a string that contains certain charater.

            %d output a time
                The time format string is optional. For example, a valid format string is 
                {%Y-%m-%d %H:%M:%S}
            %T symbol Tab
            %t thread id
            %N thread name
            %F fiber id
            %p log level
            %c log name
            %f file name
            %l line number
            %m log content
            %n symbol \n
    */
    class LogFormatter
    {
    public:
        // public class in LogFormatter
        /*
            m_pattern contains different types of FormatItem. We need to parse the m_pattern into more accurate items and store them in a vector.
        */
        class FormatItem
        {
        public:
            virtual ~FormatItem() {}
            // stream-oriented output
            virtual void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) = 0;
        };

    private:
        std::string m_pattern;                            // format pattern
        std::vector<std::shared_ptr<FormatItem>> m_items; // store items

        bool m_error = false;

        // parse the m_pattern
        // used in the constructor
        void init();

    public:
        LogFormatter(const std::string &pattern);

        // for each format item(m_items), use subclass method format() to output
        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event);

        bool isError() const { return m_error; }
        const std::string getPattern() const { return m_pattern; }
    };

    // LogAppender defines the places to receive outputs
    class LogAppender
    {
    friend class Logger;
    protected:
        LogLevel::Level m_level = LogLevel::DEBUG;
        bool m_hasFormatter = false;
        std::shared_ptr<LogFormatter> m_formatter;

    public:
        virtual ~LogAppender() {}
        std::shared_ptr<LogFormatter> getFormatter() const { return m_formatter; }
        void setFormatter(std::shared_ptr<LogFormatter> val);
        void setLevel(LogLevel::Level val) { m_level = val; }
        // pure virtual function
        // for StdoutLogAppender and FileLogAppender to realize
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) = 0;

        virtual std::string toYamlString() = 0;
    };

    // std out
    class StdoutLogAppender : public LogAppender
    {
    public:
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event);
        std::string toYamlString() override;
    };

    // file out
    class FileLogAppender : public LogAppender
    {
    private:
        std::string m_filename;
        std::ofstream m_filestream;

    public:
        FileLogAppender(const std::string& filename);
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event);
        bool reopen();
        std::string toYamlString() override;
    };

    /*
        Logger: log output assisstant
            A logger has its own LogFormatter. Use logger to output the log in the appointed LogAppenders. LogAppenders are in a list.
    */
    class Logger : public std::enable_shared_from_this<Logger>
    {
    friend class LoggerManager;
    private:
        // log name
        std::string m_name;
        // log level, Logger will output those log whose level is higher or equal than the m_level
        LogLevel::Level m_level;
        std::list<std::shared_ptr<LogAppender>> m_appenders;
        std::shared_ptr<LogFormatter> m_formatter;
        std::shared_ptr<Logger> m_root;

    public:
        Logger(const std::string &name = "root");

        // writing log and assigning the level
        void log(LogLevel::Level level, std::shared_ptr<LogEvent> event);
        // writing debug log
        void debug(std::shared_ptr<LogEvent> event);
        // writing info log
        void info(std::shared_ptr<LogEvent> event);
        // writing warn log
        void warn(std::shared_ptr<LogEvent> event);
        // writing error log
        void error(std::shared_ptr<LogEvent> event);
        // writing fatal log
        void fatal(std::shared_ptr<LogEvent> event);
        // add appender
        void addAppender(std::shared_ptr<LogAppender> appender);
        // delete appender
        void delAppender(std::shared_ptr<LogAppender> appender);
        // clear all appenders
        void clearAppenders();

        // get logger level
        LogLevel::Level getLevel() const { return m_level; }
        const std::string getName() const { return m_name; }

        void setLevel(LogLevel::Level val) { m_level = val; }
        void setFormatter(std::shared_ptr<LogFormatter> val);
        void setFormatter(const std::string &val);

        std::shared_ptr<LogFormatter> getFormatter();

        // print the log information
        std::string toYamlString();
    };

    // A wrapper for an event.
    // When calling the destructor, the log information will be outputed.
    class LogEventWrap
    {
    private:
        std::shared_ptr<LogEvent> m_event;

    public:
        LogEventWrap(std::shared_ptr<LogEvent> event);
        ~LogEventWrap();

        std::shared_ptr<LogEvent> getEvent() const { return m_event; }
        std::stringstream &getSS();
    };

    // public succeeded class from LogFormatter::FormatItem
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << LogLevel::ToString(level);
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            // we are probably using root as the logger, in this case we do not output the root but the event logger
            os << event->getLogger()->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->getFiberId();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    private:
        std::string m_format;

    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S") : m_format(format)
        {
            if (m_format.empty())
            {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            struct tm tm;
            time_t time = event->getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
        }
    };

    class FilenameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FilenameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    private:
        std::string m_string;

    public:
        StringFormatItem(const std::string &str) : m_string(str) {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << m_string;
        }
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    private:
        std::string m_string;

    public:
        TabFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, std::shared_ptr<Logger> logger, LogLevel::Level level, std::shared_ptr<LogEvent> event) override
        {
            os << "\t";
        }
    };

    // manager for all loggers
    // setting the level, format, appender
    class LoggerManager
    {
    private:
        std::map<std::string, std::shared_ptr<Logger>> m_loggers;
        std::shared_ptr<Logger> m_root;

    public:
        LoggerManager();

        std::shared_ptr<Logger> getLogger(const std::string &name);
        std::shared_ptr<Logger> getRoot() const { return m_root; }
        void init();
        std::string toYamlString();
    };

    typedef zcserver::Singleton<LoggerManager> LoggerMgr;
}

#endif