// -*- c++ -*-

// Copyright 2009-2021 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2021, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef SST_CORE_CONFIGGRAPH_H
#define SST_CORE_CONFIGGRAPH_H

#include "sst/core/params.h"
#include "sst/core/rankInfo.h"
#include "sst/core/serialization/serializable.h"
#include "sst/core/sparseVectorMap.h"
#include "sst/core/sst_types.h"
#include "sst/core/statapi/statbase.h"
#include "sst/core/statapi/statoutput.h"
#include "sst/core/unitAlgebra.h"

#include <climits>
#include <map>
#include <set>
#include <vector>

using namespace SST::Statistics;

namespace SST {

class Simulation_impl;

class Config;
class TimeLord;
class ConfigGraph;

typedef SparseVectorMap<ComponentId_t> ComponentIdMap_t;
typedef std::vector<LinkId_t>          LinkIdMap_t;

/** Represents the configuration of a generic Link */
class ConfigLink : public SST::Core::Serialization::serializable
{
public:
    LinkId_t      id;             /*!< ID of this link */
    LinkId_t      remote_tag;     /*!< Tag used to ensure remote events are delivered correctly ID of this link */
    std::string   name;           /*!< Name of this link */
    ComponentId_t component[2];   /*!< IDs of the connected components */
    std::string   port[2];        /*!< Names of the connected ports */
    SimTime_t     latency[2];     /*!< Latency from each side */
    std::string   latency_str[2]; /*!< Temp string holding latency */
    int           current_ref;    /*!< Number of components currently referring to this Link */
    bool          no_cut;         /*!< If set to true, partitioner will not make a cut through this Link */

    // inline const std::string& key() const { return name; }
    inline LinkId_t key() const { return id; }

    /** Return the minimum latency of this link (from both sides) */
    SimTime_t getMinLatency() const
    {
        if ( latency[0] < latency[1] ) return latency[0];
        return latency[1];
    }

    /** Print the Link information */
    void print(std::ostream& os) const
    {
        os << "Link " << name << " (id = " << id << ")" << std::endl;
        os << "  remote_tag = " << remote_tag << std::endl;
        os << "  component[0] = " << component[0] << std::endl;
        os << "  port[0] = " << port[0] << std::endl;
        os << "  latency[0] = " << latency[0] << std::endl;
        os << "  component[1] = " << component[1] << std::endl;
        os << "  port[1] = " << port[1] << std::endl;
        os << "  latency[1] = " << latency[1] << std::endl;
    }

    /* Do not use.  For serialization only */
    ConfigLink() {}

    void serialize_order(SST::Core::Serialization::serializer& ser) override
    {
        ser& id;
        ser& remote_tag;
        ser& name;
        ser& component[0];
        ser& component[1];
        ser& port[0];
        ser& port[1];
        ser& latency[0];
        ser& latency[1];
        ser& latency_str[0];
        ser& latency_str[1];
        ser& current_ref;
    }

    ImplementSerializable(SST::ConfigLink)

private:
    friend class ConfigGraph;
    ConfigLink(LinkId_t id) : id(id), remote_tag(id), no_cut(false)
    {
        current_ref = 0;

        // Initialize the component data items
        component[0] = ULONG_MAX;
        component[1] = ULONG_MAX;
    }

    ConfigLink(LinkId_t id, const std::string& n) : id(id), remote_tag(id), no_cut(false)
    {
        current_ref = 0;
        name        = n;

        // Initialize the component data items
        component[0] = ULONG_MAX;
        component[1] = ULONG_MAX;
    }

    void updateLatencies(TimeLord*);
};

class ConfigStatistic : public SST::Core::Serialization::serializable
{
public:
    StatisticId_t id; /*!< Unique ID of this statistic */
    Params        params;
    bool          shared;
    std::string   name;

    ConfigStatistic(StatisticId_t _id, bool _shared = false, std::string _name = "") :
        id(_id),
        shared(_shared),
        name(_name)
    {}

    ConfigStatistic() : id(stat_null_id) {}

    inline const StatisticId_t& getId() const { return id; }

