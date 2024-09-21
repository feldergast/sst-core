// Copyright 2009-2024 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2024, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#ifndef SST_CORE_SERIALIZATION_OBJECTMAP_H
#define SST_CORE_SERIALIZATION_OBJECTMAP_H

#include "sst/core/from_string.h"
#include "sst/core/warnmacros.h"

// REMOVE ME
#include "sst/core/output.h"

#include <string>
#include <vector>

namespace SST {
namespace Core {
namespace Serialization {

class ObjectMap;

// Metadata object that each ObjectMap has a pointer to in order to
// track the hierarchy information while traversing the data
// structures.  This is used because a given object can be pointed to
// by multiple other objects, so we need to track the "path" through
// which we got to the object so we can traverse back up the object
// hierarchy.
struct ObjectMapMetaData
{
    /**
       Parent object that contained this object and through which it
       was selected.
    */
    ObjectMap* parent;

    /**
       Name of this object in the context of the currently set parent
     */
    std::string name;

    /**
       Constructor for intializing data memebers
     */
    ObjectMapMetaData(ObjectMap* parent, const std::string& name):
        parent(parent),
        name(name)
    {}
};

/**
   Class created by the serializer mapping mode used to map the
   variables for objects.  This allows access to read and write the
   mapped variables.  The base class is used for non-fundamental and
   non-container types, but there is a templated child class used for
   fundameentals and containers.  The templating is needed so that the
   type information can be embedded in the code for reading and
   writing.
 */
class ObjectMap
{
protected:

    // Static empty variable vector for use by versions that don't
    // have variables (i.e. are fundamentals or classes treated as
    // fundamentals.  This is needed because getVariables() returns a
    // reference to the vector.
    static std::vector<std::pair<std::string,ObjectMap*>> emptyVars;
    
    // ObjectMap(const std::string& name, void* addr) : name_(name), addr_(addr) {}

    /**
       Metedata object for walking the object hierarchy.  When this
       object is selected by a parent object, a metadata object will
       be added.  The metadata contains a pointer to the parent and
       the name of this object in the context of the parent. If this
       object is selected and the metadata is non a nullptr, then we
       have hit a loop in the data structure.

       Under normal circumstances, the metadata allows you to get the
       full path of the object according to how you walked the
       hierarchy, and allows you to return to the parent object.  If a
       loop is detected on select, then the full path of the object
       will return to the highest level path and the metadata from
       that path to the current path will be erased.
     */
    ObjectMapMetaData* mdata = nullptr;


    /**
       Indicates wheter or not the variable is read-only
     */
    bool read_only = false;
    
    /**
       Parent of this ObjectMap. If parent is nullptr, then this is
       the top level of hierarchy
     */
    // ObjectMap* parent_ = nullptr;

    /**
       Name of the variable as given to the serializer.  This uses the
       SST_SER and SST_SER_AS_PTR macros which pass both the variable
       and the stringized name to the serializer.
     */
    // std::string name_ = "";

    /**
       Address of the variable for reading and writing
     */
    // void* addr_ = nullptr;

    /**
       Type of the variable as given by the demangled version of
       typeif<T>.name() for the type.
     */
    // std::string type_ = "";

    /**
       Vector of variables contained within this ObjectMap.  This is
       empty for fundamentals and containers
     */
    // std::vector<ObjectMap*> variables_;


    /**
       Select the specified ObjectMap as a child variable with the
       given name.  It is the caller's responsibility to ensure that
       they are only calling this on an ObjectMap returned from the
       current ObjectMap's getVariables() call.  Otherwise, the
       behavior is unspecified. This function is primarily used to
       recurse the hierarchy for the print() function.

       @return ObjectMap* that is pased in if no loopback is detected,
       nullptr otherwise.

     */
    ObjectMap* selectVariable(const std::string& name, ObjectMap* var)
    {
        // Found the variable, make sure we haven't created a
        // loop
        if ( var->mdata ) {
            // Loop detected
            return nullptr;
        }
        // No loop, set the metadata and return it
        var->mdata = new ObjectMapMetaData(this,name);
        return var;
    }

    /**
       Function implemented by derived classes to implement set(). No
       need to check for read_only, that is done in set().
    */
    virtual void set_impl(const std::string& UNUSED(value)) {}

    
public:
    /**
       Default constructor primarily used for the "top" object in the hierarchy
     */
    // ObjectMap() : name_("<top>") {}
    ObjectMap() {}


    inline bool isReadOnly() { return read_only; }
    inline void setReadOnly() { read_only = true; }
    
    /**
       Constructor for ObjectMap

       @param name Name of the variable
       @param addr Address of the variable
       @param type Type of the variable as returned by typeid<T>.name()
     */
    // ObjectMap(const std::string& name, void* addr, const std::string& type) :
    //     name_(name),
    //     addr_(addr),
    //     type_(demangle_name(type.c_str()))
    // {}

