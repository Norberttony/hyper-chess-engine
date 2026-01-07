#include "magic-bitboards.h"

// to prevent wrapping around bitboard
static const U64 not_8_rank = 18446744073709551360ULL;
static const U64 not_1_rank =    72057594037927935ULL;
static const U64 not_a_file = 18374403900871474942ULL;
static const U64 not_h_file =  9187201950435737471ULL;

U64 rookAttacks[102400];
U64 bishopAttacks[5248];

MagicSqEntry rookEntries[64] =
{
    //      MAGICS                      MASKS                  REFS        MASK BIT COUNTS
    { 36028909233389568ULL,     282578800148862ULL,     rookAttacks + 0,        12 },
    { 2323857682869526540ULL,   565157600297596ULL,     rookAttacks + 4096,     11 },
    { 612498895439667201ULL,    1130315200595066ULL,    rookAttacks + 6144,     11 },
    { 4647719282254292992ULL,   2260630401190006ULL,    rookAttacks + 8192,     11 },
    { 1297045497367777428ULL,   4521260802379886ULL,    rookAttacks + 10240,    11 },
    { 1585278064089387009ULL,   9042521604759646ULL,    rookAttacks + 12288,    11 },
    { 36058483866469248ULL,     18085043209519166ULL,   rookAttacks + 14336,    11 },
    { 216173401763881216ULL,    36170086419038334ULL,   rookAttacks + 16384,    12 },
    { 72620690028642560ULL,     282578800180736ULL,     rookAttacks + 20480,    11 },
    { 1189020671712174080ULL,   565157600328704ULL,     rookAttacks + 22528,    10 },
    { 4611967768569315585ULL,   1130315200625152ULL,    rookAttacks + 23552,    10 },
    { 5066585316540448ULL,      2260630401218048ULL,    rookAttacks + 24576,    10 },
    { 622904157820686592ULL,    4521260802403840ULL,    rookAttacks + 25600,    10 },
    { 562984389706240ULL,       9042521604775424ULL,    rookAttacks + 26624,    10 },
    { 562967154393096ULL,       18085043209518592ULL,   rookAttacks + 27648,    10 },
    { 81627760501792834ULL,     36170086419037696ULL,   rookAttacks + 28672,    11 },
    { 2893601793260912768ULL,   282578808340736ULL,     rookAttacks + 30720,    11 },
    { 459367436873904132ULL,    565157608292864ULL,     rookAttacks + 32768,    10 },
    { 577941794751258632ULL,    1130315208328192ULL,    rookAttacks + 33792,    10 },
    { 282574756782089ULL,       2260630408398848ULL,    rookAttacks + 34816,    10 },
    { 282574757298180ULL,       4521260808540160ULL,    rookAttacks + 35840,    10 },
    { 6154451465324200960ULL,   9042521608822784ULL,    rookAttacks + 36864,    10 },
    { 76635960595450122ULL,     18085043209388032ULL,   rookAttacks + 37888,    10 },
    { 2990673826582442052ULL,   36170086418907136ULL,   rookAttacks + 38912,    11 },
    { 576742785628111428ULL,    282580897300736ULL,     rookAttacks + 40960,    11 },
    { 4625197144801026048ULL,   565159647117824ULL,     rookAttacks + 43008,    10 },
    { 9008303063453760ULL,      1130317180306432ULL,    rookAttacks + 44032,    10 },
    { 1301821943382560768ULL,   2260632246683648ULL,    rookAttacks + 45056,    10 },
    { 112665861282609152ULL,    4521262379438080ULL,    rookAttacks + 46080,    10 },
    { 10415171139273728ULL,     9042522644946944ULL,    rookAttacks + 47104,    10 },
    { 4505815831022000ULL,      18085043175964672ULL,   rookAttacks + 48128,    10 },
    { 180148477631677569ULL,    36170086385483776ULL,   rookAttacks + 49152,    11 },
    { 2307039321628082816ULL,   283115671060736ULL,     rookAttacks + 51200,    11 },
    { 40532534089498632ULL,     565681586307584ULL,     rookAttacks + 53248,    10 },
    { 2460110125604552960ULL,   1130822006735872ULL,    rookAttacks + 54272,    10 },
    { 4505974794620960ULL,      2261102847592448ULL,    rookAttacks + 55296,    10 },
    { 288512022943892496ULL,    4521664529305600ULL,    rookAttacks + 56320,    10 },
    { 2342435323692191816ULL,   9042787892731904ULL,    rookAttacks + 57344,    10 },
    { 730146093915969572ULL,    18085034619584512ULL,   rookAttacks + 58368,    10 },
    { 144115789941702849ULL,    36170077829103616ULL,   rookAttacks + 59392,    11 },
    { 3566851042320596992ULL,   420017753620736ULL,     rookAttacks + 61440,    11 },
    { 563504008527904ULL,       699298018886144ULL,     rookAttacks + 63488,    10 },
    { 2307250693871829008ULL,   1260057572672512ULL,    rookAttacks + 64512,    10 },
    { 3458835028594786328ULL,   2381576680245248ULL,    rookAttacks + 65536,    10 },
    { 562968744558608ULL,       4624614895390720ULL,    rookAttacks + 66560,    10 },
    { 1443966664953692208ULL,   9110691325681664ULL,    rookAttacks + 67584,    10 },
    { 6052909436295708690ULL,   18082844186263552ULL,   rookAttacks + 68608,    10 },
    { 155374775576166404ULL,    36167887395782656ULL,   rookAttacks + 69632,    11 },
    { 4756434564400809344ULL,   35466950888980736ULL,   rookAttacks + 71680,    11 },
    { 1152974835219958016ULL,   34905104758997504ULL,   rookAttacks + 73728,    10 },
    { 2305913936308871680ULL,   34344362452452352ULL,   rookAttacks + 74752,    10 },
    { 72075186676965632ULL,     33222877839362048ULL,   rookAttacks + 75776,    10 },
    { 2252899595326720ULL,      30979908613181440ULL,   rookAttacks + 76800,    10 },
    { 2306124656056206080ULL,   26493970160820224ULL,   rookAttacks + 77824,    10 },
    { 5189290267253015552ULL,   17522093256097792ULL,   rookAttacks + 78848,    10 },
    { 2305931292291891712ULL,   35607136465616896ULL,   rookAttacks + 79872,    11 },
    { 27866168760996994ULL,     9079539427579068672ULL, rookAttacks + 81920,    12 },
    { 324295457327022725ULL,    8935706818303361536ULL, rookAttacks + 86016,    11 },
    { 37243894823534601ULL,     8792156787827803136ULL, rookAttacks + 88064,    11 },
    { 282093454100533ULL,       8505056726876686336ULL, rookAttacks + 90112,    11 },
    { 2882867055202410498ULL,   7930856604974452736ULL, rookAttacks + 92160,    11 },
    { 90353484705563265ULL,     6782456361169985536ULL, rookAttacks + 94208,    11 },
    { 9095168783810820ULL,      4485655873561051136ULL, rookAttacks + 96256,    11 },
    { 5070948516642946ULL,      9115426935197958144ULL, rookAttacks + 98304,    12 }
};