    void addParameter(const std::string& key, const std::string& value, bool overwrite);

    void serialize_order(SST::Core::Serialization::serializer& ser) override
    {
        ser& id;
        ser& shared;
        ser& name;
        ser& params;
    }

    ImplementSerializable(ConfigStatistic)

    static constexpr StatisticId_t stat_null_id = std::numeric_limits<StatisticId_t>::max();
};

class ConfigStatGroup : public SST::Core::Serialization::serializable
{
public:
    std::string                   name;
    std::map<std::string, Params> statMap;
    std::vector<ComponentId_t>    components;
    size_t                        outputID;
    UnitAlgebra                   outputFrequency;

    ConfigStatGroup(const std::string& name) : name(name), outputID(0) {}
    ConfigStatGroup() {} /* Do not use */

    bool addComponent(ComponentId_t id);
    bool addStatistic(const std::string& name, Params& p);
    bool setOutput(size_t id);
    bool setFrequency(const std::string& freq);

    /**
     * Checks to make sure that all components in the group support all
     * of the statistics as configured in the group.
     * @return pair of:  bool for OK, string for error message (if any)
     */
    std::pair<bool, std::string> verifyStatsAndComponents(const ConfigGraph* graph);

    void serialize_order(SST::Core::Serialization::serializer& ser) override
    {
        ser& name;
        ser& statMap;
        ser& components;
        ser& outputID;
        ser& outputFrequency;
    }

    ImplementSerializable(SST::ConfigStatGroup)
};

class ConfigStatOutput : public SST::Core::Serialization::serializable
{
public:
    std::string type;
    Params      params;

    ConfigStatOutput(const std::string& type) : type(type) {}
    ConfigStatOutput() {}

    void addParameter(const std::string& key, const std::string& val) { params.insert(key, val); }

    void serialize_order(SST::Core::Serialization::serializer& ser) override
    {
        ser& type;
        ser& params;
    }

    ImplementSerializable(SST::ConfigStatOutput)
};

typedef SparseVectorMap<LinkId_t, ConfigLink> ConfigLinkMap_t;

/** Represents the configuration of a generic component */
class ConfigComponent : public SST::Core::Serialization::serializable
{
    friend class ComponentInfo;

public:
    ComponentId_t         id;            /*!< Unique ID of this component */
    ConfigGraph*          graph;         /*!< Graph that this component belongs to */
    std::string           name;          /*!< Name of this component, or slot name for subcomp */
    int                   slot_num;      /*!< Slot number.  Only valid for subcomponents */
    std::string           type;          /*!< Type of this component */
    float                 weight;        /*!< Partitioning weight for this component */
    RankInfo              rank;          /*!< Parallel Rank for this component */
    std::vector<LinkId_t> links;         /*!< List of links connected */
    Params                params;        /*!< Set of Parameters */
    uint8_t               statLoadLevel; /*!< Statistic load level for this component */
    // std::vector<ConfigStatistic>  enabledStatistics; /*!< List of subcomponents */

    std::map<std::string, StatisticId_t> enabledStatNames;
    bool                                 enabledAllStats;
    ConfigStatistic                      allStatConfig;

    std::vector<ConfigComponent*> subComponents; /*!< List of subcomponents */
    std::vector<double>           coords;
    uint16_t nextSubID;  /*!< Next subID to use for children, if component, if subcomponent, subid of parent */
    bool     visited;    /*! Used when traversing graph to indicate component was visited already */
    uint16_t nextStatID; /*!< Next statID to use for children */

    static constexpr ComponentId_t null_id = std::numeric_limits<ComponentId_t>::max();

    inline const ComponentId_t& key() const { return id; }

    /** Print Component information */
    void print(std::ostream& os) const;

    ConfigComponent* cloneWithoutLinks(ConfigGraph* new_graph) const;
    ConfigComponent* cloneWithoutLinksOrParams(ConfigGraph* new_graph) const;
    void             setConfigGraphPointer(ConfigGraph* graph_ptr);

    ~ConfigComponent() {}
    ConfigComponent() :
        id(null_id),
        statLoadLevel(STATISTICLOADLEVELUNINITIALIZED),
        enabledAllStats(false),
        nextSubID(1),
        visited(false)
    {}

