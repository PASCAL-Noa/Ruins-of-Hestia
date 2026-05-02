#include <cstdint>
#include "CompileTimeHash.h"
#include "Assets.h"

#define BindResourceId(name) inline static ResourceID ResourceId = COMPILE_TIME_CRC32_STR(#name); ResourceID GetResourceId() override { return ResourceId; }

namespace GPC
{
    using ResourceID = uint32_t;

    enum class ResourceType : uint16_t
    {
        WOOD,   STONE,  METAL,  FIBER,                                    // Ressources avec Tier
        FOOD,   VILLAGER,                                                 // Impacte direct avec le village
        FRAGMENT_CARD,                                                    // Débloque des biomes (4 requis)
        PURPLE_GEMME,                                                       // Gemme universelle
        RED_GEMME,  BLUE_GEMME, GREEN_GEMME, YELLOW_GEMME,  ORANGE_GEMME, // Gemmes de couleur spécifique pour les stats
    };

    enum class ResourceTier : uint16_t
    {
        T1, T2, T3, T4, T5, NONE
    };

    struct Resource
    {
        virtual ~Resource() = default;

        void                    Add(uint32_t count);
        void                    Remove(uint32_t count);
        [[nodiscard]] bool      Has(uint32_t count) const;
        [[nodiscard]] uint32_t  GetCount() const;

        virtual std::string     GetTexture() = 0;
        virtual ResourceType    GetType() = 0;
        virtual ResourceID      GetResourceId() = 0;

    private:
        uint32_t                Count = 0;
        uint32_t                Max = 9999;
    };

    template<ResourceTier t>
    struct Wood : public Resource
    {
        static ResourceID ResourceId;
        ResourceID GetResourceId() override { return ResourceId; }

        constexpr std::string GetTexture() override
        {
            if constexpr (t == ResourceTier::T1)        return "Productivity_Wood";
            else if constexpr (t == ResourceTier::T2)   return "Productivity_Wood";
            else if constexpr (t == ResourceTier::T3)   return "Productivity_Wood";
            else if constexpr (t == ResourceTier::T4)   return "Productivity_Wood";
            else if constexpr (t == ResourceTier::T5)   return "Productivity_Wood";

            return "Productivity_Wood";
        };
        ResourceType GetType() override { return ResourceType::WOOD; }
    };
    template<> inline ResourceID Wood<ResourceTier::T1>::ResourceId = COMPILE_TIME_CRC32_STR("Wood_T1");
    template<> inline ResourceID Wood<ResourceTier::T2>::ResourceId = COMPILE_TIME_CRC32_STR("Wood_T2");
    template<> inline ResourceID Wood<ResourceTier::T3>::ResourceId = COMPILE_TIME_CRC32_STR("Wood_T3");
    template<> inline ResourceID Wood<ResourceTier::T4>::ResourceId = COMPILE_TIME_CRC32_STR("Wood_T4");
    template<> inline ResourceID Wood<ResourceTier::T5>::ResourceId = COMPILE_TIME_CRC32_STR("Wood_T5");


    template<ResourceTier t>
    struct Stone : public Resource
    {
        static ResourceID ResourceId;
        ResourceID GetResourceId() override { return ResourceId; }
        constexpr std::string GetTexture() override
        {
            if constexpr (t == ResourceTier::T1)        return "Productivity_Stone";
            else if constexpr (t == ResourceTier::T2)   return "Productivity_Stone";
            else if constexpr (t == ResourceTier::T3)   return "Productivity_Stone";
            else if constexpr (t == ResourceTier::T4)   return "Productivity_Stone";
            else if constexpr (t == ResourceTier::T5)   return "Productivity_Stone";

            return "Productivity_Stone";
        };
        ResourceType GetType() override { return ResourceType::STONE; }
    };
    template<> inline ResourceID Stone<ResourceTier::T1>::ResourceId = COMPILE_TIME_CRC32_STR("Stone_T1");
    template<> inline ResourceID Stone<ResourceTier::T2>::ResourceId = COMPILE_TIME_CRC32_STR("Stone_T2");
    template<> inline ResourceID Stone<ResourceTier::T3>::ResourceId = COMPILE_TIME_CRC32_STR("Stone_T3");
    template<> inline ResourceID Stone<ResourceTier::T4>::ResourceId = COMPILE_TIME_CRC32_STR("Stone_T4");
    template<> inline ResourceID Stone<ResourceTier::T5>::ResourceId = COMPILE_TIME_CRC32_STR("Stone_T5");


