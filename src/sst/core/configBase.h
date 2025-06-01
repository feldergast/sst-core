// Copyright 2009-2025 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2025, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef SST_CORE_CONFIGBASE_H
#define SST_CORE_CONFIGBASE_H

#include "sst/core/from_string.h"
#include "sst/core/sst_types.h"
#include "sst/core/warnmacros.h"
#include "sst/core/serialization/serialize.h"

#include <functional>
#include <getopt.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/* Forward declare for Friendship */
extern int main(int argc, char** argv);

namespace SST {

struct OptionDefinition
{

    OptionDefinition(std::function<std::string()> ext_help) :
        ext_help(ext_help)
    {}

    const std::function<std::string()> ext_help;

    virtual ~OptionDefinition()                  = default;
    virtual int  parse(std::string arg)          = 0;
    virtual void transfer(OptionDefinition* def) = 0;
    virtual void serialize(SST::Core::Serialization::serializer& UNUSED(ser)) {}
};

struct OptionDefinitionInformational : OptionDefinition
{
    const std::function<int(std::string)> print_info;

    OptionDefinitionInformational(std::function<int(std::string)> print_info) :
        OptionDefinition(nullptr),
        print_info(print_info)
    {}

    int  parse(std::string arg) override { return print_info(arg); }
    void transfer(OptionDefinition* UNUSED(def)) override { /* No data to transfer */ }
};

template <typename T>
struct OptionDefinitionImpl : OptionDefinition
{
    // Data members
    T                                         value = T();
    const std::function<int(T&, std::string)> parser;

    // Constructors
    OptionDefinitionImpl(T val, std::function<int(T&, std::string)> parser) :
        OptionDefinition(nullptr),
        value(val),
        parser(parser)
    {}

    OptionDefinitionImpl(T val, std::function<int(T&, std::string)> parser, std::function<std::string()> ext_help) :
        OptionDefinition(ext_help),
        value(val),
        parser(parser)
    {}

    // Function overloads.  This makes the OptionDefinitionImpl look
    // like a type T for assignments.  Assigning one
    // OptionDefinitionImpl to another will only copy the value.
    OptionDefinitionImpl& operator=(const T& val)
    {
        value = val;
        return *this;
    }

    OptionDefinitionImpl& operator=(const OptionDefinitionImpl& val)
    {
        value = val.value;
        return *this;
    }

    operator T() const { return value; }

    // Deleate the copy and move constructors
    OptionDefinitionImpl(const OptionDefinitionImpl&) = delete;
    OptionDefinitionImpl(OptionDefinitionImpl&&)      = default;

    // Utility functions used by the Config object to parse values
    // from the command line and copy values when types aren't known
    int  parse(std::string arg) override { return parser(value, arg); }
    void transfer(OptionDefinition* def) override { value = dynamic_cast<OptionDefinitionImpl<T>*>(def)->value; }

    void serialize(SST::Core::Serialization::serializer& ser) override
    {
        SST_SER(value);
    }
};


template <typename T, typename U>
struct OptionDefinitionPair : OptionDefinition
{
    // Data members
    T                                             value1 = T();
    U                                             value2 = U();
    const std::function<int(T&, U&, std::string)> parser;

    // Constructors
    OptionDefinitionPair(T val1, U val2, std::function<int(T&, U&, std::string)> parser) :
        OptionDefinition(nullptr),
        value1(val1),
        value2(val2),
        parser(parser)
    {}

    OptionDefinitionPair(
        T val1, U val2, std::function<int(T&, U&, std::string)> parser, std::function<std::string()> ext_help) :
        OptionDefinition(ext_help),
        value1(val1),
        value2(val2),
        parser(parser)
    {}

    // Deleate the copy and move constructors
    OptionDefinitionPair(const OptionDefinitionPair&) = delete;
    OptionDefinitionPair(OptionDefinitionPair&&)      = default;

    // Utility functions used by the Config object to parse values
    // from the command line and copy values when types aren't known
    int  parse(std::string arg) override { return parser(value1, value2, arg); }
    void transfer(OptionDefinition* def) override
    {
        value1 = dynamic_cast<OptionDefinitionPair<T, U>*>(def)->value1;
        value2 = dynamic_cast<OptionDefinitionPair<T, U>*>(def)->value2;
    }