    StatisticId_t getNextStatisticID();

    ConfigComponent* getParent() const;
    std::string      getFullName() const;

    void                   setRank(RankInfo r);
    void                   setWeight(double w);
    void                   setCoordinates(const std::vector<double>& c);
    void                   addParameter(const std::string& key, const std::string& value, bool overwrite);
    ConfigComponent*       addSubComponent(const std::string& name, const std::string& type, int slot);
    ConfigComponent*       findSubComponent(ComponentId_t);
    const ConfigComponent* findSubComponent(ComponentId_t) const;
    ConfigComponent*       findSubComponentByName(const std::string& name);
    ConfigStatistic*       findStatistic(const std::string& name) const;
    ConfigStatistic*       insertStatistic(StatisticId_t id);
    ConfigStatistic*       findStatistic(StatisticId_t) const;
    ConfigStatistic*
    enableStatistic(const std::string& statisticName, const SST::Params& params, bool recursively = false);
    ConfigStatistic* createStatistic();
    bool             reuseStatistic(const std::string& statisticName, StatisticId_t sid);
    void             addStatisticParameter(
                    const std::string& statisticName, const std::string& param, const std::string& value, bool recursively = false);
    void setStatisticParameters(const std::string& statisticName, const Params& params, bool recursively = false);
    void setStatisticLoadLevel(uint8_t level, bool recursively = false);

    void                     addGlobalParamSet(const std::string& set) { params.addGlobalParamSet(set); }
    std::vector<std::string> getParamsLocalKeys() const { return params.getLocalKeys(); }
    std::vector<std::string> getSubscribedGlobalParamSets() const { return params.getSubscribedGlobalParamSets(); }


    std::vector<LinkId_t> allLinks() const;

    void serialize_order(SST::Core::Serialization::serializer& ser) override
    {
        ser& id;
        ser& name;
        ser& slot_num;
        ser& type;
        ser& weight;
        ser& rank.rank;
        ser& rank.thread;
        ser& links;
        ser& params;
        ser& enabledStatNames;
        ser& enabledAllStats;
        ser& statistics;
        ser& enabledAllStats;
        ser& statLoadLevel;
        ser& subComponents;
        ser& coords;
        ser& nextSubID;
        ser& nextStatID;
    }

    ImplementSerializable(SST::ConfigComponent)

private:
    std::map<StatisticId_t, ConfigStatistic> statistics;

    ComponentId_t getNextSubComponentID();

    friend class ConfigGraph;
    /** Checks to make sure port names are valid and that a port isn't used twice
     */
    void checkPorts() const;

    /** Create a new Component */
    ConfigComponent(
        ComponentId_t id, ConfigGraph* graph, const std::string& name, const std::string& type, float weight,
        RankInfo rank) :
        id(id),
        graph(graph),
        name(name),
        type(type),
        weight(weight),
        rank(rank),
        statLoadLevel(STATISTICLOADLEVELUNINITIALIZED),
        enabledAllStats(false),
        nextSubID(1),
        nextStatID(1)
    {
        coords.resize(3, 0.0);
    }

    ConfigComponent(
        ComponentId_t id, ConfigGraph* graph, uint16_t parent_subid, const std::string& name, int slot_num,
        const std::string& type, float weight, RankInfo rank) :
        id(id),
        graph(graph),
        name(name),
        slot_num(slot_num),
        type(type),
        weight(weight),
        rank(rank),
        statLoadLevel(STATISTICLOADLEVELUNINITIALIZED),
        enabledAllStats(false),
        nextSubID(parent_subid),
        nextStatID(parent_subid)
    {
        coords.resize(3, 0.0);
    }
};

/** Map names to Links */
// typedef std::map<std::string,ConfigLink> ConfigLinkMap_t;
// typedef SparseVectorMap<std::string,ConfigLink> ConfigLinkMap_t;
/** Map IDs to Components */
typedef SparseVectorMap<ComponentId_t, ConfigComponent*> ConfigComponentMap_t;
/** Map names to Components */
typedef std::map<std::string, ComponentId_t>             ConfigComponentNameMap_t;
/** Map names to Parameter Sets: XML only */
typedef std::map<std::string, Params*>                   ParamsMap_t;
/** Map names to variable values:  XML only */
typedef std::map<std::string, std::string>               VariableMap_t;

class PartitionGraph;

/** A Configuration Graph
 *  A graph representing Components and Links
 */
class ConfigGraph : public SST::Core::Serialization::serializable
{
public:
    /** Print the configuration graph */
    void print(std::ostream& os) const
    {
        os << "Printing graph" << std::endl;
        for ( ConfigComponentMap_t::const_iterator i = comps.begin(); i != comps.end(); ++i ) {
            (*i)->print(os);
        }
    }