    /**
       Get the name of the variable represented by this ObjectMap

       @return Name of variable
    */
    std::string getName()
    {
        // printf("name = %s\n", mdata->name.c_str());
        // printf("parent = %p\n", mdata->parent);
        if ( mdata ) {
            return mdata->name;
        }
        return "";
    }

    /**
       Get the full hierarchical name of the variable represented by
       this ObjectMap, based on the path taken to get to this object.

       @return Full hierarchical name of variable
    */
    std::string getFullName()
    {
        if ( !mdata ) return "";

        std::string fullname = mdata->name;
        std::string slash("/");
        // path = slash + path;
        ObjectMapMetaData* curr = mdata->parent->mdata;
        while ( curr != nullptr ) {
            fullname = curr->name + slash + fullname;
            curr = curr->parent->mdata;
        }
        return fullname;
    }

    /**
       Get the type of the variable represented by the ObjectMap

       @return Type of variable
     */
    virtual std::string getType() = 0;

    /**
       Get the address of the variable represented by the ObjectMap

       @return Address of varaible
     */
    virtual void* getAddr() = 0;


    /**
       Get the list of child variables contained in this ObjectMap

       @return Refernce to vector containing ObjectMaps for this
       ObjectMap's child variables. Fundamental types will return the
       same empty vector.
     */
    virtual const std::vector<std::pair<std::string,ObjectMap*>>& getVariables() { return emptyVars; }



    /************ Functions for walking the Object Hierarchy ************/

    /**
       Get the parent for this ObjectMap

       @return Parent for this ObjectMap
     */
    ObjectMap* selectParent()
    {
        ObjectMap* ret = mdata->parent;
        if ( nullptr == ret ) {
            printf("No metadata found in selectParent()\n");
            return nullptr;
        }
        mdata = nullptr;
        return ret;
    }

    /**
       Get the ObjectMap for the specified variable

       @return ObjectMap for specified variable, if it exists, this
       otherwise
    */
    ObjectMap* selectVariable(std::string name)
    {
        ObjectMap* var = findVariable(name);

        // If we get nullptr back, then it didn't exist.  Just return this
        if ( nullptr == var ) return this;

        // See if this creates a loop
        
        if ( var->mdata ) {
            printf("Found a loop\n");
            // Loop detected.  We will need to remove all the
            // mdata elements from this object back up from
            // parent to parent until we get to the object we
            // are selecting.  This will reset the hierachy to
            // the more shallow one.
            ObjectMap* current = this;
            ObjectMap* parent = mdata->parent;
            delete current->mdata;
            current->mdata = nullptr;
            while ( parent != var ) {
                // TODO: check for parent == nullptr, which
                // would be the case where we didn't detect
                // the loop going back up the chain. This
                // would mean the metadata was corrupted
                // somehow.
                current = parent;
                parent = current->mdata->parent;
                // Clear the metadata for current
                delete current->mdata;
                current->mdata = nullptr;
            }
            return var;
        }

        // No loop, set the metadata and return it
        var->mdata = new ObjectMapMetaData(this, name);
        return var;
    }


    /**
       Adds a varaible to this ObjectMap

       @param obj ObjectMap to add as a variable
     */
    virtual void addVariable(const std::string& UNUSED(name), ObjectMap* UNUSED(obj)) {}

    // /**
    //    Set the parent for this ObjectMap

    //    @param obj ObjectMap to set as parent
    //  */
    // void addParent(ObjectMap* obj) { parent_ = obj; }


    /************ Functions for getting/setting Object's Value *************/

    /**
       Get the value of the variable as a string.  NOTE: this function
       is only valid for ObjectMaps that represent fundamental types
       or classes treated as fundamental types.

       @return Value of the represented variable as a string
     */
    virtual std::string get() { return ""; }

    /**
       Sets the value of the variable represented by the ObjectMap to
       the specified value, which is represented as a string.  The
       templated child classes for fundamentals will know how to
       convert the string to a value of the approproprite type.  NOTE:
       this fucntion is only value for ObjectMaps that represent
       fundamental types or classes treated as fundamentatl types.
     */
    void set(const std::string& value)
    {
        if ( read_only ) return;
        else set_impl(value);
    }

    /**
       Gets the value of the specified variable as a string.  NOTE:
       this function is only valid for ObjectMaps that represent
       non-fundamental types or classes not treated as fundamental
       types.

       @return Value of the specified variable as a string
     */
    virtual std::string get(const std::string& var)
    {
        ObjectMap* obj = selectVariable(var);
        if ( nullptr == obj ) return "";
        std::string ret = obj->get();
        obj->selectParent();
        return ret;
    }

