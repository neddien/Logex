#pragma once

#include "Common.h"

namespace lgx {
    class Logger
    {
    public:
        struct DefaultStyle
        {
            // Format string used for non-terminal streams (files, stringstreams…).
            // Also passed as StyleArgs::format so FormatFn callbacks can reuse it.
            std::string format = "[{datetime}] [{level}] ({prefix}): {msg}";

            // Strings printed for each log level in the {level} placeholder.
            std::string infoLabel    = "Info";
            std::string warnLabel    = "Warn";
            std::string errorLabel   = "Error";
            std::string fatalLabel   = "Fatal";
            std::string debugLabel   = "Debug";
            std::string verboseLabel = "Verbose";

            // Per-level terminal format functions.
            // Setting a field to nullptr falls back to log.style + format string.
            FormatFn defaultInfoStyle = [](const StyleArgs& a) -> std::string {
                return fmt::format(fmt::bg(fmt::color::dark_green) | fmt::fg(fmt::color::white),
                                   fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            };
            FormatFn defaultWarnStyle = [](const StyleArgs& a) -> std::string {
                return fmt::format(fmt::bg(fmt::color::orange) | fmt::fg(fmt::color::black),
                                   fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            };
            FormatFn defaultErrorStyle = [](const StyleArgs& a) -> std::string {
                return fmt::format(fmt::emphasis::italic | fmt::bg(fmt::color::red) | fmt::fg(fmt::color::white),
                                   fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            };
            FormatFn defaultFatalStyle = [](const StyleArgs& a) -> std::string {
                return fmt::format(fmt::emphasis::italic | fmt::bg(fmt::color::dark_red) | fmt::fg(fmt::color::white),
                                   fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            };
            FormatFn defaultDebugStyle = [](const StyleArgs& a) -> std::string {
                return fmt::format(fmt::emphasis::italic | fmt::bg(fmt::color::green) | fmt::fg(fmt::color::white),
                                   fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            };
            FormatFn defaultVerboseStyle = [](const StyleArgs& a) -> std::string {
                return fmt::format(fmt::emphasis::italic | fmt::bg(fmt::color::gray) | fmt::fg(fmt::color::white),
                                   fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            };
        };

        struct Properties
        {
            std::string                loggerName                   = "Logex";
            Type                       appType                      = Type::User;
            std::vector<std::ostream*> outputStreams                = { &std::cout };
            bool                       serializeToNonStdoutStreams  = false;
            bool                       writeStyleToNonStdoutStreams = false;
            bool                       verbose                      = false;
            bool                       syslog                       = false;
            std::string                defaultPrefix                = "App";
            std::string                dateTimeFormat               = "%Y-%m-%d %H:%M:%S";
            std::string                dateFormat                   = "%Y-%m-%d";
            std::string                textDateFormat               = "%b %d";
            std::string                timeFormat                   = "%H:%M:%S";
            DefaultStyle               defaultStyle                 = DefaultStyle{};
        };

    private:
        struct State
        {
            Properties              properties;
            std::thread             pollThread;
            std::condition_variable pollCV;
            bool                    run = false;
            std::deque<LogMsg>      logQueue;
            std::mutex              guard;

            State(Properties props) : properties(std::move(props)) {}
            ~State()
            {
                {
                    const std::lock_guard<std::mutex> lock{ guard };
                    run = false;
                }
                pollCV.notify_all();
                if (pollThread.joinable())
                    pollThread.join();
            }
        };
        std::shared_ptr<State> m_State;

    private:
        // -----------------------------------------------------------------------
        // Getters
        // -----------------------------------------------------------------------
    public:
        [[nodiscard]] inline auto GetOutputStreams() const noexcept -> const std::vector<std::ostream*>
        {
            if (!m_State) return {};
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.outputStreams;
        }
        [[nodiscard]] inline auto GetDefaultPrefix() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultPrefix;
        }
        [[nodiscard]] inline auto GetDateTimeFormat() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.dateTimeFormat;
        }
        [[nodiscard]] inline auto GetDateFormat() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.dateFormat;
        }
        [[nodiscard]] inline auto GetTextDateFormat() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.textDateFormat;
        }
        [[nodiscard]] inline auto GetTimeFormat() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.timeFormat;
        }
        [[nodiscard]] inline auto GetFormat() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.format;
        }
        [[nodiscard]] inline auto GetSyslog() const noexcept -> bool
        {
            if (!m_State) return false;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.syslog;
        }