    ConfigGraph() : output(Output::getDefaultObject()), nextComponentId(0)
    {
        links.clear();
        comps.clear();
        // Init the statistic output settings
        statLoadLevel = STATISTICSDEFAULTLOADLEVEL;
        statOutputs.emplace_back(STATISTICSDEFAULTOUTPUTNAME);
    }

    size_t getNumComponents() { return comps.data.size(); }

    /** Helper function to set all the ranks to the same value */
    void setComponentRanks(RankInfo rank);
    /** Checks to see if rank contains at least one component */
    bool containsComponentInRank(RankInfo rank);
    /** Verify that all components have valid Ranks assigned */
    bool checkRanks(RankInfo ranks);

    // API for programmatic initialization
    /** Create a new component with weight and rank */
    ComponentId_t addComponent(const std::string& name, const std::string& type, float weight, RankInfo rank);
    /** Create a new component */
    ComponentId_t addComponent(const std::string& name, const std::string& type);

    /** Add a parameter to a global param set */
    void addGlobalParam(const std::string& global_set, const std::string& key, const std::string& value);

    /** Set the statistic output module */
    void setStatisticOutput(const std::string& name);

    /** Add parameter to the statistic output module */
    void addStatisticOutputParameter(const std::string& param, const std::string& value);

    /** Set a set of parameter to the statistic output module */
    void setStatisticOutputParams(const Params& p);

    /** Set the statistic system load level */
    void setStatisticLoadLevel(uint8_t loadLevel);

    std::vector<ConfigStatOutput>& getStatOutputs() { return statOutputs; }

    const ConfigStatOutput& getStatOutput(size_t index = 0) const { return statOutputs[index]; }

    long getStatLoadLevel() const { return statLoadLevel; }

    /** Add a Link to a Component on a given Port */
    void addLink(
        ComponentId_t comp_id, const std::string& link_name, const std::string& port, const std::string& latency_str,
        bool no_cut = false);

    /** Set a Link to be no-cut */
    void setLinkNoCut(const std::string& link_name);

    /** Perform any post-creation cleanup processes */
    void postCreationCleanup();

    /** Check the graph for Structural errors */
    bool checkForStructuralErrors();

    // Temporary until we have a better API
    /** Return the map of components */
    ConfigComponentMap_t& getComponentMap() { return comps; }

    const std::map<std::string, ConfigStatGroup>& getStatGroups() const { return statGroups; }
    ConfigStatGroup*                              getStatGroup(const std::string& name)
    {
        auto found = statGroups.find(name);
        if ( found == statGroups.end() ) {
            bool ok;
            std::tie(found, ok) = statGroups.emplace(name, name);
        }
        return &(found->second);
    }

    bool                   containsComponent(ComponentId_t id) const;
    ConfigComponent*       findComponent(ComponentId_t);
    ConfigComponent*       findComponentByName(const std::string& name);
    const ConfigComponent* findComponent(ComponentId_t) const;

    bool             containsStatistic(StatisticId_t id) const;
    ConfigStatistic* findStatistic(StatisticId_t) const;

    /** Return the map of links */
    ConfigLinkMap_t& getLinkMap() { return links; }

    ConfigGraph* getSubGraph(uint32_t start_rank, uint32_t end_rank);
    ConfigGraph* getSubGraph(const std::set<uint32_t>& rank_set);

