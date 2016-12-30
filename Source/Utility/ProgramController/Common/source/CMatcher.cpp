
#include "../include/CMatcher.h"

#include <boost/property_tree/ptree.hpp>

#include <sstream>

namespace Utility
{
namespace ProgramController
{
   
   struct CMatch : API::IMatch
   {     
      CMatch( std::string const& name, std::shared_ptr<IProcess> const& process ) : m_name(name)
      {
         m_processes.emplace(process);
      }
      
      CMatch( std::string const& name, IProcess::SetType const& processes ) : m_name(name)
      {  
         for ( auto& process : processes )
         {  
            m_processes.emplace(process);
         }      
      }
      
      virtual std::string getName() const override
      {  return m_name; }
      
      virtual std::string toString() const override
      {  
         std::ostringstream os;
         os << getName() << ":" << std::accumulate(m_processes.begin(), m_processes.end(), std::string(), [](std::string v, std::shared_ptr<IProcess> const& process)
         {
            return v + " " + std::to_string(process->getProcessId());
         });
         return os.str();
      }
      
      virtual IProcess::SetType const& getProcesses() const override
      {  return m_processes; }
      
   private:   
      std::string m_name;
      IProcess::SetType m_processes;
   };
   
   CMatcher::CMatcher() :
      m_logger( log4cxx::Logger::getLogger( "Utility.ProgramController.CMatcher" ) )
   {}
   
   API::IMatcher& CMatcher::add( CMatchingRule rule )
   {
      m_rules.emplace(std::move(rule));
      return *this;
   }
   
   CMatchingRule::SetType const& CMatcher::getRules() const
   {
      return m_rules;
   }
     
   API::IMatch::SetType CMatcher::matches( std::shared_ptr<IProcess> const& process ) const
   {
      if ( m_rules.empty() )
      {  return API::IMatch::SetType(); }
      
      std::map< std::string, IProcess::SetType > temporary;
      for ( auto const& rule : m_rules )
      {
         if (rule.matches(process->getCommandLine()))
         {
            LOG4CXX_TRACE(m_logger, "Rule '" + rule.getName() + "' matches: " + process->toString());
            temporary[rule.getName()].emplace(process);
         }  
      }
      
      API::IMatch::SetType results;
      for ( auto& tuple : temporary ) { results.emplace(std::make_unique<CMatch>(tuple.first, tuple.second)); }
      return std::move( results );
   }
   
   API::IMatch::SetType CMatcher::matches( IProcess::SetType const& processes ) const
   {
      std::map< std::string, IProcess::SetType > temporary;
      for ( auto& process : processes )
      {  
         for ( auto const& match : matches( process ) )
         {
            temporary[ match->getName() ].emplace(process);
         }
      }
      API::IMatch::SetType results;
      for ( auto& tuple : temporary ) { results.emplace(std::make_unique<CMatch>(tuple.first, tuple.second)); }
      return std::move( results );
   }

   boost::property_tree::ptree CMatcher::serialize() const
   {  
      boost::property_tree::ptree pt;
      std::for_each(m_rules.begin(), m_rules.end(), [&](auto const& rule)
      {
         pt.add_child( "rules.rule", rule.serialize());
      });
      return std::move(pt);
   }
   
   std::unique_ptr<API::IMatcher> CMatcher::deserialize(boost::property_tree::ptree const& ptMatch)
   {
      auto matcher(std::make_unique<CMatcher>());
      for (auto const& ptRule : ptMatch.get_child( "rules" ).get_child( "" ))
      {
         matcher->add(CMatchingRule::deserialize(ptRule.second));
      }
      return std::move(matcher);      
   }
}}