        // Level label getters
        [[nodiscard]] inline auto GetInfoLabel() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.infoLabel;
        }
        [[nodiscard]] inline auto GetWarnLabel() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.warnLabel;
        }
        [[nodiscard]] inline auto GetErrorLabel() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.errorLabel;
        }
        [[nodiscard]] inline auto GetFatalLabel() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.fatalLabel;
        }
        [[nodiscard]] inline auto GetDebugLabel() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.debugLabel;
        }
        [[nodiscard]] inline auto GetVerboseLabel() const noexcept -> std::string
        {
            if (!m_State) return "";
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.verboseLabel;
        }

        // FormatFn getters
        [[nodiscard]] inline auto GetDefaultInfoStyle() const noexcept -> FormatFn
        {
            if (!m_State) return nullptr;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.defaultInfoStyle;
        }
        [[nodiscard]] inline auto GetDefaultWarnStyle() const noexcept -> FormatFn
        {
            if (!m_State) return nullptr;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.defaultWarnStyle;
        }
        [[nodiscard]] inline auto GetDefaultErrorStyle() const noexcept -> FormatFn
        {
            if (!m_State) return nullptr;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.defaultErrorStyle;
        }
        [[nodiscard]] inline auto GetDefaultFatalStyle() const noexcept -> FormatFn
        {
            if (!m_State) return nullptr;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.defaultFatalStyle;
        }
        [[nodiscard]] inline auto GetDefaultDebugStyle() const noexcept -> FormatFn
        {
            if (!m_State) return nullptr;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.defaultDebugStyle;
        }
        [[nodiscard]] inline auto GetDefaultVerboseStyle() const noexcept -> FormatFn
        {
            if (!m_State) return nullptr;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            return m_State->properties.defaultStyle.defaultVerboseStyle;
        }

        // -----------------------------------------------------------------------
        // Setters
        // -----------------------------------------------------------------------
        inline auto SetOutputStreams(std::vector<std::ostream*> oss) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.outputStreams = std::move(oss);
        }
        inline auto SetDefaultPrefix(const std::string_view newDefaultPrefix) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultPrefix = newDefaultPrefix;
        }
        inline auto SetDateTimeFormat(const std::string_view fmt) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.dateTimeFormat = fmt;
        }
        inline auto SetDateFormat(const std::string_view fmt) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.dateFormat = fmt;
        }
        inline auto SetTextDateFormat(const std::string_view fmt) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.textDateFormat = fmt;
        }
        inline auto SetTimeFormat(const std::string_view fmt) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.timeFormat = fmt;
        }
        inline auto SetFormat(const std::string_view newFormat) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.format = newFormat;
        }
        inline auto SetVerbose(const bool enable) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.verbose = enable;
        }
        inline auto SetSyslog(const bool enable) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.syslog = enable;
        }

        // Level label setters
        inline auto SetInfoLabel(const std::string_view label) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.infoLabel = label;
        }
        inline auto SetWarnLabel(const std::string_view label) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.warnLabel = label;
        }
        inline auto SetErrorLabel(const std::string_view label) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.errorLabel = label;
        }
        inline auto SetFatalLabel(const std::string_view label) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.fatalLabel = label;
        }
        inline auto SetDebugLabel(const std::string_view label) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.debugLabel = label;
        }
        inline auto SetVerboseLabel(const std::string_view label) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.verboseLabel = label;
        }

        // FormatFn setters — primary API
        inline auto SetDefaultInfoStyle(FormatFn fn) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.defaultInfoStyle = std::move(fn);
        }
        inline auto SetDefaultWarnStyle(FormatFn fn) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.defaultWarnStyle = std::move(fn);
        }
        inline auto SetDefaultErrorStyle(FormatFn fn) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.defaultErrorStyle = std::move(fn);
        }
        inline auto SetDefaultFatalStyle(FormatFn fn) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.defaultFatalStyle = std::move(fn);
        }
        inline auto SetDefaultDebugStyle(FormatFn fn) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.defaultDebugStyle = std::move(fn);
        }
        inline auto SetDefaultVerboseStyle(FormatFn fn) noexcept -> void
        {
            if (!m_State) return;
            const std::lock_guard<std::mutex> lock{ m_State->guard };
            m_State->properties.defaultStyle.defaultVerboseStyle = std::move(fn);
        }

        // Convenience overloads
        inline auto SetDefaultInfoStyle(const fmt::text_style& style) noexcept -> void
        {
            SetDefaultInfoStyle([style](const StyleArgs& a) -> std::string {
                return fmt::format(style, fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            });
        }
        inline auto SetDefaultWarnStyle(const fmt::text_style& style) noexcept -> void
        {
            SetDefaultWarnStyle([style](const StyleArgs& a) -> std::string {
                return fmt::format(style, fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            });
        }
        inline auto SetDefaultErrorStyle(const fmt::text_style& style) noexcept -> void
        {
            SetDefaultErrorStyle([style](const StyleArgs& a) -> std::string {
                return fmt::format(style, fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            });
        }
        inline auto SetDefaultFatalStyle(const fmt::text_style& style) noexcept -> void
        {
            SetDefaultFatalStyle([style](const StyleArgs& a) -> std::string {
                return fmt::format(style, fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            });
        }
        inline auto SetDefaultDebugStyle(const fmt::text_style& style) noexcept -> void
        {
            SetDefaultDebugStyle([style](const StyleArgs& a) -> std::string {
                return fmt::format(style, fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            });
        }
        inline auto SetDefaultVerboseStyle(const fmt::text_style& style) noexcept -> void
        {
            SetDefaultVerboseStyle([style](const StyleArgs& a) -> std::string {
                return fmt::format(style, fmt::runtime(a.format),
                                   fmt::arg("date", a.date), fmt::arg("time", a.time),
                                   fmt::arg("textdate", a.textdate),
                                   fmt::arg("datetime", a.datetime), fmt::arg("level", a.level),
                                   fmt::arg("prefix", a.prefix), fmt::arg("msg", a.msg));
            });
        }

        // -----------------------------------------------------------------------
        // Constructors / lifecycle
        // -----------------------------------------------------------------------
    public:
        Logger() noexcept = default;
        Logger(Properties properties)
            : m_State(std::make_shared<State>(std::move(properties)))
        {
            m_State->run = true;
            m_State->pollThread = std::thread([state = m_State.get()]() {
                PollLogs(state);
            });
        }
        ~Logger() noexcept = default;
        Logger(const Logger& other) noexcept = default;
        Logger(Logger&& other) noexcept = default;
        Logger& operator=(const Logger& other) noexcept = default;
        Logger& operator=(Logger&& other) noexcept = default;

        // -----------------------------------------------------------------------
        // Private helpers
        // -----------------------------------------------------------------------
    private:
        [[nodiscard]] static auto FormatFnFromLevel(const Properties& props, const Level level) noexcept -> const FormatFn&
        {
            switch (level)
            {
                using enum Level;

                case Warn:    return props.defaultStyle.defaultWarnStyle;
                case Error:   return props.defaultStyle.defaultErrorStyle;
                case Fatal:   return props.defaultStyle.defaultFatalStyle;
                case Debug:   return props.defaultStyle.defaultDebugStyle;
                case Verbose: return props.defaultStyle.defaultVerboseStyle;
                default:
                case Info:    return props.defaultStyle.defaultInfoStyle;
            }
        }

        [[nodiscard]] static auto LevelLabelFromLevel(const Properties& props, const Level level) noexcept -> std::string_view
        {
            switch (level)
            {
                using enum Level;

                case Warn:    return props.defaultStyle.warnLabel;
                case Error:   return props.defaultStyle.errorLabel;
                case Fatal:   return props.defaultStyle.fatalLabel;
                case Debug:   return props.defaultStyle.debugLabel;
                case Verbose: return props.defaultStyle.verboseLabel;
                default:
                case Info:    return props.defaultStyle.infoLabel;
            }
        }

        [[nodiscard]] static constexpr auto ContainsPlaceholder(const std::string_view format,
                                                                const std::string_view placeholder) noexcept -> bool
        {
            return format.find(placeholder) != std::string_view::npos;
        }

        // -----------------------------------------------------------------------
        // Poll thread
        // -----------------------------------------------------------------------
    private:
        static void PollLogs(State* state)
        {
        #ifdef __unix__
            {
                const std::lock_guard<std::mutex> lock{ state->guard };
                if (state->properties.syslog)
                {
                    int facility = LOG_USER;
                    switch (state->properties.appType)
                    {
                        case Type::User:   facility = LOG_USER;   break;
                        case Type::Daemon: facility = LOG_DAEMON; break;
                        default: break;
                    }
                    openlog(state->properties.loggerName.c_str(), LOG_PID | LOG_CONS, facility);
                }
            }
        #endif

            while (true)
            {
                std::unique_lock<std::mutex> guard{ state->guard };
                state->pollCV.wait(guard, [state]() { return !state->logQueue.empty() || !state->run; });

                if (state->logQueue.empty() && !state->run)
                    break;

                if (!state->logQueue.empty())
                {
                    auto log = std::move(state->logQueue.front());
                    state->logQueue.pop_front();
                    guard.unlock();
                    InternalLog(state, log);
                }
            }

        #ifdef __unix__
            if (state->properties.syslog)
                closelog();
        #endif
        }

        // -----------------------------------------------------------------------
        // InternalLog — formats and emits one message
        // -----------------------------------------------------------------------
        private:
        static auto InternalLog(State* state, const LogMsg& log) -> void
        {
        #ifndef LGX_DEBUG
            if (log.level == Level::Debug)
                return;
        #endif
            Properties props;
            {
                const std::lock_guard<std::mutex> lock{ state->guard };
                props = state->properties;
            }
            if (log.level == Level::Verbose && !props.verbose)
                return;

            const auto& format = props.defaultStyle.format;
            const auto& fn     = FormatFnFromLevel(props, log.level);

            // Compute time parts lazily.
            std::string datetime_str, date_str, textdate_str, time_str;
            const bool  needs_datetime = ContainsPlaceholder(format, "{datetime}");
            const bool  needs_date     = ContainsPlaceholder(format, "{date}");
            const bool  needs_textdate = ContainsPlaceholder(format, "{textdate}");
            const bool  needs_time     = ContainsPlaceholder(format, "{time}");

            if (needs_datetime || needs_date || needs_textdate || needs_time || fn)
            {
                const auto  time_now = std::chrono::system_clock::now();
                const auto  time_obj = std::chrono::system_clock::to_time_t(time_now);
                const auto* tm_ptr   = std::localtime(&time_obj);

                if (needs_datetime || fn)
                    datetime_str = fmt::format(fmt::runtime("{:" + props.dateTimeFormat + '}'), *tm_ptr);
                if (needs_date || fn)
                    date_str = fmt::format(fmt::runtime("{:" + props.dateFormat + '}'), *tm_ptr);
                if (needs_textdate || fn)
                    textdate_str = fmt::format(fmt::runtime("{:" + props.textDateFormat + '}'), *tm_ptr);
                if (needs_time || fn)
                    time_str = fmt::format(fmt::runtime("{:" + props.timeFormat + '}'), *tm_ptr);
            }

            const auto prefix_str = log.prefix.value_or(props.defaultPrefix);
            const auto level_str  = std::string{ LevelLabelFromLevel(props, log.level) };

            // Build arg_store for format-string–based output (non-terminal / fallback).
            auto arg_store = fmt::dynamic_format_arg_store<fmt::format_context>{};
            if (needs_datetime)
                arg_store.push_back(fmt::arg("datetime", datetime_str));
            if (needs_date)
                arg_store.push_back(fmt::arg("date", date_str));
            if (needs_textdate)
                arg_store.push_back(fmt::arg("textdate", textdate_str));
            if (needs_time)
                arg_store.push_back(fmt::arg("time", time_str));
            if (ContainsPlaceholder(format, "{level}"))
                arg_store.push_back(fmt::arg("level", level_str));
            if (ContainsPlaceholder(format, "{prefix}"))
                arg_store.push_back(fmt::arg("prefix", prefix_str));
            if (!ContainsPlaceholder(format, "{msg}"))
                throw std::invalid_argument("A message is always required.");
            arg_store.push_back(fmt::arg("msg", log.message));

            // An explicit per-message fmt::text_style (set via Log(prefix, level, style, …))
            // takes precedence over the per-level FormatFn.
            const bool has_explicit_style =
                log.style.has_foreground() || log.style.has_background() || log.style.has_emphasis();

            const StyleArgs style_args{
                .date     = date_str,
                .time     = time_str,
                .datetime = datetime_str,
                .textdate = textdate_str,
                .prefix   = prefix_str,
                .level    = level_str,
                .msg      = log.message,
                .format   = format,
            };

            for (const auto& stream : props.outputStreams)
            {
                if (stream == &std::cout)
                {
                    if (!has_explicit_style && fn)
                        std::cout << fn(style_args) << std::endl;
                    else
                        std::cout << fmt::vformat(log.style, format, arg_store) << std::endl;
                }
                else
                {
                    if (props.serializeToNonStdoutStreams)
                        *stream << LogMsg::ToString(log) << std::endl;
                    else if (props.writeStyleToNonStdoutStreams)
                    {
                        if (!has_explicit_style && fn)
                            *stream << fn(style_args) << std::endl;
                        else
                            *stream << fmt::vformat(log.style, format, arg_store) << std::endl;
                    }
                    else
                        *stream << fmt::vformat(format, arg_store) << std::endl;
                }
            }

    #ifdef __unix__
            if (props.syslog)
            {
                std::string syslog_fmt = "{msg}";
                auto        sl_args    = fmt::dynamic_format_arg_store<fmt::format_context>{};

                if (ContainsPlaceholder(format, "{prefix}"))
                {
                    syslog_fmt = "[{prefix}] " + syslog_fmt;
                    sl_args.push_back(fmt::arg("prefix", prefix_str));
                }
                if (!ContainsPlaceholder(format, "{msg}"))
                    throw std::invalid_argument("A message is always required.");
                sl_args.push_back(fmt::arg("msg", log.message));

                int syslog_level = LOG_INFO;
                switch (log.level)
                {
                    using enum Level;

                    case Info:    syslog_level = LOG_INFO;    break;
                    case Warn:    syslog_level = LOG_WARNING; break;
                    case Error:   syslog_level = LOG_ERR;     break;
                    case Fatal:   syslog_level = LOG_ALERT;   break;
                    case Debug:
                    case Verbose: syslog_level = LOG_DEBUG;   break;
                }

                const std::string strlogmsg = props.writeStyleToNonStdoutStreams
                    ? fmt::vformat(log.style, syslog_fmt, sl_args)
                    : fmt::vformat(syslog_fmt, sl_args);

                syslog(syslog_level, "%s", strlogmsg.c_str());
            }
    #endif
        }

        // -----------------------------------------------------------------------
        // Public Log / Swap
        // -----------------------------------------------------------------------
    public:
        Logger& Swap(Logger& other) noexcept
        {
            std::swap(m_State, other.m_State);
            return *this;
        }

        // Log a pre-built LogMsg directly.
        inline auto Log(LogMsg log) const -> void
        {
            if (!m_State) return;
            const std::scoped_lock guard{ m_State->guard };
            m_State->logQueue.push_back(std::move(log));
            m_State->pollCV.notify_one();
        }


        // Full control: explicit prefix, level, per-message style, format string + args.
        // When the per-level FormatFn is set this style is ignored for terminal output.
        template <typename... TArgs>
        auto Log(std::string prefix, const Level level, const fmt::text_style& style,
                 const std::string_view fmt, TArgs&&... args) const -> void
        {
            Log(LogMsg{ .level   = level,
                        .message = fmt::format(fmt::runtime(fmt), std::forward<TArgs>(args)...),
                        .prefix  = std::move(prefix),
                        .style   = style });
        }
        template <typename... TArgs>
        auto Log(LogMsg log, const std::string_view fmt, TArgs&&... args) const -> void
        {
            std::string prefix;
            {
                const std::lock_guard<std::mutex> lock{ m_State->guard };
                prefix = log.prefix.value_or(m_State->properties.defaultPrefix);
            }
            Log(std::move(prefix), log.level, log.style, fmt, std::forward<TArgs>(args)...);
        }
        // Uses the per-level FormatFn (no explicit style).
        template <typename... TArgs>
        auto Log(const Level level, const std::string_view fmt, TArgs&&... args) const -> void
        {
            std::string prefix;
            {
                const std::lock_guard<std::mutex> lock{ m_State->guard };
                prefix = m_State->properties.defaultPrefix;
            }
            Log(std::move(prefix), level, fmt::text_style{}, fmt, std::forward<TArgs>(args)...);
        }
        template <typename... TArgs>
        auto Log(const std::string prefix, const Level level, const std::string_view fmt,
                 TArgs&&... args) const -> void
        {
            Log(std::move(prefix), level, fmt::text_style{}, fmt, std::forward<TArgs>(args)...);
        }
        // Explicit per-message style with default prefix.
        template <typename... TArgs>
        auto Log(const Level level, const fmt::text_style& style, const std::string_view fmt,
                 TArgs&&... args) const -> void
        {
            std::string prefix;
            {
                const std::lock_guard<std::mutex> lock{ m_State->guard };
                prefix = m_State->properties.defaultPrefix;
            }
            Log(std::move(prefix), level, style, fmt, std::forward<TArgs>(args)...);
        }

        // -----------------------------------------------------------------------
        // Convenience methods
        // -----------------------------------------------------------------------
    public:
        template <typename... TArgs>
        constexpr auto Info(const std::string_view fmt, TArgs&&... args) const -> void
        {
            Log(Level::Info, fmt, std::forward<TArgs>(args)...);
        }
        template <typename... TArgs>
        constexpr auto Warn(const std::string_view fmt, TArgs&&... args) const -> void
        {
            Log(Level::Warn, fmt, std::forward<TArgs>(args)...);
        }
        template <typename... TArgs>
        constexpr auto Error(const std::string_view fmt, TArgs&&... args) const -> void
        {
            Log(Level::Error, fmt, std::forward<TArgs>(args)...);
        }
        template <typename... TArgs>
        constexpr auto Fatal(const std::string_view fmt, TArgs&&... args) const -> void
        {
            Log(Level::Fatal, fmt, std::forward<TArgs>(args)...);
        }
        template <typename... TArgs>
        constexpr auto Debug(const std::string_view fmt, TArgs&&... args) const -> void
        {
            Log(Level::Debug, fmt, std::forward<TArgs>(args)...);
        }
        template <typename... TArgs>
        constexpr auto Verbose(const std::string_view fmt, TArgs&&... args) const -> void
        {
            Log(Level::Verbose, fmt, std::forward<TArgs>(args)...);
        }
    };

    // -------------------------------------------------------------------------
    // Global logger registry
    // -------------------------------------------------------------------------
    [[nodiscard]] auto Get(const std::string& loggerName) -> Logger&;

    // -------------------------------------------------------------------------
    // Global free functions — delegate to the "global" logger
    // -------------------------------------------------------------------------
    [[nodiscard]] inline auto GetDefaultPrefix() noexcept  { return Get("global").GetDefaultPrefix(); }
    [[nodiscard]] inline auto GetDateTimeFormat() noexcept { return Get("global").GetDateTimeFormat(); }
    [[nodiscard]] inline auto GetDateFormat() noexcept     { return Get("global").GetDateFormat(); }
    [[nodiscard]] inline auto GetTextDateFormat() noexcept { return Get("global").GetTextDateFormat(); }
    [[nodiscard]] inline auto GetTimeFormat() noexcept     { return Get("global").GetTimeFormat(); }
    [[nodiscard]] inline auto GetFormat() noexcept -> std::string { return Get("global").GetFormat(); }

    [[nodiscard]] inline auto GetInfoLabel() noexcept    { return Get("global").GetInfoLabel(); }
    [[nodiscard]] inline auto GetWarnLabel() noexcept    { return Get("global").GetWarnLabel(); }
    [[nodiscard]] inline auto GetErrorLabel() noexcept   { return Get("global").GetErrorLabel(); }
    [[nodiscard]] inline auto GetFatalLabel() noexcept   { return Get("global").GetFatalLabel(); }
    [[nodiscard]] inline auto GetDebugLabel() noexcept   { return Get("global").GetDebugLabel(); }
    [[nodiscard]] inline auto GetVerboseLabel() noexcept { return Get("global").GetVerboseLabel(); }

    [[nodiscard]] inline auto GetDefaultInfoStyle() noexcept    { return Get("global").GetDefaultInfoStyle(); }
    [[nodiscard]] inline auto GetDefaultWarnStyle() noexcept    { return Get("global").GetDefaultWarnStyle(); }
    [[nodiscard]] inline auto GetDefaultErrorStyle() noexcept   { return Get("global").GetDefaultErrorStyle(); }
    [[nodiscard]] inline auto GetDefaultFatalStyle() noexcept   { return Get("global").GetDefaultFatalStyle(); }
    [[nodiscard]] inline auto GetDefaultDebugStyle() noexcept   { return Get("global").GetDefaultDebugStyle(); }
    [[nodiscard]] inline auto GetDefaultVerboseStyle() noexcept { return Get("global").GetDefaultVerboseStyle(); }

    inline void SetDefaultPrefix(const std::string_view v) noexcept  { Get("global").SetDefaultPrefix(v); }
    inline void SetDateTimeFormat(const std::string_view v) noexcept { Get("global").SetDateTimeFormat(v); }
    inline void SetDateFormat(const std::string_view v) noexcept     { Get("global").SetDateFormat(v); }
    inline void SetTextDateFormat(const std::string_view v) noexcept { Get("global").SetTextDateFormat(v); }
    inline void SetTimeFormat(const std::string_view v) noexcept     { Get("global").SetTimeFormat(v); }
    inline void SetFormat(const std::string_view v) noexcept         { Get("global").SetFormat(v); }

    inline void SetInfoLabel(const std::string_view v) noexcept    { Get("global").SetInfoLabel(v); }
    inline void SetWarnLabel(const std::string_view v) noexcept    { Get("global").SetWarnLabel(v); }
    inline void SetErrorLabel(const std::string_view v) noexcept   { Get("global").SetErrorLabel(v); }
    inline void SetFatalLabel(const std::string_view v) noexcept   { Get("global").SetFatalLabel(v); }
    inline void SetDebugLabel(const std::string_view v) noexcept   { Get("global").SetDebugLabel(v); }
    inline void SetVerboseLabel(const std::string_view v) noexcept { Get("global").SetVerboseLabel(v); }

    // FormatFn setters
    inline void SetDefaultInfoStyle(FormatFn fn) noexcept    { Get("global").SetDefaultInfoStyle(std::move(fn)); }
    inline void SetDefaultWarnStyle(FormatFn fn) noexcept    { Get("global").SetDefaultWarnStyle(std::move(fn)); }
    inline void SetDefaultErrorStyle(FormatFn fn) noexcept   { Get("global").SetDefaultErrorStyle(std::move(fn)); }
    inline void SetDefaultFatalStyle(FormatFn fn) noexcept   { Get("global").SetDefaultFatalStyle(std::move(fn)); }
    inline void SetDefaultDebugStyle(FormatFn fn) noexcept   { Get("global").SetDefaultDebugStyle(std::move(fn)); }
    inline void SetDefaultVerboseStyle(FormatFn fn) noexcept { Get("global").SetDefaultVerboseStyle(std::move(fn)); }

    // Backward-compatible fmt::text_style overloads
    inline void SetDefaultInfoStyle(const fmt::text_style& s) noexcept    { Get("global").SetDefaultInfoStyle(s); }
    inline void SetDefaultWarnStyle(const fmt::text_style& s) noexcept    { Get("global").SetDefaultWarnStyle(s); }
    inline void SetDefaultErrorStyle(const fmt::text_style& s) noexcept   { Get("global").SetDefaultErrorStyle(s); }
    inline void SetDefaultFatalStyle(const fmt::text_style& s) noexcept   { Get("global").SetDefaultFatalStyle(s); }
    inline void SetDefaultDebugStyle(const fmt::text_style& s) noexcept   { Get("global").SetDefaultDebugStyle(s); }
    inline void SetDefaultVerboseStyle(const fmt::text_style& s) noexcept { Get("global").SetDefaultVerboseStyle(s); }

    // Log free functions
    inline auto Log(const LogMsg& log) -> void { Get("global").Log(log); }

    template <typename... TArgs>
    inline auto Log(const std::string_view prefix, const Level level, const fmt::text_style& style,
                    const std::string_view fmt, TArgs&&... args) -> void
    {
        Get("global").Log(std::string{ prefix }, level, style, fmt, std::forward<TArgs>(args)...);
    }
    template <typename... TArgs>
    inline auto Log(const LogMsg& log, const std::string_view fmt, TArgs&&... args) -> void
    {
        Get("global").Log(log, fmt, std::forward<TArgs>(args)...);
    }
    template <typename... TArgs>
    inline auto Log(const Level level, const std::string_view fmt, TArgs&&... args) -> void
    {
        Get("global").Log(level, fmt, std::forward<TArgs>(args)...);
    }
    template <typename... TArgs>
    inline auto Log(const std::string_view prefix, const Level level, const std::string_view fmt,
                    TArgs&&... args) -> void
    {
        Get("global").Log(std::string{ prefix }, level, fmt, std::forward<TArgs>(args)...);
    }
    template <typename... TArgs>
    inline auto Log(const Level level, const fmt::text_style& style, const std::string_view fmt,
                    TArgs&&... args) -> void
    {
        Get("global").Log(level, style, fmt, std::forward<TArgs>(args)...);
    }
} // namespace lgx