    template<ResourceTier t>
    struct Metal : public Resource
    {
        static ResourceID ResourceId;
        ResourceID GetResourceId() override { return ResourceId; }
        constexpr std::string GetTexture() override
        {
            if constexpr (t == ResourceTier::T1)        return "Productivity_Iron";
            else if constexpr (t == ResourceTier::T2)   return "Productivity_Iron";
            else if constexpr (t == ResourceTier::T3)   return "Productivity_Iron";
            else if constexpr (t == ResourceTier::T4)   return "Productivity_Iron";
            else if constexpr (t == ResourceTier::T5)   return "Productivity_Iron";

            return "Productivity_Iron";
        };
        ResourceType GetType() override { return ResourceType::METAL; }
    };
    template<> inline ResourceID Metal<ResourceTier::T1>::ResourceId = COMPILE_TIME_CRC32_STR("Metal_T1");
    template<> inline ResourceID Metal<ResourceTier::T2>::ResourceId = COMPILE_TIME_CRC32_STR("Metal_T2");
    template<> inline ResourceID Metal<ResourceTier::T3>::ResourceId = COMPILE_TIME_CRC32_STR("Metal_T3");
    template<> inline ResourceID Metal<ResourceTier::T4>::ResourceId = COMPILE_TIME_CRC32_STR("Metal_T4");
    template<> inline ResourceID Metal<ResourceTier::T5>::ResourceId = COMPILE_TIME_CRC32_STR("Metal_T5");


    template<ResourceTier t>
    struct Fiber : public Resource
    {
        static ResourceID ResourceId;
        ResourceID GetResourceId() override { return ResourceId; }
        constexpr std::string GetTexture() override
        {
            if constexpr (t == ResourceTier::T1)        return "Productivity_Fiber";
            else if constexpr (t == ResourceTier::T2)   return "Productivity_Fiber";
            else if constexpr (t == ResourceTier::T3)   return "Productivity_Fiber";
            else if constexpr (t == ResourceTier::T4)   return "Productivity_Fiber";
            else if constexpr (t == ResourceTier::T5)   return "Productivity_Fiber";

            return "Productivity_Fiber";
        };
        ResourceType GetType() override { return ResourceType::FIBER; }
    };
    template<> inline ResourceID Fiber<ResourceTier::T1>::ResourceId = COMPILE_TIME_CRC32_STR("Fiber_T1");
    template<> inline ResourceID Fiber<ResourceTier::T2>::ResourceId = COMPILE_TIME_CRC32_STR("Fiber_T2");
    template<> inline ResourceID Fiber<ResourceTier::T3>::ResourceId = COMPILE_TIME_CRC32_STR("Fiber_T3");
    template<> inline ResourceID Fiber<ResourceTier::T4>::ResourceId = COMPILE_TIME_CRC32_STR("Fiber_T4");
    template<> inline ResourceID Fiber<ResourceTier::T5>::ResourceId = COMPILE_TIME_CRC32_STR("Fiber_T5");


    struct Food : public Resource
    {
        BindResourceId(Food);
        std::string GetTexture() override { return "Productivity_Food"; }
        ResourceType GetType() override { return ResourceType::FOOD; }
    };

    struct Villager : public Resource
    {
        BindResourceId(Villager);
        std::string GetTexture() override { return "Villager"; }
        ResourceType GetType() override { return ResourceType::VILLAGER; }
    };

    struct FragmentCard : public Resource
    {
        BindResourceId(FragmentCard);
        std::string GetTexture() override { return "Fragment_Card"; }
        ResourceType GetType() override { return ResourceType::FRAGMENT_CARD; }
    };

    struct PurpleGemme : public Resource
    {
        BindResourceId(PurpleGemme);
        std::string GetTexture() override { return "Purple_Gemme"; }
        ResourceType GetType() override { return ResourceType::PURPLE_GEMME; }
    };

    struct RedGemme : public Resource
    {
        BindResourceId(RedGemme);
        std::string GetTexture() override { return "Red_Gemme"; }
        ResourceType GetType() override { return ResourceType::RED_GEMME; }
    };

    struct BlueGemme : public Resource
    {
        BindResourceId(BlueGemme);
        std::string GetTexture() override { return "Blue_Gemme"; }
        ResourceType GetType() override { return ResourceType::BLUE_GEMME; }
    };

    struct GreenGemme : public Resource
    {
        BindResourceId(GreenGemme);
        std::string GetTexture() override { return "Green_Gemme"; }
        ResourceType GetType() override { return ResourceType::GREEN_GEMME; }
    };

    struct YellowGemme : public Resource
    {
        BindResourceId(YellowGemme);
        std::string GetTexture() override { return "Yellow_Gemme"; }
        ResourceType GetType() override { return ResourceType::YELLOW_GEMME; }
    };

    struct OrangeGemme : public Resource
    {
        BindResourceId(OrangeGemme);
        std::string GetTexture() override { return "Orange_Gemme"; }
        ResourceType GetType() override { return ResourceType::ORANGE_GEMME; }
    };
}
