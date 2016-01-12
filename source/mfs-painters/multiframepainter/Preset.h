#pragma once

#include <reflectionzeug/property/PropertyEnum.h>


enum class Preset : unsigned int
{
    CrytekSponza,
    DabrovicSponza,
    Imrod,
    Jakobi,
    Megacity,
    Mitusba,
    Transparency
};

namespace reflectionzeug
{

    template<>
    struct EnumDefaultStrings<Preset>
    {
        std::map<Preset, std::string> operator()()
        {
            return{
                { Preset::CrytekSponza, "CrytekSponza" },
                { Preset::DabrovicSponza, "DabrovicSponza" },
                { Preset::Imrod, "Imrod" },
                { Preset::Jakobi, "Jakobi" },
                { Preset::Megacity, "Megacity" },
                { Preset::Mitusba, "Mitusba" },
                { Preset::Transparency, "Transparency" },
            };
        }
    };

}