    /**
       Sets the value of the specified variable to the specified
       value, which is represented as a string.  The templated child
       classes for fundamentals will know how to convert the string to
       a value of the approproprite type.  NOTE: this fucntion is only
       valuid for ObjectMaps that represent non-fundamental types or
       classes not treated as fundamentatl types (i.e. they must have
       childrent).       
    */
    virtual void set(const std::string& var, const std::string& value, bool& found, bool& read_only)
    {
        ObjectMap* obj = selectVariable(var);
        if ( nullptr == obj ) {
            found = false;
            return;
        }
        found = true;
        if ( obj->isReadOnly() ) {
            read_only = true;
            return;
        }
        read_only = false;
        obj->set(value);
        obj->selectParent();
    }

    // /**
    //    Get the number of variables in the ObjectMap

    //    @return Number of variables added to the ObjectMap
    //  */
    // virtual size_t num_vars() { return variables_.size(); }

    /**
       Check to see if this ObjectMap represents a fundamental or a
       class treated as a fundamental.

       @return true if this ObjectMap represents a fundamental or
       class treated as a fundamental, false otherwise
     */
    virtual bool isFundamental() { return false; }

    /**
       Check to see if this ObjectMap represents a container

       @return true if this ObjectMap represents a container, false
       otherwise
     */
    virtual bool isContainer() { return false; }



    virtual ~ObjectMap()
    {
        // for ( auto* x : variables_ )
        //     delete x;
        // variables_.clear();
    }

    /**
       Static function to demangle type names returned from typeid<T>.name()

       @return demangled name
     */
    static std::string demangle_name(const char* name);

    /**
       Print information for the specified variable.

       @param name name of variable to print
       @param recurse number of levels to recurse (default is 0)

       @return true if variable is found, false otherwise
     */
    virtual bool printVariable(std::string name, int recurse = 0)
    {
        ObjectMap* var = findVariable(name);
        if ( nullptr == var ) return false;

        // Check to see if this is a loopback
        if ( nullptr != var->mdata ) {
            // Found a loop
            printf("%s (%s) = <loopback>\n", name.c_str(), var->getType().c_str());
            return true;
        }
        var->activate(this, name);
        var->printRecursive(name, 0, recurse);
        var->deactivate();
        return true;
    }

    /**
       Print the variable information.  The name of the variable must
       be passed in so that we don't need to use the metadata, which
       is used to keep track of the path selected through the object
       hierarchy.
     */
    virtual void print(int recurse = 0)
    {
        printRecursive(mdata->name, 0, recurse);
    }

    // virtual void print(std::string name = "", int level = 0, bool recurse = true)
    // {
    //     if ( mdata ) name = mdata->name;
    //     std::string indent = std::string(level, ' ');
    //     if ( isFundamental() ) {
    //         printf("%s%s = %s (%s)\n", indent.c_str(), name.c_str(), get().c_str(), getType().c_str());
    //         return;
    //     }

    //     // if ( isContainer() ) {
    //     //     printf("%s%s (%s)\n", indent.c_str(), name_.c_str(), type_.c_str());
    //     //     return;
    //     // }

    //     printf("%s%s (%s)\n", indent.c_str(), name.c_str(), getType().c_str());

    //     if ( recurse ) {
    //         for ( auto& x : getVariables() ) {
    //             // ObjectMap* sel = selectVariable(x.first, x.second);
    //             // bool loop = ( nullptr != sel->mdata );
    //             bool loop = ( nullptr != x.second->mdata );
    //             if ( loop ) {
    //                 printf("%s %s (%s) = <loopback>\n", indent.c_str(), x.first.c_str(), x.second->getType().c_str());
    //             }
    //             else {
    //                 x.second->print(x.first, level + 1, true);
    //                 // x.second->selectParent();
    //             }
    //         }
    //     }
    // }

private:
    
    inline void activate(ObjectMap* parent, const std::string& name)
    {
        mdata = new ObjectMapMetaData(parent,name);        
    }

    inline void deactivate()
    {
        delete mdata;
        mdata = nullptr;
    }
    
    inline ObjectMap* findVariable(const std::string& name)
    {
        const std::vector<std::pair<std::string,ObjectMap*>>& variables = getVariables();
        for ( auto& x : variables ) {
            if ( x.first == name ) {
                return x.second;
            }
        }
        return nullptr;
    }

