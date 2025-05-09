#pragma once
#include <JuceHeader.h>

namespace MetricTime
{
    struct MetricTimeElement
    {
        String name = {};
        double value = 0;
    };

    /*static*/ const enum class MetricTimeKey {
        SemibiscromaT,
        Semibiscroma,
        SemibiscromaD,

        BiscromaT,
        Biscroma,
        BiscromaD,
        
        SemicromaT,
        Semicroma,
        SemicromaD,
        
        CromaT,
        Croma,
        CromaD,
        
        SemiminimaT,
        Semiminima,
        SemiminimaD,
        
        MinimaT,
        Minima,
        MinimaD,
        
        SemibreveT,
        Semibreve,
        SemibreveD,
        
        BreveT,
        Breve,
        BreveD,

        LungaT,
        Lunga,
        LungaD,

        MassimaT,
        Massima,
        MassimaD
    };

    static /*const*/ std::map<int, MetricTimeElement> duration = {
        {(int)MetricTimeKey::SemibiscromaT , {"1/64t",  1.0 / 96.0}},
        {(int)MetricTimeKey::Semibiscroma  , {"1/64",   1.0 / 64.0}},
        {(int)MetricTimeKey::SemibiscromaD , {"1/64d", 3.0 / 128.0}},

        {(int)MetricTimeKey::BiscromaT     , {"1/32t",  1.0 / 48.0}},
        {(int)MetricTimeKey::Biscroma      , {"1/32",   1.0 / 32.0}},
        {(int)MetricTimeKey::BiscromaD     , {"1/32d",  3.0 / 64.0}},

        {(int)MetricTimeKey::SemicromaT    , {"1/16t",  1.0 / 24.0}},
        {(int)MetricTimeKey::Semicroma     , {"1/16",   1.0 / 16.0}},
        {(int)MetricTimeKey::SemicromaD    , {"1/16d",  3.0 / 32.0}},

        {(int)MetricTimeKey::CromaT        , {"1/8t ",  1.0 / 12.0}},
        {(int)MetricTimeKey::Croma         , {"1/8",    1.0 /  8.0}},
        {(int)MetricTimeKey::CromaD        , {"1/8d ",  3.0 / 16.0}},

        {(int)MetricTimeKey::SemiminimaT   , {"1/4t",    1.0 / 6.0}},
        {(int)MetricTimeKey::Semiminima    , {"1/4",     1.0 / 4.0}},
        {(int)MetricTimeKey::SemiminimaD   , {"1/4d",    3.0 / 8.0}},

        {(int)MetricTimeKey::MinimaT       , {"1/2t",    1.0 / 3.0}},
        {(int)MetricTimeKey::Minima        , {"1/2",     2.0 / 4.0}},
        {(int)MetricTimeKey::MinimaD       , {"1/2d",    3.0 / 4.0}},

        {(int)MetricTimeKey::SemibreveT    , {"1t",      2.0 / 3.0}},
        {(int)MetricTimeKey::Semibreve     , {"1",       4.0 / 4.0}},
        {(int)MetricTimeKey::SemibreveD    , {"1d",      6.0 / 4.0}},

        {(int)MetricTimeKey::BreveT        , {"2t",      4.0 / 3.0}},
        {(int)MetricTimeKey::Breve         , {"2",       8.0 / 4.0}},
        {(int)MetricTimeKey::BreveD        , {"2d",     12.0 / 4.0}},

        {(int)MetricTimeKey::LungaT        , {"4t",      8.0 / 3.0}},
        {(int)MetricTimeKey::Lunga         , {"4",      16.0 / 4.0}},
        {(int)MetricTimeKey::LungaD        , {"4d",     24.0 / 4.0}},

        {(int)MetricTimeKey::MassimaT      , {"8t",     16.0 / 3.0}},
        {(int)MetricTimeKey::Massima       , {"8",      32.0 / 4.0}},
        {(int)MetricTimeKey::MassimaD      , {"8d",     48.0 / 4.0}}
    };

    //StringArray* getTimeChoices()
    //{
    //    StringArray timeChoices;
    //    for (int i = 0; i < duration.size(); ++i)
    //        timeChoices.add(duration[i].name);
    //    return &timeChoices;
    //}

    const StringArray timeChoices = {
        "1/64t",
        "1/64",
        "1/64d",

        "1/32t",
        "1/32",
        "1/32d",

        "1/16t",
        "1/16",
        "1/16d",

        "1/8t ",
        "1/8",
        "1/8d ",

        "1/4t",
        "1/4",
        "1/4d",

        "1/2t",
        "1/2",
        "1/2d",

        "1t",
        "1",
        "1d",

        "2t",
        "2",
        "2d",

        "4t",
        "4",
        "4d",

        "8t",
        "8",
        "8d"
    };

}

