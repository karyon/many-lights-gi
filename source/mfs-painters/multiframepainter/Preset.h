#pragma once

#include <reflectionzeug/property/PropertyEnum.h>


enum class Preset : unsigned int
{
    CrytekSponza,
    SanMiguel,
    DabrovicSponza,
    Imrod,
    Jakobi,
    Megacity,
    Mitusba,
    Transparency,
    None
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
                { Preset::SanMiguel, "SanMiguel" },
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
