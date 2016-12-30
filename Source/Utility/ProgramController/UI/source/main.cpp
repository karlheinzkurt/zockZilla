
#include "Utility/ProgramController/Common/include/CProgramController.h"
#include "Utility/ProgramController/Common/include/CRunningStrategy.h"
#include "Infrastructure/Linux/include/CSystem.h"

#include <log4cxx/xml/domconfigurator.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <iostream>

int main( int argc, char** argv )
{  
   namespace po = boost::program_options;
   namespace fs = boost::filesystem;
   
   auto const executablePath(fs::path(argv[0]).parent_path());
   
   po::options_description desc("General options");
   desc.add_options()
      ("help,h"
          // no argument
         ,"This help message")
      ("daemon,d"
          // no argument
         ,"Run continuously")
      ("check-interval,i"      
         ,po::value< uint64_t >()->default_value( std::chrono::seconds( 60 ).count() )
         ,"Interval to check for matching processes in seconds" )
      ("counter-file,c"
         ,po::value< fs::path >()->default_value(executablePath / Utility::ProgramController::CProgramController::getDefaultCounterFilePath())
         ,"Path to file containing counter information") 
      ("log-file,l"
         ,po::value< fs::path >()->default_value(executablePath / Utility::ProgramController::CProgramController::getDefaultLogFilePath())
         ,"Path to log-file")
      ("config-file"
         ,po::value< fs::path >()->default_value(executablePath / Utility::ProgramController::CProgramController::getDefaultConfigurationFilePath())
         ,"Path to configuration file")
      ("logger-config-file"
         ,po::value< fs::path >()->default_value(executablePath / Utility::ProgramController::CProgramController::getDefaultLoggerConfigurationFilePath())
         ,"Path to logger configuration file");

   try
   {
      po::variables_map vm;
      po::store(po::parse_command_line(argc, argv, desc), vm);
      po::notify(vm);

      if (vm.count("help"))
      {
         std::cout << desc << "\n";
         return 0;
      }

      auto const counterFilePath(vm[ "counter-file" ].as<fs::path>());
      auto const logFilePath(vm[ "log-file" ].as<fs::path>());
      auto const configurationFilePath(vm[ "config-file" ].as<fs::path>());
      auto const loggerConfigurationFilePath(vm[ "logger-config-file" ].as<fs::path>());
      auto const checkInterval(std::chrono::seconds(vm["check-interval"].as<size_t>()));
      auto const runningStrategy(vm.count("daemon") > 0 
         ? std::unique_ptr<Utility::ProgramController::API::IRunningStrategy>(new Utility::ProgramController::CDaemonRunningStrategy(checkInterval))
         : std::unique_ptr<Utility::ProgramController::API::IRunningStrategy>(new Utility::ProgramController::CStraightRunningStrategy));
      
      setenv("logfile.path", logFilePath.string().c_str(), 1);
      log4cxx::xml::DOMConfigurator::configure( loggerConfigurationFilePath.string() );

      Infrastructure::Linux::CSystem system;
      auto controller(std::make_unique<Utility::ProgramController::CProgramController>(
          system
         ,*runningStrategy
         ,configurationFilePath
         ,counterFilePath));
      
      /** \todo Use signal handler stuff to enable daemon to re-read the config or to shutdown properly
          \todo Maybe move this signal handler stuff somewhere
       */
      std::vector<std::unique_ptr<Infrastructure::API::ISignalHandler>> handlers;
      handlers.emplace_back(system.installSignalHandler(Infrastructure::API::Signal::Hup,  [](auto signal){ /* \todo handle */ }));
      handlers.emplace_back(system.installSignalHandler(Infrastructure::API::Signal::Term, [](auto signal){ /* \todo handle */ }));
   }
   catch ( std::exception const& e )
   {
      std::cerr << "Error: " << e.what() << std::endl;
      return -1;
   }
   return 0;   
}