MagicSqEntry bishopEntries[64] =
{
    //      MAGICS                      MASKS                  REFS        MASK BIT COUNTS
    { 18300276100368641ULL,     18049651735527936ULL,   bishopAttacks + 0,      6 },
    { 565166173455588ULL,       70506452091904ULL,      bishopAttacks + 64,     5 },
    { 4508033648559107ULL,      275415828992ULL,        bishopAttacks + 96,     5 },
    { 2278501628510226ULL,      1075975168ULL,          bishopAttacks + 128,    5 },
    { 2306972242082269216ULL,   38021120ULL,            bishopAttacks + 160,    5 },
    { 324544088684644368ULL,    8657588224ULL,          bishopAttacks + 192,    5 },
    { 1179780407885893ULL,      2216338399232ULL,       bishopAttacks + 224,    5 },
    { 792915560292690953ULL,    567382630219776ULL,     bishopAttacks + 256,    6 },
    { 1478989337199120ULL,      9024825867763712ULL,    bishopAttacks + 320,    5 },
    { 1152983085982286880ULL,   18049651735527424ULL,   bishopAttacks + 352,    5 },
    { 5197173795559505930ULL,   70506452221952ULL,      bishopAttacks + 384,    5 },
    { 285890480178707ULL,       275449643008ULL,        bishopAttacks + 416,    5 },
    { 36592916277298688ULL,     9733406720ULL,          bishopAttacks + 448,    5 },
    { 72059827756793856ULL,     2216342585344ULL,       bishopAttacks + 480,    5 },
    { 9572915435737100ULL,      567382630203392ULL,     bishopAttacks + 512,    5 },
    { 144151476808392720ULL,    1134765260406784ULL,    bishopAttacks + 544,    5 },
    { 2255373805822476ULL,      4512412933816832ULL,    bishopAttacks + 576,    5 },
    { 2891593561634242690ULL,   9024825867633664ULL,    bishopAttacks + 608,    5 },
    { 1154048512950800896ULL,   18049651768822272ULL,   bishopAttacks + 640,    7 },
    { 1744018990094360576ULL,   70515108615168ULL,      bishopAttacks + 768,    7 },
    { 563568731754704ULL,       2491752130560ULL,       bishopAttacks + 896,    7 },
    { 151433541773820160ULL,    567383701868544ULL,     bishopAttacks + 1024,   7 },
    { 54614943051417616ULL,     1134765256220672ULL,    bishopAttacks + 1152,   5 },
    { 396598319650570888ULL,    2269530512441344ULL,    bishopAttacks + 1184,   5 },
    { 117181551930120192ULL,    2256206450263040ULL,    bishopAttacks + 1216,   5 },
    { 18331058001412352ULL,     4512412900526080ULL,    bishopAttacks + 1248,   5 },
    { 72136758942770180ULL,     9024834391117824ULL,    bishopAttacks + 1280,   7 },
    { 4684869787719892995ULL,   18051867805491712ULL,   bishopAttacks + 1408,   9 },
    { 281682225676290ULL,       637888545440768ULL,     bishopAttacks + 1920,   9 },
    { 18089440211701768ULL,     1135039602493440ULL,    bishopAttacks + 2432,   7 },
    { 4613938093664502912ULL,   2269529440784384ULL,    bishopAttacks + 2560,   5 },
    { 1127549325296914ULL,      4539058881568768ULL,    bishopAttacks + 2592,   5 },
    { 626029691423173632ULL,    1128098963916800ULL,    bishopAttacks + 2624,   5 },
    { 1299290717290043401ULL,   2256197927833600ULL,    bishopAttacks + 2656,   5 },
    { 1298162902901195776ULL,   4514594912477184ULL,    bishopAttacks + 2688,   7 },
    { 4756083815360036928ULL,   9592139778506752ULL,    bishopAttacks + 2816,   9 },
    { 37163501611458568ULL,     19184279556981248ULL,   bishopAttacks + 3328,   9 },
    { 4507998211346432ULL,      2339762086609920ULL,    bishopAttacks + 3840,   7 },
    { 291054488947655168ULL,    4538784537380864ULL,    bishopAttacks + 3968,   5 },
    { 39481281246938368ULL,     9077569074761728ULL,    bishopAttacks + 4000,   5 },
    { 3459063624219320360ULL,   562958610993152ULL,     bishopAttacks + 4032,   5 },
    { 1158059557341108244ULL,   1125917221986304ULL,    bishopAttacks + 4064,   5 },
    { 54606696856683532ULL,     2814792987328512ULL,    bishopAttacks + 4096,   7 },
    { 4611752272727113856ULL,   5629586008178688ULL,    bishopAttacks + 4224,   7 },
    { 567361086163456ULL,       11259172008099840ULL,   bishopAttacks + 4352,   7 },
    { 18016082145313344ULL,     22518341868716544ULL,   bishopAttacks + 4480,   7 },
    { 18581750837937152ULL,     9007336962655232ULL,    bishopAttacks + 4608,   5 },
    { 1155173858475508224ULL,   18014673925310464ULL,   bishopAttacks + 4640,   5 },
    { 297809338721108001ULL,    2216338399232ULL,       bishopAttacks + 4672,   5 },
    { 564066754101248ULL,       4432676798464ULL,       bishopAttacks + 4704,   5 },
    { 9113852974206992ULL,      11064376819712ULL,      bishopAttacks + 4736,   5 },
    { 432436000166381584ULL,    22137335185408ULL,      bishopAttacks + 4768,   5 },
    { 360296843625759780ULL,    44272556441600ULL,      bishopAttacks + 4800,   5 },
    { 49066847310338ULL,        87995357200384ULL,      bishopAttacks + 4832,   5 },
    { 4612816625653254144ULL,   35253226045952ULL,      bishopAttacks + 4864,   5 },
    { 319773170108211328ULL,    70506452091904ULL,      bishopAttacks + 4896,   5 },
    { 1876030995998969875ULL,   567382630219776ULL,     bishopAttacks + 4928,   6 },
    { 146435998722ULL,          1134765260406784ULL,    bishopAttacks + 4992,   5 },
    { 554199040ULL,             2832480465846272ULL,    bishopAttacks + 5024,   5 },
    { 39582422864000ULL,        5667157807464448ULL,    bishopAttacks + 5056,   5 },
    { 4467438134272ULL,         11333774449049600ULL,   bishopAttacks + 5088,   5 },
    { 5764607587526377984ULL,   22526811443298304ULL,   bishopAttacks + 5120,   5 },
    { 4611986253856374868ULL,   9024825867763712ULL,    bishopAttacks + 5152,   5 },
    { 2323859625016950804ULL,   18049651735527936ULL,   bishopAttacks + 5184,   6 }
};