    PartitionGraph* getPartitionGraph();
    PartitionGraph* getCollapsedPartitionGraph();
    void            annotateRanks(PartitionGraph* graph);
    void            getConnectedNoCutComps(ComponentId_t start, std::set<ComponentId_t>& group);

    void setComponentConfigGraphPointers();
    void serialize_order(SST::Core::Serialization::serializer& ser) override
    {
        ser& links;
        ser& comps;
        ser& statOutputs;
        ser& statLoadLevel;
        ser& statGroups;
        if ( ser.mode() == SST::Core::Serialization::serializer::UNPACK ) {
            // Need to reintialize the ConfigGraph ptrs in the
            // ConfigComponents
            setComponentConfigGraphPointers();
        }
    }

private:
    friend class Simulation_impl;
    friend class SSTSDLModelDefinition;

    Output& output;

    ComponentId_t nextComponentId;

    ConfigLinkMap_t                        links;       // SparseVectorMap
    ConfigComponentMap_t                   comps;       // SparseVectorMap
    ConfigComponentNameMap_t               compsByName; // std::map
    std::map<std::string, ConfigStatGroup> statGroups;

    std::map<std::string, LinkId_t> link_names;

    std::vector<ConfigStatOutput> statOutputs; // [0] is default
    uint8_t                       statLoadLevel;

    ImplementSerializable(SST::ConfigGraph)
};

class PartitionComponent
{
public:
    ComponentId_t id;
    float         weight;
    RankInfo      rank;
    LinkIdMap_t   links;

    ComponentIdMap_t group;

    PartitionComponent(const ConfigComponent* cc)
    {
        id     = cc->id;
        weight = cc->weight;
        rank   = cc->rank;
    }

    PartitionComponent(LinkId_t id) : id(id), weight(0), rank(RankInfo(RankInfo::UNASSIGNED, 0)) {}

    // PartitionComponent(ComponentId_t id, ConfigGraph* graph, const ComponentIdMap_t& group);
    void print(std::ostream& os, const PartitionGraph* graph) const;

    inline ComponentId_t key() const { return id; }
};

class PartitionLink
{
public:
    LinkId_t      id;
    ComponentId_t component[2];
    SimTime_t     latency[2];
    bool          no_cut;

    PartitionLink(const ConfigLink& cl)
    {
        id           = cl.id;
        component[0] = cl.component[0];
        component[1] = cl.component[1];
        latency[0]   = cl.latency[0];
        latency[1]   = cl.latency[1];
        no_cut       = cl.no_cut;
    }

    inline LinkId_t key() const { return id; }

    /** Return the minimum latency of this link (from both sides) */
    SimTime_t getMinLatency() const
    {
        if ( latency[0] < latency[1] ) return latency[0];
        return latency[1];
    }

    /** Print the Link information */
    void print(std::ostream& os) const
    {
        os << "    Link " << id << std::endl;
        os << "      component[0] = " << component[0] << std::endl;
        os << "      latency[0] = " << latency[0] << std::endl;
        os << "      component[1] = " << component[1] << std::endl;
        os << "      latency[1] = " << latency[1] << std::endl;
    }
};

typedef SparseVectorMap<ComponentId_t, PartitionComponent*> PartitionComponentMap_t;
typedef SparseVectorMap<LinkId_t, PartitionLink>            PartitionLinkMap_t;

class PartitionGraph
{
private:
    PartitionComponentMap_t comps;
    PartitionLinkMap_t      links;

public:
    /** Print the configuration graph */
    void print(std::ostream& os) const
    {
        os << "Printing graph" << std::endl;
        for ( PartitionComponentMap_t::const_iterator i = comps.begin(); i != comps.end(); ++i ) {
            (*i)->print(os, this);
        }
    }

    PartitionComponentMap_t& getComponentMap() { return comps; }
    PartitionLinkMap_t&      getLinkMap() { return links; }

    const PartitionLink& getLink(LinkId_t id) const { return links[id]; }

    size_t getNumComponents() { return comps.size(); }
};

} // namespace SST

#endif // SST_CORE_CONFIGGRAPH_H
