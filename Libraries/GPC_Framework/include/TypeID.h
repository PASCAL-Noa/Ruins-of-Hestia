#pragma once
#include <cstdint>

#define BIND_INDEXED_CLASS(class) TypeID GetTypeID() override { return GPC::GetTypeID<class>(); }

namespace GPC {

    using TypeID = uint64_t;

    template<typename T>
    static TypeID GetTypeID() {
        static TypeID ID = [] {
            static TypeID counter = 0;
            return counter++;
        }();
        return ID;
    }

    struct IndexedType {
        virtual ~IndexedType() = default;
        virtual TypeID GetTypeID() = 0;
    };

    struct ExampleClass : IndexedType {
        BIND_INDEXED_CLASS(ExampleClass);
    };

    struct ExampleChild : ExampleClass {
        BIND_INDEXED_CLASS(ExampleChild);
    };

} // GPC