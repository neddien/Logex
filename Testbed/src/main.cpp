#include <fstream>
#include <iostream>

#include <Logger.h>

auto main() -> int
{
    // Create logger instances.
    // const auto logger = lgx::Logger(lgx::Logger::Properties{ .defaultPrefix = "main.cpp" });

    // logger.Info("Current file: {}", __FILE__);
    // logger.Warn("A Warning.");
    // logger.Error("Error code: {}", std::rand() % 256);
    // logger.Fatal("A Fatal error has occured.");

    //// Log to different streams e.g., std::ofstream, std::fstream, std::stringstream, etc.
    //// Use {date} and {time} as separate placeholders, or keep {datetime} for the combined form.
    // auto       fs          = std::ofstream("./log.txt");
    // const auto file_logger = lgx::Logger{ lgx::Logger::Properties{
    //     .outputStreams               = { &fs },
    //     .serializeToNonStdoutStreams = true,
    //     .defaultPrefix               = "log.txt",
    //     .defaultStyle                = { .format = "[{date} {time}] [{level}] ({prefix}) >> {msg}\n" } } };

    // file_logger.Info("Current file: {}", __FILE__);
    // file_logger.Warn("A Warning.");
    // file_logger.Error("Error code: {}", std::rand() % 256);
    // file_logger.Fatal("A Fatal error has occured.");

    //// Log using the global logger.
    // lgx::Log(lgx::Info, "The global logger");

    //// Per-message explicit style (overrides the per-level FormatFn for terminal output).
    // lgx::Log("App", lgx::Warn, fmt::fg(fmt::color::orange) | fmt::bg(fmt::color::dark_blue),
    //          "Explicit per-message style.");

    //// Serializable log messages.
    // const auto serialized_log = lgx::LogMsg::ToString(
    //     lgx::LogMsg{ .level   = lgx::Error,
    //                  .message = fmt::format("An error occured in file: {}", __FILE__),
    //                  .style   = fmt::fg(fmt::color::black) | fmt::emphasis::bold | fmt::bg(fmt::color::aqua) });

    // Custom level labels.
    lgx::SetInfoLabel("info");
    lgx::SetWarnLabel("warn");
    lgx::SetErrorLabel("err");
    lgx::SetFatalLabel("crit");

    // Customize individual log parts with per-level FormatFn for the Global logger.
    lgx::SetDefaultInfoStyle(
        [](const lgx::StyleArgs& a) -> std::string
        {
            return fmt::format("{} {} {} {}: {}", fmt::styled(a.textdate, fmt::fg(fmt::color::green)),
                               fmt::styled(a.time, fmt::fg(fmt::color::green)),
                               fmt::styled(a.prefix, fmt::fg(fmt::color::cyan)),
                               fmt::styled(a.level, fmt::fg(fmt::color::white)), a.msg);
        });

    lgx::SetFormat("[{textdate} {time}] [{level}] ({prefix}): {msg}");

    // lgx::Log(lgx::Info, "Global logger with textdate: something somthing");

    // Local logger instance with custom styles.
    auto logger = lgx::Logger(lgx::Logger::Properties{ .defaultPrefix = "AssetRegistry", .dateFormat = "%m-%d" });

    logger.SetDefaultInfoStyle(
        [](const lgx::StyleArgs& a)
        {
            return fmt::format("{} {} {} {}: {}", fmt::styled(a.textdate, fmt::fg(fmt::color::dark_gray)),
                               fmt::styled(a.time, fmt::fg(fmt::color::dark_gray)),
                               fmt::styled(a.prefix, fmt::fg(fmt::color::deep_sky_blue)), a.level,
                               fmt::styled(a.msg, fmt::fg(fmt::color::dark_gray)));
        });

    logger.SetDefaultErrorStyle(
        [](const lgx::StyleArgs& a)
        {
            return fmt::format("{} {} {} {}: {}", fmt::styled(a.textdate, fmt::fg(fmt::color::dark_gray)),
                               fmt::styled(a.time, fmt::fg(fmt::color::dark_gray)),
                               fmt::styled(a.prefix, fmt::fg(fmt::color::deep_sky_blue)),
                               fmt::styled(a.level, fmt::fg(fmt::color::red)),
                               fmt::styled(a.msg, fmt::fg(fmt::color::red)));
        });

    logger.SetDefaultWarnStyle(
        [](const lgx::StyleArgs& a)
        {
            return fmt::format("{} {} {} {}: {}", fmt::styled(a.textdate, fmt::fg(fmt::color::dark_gray)),
                               fmt::styled(a.time, fmt::fg(fmt::color::dark_gray)),
                               fmt::styled(a.prefix, fmt::fg(fmt::color::deep_sky_blue)),
                               fmt::styled(a.level, fmt::fg(fmt::color::yellow)),
                               fmt::styled(a.msg, fmt::fg(fmt::color::yellow)));
        });

    logger.SetInfoLabel("info");
    logger.SetWarnLabel("warn");
    logger.SetErrorLabel("error");
    logger.SetFatalLabel("fatal");

    logger.Log(lgx::Info, "warden: Warden version v1.0 (build 1057 Apr 18 aarch64-gnueabi-gcc)");
    logger.Log(lgx::Info, "warden: Kernel initialized with RT clock HSI@480000000 with kernel argument(s): "
                          "init=/sbin/rcinit serial.enabled=1 serial.out=uart0");
    logger.Log(lgx::Info,
               "rcinit: RCInit version v1.0: Minimal RC for minimal systems (build 1002 Apr 18 aarch64-gnueabi-gcc)");
    logger.Log(lgx::Warn, "rcinit: mountfs[2]: Failed to mount /dev/sda1 @ /. Missing file permission: rwx");
    logger.Log(lgx::Error, "rcinit: ptty[3]: Failed to open PTTY at /dev/tty0: no such file or directory");
    logger.Log(lgx::Error, "warden: init exited with code=0xFF");
    logger.Log(lgx::Error, "warden: Kernel oops occured! Kernel has entered into a standby mode.");
    logger.Log(lgx::Info, "warden: Press ENTER for maintenance shell=/sbin/sh or ESC to reboot...");

    // Backward-compatible: pass a fmt::text_style and it wraps into a FormatFn automatically.
    // lgx::SetDefaultWarnStyle(fmt::fg(fmt::color::yellow));
    // lgx::SetDefaultErrorStyle(fmt::fg(fmt::color::red));
    // lgx::SetDefaultFatalStyle(fmt::fg(fmt::color::dark_red));
    // lgx::SetDefaultPrefix("App");

    // lgx::Log(lgx::Info, "Serialized (to string) log message: {}", serialized_log);
    // lgx::LogMsg deserialized_log = lgx::LogMsg::FromString(serialized_log);
    // lgx::Log(deserialized_log);
}
