#include <iostream>
#include <vector>

#include <sc-memory/sc_memory.hpp>


#include <sc-agents-common/utils/IteratorUtils.hpp>
#include <sc-agents-common/utils/AgentUtils.hpp>

#include "GraphUnionAgent.hpp"

using namespace utils;

ScAddr rrel_arcs, rrel_nodes, nrel_union;

namespace exampleModule
{

void printEl (const std::unique_ptr<ScMemoryContext>& context, ScAddr element)
{
    ScType type;
    type = context->GetElementType(element);

    if (type.IsNode() == ScType::Node)
    {
        std::string data;
        data = context->HelperGetSystemIdtf(element);
        std::cout << data;
    }
    else
    {
        ScAddr source, target;
        source = context->GetEdgeSource(element);
        target = context->GetEdgeTarget(element);
        std::cout << "(";
        printEl(context, source);
        std::cout << " -> ";
        printEl(context, target);
        std::cout << ")";
    }
}

bool findVertexInSet (const std::unique_ptr<ScMemoryContext>& context, ScAddr element, ScAddr set)
{
    bool find = false;
    // Получение всех элементов множества
    ScIterator3Ptr location = context->Iterator3(
            set,
            ScType::EdgeAccessConstPosPerm,
            ScType(0)
            );
    // Итерируемся по всем элементам
    while (location->Next())
    {
        // В loc записываем адрес конкретного элемента
        // принадлежащего множеству
        ScAddr loc = location->Get(2);

        if (loc != element)
        {
            find = false;
            continue;
        }
        else
        {
            find = true;
            break;
        }
    }
    return find;
}

void getEdgeNodes (const std::unique_ptr<ScMemoryContext>& context, ScAddr edge, ScAddr &v1, ScAddr &v2)
{
    v1 = context->GetEdgeSource(edge);
    v2 = context->GetEdgeTarget(edge);
}

ScAddr getGraphEdges(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph){
    ScAddr graphEdges;
    
    // Ищем множество дуг графа
    ScIterator5Ptr it = context->Iterator5(
        graph,
        ScType::EdgeAccessConstPosPerm,
        ScType(0),
        ScType::EdgeAccessConstPosPerm,
        rrel_arcs
    );

    if (it->Next()){
        // В edges записываем адрес найденного элемента
        graphEdges = it->Get(2);
    }else{
        std::cout<<"graph doesn't have edges"<<std::endl;
    }

    return graphEdges;
}

ScIterator3Ptr getEdgesIterator(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph){
    ScAddr graphEdges = getGraphEdges(context, graph);

    // Ищем все дуги графа
    ScIterator3Ptr edgesIterator = context->Iterator3(
        graphEdges,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    );

    return edgesIterator;
}

ScAddr getGraphNodes(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph){
    ScAddr graphNodes; 

    // Ищем множество врешин графа
    ScIterator5Ptr it = context->Iterator5(
        graph,
        ScType::EdgeAccessConstPosPerm,
        ScType(0),
        ScType::EdgeAccessConstPosPerm,
        rrel_nodes
    );

    if (it->Next()){
        // В nodes записываем адрес найденного элемента
        graphNodes = it->Get(2);
    }else{
        std::cout<<"graph doesn't have nodes"<<std::endl;
    }

    return graphNodes;
}

ScIterator3Ptr getNodesIterator(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph){
    ScAddr graphNodes = getGraphNodes(context, graph);

    // Ищем все дуги графа
    ScIterator3Ptr nodesIterator = context->Iterator3(
        graphNodes,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    );

    return nodesIterator;
}

void printGraph(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph){
    ScAddr arcs, nodes, v1, v2, printed_vertex;
    bool find;
    printed_vertex = context->CreateNode(ScType::Const);

    printEl(context, graph);
    std::cout << std::endl << "==================" << std::endl;

    ScIterator3Ptr edgesIterator = getEdgesIterator(context, graph);
    while (edgesIterator->Next()){
        ScAddr edge = edgesIterator->Get(2);

        getEdgeNodes (context,edge, v1, v2);
        printEl(context, v1);
        std::cout << " --> ";
        printEl(context, v2);
        std::cout << std::endl;

        // Если вершины не были напечатаны (т.е. не принадлежат множеству
        // напечатанных вершин), то заносим их туда (т.е. создаём дугу принадлежности)
        if (!findVertexInSet(context, v1, printed_vertex))
            context->CreateEdge(ScType::EdgeAccessConstPosPerm, printed_vertex, v1);
        if (!findVertexInSet(context, v2, printed_vertex))
            context->CreateEdge(ScType::EdgeAccessConstPosPerm, printed_vertex, v2);
    }

    ScIterator3Ptr nodesIterator = getNodesIterator(context, graph);
    while (nodesIterator->Next())
    {
        ScAddr node = nodesIterator->Get(2);
        //Проверяем, напечатана ли рассматриваемая вершина 
        find = findVertexInSet(context,node, printed_vertex);

        // Если вершина не была напечатана, то печатаем её
        if (!find)
        {
            printEl(context, node);
            std::cout << std::endl;
        }
    }
}

ScAddr createNewGraph(const std::unique_ptr<ScMemoryContext>& context, string graphName){
    ScAddr newGraph = context->HelperResolveSystemIdtf(graphName, ScType::NodeConst);

    context->CreateEdge(ScType::EdgeAccessConstPosPerm, Keynodes::graph, newGraph);


    ScAddr graphNodes = context->CreateNode(ScType::Const);
    ScAddr graphEdges = context->CreateNode(ScType::Const);

    ScAddr edgeNewGraphTographNodes = context->CreateEdge(ScType::EdgeAccessConstPosPerm, newGraph, graphNodes);
    ScAddr edgeNewGraphTographEdges = context->CreateEdge(ScType::EdgeAccessConstPosPerm, newGraph, graphEdges);

    context->CreateEdge(ScType::EdgeAccessConstPosPerm, rrel_nodes, edgeNewGraphTographNodes);
    context->CreateEdge(ScType::EdgeAccessConstPosPerm, rrel_arcs, edgeNewGraphTographEdges);

    return newGraph;
}

ScAddr addNodeToGraph(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph, string nodeName){
    ScAddr newNode = context->HelperResolveSystemIdtf(nodeName, ScType::NodeConst);

    ScAddr nodes = getGraphNodes(context, graph);
    context->CreateEdge(ScType::EdgeAccessConstPosPerm, nodes, newNode);

    return newNode;
}

ScAddr addEdgeToGraph(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph, string sourceNodeName, string targetNodeName){
    ScAddr sourceNode = context->HelperResolveSystemIdtf(sourceNodeName);
    ScAddr targetNode = context->HelperResolveSystemIdtf(targetNodeName);

    ScAddr edges = getGraphEdges(context, graph);
    ScAddr edgeBtwNodes = context->CreateEdge(ScType::EdgeDCommonConst, sourceNode, targetNode);
    context->CreateEdge(ScType::EdgeAccessConstPosPerm, edges, edgeBtwNodes);

    return edgeBtwNodes;
}

ScAddr getGraphUnion(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph1, ScAddr graph2){
    std::string firstGraphName = context->HelperGetSystemIdtf(graph1);
    std::string secondGraphName = context->HelperGetSystemIdtf(graph2);
    
    ScAddr resultGraph = createNewGraph(context, firstGraphName+" U "+secondGraphName);

    ScIterator3Ptr firstGraphNodesIterator = getNodesIterator(context, graph1);

    while (firstGraphNodesIterator->Next()){
        ScAddr node = firstGraphNodesIterator->Get(2);
        std::string nodeName = context->HelperGetSystemIdtf(node);
        addNodeToGraph(context, resultGraph, nodeName);
    }


    ScIterator3Ptr secondGraphNodesIterator = getNodesIterator(context, graph2);
    ScIterator3Ptr resultGraphNodesIterator;

    bool isNodeExistInResultGraph;
    while (secondGraphNodesIterator->Next()){
        isNodeExistInResultGraph = false;
        ScAddr nodeOfSecondGraph = secondGraphNodesIterator->Get(2);
        std::string nodeOfSecondGraphName = context->HelperGetSystemIdtf(nodeOfSecondGraph);

        resultGraphNodesIterator = getNodesIterator(context, resultGraph);
        while (resultGraphNodesIterator->Next()){
            ScAddr nodeOfResultGraph = resultGraphNodesIterator->Get(2);
            std::string nodeOfResultGraphName = context->HelperGetSystemIdtf(nodeOfResultGraph);
            if (nodeOfSecondGraphName==nodeOfResultGraphName){
                isNodeExistInResultGraph = true;
                break;
            }
        }

        if (isNodeExistInResultGraph == false){
            addNodeToGraph(context, resultGraph, nodeOfSecondGraphName);
        }
        
    }


    ScAddr sourceNodeOfFirstGraph, targetNodeOfFirstGraph,
            sourceNodeOfSecondGraph, targetNodeOfSecondGraph,
            sourceNodeOfResultGraph, targetNodeOfResultGraph;

    ScIterator3Ptr firstGraphEdgesIterator, secondGraphEdgesIterator, resultGarphEdgeIterator;

    firstGraphEdgesIterator = getEdgesIterator(context, graph1);
    while (firstGraphEdgesIterator->Next()){
        ScAddr firstGraphEdge = firstGraphEdgesIterator->Get(2);
        getEdgeNodes (context, firstGraphEdge, sourceNodeOfFirstGraph, targetNodeOfFirstGraph);
        std::string sourceNodeOfFirstGraphName = context->HelperGetSystemIdtf(sourceNodeOfFirstGraph);
        std::string targetNodeOfFirstGraphName = context->HelperGetSystemIdtf(targetNodeOfFirstGraph);

        addEdgeToGraph(context, resultGraph, sourceNodeOfFirstGraphName, targetNodeOfFirstGraphName);
    }

    secondGraphEdgesIterator = getEdgesIterator(context, graph2);
    
    bool isEdgeExistInResultGraph;
    while (secondGraphEdgesIterator->Next()){
        isEdgeExistInResultGraph = false;
        ScAddr secondGraphEdge = secondGraphEdgesIterator->Get(2);
        getEdgeNodes (context, secondGraphEdge, sourceNodeOfSecondGraph, targetNodeOfSecondGraph);
        std::string sourceNodeOfSecondGraphName = context->HelperGetSystemIdtf(sourceNodeOfSecondGraph);
        std::string targetNodeOfSecondGraphName = context->HelperGetSystemIdtf(targetNodeOfSecondGraph);

        resultGarphEdgeIterator = getEdgesIterator(context, resultGraph);
        while (resultGarphEdgeIterator->Next()){
            ScAddr resultGraphEdge = resultGarphEdgeIterator->Get(2);
            getEdgeNodes (context, resultGraphEdge, sourceNodeOfResultGraph, targetNodeOfResultGraph);
            std::string sourceNodeOfResultGraphName = context->HelperGetSystemIdtf(sourceNodeOfResultGraph);
            std::string targetNodeOfResultGraphName = context->HelperGetSystemIdtf(targetNodeOfResultGraph);

            if (sourceNodeOfResultGraphName == sourceNodeOfSecondGraphName && targetNodeOfResultGraphName == targetNodeOfSecondGraphName){
                isEdgeExistInResultGraph = true;
                break;
            }
        }


        if (isEdgeExistInResultGraph == false){
            addEdgeToGraph(context, resultGraph, sourceNodeOfSecondGraphName, targetNodeOfSecondGraphName);
        }
    }

    ScAddr tuple = context->CreateNode(ScType::NodeConstTuple);
    
    context->CreateEdge(ScType::EdgeAccessConstPosPerm, tuple, graph1);
    context->CreateEdge(ScType::EdgeAccessConstPosPerm, tuple, graph2);
    ScAddr edgeBtwTupleAndResultGraph = context->CreateEdge(ScType::EdgeDCommonConst, tuple, resultGraph);

    context->CreateEdge(ScType::EdgeAccessConstPosPerm, nrel_union, edgeBtwTupleAndResultGraph);

    return resultGraph; 
}

void testGraphUnion(const std::unique_ptr<ScMemoryContext>& context, ScAddr graph1, ScAddr graph2){
    printGraph(context, graph1);
    printGraph(context, graph2);

    ScAddr resultGraph = getGraphUnion(context, graph1, graph2);
    std::cout<<"\n\n\n\n\n\n\n\n===========GRAPH UNION==========="<<std::endl;
    printGraph(context, resultGraph);
}

SC_AGENT_IMPLEMENTATION(GraphUnionAgent)
{
    SC_LOG_INFO("GraphUnionAgent started");

    rrel_arcs = ms_context->HelperResolveSystemIdtf("rrel_arcs");
    rrel_nodes = ms_context->HelperResolveSystemIdtf("rrel_nodes");
    nrel_union = ms_context->HelperResolveSystemIdtf("nrel_graph_union", ScType::NodeConstNoRole);


    ScIterator3Ptr graphsIterator = ms_context->Iterator3(
        otherAddr,
        ScType::EdgeAccessConstPosPerm,
        ScType(0)
    );

    std::vector<ScAddr> graphs;

    while (graphsIterator->Next()){
        ScAddr graph = graphsIterator->Get(2);
        graphs.push_back(graph);
    }
    
    testGraphUnion(ms_context, graphs[0], graphs[1]);
 
    utils::AgentUtils::finishAgentWork(ms_context.get(), otherAddr, true);
    SC_LOG_INFO("GraphUnionAgent: finished");
    return SC_RESULT_OK;
}



} // namespace exampleModule