    void serialize(SST::Core::Serialization::serializer& ser) override
    {
        SST_SER(value1);
        SST_SER(value2);
    }
};


#define APPEND_FOR_DECL_OPTION(first, second) first##second

#define DECL_OPTION(type, name, default_val, ...)                                                 \
                                                                                                  \
public:                                                                                           \
    type name() const                                                  \
    {                                                                                             \
        return APPEND_FOR_DECL_OPTION(name, _).value;                                            \
    }                                                                                             \
                                                                                                  \
private:                                                                                          \
    OptionDefinitionImpl<type> APPEND_FOR_DECL_OPTION(name, _) = { default_val, ##__VA_ARGS__ }; \
                                                                                                  \
public:


#define DECL_OPTION_PAIR(type1, name1, default_val1, type2, name2, default_val2, ...) \
                                                                                      \
public:                                                                               \
    type1 name1() const                                    \
    {                                                                                 \
        return APPEND_FOR_DECL_OPTION(name1, _).value1;                              \
    }                                                                                 \
                                                                                      \
    type2 name2() const                                    \
    {                                                                                 \
        return APPEND_FOR_DECL_OPTION(name1, _).value2;                              \
    }                                                                                 \
                                                                                      \
private:                                                                              \
    OptionDefinitionPair<type1, type2> APPEND_FOR_DECL_OPTION(                        \
        name1, _) = { default_val1, default_val2, ##__VA_ARGS__ };                   \
                                                                                      \
public:

#define DECL_OPTION_INFO(name, ...)                     \
    OptionDefinitionInformational APPEND_FOR_DECL_OPTION(name,_) = {__VA_ARGS__}; \
                                                        \
public:


/**** Default parsing functions ****/
namespace StandardConfigParsers {

template <typename T>
int
from_string(T& var, std::string arg)
{
    try {
        var = SST::Core::from_string<T>(arg);
    }
    catch ( std::exception& e ) {
        fprintf(stderr, "Failed to parse argument: %s\n", arg.c_str());
        return -1;
    }
    return 0;
}

template <typename T>
int
from_string_default(T& var, std::string arg, const T& default_value)
{
    if ( arg.empty() )
        var = default_value;
    else {
        try {
            var = SST::Core::from_string<T>(arg);
        }
        catch ( std::exception& e ) {
            fprintf(stderr, "Failed to parse argument: %s\n", arg.c_str());
            return -1;
        }
    }
    return 0;
}

template <typename T>
int
check_parse_store_string(std::string& var, std::string arg)
{
    T   check;
    int ret = from_string<T>(check, arg);
    if ( ret != 0 ) return ret;
    var = arg;
    return 0;
}

int nonempty_string(std::string& var, std::string arg);

int append_string(std::string pre, std::string post, std::string& var, std::string arg);

int flag_set_true(bool& var, std::string arg);

int flag_set_false(bool& var, std::string arg);

int flag_default_true(bool& var, std::string arg);

int flag_default_false(bool& var, std::string arg);

int wall_time_to_seconds(uint32_t& var, std::string arg);

int element_name(std::string& var, std::string arg);
} // namespace StandardConfigParsers


/**
   Struct that holds all the getopt_long options along with the
   docuementation for the option
*/

struct LongOption
{
    struct option     opt;
    std::string       argname;
    std::string       desc;
    bool              header; // if true, desc is actually the header
    std::vector<bool> annotations;
    mutable bool      set_cmdline;
    OptionDefinition* def;

    LongOption(struct option opt, const char* argname, const char* desc, bool header, std::vector<bool> annotations,
        OptionDefinition* def) :
        opt(opt),
        argname(argname),
        desc(desc),
        header(header),
        annotations(annotations),
        set_cmdline(false),
        def(def)
    {}
};

struct AnnotationInfo
{
    char        annotation;
    std::string help;
};

// Macros to make defining options easier.  These must be called
// inside of a member function of a class inheriting from ConfigBase
// Nomenaclature is:

// FLAG - value is either true or false.  FLAG defaults to no arguments allowed
// ARG - value is a string.  ARG defaults to required argument
// OPTVAL - Takes an optional paramater

// longName - multicharacter name referenced using --
// shortName - single character name referenced using -
// text - help text
// func - function called if option is found
#define DEF_FLAG_OPTVAL(longName, shortName, text, def, ...) \
    addOption({ longName, optional_argument, 0, shortName }, "[BOOL]", text, { __VA_ARGS__ }, &def);

#define DEF_FLAG(longName, shortName, text, def, ...) \
    addOption({ longName, no_argument, 0, shortName }, "", text, { __VA_ARGS__ }, &def);

#define DEF_ARG(longName, shortName, argName, text, def, ...) \
    addOption({ longName, required_argument, 0, shortName }, argName, text, { __VA_ARGS__ }, &def);

#define DEF_ARG_OPTVAL(longName, shortName, argName, text, def, ...) \
    addOption({ longName, optional_argument, 0, shortName }, "[" argName "]", text, { __VA_ARGS__ }, &def);


#define DEF_SECTION_HEADING(text) addHeading2(text);


/**
 * Base class to parse command line options for SST Simulation
 * Configuration variables.
 *
 * NOTE: This class contains only state for parsing the command line.
 * All options will be stored in classes derived from this class.
 * This means that we don't need to be able to serialize anything in
 * this class.
 */
class ConfigBase
{
public:
    /**
       Variable used to identify the currently parsing option
     */
    static std::string currently_parsing_option;

protected:
    /**
       ConfigBase constructor.  Meant to only be created by main
       function
     */
    explicit ConfigBase(bool suppress_print) :
        suppress_print_(suppress_print)
    {}

    /**
       Default constructor used for serialization.  After
       serialization, the Config object is only used to get the values
       of set options and it can no longer parse arguments.  Given
       that, it will no longer print anything, so set suppress_print_
       to true. None of this class needs to be serialized because it
       it's state is only for parsing the arguments.
     */
    ConfigBase() :
        suppress_print_(true)
    {
        options.reserve(100);
    }


    ConfigBase(bool suppress_print, std::vector<AnnotationInfo> annotations) :
        annotations_(annotations),
        suppress_print_(suppress_print)
    {}

    /**
       Called to print the help/usage message
    */
    int printUsage();


    /**
       Called to print the extended help for an option
     */
    int printExtHelp(const std::string& option);

    /**
       Add options to the Config object.  The options will be added in
       the order they are in the input array, and across calls to the
       function.
     */
    void addOption(
        struct option opt, const char* argname, const char* desc, std::vector<bool> annotations, OptionDefinition* def);

    /**
       Adds a heading to the usage output
     */
    void addHeading(const char* desc);

    /**
       Called to get the prelude for the help/usage message
     */
    virtual std::string getUsagePrelude();

    // Function that will be called at the end of parsing so that
    // error checking can be done
    virtual int checkArgsAfterParsing();

    // Enable support for everything after -- to be passed to a
    // callback. Each arg will be passed independently to the callback
    // function
    void enableDashDashSupport(std::function<int(const char* arg)> callback);

    // Add support for positional args.  Must be added in the order the
    // args show up on the command line
    void addPositionalCallback(std::function<int(int num, const char* arg)> callback);


    /**
       Get the name of the executable being run.  This is only
       avaialable after parseCmdLine() is called.
     */
    std::string getRunName() { return run_name_; }

    /** Set a configuration string to update configuration values */
    bool setOptionExternal(const std::string& entryName, const std::string& value);

    /** Get the value of an annotation for an option */
    bool getAnnotation(const std::string& entryName, char annotation);

public:
    // Function to uniformly parse boolean values for command line
    // arguments
    static bool parseBoolean(const std::string& arg, bool& success, const std::string& option);

    static uint32_t parseWallTimeToSeconds(const std::string& arg, bool& success, const std::string& option);

    virtual ~ConfigBase() {}

    /**
       Parse command-line arguments to update configuration values.

       @return Returns 0 if execution should continue.  Returns -1 if
         there was an error.  Returns 1 if run command line only asked
         for information to be print (e.g. --help or -V, for example).
     */
    int parseCmdLine(int argc, char* argv[], bool ignore_unknown = false);

    /**
       Check to see if an option was set on the command line

       @return True if option was set on command line, false
       otherwise.  Will also return false if option is unknown.
    */
    bool wasOptionSetOnCmdLine(const std::string& option);

private:
    std::vector<LongOption>                      options;
    std::map<char, int>                          short_options;
    std::string                                  short_options_string;
    size_t                                       longest_option  = 0;
    size_t                                       num_options     = 0;
    std::function<int(const char* arg)>          dashdash_callback;
    std::function<int(int num, const char* arg)> positional_args;

    // Map to hold extended help function calls
    std::map<std::string, std::function<std::string()>> extra_help_map;

    // Annotations
    std::vector<AnnotationInfo> annotations_;

    std::string run_name_;
    bool        suppress_print_;
    bool        has_extended_help_ = false;
};

} // namespace SST

#endif // SST_CORE_CONFIGBASE_H
