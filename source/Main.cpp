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

using namespace std;
using namespace tkm::collector;
namespace fs = std::filesystem;

static void terminate(int signum)
{
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
    cout << "ERROR: Cannot fork on daemonize: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  sid = setsid();
  if (sid < 0) {
    cout << "ERROR: Cannot setsid on daemonize: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  if ((chdir("/")) < 0) {
    cout << "ERROR: Cannot chdir on daemonize: " << strerror(errno) << endl;
    exit(EXIT_FAILURE);
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
    cout << "TKM-Collector: TaskMonior collector version: "
         << tkm::tkmDefaults.getFor(tkm::Defaults::Default::Version) << "\n\n";
    cout << "Usage: tkm-collector [OPTIONS] \n\n";
    cout << "  General:\n";
    cout << "     --config, -c             <string> Configuration file path\n";
    cout << "     --daemon, -d             <noarg>  Daemonize\n";
    cout << "     --eraseDatabase, -e      <noarg>  Reinitialize database "
            "(admin login "
            "issues)\n";
    cout << "  Help:\n";
    cout << "     --help, -h                 Print this help\n\n";

    exit(EXIT_SUCCESS);
  }

  fs::path configPath(tkm::tkmDefaults.getFor(tkm::Defaults::Default::ConfPath));
  if (config_path != nullptr) {
    if (!fs::exists(config_path)) {
      cout << "Provided configuration file cannot be accessed: " << config_path << endl;
      return EXIT_FAILURE;
    }
    configPath = string(config_path);
  }

  if (daemon) {
    auto options = tkm::Options{config_path};

    if (!fs::exists(options.getFor(tkm::Options::Key::RuntimeDirectory))) {
      if (!fs::create_directories(options.getFor(tkm::Options::Key::RuntimeDirectory))) {
        cout << "ERROR: Cannot create runtime directory" << endl;
        return EXIT_FAILURE;
      }
    }

    // daemonize
    daemonize();

    // create pid file
    std::string pidFile = options.getFor(tkm::Options::Key::RuntimeDirectory);
    pidFile += "/tkm-collector.pid";
    cout << "PID file: " << pidFile << endl;
    create_pidfile(pidFile.c_str());
  }

  signal(SIGINT, terminate);
  signal(SIGTERM, terminate);
  signal(SIGPIPE, SIG_IGN);

  tkm::collector::Application app{"TKM-Collector", "TaskMonitor Collector", configPath};

  if (eraseDatabase) {
    const std::map<tkm::Defaults::Arg, std::string> forcedArgument{
        std::make_pair<tkm::Defaults::Arg, std::string>(
            tkm::Defaults::Arg::Forced,
            std::string(tkm::tkmDefaults.valFor(tkm::Defaults::Val::True)))};

    IDatabase::Request dbrq{.action = IDatabase::Action::InitDatabase, .args = forcedArgument};
    app.getDatabase()->pushRequest(dbrq);
  }

  app.run();

  return EXIT_SUCCESS;
}