    void printRecursive(const std::string& name, int level, int recurse)
    {
        std::string indent = std::string(level, ' ');
        if ( isFundamental() ) {
            printf("%s%s = %s (%s)\n", indent.c_str(), name.c_str(), get().c_str(), getType().c_str());
            return;
        }

        printf("%s%s (%s)\n", indent.c_str(), name.c_str(), getType().c_str());

        if ( level <= recurse ) {
            for ( auto& x : getVariables() ) {
                bool loop = ( nullptr != x.second->mdata );
                if ( loop ) {
                    printf("%s %s (%s) = <loopback>\n", indent.c_str(), x.first.c_str(), x.second->getType().c_str());
                }
                else {
                    x.second->activate(this, name);
                    x.second->printRecursive(x.first, level + 1, recurse);
                    x.second->deactivate();
                }
            }
        }
        
    }

};


/**
   ObjectMap object for non-fundamental, non-container types.  This
   class allows for child variables.
 */
class ObjectMapWithChildren : public ObjectMap
{
protected:

    std::vector<std::pair<std::string,ObjectMap*>> variables_;

    ObjectMapWithChildren() : ObjectMap() {}
    
public:

    /**
       Adds a variable to this ObjectMap

       @param obj ObjectMap to add as a variable
     */
    void addVariable(const std::string& name, ObjectMap* obj) override { variables_.push_back(std::make_pair(name,obj)); }


    /**
       Get the list of child variables contained in this ObjectMap

       @return Refernce to vector containing ObjectMaps for this
       ObjectMap's child variables. pair.first is the name of the
       variable in the context of this object.
     */
    const std::vector<std::pair<std::string,ObjectMap*>>& getVariables() override { return variables_; }    

};


/**
   ObjectMap object to create a level of hierarchy that doesn't
   represent a specific object.  This can be used to create views of
   data that don't align specifically with the underlying data
   structures.
 */
class ObjectMapHierarchyOnly : public ObjectMapWithChildren
{
public:    

    ObjectMapHierarchyOnly() :
        ObjectMapWithChildren()
    {}
    
    /**
       Returns empty string since there is no underlying object being
       represented

       @return empty string
     */
    std::string getType() override { return ""; }

    /**
       Returns nullptr since there is no underlying object being
       represented

       @return nullptr
     */
    void* getAddr() override { return nullptr; }

};


/**
   ObjectMap object for non-fundamental, non-container types.  This
   class allows for child variables.
 */
class ObjectMapClass : public ObjectMapWithChildren
{
protected:

    /**
       Type of the variable as given by the demangled version of
       typeif<T>.name() for the type.
     */
    std::string type_;

    /**
       Address of the variable for reading and writing
     */
    void* addr_ = nullptr;
    
public:    

    ObjectMapClass() :
        ObjectMapWithChildren()
    {}
    
    ObjectMapClass(void* addr, const std::string& type) :
        ObjectMapWithChildren(),
        type_(demangle_name(type.c_str())),
        addr_(addr)
    {}

    /**
       Get the type of the represented object

       @return type of represented object
     */
    std::string getType() override { return type_; }

    /**
       Get the address of the represented object

       @return address of represented object
     */
    void* getAddr() override { return addr_; }

};


/**
   ObjectMap object fundamental types, and classes treated as
   fundamental types.  In order for an object to be treated as a
   fundamental, it must be printable using std::to_string() and
   assignable using SST::Core::from_string().
*/
template <typename T>
class ObjectMapFundamental : public ObjectMap
{
protected:

    /**
       Address of the variable for reading and writing
     */
    T* addr_ = nullptr;
    
public:
    virtual std::string get() override { return std::to_string(*addr_); }
    virtual void set_impl(const std::string& value) override
    {
        *addr_ = SST::Core::from_string<T>(value);
    }

    bool isFundamental() override { return true; }

    /**
       Get the address of the variable represented by the ObjectMap

       @return Address of varaible
     */
    void* getAddr() override { return addr_; }

    
    // ObjectMapFundamental(const std::string& name, void* addr) :
    //     ObjectMap(name, addr)
    // {
    //     type_ = demangle_name(typeid(T).name());
    // }

    /**
       Get the list of child variables contained in this ObjectMap,
       which in this case will be empty.

       @return Refernce to vector containing ObjectMaps for this
       ObjectMap's child variables. This vector will be empty because
       fundamentals have no children
     */
    const std::vector<std::pair<std::string,ObjectMap*>>& getVariables() override { return emptyVars; }

    ObjectMapFundamental(T* addr) :
        ObjectMap(),
        addr_(addr)
    {}

    std::string getType() override
    {
        return demangle_name(typeid(T).name());
    }
};


// /**
//    Class to walk the ObjectMap hierarchy.
//  */
// class ObjectMapWalker
// {
// public:
//     ObjectMapWalker(ObjectMap* top);
    
//     selectVariable();
//     selectParent();

//     set();
//     get();
    
// };


} // namespace Serialization
} // namespace Core
} // namespace SST

#endif // SST_CORE_SERIALIZATION_OBJECTMAP_H
