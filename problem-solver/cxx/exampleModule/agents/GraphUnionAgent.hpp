#pragma once

#include <sc-memory/kpm/sc_agent.hpp>

#include "keynodes/keynodes.hpp"
#include "GraphUnionAgent.generated.hpp"

namespace exampleModule
{

class GraphUnionAgent : public ScAgent
{
  SC_CLASS(Agent, Event(Keynodes::question_graph_union_processing, ScEvent::Type::AddOutputEdge))
  SC_GENERATED_BODY()
};

} // namespace exampleModule
