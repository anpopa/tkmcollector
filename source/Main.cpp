/*-
 * SPDX-License-Identifier: MIT
 *-
 * @date      2021-2022
 * @author    Alin Popa <alin.popa@fxdata.ro>
 * @copyright MIT
 * @brief     Main function
 * @details   Application main function
 *-
 */

#include "Application.h"

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <map>
#include <sys/stat.h>
#include <taskmonitor/taskmonitor.h>

static void terminate(int signum)
{
  static_cast<void>(signum); // UNUSED
  exit(EXIT_SUCCESS);
}

static void daemonize()
{
  pid_t pid, sid;
  int fd;

  if (getppid() == 1) {
    return;
  }

  pid = fork();
  if (pid < 0) {
    std::cout << "ERROR: Cannot fork on daemonize: " << strerror(errno) << std::endl;
    ::exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    ::exit(EXIT_SUCCESS);
  }

  sid = setsid();
  if (sid < 0) {
    std::cout << "ERROR: Cannot setsid on daemonize: " << strerror(errno) << std::endl;
    ::exit(EXIT_FAILURE);
  }

  if ((chdir("/")) < 0) {
    std::cout << "ERROR: Cannot chdir on daemonize: " << strerror(errno) << std::endl;
    ::exit(EXIT_FAILURE);
  }

  fd = open("/dev/null", O_RDWR, 0);
  if (fd != -1) {
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    if (fd > 2) {
      close(fd);
    }
  }

  umask(027);
}

static int create_pidfile(const char *path)
{
  FILE *file = NULL;
  int status = 0;

  file = fopen(path, "w");
  if (file == NULL) {
    status = -1;
  }

  if (status >= 0) {
    int pid = getpid();
    if (!fprintf(file, "%d\n", pid)) {
      status = -1;
    }

    fclose(file);
  }

  return status;
}

auto main(int argc, char **argv) -> int
{
  const char *config_path = nullptr;
  int long_index = 0;
  bool daemon = false;
  bool help = false;
  bool eraseDatabase = false;
  int c;

  struct option longopts[] = {{"config", required_argument, nullptr, 'c'},
                              {"eraseDatabase", no_argument, nullptr, 'e'},
                              {"daemon", no_argument, nullptr, 'd'},
                              {"help", no_argument, nullptr, 'h'},
                              {nullptr, 0, nullptr, 0}};

  while ((c = getopt_long(argc, argv, "c:ehd", longopts, &long_index)) != -1) {
    switch (c) {
    case 'c':
      config_path = optarg;
      break;
    case 'e': {
      std::string yes = "no";
      std::cout << "Are you sure you want to erase the current database? "
                   "(cannot be undone): ";
      std::cin >> yes;
      if ((yes == "yes") || (yes == "Yes") || (yes == "YES")) {
        eraseDatabase = true;
      } else {
        std::cout << "Ignoring erase database request" << std::endl;
      }
      break;
    }
    case 'd':
      daemon = true;
      break;
    case 'h':
      help = true;
      break;
    default:
      break;
    }
  }

  if (help) {
    std::cout << "TaskMonitorCollector: TaskMonior collector\n"
              << "Version: " << tkm::tkmDefaults.getFor(tkm::Defaults::Default::Version)
              << " libtkm: " << TKMLIB_VERSION << "\n\n";
    std::cout << "Usage: tkmcollector [OPTIONS] \n\n";
    std::cout << "  General:\n";
    std::cout << "     --config, -c             <string> Configuration file path\n";
    std::cout << "     --daemon, -d             <noarg>  Daemonize\n";
    std::cout << "     --eraseDatabase, -e      <noarg>  Reinitialize database\n";
    std::cout << "  Help:\n";
    std::cout << "     --help, -h                 Print this help\n\n";

    ::exit(EXIT_SUCCESS);
  }

  std::filesystem::path configPath(tkm::tkmDefaults.getFor(tkm::Defaults::Default::ConfPath));
  if (config_path != nullptr) {
    if (!std::filesystem::exists(config_path)) {
      std::cout << "Provided configuration file cannot be accessed: " << config_path << std::endl;
      return EXIT_FAILURE;
    }
    configPath = std::string(config_path);
  }

  if (daemon) {
    auto options = tkm::Options{config_path};

    if (!std::filesystem::exists(options.getFor(tkm::Options::Key::RuntimeDirectory))) {
      if (!std::filesystem::create_directories(
              options.getFor(tkm::Options::Key::RuntimeDirectory))) {
        std::cout << "ERROR: Cannot create runtime directory" << std::endl;
        return EXIT_FAILURE;
      }
    }

    // daemonize
    daemonize();

    // create pid file
    std::string pidFile = options.getFor(tkm::Options::Key::RuntimeDirectory);
    pidFile += "/tkmcollector.pid";
    std::cout << "PID file: " << pidFile << std::endl;
    create_pidfile(pidFile.c_str());
  }

  signal(SIGINT, terminate);
  signal(SIGTERM, terminate);
  signal(SIGPIPE, SIG_IGN);

  tkm::collector::Application app{"TKMCollector", "TaskMonitor Collector", configPath};

  if (eraseDatabase) {
    const std::map<tkm::Defaults::Arg, std::string> forcedArgument{
        std::make_pair<tkm::Defaults::Arg, std::string>(
            tkm::Defaults::Arg::Forced,
            std::string(tkm::tkmDefaults.valFor(tkm::Defaults::Val::True)))};

    tkm::collector::IDatabase::Request dbrq{.client = nullptr,
                                            .action =
                                                tkm::collector::IDatabase::Action::InitDatabase,
                                            .args = forcedArgument,
                                            .bulkData = std::make_any<int>(0)};
    app.getDatabase()->pushRequest(dbrq);
  }

  app.run();

  return EXIT_SUCCESS;
}
