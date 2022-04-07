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

using namespace std;
using namespace tkm::collector;
namespace fs = std::filesystem;

static void terminate(int signum)
{
  exit(EXIT_SUCCESS);
}

auto main(int argc, char **argv) -> int
{
  const char *config_path = nullptr;
  int long_index = 0;
  bool help = false;
  bool eraseDatabase = false;
  int c;

  struct option longopts[] = {{"config", required_argument, nullptr, 'c'},
                              {"eraseDatabase", no_argument, nullptr, 'e'},
                              {"help", no_argument, nullptr, 'h'},
                              {nullptr, 0, nullptr, 0}};

  while ((c = getopt_long(argc, argv, "c:eh", longopts, &long_index)) != -1) {
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
    cout << "     --eraseDatabase, -e      <noarg>  Reinitialize database "
            "(admin login "
            "issues)\n";
    cout << "  Help:\n";
    cout << "     --help, -h                 Print this help\n\n";

    exit(EXIT_SUCCESS);
  }

  signal(SIGINT, terminate);
  signal(SIGTERM, terminate);
  signal(SIGPIPE, SIG_IGN);

  fs::path configPath(tkm::tkmDefaults.getFor(tkm::Defaults::Default::ConfPath));
  if (config_path != nullptr) {
    if (!fs::exists(config_path)) {
      cout << "Provided configuration file cannot be accessed: " << config_path << endl;
      return EXIT_FAILURE;
    }
    configPath = string(config_path);
  }

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