void initMagicBitboards(int isBishop)
{
    // initialize magic bitboards
    for (int s = 0; s < 64; s++)
    {
        MagicSqEntry *entry = isBishop ? bishopEntries + s : rookEntries + s;
        U64 maxDefenders = entry->mask;
        U64 number = entry->magic;

        // get all of the bit indexes on defender mask
        int bitIndexes[20] = { 0 };
        int bits = 0;
        while (maxDefenders)
        {
            int index = pop_lsb(maxDefenders);
            bitIndexes[bits++] = index;
            maxDefenders ^= 1ULL << index;
        }

        for (int c = 0; c < (1 << bits); c++)
        {
            // generate test mask given bit indexes
            U64 testMask = 0ULL;
            for (int b = 0; b < bits; b++)
            {
                testMask |= (1ULL << bitIndexes[b]) * ((c & (1ULL << b)) > 0);
            }

            // use the testMask to test the magic number
            int testIndex = (int)((testMask * number) >> (64 - bits));
            U64 attacks = isBishop ? genBishopAttacks(s, testMask) : genRookAttacks(s, testMask);

            *(entry->ref + testIndex) = attacks;
        }
    }
}

U64 genRookAttacks(int sq, U64 blockers)
{
    U64 attacks = 0;
    U64 iterSq = 1ULL << sq;

    // keep looping while no blocker and not about to wrap around the board
    // to the right
    while (!(blockers & attacks) && iterSq & not_h_file)
    {
        iterSq <<= 1;
        attacks |= iterSq;
    }
    // so that all future loops do not think they are immediately attacking a piece
    blockers &= ~attacks;

    // to the left
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_a_file)
    {
        iterSq >>= 1;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    // up
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_8_rank)
    {
        iterSq >>= 8;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    // down
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_1_rank)
    {
        iterSq <<= 8;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    return attacks;
}

U64 genBishopAttacks(int sq, U64 blockers)
{
    U64 attacks = 0;
    U64 iterSq = 1ULL << sq;

    // keep looping while no blocker and not about to wrap around the board
    // down and right
    while (!(blockers & attacks) && iterSq & not_h_file && iterSq & not_1_rank)
    {
        iterSq <<= 9;
        attacks |= iterSq;
    }
    // so that all future loops do not think they are immediately attacking a piece
    blockers &= ~attacks;

    // down and left
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_a_file && iterSq & not_1_rank)
    {
        iterSq <<= 7;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    // up and right
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_8_rank && iterSq & not_h_file)
    {
        iterSq >>= 7;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    // up and left
    iterSq = 1ULL << sq;
    while (!(blockers & attacks) && iterSq & not_8_rank && iterSq & not_a_file)
    {
        iterSq >>= 9;
        attacks |= iterSq;
    }
    blockers &= ~attacks;

    return attacks;
}

int intPow(int a, int b)
{
    int val = 1;
    for (int i = 0; i < b; i++)
    {
        val *= a;
    }
    return val;
}
